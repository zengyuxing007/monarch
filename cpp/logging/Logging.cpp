/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/logging/Logging.h"

using namespace db::logging;

bool Logging::initialize()
{
   Category::initialize();
   LoggingCategories::initialize();
   Logger::initialize();

   return true;
}

void Logging::cleanup()
{
   Logger::cleanup();
   LoggingCategories::cleanup();
   Category::cleanup();
}
