/*
 * Copyright (c) 2009-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_sql_mysql_MySqlDatabaseClient_H
#define monarch_sql_mysql_MySqlDatabaseClient_H

#include "monarch/sql/DatabaseClient.h"

namespace monarch
{
namespace sql
{
namespace mysql
{

/**
 * A MySqlDatabaseClient provides mysql specific overloads for the
 * DatabaseClient class.
 *
 * @author Dave Longley
 */
class MySqlDatabaseClient : public monarch::sql::DatabaseClient
{
public:
   /**
    * Creates a new MySqlDatabaseClient.
    */
   MySqlDatabaseClient();

   /**
    * Destructs this MySqlDatabaseClient.
    */
   virtual ~MySqlDatabaseClient();

   /**
    * Creates an SqlExecutable that will insert a row into a table. All
    * applicable values in the given object will be inserted into the given
    * table, according to its schema. If a duplicate key error occurs, it
    * will be ignored and no insert will occur. This can be checked by
    * inspecting the rowsAffected property on the sql executable.
    *
    * @param table the name of the table to INSERT OR IGNORE INTO.
    * @param row the object with data to insert as a row.
    *
    * @return the SqlExecutable if successful, NULL if an Exception occurred.
    */
   virtual SqlExecutableRef insertOrIgnore(
      const char* table, monarch::rt::DynamicObject& row);

   /**
    * Creates an SqlExecutable that will insert a row into a table and if
    * a duplicate key is found, it will update that row. In effect, the
    * special mysql INSERT INTO ... ON DUPLICATE KEY UPDATE will be executed.
    * All applicable values in the given object will be inserted into/updated
    * in the given table, according to its schema.
    *
    * @param table the name of the table to insert into/update.
    * @param row the object with data to insert/update as a row.
    *
    * @return the SqlExecutable if successful, NULL if an Exception occurred.
    */
   virtual SqlExecutableRef insertOnDuplicateKeyUpdate(
      const char* table, monarch::rt::DynamicObject& row);
};

} // end namespace mysql
} // end namespace sql
} // end namespace monarch
#endif
