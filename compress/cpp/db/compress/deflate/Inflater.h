/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_compress_deflate_Inflater_H
#define db_compress_deflate_Inflater_H

#include "db/io/DataMutationAlgorithm.h"

#include <zlib.h>

namespace db
{
namespace compress
{
namespace deflate
{

/**
 * An Inflater is used to uncompress data that has been compressed using
 * the DEFLATE algorithm.
 * 
 * It should be used in conjunction with a mutator input/output stream.
 * 
 * This implementation simply wraps zlib (www.zlib.org).
 * 
 * @author Dave Longley
 */
class Inflater : public db::io::DataMutationAlgorithm
{
protected:
   /**
    * The zip stream for decompressing data.
    */
   
   
public:
   /**
    * Creates a new Inflater.
    */
   Inflater();
   
   /**
    * Destructs this Inflater.
    */
   virtual ~Inflater();
   
   /**
    * Gets data out of the source ByteBuffer, mutates it in some implementation
    * specific fashion, and then puts it in the destination ByteBuffer. The
    * actual number of mutated bytes is returned, which may be zero if there
    * are not enough bytes in the source buffer to produce mutated bytes.
    * 
    * Note: The destination buffer will be resized to accommodate any mutated
    * bytes.
    * 
    * @param src the source ByteBuffer with bytes to mutate.
    * @param dest the destination ByteBuffer to write the mutated bytes to.
    * @param finish true to finish the mutation algorithm, false not to.
    * 
    * @return 1 if there was enough data in the source buffer to run the
    *         mutation algorithm (which may or may not produce mutated bytes),
    *         0 if more data is required, or -1 if an exception occurred.
    */
   virtual int mutateData(
      db::io::ByteBuffer* src, db::io::ByteBuffer* dest, bool finish);
};

} // end namespace deflate
} // end namespace compress
} // end namespace db
#endif