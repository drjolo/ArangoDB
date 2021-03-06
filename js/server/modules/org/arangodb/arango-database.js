/*jslint indent: 2, nomen: true, maxlen: 100, sloppy: true, vars: true, white: true, plusplus: true */
/*global require, exports */

////////////////////////////////////////////////////////////////////////////////
/// @brief ArangoDatabase
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2013 triagens GmbH, Cologne, Germany
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
/// @author Achim Brandt
/// @author Dr. Frank Celler
/// @author Copyright 2012-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

var internal = require("internal");

// -----------------------------------------------------------------------------
// --SECTION--                                                    ArangoDatabase
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoShell
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief constructor
////////////////////////////////////////////////////////////////////////////////

exports.ArangoDatabase = internal.ArangoDatabase;

var ArangoDatabase = exports.ArangoDatabase;

// must called after export
var ArangoCollection = require("org/arangodb/arango-collection").ArangoCollection;
var ArangoError = require("org/arangodb/arango-error").ArangoError;
var ArangoStatement = require("org/arangodb/arango-statement").ArangoStatement;

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoShell
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief prints a database
////////////////////////////////////////////////////////////////////////////////

ArangoDatabase.prototype._PRINT = function(seen, path, names, level) {
  internal.output(this.toString());
};

////////////////////////////////////////////////////////////////////////////////
/// @brief strng representation of a database
////////////////////////////////////////////////////////////////////////////////

ArangoDatabase.prototype.toString = function(seen, path, names, level) {
  return "[ArangoDatabase \"" + this._path + "\"]";
};

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                   query functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoShell
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief factory method to create a new statement
////////////////////////////////////////////////////////////////////////////////

ArangoDatabase.prototype._createStatement = function (data) {  
  return new ArangoStatement(this, data);
};

////////////////////////////////////////////////////////////////////////////////
/// @brief factory method to create and execute a new statement
////////////////////////////////////////////////////////////////////////////////

ArangoDatabase.prototype._query = function (data) {  
  return new ArangoStatement(this, { query: data }).execute();
};

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                              collection functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoShell
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief drops a collection
///
/// @FUN{db._drop(@FA{collection})}
///
/// Drops a @FA{collection} and all its indexes.
///
/// @FUN{db._drop(@FA{collection-identifier})}
///
/// Drops a collection identified by @FA{collection-identifier} and all its
/// indexes. No error is thrown if there is no such collection.
///
/// @FUN{db._drop(@FA{collection-name})}
///
/// Drops a collection named @FA{collection-name} and all its indexes. No error
/// is thrown if there is no such collection.
///
/// @EXAMPLES
///
/// Drops a collection:
///
/// @verbinclude shell_collection-drop-db
///
/// Drops a collection identified by name:
///
/// @verbinclude shell_collection-drop-name-db
////////////////////////////////////////////////////////////////////////////////

ArangoDatabase.prototype._drop = function(name) {
  var collection = name;

  if (! (name instanceof ArangoCollection)) {
    collection = internal.db._collection(name);
  }

  if (collection === null) {
    return;
  }

  return collection.drop();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief truncates a collection
///
/// @FUN{db._truncate(@FA{collection})}
///
/// Truncates a @FA{collection}, removing all documents but keeping all its
/// indexes.
///
/// @FUN{db._truncate(@FA{collection-identifier})}
///
/// Truncates a collection identified by @FA{collection-identified}. No error is
/// thrown if there is no such collection.
///
/// @FUN{db._truncate(@FA{collection-name})}
///
/// Truncates a collection named @FA{collection-name}. No error is thrown if
/// there is no such collection.
///
/// @EXAMPLES
///
/// Truncates a collection:
///
/// @verbinclude shell_collection-truncate-db
///
/// Truncates a collection identified by name:
///
/// @verbinclude shell_collection-truncate-name-db
////////////////////////////////////////////////////////////////////////////////

ArangoDatabase.prototype._truncate = function(name) {
  var collection = name;

  if (! (name instanceof ArangoCollection)) {
    collection = internal.db._collection(name);
  }

  if (collection === null) {
    return;
  }

  collection.truncate();
};

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                   index functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoShell
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief index id regex
////////////////////////////////////////////////////////////////////////////////

ArangoDatabase.indexRegex = /^([a-zA-Z0-9\-_]+)\/([0-9]+)$/;

////////////////////////////////////////////////////////////////////////////////
/// @brief finds an index
///
/// @FUN{db._index(@FA{index-handle})}
///
/// Returns the index with @FA{index-handle} or null if no such index exists.
///
/// @EXAMPLES
///
/// @verbinclude shell_index-read-db
////////////////////////////////////////////////////////////////////////////////

ArangoDatabase.prototype._index = function(id) {
  if (id.hasOwnProperty("id")) {
    id = id.id;
  }

  var pa = ArangoDatabase.indexRegex.exec(id);
  var err;

  if (pa === null) {
    err = new ArangoError();
    err.errorNum = internal.errors.ERROR_ARANGO_INDEX_HANDLE_BAD.code;
    err.errorMessage = internal.errors.ERROR_ARANGO_INDEX_HANDLE_BAD.message;
    throw err;
  }

  var col = this._collection(pa[1]);

  if (col === null) {
    err = new ArangoError();
    err.errorNum = internal.errors.ERROR_ARANGO_COLLECTION_NOT_FOUND.code;
    err.errorMessage = internal.errors.ERROR_ARANGO_COLLECTION_NOT_FOUND.message;
    throw err;
  }

  var indexes = col.getIndexes();
  var i;

  for (i = 0;  i < indexes.length;  ++i) {
    var index = indexes[i];

    if (index.id === id) {
      return index;
    }
  }

  return null;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief drops an index
///
/// @FUN{db._dropIndex(@FA{index})}
///
/// Drops the @FA{index}.  If the index does not exists, then @LIT{false} is
/// returned. If the index existed and was dropped, then @LIT{true} is
/// returned. Note that you cannot drop the primary index.
///
/// @FUN{db._dropIndex(@FA{index-handle})}
///
/// Drops the index with @FA{index-handle}.
///
/// @EXAMPLES
///
/// @verbinclude shell_index-drop-index-db
////////////////////////////////////////////////////////////////////////////////

ArangoDatabase.prototype._dropIndex = function(id) {
  if (id.hasOwnProperty("id")) {
    id = id.id;
  }

  var pa = ArangoDatabase.indexRegex.exec(id);
  var err;

  if (pa === null) {
    err = new ArangoError();
    err.errorNum = internal.errors.ERROR_ARANGO_INDEX_HANDLE_BAD.code;
    err.errorMessage = internal.errors.ERROR_ARANGO_INDEX_HANDLE_BAD.message;
    throw err;
  }

  var col = this._collection(pa[1]);

  if (col === null) {
    err = new ArangoError();
    err.errorNum = internal.errors.ERROR_ARANGO_COLLECTION_NOT_FOUND.code;
    err.errorMessage = internal.errors.ERROR_ARANGO_COLLECTION_NOT_FOUND.message;
    throw err;
  }

  return col.dropIndex(id);
};

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// @addtogroup\\|// --SECTION--\\|/// @}\\|/\\*jslint"
// End:
