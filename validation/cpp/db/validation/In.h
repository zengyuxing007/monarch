/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_validation_In_H
#define db_validation_In_H

#include "db/validation/Validator.h"

namespace db
{
namespace validation
{

/**
 * Validates an object is in an array or is a key in a map.
 * 
 * @author David I. Lehn
 */
class In : public Validator
{
protected:
   /* Object with valid contents */
   db::rt::DynamicObject mContents;

public:
   /**
    * Creates a new validator.
    * 
    * @param errorMessage custom error message
    */
   In(db::rt::DynamicObject& contents, const char* errorMessage = NULL);
   
   /**
    * Destructs this validator.
    */
   virtual ~In();
   
   /**
    * Checks if an object is valid.
    * 
    * @param obj the object to validate.
    * @param context context to use during validation.
    * 
    * @return true if obj is valid, false and exception set otherwise.
    */
   virtual bool isValid(
      db::rt::DynamicObject& obj,
      ValidatorContext* context);
   using Validator::isValid;
};

} // end namespace validation
} // end namespace db
#endif