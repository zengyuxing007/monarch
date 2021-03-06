/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_ws_PathHandler_H
#define monarch_ws_PathHandler_H

#include "monarch/ws/ChannelExceptionHandler.h"
#include "monarch/ws/RequestAuthenticator.h"

#include <vector>

namespace monarch
{
namespace ws
{

/**
 * A PathHandler handles a path for a WebService in some fashion. Its
 * operator() should receive the content from a client using the passed
 * ServiceChannel and then it should send an appropriate response.
 *
 * @author Dave Longley
 */
class PathHandler : ChannelExceptionHandler
{
protected:
   /**
    * True if a secure connection is required, false if not.
    */
   bool mSecureOnly;

   /**
    * A list of request authenticators.
    */
   typedef std::vector<RequestAuthenticatorRef> RequestAuthList;
   RequestAuthList mAuthMethods;

   /**
    * The exception handler to use.
    */
   ChannelExceptionHandler* mExceptionHandler;
   ChannelExceptionHandlerRef mExceptionHandlerRef;

public:
   /**
    * Creates a new PathHandler.
    *
    * @param secureOnly true if this PathHandler should return a 404 if the
    *           connection is not secure.
    */
   PathHandler(bool secureOnly = false);

   /**
    * Destructs this PathHandler.
    */
   virtual ~PathHandler();

   /**
    * Checks to see if the handler can handle the client's request. If not,
    * an exception must be set that will be sent to the client. The default
    * implementation will call checkAuthentication().
    *
    * @param ch the communication channel with the client.
    *
    * @return true if the request can be handled.
    */
   virtual bool canHandleRequest(ServiceChannel* ch);

   /**
    * Checks to see if the request sent over the given channel is authenticated
    * according to one of the given authentication methods. If the request is
    * authenticated by one of the methods then the authentication method and
    * data will be set. The first method of authentication that passed can
    * be retrieved from the channel.
    *
    * If no authentication methods were specified, then the channel's
    * authentication method will be set to NULL but this method will
    * return true.
    *
    * If no authentication methods are matched, then this method will return
    * false with an exception set.
    *
    * If the NULL authentication method was specified (anonymous) as permitted,
    * then this method will only return true if no other authentication method
    * was attempted by the client. If any method was attempted and failed,
    * this method will return false.
    *
    * @return true if authenticated, false if not with exception set.
    */
   virtual bool checkAuthentication(ServiceChannel* ch);

   /**
    * Handles the client's request. Does whatever is necessary to handle the
    * client's request and sends a response.
    *
    * @param ch the communication channel with the client.
    */
   virtual void handleRequest(ServiceChannel* ch);

   /**
    * Handle's the client's request by receiving its content, if any, and
    * sending an appropriate response.
    *
    * The default implementation for this method will first call
    * canHandleRequest() and then if that returns true it will call
    * handleRequest().
    *
    * @param ch the communication channel with the client.
    */
   virtual void operator()(ServiceChannel* ch);

   /**
    * Returns true if this handler requires a secure connection (ie: SSL/TLS),
    * false if not.
    *
    * @return true if this handler requires a secure connection, false if not.
    */
   virtual bool secureConnectionRequired();

   /**
    * Adds a RequestAuthenticator to this handler. Authentication methods are
    * checked in the order that they are added.
    *
    * @param method the RequestAuthenticator to add, NULL for anonymous.
    */
   virtual void addRequestAuthenticator(RequestAuthenticatorRef method);

   /**
    * Sets the ChannelExceptionHandler to call when an exception occurs. To
    * cause this PathHandler to manage the memory for the handler set cleanup
    * to true or call setExceptionHandlerRef() instead.
    *
    * @param h the ChannelExceptionHandler to use.
    * @param cleanup true to handle cleanup for the handler, false not to.
    */
   virtual void setExceptionHandler(
      ChannelExceptionHandler* h, bool cleanup = false);

   /**
    * Sets the ChannelExceptionHandler to call when an exception occurs. This
    * PathHandler will manage the memory for the handler using reference
    * counting.
    *
    * @param h the ChannelExceptionHandler to use.
    */
   virtual void setExceptionHandlerRef(ChannelExceptionHandlerRef h);

   /**
    * {@inheritDoc}
    */
   virtual void handleChannelException(
      ServiceChannel* ch, monarch::rt::ExceptionRef& e);
};

// type definition for a reference counted PathHandler
typedef monarch::rt::Collectable<PathHandler> PathHandlerRef;

} // end namespace ws
} // end namespace monarch
#endif
