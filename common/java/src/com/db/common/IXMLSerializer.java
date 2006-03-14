/*
 * Copyright (c) 2003 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common;

import org.w3c.dom.Element;

/**
 * An interface that allows an object to be serialized to
 * and from XML.
 *  
 * @author dlongley
 */
public interface IXMLSerializer
{
   /**
    * This method takes options that are used to configure
    * how to convert to and from xml.
    *
    * @param options the configuration options.
    * @return true if options successfully set, false if not.
    */
   public boolean setSerializerOptions(int options);

   /**
    * This method gets the options that are used to configure
    * how to convert to and from xml.
    *
    * @return the configuration options.
    */
   public int getSerializerOptions();
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag();
   
   /**
    * This method takes the object representation and creates an
    * XML-based representation of the object.
    *
    * @return the XML-based representation of the object.
    */
   public String convertToXML();

   /**
    * This method takes the object representation and creates an
    * XML-based representation of the object.
    *
    * @param indentLevel the number of spaces to place before the text
    *                    after each new line.
    * @return the XML-based representation of the object.
    */
   public String convertToXML(int indentLevel);

   /**
    * This method takes XML text (in full document form) and converts
    * it to it's internal representation.
    *
    * @param xmlText the xml text document that represents the object.
    * @return true if successful, false otherwise.    
    */
   public boolean convertFromXML(String xmlText);
   
   /**
    * This method takes a parsed DOM XML element and converts it
    * back into this object's representation.
    *
    * @param element the parsed element that contains this objects information.
    * @return true if successful, false otherwise.
    */
   public boolean convertFromXML(Element element);
}
