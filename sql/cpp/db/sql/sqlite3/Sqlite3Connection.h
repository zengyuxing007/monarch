/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_sql_sqlite3_Sqlite3Connection_H
#define db_sql_sqlite3_Sqlite3Connection_H

#include <sqlite3.h>

#include "db/sql/Connection.h"
#include "db/sql/sqlite3/Sqlite3Exception.h"

namespace db
{
namespace sql
{
namespace sqlite3
{

// forward declaration
class Sqlite3Statement;

/**
 * An Sqlite3Connection is a Connection to an sqlite3 database.
 * 
 * @author Dave Longley
 * @author David I. Lehn
 */
class Sqlite3Connection : public db::sql::Connection
{
protected:
   /**
    * The handle to the sqlite3 database.
    */
   ::sqlite3* mHandle;
   
   /**
    * These classes are friends so they can access the C handle to
    * the database.
    */
   friend class Sqlite3Statement;
   friend class Sqlite3Exception;
   
public:
   /**
    * Creates a new Connection.
    */
   Sqlite3Connection();
   
   /**
    * Destructs this Connection.
    */
   virtual ~Sqlite3Connection();
   
   /**
    * Connects to the database specified by the given url.
    * 
    * @param url Sqlite3 parameters in URL form:
    *        "sqlite://user:password@/path/to/example.db"
    *        A SQLite3 database called example.db
    * 
    * @return an SqlException if one occurred, NULL if not.
    */
   virtual SqlException* connect(const char* url);
   
   /**
    * Connects to the database specified by the given url.
    * 
    * @param url Sqlite3 parameters in URL form:
    *        "sqlite://user:password@/path/to/example.db"
    *        A SQLite3 database called example.db
    * 
    * @return an SqlException if one occurred, NULL if not.
    */
   virtual SqlException* connect(db::net::Url* url);
   
   /**
    * Prepares a Statement for execution. The Statement is heap-allocated and
    * must be freed by the caller of this method.
    * 
    * @param sql the standard query language text of the Statement.
    * 
    * @return the new Statement to be freed by caller, NULL if an
    *         exception occurred.
    */
   virtual Statement* prepare(const char* sql);
   
   /**
    * Closes this connection.
    */
   virtual void close();
   
   /**
    * Commits the current transaction.
    * 
    * @return an SqlException if one occurred, NULL if not.
    */
   virtual SqlException* commit();
   
   /**
    * Rolls back the current transaction.
    * 
    * @return an SqlException if one occurred, NULL if not.
    */
   virtual SqlException* rollback();
};

} // end namespace sqlite3
} // end namespace sql
} // end namespace db
#endif