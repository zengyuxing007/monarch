/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_StateMutator_H
#define db_modest_StateMutator_H

#include "db/modest/State.h"
#include "db/modest/Operation.h"

namespace db
{
namespace modest
{

/**
 * A StateMutator can be used by an Operation to alter the State of an Engine
 * directly before and/or after the Operation's execution.
 * 
 * @author Dave Longley
 */
class StateMutator
{
public:
   /**
    * Creates a new StateMutator.
    */
   StateMutator() {};
   
   /**
    * Destructs this StateMutator.
    */
   virtual ~StateMutator() {};
   
   /**
    * Alters the passed State directly before an Operation executes.
    * 
    * @param s the State to alter.
    * @param op the Operation to be executed.
    */
   virtual void mutatePreExecutionState(State* s, Operation op) {};
   
   /**
    * Alters the passed State directly after an Operation finishes or
    * was canceled.
    * 
    * The passed Operation may be checked to see if it finished or was
    * canceled, etc.
    * 
    * @param s the State to alter.
    * @param op the Operation that finished or was canceled.
    */
   virtual void mutatePostExecutionState(State* s, Operation op) {};
};

} // end namespace modest
} // end namespace db
#endif
