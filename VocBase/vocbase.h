////////////////////////////////////////////////////////////////////////////////
/// @brief vocbase
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2010-2011 triagens GmbH, Cologne, Germany
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
/// @author Dr. Frank Celler
/// @author Copyright 2011, triagens GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_DURHAM_VOC_BASE_VOCBASE_H
#define TRIAGENS_DURHAM_VOC_BASE_VOCBASE_H 1

#include <BasicsC/common.h>

#include <BasicsC/associative.h>
#include <BasicsC/locks.h>
#include <BasicsC/threads.h>
#include <BasicsC/vector.h>

#ifdef __cplusplus
extern "C" {
#endif

struct TRI_doc_collection_s;
struct TRI_col_parameter_s;
struct TRI_shadow_store_s;

// -----------------------------------------------------------------------------
// --SECTION--                                                  public constants
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief page size
////////////////////////////////////////////////////////////////////////////////

extern size_t PageSize;

////////////////////////////////////////////////////////////////////////////////
/// @brief maximal path length
////////////////////////////////////////////////////////////////////////////////

#define TRI_COL_PATH_LENGTH     (4096)

////////////////////////////////////////////////////////////////////////////////
/// @brief default maximal collection journal size
////////////////////////////////////////////////////////////////////////////////

#define DEFAULT_MAXIMAL_SIZE (1024 * 1024 * 128)

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                      public types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief error codes
///
/// If you add a new error code, you must also add the description in
/// @ref TRI_InitialiseVocBase.
///
/// Please note that the error numbers defined here must not conflict with error
/// numbers defined for other parts of the program (e.g. in VocBase/vocbase.h)
////////////////////////////////////////////////////////////////////////////////

enum {

  // general errors
  TRI_VOC_ERROR_ILLEGAL_STATE        = 1000,
  TRI_VOC_ERROR_SHAPER_FAILED        = 1001,
  TRI_VOC_ERROR_CORRUPTED_DATAFILE   = 1002,
  TRI_VOC_ERROR_MMAP_FAILED          = 1003,
  TRI_VOC_ERROR_MSYNC_FAILED         = 1004,
  TRI_VOC_ERROR_NO_JOURNAL           = 1005,
  TRI_VOC_ERROR_DATAFILE_SEALED      = 1006,
  TRI_VOC_ERROR_CORRUPTED_COLLECTION = 1007,
  TRI_VOC_ERROR_UNKNOWN_TYPE         = 1008,
  TRI_VOC_ERROR_ILLEGAL_PARAMETER    = 1009,
  TRI_VOC_ERROR_INDEX_EXISTS         = 1010,
  TRI_VOC_ERROR_CONFLICT             = 1011,

  // open errors
  TRI_VOC_ERROR_WRONG_PATH = 2000,

  // close errors
  TRI_VOC_ERROR_CANNOT_RENAME = 3001,

  // write errors
  TRI_VOC_ERROR_WRITE_FAILED     = 4000,
  TRI_VOC_ERROR_READ_ONLY        = 4001,
  TRI_VOC_ERROR_DATAFILE_FULL    = 4002,
  TRI_VOC_ERROR_FILESYSTEM_FULL  = 4004,

  // read errors
  TRI_VOC_ERROR_READ_FAILED         = 5000,
  TRI_VOC_ERROR_FILE_NOT_FOUND      = 5001,
  TRI_VOC_ERROR_FILE_NOT_ACCESSIBLE = 5002,

  // document errors
  TRI_VOC_ERROR_DOCUMENT_NOT_FOUND = 6000
};

////////////////////////////////////////////////////////////////////////////////
/// @brief tick type (48bit)
////////////////////////////////////////////////////////////////////////////////

typedef uint64_t TRI_voc_tick_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief collection identifier type
////////////////////////////////////////////////////////////////////////////////

typedef uint64_t TRI_voc_cid_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief datafile identifier type
////////////////////////////////////////////////////////////////////////////////

typedef uint64_t TRI_voc_fid_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief document identifier type
////////////////////////////////////////////////////////////////////////////////

typedef uint64_t TRI_voc_did_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief revision identifier type
////////////////////////////////////////////////////////////////////////////////

typedef uint64_t TRI_voc_rid_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief step identifier type
////////////////////////////////////////////////////////////////////////////////

typedef uint64_t TRI_voc_eid_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief transaction identifier type
////////////////////////////////////////////////////////////////////////////////

typedef uint64_t TRI_voc_tid_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief size type
////////////////////////////////////////////////////////////////////////////////

typedef uint32_t TRI_voc_size_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief signed size type
////////////////////////////////////////////////////////////////////////////////

typedef int32_t TRI_voc_ssize_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief boolean flag type
////////////////////////////////////////////////////////////////////////////////

typedef uint32_t TRI_voc_flag_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief milli-seconds
////////////////////////////////////////////////////////////////////////////////

typedef uint32_t TRI_voc_ms_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief crc type
////////////////////////////////////////////////////////////////////////////////

typedef uint32_t TRI_voc_crc_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief collection storage type
////////////////////////////////////////////////////////////////////////////////

typedef uint32_t TRI_col_type_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief database
///
/// Note that access to _collections, _collectionsByName, and _collectionsById
/// is only allowed while holding the _lock.
////////////////////////////////////////////////////////////////////////////////

typedef struct TRI_vocbase_s {
  char const* _path;
  char* _lockFile;

  TRI_vector_pointer_t _collections;

  TRI_associative_pointer_t _collectionsByName;
  TRI_associative_pointer_t _collectionsById;

  TRI_read_write_lock_t _lock;

  sig_atomic_t _active;
  TRI_thread_t _synchroniser;
  TRI_thread_t _compactor;

  struct TRI_shadow_store_s* _cursors;
}
TRI_vocbase_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief document collection container
////////////////////////////////////////////////////////////////////////////////

typedef struct TRI_vocbase_col_s {
  TRI_vocbase_t* _vocbase;

  TRI_col_type_t _type;              // collection type
  TRI_voc_cid_t _cid;                // collecttion identifier
  char _name[TRI_COL_PATH_LENGTH];   // name of the collection

  char const* _path;

  struct TRI_doc_collection_s* _collection;  // NULL or pointer to loaded collection
  sig_atomic_t _newBorn;                     // true if collection is newly born
  sig_atomic_t _loaded;                      // true if collection is loaded
  sig_atomic_t _corrupted;                   // true if collection could not be loaded
}
TRI_vocbase_col_t;

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new tick
////////////////////////////////////////////////////////////////////////////////

TRI_voc_tick_t TRI_NewTickVocBase (void);

////////////////////////////////////////////////////////////////////////////////
/// @brief updates the tick counter
////////////////////////////////////////////////////////////////////////////////

void TRI_UpdateTickVocBase (TRI_voc_tick_t tick);

////////////////////////////////////////////////////////////////////////////////
/// @brief msyncs a memory block between begin (incl) and end (excl)
////////////////////////////////////////////////////////////////////////////////

bool TRI_msync (int fd, char const* begin, char const* end);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief opens an exiting database, loads all collections
////////////////////////////////////////////////////////////////////////////////

TRI_vocbase_t* TRI_OpenVocBase (char const* path);

////////////////////////////////////////////////////////////////////////////////
/// @brief closes a database and all collections
////////////////////////////////////////////////////////////////////////////////

void TRI_CloseVocBase (TRI_vocbase_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief looks up a (document) collection by name
////////////////////////////////////////////////////////////////////////////////

TRI_vocbase_col_t const* TRI_LookupCollectionByNameVocBase (TRI_vocbase_t*, char const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief looks up a (document) collection by identifier
////////////////////////////////////////////////////////////////////////////////

TRI_vocbase_col_t const* TRI_LookupCollectionByIdVocBase (TRI_vocbase_t*, TRI_voc_cid_t);

////////////////////////////////////////////////////////////////////////////////
/// @brief finds up a (document) collection by name
////////////////////////////////////////////////////////////////////////////////

TRI_vocbase_col_t const* TRI_FindCollectionByNameVocBase (TRI_vocbase_t*, char const*, bool bear);

////////////////////////////////////////////////////////////////////////////////
/// @brief returns all known collections
////////////////////////////////////////////////////////////////////////////////

TRI_vector_pointer_t TRI_CollectionsVocBase (TRI_vocbase_t* vocbase);

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a new (document) collection
////////////////////////////////////////////////////////////////////////////////

TRI_vocbase_col_t const* TRI_CreateCollectionVocBase (TRI_vocbase_t*, struct TRI_col_parameter_s*);

////////////////////////////////////////////////////////////////////////////////
/// @brief loads an existing (document) collection
////////////////////////////////////////////////////////////////////////////////

TRI_vocbase_col_t const* TRI_LoadCollectionVocBase (TRI_vocbase_t* vocbase, char const* name);

////////////////////////////////////////////////////////////////////////////////
/// @brief reserves a new collection or returns an existing
////////////////////////////////////////////////////////////////////////////////

TRI_vocbase_col_t const* TRI_BearCollectionVocBase (TRI_vocbase_t* vocbase, char const* name);

////////////////////////////////////////////////////////////////////////////////
/// @brief manifests a new born collection
////////////////////////////////////////////////////////////////////////////////

bool TRI_ManifestCollectionVocBase (TRI_vocbase_t* vocbase, TRI_vocbase_col_t const*);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                            MODULE
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief initialises the voc database components
////////////////////////////////////////////////////////////////////////////////

void TRI_InitialiseVocBase (void);

////////////////////////////////////////////////////////////////////////////////
/// @brief shut downs the voc database components
////////////////////////////////////////////////////////////////////////////////

void TRI_ShutdownVocBase (void);

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
