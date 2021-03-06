/*
 * Copyright (c) 2008-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/DynamicObjectInputStream.h"

#include "monarch/io/ByteArrayOutputStream.h"

using namespace monarch::data;
using namespace monarch::io;
using namespace monarch::rt;

DynamicObjectInputStream::DynamicObjectInputStream(
   DynamicObject& dyno, DynamicObjectWriter* writer, bool cleanup) :
   mHasData(false),
   mDyno(dyno),
   mWriter(writer),
   mCleanupWriter(cleanup),
   mBufferInputStream(NULL, 0)
{
}

DynamicObjectInputStream::~DynamicObjectInputStream()
{
   if(mCleanupWriter)
   {
      delete mWriter;
   }
}

int DynamicObjectInputStream::read(char* b, int length)
{
   int rval = 0;

   if(!mHasData)
   {
      // do full serialization into buffer
      ByteArrayOutputStream baos(&mBuffer, true);
      if(mWriter->write(mDyno, &baos))
      {
         mBufferInputStream.setByteArray(mBuffer.data(), mBuffer.length());
         mHasData = true;
      }
      else
      {
         rval = -1;
      }
   }

   if(rval == 0)
   {
      // Conversion done, just feed more data out
      rval = mBufferInputStream.read(b, length);
   }

   return rval;
}

DynamicObjectWriter* DynamicObjectInputStream::getWriter()
{
   return mWriter;
}
