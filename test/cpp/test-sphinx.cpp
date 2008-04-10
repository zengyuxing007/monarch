/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>
#include <sstream>

#include "db/sphinx/SphinxClient.h"
#include "db/data/json/JsonWriter.h"
#include "db/io/OStreamOutputStream.h"
#include "db/rt/Exception.h"
#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"

using namespace std;
using namespace db::sphinx;
using namespace db::data::json;
using namespace db::io;
using namespace db::net;
using namespace db::rt;
using namespace db::test;

void runSphinxClientTest(TestRunner &tr, db::test::Tester& tester)
{
   tr.group("SphinxClient");
   
   tr.test("searchd protocol");
   {
      Url url("sphinx://omega:3312");
      
      SphinxCommand cmd;
      SphinxResponse response;
      SphinxClient client;
      
      cmd["type"] = SPHINX_SEARCHD_CMD_SEARCH;
      cmd["query"] = "test";
      cmd["matchOffset"] = 0;
      cmd["matchCount"] = 20;
      cmd["matchMode"] = SPHINX_MATCH_ALL;
      cmd["rankMode"] = SPHINX_RANK_PROXIMITY_BM25;
      cmd["sortMode"] = SPHINX_SORT_RELEVANCE;
      cmd["weights"]->append() = 100;
      cmd["weights"]->append() = 1;
      cmd["indexes"] = "*";
      cmd["minId"] = 0;
      cmd["maxId"] = 0;
      cmd["maxMatches"] = 1000;
      cmd["groupSort"] = "@group desc";
      
      client.execute(&url, cmd, response);
      assertNoException();
      
      ostringstream oss;
      OStreamOutputStream os(&oss);
      JsonWriter writer;
      writer.setCompact(false);
      writer.setIndentation(0, 3);
      writer.write(response, &os);
      
      cout << endl << "Response:" << endl;
      cout << oss.str() << endl;
   }
   tr.passIfNoException();
   
   tr.ungroup();
}

class DbSphinxClientTester : public db::test::Tester
{
public:
   DbSphinxClientTester()
   {
      setName("SphinxClient");
   }

   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      runSphinxClientTest(tr, *this);
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
      return 0;
   }
};

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbSphinxClientTester)
#endif