/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/ByteArrayInputStream.h"

#include <string>

using namespace db::io;

ByteArrayInputStream::ByteArrayInputStream(const char* b, int length)
{
   mBytes = b;
   mLength = length;
}

ByteArrayInputStream::~ByteArrayInputStream()
{
}

int ByteArrayInputStream::read(char* b, int length)
{
   int rval = 0;
   
   if(mLength > 0)
   {
      // copy bytes into passed buffer
      rval = (length > mLength) ? mLength : length;
      memcpy(b, mBytes, rval);
      
      // increment bytes pointer, decrement length
      mBytes += rval;
      mLength -= rval;
   }
   
   return rval;
}

void ByteArrayInputStream::setByteArray(const char* b, int length)
{
   mBytes = b;
   mLength = length;
}