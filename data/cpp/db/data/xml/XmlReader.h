/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_xml_XmlReader_H
#define db_data_xml_XmlReader_H

#include "db/data/DataBinding.h"
#include "db/io/InputStream.h"

#include <expat.h>
#include <list>

namespace db
{
namespace data
{
namespace xml
{

/**
 * An XmlReader provides an interface for deserializing objects from
 * XML (eXtensible Markup Language).
 * 
 * @author Dave Longley
 */
class XmlReader
{
protected:
   /**
    * The xml parser for this reader.
    */
   XML_Parser mParser;
   
   /**
    * True if this xml parser has started, false if not.
    */
   bool mStarted;
   
   /**
    * A stack of DataBindings.
    */
   std::list<DataBinding*> mDataBindingsStack;
   
   /**
    * Handles start elements for this reader.
    * 
    * @param name the name of the element (namespace-uri|element-name).
    * @param attrs the attributes of the element.
    */
   virtual void startElement(const XML_Char* name, const XML_Char** attrs);
   
   /**
    * Handles end elements for this reader.
    * 
    * @param name the name of the element (namespace-uri|element-name). 
    */
   virtual void endElement(const XML_Char* name);
   
   /**
    * Handles character data for this reader.
    * 
    * @param data the read data.
    * @param length the length of the data.
    */
   virtual void appendData(const XML_Char* data, int length);
   
   /**
    * The character encoding.
    */
   static const char* CHAR_ENCODING;
   
   /**
    * The read size in bytes.
    */
   static unsigned int READ_SIZE;
   
   /**
    * Parses a namespace uri out of the given name and sets the passed
    * name pointer to the start of the local name and the passed namespace
    * pointer to the start of a null-terminated namespace string.
    * 
    * @param fullName the namespace|local-name string.
    * @param ns the pointer to point at the namespace string. 
    */
   static void parseNamespace(const char** fullName, char** ns);
   
   /**
    * Handles start elements.
    * 
    * @param xr the XmlReader that read in the element.
    * @param name the name of the element (namespace-uri|element-name).
    * @param attrs the attributes of the element.
    */
   static void startElement(
      void* xr, const XML_Char* name, const XML_Char** attrs);
   
   /**
    * Handles end elements.
    * 
    * @param xr the XmlReader that read in the element.
    * @param name the name of the element (namespace-uri|element-name). 
    */
   static void endElement(void* xr, const XML_Char* name);
   
   /**
    * Handles character data.
    * 
    * @param xr the XmlReader that read in the data.
    * @param data the data that was read.
    * @param length the length of the data.
    */
   static void appendData(void* xr, const XML_Char* data, int length);
   
public:
   /**
    * Creates a new XmlReader.
    */
   XmlReader();
   
   /**
    * Destructs this XmlReader.
    */
   virtual ~XmlReader();
   
   /**
    * Starts deserializing an object from xml. This XmlReader can be re-used
    * by calling start() with the same or a new data binding.
    * 
    * @param db the DataBinding for the object to deserialize.
    */
   virtual void start(db::data::DataBinding* db);
   
   /**
    * This method reads xml from the passed InputStream until the end of
    * the stream, blocking if necessary.
    * 
    * The start() method must be called at least once before calling read(). As
    * the xml is read, the data binding provided in start() is used to
    * deserialize an object.
    * 
    * This method may be called multiple times if the input stream needs to
    * be populated in between calls or if multiple input streams are used.
    * 
    * The finish() method must be called to complete the deserialization.
    * 
    * @param db the DataBinding for the object to deserialize.
    * @param is the InputStream to read the xml from.
    * 
    * @return an IOException if one occurred, NULL if not.
    */
   virtual db::io::IOException* read(db::io::InputStream* is);
   
   /**
    * Finishes deserializing an object from xml. This method must be called
    * to complete deserialization.
    * 
    * @return an IOException is one occurred, NULL if not.
    */
   virtual db::io::IOException* finish();
};

} // end namespace xml
} // end namespace data
} // end namespace db
#endif