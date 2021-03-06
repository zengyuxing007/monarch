/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_crypto_X509Certificate_H
#define monarch_crypto_X509Certificate_H

#include "monarch/crypto/PublicKey.h"
#include "monarch/rt/DynamicObject.h"

#include <openssl/x509.h>

namespace monarch
{
namespace crypto
{

/**
 * An X509Certificate represents an X.509 certificate. It uses OpenSSL's
 * implementation.
 *
 * @author Dave Longley
 */
class X509Certificate
{
protected:
   /**
    * The underlying X509 openssl data structure.
    */
   X509* mX509;

   /**
    * The public key in this certificate.
    */
   PublicKeyRef mPublicKey;

public:
   /**
    * Creates a new X509Certificate from an X.509 structure.
    *
    * @param x509 the x509 data structure (to be freed by this instance).
    */
   X509Certificate(X509* x509);

   /**
    * Destructs this X509Certificate.
    */
   virtual ~X509Certificate();

   /**
    * Gets a pointer to the OpenSSL X509 structure that stores the cert data.
    *
    * @return a pointer to the OpenSSL X509 structure that stores the cert data.
    */
   virtual X509* getX509();

   /**
    * Gets the X.509 version for this certificate.
    *
    * @return the X.509 for this certificate.
    */
   virtual int64_t getVersion();

   /**
    * Gets the public key for this certificate.
    *
    * @return the public key for this certificate.
    */
   virtual PublicKeyRef& getPublicKey();

   /**
    * Gets the subject information for this certificate.
    *
    * @return the subject information for this certificate.
    */
   virtual monarch::rt::DynamicObject getSubject();

   /**
    * Gets the issuer information for this certificate.
    *
    * @return the issuer information for this certificate.
    */
   virtual monarch::rt::DynamicObject getIssuer();

   /**
    * Gets the extension information for this certificate.
    *
    * @return the extension information for this certificate.
    */
   virtual monarch::rt::DynamicObject getExtensions();

   /**
    * A convenience method that gets a field value from a subject or issuer
    * based on its shortname, ie "CN" will get the value of the common name
    * in the given subject or issuer.
    *
    * @param subjectOrIssuer a certificate subject or issuer.
    * @param sn the shortname, ie "CN" for common name, "O" for organization.
    *
    * @return the value of the given shortname.
    */
   static std::string getField(
      monarch::rt::DynamicObject& subjectOrIssuer, const char* sn);
};

// typedef for a reference-counted X509Certificate
typedef monarch::rt::Collectable<X509Certificate> X509CertificateRef;

} // end namespace crypto
} // end namespace monarch
#endif
