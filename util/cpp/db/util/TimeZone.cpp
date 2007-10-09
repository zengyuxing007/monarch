/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/TimeZone.h"
#include "db/rt/System.h"

using namespace std;
using namespace db::rt;
using namespace db::util;

TimeZone::TimeZone(unsigned int minutesWest)
{
   mMinutesWest = minutesWest;
}

TimeZone::~TimeZone()
{
}

unsigned int TimeZone::getMinutesWest()
{
   return mMinutesWest;
}

TimeZone TimeZone::getTimeZone(const string& tz)
{
   TimeZone rval;
   
   if(strcmp(tz.c_str(), "GMT") == 0)
   {
      rval.mMinutesWest = 0;
   }
   else if(strcmp(tz.c_str(), "EDT") == 0)
   {
      // 4 hours west
      rval.mMinutesWest = 240;
   }
   else if(strcmp(tz.c_str(), "EST") == 0)
   {
      // 5 hours west
      rval.mMinutesWest = 300;
   }
   else if(strcmp(tz.c_str(), "PDT") == 0)
   {
      // 7 hours west
      rval.mMinutesWest = 420;
   }
   else if(strcmp(tz.c_str(), "PST") == 0)
   {
      // 8 hours west
      rval.mMinutesWest = 480;
   }
   else
   {
      rval.mMinutesWest = gGetTimeZoneMinutesWest();
   }
   
   return rval;
}