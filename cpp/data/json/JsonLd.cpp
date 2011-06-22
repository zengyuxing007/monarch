/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/json/JsonLd.h"

#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/rt/Exception.h"
#include "monarch/util/StringTools.h"

#include <cstdio>

using namespace std;
using namespace monarch::data;
using namespace monarch::data::json;
using namespace monarch::rt;
using namespace monarch::util;

#define RDF_NS            "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define XSD_NS            "http://www.w3.org/2001/XMLSchema#"

#define RDF_TYPE          RDF_NS "type"
#define XSD_ANY_TYPE      XSD_NS "anyType"
#define XSD_BOOLEAN       XSD_NS "boolean"
#define XSD_DOUBLE        XSD_NS "double"
#define XSD_INTEGER       XSD_NS "integer"
#define XSD_ANY_URI       XSD_NS "anyURI"

#define EXCEPTION_TYPE    "monarch.data.json.JsonLd"

JsonLd::JsonLd()
{
}

JsonLd::~JsonLd()
{
}

/**
 * Creates the JSON-LD default context.
 *
 * @return the JSON-LD default context.
 */
static DynamicObject _createDefaultContext()
{
   DynamicObject ctx;
   ctx["a"] = RDF_TYPE;
   ctx["rdf"] = RDF_NS;
   ctx["rdfs"] = "http://www.w3.org/2000/01/rdf-schema#";
   ctx["owl"] = "http://www.w3.org/2002/07/owl#";
   ctx["xsd"] = "http://www.w3.org/2001/XMLSchema#";
   ctx["dcterms"] = "http://purl.org/dc/terms/";
   ctx["foaf"] = "http://xmlns.com/foaf/0.1/";
   ctx["cal"] = "http://www.w3.org/2002/12/cal/ical#";
   ctx["vcard"] = "http://www.w3.org/2006/vcard/ns#";
   ctx["geo"] = "http://www.w3.org/2003/01/geo/wgs84_pos#";
   ctx["cc"] = "http://creativecommons.org/ns#";
   ctx["sioc"] = "http://rdfs.org/sioc/ns#";
   ctx["doap"] = "http://usefulinc.com/ns/doap#";
   ctx["com"] = "http://purl.org/commerce#";
   ctx["ps"] = "http://purl.org/payswarm#";
   ctx["gr"] = "http://purl.org/goodrelations/v1#";
   ctx["sig"] = "http://purl.org/signature#";
   ctx["ccard"] = "http://purl.org/commerce/creditcard#";
   ctx["@vocab"] = "";

   DynamicObject& coerce = ctx["@coerce"];
   coerce["xsd:anyURI"]->append("foaf:homepage");
   coerce["xsd:anyURI"]->append("foaf:member");
   coerce["xsd:integer"] = "foaf:age";

   return ctx;
}

/**
 * Compacts an IRI into a term or CURIE it can be. IRIs will not be compacted
 * to relative IRIs if they match the given context's default vocabulary.
 *
 * @param ctx the context to use.
 * @param iri the IRI to compact.
 * @param usedCtx a context to update if a value was used from "ctx".
 *
 * @return the compacted IRI as a term or CURIE or the original IRI.
 */
static string _compactIri(
   DynamicObject& ctx, const char* iri, DynamicObject* usedCtx)
{
   string rval;

   // check the context for a term that could shorten the IRI
   // (give preference to terms over CURIEs)
   DynamicObjectIterator i = ctx.getIterator();
   while(rval.empty() && i->hasNext())
   {
      // get next IRI and key from the context
      const char* ctxIri = i->next();
      const char* key = i->getName();

      // skip special context keys (start with '@')
      if(key[0] != '@')
      {
         // compact to a term
         if(strcmp(iri, ctxIri) == 0)
         {
            rval = key;
            if(usedCtx != NULL)
            {
               (*usedCtx)[key] = ctxIri;
            }
         }
      }
   }

   // if term not found, check the context for a CURIE prefix
   i = ctx.getIterator();
   while(rval.empty() && i->hasNext())
   {
      // get next IRI and key from the context
      const char* ctxIri = i->next();
      const char* key = i->getName();

      // skip special context keys (start with '@')
      if(key[0] != '@')
      {
         // see if IRI begins with the next IRI from the context
         const char* ptr = strstr(iri, ctxIri);
         if(ptr != NULL && ptr == iri)
         {
            size_t len1 = strlen(iri);
            size_t len2 = strlen(ctxIri);

            // compact to a CURIE
            if(len1 > len2)
            {
               // add 2 to make room for null-terminator and colon
               rval = StringTools::format("%s:%s", key, ptr + len2);
               if(usedCtx != NULL)
               {
                  (*usedCtx)[key] = ctxIri;
               }
            }
         }
      }
   }

   // could not compact IRI
   if(rval.empty())
   {
      rval = iri;
   }

   return rval;
}

/**
 * Expands a term into an absolute IRI. The term may be a regular term, a
 * CURIE, a relative IRI, or an absolute IRI. In any case, the associated
 * absolute IRI will be returned.
 *
 * @param ctx the context to use.
 * @param term the term to expand.
 * @param usedCtx a context to update if a value was used from "ctx".
 *
 * @return the expanded term as an absolute IRI.
 */
static string _expandTerm(
   DynamicObject& ctx, const char* term, DynamicObject* usedCtx)
{
   string rval;

   // 1. If the property has a colon, then it is a CURIE or an absolute IRI:
   const char* ptr = strchr(term, ':');
   if(ptr != NULL)
   {
      // get the potential CURIE prefix
      size_t len = ptr - term + 1;
      char prefix[len];
      snprintf(prefix, len, "%s", term);

      // 1.1. See if the prefix is in the context:
      if(ctx->hasMember(prefix))
      {
         // prefix found, expand property to absolute IRI
         DynamicObject& iri = ctx[prefix];
         len = strlen(iri->getString()) + strlen(ptr + 1) + 3;
         rval = StringTools::format("%s%s", iri->getString(), ptr + 1);
         if(usedCtx != NULL)
         {
            (*usedCtx)[prefix] = iri->getString();
         }
      }
      // 1.2. Prefix is not in context, property is already an absolute IRI:
      else
      {
         rval = term;
      }
   }
   // 2. If the property is in the context, then it's a term.
   else if(ctx->hasMember(term))
   {
      rval = ctx[term]->getString();
      if(usedCtx != NULL)
      {
         (*usedCtx)[term] = rval.c_str();
      }
   }
   // 3. The property is the special-case '@'.
   else if(strcmp(term, "@") == 0)
   {
      rval = "@";
   }
   // 4. The property is a relative IRI, prepend the default vocab.
   else
   {
      rval = StringTools::format("%s%s", ctx["@vocab"]->getString(), term);
      if(usedCtx != NULL)
      {
         (*usedCtx)["@vocab"] = ctx["@vocab"]->getString();
      }
   }

   return rval;
}

/**
 * Sets a subject's property to the given object value. If a value already
 * exists, it will be appended to an array.
 *
 * @param s the subject.
 * @param p the property.
 * @param o the object.
 */
static void _setProperty(DynamicObject s, const char* p, DynamicObject o)
{
   if(s->hasMember(p))
   {
      s[p].push(o);
   }
   else
   {
      s[p] = o;
   }
}

/**
 * Gets the coerce type for the given property.
 *
 * @param ctx the context to use.
 * @param property the property to get the coerced type for.
 * @param usedCtx a context to update if a value was used from "ctx".
 *
 * @return the coerce type, NULL for none.
 */
static DynamicObject _getCoerceType(
   DynamicObject& ctx, const char* property, DynamicObject* usedCtx)
{
   DynamicObject rval(NULL);

   // get expanded property
   string prop = _expandTerm(ctx, property, NULL);
   const char* p = prop.c_str();

   // built-in type coercion JSON-LD-isms
   if(strcmp(p, "@") == 0 || strcmp(p, RDF_TYPE) == 0)
   {
      rval = DynamicObject();
      rval = XSD_ANY_URI;
   }
   // check type coercion for property
   else
   {
      // force compacted property
      prop = _compactIri(ctx, p, NULL);
      p = prop.c_str();

      DynamicObjectIterator i = ctx["@coerce"].getIterator();
      while(rval.isNull() && i->hasNext())
      {
         DynamicObject& props = i->next();
         DynamicObjectIterator pi = props.getIterator();
         while(rval.isNull() && pi->hasNext())
         {
            if(pi->next() == p)
            {
               rval = DynamicObject();
               rval = _expandTerm(ctx, i->getName(), usedCtx).c_str();
               if(usedCtx != NULL)
               {
                  if(!(*usedCtx)["@coerce"]->hasMember(i->getName()))
                  {
                     (*usedCtx)["@coerce"][i->getName()] = p;
                  }
                  else
                  {
                     DynamicObject& c = (*usedCtx)["@coerce"][i->getName()];
                     if((c->getType() == Array && c->indexOf(p) == -1) ||
                        (c->getType() == String && c != p))
                     {
                        c.push(p);
                     }
                  }
               }
            }
         }
      }
   }

   return rval;
}

/**
 * Recursively compacts a value. This method will compact IRIs to CURIEs or
 * terms and do reverse type coercion to compact a value.
 *
 * @param ctx the context to use.
 * @param property the property that points to the value, NULL for none.
 * @param value the value to compact.
 * @param usedCtx a context to update if a value was used from "ctx".
 *
 * @return the compacted value, NULL on error.
 */
static DynamicObject _compact(
   DynamicObject ctx, const char* property, DynamicObject& value,
   DynamicObject* usedCtx)
{
   DynamicObject rval(NULL);

   if(value.isNull())
   {
      rval.setNull();
   }
   else if(value->getType() == Array)
   {
      // recursively add compacted values to array
      rval = DynamicObject(Array);
      DynamicObjectIterator i = value.getIterator();
      while(!rval.isNull() && i->hasNext())
      {
         DynamicObject next = _compact(ctx, property, i->next(), usedCtx);
         if(next.isNull())
         {
            // error
            rval.setNull();
         }
         else
         {
            rval->append(next);
         }
      }
   }
   // graph literal/disjoint graph
   else if(
      value->getType() == Map &&
      value->hasMember("@") &&
      value["@"]->getType() == Array)
   {
      rval = DynamicObject();
      rval["@"] = _compact(ctx, property, value["@"], usedCtx);
   }
   // value has sub-properties if it doesn't define a literal or IRI value
   else if(
      value->getType() == Map &&
      !value->hasMember("@literal") &&
      !value->hasMember("@iri"))
   {
      // recursively handle sub-properties that aren't a sub-context
      rval = DynamicObject(Map);
      DynamicObjectIterator i = value.getIterator();
      while(!rval.isNull() && i->hasNext())
      {
         DynamicObject next = i->next();
         if(strcmp(i->getName(), "@context") != 0)
         {
            next = _compact(ctx, i->getName(), next, usedCtx);
            if(next.isNull())
            {
               // error
               rval.setNull();
            }
            else
            {
               // set object to compacted property
               _setProperty(
                  rval, _compactIri(ctx, i->getName(), usedCtx).c_str(),
                  next);
            }
         }
      }
   }
   else
   {
      // get coerce type
      DynamicObject coerce = _getCoerceType(ctx, property, usedCtx);

      // get type from value, to ensure coercion is valid
      DynamicObject type(NULL);
      if(value->getType() == Map)
      {
         // type coercion can only occur if language is not specified
         if(!value->hasMember("@language"))
         {
            type = DynamicObject();

            // datatype must match coerce type if specified
            if(value->hasMember("@datatype"))
            {
               type = value["@datatype"];
            }
            // datatype is IRI
            else if(value->hasMember("@iri"))
            {
               type = XSD_ANY_URI;
            }
            // can be coerced to any type
            else
            {
               type = coerce;
            }
         }
      }
      // type can be coerced to anything
      else if(value->getType() == String)
      {
         type = coerce;
      }

      // types that can be auto-coerced from a JSON-builtin
      if(coerce.isNull() &&
         (type == XSD_BOOLEAN || type == XSD_INTEGER || type == XSD_DOUBLE))
      {
         coerce = type;
      }

      // do reverse type-coercion
      if(!coerce.isNull())
      {
         // type is only null if a language was specified, which is an error
         // if type coercion is specified
         if(type.isNull())
         {
            ExceptionRef e = new Exception(
               "Cannot coerce type when a language is specified. The language "
               "information would be lost.",
               EXCEPTION_TYPE ".CoerceLanguageError");
            Exception::set(e);
         }
         // if the value type does not match the coerce type, it is an error
         else if(type != coerce)
         {
            ExceptionRef e = new Exception(
               "Cannot coerce type because the datatype does not match.",
               EXCEPTION_TYPE ".InvalidCoerceType");
            Exception::set(e);
         }
         // do reverse type-coercion
         else
         {
            rval = DynamicObject();
            if(value->getType() == Map)
            {
               if(value->hasMember("@iri"))
               {
                  rval = value["@iri"]->getString();
               }
               else if(value->hasMember("@literal"))
               {
                  rval = value["@literal"].clone();
               }
            }
            else
            {
               rval = value.clone();
            }

            // do basic JSON types conversion
            if(coerce == XSD_BOOLEAN)
            {
               rval->setType(Boolean);
            }
            else if(coerce == XSD_DOUBLE)
            {
               rval->setType(Double);
            }
            else if(coerce == XSD_INTEGER)
            {
               rval->setType(Int64);
            }
         }
      }
      // no type-coercion, just copy value
      else
      {
         rval = value.clone();
      }

      // compact IRI
      if(!rval.isNull() && type == XSD_ANY_URI)
      {
         if(rval->getType() == Map)
         {
            rval["@iri"] = _compactIri(ctx, rval["@iri"], usedCtx).c_str();
         }
         else
         {
            rval = _compactIri(ctx, rval, usedCtx).c_str();
         }
      }
   }

   return rval;
}

/**
 * Recursively expands a value using the given context. Any context in
 * the value will be removed.
 *
 * @param ctx the context.
 * @param property the property that points to the value, NULL for none.
 * @param value the value to expand.
 * @param expandSubjects true to expand subjects (normalize), false not to.
 *
 * @return the expanded value, NULL on error.
 */
static DynamicObject _expand(
   DynamicObject ctx, const char* property, DynamicObject& value,
   bool expandSubjects)
{
   DynamicObject rval(NULL);

   // TODO: add data format error detection?

   // if no property is specified and the value is a string (this means the
   // value is a property itself), expand to an IRI
   if(property == NULL && value->getType() == String)
   {
      rval = DynamicObject();
      rval = _expandTerm(ctx, value, NULL).c_str();
   }
   else if(value->getType() == Array)
   {
      // recursively add expanded values to array
      rval = DynamicObject(Array);
      DynamicObjectIterator i = value.getIterator();
      while(!rval.isNull() && i->hasNext())
      {
         DynamicObject next = _expand(ctx, property, i->next(), expandSubjects);
         if(next.isNull())
         {
            // error
            rval.setNull();
         }
         else
         {
            rval->append(next);
         }
      }
   }
   else if(value->getType() == Map)
   {
      // value has sub-properties if it doesn't define a literal or IRI value
      if(!value->hasMember("@literal") && !value->hasMember("@iri"))
      {
         // if value has a context, use it
         if(value->hasMember("@context"))
         {
            ctx = JsonLd::mergeContexts(ctx, value["@context"]);
         }

         if(!ctx.isNull())
         {
            // recursively handle sub-properties that aren't a sub-context
            rval = DynamicObject(Map);
            DynamicObjectIterator i = value.getIterator();
            while(!rval.isNull() && i->hasNext())
            {
               DynamicObject obj = i->next();
               if(i->getName()[0] != '@')
               {
                  // expand property
                  string p = _expandTerm(ctx, i->getName(), NULL);

                  // expand object
                  obj = _expand(ctx, p.c_str(), obj, expandSubjects);
                  if(obj.isNull())
                  {
                     // error
                     rval.setNull();
                  }
                  else
                  {
                     // set object to expanded property
                     _setProperty(rval, p.c_str(), obj);
                  }
               }
               else if(strcmp(i->getName(), "@context") != 0)
               {
                  // preserve non-context json-ld keywords
                  _setProperty(rval, i->getName(), obj.clone());
               }
            }
         }
      }
      // value is already expanded
      else
      {
         rval = value.clone();
      }
   }
   else
   {
      rval = DynamicObject();

      // do type coercion
      DynamicObject coerce = _getCoerceType(ctx, property, NULL);

      // automatic coercion for basic JSON types
      if(coerce.isNull() && (value->isNumber() || value->getType() == Boolean))
      {
         coerce = DynamicObject();
         if(value->getType() == Boolean)
         {
            coerce = XSD_BOOLEAN;
         }
         else if(value->isInteger())
         {
            coerce = XSD_INTEGER;
         }
         else
         {
            coerce = XSD_DOUBLE;
         }
      }

      // only expand subjects if requested
      if(!coerce.isNull() && (strcmp(property, "@") != 0 || expandSubjects))
      {
         // expand IRI
         if(coerce == XSD_ANY_URI)
         {
            rval["@iri"] = _expandTerm(ctx, value, NULL).c_str();
         }
         // other datatype
         else
         {
            // do special JSON-LD double format
            rval["@literal"] = StringTools::format(
               "%1.6e", value->getDouble()).c_str();
            rval["@datatype"] = coerce;
         }
      }
      // nothing to coerce
      else
      {
         rval = value->getString();
      }
   }

   return rval;
}

inline static bool _isBlankNodeIri(const char* v)
{
   return strstr(v, "_:") == v;
}

inline static bool _isNamedBlankNode(DynamicObject& v)
{
   // look for "_:" at the beginning of the subject
   return (
      v->getType() == Map &&
      v->hasMember("@") &&
      v["@"]->hasMember("@iri") &&
      _isBlankNodeIri(v["@"]["@iri"]));
}

inline static bool _isBlankNode(DynamicObject& v)
{
   // look for no subject or named blank node
   return (
      v->getType() == Map &&
      !(v->hasMember("@iri") || v->hasMember("@literal")) &&
      (!v->hasMember("@") || _isNamedBlankNode(v)));
}

static bool _isBlankNodeObject(DynamicObject& v)
{
   return (
      v->getType() == Map &&
      v->hasMember("@iri") &&
      _isBlankNodeIri(v["@iri"]));
}

/**
 * Compares two values.
 *
 * @param v1 the first value.
 * @param v2 the second value.
 *
 * @return -1 if v1 < v2, 0 if v1 == v2, 1 if v1 > v2.
 */
static int _compare(DynamicObject v1, DynamicObject v2)
{
   int rval = 0;

   if(v1->getType() == Array && v2->getType() == Array)
   {
      for(int i = 0; i < v1->length() && rval == 0; ++i)
      {
         rval = _compare(v1[i], v2[i]);
      }
   }
   else
   {
      rval = (v1 < v2 ? -1 : (v1 > v2 ? 1 : 0));
   }

   return rval;
};

/**
 * Compares two values.
 *
 * @param v1 the first value.
 * @param v2 the second value.
 *
 * @return -1 if v1 < v2, 0 if v1 == v2, 1 if v1 > v2.
 */
static int _compare(int v1, int v2)
{
   return (v1 < v2 ? -1 : (v1 > v2 ? 1 : 0));
}

/**
 * Compares two keys in an object. If the key exists in one object
 * and not the other, that object is less. If the key exists in both objects,
 * then the one with the lesser value is less.
 *
 * @param o1 the first object.
 * @param o2 the second object.
 * @param key the key.
 *
 * @return -1 if o1 < o2, 0 if o1 == o2, 1 if o1 > o2.
 */
static int _compareObjectKeys(
   DynamicObject& o1, DynamicObject& o2, const char* key)
{
   int rval = 0;
   if(o1->hasMember(key))
   {
      if(o2->hasMember(key))
      {
         rval = _compare(o1[key], o2[key]);
      }
      else
      {
         rval = -1;
      }
   }
   else if(o2->hasMember(key))
   {
      rval = 1;
   }
   return rval;
};

/**
 * Compares two object values.
 *
 * @param o1 the first object.
 * @param o2 the second object.
 *
 * @return -1 if o1 < o2, 0 if o1 == o2, 1 if o1 > o2.
 */
static int _compareObjects(DynamicObject& o1, DynamicObject& o2)
{
   int rval = 0;

   if(o1->getType() == String)
   {
      if(o2->getType() != String)
      {
         rval = -1;
      }
      else
      {
         rval = _compare(o1, o2);
      }
   }
   else if(o2->getType() == String)
   {
      rval = 1;
   }
   else
   {
      rval = _compareObjectKeys(o1, o2, "@literal");
      if(rval == 0)
      {
         if(o1->hasMember("@literal"))
         {
            rval = _compareObjectKeys(o1, o2, "@datatype");
            if(rval == 0)
            {
               rval = _compareObjectKeys(o1, o2, "@language");
            }
         }
         // both are "@iri" objects
         else
         {
            rval = _compare(o1["@iri"], o2["@iri"]);
         }
      }
   }

   return rval;
};

/**
 * Sort function for comparing two objects.
 *
 * @param o1 the first object.
 * @param o2 the second object.
 *
 * @return true if o1 < o2, false if not.
 */
static bool _sortObjects(DynamicObject o1, DynamicObject o2)
{
   return _compareObjects(o1, o2) == -1;
}

/**
 * Compares the object values between two bnodes.
 *
 * @param a the first bnode.
 * @param b the second bnode.
 *
 * @return -1 if a < b, 0 if a == b, 1 if a > b.
 */
static int _compareBlankNodeObjects(DynamicObject& a, DynamicObject& b)
{
   int rval = 0;

   /*
   3. For each property, compare sorted object values.
   3.1. The bnode with fewer objects is first.
   3.2. For each object value, compare only literals and non-bnodes.
   3.2.1.  The bnode with fewer non-bnodes is first.
   3.2.2. The bnode with a string object is first.
   3.2.3. The bnode with the alphabetically-first string is first.
   3.2.4. The bnode with a @literal is first.
   3.2.5. The bnode with the alphabetically-first @literal is first.
   3.2.6. The bnode with the alphabetically-first @datatype is first.
   3.2.7. The bnode with a @language is first.
   3.2.8. The bnode with the alphabetically-first @language is first.
   3.2.9. The bnode with the alphabetically-first @iri is first.
   */

   DynamicObjectIterator i = a.getIterator();
   while(i->hasNext() && rval == 0)
   {
      const char* p = i->next();

      // step #3.1
      int lenA = (a[p]->getType() == Array) ? a[p]->length() : 1;
      int lenB = (b[p]->getType() == Array) ? b[p]->length() : 1;
      rval = _compare(lenA, lenB);

      // step #3.2.1
      if(rval == 0)
      {
         // normalize objects to an array
         DynamicObject objsA = a[p];
         DynamicObject objsB = b[p];
         if(objsA->getType() != Array)
         {
            DynamicObject tmp = objsA;
            objsA = DynamicObject();
            objsA.push(tmp);

            tmp = objsB;
            objsB = DynamicObject();
            objsB.push(tmp);
         }

         // filter non-bnodes (remove bnodes from comparison)
         objsA = objsA.filter(&_isBlankNodeObject);
         objsB = objsB.filter(&_isBlankNodeObject);
         rval = _compare(objsA->length(), objsB->length());

         // steps #3.2.2-3.2.9
         if(rval == 0)
         {
            for(int i = 0; i < objsA->length() && rval == 0; ++i)
            {
               rval = _compareObjects(objsA[i], objsB[i]);
            }
         }
      }
   }

   return rval;
};

/**
 * Filter for duplicate IRIs.
 */
struct FilterDuplicateIris : public DynamicObject::FilterFunctor
{
   const char* iri;
   FilterDuplicateIris(const char* iri)
   {
      this->iri = iri;
   };
   bool operator()(const DynamicObject& d) const
   {
      return (d->getType() == Map && d->hasMember("@iri") && d["@iri"] == iri);
   }
};

/**
 * Flattens the given value into a map of unique subjects. It is assumed that
 * all blank nodes have been uniquely named before this call. Array values for
 * properties will be sorted.
 *
 * @param parent the value's parent, NULL for none.
 * @param parentProperty the property relating the value to the parent.
 * @param value the value to flatten.
 * @param subjects the map of subjects to write to.
 *
 * @return true on success, false on failure with exception set.
 */
static bool _flatten(
   DynamicObject* parent, const char* parentProperty,
   DynamicObject& value, DynamicObject& subjects)
{
   bool rval = true;

   DynamicObject flattened(NULL);
   if(value->getType() == Array)
   {
      DynamicObjectIterator i = value.getIterator();
      while(rval && i->hasNext())
      {
         rval = _flatten(parent, parentProperty, i->next(), subjects);
      }

      // if value is a list of objects, sort them
      if(value->length() > 0 &&
         (value[0]->getType() == String ||
         (value[0]->getType() == Map &&
         (value[0]->hasMember("@literal") || value[0]->hasMember("@iri")))))
      {
         // sort values
         value.sort(_sortObjects);
      }
   }
   else if(value->getType() == Map)
   {
      // graph literal/disjoint graph
      if(value->hasMember("@") && value["@"]->getType() == Array)
      {
         // cannot flatten embedded graph literals
         if(parent != NULL)
         {
            ExceptionRef e = new Exception(
               "Embedded graph literals cannot be flattened.",
               EXCEPTION_TYPE ".GraphLiteralFlattenError");
            Exception::set(e);
            rval = false;
         }
         // top-level graph literal
         else
         {
            DynamicObjectIterator i = value["@"].getIterator();
            while(rval && i->hasNext())
            {
               rval = _flatten(parent, parentProperty, i->next(), subjects);
            }
         }
      }
      // already-expanded value
      else if(value->hasMember("@literal") || value->hasMember("@iri"))
      {
         flattened = value.clone();
      }
      // subject
      else
      {
         // create or fetch existing subject
         DynamicObject subject(NULL);
         if(value->hasMember("@") && subjects->hasMember(value["@"]))
         {
            // FIXME: "@" might be a graph literal (as {})
            subject = subjects[value["@"]["@iri"]->getString()];
         }
         else
         {
            subject = DynamicObject(Map);
            if(value->hasMember("@"))
            {
               // FIXME: "@" might be a graph literal (as {})
               subjects[value["@"]["@iri"]->getString()] = subject;
            }
         }
         flattened = subject;

         // flatten embeds
         DynamicObjectIterator i = value.getIterator();
         while(rval && i->hasNext())
         {
            DynamicObject& next = i->next();
            const char* key = i->getName();
            if(next->getType() == Array)
            {
               subject[key]->setType(Array);
               rval = _flatten(&subject[key], NULL, next, subjects);
               if(rval && subject[key]->length() == 1)
               {
                  // convert subject[key] to object if only 1 value was added
                  subject[key] = subject[key][0];
               }
            }
            else
            {
               rval = _flatten(&subject, key, next, subjects);
            }
         }
      }
   }
   // string value
   else
   {
      flattened = value.clone();
      flattened->setType(String);
   }

   // add flattened value to parent
   if(rval && !flattened.isNull() && parent != NULL)
   {
      // remove top-level '@' for subjects
      // 'http://mypredicate': {'@': {'@iri': 'http://mysubject'}} becomes
      // 'http://mypredicate': {'@iri': 'http://mysubject'}
      if(flattened->getType() == Map && flattened->hasMember("@"))
      {
         flattened = flattened["@"];
      }

      if((*parent)->getType() == Array)
      {
         // do not add duplicate IRIs for the same property
         bool duplicate = false;
         if(flattened->getType() == Map && flattened->hasMember("@iri"))
         {
            FilterDuplicateIris filter(flattened["@iri"]);
            duplicate = parent->filter(filter)->length() > 0;
         }
         if(!duplicate)
         {
            (*parent).push(flattened);
         }
      }
      else
      {
         (*parent)[parentProperty] = flattened;
      }
   }

   return rval;
}


/**
 * A blank node name generator using the given prefix for the blank nodes.
 *
 * @param prefix the prefix to use.
 *
 * @return the blank node name generator.
 */
struct NameGenerator
{
   int count;
   string base;
   string name;
   NameGenerator(const char* prefix)
   {
      this->base = StringTools::format("_:%s", prefix);
   };
   const char* next()
   {
      name = StringTools::format("%s%d", base.c_str(), ++count);
      return current();
   };
   const char* current()
   {
      return name.c_str();
   };
   bool inNamespace(const char* iri)
   {
      return strstr(iri, base.c_str()) == iri;
   };
};

/**
 * Blank node canonicalization state.
 */
struct C14NState
{
   DynamicObject memo;
   DynamicObject edges;
   DynamicObject subjects;
   NameGenerator ng;
   C14NState() :
      memo(Map),
      edges(Map),
      subjects(Map),
      ng("tmp")
   {
      this->edges["refs"]->setType(Map);
      this->edges["props"]->setType(Map);
   };
};

/**
 * Populates a map of all named subjects from the given input and an array
 * of all unnamed bnodes (includes embedded ones).
 *
 * @param input the input (must be expanded, no context).
 * @param subjects the subjects map to populate.
 */
static void _collectSubjects(
   DynamicObject& input, DynamicObject& subjects, DynamicObject& bnodes)
{
   if(input->getType() == Array)
   {
      DynamicObjectIterator i = input.getIterator();
      while(i->hasNext())
      {
         _collectSubjects(i->next(), subjects, bnodes);
      }
   }
   else if(input->getType() == Map)
   {
      // named subject
      if(input->hasMember("@"))
      {
         subjects[input["@"]["@iri"]->getString()] = input;
      }
      // unnamed blank node
      else if(_isBlankNode(input))
      {
         bnodes.push(input);
      }

      // recurse through subject properties
      DynamicObjectIterator i = input.getIterator();
      while(i->hasNext())
      {
         _collectSubjects(i->next(), subjects, bnodes);
      }
   }
};

/**
 * Assigns unique names to blank nodes that are unnamed in the given input.
 *
 * @param state canonicalization state.
 * @param input the input to assign names to.
 */
static void _nameBlankNodes(C14NState& state, DynamicObject& input)
{
   // collect subjects and unnamed bnodes
   DynamicObject subjects(Map);
   DynamicObject bnodes(Array);
   _collectSubjects(input, subjects, bnodes);

   // uniquely name all unnamed bnodes
   DynamicObjectIterator i = bnodes.getIterator();
   while(i->hasNext())
   {
      DynamicObject& bnode = i->next();
      if(!bnode->hasMember("@"))
      {
         // generate names until one is unique
         while(subjects->hasMember(state.ng.next()));
         bnode["@"]["@iri"] = state.ng.current();
         subjects[state.ng.current()] = bnode;
      }
   }
};

/**
 * Renames a blank node, changing its references, etc. The method assumes
 * that the given name is unique.
 *
 * @param state canonicalization state.
 * @param b the blank node to rename.
 * @param id the new name to use.
 */
static void _renameBlankNode(
   C14NState& state, DynamicObject& b, const char* id)
{
   const char* old = b["@"]["@iri"];

   // update subjects map
   DynamicObject& subjects = state.subjects;
   subjects[id] = subjects[old];
   subjects->removeMember(old);

   // update reference and property lists
   DynamicObject& edges = state.edges;
   edges["refs"][id] = edges["refs"][old];
   edges["props"][id] = edges["props"][old];
   edges["refs"]->removeMember(old);
   edges["props"]->removeMember(old);

   // update references to this bnode
   DynamicObject& refs = edges["refs"][id]["all"];
   DynamicObjectIterator i1 = refs.getIterator();
   while(i1->hasNext())
   {
      const char* iri = i1->next()["s"];
      if(strcmp(iri, old) == 0)
      {
         iri = id;
      }
      DynamicObject& ref = subjects[iri];
      DynamicObject& props = state.edges["props"][iri]["all"];
      DynamicObjectIterator i2 = props.getIterator();
      while(i2->hasNext())
      {
         DynamicObject& prop = i2->next();
         if(prop["s"] == old)
         {
            prop["s"] = id;

            // normalize property to array for single code-path
            const char* p = prop["p"];
            DynamicObject tmp(NULL);
            if(ref[p]->getType() == Map)
            {
               tmp = DynamicObject();
               tmp.push(ref[p]);
            }
            else if(ref[p]->getType() == Array)
            {
               tmp = ref[p];
            }
            else
            {
               tmp = DynamicObject(Array);
            }
            DynamicObjectIterator i3 = tmp.getIterator();
            while(i3->hasNext())
            {
               DynamicObject& next = i3->next();
               if(next->getType() == Map &&
                  next->hasMember("@iri") && next["@iri"] == old)
               {
                  next["@iri"] = id;
               }
            }
         }
      }
   }

   // update references from this bnode
   DynamicObject& props = state.edges["props"][id]["all"];
   DynamicObjectIterator i = props.getIterator();
   while(i->hasNext())
   {
      DynamicObject& p = i->next();
      const char* iri = p["s"];
      DynamicObjectIterator ri = state.edges["refs"][iri]["all"].getIterator();
      while(ri->hasNext())
      {
         DynamicObject& ref = ri->next();
         if(ref["s"] == old)
         {
            ref["s"] = id;
         }
      }
   }

   // update bnode IRI
   b["@"]["@iri"] = id;
};

/**
 * Compares two edges. Edges with an IRI (vs. a bnode ID) come first, then
 * alphabetically-first IRIs, then alphabetically-first properties. If a blank
 * node appears in the blank node equality memo then they will be compared
 * after properties, otherwise they won't be.
 *
 * @param state the canonicalization state.
 * @param a the first edge.
 * @param b the second edge.
 *
 * @return -1 if a < b, 0 if a == b, 1 if a > b.
 */
static int _compareEdges(C14NState& state, DynamicObject& a, DynamicObject& b)
{
   int rval = 0;

   bool bnodeA = _isBlankNodeIri(a["s"]);
   bool bnodeB = _isBlankNodeIri(b["s"]);
   DynamicObject& memo = state.memo;

   if((bnodeA ^ bnodeB) == 1)
   {
      rval = bnodeA ? 1 : -1;
   }
   else
   {
      if(!bnodeA)
      {
         rval = _compare(a["s"], b["s"]);
      }
      if(rval == 0)
      {
         rval = _compare(a["p"], b["p"]);
      }
      if(rval == 0 && bnodeA &&
         memo->hasMember(a["s"]) &&
         memo[a["s"]->getString()]->hasMember(b["s"]))
      {
         rval = memo[a["s"]->getString()][b["s"]->getString()];
      }
   }

   return rval;
};

/**
 * Comparator for comparing edges during sorting.
 */
struct CompareEdges : public std::less<DynamicObject>
{
   C14NState* state;
   CompareEdges(C14NState* state)
   {
      this->state = state;
   };
   bool operator()(DynamicObject& a, DynamicObject& b)
   {
      return _compareEdges(*state, a, b) == -1;
   };
};

// prototypes for recursively used functions
static int _compareEdgeType(
   C14NState& state, DynamicObject& a, DynamicObject& b,
   const char* p, const char* dir, DynamicObject& iso);
static int _deepCompareEdges(
   C14NState& state, DynamicObject& a, DynamicObject& b,
   const char* dir, DynamicObject& iso);

/**
 * Deeply names the given blank node by first naming it if it doesn't already
 * have an appropriate prefix, and then by naming its properties and then
 * references.
 *
 * @param state the canonicalization state.
 * @param b the bnode to name.
 */
static void _deepNameBlankNode(C14NState& state, DynamicObject& b)
{
   // rename bnode (if not already renamed)
   const char* iri = b["@"]["@iri"];
   if(!state.ng.inNamespace(iri))
   {
      _renameBlankNode(state, b, state.ng.next());
      iri = state.ng.current();

      DynamicObject& subjects = state.subjects;

      // FIXME: can bnode edge sorting be optimized out due to sorting them
      // when they are unequal in other parts of this algorithm?
      CompareEdges sorter(&state);

      // rename bnode properties
      DynamicObject& props = state.edges["props"][iri]["bnodes"];
      props.sort(sorter);
      DynamicObjectIterator i = props.getIterator();
      while(i->hasNext())
      {
         DynamicObject& next = i->next();
         if(subjects->hasMember(next["s"]))
         {
            _deepNameBlankNode(state, subjects[next["s"]->getString()]);
         }
      }

      // rename bnode references
      DynamicObject& refs = state.edges["refs"][iri]["bnodes"];
      refs.sort(sorter);
      i = refs.getIterator();
      while(i->hasNext())
      {
         DynamicObject& next = i->next();
         if(subjects->hasMember(next["s"]))
         {
            _deepNameBlankNode(state, subjects[next["s"]->getString()]);
         }
      }
   }
};

/**
 * Performs a shallow sort comparison on the given bnodes.
 *
 * @param state the canonicalization state.
 * @param a the first bnode.
 * @param b the second bnode.
 *
 * @return -1 if a < b, 0 if a == b, 1 if a > b.
 */
static int _shallowCompareBlankNodes(
   C14NState& state, DynamicObject& a, DynamicObject& b)
{
   int rval = 0;

   /* ShallowSort Algorithm (when comparing two bnodes):
      1. Compare the number of properties.
      1.1. The bnode with fewer properties is first.
      2. Compare alphabetically sorted-properties.
      2.1. The bnode with the alphabetically-first property is first.
      3. For each property, compare object values.
      4. Compare the number of references.
      4.1. The bnode with fewer references is first.
      5. Compare sorted references.
      5.1. The bnode with the reference iri (vs. bnode) is first.
      5.2. The bnode with the alphabetically-first reference iri is first.
      5.3. The bnode with the alphabetically-first reference property is first.
    */
   DynamicObject pA = a.keys();
   DynamicObject pB = b.keys();

   // step #1
   rval = _compare(pA->length(), pB->length());

   // step #2
   if(rval == 0)
   {
      rval = _compare(pA.sort(), pB.sort());
   }

   // step #3
   if(rval == 0)
   {
      rval = _compareBlankNodeObjects(a, b);
   }

   if(rval == 0)
   {
      const char* iriA = a["@"]["@iri"];
      const char* iriB = b["@"]["@iri"];
      DynamicObject& edgesA = state.edges["refs"][iriA]["all"];
      DynamicObject& edgesB = state.edges["refs"][iriB]["all"];

      // step #4
      rval = _compare(edgesA->length(), edgesB->length());

      // step #5
      if(rval == 0)
      {
         for(int i = 0; i < edgesA->length() && rval == 0; ++i)
         {
            rval = _compareEdges(state, edgesA[i], edgesB[i]);
         }
      }
   }

   return rval;
};

/**
 * Compares two blank nodes for equivalence.
 *
 * @param state the canonicalization state.
 * @param a the first blank node.
 * @param b the second blank node.
 * @param iso the current subgraph isomorphism for connected bnodes.
 *
 * @return -1 if a < b, 0 if a == b, 1 if a > b.
 */
static int _deepCompareBlankNodes(
   C14NState& state, DynamicObject& a, DynamicObject& b, DynamicObject& iso)
{
   int rval = 0;

   // use memoized comparison if available
   const char* iriA = a["@"]["@iri"];
   const char* iriB = b["@"]["@iri"];
   if(state.memo[iriA]->hasMember(iriB))
   {
      rval = state.memo[iriA][iriB];
   }
   else
   {
      // do shallow compare first
      rval = _shallowCompareBlankNodes(state, a, b);
      if(rval != 0)
      {
         // compare done
         state.memo[iriA][iriB] = rval;
         state.memo[iriB][iriA] = -rval;
      }
      // deep comparison is necessary
      else
      {
         // compare properties
         rval = _deepCompareEdges(state, a, b, "props", iso);

         // compare references
         if(rval == 0)
         {
            rval = _deepCompareEdges(state, a, b, "refs", iso);
         }

         // update memo
         if(!state.memo[iriA]->hasMember(iriB))
         {
            state.memo[iriA][iriB] = rval;
            state.memo[iriB][iriA] = -rval;
         }
      }
   }

   return rval;
};

/**
 * Comparator for deeply-sorting blank nodes.
 */
struct DeepCompareBlankNodes : public std::less<DynamicObject>
{
   C14NState* state;
   DynamicObject iso;
   DeepCompareBlankNodes(C14NState* state, DynamicObject iso)
   {
      this->state = state;
      this->iso = iso;
   };
   bool operator()(DynamicObject& a, DynamicObject& b)
   {
      return _deepCompareBlankNodes(*state, a, b, iso) == -1;
   };
};

/**
 * A filter for blank node edges.
 *
 * @param e the edge to filter on.
 *
 * @return true if the edge is a blank node IRI.
 */
static bool _filterBlankNodeEdge(DynamicObject& e)
{
   return _isBlankNodeIri(e["s"]);
}

/**
 * Populates the given reference map with all of the subject edges in the
 * graph. The references will be categorized by the direction of the edges,
 * where 'props' is for properties and 'refs' is for references to a subject as
 * an object. The edge direction categories for each IRI will be sorted into
 * groups 'all' and 'bnodes'.
 */
static void _collectEdges(C14NState& state)
{
   DynamicObject& refs = state.edges["refs"];
   DynamicObject& props = state.edges["props"];

   // collect all references and properties
   DynamicObjectIterator i = state.subjects.getIterator();
   while(i->hasNext())
   {
      DynamicObject& subject = i->next();
      const char* iri = i->getName();

      DynamicObjectIterator oi = subject.getIterator();
      while(oi->hasNext())
      {
         DynamicObject& object = oi->next();
         const char* key = oi->getName();
         if(strcmp(key, "@") != 0)
         {
            // normalize to array for single codepath
            DynamicObject tmp(NULL);
            if(object->getType() != Array)
            {
               tmp = DynamicObject(Array);
               tmp.push(object);
            }
            else
            {
               tmp = object;
            }

            DynamicObjectIterator ti = tmp.getIterator();
            while(ti->hasNext())
            {
               DynamicObject& o = ti->next();
               if(o->getType() == Map && o->hasMember("@iri") &&
                  state.subjects->hasMember(o["@iri"]->getString()))
               {
                  const char* objIri = o["@iri"];

                  // map object to this subject
                  DynamicObject e1;
                  e1["s"] = iri;
                  e1["p"] = key;
                  refs[objIri]["all"].push(e1);

                  // map this subject to object
                  DynamicObject e2;
                  e2["s"] = objIri;
                  e2["p"] = key;
                  props[iri]["all"].push(e2);
               }
            }
         }
      }
   }

   // create sorted categories
   CompareEdges sorter(&state);
   i = state.edges.getIterator();
   while(i->hasNext())
   {
      DynamicObjectIterator ii = i->next().getIterator();
      while(ii->hasNext())
      {
         DynamicObject& next = i->next();
         next["all"].sort(sorter);
         next["bnodes"] = next["all"].filter(_filterBlankNodeEdge);
      }
   }
};

/**
 * Compares the edges between two nodes for equivalence.
 *
 * @param state the canonicalization state.
 * @param a the first bnode.
 * @param b the second bnode.
 * @param dir the edge direction ("props" or "refs").
 * @param iso the current subgraph isomorphism for connected bnodes.
 *
 * @return -1 if a < b, 0 if a == b, 1 if a > b.
 */
static int _deepCompareEdges(
   C14NState& state, DynamicObject& a, DynamicObject& b,
   const char* dir, DynamicObject& iso)
{
   int rval = 0;

   /* Edge comparison algorithm:
      1. Compare adjacent bnode lists for matches.
      1.1. If a bnode ID is in the potential isomorphism, then its associated
         bnode *must* be in the other bnode under the same property.
      1.2. If a bnode ID is not in the potential isomorphism yet, then the
         associated bnode *must* have a bnode with the same property from the
         same bnode group that isn't in the isomorphism yet to match up.
         Iterate over each bnode in the group until an equivalent one is found.
      1.3. Recurse to compare the chosen bnodes.
      1.4. The bnode with lowest group index amongst bnodes with the same
         property name is first.
    */

   // for every bnode edge in A, make sure there's a match in B
   const char* iriA = a["@"]["@iri"];
   const char* iriB = b["@"]["@iri"];
   DynamicObject& edgesA = state.edges[dir][iriA]["bnodes"];
   DynamicObject& edgesB = state.edges[dir][iriB]["bnodes"];
   DynamicObjectIterator ai = edgesA.getIterator();
   while(ai->hasNext() && rval == 0)
   {
      DynamicObject& edgeA = ai->next();
      bool found = false;

      // step #1.1
      if(iso->hasMember(edgeA["s"]))
      {
         const char* match = iso[edgeA["s"]->getString()];
         DynamicObjectIterator bi = edgesB.getIterator();
         for(int bi = 0;
             bi < edgesB->length() && edgesB[bi]["p"] <= edgeA["p"]; ++bi)
         {
            DynamicObject& edgeB = edgesB[bi];
            if(edgeB["p"] == edgeA["p"])
            {
               found = (edgeB["s"] == match);
               break;
            }
         }
      }
      // step #1.2
      else
      {
         for(int bi = 0;
             bi < edgesB->length() && edgesB[bi]["p"] <= edgeA["p"] && !found;
             ++bi)
         {
            DynamicObject& edgeB = edgesB[bi];
            if(edgeB["p"] == edgeA["p"] && !iso->hasMember(edgeB["s"]))
            {
               // add bnode pair temporarily to iso
               iso[edgeA["s"]->getString()] = edgeB["s"];
               iso[edgeB["s"]->getString()] = edgeA["s"];

               // step #1.3
               DynamicObject& sA = state.subjects[edgeA["s"]->getString()];
               DynamicObject& sB = state.subjects[edgeB["s"]->getString()];
               if(_deepCompareBlankNodes(state, sA, sB, iso) == 0)
               {
                  found = true;
               }
               else
               {
                  // remove non-matching bnode pair from iso
                  iso->removeMember(edgeA["s"]);
                  iso->removeMember(edgeB["s"]);
               }
            }
         }
      }

      // step #1.4
      if(!found)
      {
         // no matching bnode pair found, sort order is the bnode with the
         // least bnode for edgeA's property
         rval = _compareEdgeType(state, a, b, edgeA["p"], dir, iso);
      }
   }

   return rval;
};

/**
 * Returns the bnode properties for a particular bnode in sorted order.
 *
 * @param state the canonicalization state.
 * @param b the bnode.
 * @param p the property (edge type).
 * @param direction the direction of the edge ("props" or "refs").
 * @param iso the current subgraph isomorphism for connected bnodes.
 *
 * @return the sorted bnodes for the property.
 */
static DynamicObject _getSortedAdjacents(
   C14NState& state, DynamicObject& b,
   const char* p, const char* dir, DynamicObject& iso)
{
   DynamicObject rval(Array);

   // add all bnodes for the given property
   const char* iri = b["@"]["@iri"];
   DynamicObject& edges = state.edges[dir][iri]["bnodes"];
   for(int i = 0; i < edges->length() && edges[i]["p"] <= p; ++i)
   {
      if(edges[i]["p"] == p)
      {
         rval.push(state.subjects[edges[i]["s"]->getString()]);
      }
   }

   // return sorted bnodes
   DeepCompareBlankNodes sorter(&state, iso);
   rval.sort(sorter);
   return rval;
};

/**
 * Compares bnodes along the same edge type to determine which is less.
 *
 * @param state the canonicalization state.
 * @param a the first bnode.
 * @param b the second bnode.
 * @param p the property.
 * @param dir the direction of the edge ("props" or "refs").
 * @param iso the current subgraph isomorphism for connected bnodes.
 *
 * @return -1 if a < b, 0 if a == b, 1 if a > b.
 */
static int _compareEdgeType(
   C14NState& state, DynamicObject& a, DynamicObject& b,
   const char* p, const char* dir, DynamicObject& iso)
{
   int rval = 0;

   // compare adjacent bnodes for smallest
   DynamicObject adjA = _getSortedAdjacents(state, a, p, dir, iso);
   DynamicObject adjB = _getSortedAdjacents(state, a, p, dir, iso);
   for(int i = 0; i < adjA->length() && rval == 0; ++i)
   {
      rval = _deepCompareBlankNodes(state, adjA[i], adjB[i], iso);
   }

   return rval;
};

/**
 * Canonically names blank nodes in the given input.
 *
 * @param state the canonicalization state.
 * @param input the flat input graph to assign names to.
 */
static void _canonicalizeBlankNodes(C14NState& state, DynamicObject& input)
{
   // collect subjects and bnodes from flat input graph
   DynamicObject& memo = state.memo;
   DynamicObject& subjects = state.subjects;
   DynamicObject& edges = state.edges;
   DynamicObject bnodes(Array);
   DynamicObjectIterator i = input.getIterator();
   while(i->hasNext())
   {
      DynamicObject& next = i->next();
      const char* iri = next["@"]["@iri"];
      subjects[iri] = next;
      edges["refs"][iri]["all"]->setType(Array);
      edges["refs"][iri]["bnodes"]->setType(Array);
      edges["props"][iri]["all"]->setType(Array);
      edges["props"][iri]["bnodes"]->setType(Array);
      if(_isBlankNodeIri(iri))
      {
         bnodes.push(next);
      }
   }

   // build map of memoized bnode comparisons
   i = bnodes.getIterator();
   while(i->hasNext())
   {
      const char* iri = i->next()["@"]["@iri"];
      memo[iri]->setType(Map);
   }

   // collect edges in the graph
   _collectEdges(state);

   // sort blank nodes
   DeepCompareBlankNodes sorter(&state, DynamicObject(Map));
   bnodes.sort(sorter);

   // create canonical blank node name generator
   NameGenerator c14n("c14n");

   // rename all bnodes that have canonical names to temporary names
   i = bnodes.getIterator();
   while(i->hasNext())
   {
      DynamicObject& bnode = i->next();
      if(c14n.inNamespace(bnode["@"]["@iri"]))
      {
         // generate names until one is unique
         while(subjects->hasMember(state.ng.next()));
         _renameBlankNode(state, bnode, state.ng.current());
      }
   }

   // change internal name generator from tmp one to canonical one
   state.ng = c14n;

   // deeply-iterate over bnodes canonically-naming them
   i = bnodes.getIterator();
   while(i->hasNext())
   {
      _deepNameBlankNode(state, i->next());
   }

   // sort property lists that now have canonically-named bnodes
   i = edges["props"].getIterator();
   while(i->hasNext())
   {
      DynamicObject& next = i->next();
      if(next["bnodes"]->length() > 0)
      {
         DynamicObject& bnode = subjects[i->getName()];
         DynamicObjectIterator pi = bnode.getIterator();
         while(pi->hasNext())
         {
            DynamicObject& prop = pi->next();
            const char* p = pi->getName();
            if(p[0] != '@' && prop->getType() == Array)
            {
               prop.sort(_sortObjects);
            }
         }
      }
   }
};

/**
 * Compares two blank node via their canonicalized IRIs.
 *
 * @param a the first blank node.
 * @param b the second blank node.
 *
 * @return true if the first blank node's IRI is less than the second's.
 */
static bool _compareBlankNodeIris(DynamicObject a, DynamicObject b)
{
   return _compare(a["@"]["@iri"], b["@"]["@iri"]) == -1;
}

bool JsonLd::normalize(DynamicObject& in, DynamicObject& out)
{
   bool rval = true;

   // TODO: validate context

   // prepare output
   out->setType(Array);
   out->clear();

   if(!in.isNull())
   {
      // get default context
      DynamicObject ctx = _createDefaultContext();

      // expand input
      DynamicObject expanded = _expand(ctx, NULL, in, true);
      rval = !expanded.isNull();
      if(rval)
      {
         // create canonicalization state
         C14NState state;

         // assign names to unnamed bnodes
         _nameBlankNodes(state, expanded);

         // flatten
         DynamicObject subjects(Map);
         rval = _flatten(NULL, NULL, expanded, subjects);

         if(rval)
         {
            // append unique subjects to array
            DynamicObjectIterator i = subjects.getIterator();
            while(i->hasNext())
            {
               out->append(i->next());
            }

            // canonicalize blank nodes
            _canonicalizeBlankNodes(state, out);

            // sort output
            out.sort(&_compareBlankNodeIris);
         }
      }
   }

   return rval;
}

bool JsonLd::removeContext(DynamicObject& in, DynamicObject& out)
{
   bool rval = true;

   if(in.isNull())
   {
      out.setNull();
   }
   else
   {
      DynamicObject ctx = _createDefaultContext();
      out = _expand(ctx, NULL, in, false);
      rval = !out.isNull();
   }

   return rval;
}

bool JsonLd::addContext(
   DynamicObject& context, DynamicObject& in, DynamicObject& out)
{
   bool rval = true;

   // TODO: should context simplification be optional? (ie: remove context
   // entries that are not used in the output)

   DynamicObject ctx = JsonLd::mergeContexts(_createDefaultContext(), context);
   if(!ctx.isNull())
   {
      // setup output context
      DynamicObject ctxOut(Map);

      // compact
      out = _compact(ctx, NULL, in, &ctxOut);
      rval = !out.isNull();

      // add context if used
      if(rval && ctxOut->length() > 0)
      {
         // add copy of context to every entry in output array
         if(out->getType() == Array)
         {
            DynamicObjectIterator i = out.getIterator();
            while(i->hasNext())
            {
               DynamicObject& next = i->next();
               next["@context"] = ctxOut.clone();
            }
         }
         else
         {
            out["@context"] = ctxOut;
         }
      }
   }

   return rval;
}

bool JsonLd::changeContext(
   DynamicObject& context, DynamicObject& in, DynamicObject& out)
{
   // remove context and then add new one
   DynamicObject tmp;
   return removeContext(in, tmp) && addContext(context, tmp, out);
}

DynamicObject JsonLd::mergeContexts(
   DynamicObject ctx1, DynamicObject ctx2)
{
   // copy contexts
   DynamicObject merged = ctx1.clone();
   DynamicObject copy = ctx2.clone();

   // if the new context contains any IRIs that are in the merged context,
   // remove them from the merged context, they will be overwritten
   DynamicObjectIterator i = ctx2.getIterator();
   while(i->hasNext())
   {
      // ignore special keys starting with '@'
      DynamicObject& iri = i->next();
      if(i->getName()[0] != '@')
      {
         DynamicObjectIterator mi = merged.getIterator();
         while(mi->hasNext())
         {
            DynamicObject& miri = mi->next();
            if(miri == iri)
            {
               mi->remove();
               break;
            }
         }
      }
   }

   // @coerce must be specially-merged, remove from context
   DynamicObject c1 = merged["@coerce"];
   DynamicObject c2 = copy["@coerce"];
   c2->setType(Map);
   merged->removeMember("@coerce");
   copy->removeMember("@coerce");

   // merge contexts (do not append)
   merged.merge(copy, false);

   // special-merge @coerce
   i = c1.getIterator();
   while(i->hasNext())
   {
      DynamicObject& props = i->next();

      // append existing-type properties that don't already exist
      if(c2->hasMember(i->getName()))
      {
         DynamicObjectIterator pi = c2[i->getName()].getIterator();
         while(pi->hasNext())
         {
            DynamicObject& p = pi->next();
            if((props->getType() != Array && props != p) ||
               (props->getType() == Array && props->indexOf(p) == -1))
            {
               props.push(p);
            }
         }
      }
   }

   // add new types from new @coerce
   i = c2.getIterator();
   while(i->hasNext())
   {
      DynamicObject& props = i->next();
      if(!c1->hasMember(i->getName()))
      {
         c1[i->getName()] = props;
      }
   }

   // ensure there are no property duplicates in @coerce
   DynamicObject unique(Map);
   DynamicObject dups(Array);
   i = c1.getIterator();
   while(i->hasNext())
   {
      DynamicObjectIterator pi = i->next().getIterator();
      while(pi->hasNext())
      {
         DynamicObject& p = pi->next();
         if(!unique->hasMember(p))
         {
            unique[p->getString()] = true;
         }
         else if(dups->indexOf(p) == -1)
         {
            dups->append(p);
         }
      }
   }

   if(dups->length() > 0)
   {
      ExceptionRef e = new Exception(
         "Invalid type coercion specification. More than one type "
         "specified for at least one property.",
         EXCEPTION_TYPE ".CoerceSpecError");
      e->getDetails()["duplicates"] = dups;
      Exception::set(e);
      merged.setNull();
   }
   else
   {
      merged["@coerce"] = c1;
   }

   return merged;
}

string JsonLd::expandTerm(DynamicObject ctx, const char* term)
{
   return _expandTerm(ctx, term, NULL);
}

string JsonLd::compactIri(DynamicObject ctx, const char* iri)
{
   return _compactIri(ctx, iri, NULL);
}
