/*
 * Copyright (c) 2008-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/ValidatorContext.h"

#include "monarch/rt/Exception.h"

#include <cstdlib>

using namespace monarch::rt;
using namespace monarch::validation;

ValidatorContext::ValidatorContext(DynamicObject* state) :
   mState(state),
   mPath(NULL),
   mSetExceptions(true),
   mMaskType(ValidatorContext::MaskNone),
   mResults(NULL)
{
   clearResults();
}

ValidatorContext::~ValidatorContext()
{
   if(mPath != NULL)
   {
      for(std::vector<char*>::iterator i = mPath->begin();
          i != mPath->end(); ++i)
      {
         free(*i);
      }

      delete mPath;
   }
}

monarch::rt::DynamicObject& ValidatorContext::getState()
{
   return *mState;
}

void ValidatorContext::setMaskType(MaskType mt)
{
   mMaskType = mt;
}

ValidatorContext::MaskType ValidatorContext::getMaskType()
{
   return mMaskType;
}

bool ValidatorContext::setExceptions(bool set)
{
   bool rval = mSetExceptions;
   mSetExceptions = set;
   return rval;
}

void ValidatorContext::pushPath(const char* path)
{
   if(mPath == NULL)
   {
      mPath = new std::vector<char*>;
   }
   mPath->push_back(strdup(path));
}

void ValidatorContext::popPath()
{
   if(mPath != NULL)
   {
      char* path = mPath->back();
      mPath->pop_back();
      free(path);
   }
}

unsigned int ValidatorContext::getDepth()
{
   unsigned int rval = 0;
   if(mPath != NULL)
   {
      rval = mPath->size();
   }
   return rval;
}

std::string ValidatorContext::getPath()
{
   std::string rval;

   if(mPath != NULL)
   {
      for(std::vector<char*>::iterator i = mPath->begin();
         i != mPath->end();
         ++i)
      {
         rval.append(*i);
      }
   }

   return rval;
}

void ValidatorContext::addSuccess()
{
   mResults["successes"] = mResults["successes"]->getUInt32() + 1;
}

DynamicObject ValidatorContext::addError(
   const char* type, DynamicObject* object)
{
   DynamicObject errorDetail;

   // setup error detail
   errorDetail["type"] = type;
   // FIXME: localize message -- lehn
   // FIXME: really? do we need to mention this, because we'd have to
   //        do this for every string in the system.. -- manu
   errorDetail["message"] = "The given value does not meet all of the data "
      "validation requirements. Please examine the error details for more "
      "information about the specific requirements.";
   if(object != NULL && (mMaskType & ValidatorContext::MaskInvalidValues) == 0)
   {
      errorDetail["invalidValue"] = *object;
   }

   // add error detail to results errors
   std::string fullpath = getPath();
   mResults["errors"][fullpath.c_str()] = errorDetail;

   // Skip setting exceptions if requested. Return errorDetail regardless.
   if(mSetExceptions)
   {
      ExceptionRef e;

      if(!Exception::isSet())
      {
         e = new Exception(
            "The given object does not meet all of the data validation "
            "requirements. Please examine the error details for more "
            "information about the specific requirements.",
            "monarch.validation.ValidationError");
         Exception::set(e);
      }
      else
      {
         e = Exception::get();
         // Check if we are adding to a ValidationError
         if(!e->isType("monarch.validation.ValidationError"))
         {
            // FIXME: this is a bit bogus. If validation checking keeps causing
            // other exceptions then a long cause chain could be generated
            // switching between ValidationError and other types.
            e = new Exception(
               "The given object does not meet all of the data validation "
               "requirements. Please examine the error details for more "
               "information about the specific requirements.",
               "monarch.validation.ValidationError");
            Exception::push(e);
         }
      }

      // add detail to "errors" section of exception details
      e->getDetails()["errors"][fullpath.c_str()] = errorDetail;
   }

   return errorDetail;
}

DynamicObject ValidatorContext::getResults()
{
   return mResults;
}

void ValidatorContext::clearResults()
{
   mResults = DynamicObject();
   mResults["successes"] = 0;
   mResults["errors"]->setType(Map);
   mResults["errors"]->clear();
}
