/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/crypto/AsymmetricKeyFactory.h"

#include "monarch/rt/System.h"
#include "monarch/rt/DynamicObject.h"
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/rt/Exception.h"
#include "monarch/util/Math.h"

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/x509v3.h>

#include <cstring>

using namespace std;
using namespace monarch::crypto;
using namespace monarch::rt;
using namespace monarch::util;

#define EXCEPTION_UNSUPPORTED_ALGORITHM "monarch.crypto.UnsupportedAlgorithm"
#define EXCEPTION_PRIVATE_KEY_IO        "monarch.crypto.PrivateKey.IO"
#define EXCEPTION_PUBLIC_KEY_IO         "monarch.crypto.PublicKey.IO"
#define EXCEPTION_CERTIFICATE_IO        "monarch.crypto.Certificate.IO"

AsymmetricKeyFactory::AsymmetricKeyFactory()
{
}

AsymmetricKeyFactory::~AsymmetricKeyFactory()
{
}

/**
 * A callback function that is called to obtain a password to unlock
 * an encrypted key.
 *
 * @param b the buffer to populate with a password.
 * @param length the length of the buffer to populate.
 * @param flag a flag that is reserved for future use.
 * @param userData a pointer to some user data.
 *
 * @return the length of the password.
 */
static int passwordCallback(char* b, int length, int flag, void* userData)
{
   // interpret user data as a const char*
   const char* password = (const char*)userData;
   if(password != NULL)
   {
      // truncate the password length as necessary (leave room in the
      // passed buffer for a NULL terminator)
      int len = strlen(password);
      length = (len < length - 1) ? len : length - 1;

      // copy the password into the given buffer
      memcpy(b, password, length);
   }
   else
   {
      // no password available
      length = 0;
   }

   // return the password length
   return length;
}

void AsymmetricKeyFactory::createDsaKeyPair(
   PrivateKeyRef& privateKey, PublicKeyRef& publicKey)
{
   // generate DSA parameters
   DSA* dsa = DSA_generate_parameters(
      2048, NULL, 0, NULL, NULL, NULL, NULL);
   if(dsa != NULL)
   {
      // generate DSA keys
      if(DSA_generate_key(dsa) == 1)
      {
         // store private/public key parameters
         BIGNUM* p = dsa->p;
         BIGNUM* q = dsa->q;
         BIGNUM* g = dsa->g;
         BIGNUM* x = dsa->priv_key;
         BIGNUM* y = dsa->pub_key;

         // clear private parameters
         dsa->priv_key = NULL;

         // create public key
         EVP_PKEY* pub = EVP_PKEY_new();
         EVP_PKEY_set1_DSA(pub, dsa);
         publicKey = new PublicKey(pub);

         // clear public parameters
         dsa->p = NULL;
         dsa->q = NULL;
         dsa->g = NULL;
         dsa->pub_key = NULL;

         // restore private parameters
         dsa->priv_key = x;

         // create private key
         EVP_PKEY* priv = EVP_PKEY_new();
         EVP_PKEY_set1_DSA(priv, dsa);
         privateKey = new PrivateKey(priv);

         // restore public parameters
         dsa->p = p;
         dsa->q = q;
         dsa->g = g;
         dsa->pub_key = y;

         // free DSA
         DSA_free(dsa);
      }
   }
}

void AsymmetricKeyFactory::createRsaKeyPair(
   PrivateKeyRef& privateKey, PublicKeyRef& publicKey)
{
   // generate RSA keys
   RSA* rsa = RSA_generate_key(2048, 65537, NULL, NULL);
   if(rsa != NULL)
   {
      // store private/public key parameters
      BIGNUM* d = rsa->d;
      BIGNUM* e = rsa->e;
      BIGNUM* p = rsa->p;
      BIGNUM* q = rsa->q;
      BIGNUM* dmp1 = rsa->dmp1;
      BIGNUM* dmq1 = rsa->dmq1;
      BIGNUM* iqmp = rsa->iqmp;

      // clear public parameters
      rsa->e = NULL;

      // create private key
      EVP_PKEY* priv = EVP_PKEY_new();
      EVP_PKEY_set1_RSA(priv, rsa);
      privateKey = new PrivateKey(priv);

      // clear private parameters
      rsa->d = NULL;
      rsa->p = NULL;
      rsa->q = NULL;
      rsa->dmp1 = NULL;
      rsa->dmq1 = NULL;
      rsa->iqmp = NULL;

      // restore public parameters
      rsa->e = e;

      // create public key
      EVP_PKEY* pub = EVP_PKEY_new();
      EVP_PKEY_set1_RSA(pub, rsa);
      publicKey = new PublicKey(pub);

      // restore private parameters
      rsa->d = d;
      rsa->p = p;
      rsa->q = q;
      rsa->dmp1 = dmp1;
      rsa->dmq1 = dmq1;
      rsa->iqmp = iqmp;

      // free RSA
      RSA_free(rsa);
   }
}

bool AsymmetricKeyFactory::createKeyPair(
   const char* algorithm, PrivateKeyRef& privateKey, PublicKeyRef& publicKey)
{
   bool rval = true;

   // set private and public keys to null
   privateKey.setNull();
   publicKey.setNull();

   // add random bytes from the time
   struct timeval tv;
   gettimeofday(&tv, 0);
   RAND_add(&tv, sizeof(tv), 0.0);

   if(strcmp(algorithm, "DSA") == 0)
   {
      // create DSA key pair
      createDsaKeyPair(privateKey, publicKey);
   }
   else if(strcmp(algorithm, "RSA") == 0)
   {
      // create RSA key pair
      createRsaKeyPair(privateKey, publicKey);
   }
   else
   {
      // unknown algorithm
      ExceptionRef e = new Exception(
         "Key algorithm is not supported.",
         EXCEPTION_UNSUPPORTED_ALGORITHM);
      e->getDetails()["algorithm"] = algorithm;
      Exception::set(e);
      rval = false;
   }

   return rval;
}

PrivateKeyRef AsymmetricKeyFactory::loadPrivateKeyFromPem(
   const char* pem, int length, const char* password)
{
   PrivateKeyRef key;

   // create a read-only memory bio
   BIO* bio = BIO_new_mem_buf((void*)pem, length);

   // try to load private key from bio
   EVP_PKEY* pkey = NULL;
   if(password != NULL)
   {
      // use password to load key
      pkey = PEM_read_bio_PrivateKey(
         bio, &pkey, passwordCallback, (void*)password);
   }
   else
   {
      // no password provided
      pkey = PEM_read_bio_PrivateKey(bio, &pkey, NULL, NULL);
   }

   // free the bio
   BIO_free_all(bio);

   if(pkey != NULL)
   {
      // wrap the PKEY structure in a PrivateKey
      key = new PrivateKey(pkey);
   }
   else
   {
      ExceptionRef e = new Exception(
         "Could not load private key from PEM.",
         EXCEPTION_PRIVATE_KEY_IO);
      e->getDetails()["error"] = ERR_error_string(ERR_get_error(), NULL);
      Exception::set(e);
   }

   return key;
}

string AsymmetricKeyFactory::writePrivateKeyToPem(
   PrivateKeyRef& key, const char* password)
{
   string rval;

   // create a memory BIO
   BIO* bio = BIO_new(BIO_s_mem());

   // write the key to the bio
   int error;
   if(password != NULL)
   {
      // use password
      error = PEM_write_bio_PKCS8PrivateKey(
         bio, key->getPKEY(), EVP_des_ede3_cbc(),
         NULL, 0, passwordCallback, (void*)password);
   }
   else
   {
      // no password
      error = PEM_write_bio_PKCS8PrivateKey(
         bio, key->getPKEY(), NULL,
         NULL, 0, NULL, NULL);
   }

   if(error != 0)
   {
      // get the memory buffer from the bio
      BUF_MEM* mem;
      BIO_get_mem_ptr(bio, &mem);

      // add characters to the string
      rval.append(mem->data, mem->length);

      // free the bio
      BIO_free(bio);
   }
   else
   {
      ExceptionRef e = new Exception(
         "Could not write private key to PEM.",
         EXCEPTION_PRIVATE_KEY_IO);
      e->getDetails()["error"] = ERR_error_string(ERR_get_error(), NULL);
      Exception::set(e);
   }

   return rval;
}

PublicKeyRef AsymmetricKeyFactory::loadPublicKeyFromPem(
   const char* pem, int length)
{
   PublicKeyRef key;

   // create a read-only memory bio
   BIO* bio = BIO_new_mem_buf((void*)pem, length);
   BIO_set_close(bio, BIO_NOCLOSE);

   // try to load public key from bio
   EVP_PKEY* pkey = NULL;
   pkey = PEM_read_bio_PUBKEY(bio, &pkey, NULL, NULL);

   // free the bio
   BIO_free(bio);

   if(pkey != NULL)
   {
      // wrap the PKEY structure in a PublicKey
      key = new PublicKey(pkey);
   }
   else
   {
      ExceptionRef e = new Exception(
         "Could not load public key from PEM.",
         EXCEPTION_PUBLIC_KEY_IO);
      e->getDetails()["error"] = ERR_error_string(ERR_get_error(), NULL);
      Exception::set(e);
   }

   return key;
}

string AsymmetricKeyFactory::writePublicKeyToPem(PublicKeyRef& key)
{
   string rval;

   // create a memory BIO
   BIO* bio = BIO_new(BIO_s_mem());

   // write the key to the bio
   int error = PEM_write_bio_PUBKEY(bio, key->getPKEY());
   if(error != 0)
   {
      // get the memory buffer from the bio
      BUF_MEM* mem;
      BIO_get_mem_ptr(bio, &mem);

      // add characters to the string
      rval.append(mem->data, mem->length);

      // free the bio
      BIO_free(bio);
   }
   else
   {
      ExceptionRef e = new Exception(
         "Could not write public key to PEM.",
         EXCEPTION_PUBLIC_KEY_IO);
      e->getDetails()["error"] = ERR_error_string(ERR_get_error(), NULL);
      Exception::set(e);
   }

   return rval;
}

X509CertificateRef AsymmetricKeyFactory::createCertificate(
   uint32_t version,
   PrivateKeyRef& privateKey, PublicKeyRef& publicKey,
   DynamicObject subject, DynamicObject issuer,
   Date* startDate, Date* endDate, BigInteger& serial,
   DynamicObject* extensions, X509CertificateRef* issuerCert)
{
   X509CertificateRef rval(NULL);

   /* Structure of a v3 X.509 certificate:
      Certificate
         Version
         Serial Number
         Algorithm ID
         Issuer
         Validity
            + Not Before
            + Not After
         Subject
         Subject Public Key Info
            + Public Key Algorithm
            + Subject Public Key
         Issuer Unique Identifier (Optional)  // introduced in v2
         Subject Unique Identifier (Optional) // introduced in v2
         Extensions (Optional)                // introduced in v3
            + ...
      Certificate Signature Algorithm
      Certificate Signature
    */

   bool pass;
   bool extError = false;

   // create certificate object, v1 = 0x0, v2 = 0x1, v3 = 0x2
   X509* x509 = X509_new();
   pass = (X509_set_version(x509, version) != 0);

   // set serial number
   pass = pass && BN_to_ASN1_INTEGER(
      serial.getBIGNUM(), X509_get_serialNumber(x509));

   // get starting date and ending dates in seconds relative to now
   time_t now = Date().getSeconds();
   time_t startSeconds = (startDate != NULL) ?
      (startDate->getSeconds() - now) : 0;
   time_t endSeconds = (endDate != NULL) ?
      (endDate->getSeconds() - now) : 0;

   // set not before to current time, set not after to given days
   pass = pass &&
      (X509_gmtime_adj(X509_get_notBefore(x509), startSeconds) != NULL) &&
      (X509_gmtime_adj(X509_get_notAfter(x509), endSeconds) != NULL);

   // assign public key to certificate
   pass = pass && X509_set_pubkey(x509, publicKey->getPKEY());

   // get the subject so its entry can be modified
   X509_NAME* sname = X509_get_subject_name(x509);

   /* Add attributes for subject:
   CN: Common Name (site's domain, i.e. localhost, myserver.com)
   OU: Organizational Unit
   O : Organization
   L : Locality (city, i.e. New York)
   ST: State (i.e., Virginia)
   C : Country (i.e., US)
   */
   // Notes:
   // MBSTRING_UTF8 means the entry is of type utf-8 "bytes"
   // first -1 tells the function to use strlen() to get the length
   // second -1 and 0 tells the function to append the entry
   if(pass)
   {
      DynamicObjectIterator i = subject.getIterator();
      while(pass && i->hasNext())
      {
         DynamicObject& item = i->next();
         pass = X509_NAME_add_entry_by_txt(
            sname, item["type"]->getString(), MBSTRING_UTF8,
            (const unsigned char*)item["value"]->getString(), -1, -1, 0);
      }
   }

   // get the issuer so its entry can be modified
   X509_NAME* iname = X509_get_issuer_name(x509);

   // build issuer
   if(pass)
   {
      DynamicObjectIterator i = issuer.getIterator();
      while(pass && i->hasNext())
      {
         DynamicObject& item = i->next();
         pass = X509_NAME_add_entry_by_txt(
            iname, item["type"]->getString(), MBSTRING_UTF8,
            (const unsigned char*)item["value"]->getString(), -1, -1, 0);
      }
   }

   // add extensions, if any
   if(pass && version >= 0x2 && extensions != NULL)
   {
      X509V3_CTX ctx;
      X509* issuerX509 = (issuerCert == NULL) ? NULL : (*issuerCert)->getX509();
      // ctx, issuer, subject, request, revocation list, flags
      X509V3_set_ctx(&ctx, issuerX509, x509, NULL, NULL, 0);
      X509_EXTENSION* ext;

      // add extensions, examples of extensions are:
      // "basicConstraints" => "critical,CA:FALSE"
      // "basicConstraints" => "critical,CA:TRUE"
      // "keyUsage" => "digitalSignature,keyEncipherment"
      // "keyUsage" => "keyCertSign,cRLSign"
      // "authorityKeyIdentifier" => "keyid, issuer:always"
      // "extendedKeyUsage" => "serverAuth,clientAuth"
      DynamicObjectIterator i = extensions->getIterator();
      while(pass && i->hasNext())
      {
         DynamicObject& item = i->next();
         ext = X509V3_EXT_conf(
            NULL, &ctx,
            (char*)item["type"]->getString(),
            (char*)item["value"]->getString());
         if(ext == NULL)
         {
            ExceptionRef e = new Exception(
               "Could not add extension.",
               "monarch.crypto.Certificate.InvalidExtension");
            e->getDetails()["type"] = item["type"]->getString();
            Exception::set(e);
            pass = false;
            extError = true;
         }
         else
         {
            X509_add_ext(x509, ext, -1);
            X509_EXTENSION_free(ext);
         }
      }
   }

   // sign certificate
   if(pass)
   {
      const EVP_MD* hashAlgorithm = NULL;
      const char* algorithm = privateKey->getAlgorithm();
      if(strcmp(algorithm, "RSA") == 0)
      {
         hashAlgorithm = EVP_sha1();
      }
      else if(strcmp(algorithm, "DSA") == 0)
      {
         hashAlgorithm = EVP_dss1();
      }
      else
      {
         // unknown algorithm
         ExceptionRef e = new Exception(
            "Key algorithm is not supported.",
            EXCEPTION_UNSUPPORTED_ALGORITHM);
         e->getDetails()["algorithm"] = algorithm;
         Exception::set(e);
         pass = false;
      }

      pass = pass && X509_sign(x509, privateKey->getPKEY(), hashAlgorithm);
   }

   if(pass)
   {
      // create X509Certificate object
      rval = new X509Certificate(x509);
   }
   else
   {
      ExceptionRef e = new Exception(
         "Could not create X.509 certificate.",
         "monarch.crypto.Certificate.CreationError");
      e->getDetails()["subject"] = subject.clone();
      if(extensions != NULL)
      {
         e->getDetails()["extensions"] = extensions->clone();
      }
      e->getDetails()["error"] = ERR_error_string(ERR_get_error(), NULL);
      extError ? Exception::push(e) : Exception::set(e);
   }

   return rval;
}

X509CertificateRef AsymmetricKeyFactory::loadCertificateFromPem(
   const char* pem, int length)
{
   X509CertificateRef cert;

   // create a read-only memory bio
   BIO* bio = BIO_new_mem_buf((void*)pem, length);
   BIO_set_close(bio, BIO_NOCLOSE);

   // try to load certificate from bio
   X509* x509 = NULL;
   x509 = PEM_read_bio_X509(bio, &x509, NULL, NULL);

   // free the bio
   BIO_free(bio);

   if(x509 != NULL)
   {
      // wrap the X.509 structure in a X509Certificate
      cert = new X509Certificate(x509);
   }
   else
   {
      ExceptionRef e = new Exception(
         "Could not load X.509 certificate from PEM.",
         EXCEPTION_PUBLIC_KEY_IO);
      e->getDetails()["error"] = ERR_error_string(ERR_get_error(), NULL);
      Exception::set(e);
   }

   return cert;
}

string AsymmetricKeyFactory::writeCertificateToPem(X509CertificateRef& cert)
{
   string rval;

   // create a memory BIO
   BIO* bio = BIO_new(BIO_s_mem());

   // write the certificate to the bio
   int error = PEM_write_bio_X509(bio, cert->getX509());
   if(error != 0)
   {
      // get the memory buffer from the bio
      BUF_MEM* mem;
      BIO_get_mem_ptr(bio, &mem);

      // add characters to the string
      rval.append(mem->data, mem->length);

      // free the bio
      BIO_free(bio);
   }
   else
   {
      ExceptionRef e = new Exception(
         "Could not write X.509 certificate to PEM.",
         EXCEPTION_CERTIFICATE_IO);
      e->getDetails()["error"] = ERR_error_string(ERR_get_error(), NULL);
      Exception::set(e);
   }

   return rval;
}
