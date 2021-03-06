/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_http_HttpRequest_H
#define monarch_http_HttpRequest_H

#include "monarch/io/InputStream.h"
#include "monarch/io/OutputStream.h"
#include "monarch/http/HttpRequestHeader.h"
#include "monarch/http/HttpTrailer.h"

namespace monarch
{
namespace http
{

// forward declare classes
class HttpConnection;
class HttpResponse;

/**
 * An HttpRequest is a web request that uses the HTTP protocol.
 *
 * @author Dave Longley
 */
class HttpRequest
{
protected:
   /**
    * The header for this request.
    */
   HttpRequestHeader mHeader;

   /**
    * The HttpConnection this request is for.
    */
   HttpConnection* mConnection;

public:
   /**
    * Creates a new HttpRequest for the passed HttpConnection.
    *
    * @param hc the HttpConnection this request is for.
    */
   HttpRequest(HttpConnection* hc);

   /**
    * Destructs this HttpRequest.
    */
   virtual ~HttpRequest();

   /**
    * Creates a new HttpResponse.
    *
    * The caller of this method is responsible for freeing the created response.
    *
    * @return the new HttpResponse.
    */
   virtual HttpResponse* createResponse();

   /**
    * Sends the header for this request. This method will block until the
    * entire header has been sent, the connection times out, or the thread
    * is interrupted.
    *
    * @return true if the header was sent, false if an Exception occurred.
    */
   virtual bool sendHeader();

   /**
    * Receives the header for this request. This method will block until the
    * entire header has been received, the connection times out, or the thread
    * is interrupted.
    *
    * @return true if the header was received, false if an Exception occurred.
    */
   virtual bool receiveHeader();

   /**
    * Sends the body for this request. This method will block until the
    * entire body has been sent, the connection times out, or the thread is
    * interrupted.
    *
    * @param is the InputStream to read the body from.
    * @param trailer header trailers to send.
    *
    * @return true if the body was sent, false if an Exception occurred.
    */
   virtual bool sendBody(monarch::io::InputStream* is, HttpTrailer* trailer = NULL);

   /**
    * Gets a heap-allocated OutputStream for sending a message body. The
    * stream must be closed and deleted when it is finished being used. Closing
    * the stream will not shut down output or close the connection. The stream
    * will automatically handle transfer-encoding (i.e. "chunked") based on
    * the passed header. The stream will also automatically update the number
    * of content bytes sent by this connection. The stream will not ensure that
    * the number of content-bytes sent matches the Content-Length (if one
    * was specified).
    *
    * @param header the header to send the message body for.
    * @param trailer any trailer headers to send if appropriate.
    *
    * @return the heap-allocated OutputStream for sending a message body.
    */
   virtual monarch::io::OutputStream* getBodyOutputStream(
      HttpTrailer* trailer = NULL);

   /**
    * Receives the body for this request. This method will block until the
    * entire body has been received, the connection times out, or the thread
    * is interrupted.
    *
    * @param os the OutputStream to write the body to.
    * @param trailer used to store received header trailers.
    *
    * @return true if the body was received, false if an Exception occurred.
    */
   virtual bool receiveBody(
      monarch::io::OutputStream* os, HttpTrailer* trailer = NULL);

   /**
    * Gets a heap-allocated InputStream for receiving a message body. The
    * stream must be closed and deleted when it is finished being used. Closing
    * the stream will not shut down input or close the connection. The stream
    * will automatically handle transfer-encoding (i.e. "chunked") based on
    * the passed header. The stream will also automatically update the number
    * of content bytes received by this connection.
    *
    * @param trailer user to store any received trailer headers.
    *
    * @return the heap-allocated InputStream for receiving a message body.
    */
   virtual monarch::io::InputStream* getBodyInputStream(HttpTrailer* trailer = NULL);

   /**
    * Gets the header for this request. This will not receive the header
    * from the underlying connection, it will only return the header object.
    *
    * @return the header for this request.
    */
   virtual HttpRequestHeader* getHeader();

   /**
    * Gets the HttpConnection associated with this request.
    *
    * @return the HttpConnection associated with this request.
    */
   virtual HttpConnection* getConnection();
};

// typedef for a counted reference to an HttpRequest
typedef monarch::rt::Collectable<HttpRequest> HttpRequestRef;

} // end namespace http
} // end namespace monarch
#endif
