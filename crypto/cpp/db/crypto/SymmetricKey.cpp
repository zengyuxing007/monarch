/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/crypto/SymmetricKey.h"

using namespace db::crypto;

SymmetricKey::SymmetricKey(const char* algorithm)
{
   // no key data yet
   mData = NULL;
   mDataLength = 0;
   
   // no IV yet
   mIv = NULL;
   mIvLength = 0;
   
   // set algorithm
   mAlgorithm = new char[strlen(algorithm) + 1];
   strcpy(mAlgorithm, algorithm);
   
   // default to unencrypted
   mEncrypted = false;
}

SymmetricKey::~SymmetricKey()
{
   // clean up
   freeData();
   freeIv();
   delete mAlgorithm;
}

void SymmetricKey::freeData()
{
   // free key data
   if(mData != NULL)
   {
      delete [] mData;
   }
   
   mData = NULL;
   mDataLength = 0;
}

void SymmetricKey::freeIv()
{
   // free IV
   if(mIv != NULL)
   {
      delete [] mIv;
   }
   
   mIv = NULL;
   mIvLength = 0;
}

void SymmetricKey::assignData(
   char* data, unsigned int length,
   char* iv, unsigned int ivLength, bool encrypted)
{
   // free existing data and IV
   freeData();
   freeIv();
   
   // set new data
   mData = data;
   mDataLength = length;
   mIv = iv;
   mIvLength = ivLength;
   
   // set encrypted flag
   mEncrypted = encrypted;
}

void SymmetricKey::setData(
   const char* data, unsigned int length,
   const char* iv, unsigned int ivLength, bool encrypted)
{
   // free existing data as necessary
   if(mData != NULL && mDataLength <= length)
   {
      freeData();
   }
   
   // free existing IV as necessary
   if(iv == NULL || (mIv != NULL && mIvLength <= ivLength))
   {
      freeIv();
   }
   
   // allocate data as necessary
   if(mData == NULL)
   {
      mData = new char[length];
   }
   
   // allocate IV as necessary
   if(mIv == NULL && iv != NULL)
   {
      mIv = new char[ivLength];
   }
   
   // copy data
   memcpy(mData, data, length);
   mDataLength = length;
   
   // copy iv
   if(iv != NULL)
   {
      memcpy(mIv, iv, ivLength);
      mIvLength = ivLength;
   }
   
   // set encrypted flag
   mEncrypted = encrypted;
}

void SymmetricKey::getData(
   char** data, unsigned int& length,
   char** iv, unsigned int& ivLength)
{
   *data = mData;
   length = mDataLength;
   *iv = mIv;
   ivLength = mIvLength;
}

const char* SymmetricKey::getAlgorithm()
{
   return mAlgorithm;
}

bool SymmetricKey::isEncrypted()
{
   return mEncrypted;
}