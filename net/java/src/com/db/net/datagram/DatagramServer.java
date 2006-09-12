/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.datagram;

import java.util.Collection;
import java.util.Iterator;
import java.util.Vector;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.util.BoxingHashMap;

/**
 * A Datagram Server is a server that listens for datagrams and responds
 * with them. 
 * 
 * @author Dave Longley
 */
public class DatagramServer
{
   /**
    * The ports for this server in the order that they were added.
    */
   protected Vector mPorts;
   
   /**
    * A table mapping ports to datagram handlers.
    */
   protected BoxingHashMap mPortToDatagramHandler;   

   /**
    * Whether or not this server is running.
    */
   protected boolean mRunning;
   
   /**
    * Creates a new DatagramServer.
    */
   public DatagramServer()
   {
      // create list for ports
      mPorts = new Vector();
      
      // create the port to handler map
      mPortToDatagramHandler = new BoxingHashMap();
      
      // server not running by default
      mRunning = false;
   }
   
   /**
    * Disposes this DatagramServer. Stops this DatagramServer.
    */
   protected void finalize()
   {
      stop();
   }   
   
   /**
    * Maps a port to a datagram handler.
    * 
    * @param port the port to map to a datagram handler.
    * @param dh the datagram handler.
    */
   protected void mapPortToDatagramHandler(int port, DatagramHandler dh)
   {
      mPortToDatagramHandler.put(port, dh);
   }
   
   /**
    * Adds a datagram handler to this server. Only one datagram handler is
    * permitted per port.
    * 
    * A datagram handler will fail to be added to listen on the given port
    * if there already exists another datagram handler listening on that port.

    * If the server is already running, then the datagram handler will begin
    * accepting datagrams on the specified port.
    * 
    * @param dh the datagram handler to add to this server.
    * @param port the port the datagram handler will listen on.
    * 
    * @return true if the datagram handler was successfully added, false if
    *         it was not.
    */
   public synchronized boolean addDatagramHandler(DatagramHandler dh, int port)
   {
      boolean rval = false;
      
      // see if there is already a datagram handler on the provided port
      DatagramHandler existingHandler = getDatagramHandler(port);
      if(existingHandler == null)
      {
         // add port to the port list
         mPorts.add("" + port);
         
         // so assign the port to the handler
         mapPortToDatagramHandler(port, dh);
         
         // if the server has already started, then start accepting datagrams
         // on the port
         if(isRunning())
         {
            // start accepting datagrams on the port
            dh.startAcceptingDatagrams(port);
         }
         
         // the handler was added
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Removes a datagram handler from this server. If the server is already
    * running, then it will stop accepting datagrams.
    * 
    * @param dh the datagram handler to remove from this server.
    */
   public synchronized void removeDatagramHandler(DatagramHandler dh)
   {
      // see if the handler exists
      dh = getDatagramHandler(dh.getPort());
      if(dh != null)
      {
         if(isRunning())
         {
            // stop accepting datagrams
            dh.stopAcceptingDatagrams();
            
            // stop datagram servicer
            dh.stopDatagramServicer();
         }
         
         // remove the port from the port list
         mPorts.remove("" + dh.getPort());
         
         // unmap the datagram handler
         mPortToDatagramHandler.remove(dh.getPort());
      }
   }
   
   /**
    * Removes all datagram handlers.
    */
   public synchronized void removeAllDatagramHandlers()
   {
      for(Iterator i = mPortToDatagramHandler.values().iterator();
          i.hasNext();)
      {
         DatagramHandler dh = (DatagramHandler)i.next();
         
         // stop accepting datagrams
         dh.stopAcceptingDatagrams();
         
         // stop datagram servicer
         dh.stopDatagramServicer();
      }
      
      // clear port list
      mPorts.clear();
      
      // clear map
      mPortToDatagramHandler.clear();
   }
   
   /**
    * Starts this server. All datagram handlers handlers will begin accepting
    * datagrams.
    */
   public synchronized void start()
   {
      // only start server if it isn't running
      if(!isRunning())
      {
         getLogger().debug(getClass(), "starting datagram server...");

         // start accepting datagrams on all datagram handlers
         for(Iterator i = mPorts.iterator(); i.hasNext();)
         {
            int port = Integer.parseInt((String)i.next());
            DatagramHandler dh = getDatagramHandler(port);
            
            // start accepting datagrams
            dh.startAcceptingDatagrams(port);
         }
         
         // server is running
         mRunning = true;
      }
         
      getLogger().debug(getClass(), "datagram server started.");
   }
   
   /**
    * Stops this datagram server. All datagram handlers will stop accepting
    * datagrams.
    */
   public synchronized void stop()
   {
      // only stop server if it is running
      if(isRunning())
      {
         getLogger().debug(getClass(), "stopping datagram server...");
         
         // stop all datagram handlers
         for(Iterator i = mPorts.iterator(); i.hasNext();)
         {
            int port = Integer.parseInt((String)i.next());
            DatagramHandler dh = getDatagramHandler(port);
            
            // stop accepting datagrams
            dh.stopAcceptingDatagrams();
            
            // stop datagram servicer
            dh.stopDatagramServicer();
         }
         
         // server is not running
         mRunning = false;
      }
      
      getLogger().debug(getClass(), "datagram server stopped.");
   }
   
   /**
    * Returns true if this datagram server is running, false if not.
    *
    * @return true if this datagram server is running, false if not.
    */
   public synchronized boolean isRunning()
   {
      return mRunning;
   }
   
   /**
    * Gets the datagram handler for the specified port.
    * 
    * @param port the port to get the datagram handler for.
    * 
    * @return the datagram handler for the specified port or null
    *         if no datagram handler is assigned to the port.
    */
   public DatagramHandler getDatagramHandler(int port)
   {
      return (DatagramHandler)mPortToDatagramHandler.get(port);
   }
   
   /**
    * Gets a list of all of the datagram handlers for this datagram server
    * in the order that they were added to this datagram server.
    * 
    * @return a list of all of the datagram handlers for this datagram
    *         server in the order that they were added to this datagram server.
    */
   public Collection getDatagramListeners()
   {
      Vector vector = new Vector();
      
      // add all datagram handlers according to port order
      for(Iterator i = mPorts.iterator(); i.hasNext();)
      {
         int port = Integer.parseInt((String)i.next());
         DatagramHandler dh = getDatagramHandler(port);
         
         // add datagram handler
         vector.add(dh);
      }
      
      return vector;
   }
   
   /**
    * Gets a list of all of the ports for this datagram server in the order
    * that they were added to this datagram server.
    * 
    * @return a list of all of the ports for this datagram server in the order
    *         that they were added to this datagram server.
    */
   public int[] getDatagramListenerPorts()
   {
      int[] ports = new int[mPorts.size()];
      
      int n = 0;
      for(Iterator i = mPorts.iterator(); i.hasNext();)
      {
         try
         {
            int port = Integer.parseInt((String)i.next());
            ports[n++] = port;
         }
         catch(Throwable ignore)
         {
         }
      }
      
      return ports;
   }
   
   /**
    * Gets the logger for this server.
    * 
    * @return the logger for this server.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }
}
