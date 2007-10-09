/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_SslSocket_H
#define db_net_SslSocket_H

#include "db/net/TcpSocket.h"
#include "db/net/SocketWrapper.h"
#include "db/net/SslContext.h"

#include <openssl/ssl.h>

namespace db
{
namespace net
{

/**
 * An SslSocket is a Socket that uses the TCP/IP protocol and the Secure
 * Sockets Layer (SSL v2/v3) and Transport Layer Security (TLS v1).
 * 
 * @author Dave Longley
 */
class SslSocket : public SocketWrapper
{
protected:
   /**
    * The SSL object for this socket.
    */
   SSL* mSSL;
   
   /**
    * A BIO (Basic Input/Output) for SSL data.
    * 
    * A read on this BIO will read data that has been pulled from the Socket.
    * 
    * A write on this BIO will provide SSL data for the Socket BIO to
    * send out to the Socket.
    */
   BIO* mSSLBio;
   
   /**
    * A BIO (Basic Input/Output) for socket data.
    * 
    * A read on this BIO will read SSL data written by the SSL layer. That
    * data can then be sent out to the Socket.
    * 
    * A write on this BIO will provide data for the SSL layer to read. Data
    * read from the Socket is written to this BIO.
    */
   BIO* mSocketBio;
   
   /**
    * True if an SSL session has been negotiated via a handshake, false if not.
    */
   bool mSessionNegotiated;
   
   /**
    * The stream for reading from the Socket.
    */
   db::io::InputStream* mInputStream;
   
   /**
    * The stream for writing to the Socket.
    */
   db::io::OutputStream* mOutputStream;
   
   /**
    * Reads some raw data from the underlying TCP socket and stores it in the
    * SSL read BIO. This method will block until at least one byte can be
    * read or until the end of the stream is reached (the Socket has closed).
    * 
    * @return the number of bytes read or 0 if the end of the stream has been
    *         reached (the Socket has closed) or -1 if an exception occurred.
    */
   virtual int tcpRead();
   
   /**
    * Flushes the data from the SSL write BIO to the underlying TCP Socket.
    * This method will block until all of the data has been written.
    * 
    * @return true if the write was successful, false if an exception occurred.
    */
   virtual bool tcpWrite();
   
public:
   /**
    * Creates a new SslSocket that wraps the passed TcpSocket.
    * 
    * @param context the SslContext underwhich to create this socket.
    * @param socket the TcpSocket to wrap.
    * @param client true if the TcpSocket is a client socket, false if it
    *               is a server socket.
    * @param cleanup true to reclaim the memory used for the wrapped Socket
    *                upon destruction, false to do nothing.
    */
   SslSocket(
      SslContext* context,
      TcpSocket* socket, bool client, bool cleanup = false);
   
   /**
    * Destructs this SslSocket.
    */
   virtual ~SslSocket();
   
   /**
    * Closes this Socket. This will be done automatically when the Socket is
    * destructed.
    */
   virtual void close();   
   
   /**
    * Explicitly performs an SSL handshake to initiate communications. A
    * handshake will be automatically performed by calling receive() or
    * send() (or using this Socket's input/output streams) -- so this method
    * does not *need* to be called. It can be called if an explicit handshake
    * is desired.
    * 
    * @return true if the handshake was successful, false if an exception
    *         occurred.
    */
   virtual bool performHandshake();
   
   /**
    * Writes raw data to this Socket. This method will block until all of
    * the data has been written.
    * 
    * Note: This method is *not* preferred. Use getOutputStream() to obtain the
    * output stream for this Socket.
    * 
    * @param b the array of bytes to write.
    * @param length the number of bytes to write to the stream.
    * 
    * @return true if the data was sent, false if an exception occurred.
    */
   virtual bool send(const char* b, int length);
   
   /**
    * Reads raw data from this Socket. This method will block until at least
    * one byte can be read or until the end of the stream is reached (the
    * Socket has closed). A value of 0 will be returned if the end of the
    * stream has been reached, otherwise the number of bytes read will be
    * returned.
    * 
    * Note: This method is *not* preferred. Use getInputStream() to obtain the
    * input stream for this Socket.
    * 
    * @param b the array of bytes to fill.
    * @param length the maximum number of bytes to read into the buffer.
    * 
    * @return the number of bytes read from the stream or 0 if the end of the
    *         stream (the Socket has closed) has been reached or -1 if an error
    *         occurred.
    */
   virtual int receive(char* b, int length);   
   
   /**
    * Gets the InputStream for reading from this Socket.
    * 
    * @return the InputStream for reading from this Socket.
    */
   virtual db::io::InputStream* getInputStream();
   
   /**
    * Gets the OutputStream for writing to this Socket.
    * 
    * @return the OutputStream for writing to this Socket.
    */
   virtual db::io::OutputStream* getOutputStream();
};

} // end namespace net
} // end namespace db
#endif