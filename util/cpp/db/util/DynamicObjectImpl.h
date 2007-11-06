/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_DynamicObjectImpl_H
#define db_util_DynamicObjectImpl_H

#include <map>
#include <vector>

namespace db
{
namespace util
{

// forward declare DynamicObject, DynamicObjectIteratorImpl
class DynamicObject;
class DynamicObjectIteratorImpl;

/**
 * The possible types for a DynamicObject.
 */
typedef enum DynamicObjectType
{
   String, Boolean, Int32, UInt32, Int64, UInt64, Double, Map, Array
};

/**
 * A DynamicObjectImpl is an implementation for a DynamicObject.
 * 
 * @author Dave Longley
 */
class DynamicObjectImpl
{
public:
   /**
    * A MemberComparator compares two member names.
    */
   typedef struct MemberComparator
   {
      /**
       * Compares two null-terminated strings, returning true if the first is
       * less than the second, false if not. The compare is case-sensitive.
       * 
       * @param s1 the first string.
       * @param s2 the second string.
       * 
       * @return true if the s1 < s2, false if not.
       */
      bool operator()(const char* s1, const char* s2) const
      {
         return strcmp(s1, s2) < 0;
      }
   };
   
   /**
    * The definition for a DynamicObject map and array.
    */
   typedef std::map<const char*, DynamicObject, MemberComparator> ObjectMap;
   typedef std::vector<DynamicObject> ObjectArray;
   
protected:
   /**
    * The type for this object.
    */
   DynamicObjectType mType;
   
   /**
    * The value for this object.
    */
   union
   {
      char* mString;
      bool mBoolean;
      int mInt32;
      unsigned int mUInt32;
      long long mInt64;
      unsigned long long mUInt64;
      double mDouble;
      ObjectMap* mMap;
      ObjectArray* mArray;
   };
   
   /**
    * Make DynamicObjectIteratorImpl a friend so it can iterate over
    * a map or an array.
    */
   friend class DynamicObjectIteratorImpl;
   
   /**
    * Frees the data associated with this DynamicObjectImpl.
    */
   virtual void freeData();
   
   /**
    * A helper function that sets this object's value to the passed string.
    * 
    * @param value the string value for this object.
    */
   virtual void setString(const char* value);
   
public:
   /**
    * Creates a new DynamicObjectImpl.
    */
   DynamicObjectImpl();
   
   /**
    * Destructs this DynamicObjectImpl.
    */
   virtual ~DynamicObjectImpl();
   
   /**
    * Sets this object's value to a string.
    * 
    * @param value the value for this object.
    */
   virtual void operator=(const std::string& value);
   
   /**
    * Sets this object's value to a boolean.
    * 
    * @param value the value for this object.
    */
   virtual void operator=(bool value);
   
   /**
    * Sets this object's value to a 32-bit integer.
    * 
    * @param value the value for this object.
    */
   virtual void operator=(int value);
   
   /**
    * Sets this object's value to a 32-bit unsigned integer.
    * 
    * @param value the value for this object.
    */
   virtual void operator=(unsigned int value);
   
   /**
    * Sets this object's value to a 64-bit integer.
    * 
    * @param value the value for this object.
    */
   virtual void operator=(long long value);
   
   /**
    * Sets this object's value to a 64-bit unsigned integer.
    * 
    * @param value the value for this object.
    */
   virtual void operator=(unsigned long long value);
   
   /**
    * Sets this object's value to a double.
    * 
    * @param value the value for this object.
    */
   virtual void operator=(double value);
   
   /**
    * Gets a DynamicObject based on its member name.
    * 
    * @param name the name of the member.
    * 
    * @return the DynamicObject.
    */
   virtual DynamicObject& operator[](const std::string& name);
   
   /**
    * Gets a DynamicObject based on its index. A negative index will
    * index in reverse, with -1 referring to the last element.
    * 
    * @param index the index of the member.
    * 
    * @return the DynamicObject.
    */
   virtual DynamicObject& operator[](int index);
   
   /**
    * Sets this object's type. Its existing value will be converted if
    * possible.
    * 
    * @param type the type to set this object to.
    */
   virtual void setType(DynamicObjectType type);
   
   /**
    * Gets this object's type.
    * 
    * @return the type of this object.
    */
   virtual DynamicObjectType getType();
   
   /**
    * Gets this object's value as a string.
    * 
    * @return the value of this object.
    */
   virtual const char* getString();
   
   /**
    * Gets this object's value as a boolean.
    * 
    * @return the value of this object.
    */
   virtual bool getBoolean();
   
   /**
    * Gets this object's value as a 32-bit integer.
    * 
    * @return the value of this object.
    */
   virtual int getInt32();
   
   /**
    * Gets this object's value as a 32-bit unsigned integer.
    * 
    * @return the value of this object.
    */
   virtual unsigned int getUInt32();
   
   /**
    * Gets this object's value as a 64-bit integer.
    * 
    * @return the value of this object.
    */
   virtual long long getInt64();
   
   /**
    * Gets this object's value as a 64-bit unsigned integer.
    * 
    * @return the value of this object.
    */
   virtual unsigned long long getUInt64();
   
   /**
    * Gets this object's value as a double.
    * 
    * @return the value of this object.
    */
   virtual double getDouble();
   
   /**
    * Returns true if this object has the specified member, false if not.
    * 
    * @param name the name of the member.
    * 
    * @return true if this object has a member with the specified name,
    *         false if not.
    */
   virtual bool hasMember(const char* name);
   
   /**
    * Gets the length of this object based on its type.
    * 
    * String length = number of characters in the string.
    * Number length = size of number type.
    * Boolean length = size of bool type.
    * Map length = number of entries in the map.
    * Array length = number of elements in the array.
    */
   virtual int length();
};

} // end namespace util
} // end namespace db
#endif