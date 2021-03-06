/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_util_Url_H
#define monarch_util_Url_H

#include "monarch/rt/DynamicObject.h"

#include <string>

namespace monarch
{
namespace util
{

/**
 * A Url represents a Uniform Resource Locator (URL).
 *
 * A URL is written as follows:
 *
 * <scheme>:<scheme-specific-part>
 *
 * A common, but not required, syntax for urls is as follows:
 *
 * url = <scheme>://<authority><path>?<query>
 * authority = <userinfo>@<host>:<port>
 * userinfo = <user>:<password>
 *
 * See also: RFC 3986
 *
 * @author Dave Longley
 */
class Url
{
protected:
   /**
    * True if this url is relative, false if it is absolute.
    */
   bool mRelative;

   /**
    * The scheme for this url.
    */
   std::string mScheme;

   /**
    * The scheme-specific-part for this url.
    */
   std::string mSchemeSpecificPart;

   /**
    * The authority part of this url, if one exists.
    */
   std::string mAuthority;

   /**
    * The userinfo of this url, if one exists.
    */
   std::string mUserInfo;

   /**
    * The user of this url, if one exists.
    */
   std::string mUser;

   /**
    * The password of this url, if one exists.
    */
   std::string mPassword;

   /**
    * The path of this url, if one exists.
    */
   std::string mPath;

   /**
    * The query of this url, if one exists.
    */
   std::string mQuery;

   /**
    * The host for this url, if one exists.
    */
   std::string mHost;

   /**
    * The port for this url, if one exists.
    */
   int mPort;

public:
   /**
    * Creates an empty Url.
    */
   Url();

   /**
    * Creates a new Url from the passed string. An exception may
    * be raised if the url is malformed.
    *
    * @param url the url in string format.
    */
   Url(const char* url);

   /**
    * Creates a new Url from the passed string. An exception may
    * be raised if the url is malformed.
    *
    * @param url the url in string format.
    */
   Url(const std::string& url);

   /**
    * Copies the passed url.
    *
    * @param copy the url to copy.
    */
   Url(const Url& copy);

   /**
    * Destructs this Url.
    */
   virtual ~Url();

   /**
    * Sets this Url equal to another one.
    *
    * @param rhs the Url to set this one equal to.
    */
   virtual Url& operator=(const Url& rhs);

   /**
    * Sets this Url to the passed string.
    *
    * @param url the string to create this Url from.
    *
    * @return false if the URL is malformed, true if not.
    */
   virtual bool setUrl(const std::string& url);

   /**
    * Sets this Url to the passed string.
    *
    * @param url the string to create this Url from.
    *
    * @return false if the URL is malformed, true if not.
    */
   virtual bool setUrl(const char* rurl);

   /**
    * Sets a relative reference Url. See (RFC 3986 sec 4.2).
    *
    * @param url the string to create this Url from.
    *
    * @return false if the URL is malformed, true if not.
    */
   virtual bool setRelativeUrl(const std::string& rurl);

   /**
    * Sets a relative reference Url. See (RFC 3986 sec 4.2).
    *
    * @param url the string to create this Url from.
    *
    * @return false if the URL is malformed, true if not.
    */
   virtual bool setRelativeUrl(const char* url);

   /**
    * Sets this Url to the passed formatted string.
    *
    * If the format string contains url-encoded characters, then it must have
    * its '%' characters escaped (to "%%").
    *
    * @param format the format for the url.
    * @param ... the format parameters.
    *
    * @return false if the URL is malformed, true if not.
    */
   virtual bool format(const char* format, ...)
#ifdef __GNUC__
      __attribute__ ((format (printf, 2, 3)))
#endif
         ;

   /**
    * Returns true if this url is relative, false if it is absolute.
    *
    * @return true if this url is relative, false if it is absolute.
    */
   virtual bool isRelative() const;

   /**
    * Sets the scheme for this url.
    *
    * @param scheme the new scheme to use.
    */
   virtual void setScheme(const char* scheme);

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
    * Gets the authority of this url, if one exists.
    *
    * @return the authority of this url or a blank string.
    */
   virtual const std::string& getAuthority();

   /**
    * Gets the userinfo this url, if it exists.
    *
    * @return the userinfo of this url or a blank string.
    */
   virtual const std::string& getUserInfo();

   /**
    * Gets the user of this url, if one exists.
    *
    * @return the user of this url or a blank string.
    */
   virtual const std::string& getUser();

   /**
    * Gets the password of this url, if one exists.
    *
    * @return the password of this url or a blank string.
    */
   virtual const std::string& getPassword();

   /**
    * Gets the path of this url, if one exists.
    *
    * @return the path of this url or a blank string.
    */
   virtual const std::string& getPath();

   /**
    * Gets the path of this url in tokenized form, starting after the
    * optionally passed base path.
    *
    * @param result the DynamicObject to store the tokens in.
    * @param basePath the base path to start tokenizing after.
    *
    * @return true if the tokenization worked, false if the base path did
    *         not exist on the path.
    */
   virtual bool getTokenizedPath(
      monarch::rt::DynamicObject& result, const char* basePath = "");

   /**
    * Gets the query of this url, if one exists.
    *
    * @return the query of this url or a blank string.
    */
   virtual const std::string& getQuery();

   /**
    * Adds a variable to this url. The key and value will be URL-encoded.
    *
    * @param key the key of the variable.
    * @param value the value of the variable.
    */
   virtual void addQueryVariable(const char* key, const char* value);

   /**
    * Adds variables to this url. The variables in the passed map will be
    * URL-encoded. Map members which are arrays will result in multiple query
    * key-value pairs for the same key.
    *
    * @param vars the DynamicObject Map with key-value pairs to add to the
    *             query.
    */
   virtual void addQueryVariables(monarch::rt::DynamicObject& vars);

   /**
    * Gets the variables from the query of this url.
    *
    * @param vars the DynamicObject Map to populate with the variables from
    *             the query.  vars will be converted to a Map if it is some
    *             other type.
    * @param asArrays true to create an array to hold all values for each key,
    *           false to use only the last value for each key.
    *
    * @return true if variables were present, false if not.
    */
   virtual bool getQueryVariables(
      monarch::rt::DynamicObject& vars, bool asArrays = false);

   /**
    * Combines the path and query of this url, if one exists.
    *
    * @return the full path of this url or a blank string.
    */
   virtual std::string getPathAndQuery();

   /**
    * Convenience method to get the host of this url (not including
    * the port).
    *
    * @return the host of this url.
    */
   virtual const std::string& getHost();

   /**
    * Convenience method to get the port of this url.
    *
    * @return the port of this url.
    */
   virtual int getPort();

   /**
    * Convenience method to get the host and port of this url.
    *
    * @return a "host:port" string.
    */
   virtual std::string getHostAndPort();

   /**
    * Convenience method to get the scheme, host, and port of this url.
    *
    * @return a "scheme://host:port" string.
    */
   virtual std::string getSchemeHostAndPort();

   /**
    * Gets the default port for the scheme (protocol) of this url.
    *
    * @return the default port for the scheme (protocol) of this url.
    */
   virtual int getDefaultPort();

   /**
    * Normalize this URL according to RFC 3986.
    *
    * http://tools.ietf.org/html/rfc3986
    */
   virtual void normalize();

   /**
    * Writes this url to a string.
    *
    * @return the string representation for this url.
    */
   virtual std::string toString() const;

   /**
    * Gets the parent path of the given path.
    *
    * @param path the path to get the parent path of.
    *
    * @return the parent path.
    */
   static std::string getParentPath(const char* path);

   /**
    * Parses a 'host:port' string into a hostname string and port string.
    *
    * @param input the 'host:port' string.
    * @param host the string to store the hostname in.
    * @param port the string to store the port in.
    */
   static void parseHostAndPort(
      const char* input, std::string& host, std::string& port);

   /**
    * URL-encodes the passed string.
    *
    * @param str the string to URL-encode.
    * @param length the length of the string.
    * @param spaceToPlus true to replace ' ' with '+', false to use '%20'.
    *
    * @return the URL-encoded string.
    */
   static std::string encode(
      const char* str, unsigned int length, bool spaceToPlus);

   /**
    * URL-encodes the passed string.
    *
    * @param str the string to URL-encode.
    * @param spaceToPlus true to replace ' ' with '+', false to use '%20'.
    *
    * @return the URL-encoded string.
    */
   static std::string encode(const char* str, bool spaceToPlus = true);

   /**
    * URL-decodes the passed string.
    *
    * @param str the string to URL-decode.
    * @param length the length of the string.
    *
    * @return the URL-decoded string.
    */
   static std::string decode(const char* str, unsigned int length);

   /**
    * URL-decodes the passed string.
    *
    * @param str the string to URL-decode.
    *
    * @return the URL-decoded string.
    */
   static std::string decode(const char* str);

   /**
    * URL-form-encodes the passed form to a string.
    *
    * Useful for "Content-Type: application/x-www-form-urlencoded"
    *
    * @param form a DynamicObject map with key-value pairs to encode.
    *
    * @return the URL-form-encoded string.
    */
   static std::string formEncode(monarch::rt::DynamicObject& form);

   /**
    * URL-form-decodes the passed string into a form.
    *
    * Useful for "Content-Type: application/x-www-form-urlencoded"
    *
    * @param form the DynamicObject map to populate with the decoded key-values.
    * @param str the NULL terminated string to URL-form-decode.
    * @param asArrays true to create an array to hold all values for each key,
    *           false to use only the last value for each key.
    *
    * @return true if form data was found, false if not.
    */
   static bool formDecode(
      monarch::rt::DynamicObject& form, const char* str, bool asArrays = false);

   /**
    * Convert a DynamicObject to the same structure used by formDecode with
    * asArrays set to true. This will adjust all non-array members to be
    * arrays in-place. Members that are already arrays are left as-is.
    *
    * @param form the DynamicObject map to convert.
    *
    * @return true if conversion was successful, false if not.
    */
   static bool formConvertToArrays(monarch::rt::DynamicObject& form);

protected:
   /**
    * Sets this Url to the passed formatted string.
    *
    * @param format the format for the url.
    * @param varargs the format parameters.
    *
    * @return false if the URL is malformed, true if not.
    */
   virtual bool setUrl(const char* format, va_list varargs);

   /**
    * Parse scheme specific part.
    *
    * @return false if the URL is malformed, true if not.
    */
   virtual bool parseSchemeSpecificPart();
};

// type definition for a reference collected Url
typedef monarch::rt::Collectable<Url> UrlRef;

} // end namespace util
} // end namespace monarch
#endif
