/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/http/HttpChunkedTransferInputStream.h"

#include "monarch/rt/DynamicObject.h"
#include "monarch/util/Convert.h"

#include <cstdlib>
#include <climits>

using namespace std;
using namespace monarch::io;
using namespace monarch::http;
using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::util;

HttpChunkedTransferInputStream::HttpChunkedTransferInputStream(
   ConnectionInputStream* is, HttpTrailer* trailer) :
PeekInputStream(is, false)
{
   // store trailer
   mTrailer = trailer;

   // no current chunk yet
   mChunkBytesLeft = 0;

   // not last chunk yet
   mLastChunk = false;

   // store the thread reading from this stream
   mThread = Thread::currentThread();
}

HttpChunkedTransferInputStream::~HttpChunkedTransferInputStream()
{
}

int HttpChunkedTransferInputStream::read(char* b, int length)
{
   int rval = 0;

   // get underlying connection input stream
   ConnectionInputStream* is =
      static_cast<ConnectionInputStream*>(mInputStream);

   if(mChunkBytesLeft == 0 && !mLastChunk)
   {
      // read chunk-size
      string chunkSize;
      rval = is->readCrlf(chunkSize);
      if(rval == 1)
      {
         // ignore chunk-extension
         int sizeLength = chunkSize.length();
         const char* size = strchr(chunkSize.c_str(), ' ');
         if(size != NULL)
         {
            sizeLength = size - chunkSize.c_str();
         }

         // get size of chunk data
         unsigned int tempBytesLeft;
         if(!Convert::hexToInt(chunkSize.c_str(), sizeLength, tempBytesLeft) ||
            tempBytesLeft > INT_MAX)
         {
            // the chunk size could not be read!
            ExceptionRef e = new Exception(
               "Invalid HTTP chunk size.",
               "monarch.net.http.BadChunkedEncoding");
            Exception::set(e);
            rval = -1;
         }
         else
         {
            // this is the last chunk if length is 0
            mChunkBytesLeft = (int)tempBytesLeft;
            mLastChunk = (mChunkBytesLeft == 0);
            rval = 0;
         }
      }
      else
      {
         if(rval == 0)
         {
            // the chunk size could not be read!
            ExceptionRef e = new Exception(
               "Could not read HTTP chunk size. End of stream.",
               "monarch.net.http.EndOfStream");
            Exception::set(e);
            rval = -1;
         }
         else
         {
            // create special exception if not a blocking exception
            ExceptionRef e = Exception::get();
            if(!e->getDetails()->hasMember("wouldBlock"))
            {
               // the chunk size could not be read!
               e = new Exception(
                  "Could not read HTTP chunk size.",
                  "monarch.net.http.BadChunkedEncoding");
               Exception::push(e);
            }
         }
      }
   }

   // read some chunk bytes into the passed data buffer
   int numBytes = 1;
   if(mChunkBytesLeft > 0 && rval != -1 && numBytes > 0 &&
      !mThread->isInterrupted())
   {
      int readSize = (length < mChunkBytesLeft) ? length : mChunkBytesLeft;
      numBytes = is->read(b, readSize);
      if(numBytes > 0)
      {
         // decrement bytes left
         mChunkBytesLeft -= numBytes;

         // increment bytes read
         rval += numBytes;
      }
      else
      {
         ExceptionRef e = new Exception(
            "Could not read HTTP chunk.",
            "monarch.net.http.BadChunkedEncoding");
         Exception::set(e);
         rval = -1;
      }
   }

   if(rval != -1)
   {
      // if this is the last chunk, then read in the
      // chunk trailer and last CRLF
      if(mLastChunk)
      {
         // build trailer headers
         string trailerHeaders;
         string line;
         while(is->readCrlf(line) > 0 && line.length() > 0)
         {
            trailerHeaders.append(line);
            trailerHeaders.append(HttpHeader::CRLF);
         }

         // parse trailer headers, if appropriate
         if(mTrailer != NULL)
         {
            mTrailer->parse(trailerHeaders);
         }
      }
      else if(mChunkBytesLeft == 0)
      {
         // read chunk-data CRLF
         string throwout;
         is->readCrlf(throwout);
      }
      else if(numBytes == 0)
      {
         // if the chunk bytes left is greater than zero and end of stream
         // was read, then whole chunk wasn't read
         ExceptionRef e = new Exception(
            "Could not read entire HTTP chunk.",
            "monarch.net.http.BadChunkedEncoding");
         Exception::set(e);
         rval = -1;
      }
   }

   return rval;
}

void HttpChunkedTransferInputStream::close()
{
   // reset
   mChunkBytesLeft = 0;
   mLastChunk = false;

   // do not close underlying stream
}
