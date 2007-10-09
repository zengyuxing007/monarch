/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/modest/State.h"

using namespace std;
using namespace db::modest;

State::State()
{
}

State::~State()
{
   // delete all state variables
   for(map<const char*, Variable*, VarNameComparator>::iterator i =
       mVarTable.begin(); i != mVarTable.end(); i++)
   {
      // delete name
      delete [] i->first;
      
      // free variable
      freeVariable(i->second);
   }
}

State::Variable* State::getVariable(const char* name)
{
   Variable* rval = NULL;
   
   map<const char*, Variable*, VarNameComparator>::iterator i =
      mVarTable.find(name);
   if(i != mVarTable.end())
   {
      rval = i->second;
   }
   
   return rval;
}

State::Variable* State::createVariable(const char* name, Variable::Type type)
{
   // check for an existing variable
   Variable* var = getVariable(name);
   if(var == NULL)
   {
      // create new variable
      var = new Variable();
      var->type = type;
      
      // create string as appropriate
      if(type == Variable::String)
      {
         var->s = new string();
      }
      
      // store variable in table
      char* str = new char[strlen(name) + 1];
      strcpy(str, name);
      mVarTable.insert(make_pair(str, var));
   }
   
   if(var->type != type)
   {
      // allocate or delete string as appropriate
      if(type == Variable::String)
      {
         var->s = new string();
      }
      else if(var->type == Variable::String)
      {
         delete var->s;
      }
   }
   
   return var;
}

void State::freeVariable(Variable* var)
{
   // delete string if applicable
   if(var->type == Variable::String)
   {
      delete var->s;
   }
   
   // delete variable
   delete var;
}

void State::setBoolean(const char* name, bool value)
{
   // create the variable and set its value
   Variable* var = createVariable(name, Variable::Boolean);
   var->b = value;
}

bool State::getBoolean(const char* name, bool& value)
{
   bool rval = false;
   
   Variable* var = getVariable(name);
   if(var != NULL && var->type == Variable::Boolean)
   {
      value = var->b;
      rval = true;
   }
   
   return rval;
}

void State::setInteger(const char* name, int value)
{
   // create the variable and set its value
   Variable* var = createVariable(name, Variable::Integer);
   var->i = value;
}

bool State::getInteger(const char* name, int& value)
{
   bool rval = false;
   
   Variable* var = getVariable(name);
   if(var != NULL && var->type == Variable::Integer)
   {
      value = var->i;
      rval = true;
   }
   
   return rval;
}

void State::setString(const char* name, const string& value)
{
   // create the variable and set its value
   Variable* var = createVariable(name, Variable::String);
   var->s->assign(value);
}

bool State::getString(const char* name, string& value)
{
   bool rval = false;
   
   Variable* var = getVariable(name);
   if(var != NULL && var->type == Variable::String)
   {
      value = *(var->s);
      rval = true;
   }
   
   return rval;
}

void State::removeVariable(const char* name)
{
   map<const char*, Variable*, VarNameComparator>::iterator i =
      mVarTable.find(name);
   if(i != mVarTable.end())
   {
      // delete name
      delete [] i->first;
      
      // free variable
      freeVariable(i->second);
      
      // remove variable from table
      mVarTable.erase(i);
   }
}