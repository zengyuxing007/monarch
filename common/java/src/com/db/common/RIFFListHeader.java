/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common;

/**
 * A RIFF List Header.
 * 
 * The format of a RIFF List is such:
 * 
 * | bytes 1-4  | bytes 5-8 | bytes 9-12 | bytes 13+ |
 * ---------------------------------------------------
 * |   "LIST"   | list size |   "XXXX"   | list data |
 * 
 * @author Dave Longley
 */
public class RIFFListHeader
{
   /**
    * A RIFF Chunk Header.
    */
   protected RIFFChunkHeader mChunkHeader;
   
   /**
    * The 4 byte list identifier.
    */
   protected String mId;
   
   /**
    * Constants.
    */
   public static final int LIST_HEADER_SIZE = 12;
   
   /**
    * Constructs a RIFF List Header with default values.
    */
   public RIFFListHeader()
   {
      this(null, LIST_HEADER_SIZE);
   }
   
   /**
    * Constructs a RIFF List Header with the passed parameters.
    * 
    * @param id the identifier of the list.
    */
   public RIFFListHeader(String id)
   {
      this(id, LIST_HEADER_SIZE);
   }   
   
   /**
    * Constructs a RIFF List Header with the passed parameters.
    * 
    * @param id the identifier of the list.
    * @param size the size of the list.
    */
   public RIFFListHeader(String id, int size)
   {
      mChunkHeader = new RIFFChunkHeader("LIST", size);
      
      if(id == null || id.length() != 4)
      {
         mId = "";
      }
      else
      {
         mId = id;
      }
   }
   
   /**
    * Converts the header into a 12 byte array.
    * 
    * @return the header as a 12 byte array. 
    */
   public byte[] convertToBytes()
   {
      byte[] chunk = mChunkHeader.convertToBytes();
      
      byte[] list = new byte[LIST_HEADER_SIZE];
      System.arraycopy(chunk, 0, list, 0, chunk.length);
      
      if(mId.length() == 4)
      {
         list[8]  = (byte)mId.charAt(0);
         list[9]  = (byte)mId.charAt(1);
         list[10] = (byte)mId.charAt(2);
         list[11] = (byte)mId.charAt(3);
      }

      return list;
   }
   
   /**
    * Converts the header from a byte array with at least 12 bytes.
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
      
      if(b != null && length >= LIST_HEADER_SIZE)
      {
         if(mChunkHeader.convertFromBytes(b, offset, length))
         {
            // make sure chunk identifier is LIST
            if(mChunkHeader.getIdentifier().equals("LIST"))
            {
               mId = "";
               mId += (char)b[offset + 8];
               mId += (char)b[offset + 9];
               mId += (char)b[offset + 10];
               mId += (char)b[offset + 11];
               
               rval = true;
            }
         }
      }
      
      setValid(rval);
      
      return rval;      
   }
   
   /**
    * Sets the 4 byte identifier for this list if the passed string is
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
    * Gets the 4 byte identifier for this list.
    * 
    * @return the list identifier.
    */
   public String getIdentifier()
   {
      return mId;
   }
   
   /**
    * Sets the list size.
    *
    * @param size the size of the RIFF list. 
    */
   public void setListSize(long size)
   {
      mChunkHeader.setChunkSize(size);
   }
   
   /**
    * Gets the list size.
    * 
    * @return the size of the RIFF list.
    */
   public long getListSize()
   {
      return mChunkHeader.getChunkSize();
   }
   
   /**
    * Returns whether or not this list is valid.
    * 
    * @return true if valid, false if not.
    */
   public boolean isValid()
   {
      return mChunkHeader.isValid();
   }
   
   /**
    * Sets whether or not this list is valid.
    * 
    * @param valid true to set to valid, false to set to invalid.
    */
   public void setValid(boolean valid)
   {
      mChunkHeader.setValid(valid);
   }
}
