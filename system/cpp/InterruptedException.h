/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef InterruptedException_H
#define InterruptedException_H

#include "Exception.h"

namespace db
{
namespace system
{

/**
 * An InterruptedException is thrown when some kind of operation is interrupted.
 *
 * @author Dave Longley
 */
class InterruptedException : public Exception
{
public:
   /**
    * Creates a new InterruptedException.
    *
    * A message and code may be optionally specified.
    *
    * @param message the message for this Exception.
    * @param code the code for this Exception.
    */
   InterruptedException(std::string message = "", std::string code = "");
};

} // end namespace system
} // end namespace db
#endif
