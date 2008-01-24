/*
 * Copyright (c) 2005-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/riff/RiffListHeader.h"

using namespace db::data;
using namespace db::data::riff;
using namespace db::io;

RiffListHeader::RiffListHeader(fourcc_t id, uint32_t size) :
   mChunkHeader(CHUNK_ID, size + 4)
{
   mId = id;
}

RiffListHeader::~RiffListHeader()
{
}

bool RiffListHeader::writeTo(OutputStream& os)
{
   bool rval;
   
   const int size = RiffChunkHeader::HEADER_SIZE + HEADER_SIZE; 
   char buf[size];
   convertToBytes(buf);
   rval = os.write(buf, size);
   
   return rval;
}   

void RiffListHeader::convertToBytes(char* b)
{
   mChunkHeader.convertToBytes(b);
   DB_FOURCC_TO_STR(mId, b + RiffChunkHeader::HEADER_SIZE);
}

bool RiffListHeader::convertFromBytes(const char* b, int length)
{
   bool rval = false;
   
   if(b != NULL && length >= HEADER_SIZE)
   {
      if(mChunkHeader.convertFromBytes(b, length))
      {
         // make sure chunk identifier is LIST
         if(mChunkHeader.getIdentifier() == CHUNK_ID)
         {
            mId = DB_FOURCC_FROM_STR(b + RiffChunkHeader::HEADER_SIZE);
            rval = true;
         }
      }
   }
   
   setValid(rval);
   
   return rval;      
}

void RiffListHeader::setIdentifier(fourcc_t id)
{
   mId = id;
}

fourcc_t RiffListHeader::getIdentifier()
{
   return mId;
}

void RiffListHeader::setChunkSize(uint32_t size)
{
   mChunkHeader.setChunkSize(size);
}

uint32_t RiffListHeader::getChunkSize()
{
   return mChunkHeader.getChunkSize();
}   

void RiffListHeader::setListSize(uint32_t size)
{
   mChunkHeader.setChunkSize(size + HEADER_SIZE);
}

uint32_t RiffListHeader::getListSize()
{
   return mChunkHeader.getChunkSize() - HEADER_SIZE;
}

bool RiffListHeader::isValid()
{
   return mChunkHeader.isValid();
}

void RiffListHeader::setValid(bool valid)
{
   mChunkHeader.setValid(valid);
}