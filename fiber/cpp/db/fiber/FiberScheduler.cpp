/*
 * Copyright (c) 2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/fiber/FiberScheduler.h"

#include <algorithm>
#include <setjmp.h>

using namespace std;
using namespace db::fiber;
using namespace db::modest;
using namespace db::rt;

FiberScheduler::FiberScheduler()
{
   // add first FiberId
   mFiberIdFreeList.push_back(1);
}

FiberScheduler::~FiberScheduler()
{
   // ensure stopped
   FiberScheduler::stop();
   
   // delete all fibers
   for(FiberMap::iterator i = mFiberMap.begin(); i != mFiberMap.end(); i++)
   {
      i->second->setState(Fiber::Dead);
      delete i->second;
   }
   mFiberMap.clear();
}

void FiberScheduler::start(OperationRunner* opRunner, int numOps)
{
   // create "numOps" Operations
   for(int i = 0; i < numOps; i++)
   {
      // create Operation
      Operation op(*this);
      mOpList.add(op);
   }
   
   // queue Operations
   mOpList.queue(opRunner);
}

inline void FiberScheduler::stop()
{
   // terminate all operations
   mOpList.terminate();
   
   // delete all thread contexts
   for(ContextList::iterator i = mContextList.begin();
       i != mContextList.end(); i++)
   {
      delete *i;
   }
   mContextList.clear();
}

bool FiberScheduler::waitForLastFiberExit(bool stop)
{
   bool rval = true;
   
   mNoFibersWaitLock.lock();
   {
      // wait on the no fibers lock until there are no more fibers
      while(rval && !mFiberMap.empty())
      {
         rval = mNoFibersWaitLock.wait();
      }
   }
   mNoFibersWaitLock.unlock();
   
   if(rval && stop)
   {
      // stop fiber scheduler
      this->stop();
   }
   
   return rval;
}

FiberId FiberScheduler::addFiber(Fiber* fiber)
{
   FiberId id;
   
   // lock scheduler to add fiber
   mScheduleLock.lock();
   {
      // get available FiberId
      id = mFiberIdFreeList.front();
      mFiberIdFreeList.pop_front();
      
      // add new id if list is empty
      if(mFiberIdFreeList.empty())
      {
         mFiberIdFreeList.push_back(id + 1);
      }
      
      // assign id and scheduler to fiber
      fiber->setScheduler(id, this);
      
      // add fiber to map and queue
      mFiberMap.insert(make_pair(id, fiber));
      mFiberQueue.push_back(fiber);
      
      // notify that a fiber is available for scheduling
      fiberAvailable();
   }
   mScheduleLock.unlock();
   
   return id;
}

void FiberScheduler::run()
{
   // get and store scheduler context for this thread
   FiberContext* scheduler = new FiberContext();
   mScheduleLock.lock();
   {
      mContextList.push_back(scheduler);
   }
   mScheduleLock.unlock();
   
   // continue scheduling fibers while this thread is not interrupted
   bool tryInit = true;
   Thread* t = Thread::currentThread();
   while(!t->isInterrupted())
   {
      // synchronously get the next fiber to schedule
      Fiber* fiber = nextFiber();
      
      // if there is no fiber to schedule
      if(fiber == NULL)
      {
         // wait until one is available
         waitForFiber();
      }
      // else a fiber has been found
      else
      {
         if(fiber->getState() == Fiber::New)
         {
            // initialize the fiber's context
            if(tryInit && fiber->getContext()->init(fiber))
            {
               // set fiber state to running
               fiber->setState(Fiber::Running);
            }
            else
            {
               // do not try init again until a fiber's stack is reclaimed
               tryInit = false;
            }
         }
         
         if(fiber->getState() != Fiber::Running)
         {
            // failed to init fiber, not enough memory, lock to re-queue it
            mScheduleLock.lock();
            {
               mFiberQueue.push_back(fiber);
            }
            mScheduleLock.unlock();
         }
         else
         {
            // swap in the fiber's context
            scheduler->swap(fiber->getContext());
            
            // lock scheduling while adding fiber back to queue
            mScheduleLock.lock();
            {
               if(fiber->getState() != Fiber::Sleeping)
               {
                  // if fiber is running, put it in the back of the queue
                  if(fiber->getState() == Fiber::Running)
                  {
                     mFiberQueue.push_back(fiber);
                  }
                  // fiber is dying, so put it in front for quicker cleanup
                  else
                  {
                     mFiberQueue.push_front(fiber);
                     
                     // because fiber's stack memory will be reclaimed, it
                     // is safe to try init on new fibers again
                     tryInit = true;
                  }
                  
                  // notify that a fiber is available
                  fiberAvailable();
               }
            }
            mScheduleLock.unlock();
         }
      }
   }
}

void FiberScheduler::yield(Fiber* fiber)
{
   // swap scheduler back in
   fiber->getContext()->swapBack();
}

void FiberScheduler::sleep(Fiber* fiber)
{
   // lock scheduling to insert sleeping fiber entry
   mScheduleLock.lock();
   {
      // only *actually* sleep fiber if it can be sleeped at the moment
      if(fiber->canSleep())
      {
         fiber->setState(Fiber::Sleeping);
         mSleepingFibers.insert(make_pair(fiber->getId(), fiber));
      }
   }
   mScheduleLock.unlock();
   
   // swap scheduler back in
   fiber->getContext()->swapBack();
}

void FiberScheduler::wakeupSelf(Fiber* fiber)
{
   // lock scheduling while waking up sleeping fiber
   mScheduleLock.lock();
   {
      // only *actually* wake up self if sleeping
      if(fiber->getState() == Fiber::Sleeping)
      {
         wakeup(fiber->getId());
      }
   }
   mScheduleLock.unlock();
}

void FiberScheduler::wakeup(FiberId id)
{
   // lock scheduling while waking up sleeping fiber
   mScheduleLock.lock();
   {
      // find the sleeping fiber
      FiberMap::iterator i = mSleepingFibers.find(id);
      if(i != mSleepingFibers.end())
      {
         // update fiber state, add to queue, remove from sleeping fibers map
         i->second->setState(Fiber::Running);
         mFiberQueue.push_back(i->second);
         mSleepingFibers.erase(i);
         
         // notify that a fiber is available
         fiberAvailable();
      }
   }
   mScheduleLock.unlock();
}

void FiberScheduler::exit(Fiber* fiber)
{
   fiber->setState(Fiber::Exited);
   
   // load scheduler back in
   fiber->getContext()->loadBack();
}

Fiber* FiberScheduler::nextFiber()
{
   Fiber* rval = NULL;
   
   // lock scheduling while iterating over and modifying fiber queue
   mScheduleLock.lock();
   {
      // look for a fiber that can be scheduled:
      
      // cycle through fibers not more than once
      FiberId cycleEnd = 0;
      for(FiberQueue::iterator i = mFiberQueue.begin();
          rval == NULL && i != mFiberQueue.end() && (*i)->getId() != cycleEnd;)
      {
         // pop fiber off queue
         Fiber* fiber = *i;
         i = mFiberQueue.erase(i);
         
         // check state of fiber
         switch(fiber->getState())
         {
            // if fiber is new or running, it can be scheduled
            case Fiber::New:
            case Fiber::Running:
               rval = fiber;
               break;
               // exited or dead fibers must be deleted
            case Fiber::Exited:
               fiber->setState(Fiber::Dead);
            case Fiber::Dead:
               // add fiber ID to free list, remove fiber from scheduler
               mFiberIdFreeList.push_front(fiber->getId());
               mFiberMap.erase(fiber->getId());
               delete fiber;
               fiber = NULL;
               break;
               // sleeping fibers cannot be scheduled
            case Fiber::Sleeping:
               break;
         }
         
         // if current fiber cannot be scheduled
         if(rval == NULL && fiber != NULL)
         {
            // push fiber onto end of queue
            mFiberQueue.push_back(fiber);
            
            // ensure current fiber is not checked again
            if(cycleEnd == 0)
            {
               cycleEnd = fiber->getId();
            }
         }
      }
      
      if(mFiberMap.empty())
      {
         // notify that no fibers are available
         noFibersAvailable();
      }
   }
   mScheduleLock.unlock();
   
   return rval;
}

inline void FiberScheduler::fiberAvailable()
{
   mFiberWaitLock.lock();
   mFiberWaitLock.notifyAll();
   mFiberWaitLock.unlock();
}

inline void FiberScheduler::waitForFiber()
{
   mFiberWaitLock.lock();
   mFiberWaitLock.wait();
   mFiberWaitLock.unlock();
}

inline void FiberScheduler::noFibersAvailable()
{
   mNoFibersWaitLock.lock();
   mNoFibersWaitLock.notifyAll();
   mNoFibersWaitLock.unlock();
}
