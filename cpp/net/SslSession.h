/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_net_SslSession_H
#define monarch_net_SslSession_H

#include "monarch/rt/Collectable.h"

#include <openssl/ssl.h>

namespace monarch
{
namespace net
{

/**
 * The SslSessionImpl is a simple container that will store an
 * SSL_SESSION and free it when it is destructed.
 *
 * @author Dave Longley
 */
class SslSessionImpl
{
public:
   SSL_SESSION* session;
   SslSessionImpl(SSL_SESSION* s = NULL)
   {
      session = s;
   }

   virtual ~SslSessionImpl()
   {
      SSL_SESSION_free(session);
   }
};

/**
 * The SslSession type is a reference counted container for an SSL_SESSION.
 */
typedef monarch::rt::Collectable<SslSessionImpl> SslSession;

} // end namespace net
} // end namespace monarch
#endif
