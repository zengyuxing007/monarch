/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_io_FileInputStream_H
#define db_io_FileInputStream_H

#include "db/io/File.h"
#include "db/io/InputStream.h"

#include <fstream>

namespace db
{
namespace io
{

/**
 * A FileInputStream is used to read bytes from a File.
 * 
 * @author Dave Longley
 */
class FileInputStream : public InputStream
{
protected:
   /**
    * The File to read from.
    */
   File* mFile;
   
   /**
    * The ifstream to read with.
    */
   std::ifstream mStream;
   
   /**
    * Ensures the file is open for reading.
    * 
    * @return true if the file is opened for reading, false if it cannot be
    *         opened.
    */
   bool ensureOpen();

public:
   /**
    * Creates a new FileInputStream that opens the passed File for reading.
    * 
    * @param file the File to read from.
    */
   FileInputStream(File* file);
   
   /**
    * Destructs this FileInputStream.
    */
   virtual ~FileInputStream();
   
   /**
    * Reads some bytes from the stream. This method will block until at least
    * one byte can be read or until the end of the stream is reached. A
    * value of 0 will be returned if the end of the stream has been reached,
    * a value of -1 will be returned if an IO exception occurred, otherwise
    * the number of bytes read will be returned.
    * 
    * @param b the array of bytes to fill.
    * @param length the maximum number of bytes to read into the buffer.
    * 
    * @return the number of bytes read from the stream or 0 if the end of the
    *         stream has been reached or -1 if an IO exception occurred.
    */
   virtual int read(char* b, int length);   
   
   // FIXME: implement peek, do a read then an unget with ifstream
   
   /**
    * Closes the stream.
    */
   virtual void close();
};

} // end namespace io
} // end namespace db
#endif