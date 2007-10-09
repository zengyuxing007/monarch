/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_SocketDataPresenterList_H
#define db_net_SocketDataPresenterList_H

#include "db/rt/Object.h"
#include "db/net/SocketDataPresenter.h"

#include <list>

namespace db
{
namespace net
{

/**
 * A SocketDataPresenterList is a SocketDataPresenter that maintains a list of
 * other SocketDataPresenters. If a wrapped Socket is requested from this
 * SocketDataPresenter, it will return the first wrapped Socket that can be
 * produced from the list of SocketDataPresenters.
 * 
 * @author Dave Longley
 */
class SocketDataPresenterList :
public virtual db::rt::Object, public SocketDataPresenter
{
protected:
   /**
    * A list of available SocketDataPresenters.
    */
   std::list<SocketDataPresenter*> mDataPresenters;
   
   /**
    * Set to true to clean up the memory for the SocketDataPresenters in this
    * list, false otherwise.
    */
   bool mCleanup;
   
public:
   /**
    * Creates a new SocketDataPresenterList.
    * 
    * @param cleanup true if the SocketDataPresenters added to this list
    *                should be freed when this list is destructed, false if not.
    */
   SocketDataPresenterList(bool cleanup);
   
   /**
    * Destructs this SocketDataPresenterList.
    */
   virtual ~SocketDataPresenterList();
   
   /**
    * Adds a SocketDataPresenter to this list.
    * 
    * @param sdp the SocketDataPresenter to add.
    */
   virtual void add(SocketDataPresenter* sdp);
   
   /**
    * Wraps the passed Socket if this presenter can provide the presentation
    * layer for its data. The created Socket should cleanup the passed Socket
    * when it is freed.
    * 
    * @param s the Socket to wrap.
    * @param secure true if the wrapper is considered a secure protocol,
    *               like SSL, false if not.
    * 
    * @return the wrapped Socket or NULL if this presenter cannot provide
    *         the presentation layer for the socket data.
    */
   virtual Socket* createPresentationWrapper(Socket* s, bool& secure);
};

} // end namespace net
} // end namespace db
#endif