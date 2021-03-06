/*
 * Copyright (c) 2008-2010 Digital Bazaar, Inc. All rights reserved.
 *
 * This test file is used to test the Monarch ConfigManager without the need
 * for it to use the Monarch App Tester framework -- which itself depends on
 * the ConfigManager. Only simple tests are run, namely tests to ensure that
 * the simplest config files can be loaded and that the simplest configuration
 * values can be combined and merged together, including groups.
 */
#include "monarch/config/ConfigManager.h"
#include "monarch/data/json/JsonReader.h"
#include "monarch/data/json/JsonWriter.h"
#include "monarch/io/FileInputStream.h"
#include "monarch/io/FileOutputStream.h"
#include "monarch/test/Test.h"

using namespace std;
using namespace monarch::config;
using namespace monarch::data::json;
using namespace monarch::io;
using namespace monarch::rt;
using namespace monarch::test;

#ifdef WIN32
#define TMPDIR "c:/WINDOWS/Temp"
#else
#define TMPDIR "/tmp"
#endif

static void _writeConfig(Config& config, const char* path)
{
   File file(path);
   FileOutputStream fos(file);
   JsonWriter writer;
   writer.setCompact(false);
   writer.write(config, &fos);
   fos.close();
   assertNoExceptionSet();
}

static void _readConfig(Config& config, const char* path)
{
   File file(path);
   FileInputStream fis(file);
   JsonReader reader;
   reader.start(config);
   reader.read(&fis) && reader.finish();
   fis.close();
   assertNoExceptionSet();
}

static void _testConfigs(
   ConfigManager& cm,
   Config& system,
   Config& engine, Config& ui,
   Config& user1, Config& user2,
   Config& child2)
{
   // FIXME: need to add failure tests (i.e. invalid config IDs, conflicts)

   // test system raw config
   {
      printf("Testing system raw config... ");

      Config raw = cm.getConfig("system", true);
      assertNoExceptionSet();
      assertDynoCmp(raw, system);

      printf("PASS.\n");
   }

   // test system merged config
   {
      printf("Testing system merged config... ");

      // create expect config
      Config expect;
      expect["path"]->setType(Array);
      expect["cowSays"] = "moo";
      expect["dogSays"] = "bowwow";
      expect["fruits"]["apple"] = "red";
      expect["vegetables"]["carrot"] = "orange";
      expect["vegetables"]["eggplant"] = "purple";
      DynamicObject two;
      two["one"] = "green";
      expect["vegetables"]["pepper"]->append(two);
      expect["vegetables"]["pepper"]->append("yellow");

      Config merged = cm.getConfig("system", false);
      assertNoExceptionSet();
      assertDynoCmp(merged, expect);

      printf("PASS.\n");
   }

   // test engine raw config
   {
      printf("Testing engine raw config... ");

      Config raw = cm.getConfig("engine", true);
      assertNoExceptionSet();
      assertDynoCmp(raw, engine);

      assertNoExceptionSet();
      printf("PASS.\n");
   }

   // test engine merged config
   {
      printf("Testing engine merged config... ");

      // create expect config
      Config expect;
      expect["path"]->setType(Array);
      expect["path"]->append("/usr/bin");
      expect["cowSays"] = "moo";
      expect["dogSays"] = "woof";
      expect["fruits"]["apple"] = "red";
      expect["fruits"]["banana"] = "yellow";
      expect["vegetables"]["carrot"] = "orange";
      DynamicObject two;
      two["one"] = "green";
      two["two"] = "red";
      expect["vegetables"]["pepper"]->append(two);
      expect["vegetables"]["pepper"]->append("yellow");

      Config merged = cm.getConfig("engine", false);
      assertNoExceptionSet();
      assertDynoCmp(merged, expect);

      assertNoExceptionSet();
      printf("PASS.\n");
   }

   // test ui raw config
   {
      printf("Testing ui raw config... ");

      Config raw = cm.getConfig("ui", true);
      assertNoExceptionSet();
      assertDynoCmp(raw, ui);

      assertNoExceptionSet();
      printf("PASS.\n");
   }

   // test ui merged config
   {
      printf("Testing ui merged config... ");

      // create expect config
      Config expect;
      expect["path"]->setType(Array);
      expect["path"]->append("/tmp/ui-tool");
      expect["cowSays"] = "moo";
      expect["dogSays"] = "bowwow";
      expect["fruits"]["apple"] = "red";
      expect["fruits"]["pear"] = "green";
      expect["vegetables"]["carrot"] = "orange";
      DynamicObject two;
      two["one"] = "green";
      two["two"] = "red";
      expect["vegetables"]["pepper"]->append(two);
      expect["vegetables"]["pepper"]->append("yellow");

      Config merged = cm.getConfig("ui", false);
      assertNoExceptionSet();
      assertDynoCmp(merged, expect);

      printf("PASS.\n");
   }

   // test app group raw config
   {
      printf("Testing app group raw config... ");

      // invalid to request a raw groupid so check for that condition
      Config raw = cm.getConfig("app", true);
      assertNoExceptionSet();
      assert(!raw.isNull());
      Exception::clear();

      printf("PASS.\n");
   }

   // test app group merged config
   {
      printf("Testing app group merged config... ");

      // create expect config
      Config expect;
      expect["path"]->setType(Array);
      expect["path"]->append("/usr/bin");
      expect["path"]->append("/tmp/ui-tool");
      expect["cowSays"] = "moo";
      expect["dogSays"] = "woof";
      expect["fruits"]["apple"] = "red";
      expect["fruits"]["banana"] = "yellow";
      expect["fruits"]["pear"] = "green";
      expect["vegetables"]["carrot"] = "orange";
      DynamicObject two;
      two["one"] = "green";
      two["two"] = "red";
      expect["vegetables"]["pepper"]->append(two);
      expect["vegetables"]["pepper"]->append("yellow");

      Config merged = cm.getConfig("app", false);
      assertNoExceptionSet();
      assertDynoCmp(merged, expect);

      printf("PASS.\n");
   }

   // test user1 raw config
   {
      printf("Testing user1 raw config... ");

      Config raw = cm.getConfig("user1", true);
      assertNoExceptionSet();
      assertDynoCmp(raw, user1);

      assertNoExceptionSet();
      printf("PASS.\n");
   }

   // test user1 merged config
   {
      printf("Testing user1 merged config... ");

      // create expect config
      Config expect;
      expect["path"]->setType(Array);
      expect["path"]->append("/usr/bin");
      expect["path"]->append("/tmp/ui-tool");
      expect["path"]->append("/home/user1");
      expect["cowSays"] = "moo";
      expect["dogSays"] = "woof";
      expect["fruits"]["apple"] = "red";
      expect["fruits"]["banana"] = "yellow";
      expect["fruits"]["pear"] = "green";
      expect["fruits"]["apricot"] = "orange";
      expect["vegetables"]["carrot"] = "orange";

      Config merged = cm.getConfig("user1", false);
      assertNoExceptionSet();
      assertDynoCmp(merged, expect);

      printf("PASS.\n");
   }

   // test user2 raw config
   {
      printf("Testing user2 raw config... ");

      Config raw = cm.getConfig("user2", true);
      assertNoExceptionSet();
      assertDynoCmp(raw, user2);

      assertNoExceptionSet();
      printf("PASS.\n");
   }

   // test user2 merged config
   {
      printf("Testing user2 merged config... ");

      // create expect config
      Config expect;
      expect["path"]->setType(Array);
      expect["path"]->append("/usr/bin");
      expect["path"]->append("/tmp/ui-tool");
      expect["path"]->append("/home/user2");
      expect["cowSays"] = "moo";
      expect["dogSays"] = "woof";
      expect["fruits"]["apple"] = "red";
      expect["fruits"]["banana"] = "yellow";
      expect["fruits"]["pear"] = "green";
      expect["vegetables"]["carrot"] = "orange";
      DynamicObject two;
      two["one"] = "green";
      two["two"] = "red";
      expect["vegetables"]["pepper"]->append(two);
      expect["vegetables"]["pepper"]->append("yellow");
      expect["bacon"]["cooked"] = "red";
      expect["bacon"]["raw"] = "pink";

      Config merged = cm.getConfig("user2", false);
      assertNoExceptionSet();
      assertDynoCmp(merged, expect);

      printf("PASS.\n");
   }

   // test child2 raw config
   {
      printf("Testing child2 raw config... ");

      Config raw = cm.getConfig("child2", true);
      assertNoExceptionSet();
      assertDynoCmp(raw, child2);

      assertNoExceptionSet();
      printf("PASS.\n");
   }

   // test child2 merged config
   {
      printf("Testing child2 merged config... ");

      // create expect config
      Config expect;
      expect["path"]->setType(Array);
      expect["path"]->append("/home/child2");
      expect["cowSays"] = "moo";
      expect["dogSays"] = "woof";
      expect["fruits"]["apple"] = "red";
      expect["fruits"]["banana"] = "yellow";
      expect["fruits"]["pear"] = "green";
      expect["vegetables"]["carrot"] = "orange";
      DynamicObject two;
      two["one"] = "green";
      two["two"] = "red";
      expect["vegetables"]["pepper"]->append(two);
      expect["vegetables"]["pepper"]->append("yellow");
      expect["bacon"]["cooked"] = "red";
      expect["shoes"] = "black";

      Config merged = cm.getConfig("child2", false);
      assertNoExceptionSet();
      assertDynoCmp(merged, expect);

      printf("PASS.\n");
   }

   // test setting user2 value
   {
      printf("Testing setting user2 value... ");

      Config raw = cm.getConfig("user2", true);
      raw[ConfigManager::MERGE]["added"] = true;
      cm.setConfig(raw);
      assertNoExceptionSet();

      printf("PASS\n");

      printf("Testing user2 raw config after setting value... ");

      Config raw2 = cm.getConfig("user2", true);
      assertNoExceptionSet();
      assertDynoCmp(raw2, raw);

      assertNoExceptionSet();
      printf("PASS.\n");
   }

   // test user2 merged config
   {
      printf("Testing user2 merged config after setting value... ");

      // create expect config
      Config expect;
      expect["path"]->setType(Array);
      expect["path"]->append("/usr/bin");
      expect["path"]->append("/tmp/ui-tool");
      expect["path"]->append("/home/user2");
      expect["cowSays"] = "moo";
      expect["dogSays"] = "woof";
      expect["fruits"]["apple"] = "red";
      expect["fruits"]["banana"] = "yellow";
      expect["fruits"]["pear"] = "green";
      expect["vegetables"]["carrot"] = "orange";
      DynamicObject two;
      two["one"] = "green";
      two["two"] = "red";
      expect["vegetables"]["pepper"]->append(two);
      expect["vegetables"]["pepper"]->append("yellow");
      expect["bacon"]["cooked"] = "red";
      expect["bacon"]["raw"] = "pink";
      expect["added"] = true;

      Config merged = cm.getConfig("user2", false);
      assertNoExceptionSet();
      assertDynoCmp(merged, expect);

      printf("PASS.\n");
   }

   // test child2 merged config
   {
      printf("Testing child2 merged config after changing user2... ");

      // create expect config
      Config expect;
      expect["path"]->setType(Array);
      expect["path"]->append("/home/child2");
      expect["cowSays"] = "moo";
      expect["dogSays"] = "woof";
      expect["fruits"]["apple"] = "red";
      expect["fruits"]["banana"] = "yellow";
      expect["fruits"]["pear"] = "green";
      expect["vegetables"]["carrot"] = "orange";
      DynamicObject two;
      two["one"] = "green";
      two["two"] = "red";
      expect["vegetables"]["pepper"]->append(two);
      expect["vegetables"]["pepper"]->append("yellow");
      expect["bacon"]["cooked"] = "red";
      expect["shoes"] = "black";
      expect["added"] = true;

      Config merged = cm.getConfig("child2", false);
      assertNoExceptionSet();
      assertDynoCmp(merged, expect);

      printf("PASS.\n");
   }
}

static void _testConfigs(
   Config& system,
   Config& engine, Config& ui,
   Config& user1, Config& user2,
   Config& child2)
{
   ConfigManager cm;

   // add system config
   {
      printf("Testing adding system config... ");

      assert(cm.addConfig(system));

      assertNoExceptionSet();
      printf("PASS.\n");
   }

   // add engine config
   {
      printf("Testing adding engine config... ");

      assert(cm.addConfig(engine));

      assertNoExceptionSet();
      printf("PASS.\n");
   }

   // add ui config
   {
      printf("Testing adding ui config... ");

      assert(cm.addConfig(ui));

      assertNoExceptionSet();
      printf("PASS.\n");
   }

   // add user1 config
   {
      printf("Testing adding user1 config... ");

      assert(cm.addConfig(user1));

      assertNoExceptionSet();
      printf("PASS.\n");
   }

   // add user2 config
   {
      printf("Testing adding user2 config... ");

      assert(cm.addConfig(user2));

      assertNoExceptionSet();
      printf("PASS.\n");
   }

   // add child2 config
   {
      printf("Testing adding child2 config... ");

      assert(cm.addConfig(child2));

      assertNoExceptionSet();
      printf("PASS.\n");
   }

   _testConfigs(cm, system, engine, ui, user1, user2, child2);
}

static void _testConfigFiles(
   const char* systemPath,
   const char* enginePath,
   const char* uiPath,
   const char* user1Path,
   const char* user2Path,
   const char* child2Path)
{
   ConfigManager cm;

   // read configs from disk
   Config system;
   Config engine;
   Config ui;
   Config user1;
   Config user2;
   Config child2;
   _readConfig(system, systemPath);
   _readConfig(engine, enginePath);
   _readConfig(ui, uiPath);
   _readConfig(user1, user1Path);
   _readConfig(user2, user2Path);
   _readConfig(child2, child2Path);

   // add system config
   {
      printf("Testing adding system config file... ");

      assert(cm.addConfigFile(systemPath));

      assertNoExceptionSet();
      printf("PASS.\n");
   }

   // add engine config
   {
      printf("Testing adding engine config file... ");

      assert(cm.addConfigFile(enginePath));

      assertNoExceptionSet();
      printf("PASS.\n");
   }

   // add ui config
   {
      printf("Testing adding ui config file... ");

      assert(cm.addConfigFile(uiPath));

      assertNoExceptionSet();
      printf("PASS.\n");
   }

   // add user1 config
   {
      printf("Testing adding user1 config file... ");

      assert(cm.addConfigFile(user1Path));

      assertNoExceptionSet();
      printf("PASS.\n");
   }

   // add user2 config
   {
      printf("Testing adding user2 config file... ");

      assert(cm.addConfigFile(user2Path));

      assertNoExceptionSet();
      printf("PASS.\n");
   }

   // add child2 config
   {
      printf("Testing adding child2 config file... ");

      assert(cm.addConfigFile(child2Path));

      assertNoExceptionSet();
      printf("PASS.\n");
   }

   _testConfigs(system, engine, ui, user1, user2, child2);
}

static void _initConfigs(
   Config& system,
   Config& engine, Config& ui,
   Config& user1, Config& user2,
   Config& child2)
{
   // build system config
   {
      // set properties
      system[ConfigManager::ID] = "system";

      // set merge info
      Config& merge = system[ConfigManager::MERGE];
      merge["path"]->setType(Array);
      merge["cowSays"] = "moo";
      merge["dogSays"] = "bowwow";
      merge["fruits"]["apple"] = "red";
      merge["vegetables"]["carrot"] = "orange";
      merge["vegetables"]["eggplant"] = "purple";
      DynamicObject two;
      two["one"] = "green";
      merge["vegetables"]["pepper"]->append(two);
      merge["vegetables"]["pepper"]->append("yellow");
   }

   // build engine config
   {
      // set properties
      engine[ConfigManager::ID] = "engine";
      engine[ConfigManager::PARENT] = "system";
      engine[ConfigManager::GROUP] = "app";

      // set merge info
      Config& merge = engine[ConfigManager::MERGE];
      merge["dogSays"] = "woof";
      merge["fruits"]["banana"] = "yellow";
      DynamicObject two;
      two["two"] = "red";
      merge["vegetables"]["pepper"]->append(two);

      // set append info
      Config& append = engine[ConfigManager::APPEND];
      append["path"]->append("/usr/bin");

      // set remove info
      Config& remove= engine[ConfigManager::REMOVE];
      remove["vegetables"]["eggplant"] = "";
   }

   // build ui config
   {
      // set properties
      ui[ConfigManager::ID] = "ui";
      ui[ConfigManager::PARENT] = "system";
      ui[ConfigManager::GROUP] = "app";

      // set merge info
      Config& merge = ui[ConfigManager::MERGE];
      merge["fruits"]["pear"] = "green";
      DynamicObject two;
      two["two"] = "red";
      merge["vegetables"]["pepper"]->append(two);

      // set append info
      Config& append = ui[ConfigManager::APPEND];
      append["path"]->append("/tmp/ui-tool");

      // set remove info
      Config& remove= ui[ConfigManager::REMOVE];
      remove["vegetables"]["eggplant"] = "";
   }

   // build user1 config
   {
      // set properties
      user1[ConfigManager::ID] = "user1";
      user1[ConfigManager::PARENT] = "app";

      // set merge info
      Config& merge = user1[ConfigManager::MERGE];
      merge["fruits"]["apricot"] = "orange";

      // set append info
      Config& append = user1[ConfigManager::APPEND];
      append["path"]->append("/home/user1");

      // set remove info
      Config& remove= user1[ConfigManager::REMOVE];
      remove["vegetables"]["pepper"]->setType(Array);
   }

   // build user2 config
   {
      // set properties
      user2[ConfigManager::ID] = "user2";
      user2[ConfigManager::PARENT] = "app";

      // set merge info
      Config& merge = user2[ConfigManager::MERGE];
      merge["bacon"]["cooked"] = "red";
      merge["bacon"]["raw"] = "pink";

      // set append info
      Config& append = user2[ConfigManager::APPEND];
      append["path"]->append("/home/user2");
   }

   // build child2 config
   // *Note: child2 is a child user that is based off of user2.
   {
      // set properties
      child2[ConfigManager::ID] = "child2";
      child2[ConfigManager::PARENT] = "user2";

      // set merge info
      Config& merge = child2[ConfigManager::MERGE];
      merge["shoes"] = "black";

      // set append info
      Config& append = child2[ConfigManager::APPEND];
      append["path"]->append("/home/child2");

      // set remove info
      Config& remove= child2[ConfigManager::REMOVE];
      remove["path"]->setType(Array);
      remove["bacon"]["raw"] = "";
   }
}

static void testConfigFiles()
{
   // create configs
   Config system;
   Config engine;
   Config ui;
   Config user1;
   Config user2;
   Config child2;
   _initConfigs(system, engine, ui, user1, user2, child2);

   // write configs to disk
   _writeConfig(system, TMPDIR "/test-system.config");
   _writeConfig(engine, TMPDIR "/test-engine.config");
   _writeConfig(ui, TMPDIR "/test-ui.config");
   _writeConfig(user1, TMPDIR "/test-user1.config");
   _writeConfig(user2, TMPDIR "/test-user2.config");
   _writeConfig(child2, TMPDIR "/test-child2.config");

   _testConfigFiles(
      TMPDIR "/test-system.config",
      TMPDIR "/test-engine.config",
      TMPDIR "/test-ui.config",
      TMPDIR "/test-user1.config",
      TMPDIR "/test-user2.config",
      TMPDIR "/test-child2.config");
}

void testConfigs()
{
   Config system;
   Config engine;
   Config ui;
   Config user1;
   Config user2;
   Config child2;

   _initConfigs(system, engine, ui, user1, user2, child2);
   _testConfigs(system, engine, ui, user1, user2, child2);
}

static void testFailures()
{
   // FIXME: failure tests could be more comprehensive

   // init configs
   Config system;
   Config engine;
   Config ui;
   Config user1;
   Config user2;
   Config child2;
   _initConfigs(system, engine, ui, user1, user2, child2);

   // add conflict to ui
   ui[ConfigManager::MERGE]["fruits"]["banana"] = "barf";

   ConfigManager cm;

   assert(cm.addConfig(system));
   assertNoExceptionSet();
   assert(cm.addConfig(engine));
   assertNoExceptionSet();
   assert(!cm.addConfig(ui));
   assertExceptionSet();
   Exception::clear();

   // remove conflict
   ui[ConfigManager::MERGE]["fruits"]->removeMember("banana");
   assert(cm.addConfig(ui));
   assertNoExceptionSet();
   assert(cm.addConfig(user1));
   assertNoExceptionSet();
   assert(cm.addConfig(user2));
   assertNoExceptionSet();
   assert(cm.addConfig(child2));

   // try to get bogus config ID
   {
      Config bogus(NULL);
      bogus = cm.getConfig("bogus", true);
      assertExceptionSet();
      Exception::clear();
      bogus = cm.getConfig("bogus", false);
      assertExceptionSet();
      Exception::clear();
   }

   // try to add config with bogus parent
   {
      Config config;
      config[ConfigManager::ID] = "fail";
      config[ConfigManager::PARENT] = "bogus";
      config[ConfigManager::MERGE]["test"] = "data";
      assert(!cm.addConfig(config));
      assertExceptionSet();
      Exception::clear();
   }

   // try to change a merged config
   {
      Config _user1 = cm.getConfig("user1", false);
      _user1[ConfigManager::MERGE]["modify"] = true;
      cm.setConfig(_user1);
      assertExceptionSet();
      Exception::clear();
   }

   // try to change the group on a config
   {
      Config _user1 = cm.getConfig("user1", true);
      _user1[ConfigManager::GROUP] = "app";
      cm.setConfig(_user1);
      assertExceptionSet();
      Exception::clear();
   }

   // try to change the parent on a config
   {
      Config _user1 = cm.getConfig("user1", true);
      _user1[ConfigManager::PARENT] = "system";
      cm.setConfig(_user1);
      assertExceptionSet();
      Exception::clear();
   }
}

static void testRemove()
{
   // Test case for REMOVE

   // init configs
   Config c1;
   c1[ConfigManager::ID] = "test1";
   c1[ConfigManager::MERGE]["a"] = "a";
   c1[ConfigManager::MERGE]["b"] = "b";

   Config c2;
   c2[ConfigManager::ID] = "test2";
   c2[ConfigManager::PARENT] = "test1";
   c2[ConfigManager::REMOVE]["b"] = "";

   Config c3;
   c3[ConfigManager::ID] = "test3";
   c3[ConfigManager::PARENT] = "test1";
   c3[ConfigManager::REMOVE]["b"].setNull();

   //dumpDynamicObject(c1);
   //dumpDynamicObject(c2);
   //dumpDynamicObject(c3);

   // test removal with string value
   {
      ConfigManager cm;
      Config expected;

      expected["a"] = "a";
      expected["b"] = "b";

      assertNoException(cm.addConfig(c1));
      assertNamedDynoCmp("result", cm.getConfig("test1"), "expected", expected);

      expected->clear();
      expected["a"] = "a";

      assertNoException(cm.addConfig(c2));
      assertNamedDynoCmp("result", cm.getConfig("test2"), "expected", expected);
   }

   // test removal with null
   {
      ConfigManager cm;
      Config expected;

      expected["a"] = "a";
      expected["b"] = "b";

      assertNoException(cm.addConfig(c1));
      assertNamedDynoCmp("result", cm.getConfig("test1"), "expected", expected);

      expected->clear();
      expected["a"] = "a";

      assertNoException(cm.addConfig(c3));
      assertNamedDynoCmp("result", cm.getConfig("test3"), "expected", expected);
   }
}

int main()
{
   printf("Testing ConfigManager...\n\n");

   testConfigs();
   testConfigFiles();
   testFailures();
   testRemove();

   printf("\nALL TESTS PASS.\n");
   printf("Done. Total:1 Passed:1 Failed:0 Warnings:0 Unknown:0.\n");
   Thread::exit();
   return 0;
}

#undef TMPDIR
