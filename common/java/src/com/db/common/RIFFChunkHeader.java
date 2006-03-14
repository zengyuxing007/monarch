/*
 * Copyright (c) 2005 Digital Bazar, Inc.  All rights reserved.
 */
package com.db.common;

/**
 * A RIFF Chunk. The chunk size does *NOT* include the header size.
 * 
 * The format of a RIFF Chunk is such:
 * 
 * | bytes 1-4  | bytes 5-8  |  bytes 9+  |
 * ----------------------------------------
 * | identifier | chunk size | chunk data |
 * 
 * @author Dave Longley
 */
public class RIFFChunkHeader
{
   /**
    * The 4 byte chunk identifier. "JUNK" for junk chunks.
    */
   protected String mId;

   /**
    * The size of the chunk.
    */
   protected long mChunkSize;
   
   /**
    * Whether or not this chunk is valid.
    */
   protected boolean mValid;
   
   /**
    * Constants.
    */
   public static final int CHUNK_HEADER_SIZE = 8;
   
   /**
    * Constructs a RIFF Chunk Header with default values.
    */
   public RIFFChunkHeader()
   {
      this(null, CHUNK_HEADER_SIZE);
   }
   
   /**
    * Constructs a RIFF Chunk Header with the passed parameters.
    * 
    * @param id the identifier of the chunk.
    */
   public RIFFChunkHeader(String id)   
   {
      this(id, CHUNK_HEADER_SIZE);
   }
   
   /**
    * Constructs a RIFF Chunk Header with the passed parameters.
    * 
    * @param id the identifier of the chunk.
    * @param size the size of the chunk.
    */
   public RIFFChunkHeader(String id, int size)
   {
      if(id == null || id.length() != 4)
      {
         id = "";
      }
      else
      {
         mId = id;
      }
      
      mChunkSize = size;
      mValid = true;
   }
   
   /**
    * Converts size into a 4 byte array. The array is ordered from least
    * significant byte to most. 
    * 
    * @return a 4 byte array representing the size.
    */
   protected byte[] convertSizeToBytes()
   {
      byte[] size = new byte[4];
      
      // convert to 4 bytes, most significant byte first
      for(int i = 3; i >= 0; i--)
      {
         size[i] = (byte)(((mChunkSize >> (i * 8))) & 0xFF);
      }
      
      return size;
   }
   
   /**
    * Converts a 4 bytes from an array into an integer size. The 4 bytes are
    * ordered from least significant byte to most.
    * 
    * @param b the byte array to read from.
    * @param offset the offset to start reading from.
    * @param length number of valid bytes in the buffer following the
    *               offset.
    * @return true if successful, false if not. 
    */
   protected boolean convertBytesToSize(byte[] b, int offset, int length)
   {
      boolean rval = false;
      
      if(b != null && length > 3)
      {
         mChunkSize = 0;
         
         // least significant byte is first
         for(int i = 3; i >= 0; i--)
         {
            mChunkSize |= ((b[offset + i] & 0xFF) << (i * 8));
         }
         
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Converts the chunk header into an 8 byte array.
    * 
    * @return the chunk header into an 8 byte array. 
    */
   public byte[] convertToBytes()
   {
      byte[] chunk = new byte[CHUNK_HEADER_SIZE];
      
      if(mId.length() == 4)
      {
         chunk[0] = (byte)mId.charAt(0);
         chunk[1] = (byte)mId.charAt(1);
         chunk[2] = (byte)mId.charAt(2);
         chunk[3] = (byte)mId.charAt(3);
      }
      
      byte[] size = convertSizeToBytes();
      chunk[4] = size[0];
      chunk[5] = size[1];
      chunk[6] = size[2];
      chunk[7] = size[3];
      
      return chunk;
   }
   
   /**
    * Converts the chunk header from a byte array with at least 8 bytes.
    * 
    * @param b the byte array to convert from.
    * @param offset the offset to start converting from.
    * @param length the number of valid bytes in the buffer following the
    *               offset.
    * @return true if successful, false if not.
    */
   public boolean convertFromBytes(byte[] b, int offset, int length)
   {
      boolean rval = false;
      
      if(b != null && length >= CHUNK_HEADER_SIZE)
      {
         mId = "";
         mId += (char)b[offset];
         mId += (char)b[offset + 1];
         mId += (char)b[offset + 2];
         mId += (char)b[offset + 3];
         
         rval = true;
         
         rval &= convertBytesToSize(b, offset + 4, length);
      }
      
      setValid(rval);
      
      return rval;
   }
   
   /**
    * Sets the 4 byte identifier for this chunk if the passed string is
    * 4 characters long.
    * 
    * @param id the identifier to set.
    * @return true if set, false if not.
    */
   public boolean setIdentifier(String id)
   {
      boolean rval = false;
      
      if(id != null && id.length() == 4)
      {
         mId = id;
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Gets the 4 byte identifier for this chunk.
    * 
    * @return the chunk identifier.
    */
   public String getIdentifier()
   {
      return mId;
   }
   
   /**
    * Sets the size. This is the size of the chunk.
    *
    * @param size the size to use.
    */
   public void setChunkSize(long size)
   {
      mChunkSize = size;
   }
   
   /**
    * Returns the chunk size as reported in the chunk. 
    * 
    * @return the chunk size.
    */
   public long getChunkSize()
   {
      return mChunkSize;
   }
   
   /**
    * Returns whether or not this chunk is valid.
    * 
    * @return true if valid, false if not.
    */
   public boolean isValid()
   {
      return mValid;
   }
   
   /**
    * Sets whether or not this chunk is valid.
    * 
    * @param valid true to set to valid, false to set to invalid.
    */
   public void setValid(boolean valid)
   {
      mValid = valid;
   }
}
