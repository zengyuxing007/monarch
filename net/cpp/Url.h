/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Url_H
#define Url_H

#include "Object.h"
#include "MalformedUrlException.h"
#include <string>

namespace db
{
namespace net
{

/**
 * A Url represents a Uniform Resource Locator (URL).
 * 
 * A URL is written as follows:
 * 
 * <scheme>:<scheme-specific-part>
 * 
 * @author Dave Longley
 */
class Url : public virtual db::rt::Object
{
protected:
   /**
    * The scheme for this url.
    */
   std::string mScheme;
   
   /**
    * The scheme-specific-part for this url.
    */
   std::string mSchemeSpecificPart;
   
public:
   /**
    * Creates a new Url from the passed string.
    * 
    * @param url the string to create this Url from.
    * 
    * @exception MalformedUrlException thrown if the URL is malformed.
    */
   Url(const std::string& url) throw(MalformedUrlException);
   
   /**
    * Destructs this Url.
    */
   virtual ~Url();
   
   /**
    * Gets the scheme for this url.
    * 
    * @return the scheme for this url.
    */
   virtual const std::string& getScheme();
   
   /**
    * Gets the scheme-specific-part for this url.
    *
    * @return the scheme-specific-part for this url.
    */
   virtual const std::string& getSchemeSpecificPart();
   
   /**
    * URL-encodes the passed string.
    * 
    * @param str the string to URL-encode.
    * @param length the length of the string.
    * 
    * @return the URL-encoded string.
    */
   static std::string encode(const char* str, unsigned int length);
   
   /**
    * URL-decodes the passed string.
    * 
    * @param str the string to URL-decode.
    * @param length the length of the string.
    * 
    * @return the URL-decoded string.
    */
   static std::string decode(const char* str, unsigned int length);
};

} // end namespace net
} // end namespace db
#endif
