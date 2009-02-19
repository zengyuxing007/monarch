/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/io/InputStream.h"

#include "db/rt/Exception.h"

using namespace db::io;
using namespace db::rt;

InputStream::InputStream()
{
}

InputStream::~InputStream()
{
}

int InputStream::peek(char* b, int length, bool block)
{
   // extending classes must implement this method if they want support
   ExceptionRef e = new Exception(
      "InputStream::peek() is not implemented.",
      "db.io.NotImplemented.");
   Exception::setLast(e, false);
   return -1;
}

long long InputStream::skip(long long count)
{
   long long skipped = 0;
   
   // read and discard bytes
   char b[2048];
   int numBytes = 0;
   int length = (count < 2048) ? count : 2048;
   while(count > 0 && (numBytes = read(b, length)) > 0)
   {
      skipped += numBytes;
      count -= numBytes;
      length = (count < 2048) ? count : 2048;
   }
   
   if(skipped == 0 && numBytes == -1)
   {
      skipped = -1;
   }
   
   return skipped;
}
