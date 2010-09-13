/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_v8_V8Engine_h
#define monarch_v8_V8Engine_h

#include "monarch/v8/V8EngineApi.h"

#include <v8.h>

namespace monarch
{
namespace v8
{

/**
 * A V8Engine can run JavaScript using the V8 engine.
 * 
 * @author David I. Lehn
 */
class V8Engine : public V8EngineApi
{
protected:
   /**
    * Persistent context for this engine.
    */
   ::v8::Persistent< ::v8::Context> mContext;

public:
   /**
    * Creates a new V8Engine.
    */
   V8Engine();
   
   /**
    * Destructs this V8Engine.
    */
   virtual ~V8Engine();

   /**
    * {@inheritDoc}
    */
   virtual bool initialize();

   /**
    * {@inheritDoc}
    */
   virtual bool setDynamicObject(
      const char* name, monarch::rt::DynamicObject& dyno);

   /**
    * {@inheritDoc}
    */
   virtual bool getDynamicObject(
      const char* name, monarch::rt::DynamicObject& dyno);

   /**
    * {@inheritDoc}
    */
   virtual bool runScript(const char* js, std::string& result);
};

} // end namespace v8
} // end namespace monarch
#endif
