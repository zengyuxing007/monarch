/*
 * Copyright (c) 2009-2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_LIMIT_MACROS

#include "monarch/net/BandwidthThrottlerChain.h"

#include "monarch/util/Math.h"

using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::util;

BandwidthThrottlerChain::BandwidthThrottlerChain()
{
}

BandwidthThrottlerChain::~BandwidthThrottlerChain()
{
}

void BandwidthThrottlerChain::add(BandwidthThrottler* bt)
{
   mChain.push_back(bt);
}

void BandwidthThrottlerChain::add(BandwidthThrottlerRef& bt)
{
   // save reference
   mThrottlerRefList.push_back(bt);
   add(&(*bt));
}

bool BandwidthThrottlerChain::requestBytes(int count, int& permitted)
{
   bool rval = true;

   // initialize permitted to count
   permitted = count;

   if(!mChain.empty())
   {
      // request bytes from each throttler in the chain, limit max permitted
      // to minimum permitted by a throttler in the chain
      int maxPermitted = -1;
      for(ThrottlerChain::iterator i = mChain.begin();
          rval && i != mChain.end(); ++i)
      {
         rval = (*i)->requestBytes(count, permitted);
         if(rval && (maxPermitted == -1 || permitted < maxPermitted))
         {
            maxPermitted = permitted;
         }
      }

      // use maxPermitted if maxPermitted was set by a throttler in the chain
      if(maxPermitted != -1)
      {
         permitted = maxPermitted;
      }
   }

   return rval;
}

void BandwidthThrottlerChain::addAvailableBytes(int bytes)
{
   if(!mChain.empty())
   {
      // add bytes back to each throttler in the chain, starting in reverse
      for(ThrottlerChain::reverse_iterator i = mChain.rbegin();
          i != mChain.rend(); ++i)
      {
         (*i)->addAvailableBytes(bytes);
      }
   }
}

int BandwidthThrottlerChain::getAvailableBytes()
{
   int rval = INT32_MAX;

   if(!mChain.empty())
   {
      rval = mChain.back()->getAvailableBytes();
   }

   return rval;
}

void BandwidthThrottlerChain::setRateLimit(int rateLimit)
{
   if(!mChain.empty())
   {
      mChain.back()->setRateLimit(rateLimit);
   }
}

int BandwidthThrottlerChain::getRateLimit()
{
   return (mChain.empty() ? 0 : mChain.back()->getRateLimit());
}
