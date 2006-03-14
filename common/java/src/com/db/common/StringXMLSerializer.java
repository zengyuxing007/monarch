/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common;

import org.w3c.dom.Element;

/**
 * A StringXMLSerializer is a very simplistic XML serializer. Its purpose
 * is to serialize a string to XML for use with other XML objects.
 * 
 * @author Dave Longley
 */
public class StringXMLSerializer implements IXMLSerializer
{
   /**
    * The string.
    */
   protected String mString;
   
   /**
    * Creates a new string xml serializer.
    */
   public StringXMLSerializer()
   {
      setString("");
   }

   /**
    * Creates a new string xml serializer.
    * 
    * @param string the string to set.
    */
   public StringXMLSerializer(String string)
   {
      setString(string);
   }
   
   /**
    * Sets the string.
    * 
    * @param string the string to set.
    */
   public void setString(String string)
   {
      mString = string;
   }
   
   /**
    * Gets the string.
    * 
    * @return the string.
    */
   public String getString()
   {
      return mString;
   }
   
   /**
    * This method takes options that are used to configure
    * how to convert to and from xml.
    *
    * @param options the configuration options.
    * @return true if options successfully set, false if not.    
    */
   public boolean setSerializerOptions(int options)
   {
      return false;
   }

   /**
    * This method gets the options that are used to configure
    * how to convert to and from xml.
    *
    * @return the configuration options.
    */
   public int getSerializerOptions()
   {
      return 0;
   }

   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "string";
   }   

   /**
    * This method takes the object representation and creates an
    * XML-based representation of the object.
    * 
    * @return the xml-based representation of this object.
    */
   public String convertToXML()
   {
      return convertToXML(0);
   }
   
   /**
    * This method takes XML text (in full document form) and converts
    * it to it's internal representation.
    *
    * @param indentLevel the number of spaces to place before the text
    *                    after each new line.
    * @return true if successful, false otherwise.    
    */
   public String convertToXML(int indentLevel)
   {
      StringBuffer xml = new StringBuffer();
      StringBuffer indent = new StringBuffer("\n");
      for(int i = 0; i < indentLevel; i++)
      {
         indent.append(' ');
      }

      xml.append('<');
      xml.append(getRootTag());
      xml.append('>');
      xml.append(XMLCoder.encode(getString()));
      xml.append("</");
      xml.append(getRootTag());
      xml.append('>');

      return xml.toString();
   }
   
   /**
    * This method takes XML text (in full document form) and converts
    * it to it's internal representation.
    *
    * @param xmlText the xml text document that represents the object.
    * @return true if successful, false otherwise.    
    */
   public boolean convertFromXML(String xmlText)
   {
      return false;
   }
   
   /**
    * This method takes a parsed DOM XML element and converts it
    * back into this object's representation.
    *
    * @param element the parsed element that contains this objects information.
    * @return true if successful, false otherwise.
    */
   public boolean convertFromXML(Element element)
   {
      boolean rval = true;
      
      ElementReader er = new ElementReader(element);
      setString(XMLCoder.decode(er.getStringValue()));
      
      return rval;
   }
}
