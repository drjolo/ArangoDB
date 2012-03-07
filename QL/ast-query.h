////////////////////////////////////////////////////////////////////////////////
/// @brief AST query declarations
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2010-2012 triagens GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is triAGENS GmbH, Cologne, Germany
///
/// @author Jan Steemann
/// @author Copyright 2012, triagens GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_DURHAM_QL_ASTQUERY
#define TRIAGENS_DURHAM_QL_ASTQUERY

#include <BasicsC/common.h>
#include <BasicsC/conversions.h>
#include <BasicsC/vector.h>
#include <BasicsC/associative.h>
#include <BasicsC/hashes.h>
#include <BasicsC/strings.h>
#include <BasicsC/string-buffer.h>

#include "VocBase/query-node.h"
#include "VocBase/query-context.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup QL
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief Query parts (used for ref counting)
////////////////////////////////////////////////////////////////////////////////

typedef enum {
  REF_TYPE_SELECT,
  REF_TYPE_WHERE,
  REF_TYPE_ORDER,
  REF_TYPE_JOIN
}
QL_ast_query_ref_type_e;

////////////////////////////////////////////////////////////////////////////////
/// @brief Query types
////////////////////////////////////////////////////////////////////////////////

typedef enum {
  QUERY_TYPE_UNDEFINED = 0,
  QUERY_TYPE_EMPTY,
  QUERY_TYPE_SELECT
} 
QL_ast_query_type_e;

////////////////////////////////////////////////////////////////////////////////
/// @brief SELECT query types
////////////////////////////////////////////////////////////////////////////////

typedef enum {
  QLQuerySelectTypeUndefined = 0,

  QLQuerySelectTypeSimple,
  QLQuerySelectTypeEvaluated
} 
QL_ast_query_select_type_e;

////////////////////////////////////////////////////////////////////////////////
/// @brief WHERE types of a query
////////////////////////////////////////////////////////////////////////////////

typedef enum {
  QLQueryWhereTypeUndefined = 0,

  QLQueryWhereTypeMustEvaluate,
  QLQueryWhereTypeAlwaysTrue,
  QLQueryWhereTypeAlwaysFalse
} 
QL_ast_query_where_type_e;

////////////////////////////////////////////////////////////////////////////////
/// @brief ORDER BY types of a query
////////////////////////////////////////////////////////////////////////////////

typedef enum {
  QLQueryOrderTypeUndefined = 0,

  QLQueryOrderTypeMustEvaluate,
  QLQueryOrderTypeNone
} 
QL_ast_query_order_type_e;

////////////////////////////////////////////////////////////////////////////////
/// @brief SELECT part of a query 
////////////////////////////////////////////////////////////////////////////////

typedef struct QL_ast_query_select_s {
  TRI_query_node_t*          _base;
  QL_ast_query_select_type_e _type;
  bool                       _usesBindParameters;
  char*                      _functionCode;
} 
QL_ast_query_select_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief FROM part of a query 
////////////////////////////////////////////////////////////////////////////////

typedef struct QL_ast_query_from_s {
  TRI_query_node_t*         _base;
  TRI_associative_pointer_t _collections;
} 
QL_ast_query_from_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief WHERE part of a query 
////////////////////////////////////////////////////////////////////////////////

typedef struct QL_ast_query_where_s {
  TRI_query_node_t*          _base;
  QL_ast_query_where_type_e  _type;
  bool                       _usesBindParameters;
  char*                      _functionCode;
  TRI_js_exec_context_t*     _context;
} 
QL_ast_query_where_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief ORDER part of a query 
////////////////////////////////////////////////////////////////////////////////

typedef struct QL_ast_query_order_s {
  TRI_query_node_t*          _base;
  QL_ast_query_order_type_e  _type;
  bool                       _usesBindParameters;
  char*                      _functionCode;
} 
QL_ast_query_order_t;


////////////////////////////////////////////////////////////////////////////////
/// @brief LIMIT part of a query 
////////////////////////////////////////////////////////////////////////////////

typedef struct QL_ast_query_limit_s {
  int64_t  _offset;
  int64_t  _count;
  bool     _isUsed;
} 
QL_ast_query_limit_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief Geo restriction types
////////////////////////////////////////////////////////////////////////////////

typedef enum {
  RESTRICT_WITHIN = 1,
  RESTRICT_NEAR   = 2
}
QL_ast_query_geo_restriction_e;

////////////////////////////////////////////////////////////////////////////////
/// @brief single geo restriction 
////////////////////////////////////////////////////////////////////////////////

typedef struct QL_ast_query_geo_restriction_s {
  char* _alias;
  QL_ast_query_geo_restriction_e _type;
  struct {
    char* _collection;
    char* _field;
  } _compareLat;
  struct {
    char* _collection;
    char* _field;
  } _compareLon;
  double _lat;
  double _lon;
  union {
    double _radius;
    size_t _numDocuments;
  } _arg;
}
QL_ast_query_geo_restriction_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief Geo restrictions of a query
////////////////////////////////////////////////////////////////////////////////

typedef struct QL_ast_query_geo_s {
  TRI_associative_pointer_t _restrictions;
} 
QL_ast_query_geo_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief Query declaration
///
/// A query consists of multiple parts, e.g. the selection part, the from part
/// containing the referenced collections, an optional where condition, an
/// optional order part and an optional limit
///
/// This struct might change later if further types of queries are introduced
/// that have different properties.
////////////////////////////////////////////////////////////////////////////////

typedef struct QL_ast_query_s {
  QL_ast_query_type_e    _type;

  QL_ast_query_select_t  _select;
  QL_ast_query_from_t    _from;
  QL_ast_query_where_t   _where;
  QL_ast_query_order_t   _order;
  QL_ast_query_limit_t   _limit;
  QL_ast_query_geo_t     _geo;

  bool                   _isEmpty;
} 
QL_ast_query_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief Collection referenced by a query
////////////////////////////////////////////////////////////////////////////////

typedef struct QL_ast_query_collection_s {
  char*                           _name;
  char*                           _alias;
  bool                            _isPrimary;
  struct {
    size_t                        _select;
    size_t                        _where;
    size_t                        _order;
    size_t                        _join;
  }                               _refCount;
  size_t                          _declarationOrder;
  QL_ast_query_geo_restriction_t* _geoRestriction;
  QL_ast_query_where_t            _where;
} 
QL_ast_query_collection_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief Hash function used to hash collection aliases
////////////////////////////////////////////////////////////////////////////////

uint64_t QLHashKey (TRI_associative_pointer_t*, void const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief Hash function used to hash elements in the collection
////////////////////////////////////////////////////////////////////////////////

uint64_t QLHashCollectionElement (TRI_associative_pointer_t*, void const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief Comparison function used to determine hash key equality
////////////////////////////////////////////////////////////////////////////////

bool QLEqualCollectionKeyElement (TRI_associative_pointer_t*,  
                                  void const*, 
                                  void const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief free collections previously registered
////////////////////////////////////////////////////////////////////////////////

void QLAstQueryFreeCollections (TRI_associative_pointer_t* const);

////////////////////////////////////////////////////////////////////////////////
/// @brief Initialize data structures for a query
////////////////////////////////////////////////////////////////////////////////

void QLAstQueryInit (QL_ast_query_t* const);

////////////////////////////////////////////////////////////////////////////////
/// @brief De-allocate data structures for a query
////////////////////////////////////////////////////////////////////////////////

void QLAstQueryFree (QL_ast_query_t* const);

////////////////////////////////////////////////////////////////////////////////
/// @brief get the total ref count for a collection
////////////////////////////////////////////////////////////////////////////////

size_t QLAstQueryGetTotalRefCount (QL_ast_query_t*, 
                                   const char*);

////////////////////////////////////////////////////////////////////////////////
/// @brief get the ref count for a collection
////////////////////////////////////////////////////////////////////////////////

size_t QLAstQueryGetRefCount (QL_ast_query_t*, 
                              const char*,
                              const QL_ast_query_ref_type_e);

////////////////////////////////////////////////////////////////////////////////
/// @brief Increment ref count for a collection
////////////////////////////////////////////////////////////////////////////////

void QLAstQueryAddRefCount (QL_ast_query_t*, 
                            const char*, 
                            const QL_ast_query_ref_type_e);

////////////////////////////////////////////////////////////////////////////////
/// @brief Check if a collection was defined in a query
////////////////////////////////////////////////////////////////////////////////

bool QLAstQueryIsValidAlias (QL_ast_query_t*, const char*, const size_t);

////////////////////////////////////////////////////////////////////////////////
/// @brief Check if a collection was defined in a query, taking order of 
/// declaration into account
////////////////////////////////////////////////////////////////////////////////

bool QLAstQueryIsValidAliasOrdered (QL_ast_query_t*, const char*, const size_t);

////////////////////////////////////////////////////////////////////////////////
/// @brief Return the collection name for its alias
////////////////////////////////////////////////////////////////////////////////

char* QLAstQueryGetCollectionNameForAlias (QL_ast_query_t*, const char*);

////////////////////////////////////////////////////////////////////////////////
/// @brief Add a collection to the query
////////////////////////////////////////////////////////////////////////////////

bool QLAstQueryAddCollection (QL_ast_query_t*, const char*, const char*);

////////////////////////////////////////////////////////////////////////////////
/// @brief get the alias of the primary collection used in the query
////////////////////////////////////////////////////////////////////////////////

char* QLAstQueryGetPrimaryAlias (const QL_ast_query_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief get a geo restriction prototype
////////////////////////////////////////////////////////////////////////////////

QL_ast_query_geo_restriction_t* QLAstQueryCreateRestriction (void);

////////////////////////////////////////////////////////////////////////////////
/// @brief clone a geo restriction 
////////////////////////////////////////////////////////////////////////////////

QL_ast_query_geo_restriction_t* QLAstQueryCloneRestriction 
  (const QL_ast_query_geo_restriction_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief free a geo restriction 
////////////////////////////////////////////////////////////////////////////////
      
void QLAstQueryFreeRestriction (QL_ast_query_geo_restriction_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief add a geo restriction 
////////////////////////////////////////////////////////////////////////////////

bool QLAstQueryAddGeoRestriction (QL_ast_query_t*, 
                                  const TRI_query_node_t*,
                                  const TRI_query_node_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief Create a string from a member name
////////////////////////////////////////////////////////////////////////////////

TRI_string_buffer_t* QLAstQueryGetMemberNameString (TRI_query_node_t*, bool); 

////////////////////////////////////////////////////////////////////////////////
/// @brief Hash a member name for comparisons
////////////////////////////////////////////////////////////////////////////////

uint64_t QLAstQueryGetMemberNameHash (TRI_query_node_t*);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|// --SECTION--\\|/// @\\}\\)"
// End:

