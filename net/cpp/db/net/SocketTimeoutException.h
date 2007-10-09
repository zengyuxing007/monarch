/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_SocketTimeoutException_H
#define db_net_SocketTimeoutException_H

#include "db/net/SocketException.h"

namespace db
{
namespace net
{

/**
 * A SocketTimeoutException is raised when a socket timeout occurs.
 *
 * @author Dave Longley
 */
class SocketTimeoutException : public SocketException
{
public:
   /**
    * Creates a new SocketTimeoutException.
    *
    * A message, type, and code may be optionally specified.
    *
    * @param message the message for this Exception.
    * @param type the type for this Exception.
    * @param code the code for this Exception.
    */
   SocketTimeoutException(
      const char* message = "",
      const char* type = "db.net.SocketTimeout", int code = 0);
   
   /**
    * Destructs this SocketTimeoutException.
    */
   virtual ~SocketTimeoutException();
};

} // end namespace net
} // end namespace db
#endif