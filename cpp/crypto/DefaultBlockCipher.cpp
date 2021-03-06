/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/crypto/DefaultBlockCipher.h"

#include "monarch/crypto/SymmetricKeyFactory.h"
#include "monarch/rt/Exception.h"

#include <openssl/err.h>

using namespace monarch::crypto;
using namespace monarch::io;
using namespace monarch::rt;

DefaultBlockCipher::DefaultBlockCipher() : AbstractBlockCipher(true)
{
}

DefaultBlockCipher::~DefaultBlockCipher()
{
}

bool DefaultBlockCipher::startEncrypting(
   const char* algorithm, SymmetricKey* symmetricKey)
{
   // create a symmetric key
   SymmetricKeyFactory factory;
   factory.createKey(algorithm, symmetricKey);

   // start encrypting
   return startEncrypting(symmetricKey);
}

bool DefaultBlockCipher::startEncrypting(SymmetricKey* symmetricKey)
{
   bool rval = false;

   // enable encryption mode
   mEncryptMode = true;

   // reset input and output bytes
   mInputBytes = mOutputBytes = 0;

   // get the cipher function
   mCipherFunction = getCipherFunction(symmetricKey->getAlgorithm());
   if(mCipherFunction != NULL)
   {
      // initialize encryption
      if(EVP_EncryptInit_ex(
         &mCipherContext, mCipherFunction, NULL,
         (unsigned char*)symmetricKey->data(),
         (unsigned char*)symmetricKey->iv()) == 1)
      {
         rval = true;
      }
      else
      {
         ExceptionRef e = new Exception(
            "Could not start encrypting.",
            "monarch.crypto.BlockCipher.StartEncryptionError");
         e->getDetails()["error"] = ERR_error_string(ERR_get_error(), NULL);
         Exception::set(e);
      }
   }

   return rval;
}

bool DefaultBlockCipher::startDecrypting(SymmetricKey* symmetricKey)
{
   bool rval = false;

   // disable encryption mode
   mEncryptMode = false;

   // reset input and output bytes
   mInputBytes = mOutputBytes = 0;

   // get the cipher function
   mCipherFunction = getCipherFunction(symmetricKey->getAlgorithm());
   if(mCipherFunction != NULL)
   {
      // initialize decryption
      if(EVP_DecryptInit_ex(
         &mCipherContext, mCipherFunction, NULL,
         (unsigned char*)symmetricKey->data(),
         (unsigned char*)symmetricKey->iv()) == 1)
      {
         rval = true;
      }
      else
      {
         ExceptionRef e = new Exception(
            "Could not start decrypting.",
            "monarch.crypto.BlockCipher.StartDecryptionError");
         e->getDetails()["error"] = ERR_error_string(ERR_get_error(), NULL);
         Exception::set(e);
      }
   }

   return rval;
}

bool DefaultBlockCipher::update(
   const char* in, int inLength, char* out, int& outLength)
{
   bool rval = false;

   // only proceed if the cipher function has been set
   if(mCipherFunction != NULL)
   {
      if(isEncryptEnabled())
      {
         // encrypt more data
         if(EVP_EncryptUpdate(
            &mCipherContext, (unsigned char*)out, &outLength,
            (unsigned char*)in, inLength) == 1)
         {
            rval = true;
         }
         else
         {
            ExceptionRef e = new Exception(
               "Could not encrypt data.",
               "monarch.crypto.BlockCipher.EncryptionError");
            e->getDetails()["error"] = ERR_error_string(ERR_get_error(), NULL);
            Exception::set(e);
         }
      }
      else
      {
         // decrypt more data
         if(EVP_DecryptUpdate(
            &mCipherContext, (unsigned char*)out, &outLength,
            (unsigned char*)in, inLength) == 1)
         {
            rval = true;
         }
         else
         {
            ExceptionRef e = new Exception(
               "Could not decrypt data.",
               "monarch.crypto.BlockCipher.DecryptionError");
            e->getDetails()["error"] = ERR_error_string(ERR_get_error(), NULL);
            Exception::set(e);
         }
      }

      if(rval)
      {
         // update input and output bytes
         mInputBytes += inLength;
         mOutputBytes += outLength;
      }
   }
   else
   {
      ExceptionRef e = new Exception(
         "Cannot update cipher; cipher not started.",
         "monarch.crypto.BlockCipher.MethodCallOutOfOrder");
      e->getDetails()["error"] = ERR_error_string(ERR_get_error(), NULL);
      Exception::set(e);
   }

   return rval;
}

bool DefaultBlockCipher::finish(char* out, int& length)
{
   bool rval = false;

   // only proceed if the cipher function has been set
   if(mCipherFunction != NULL)
   {
      if(isEncryptEnabled())
      {
         // finish encrypting
         if(EVP_EncryptFinal_ex(
            &mCipherContext, (unsigned char*)out, &length) == 1)
         {
            rval = true;
         }
         else
         {
            ExceptionRef e = new Exception(
               "Could not finish encrypting.",
               "monarch.crypto.BlockCipher.EncryptionError");
            e->getDetails()["error"] = ERR_error_string(ERR_get_error(), NULL);
            Exception::set(e);
         }
      }
      else
      {
         // finish decrypting
         if(EVP_DecryptFinal_ex(
            &mCipherContext, (unsigned char*)out, &length) == 1)
         {
            rval = true;
         }
         else
         {
            ExceptionRef e = new Exception(
               "Could not finish decrypting.",
               "monarch.crypto.BlockCipher.DecryptionError");
            e->getDetails()["error"] = ERR_error_string(ERR_get_error(), NULL);
            Exception::set(e);
         }
      }

      if(rval)
      {
         // update output bytes
         mOutputBytes += length;
      }
   }
   else
   {
      ExceptionRef e = new Exception(
         "Cannot finish cipher; cipher not started.",
         "monarch.crypto.BlockCipher.MethodCallOutOfOrder");
      e->getDetails()["error"] = ERR_error_string(ERR_get_error(), NULL);
      Exception::set(e);
   }

   return rval;
}

uint64_t DefaultBlockCipher::getTotalInput()
{
   return mInputBytes;
}

uint64_t DefaultBlockCipher::getTotalOutput()
{
   return mOutputBytes;
}
