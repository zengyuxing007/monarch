/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/HttpHeader.h"

#include "db/util/StringTools.h"

#include <cstdlib>
#include <cstring>

using namespace std;
using namespace db::io;
using namespace db::net::http;
using namespace db::util;

// define CRLF
const char* HttpHeader::CRLF = "\r\n";

HttpHeader::HttpHeader()
{
   mVersion = NULL;
   mFieldsSize = 0;
}

HttpHeader::~HttpHeader()
{
   if(mVersion != NULL)
   {
      free(mVersion);
   }
   
   for(FieldMap::iterator i = mFields.begin(); i != mFields.end(); i++)
   {
      free((char*)i->first);
   }
}

bool HttpHeader::parseStartLine(const char* str, unsigned int length)
{
   // no start line
   return true;
}

void HttpHeader::getStartLine(string& line)
{
   // no start line
   line.erase();
}

bool HttpHeader::hasStartLine()
{
   // no start line
   return false;
}

void HttpHeader::setVersion(const char* version)
{
   if(mVersion != NULL)
   {
      free(mVersion);
   }
   mVersion = strdup(version);
}

const char* HttpHeader::getVersion()
{
   if(mVersion == NULL)
   {
      // default to HTTP/1.1
      mVersion = strdup("HTTP/1.1");
   }
   
   return mVersion;
}

void HttpHeader::setField(const char* name, long long value)
{
   char temp[25];
   sprintf(temp, "%lli", value);
   setField(name, temp);
}

void HttpHeader::setField(const char* name, const string& value)
{
   // update old field if possible
   FieldMap::iterator i = mFields.find(name);
   if(i != mFields.end())
   {
      mFieldsSize -= i->second.length();
      i->second = value;
      mFieldsSize += value.length();
   }
   else
   {
      // set new field
      mFieldsSize += strlen(name);
      mFieldsSize += value.length();
      char* fieldName = strdup(name);
      biCapitalize(fieldName);
      mFields.insert(make_pair(fieldName, value));
   }
}

void HttpHeader::addField(const char* name, const string& value)
{
   // get existing value
   string existing;
   if(getField(name, existing))
   {
      // append to existing value
      existing.append(", ");
   }
   
   existing.append(value);
   setField(name, existing);
}

void HttpHeader::removeField(const char* name)
{
   // erase field
   FieldMap::iterator i = mFields.find(name);
   if(i != mFields.end())
   {
      mFieldsSize -= strlen(name);
      mFieldsSize -= i->second.length();
      mFields.erase(i);
   }
}

void HttpHeader::clearFields()
{
   for(FieldMap::iterator i = mFields.begin(); i != mFields.end(); i++)
   {
      free((char*)i->first);
   }
   
   mFieldsSize = 0;
   mFields.clear();
}

bool HttpHeader::getField(const char* name, long long& value)
{
   bool rval = false;
   
   string str;
   if(getField(name, str))
   {
      char* endptr = NULL;
      value = strtoll(str.c_str(), &endptr, 10);
      rval = endptr != str.c_str() && *endptr == '\0';      
   }
   
   return rval;
}

bool HttpHeader::getField(const char* name, string& value)
{
   bool rval = false;
   
   // find field entry
   FieldMap::iterator i = mFields.find(name);
   if(i != mFields.end())
   {
      // get value
      value = i->second;
      rval = true;
   }
   
   return rval;
}

bool HttpHeader::hasField(const char* name)
{
   bool rval = false;
   
   // find field entry
   FieldMap::iterator i = mFields.find(name);
   if(i != mFields.end())
   {
      rval = true;
   }
   
   return rval;
}

bool HttpHeader::parse(const string& str)
{
   bool rval = false;
   
   // clear fields
   clearFields();
   
   bool startLine = hasStartLine();
   const char* start = str.c_str();
   char* cr;
   char* colon;
   while((cr = strchr(start, '\r')) != NULL)
   {
      // look for CRLF
      if(strchr(cr, '\n') == cr + 1)
      {
         if(startLine)
         {
            rval = parseStartLine(start, cr - start);
            startLine = false;
         }
         else
         {
            // found a CRLF, now find colon
            if((colon = strchr(start, ':')) != NULL && colon < cr)
            {
               // get field name
               char name[(colon - start) + 1];
               memcpy(name, start, colon - start);
               name[colon - start] = 0;
               
               // skip whitespace
               colon++;
               for(; *colon == ' ' && colon < cr; colon++);
               
               // get field value
               char value[cr - colon + 1];
               strncpy(value, colon, cr - colon);
               value[(cr - colon)] = 0;
               
               // set field
               setField(name, value);
            }
         }
         
         start = cr + 2;
      }
      else
      {
         start = cr + 1;
      }
   }
   
   return rval;
}

string HttpHeader::toString()
{
   string str;
   str.reserve(512);
   
   // get the start line
   getStartLine(str);
   
   // determine total fields size:
   // (CRLF + fields size + fields * (": " + CRLF))
   char fields[2 + mFieldsSize + mFields.size() * 4];
   char* s = fields;
   
   // append CRLF if there is a start line
   if(str.length() > 0)
   {
      s[0] = '\r';
      s[1] = '\n';
      s += 2;
   }
   
   // append all fields
   int length;
   for(FieldMap::iterator i = mFields.begin(); i != mFields.end(); i++)
   {
      // get field name length
      length = strlen(i->first);
      
      // append name
      memcpy(s, i->first, length);
      s += length;
      
      // append delimiter
      s[0] = ':';
      s[1] = ' ';
      s += 2;
      
      // append value
      memcpy(s, i->second.c_str(), i->second.length());
      s += i->second.length();
      
      // append CRLF
      s[0] = '\r';
      s[1] = '\n';
      s += 2;
   }
   
   // add CRLF
   s[0] = '\r';
   s[1] = '\n';
   s += 2;
   
   // append fields
   str.append(fields, s - fields);
   
   return str;
}

bool HttpHeader::write(OutputStream* os)
{
   bool rval = true;
   
   // get the start line
   string line;
   getStartLine(line);
   
   // write the start line and CRLF if one exists
   if(line.length() > 0)
   {
      rval =
         os->write(line.c_str(), line.length()) &&
         os->write(CRLF, 2);
   }
   
   // write all fields
   for(FieldMap::iterator i = mFields.begin(); rval && i != mFields.end(); i++)
   {
      // write name, delimiter, value, and CRLF
      rval =
         os->write(i->first, strlen(i->first)) &&
         os->write(": ", 2) &&
         os->write(i->second.c_str(), i->second.length()) &&
         os->write(CRLF, 2);
   }
   
   // write ending CRLF
   rval = rval && os->write(CRLF, 2);
   
   return rval;
}

void HttpHeader::setDate(Date* date)
{
   // get GMT time zone
   TimeZone gmt = TimeZone::getTimeZone("GMT");
   string str;
   const char* format = "%a, %d %b %Y %H:%M:%S GMT";
   
   if(date == NULL)
   {
      // get current date
      Date now;
      now.format(str, format, &gmt);
   }
   else
   {
      date->format(str, format, &gmt);
   }
   
   // set date field
   setField("Date", str);
}

bool HttpHeader::getDate(Date& date)
{
   bool rval = false;
   
   string str;
   if(getField("Date", str))
   {
      // get GMT time zone
      TimeZone gmt = TimeZone::getTimeZone("GMT");
      rval = date.parse(str.c_str(), "%a, %d %b %Y %H:%M:%S", &gmt);
   }
   
   return rval;
}

void HttpHeader::biCapitalize(char* name)
{
   // capitalize first letter and letters after hyphens
   // decapitalize other letters
   // NOTE: hardcoded version is faster than using toupper/tolower
   if(name != NULL && *name != '\0')
   {
      char* ptr = name;
      // cap first
      if(*ptr >= 'a' && *ptr <= 'z')
      {
         *ptr -= 'a' - 'A';
      }
      for(ptr++; *ptr != '\0'; ptr++)
      {
         // cap after '-'
         if(*(ptr - 1) == '-')
         {
            if(*ptr >= 'a' && *ptr <= 'z')
            {
               *ptr -= 'a' - 'A';
            }
         }
         // decap rest
         else if(*ptr >= 'A' && *ptr <= 'Z')
         {
            *ptr += 'a' - 'A';
         }
      }
   }
}
