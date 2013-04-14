////////////////////////////////////////////////////////////////////////////////
/// @brief transaction subsystem
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2004-2013 triAGENS GmbH, Cologne, Germany
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
/// @author Copyright 2012-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "transaction.h"

#include "BasicsC/conversions.h"
#include "BasicsC/logging.h"
#include "BasicsC/tri-strings.h"

#include "VocBase/primary-collection.h"
#include "VocBase/document-collection.h"
#include "VocBase/vocbase.h"

#define LOG_TRX(trx, level, format, ...) \
  LOG_TRACE("trx #%llu.%d (%s): " format, (unsigned long long) trx->_id, (int) level, StatusTransaction(trx->_status), __VA_ARGS__)

// -----------------------------------------------------------------------------
// --SECTION--                                               TRANSACTION CONTEXT
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief hashes the collection id
////////////////////////////////////////////////////////////////////////////////

#if 0

static uint64_t HashKeyCid (TRI_associative_pointer_t* array, void const* key) {
  TRI_voc_cid_t const* k = key;

  return (uint64_t) *k;
}

#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief hashs the collection id
////////////////////////////////////////////////////////////////////////////////

#if 0

static uint64_t HashElementCid (TRI_associative_pointer_t* array, void const* element) {
  TRI_transaction_collection_global_t const* e = element;

  return (uint64_t) e->_cid;
}

#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief compares a collection id and a collection
////////////////////////////////////////////////////////////////////////////////

#if 0

static bool EqualKeyCid (TRI_associative_pointer_t* array, void const* key, void const* element) {
  TRI_voc_cid_t const* k = key;
  TRI_transaction_collection_global_t const* e = element;

  return *k == e->_cid;
}

#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief create a global instance of a collection
////////////////////////////////////////////////////////////////////////////////

#if 0

static TRI_transaction_collection_global_t* CreateGlobalInstance (TRI_voc_cid_t cid) {
  TRI_transaction_collection_global_t* globalInstance;

  globalInstance = TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, sizeof(TRI_transaction_collection_global_t), false);

  if (globalInstance != NULL) {
    globalInstance->_cid                       = cid;
    globalInstance->_stats._lastStartedReader  = 0;
    globalInstance->_stats._lastFinishedReader = 0;
    globalInstance->_stats._lastStartedWriter  = 0;
    globalInstance->_stats._lastAbortedWriter  = 0;
    globalInstance->_stats._lastFinishedWriter = 0;

    TRI_InitReadWriteLock(&globalInstance->_lock);
  }

  return globalInstance;
}

#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief create a global instance of a collection
////////////////////////////////////////////////////////////////////////////////

#if 0

static void FreeGlobalInstance (TRI_transaction_collection_global_t* globalInstance) {
  TRI_DestroyReadWriteLock(&globalInstance->_lock);
  TRI_Free(TRI_UNKNOWN_MEM_ZONE, globalInstance);
}

#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief get the global instance of a collection
////////////////////////////////////////////////////////////////////////////////

#if 0

static TRI_transaction_collection_global_t* GetGlobalInstance (TRI_transaction_context_t* context,
                                                               TRI_voc_cid_t cid,
                                                               const bool create) {
  TRI_transaction_collection_global_t* globalInstance;
  TRI_transaction_collection_global_t* found;

  // acquire a read-lock on the global list of collections
  TRI_ReadLockReadWriteLock(&context->_collectionsLock);
  globalInstance = TRI_LookupByKeyAssociativePointer(&context->_collections, &cid);
  TRI_ReadUnlockReadWriteLock(&context->_collectionsLock);

  if (globalInstance != NULL) {
    // we already have a global instance
    return globalInstance;
  }

  // collection not found

  if (! create) {
    // not asked to create a new global instance
    return NULL;
  }

  // create a new global instance
  globalInstance = CreateGlobalInstance(cid);

  if (globalInstance == NULL) {
    return NULL;
  }

  // write lock and insert the new instance, but only if no one else has inserted it
  TRI_WriteLockReadWriteLock(&context->_collectionsLock);
  found = TRI_InsertKeyAssociativePointer(&context->_collections, &cid, globalInstance, false);
  TRI_WriteUnlockReadWriteLock(&context->_collectionsLock);

  if (found != NULL) {
    // someone else inserted the global instance instead. so we'll return the
    // already existing one
    FreeGlobalInstance(globalInstance);
    globalInstance = found;
  }

  // return whatever we've got
  return globalInstance;
}

#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief update the global transaction statistics
////////////////////////////////////////////////////////////////////////////////

#if 0

static int UpdateGlobalStats (const TRI_transaction_t const* trx,
                              const TRI_transaction_status_e status) {
  size_t i, n;

  n = trx->_collections._length;

  for (i = 0; i < n; ++i) {
    TRI_transaction_collection_t* trxCollection;
    TRI_transaction_collection_global_t* globalInstance;

    trxCollection = (TRI_transaction_collection_t*) TRI_AtVectorPointer(&trx->_collections, i);

    globalInstance = trxCollection->_globalInstance;
    assert(globalInstance != NULL);

    if (globalInstance == NULL) {
      LOG_ERROR("internal error in UpdateGlobalStats");
      return TRI_ERROR_INTERNAL;
    }

    TRI_WriteLockReadWriteLock(&globalInstance->_lock);

    if (trxCollection->_accessType == TRI_TRANSACTION_READ) {
      if (status == TRI_TRANSACTION_RUNNING) {    
        globalInstance->_stats._lastStartedReader = trx->_id;
      }
      else if (status == TRI_TRANSACTION_ABORTED ||
               status == TRI_TRANSACTION_COMMITTED) {
        globalInstance->_stats._lastFinishedReader = trx->_id;
      }
    }
    else {
      if (status == TRI_TRANSACTION_RUNNING) {    
        globalInstance->_stats._lastStartedWriter = trx->_id;
      }
      else if (status == TRI_TRANSACTION_ABORTED) {
        globalInstance->_stats._lastAbortedWriter = trx->_id;
      }
      else if (status == TRI_TRANSACTION_COMMITTED) {
        globalInstance->_stats._lastFinishedWriter = trx->_id;
      }
    }

    TRI_WriteUnlockReadWriteLock(&globalInstance->_lock);
  }

  return TRI_ERROR_NO_ERROR;
}

#endif

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief create the global transaction context
////////////////////////////////////////////////////////////////////////////////

TRI_transaction_context_t* TRI_CreateTransactionContext (TRI_vocbase_t* const vocbase) {
  TRI_transaction_context_t* context;

  context = (TRI_transaction_context_t*) TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, sizeof(TRI_transaction_context_t), false);

  if (context == NULL) {
    return context;
  }

  context->_vocbase = vocbase;

#if 0 
  TRI_InitAssociativePointer(&context->_collections,
                             TRI_UNKNOWN_MEM_ZONE,
                             HashKeyCid,
                             HashElementCid,
                             EqualKeyCid,
                             NULL);
  TRI_InitReadWriteLock(&context->_collectionsLock);
#endif

  return context;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief free the global transaction context
////////////////////////////////////////////////////////////////////////////////

void TRI_FreeTransactionContext (TRI_transaction_context_t* const context) {
#if 0
  TRI_DestroyAssociativePointer(&context->_collections);
  TRI_DestroyReadWriteLock(&context->_collectionsLock);
#endif

  TRI_Free(TRI_UNKNOWN_MEM_ZONE, context);
}

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
/// @brief free all data associated with a specific collection
/// this function gets called for all collections that are dropped
////////////////////////////////////////////////////////////////////////////////

void TRI_RemoveCollectionTransactionContext (TRI_transaction_context_t* context,
                                             const TRI_voc_cid_t cid) {
#if 0

  TRI_transaction_collection_global_t* globalInstance;

  globalInstance = GetGlobalInstance(context, cid, false);
  
  if (globalInstance != NULL) {
    // write lock and remove the instance
    TRI_WriteLockReadWriteLock(&context->_collectionsLock);
    TRI_RemoveKeyAssociativePointer(&context->_collections, &cid);
    TRI_WriteUnlockReadWriteLock(&context->_collectionsLock);

    FreeGlobalInstance(globalInstance);
  }

#endif
}

////////////////////////////////////////////////////////////////////////////////
/// @brief populates a struct with transaction statistics for a collections
////////////////////////////////////////////////////////////////////////////////

#if 0

int TRI_StatsCollectionTransactionContext (TRI_transaction_context_t* context,
                                           const TRI_voc_cid_t cid,
                                           TRI_transaction_collection_stats_t* stats) {
  TRI_transaction_collection_global_t* globalInstance;

  globalInstance = GetGlobalInstance(context, cid, false);

  if (globalInstance == NULL) {
    return TRI_ERROR_ARANGO_COLLECTION_NOT_FOUND;
  }

  TRI_ReadLockReadWriteLock(&globalInstance->_lock);
  memcpy(stats, &globalInstance->_stats, sizeof(TRI_transaction_collection_stats_t));
  TRI_ReadUnlockReadWriteLock(&globalInstance->_lock);

  return TRI_ERROR_NO_ERROR;
}

#endif

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                       TRANSACTION
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                     private types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief type containing a write operation for a transaction collection
////////////////////////////////////////////////////////////////////////////////

typedef struct transaction_operation_s {
  TRI_doc_mptr_t*               _newHeader;
  TRI_doc_mptr_t*               _oldHeader;
  TRI_doc_mptr_t                _oldData;
  TRI_df_marker_t*              _marker;
  size_t                        _markerSize;
  TRI_voc_document_operation_e  _type;
}
transaction_operation_t;

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief return the status of the transaction as a string
////////////////////////////////////////////////////////////////////////////////

static const char* StatusTransaction (const TRI_transaction_status_e status) {
  switch (status) {
    case TRI_TRANSACTION_UNDEFINED:
      return "undefined";
    case TRI_TRANSACTION_CREATED:
      return "created";
    case TRI_TRANSACTION_RUNNING:
      return "running";
    case TRI_TRANSACTION_COMMITTED:
      return "committed";
    case TRI_TRANSACTION_ABORTED:
      return "aborted";
    case TRI_TRANSACTION_FAILED:
      return "failed";
  }

  assert(false);
  return "unknown";
}

////////////////////////////////////////////////////////////////////////////////
/// @brief count the number of write collections
////////////////////////////////////////////////////////////////////////////////

static int CountWriteCollections (TRI_transaction_t* const trx) {
  int numCollections;

  numCollections = 0;

  if (trx->_hasOperations) {
    size_t i, n;

    n = trx->_collections._length;

    for (i = 0; i < n; ++i) {
      TRI_transaction_collection_t* trxCollection;

      trxCollection = TRI_AtVectorPointer(&trx->_collections, i);

      if (trxCollection->_operations != NULL) {
        numCollections++;
      }
    }
  }

  return numCollections;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief initialise operations for a collection
////////////////////////////////////////////////////////////////////////////////

static int InitCollectionOperations (TRI_transaction_collection_t* trxCollection) {
  int res;

  trxCollection->_operations = TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, sizeof(TRI_vector_t), false);

  if (trxCollection->_operations == NULL) {
    return TRI_ERROR_OUT_OF_MEMORY;
  }

  res = TRI_InitVector2(trxCollection->_operations, TRI_UNKNOWN_MEM_ZONE, sizeof(transaction_operation_t), 4);
   
  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief add an operation for a collection
////////////////////////////////////////////////////////////////////////////////

static int AddCollectionOperation (TRI_transaction_collection_t* trxCollection,
                                   TRI_voc_document_operation_e type,
                                   TRI_doc_mptr_t* newHeader,
                                   TRI_doc_mptr_t* oldHeader,
                                   TRI_doc_mptr_t* oldData,
                                   TRI_df_marker_t* marker,
                                   size_t totalSize) {
  transaction_operation_t trxOperation;
  int res;
  

  if (trxCollection->_operations == NULL) {
    res = InitCollectionOperations(trxCollection);

    if (res != TRI_ERROR_NO_ERROR) {
      return res;
    }
  }

  trxOperation._type       = type;
  trxOperation._newHeader  = newHeader;
  trxOperation._oldHeader  = oldHeader;
  trxOperation._marker     = marker;
  trxOperation._markerSize = totalSize;
  
  if (oldData != NULL) {
    trxOperation._oldData  = *oldData;
  }
  else {
    memset(&trxOperation._oldData, 0, sizeof(TRI_doc_mptr_t));
  }
  
  res = TRI_PushBackVector(trxCollection->_operations, &trxOperation);
  
  if (res == TRI_ERROR_NO_ERROR) {
    if (type == TRI_VOC_DOCUMENT_OPERATION_UPDATE) {
      TRI_document_collection_t* document = (TRI_document_collection_t*) trxCollection->_collection->_collection;

      document->_headers->moveBack(document->_headers, oldHeader);
    }
    else if (type == TRI_VOC_DOCUMENT_OPERATION_REMOVE) {
      TRI_document_collection_t* document = (TRI_document_collection_t*) trxCollection->_collection->_collection;

      document->_headers->unlink(document->_headers, oldHeader);
    }
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// write an abort marker for a collection
////////////////////////////////////////////////////////////////////////////////

static int WriteCollectionAbort (TRI_transaction_collection_t* trxCollection) {
  TRI_doc_abort_transaction_marker_t* abortMarker;
  TRI_document_collection_t* document;
  TRI_transaction_t* trx;
  TRI_df_marker_t* result;
  int res;
  
  trx = trxCollection->_transaction;
  document = (TRI_document_collection_t*) trxCollection->_collection->_collection;
    
  // create the "commit transaction" marker
  res = TRI_CreateMarkerAbortTransaction(trx, &abortMarker);

  if (res != TRI_ERROR_NO_ERROR) {
    return res;
  }
  
  res = TRI_WriteMarkerDocumentCollection(document, 
                                          &abortMarker->base,
                                          abortMarker->base._size,
                                          NULL,
                                          &result, 
                                          false /* trxCollection->_waitForSync */);
    
  TRI_Free(TRI_UNKNOWN_MEM_ZONE, abortMarker);

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// write a commit marker for a collection
////////////////////////////////////////////////////////////////////////////////
  
static int WriteCollectionCommit (TRI_transaction_collection_t* trxCollection) {
  TRI_doc_commit_transaction_marker_t* commitMarker;
  TRI_document_collection_t* document;
  TRI_transaction_t* trx;
  TRI_df_marker_t* result;
  int res;
  
  trx = trxCollection->_transaction;
  document = (TRI_document_collection_t*) trxCollection->_collection->_collection;

  // create the "commit transaction" marker
  res = TRI_CreateMarkerCommitTransaction(trx, &commitMarker);

  if (res != TRI_ERROR_NO_ERROR) {
    return res;
  }
  
  res = TRI_WriteMarkerDocumentCollection(document, 
                                          &commitMarker->base,
                                          commitMarker->base._size,
                                          NULL,
                                          &result, 
                                          trx->_waitForSync);
    
  TRI_Free(TRI_UNKNOWN_MEM_ZONE, commitMarker);

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// write a prepare marker for a collection
////////////////////////////////////////////////////////////////////////////////
  
static int WriteCollectionPrepare (TRI_transaction_collection_t* trxCollection) {
  TRI_doc_prepare_transaction_marker_t* prepareMarker;
  TRI_document_collection_t* document;
  TRI_transaction_t* trx;
  TRI_df_marker_t* result;
  int res;
  
  trx = trxCollection->_transaction;
  document = (TRI_document_collection_t*) trxCollection->_collection->_collection;

  // create the "prepare transaction" marker
  res = TRI_CreateMarkerPrepareTransaction(trx, &prepareMarker);

  if (res != TRI_ERROR_NO_ERROR) {
    return res;
  }
  
  res = TRI_WriteMarkerDocumentCollection(document, 
                                          &prepareMarker->base,
                                          prepareMarker->base._size,
                                          NULL,
                                          &result, 
                                          trx->_waitForSync);
    
  TRI_Free(TRI_UNKNOWN_MEM_ZONE, prepareMarker);

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief write all operations for a collection, 
/// wrapped inside begin...commit|abort markers
////////////////////////////////////////////////////////////////////////////////

static int WriteCollectionOperations (TRI_transaction_collection_t* trxCollection,
                                      size_t numCollections) {
  TRI_document_collection_t* document;
  TRI_transaction_t* trx;
  TRI_df_marker_t* result;
  TRI_doc_begin_transaction_marker_t* beginMarker;
  int res;
  size_t i, n;

  document = (TRI_document_collection_t*) trxCollection->_collection->_collection;
  trx = trxCollection->_transaction;
  n = trxCollection->_operations->_length;

  assert(n > 0);

  // create the "start transaction" marker
  res = TRI_CreateMarkerBeginTransaction(trx, &beginMarker, (uint32_t) numCollections);

  if (res != TRI_ERROR_NO_ERROR) {
    return res;
  }
    
  res = TRI_WriteMarkerDocumentCollection(document, 
                                          &beginMarker->base,
                                          beginMarker->base._size,
                                          NULL,
                                          &result, 
                                          false);
  
  TRI_Free(TRI_UNKNOWN_MEM_ZONE, beginMarker);
  
  if (res != TRI_ERROR_NO_ERROR) {
    return res;
  }

  // write the individual operations
  for (i = 0; i < n; ++i) {
    transaction_operation_t* trxOperation;

    trxOperation = TRI_AtVector(trxCollection->_operations, i);

    res = TRI_WriteOperationDocumentCollection(document, 
                                               trxOperation->_type,
                                               trxOperation->_newHeader, 
                                               trxOperation->_oldHeader, 
                                               &trxOperation->_oldData, 
                                               trxOperation->_marker, 
                                               trxOperation->_markerSize, 
                                               false);

    if (res != TRI_ERROR_NO_ERROR) {
      break;
    }
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief coordination struct for x-collection transactions
////////////////////////////////////////////////////////////////////////////////

typedef struct trx_coordinator_s {
  TRI_json_t*    _json;
  TRI_voc_key_t  _key;
  TRI_doc_mptr_t _mptr;
}
trx_coordinator_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief insert the id of the transaction into the _trx collection
////////////////////////////////////////////////////////////////////////////////

static int InsertTrxCallback (TRI_transaction_collection_t* trxCollection,
                              void* data) {
  TRI_shaped_json_t* shaped;
  TRI_primary_collection_t* primary;
  trx_coordinator_t* coordinator;
  int res;

  primary = (TRI_primary_collection_t*) trxCollection->_collection->_collection;
  coordinator = data;

  if (coordinator->_json == NULL) {
    return TRI_ERROR_OUT_OF_MEMORY;
  }

  shaped = TRI_ShapedJsonJson(primary->_shaper, coordinator->_json);

  if (shaped == NULL) {
    return TRI_ERROR_OUT_OF_MEMORY;
  }

  res = primary->insert(trxCollection, coordinator->_key, &coordinator->_mptr, TRI_DOC_MARKER_KEY_DOCUMENT, shaped, NULL, false, false);
  TRI_FreeShapedJson(primary->_shaper, shaped);

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove the id of the transaction from the _trx collection
////////////////////////////////////////////////////////////////////////////////

static int RemoveTrxCallback (TRI_transaction_collection_t* trxCollection,
                              void* data) {
  trx_coordinator_t* coordinator;
  TRI_primary_collection_t* primary;
  int res;

  primary = (TRI_primary_collection_t*) trxCollection->_collection->_collection;
  coordinator = data;

  res = primary->remove(trxCollection, coordinator->_key, NULL, false, true);

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief write all operations for a transaction
////////////////////////////////////////////////////////////////////////////////

static int WriteOperations (TRI_transaction_t* const trx) {
  int res;

  res = TRI_ERROR_NO_ERROR;
  
  if (trx->_hasOperations) {
    size_t i, j, n;
    size_t numCollections;

    n = trx->_collections._length;
    j = n;
    numCollections = CountWriteCollections(trx);
    
    TRI_ASSERT_MAINTAINER(numCollections > 0);

    TRI_ASSERT_MAINTAINER(n >= numCollections);

    for (i = 0; i < n; ++i) {
      TRI_transaction_collection_t* trxCollection;

      trxCollection = TRI_AtVectorPointer(&trx->_collections, i);

      if (trxCollection->_operations == NULL) {
        // no markers available for collection
        continue;
      }

      res = WriteCollectionOperations(trxCollection, numCollections);

      if (res != TRI_ERROR_NO_ERROR) {
        j = i;
        break;
      }
    }

    if (res != TRI_ERROR_NO_ERROR) {
      // something went wrong. now write abort markers...

      for (i = 0; i <= j; ++i) {
        TRI_transaction_collection_t* trxCollection;

        trxCollection = TRI_AtVectorPointer(&trx->_collections, i);

        if (trxCollection->_operations == NULL) {
          // no markers available for collection
          continue;
        }

        WriteCollectionAbort(trxCollection);
      }
    }
    else {
      // everything fine here. now write prepare and|or commit markers...
      for (i = 0; i < n; ++i) {
        TRI_transaction_collection_t* trxCollection;

        trxCollection = TRI_AtVectorPointer(&trx->_collections, i);

        if (trxCollection->_operations == NULL) {
          // no markers available for collection
          continue;
        }
        
        if (numCollections == 1) {
          // directly write commit marker
          res = WriteCollectionCommit(trxCollection);
        }
        else {
          // write prepare marker first
          res = WriteCollectionPrepare(trxCollection);
        }

        if (res != TRI_ERROR_NO_ERROR) {
          break;
        }
      }

      if (numCollections > 1 && res == TRI_ERROR_NO_ERROR) {
        trx_coordinator_t coordinator;
        memset(&coordinator, 0, sizeof(trx_coordinator_t));

        coordinator._json = TRI_CreateArrayJson(TRI_CORE_MEM_ZONE);
        // use trx id as the key
        coordinator._key = TRI_StringUInt64(trx->_id);

        res = TRI_ExecuteSingleOperationTransaction(trx->_context->_vocbase, 
                                                    TRI_TRANSACTION_COORDINATOR_COLLECTION, 
                                                    TRI_TRANSACTION_WRITE, 
                                                    InsertTrxCallback, 
                                                    &coordinator);

        if (res == TRI_ERROR_NO_ERROR) {
          // now write the final commit markers
          for (i = 0; i < n; ++i) {
            TRI_transaction_collection_t* trxCollection;

            trxCollection = TRI_AtVectorPointer(&trx->_collections, i);

            if (trxCollection->_operations == NULL) {
              // no markers available for collection
              continue;
            }

            res = WriteCollectionCommit(trxCollection);

            if (res != TRI_ERROR_NO_ERROR) {
              break;
            }
          }

          if (res == TRI_ERROR_NO_ERROR) {
            res = TRI_ExecuteSingleOperationTransaction(trx->_context->_vocbase, 
                                                        TRI_TRANSACTION_COORDINATOR_COLLECTION,
                                                        TRI_TRANSACTION_WRITE, 
                                                        RemoveTrxCallback, 
                                                        &coordinator);
          }
        }

        if (coordinator._key != NULL) {
          TRI_Free(TRI_CORE_MEM_ZONE, coordinator._key);
        }

        if (coordinator._json != NULL) {
          TRI_FreeJson(TRI_CORE_MEM_ZONE, coordinator._json);
        }
      }
    } 
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief rollback all operations for a collection
////////////////////////////////////////////////////////////////////////////////

static int RollbackCollectionOperations (TRI_transaction_collection_t* trxCollection) {
  TRI_document_collection_t* document;
  int res;
  size_t i;

  document = (TRI_document_collection_t*) trxCollection->_collection->_collection;
  i = trxCollection->_operations->_length;

  assert(i > 0);
  res = TRI_ERROR_NO_ERROR;

  // revert the individual operations
  while (i-- > 0) {
    transaction_operation_t* trxOperation;
    int r;

    trxOperation = TRI_AtVector(trxCollection->_operations, i);

    // note: rolling back an insert operation will also free the new header
    r = TRI_RollbackOperationDocumentCollection(document, 
                                                trxOperation->_type,
                                                trxOperation->_newHeader, 
                                                trxOperation->_oldHeader, 
                                                &trxOperation->_oldData); 

    if (r != TRI_ERROR_NO_ERROR) {
      LOG_ERROR("unable to rollback operation in collection");
      // we'll return the first error
      res = r;
    }
    
  }

  TRI_SetRevisionDocumentCollection(document, trxCollection->_originalRevision);
  
  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief rollback all operations for a transaction
////////////////////////////////////////////////////////////////////////////////

static void RollbackOperations (TRI_transaction_t* trx) {
  size_t i, n;

  TRI_ASSERT_MAINTAINER(trx->_hasOperations);
  
  n = trx->_collections._length;
  
  for (i = 0; i < n; ++i) {
    TRI_transaction_collection_t* trxCollection;

    trxCollection = TRI_AtVectorPointer(&trx->_collections, i);

    if (trxCollection->_operations == NULL) {
      continue;
    }

    RollbackCollectionOperations(trxCollection);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief free all operations for a collection
////////////////////////////////////////////////////////////////////////////////

static void FreeCollectionOperations (TRI_transaction_collection_t* trxCollection,
                                      bool wasCommitted) {
  TRI_document_collection_t* document;
  size_t i, n;

  n = trxCollection->_operations->_length;
  document = (TRI_document_collection_t*) trxCollection->_collection->_collection;

  for (i = 0; i < n; ++i) {
    transaction_operation_t* trxOperation = TRI_AtVector(trxCollection->_operations, i);
    
    if (wasCommitted) {
      if (trxOperation->_type == TRI_VOC_DOCUMENT_OPERATION_REMOVE) {
        document->_headers->release(document->_headers, trxOperation->_oldHeader);
      }
    }
    
    TRI_Free(TRI_UNKNOWN_MEM_ZONE, trxOperation->_marker);
  }
}
                
////////////////////////////////////////////////////////////////////////////////
/// @brief free all operations for a transaction
////////////////////////////////////////////////////////////////////////////////

static void FreeOperations (TRI_transaction_t* trx) {
  size_t i, n;

  TRI_ASSERT_MAINTAINER(trx->_hasOperations);
    
  n = trx->_collections._length;
  
  for (i = 0; i < n; ++i) {
    TRI_transaction_collection_t* trxCollection;

    trxCollection = TRI_AtVectorPointer(&trx->_collections, i);

    if (trxCollection->_operations == NULL) {
      continue;
    }

    FreeCollectionOperations(trxCollection, trx->_status == TRI_TRANSACTION_COMMITTED);

    TRI_FreeVector(TRI_UNKNOWN_MEM_ZONE, trxCollection->_operations);
    trxCollection->_operations = NULL;
  }

  trx->_hasOperations = false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief find a collection in the transaction's list of collections
////////////////////////////////////////////////////////////////////////////////

static TRI_transaction_collection_t* FindCollection (const TRI_transaction_t* const trx,
                                                     const TRI_voc_cid_t cid,
                                                     size_t* position) {
  size_t i, n;

  n = trx->_collections._length;
  
  for (i = 0; i < n; ++i) {
    TRI_transaction_collection_t* trxCollection = TRI_AtVectorPointer(&trx->_collections, i);
    
    if (cid < trxCollection->_cid) {
      // collection not found
      break;
    }

    if (cid == trxCollection->_cid) {
      // found
      return trxCollection;
    }

    // next
  }
    
  if (position != NULL) {
    // update the insert position if required
    *position = i;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a transaction collection container
////////////////////////////////////////////////////////////////////////////////

static TRI_transaction_collection_t* CreateCollection (TRI_transaction_t* trx,
                                                       const TRI_voc_cid_t cid,
                                                       const TRI_transaction_type_e accessType,
                                                       const int nestingLevel) {
  TRI_transaction_collection_t* trxCollection;
#if 0
  TRI_transaction_collection_global_t* globalInstance;
  
  globalInstance = GetGlobalInstance(trx->_context, cid, true);

  if (globalInstance == NULL) {
    return NULL;
  }
#endif

  trxCollection = TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, sizeof(TRI_transaction_collection_t), false);

  if (trxCollection == NULL) {
    // OOM
    return NULL;
  }

  // initialise collection properties
  trxCollection->_transaction      = trx;
  trxCollection->_cid              = cid;
  trxCollection->_accessType       = accessType;
  trxCollection->_nestingLevel     = nestingLevel;
  trxCollection->_collection       = NULL;
#if 0
  trxCollection->_globalInstance   = globalInstance;
#endif
  trxCollection->_operations       = NULL;
  trxCollection->_originalRevision = 0;
  trxCollection->_locked           = false;
  trxCollection->_waitForSync      = false;

  return trxCollection;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief free a transaction collection container
////////////////////////////////////////////////////////////////////////////////

static void FreeCollection (TRI_transaction_collection_t* trxCollection) {
  assert(trxCollection != NULL);
  assert(trxCollection->_operations == NULL);

  TRI_Free(TRI_UNKNOWN_MEM_ZONE, trxCollection);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief lock a collection
////////////////////////////////////////////////////////////////////////////////

static int LockCollection (TRI_transaction_collection_t* trxCollection,
                           const TRI_transaction_type_e type,
                           const int nestingLevel) {
  TRI_primary_collection_t* primary;
  TRI_transaction_t* trx;
  int res;

  assert(trxCollection != NULL);

  trx = trxCollection->_transaction;
  
  if ((trx->_hints & (TRI_transaction_hint_t) TRI_TRANSACTION_HINT_LOCK_NEVER) != 0) {
    // never lock
    return TRI_ERROR_NO_ERROR;
  }

  assert(trxCollection->_collection != NULL);
  assert(trxCollection->_collection->_collection != NULL);
  assert(trxCollection->_locked == false);

  primary = trxCollection->_collection->_collection;

  if (type == TRI_TRANSACTION_READ) {
    LOG_TRX(trx,
            nestingLevel, 
            "read-locking collection %llu", 
            (unsigned long long) trxCollection->_cid);
    if (trx->_timeout == 0) {
      res = primary->beginRead(primary); 
    }
    else {
      res = primary->beginReadTimed(primary, trx->_timeout, TRI_TRANSACTION_DEFAULT_SLEEP_DURATION);
    }
  }
  else {
    LOG_TRX(trx,
            nestingLevel, 
            "write-locking collection %llu", 
            (unsigned long long) trxCollection->_cid);
    if (trx->_timeout == 0) {
      res = primary->beginWrite(primary); 
    }
    else {
      res = primary->beginWriteTimed(primary, trx->_timeout, TRI_TRANSACTION_DEFAULT_SLEEP_DURATION);
    }
  }

  if (res == TRI_ERROR_NO_ERROR) {
    trxCollection->_locked = true;
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief unlock a collection
////////////////////////////////////////////////////////////////////////////////

static int UnlockCollection (TRI_transaction_collection_t* trxCollection,
                             const TRI_transaction_type_e type,
                             const int nestingLevel) {
  TRI_primary_collection_t* primary;

  assert(trxCollection != NULL);
  
  if ((trxCollection->_transaction->_hints & (TRI_transaction_hint_t) TRI_TRANSACTION_HINT_LOCK_NEVER) != 0) {
    // never unlock
    return TRI_ERROR_NO_ERROR;
  }

  assert(trxCollection->_collection != NULL);
  assert(trxCollection->_collection->_collection != NULL);
  assert(trxCollection->_locked == true);

  primary = trxCollection->_collection->_collection;

  if (type == TRI_TRANSACTION_READ) {
    LOG_TRX(trxCollection->_transaction,
            nestingLevel, 
            "read-unlocking collection %llu", 
            (unsigned long long) trxCollection->_cid);
    primary->endRead(primary);
  }
  else {
    LOG_TRX(trxCollection->_transaction, 
            nestingLevel, 
            "write-unlocking collection %llu", 
            (unsigned long long) trxCollection->_cid);
    primary->endWrite(primary);
  }

  trxCollection->_locked = false;

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief notify all collections of a transaction status change
////////////////////////////////////////////////////////////////////////////////

#if 0

static int NotifyCollections (TRI_transaction_t* const trx,
                              const TRI_transaction_status_e status) {
  size_t i, n;

  n = trx->_collections._length;

  for (i = 0; i < n; ++i) {
    TRI_transaction_collection_t* trxCollection;

    trxCollection = (TRI_transaction_collection_t*) TRI_AtVectorPointer(&trx->_collections, i);

    if (trxCollection->_collection != NULL &&
        trxCollection->_collection->_collection != NULL) {
      TRI_primary_collection_t* primary;
      bool lock;
      
      primary = trxCollection->_collection->_collection;
      lock = ! trxCollection->_locked;

      if (lock) {
        primary->beginRead(primary);
      }

      // TODO: adjust the signature and pass the transaction in
      primary->notifyTransaction(primary, status);

      if (lock) {
        primary->endRead(primary);
      }
    }
  }

  return TRI_ERROR_NO_ERROR;
}

#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief use all participating collections of a transaction
////////////////////////////////////////////////////////////////////////////////

static int UseCollections (TRI_transaction_t* const trx,
                           const int nestingLevel) {
  size_t i, n;

  n = trx->_collections._length;

  // process collections in forward order
  for (i = 0; i < n; ++i) {
    TRI_transaction_collection_t* trxCollection;

    trxCollection = (TRI_transaction_collection_t*) TRI_AtVectorPointer(&trx->_collections, i);

    if (trxCollection->_nestingLevel != nestingLevel) {
      // only process our own collections
      continue;
    }
      
    if (trxCollection->_collection == NULL) {
      if ((trx->_hints & (TRI_transaction_hint_t) TRI_TRANSACTION_HINT_LOCK_NEVER) == 0) {
        LOG_TRX(trx, nestingLevel, "using collection %llu", (unsigned long long) trxCollection->_cid);
        trxCollection->_collection = TRI_UseCollectionByIdVocBase(trx->_context->_vocbase, trxCollection->_cid);
      }
      else {
        trxCollection->_collection = TRI_LookupCollectionByIdVocBase(trx->_context->_vocbase, trxCollection->_cid);
      }

      if (trxCollection->_collection == NULL) {
        // something went wrong
        return TRI_errno();
      }

      if (trxCollection->_collection->_collection == NULL) {
        // something went wrong
        return TRI_errno();
      }

      // store the waitForSync property
      trxCollection->_waitForSync = trxCollection->_collection->_collection->base._info._waitForSync;
    }

    assert(trxCollection->_collection != NULL);
    assert(trxCollection->_collection->_collection != NULL);
    
    if (! trxCollection->_locked &&
        ((trx->_hints & (TRI_transaction_hint_t) TRI_TRANSACTION_HINT_LOCK_ENTIRELY) != 0)) {
      int res = LockCollection(trxCollection, trxCollection->_accessType, nestingLevel);

      if (res != TRI_ERROR_NO_ERROR) {
        return res;
      }
    }
  }

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief release collection locks for a transaction
////////////////////////////////////////////////////////////////////////////////

static int ReleaseCollections (TRI_transaction_t* const trx,
                               const int nestingLevel) {
  size_t i;
  int res;

  res = TRI_ERROR_NO_ERROR;

  i = trx->_collections._length;

  // process collections in reverse order
  while (i-- > 0) {
    TRI_transaction_collection_t* trxCollection;

    trxCollection = TRI_AtVectorPointer(&trx->_collections, i);

    if (trxCollection->_locked && 
        (nestingLevel == 0 || trxCollection->_nestingLevel == nestingLevel)) {
      // unlock our own locks
      UnlockCollection(trxCollection, trxCollection->_accessType, nestingLevel);
    }

    // the top level transaction releases all collections
    if (nestingLevel == 0 && trxCollection->_collection != NULL) {
      if ((trx->_hints & (TRI_transaction_hint_t) TRI_TRANSACTION_HINT_LOCK_NEVER) == 0) {
        // unuse collection
        LOG_TRX(trx, nestingLevel, "unusing collection %llu", (unsigned long long) trxCollection->_cid);
  
        TRI_ReleaseCollectionVocBase(trx->_context->_vocbase, trxCollection->_collection);
      }

      trxCollection->_locked = false;
      trxCollection->_collection = NULL;
    }
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief update the status of a transaction
////////////////////////////////////////////////////////////////////////////////

static int UpdateTransactionStatus (TRI_transaction_t* const trx,
                                    const TRI_transaction_status_e status) {
  assert(trx->_status == TRI_TRANSACTION_CREATED || trx->_status == TRI_TRANSACTION_RUNNING);

  if (trx->_status == TRI_TRANSACTION_CREATED) {
    assert(status == TRI_TRANSACTION_RUNNING || status == TRI_TRANSACTION_FAILED);
  }
  else if (trx->_status == TRI_TRANSACTION_RUNNING) {
    assert(status == TRI_TRANSACTION_COMMITTED || status == TRI_TRANSACTION_ABORTED);
  }

  if (status == TRI_TRANSACTION_RUNNING ||
      status == TRI_TRANSACTION_ABORTED ||
      status == TRI_TRANSACTION_COMMITTED) {
    // notify all participating collections about the status change
    // currently not necessary. TODO: re-add later with concurrent indexes
#if 0
    UpdateGlobalStats(trx, status);
#endif

    // update the global statistics
    // currently not necessary. TODO: re-add later with concurrent indexes
#if 0
    NotifyCollections(trx, status);
#endif
  }

  trx->_status = status;

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new transaction container
////////////////////////////////////////////////////////////////////////////////

TRI_transaction_t* TRI_CreateTransaction (TRI_transaction_context_t* const context,
                                          double timeout,
                                          bool waitForSync) {
  TRI_transaction_t* trx;

  trx = (TRI_transaction_t*) TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, sizeof(TRI_transaction_t), false);

  if (trx == NULL) {
    // out of memory
    return NULL;
  }

  trx->_context       = context;

  // note: the real transaction id will be acquired on transaction start
  trx->_id            = 0; 
  
  trx->_status        = TRI_TRANSACTION_CREATED;
  trx->_type          = TRI_TRANSACTION_READ;
  trx->_hints         = 0;
  trx->_nestingLevel  = 0;
  trx->_timeout       = TRI_TRANSACTION_DEFAULT_LOCK_TIMEOUT;
  trx->_hasOperations = false;
  trx->_waitForSync   = waitForSync;

  if (timeout > 0.0) {
    trx->_timeout     = (uint64_t) (timeout * 1000000.0);
  }
  else if (timeout == 0.0) {
    trx->_timeout     = (uint64_t) 0;
  }

  TRI_InitVectorPointer2(&trx->_collections, TRI_UNKNOWN_MEM_ZONE, 2);

  return trx;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief free a transaction container
////////////////////////////////////////////////////////////////////////////////

void TRI_FreeTransaction (TRI_transaction_t* const trx) {
  size_t i;

  assert(trx != NULL);

  if (trx->_status == TRI_TRANSACTION_RUNNING) {
    TRI_AbortTransaction(trx, 0);
  }

  // free all collections
  i = trx->_collections._length;
  while (i-- > 0) {
    TRI_transaction_collection_t* trxCollection = TRI_AtVectorPointer(&trx->_collections, i);

    FreeCollection(trxCollection);
  }

  TRI_DestroyVectorPointer(&trx->_collections);

  TRI_Free(TRI_UNKNOWN_MEM_ZONE, trx);
}

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
/// @brief return the collection from a transaction
////////////////////////////////////////////////////////////////////////////////

bool TRI_WasSynchronousCollectionTransaction (TRI_transaction_t const* trx,
                                              const TRI_voc_cid_t cid) {
  
  TRI_transaction_collection_t* trxCollection;
  
  assert(trx->_status == TRI_TRANSACTION_RUNNING ||
         trx->_status == TRI_TRANSACTION_ABORTED ||
         trx->_status == TRI_TRANSACTION_COMMITTED);
   
  trxCollection = FindCollection(trx, cid, NULL);
  
  if (trxCollection == NULL || trxCollection->_collection == NULL) {
    // not found or not opened. probably a mistake made by the caller
    return false;
  }

  return trxCollection->_waitForSync;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the collection from a transaction
////////////////////////////////////////////////////////////////////////////////

TRI_transaction_collection_t* TRI_GetCollectionTransaction (TRI_transaction_t const* trx,
                                                            const TRI_voc_cid_t cid,
                                                            const TRI_transaction_type_e accessType) {
  
  TRI_transaction_collection_t* trxCollection;
  
  assert(trx->_status == TRI_TRANSACTION_CREATED ||
         trx->_status == TRI_TRANSACTION_RUNNING);
   
  trxCollection = FindCollection(trx, cid, NULL);
  
  if (trxCollection == NULL) {
    // not found
    return NULL;
  }
  
  if (trxCollection->_collection == NULL) {
    if ((trxCollection->_transaction->_hints & (TRI_transaction_hint_t) TRI_TRANSACTION_HINT_LOCK_NEVER) == 0) {
      // not opened. probably a mistake made by the caller
      return NULL;
    }
    else {
      // ok
    }
  }
  
  // check if access type matches
  if (accessType == TRI_TRANSACTION_WRITE && trxCollection->_accessType == TRI_TRANSACTION_READ) {
    // type doesn't match. probably also a mistake by the caller
    return NULL;
  }

  return trxCollection;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief add a collection to a transaction
////////////////////////////////////////////////////////////////////////////////

int TRI_AddCollectionTransaction (TRI_transaction_t* const trx,
                                  const TRI_voc_cid_t cid,
                                  const TRI_transaction_type_e accessType,
                                  const int nestingLevel) {

  TRI_transaction_collection_t* trxCollection;
  size_t position;
    
  LOG_TRX(trx, nestingLevel, "adding collection %llu", (unsigned long long) cid);
          
  // upgrade transaction type if required
  if (nestingLevel == 0) {
    assert(trx->_status == TRI_TRANSACTION_CREATED);

    if (accessType == TRI_TRANSACTION_WRITE && 
        trx->_type == TRI_TRANSACTION_READ) {
      // if one collection is written to, the whole transaction becomes a write-transaction
      trx->_type = TRI_TRANSACTION_WRITE;
    }
  }

  // check if we already have got this collection in the _collections vector
  trxCollection = FindCollection(trx, cid, &position);

  if (trxCollection != NULL) {
    // collection is already contained in vector
   
    if (accessType == TRI_TRANSACTION_WRITE && trxCollection->_accessType != accessType) {
      if (nestingLevel > 0) {
        // trying to write access a collection that is only marked with read-access 
        return TRI_ERROR_TRANSACTION_UNREGISTERED_COLLECTION;
      }

      assert(nestingLevel == 0);

      // upgrade collection type to write-access
      trxCollection->_accessType = TRI_TRANSACTION_WRITE;
    } 

    if (nestingLevel < trxCollection->_nestingLevel) {
      trxCollection->_nestingLevel = nestingLevel;
    }

    // all correct
    return TRI_ERROR_NO_ERROR;
  }


  // collection not found.

  if (nestingLevel > 0 && accessType == TRI_TRANSACTION_WRITE) {
    // trying to write access a collection in an embedded transaction
    return TRI_ERROR_TRANSACTION_UNREGISTERED_COLLECTION;
  }

  // collection was not contained. now create and insert it
  trxCollection = CreateCollection(trx, cid, accessType, nestingLevel);

  if (trxCollection == NULL) {
    // out of memory
    return TRI_ERROR_OUT_OF_MEMORY;
  }

  // insert collection at the correct position
  if (TRI_InsertVectorPointer(&trx->_collections, trxCollection, position) != TRI_ERROR_NO_ERROR) {
    FreeCollection(trxCollection);

    return TRI_ERROR_OUT_OF_MEMORY;
  }

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief request a lock for a collection
////////////////////////////////////////////////////////////////////////////////

int TRI_LockCollectionTransaction (TRI_transaction_collection_t* trxCollection,
                                   const TRI_transaction_type_e accessType,
                                   const int nestingLevel) {

  if (accessType == TRI_TRANSACTION_WRITE && trxCollection->_accessType != TRI_TRANSACTION_WRITE) {
    // wrong lock type
    return TRI_ERROR_INTERNAL;
  }

  if (trxCollection->_locked) {
    // already locked
    return TRI_ERROR_NO_ERROR;
  }

  return LockCollection(trxCollection, accessType, nestingLevel);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief request an unlock for a collection
////////////////////////////////////////////////////////////////////////////////

int TRI_UnlockCollectionTransaction (TRI_transaction_collection_t* trxCollection,
                                     const TRI_transaction_type_e accessType,
                                     const int nestingLevel) {
  
  if (accessType == TRI_TRANSACTION_WRITE && trxCollection->_accessType != TRI_TRANSACTION_WRITE) {
    // wrong lock type
    return TRI_ERROR_INTERNAL;
  }

  if (! trxCollection->_locked) {
    // already unlocked
    return TRI_ERROR_NO_ERROR;
  }

  return UnlockCollection(trxCollection, accessType, nestingLevel);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief check if a collection is locked in a transaction
////////////////////////////////////////////////////////////////////////////////

bool TRI_IsLockedCollectionTransaction (TRI_transaction_collection_t* trxCollection,
                                        const TRI_transaction_type_e accessType,
                                        const int nestingLevel) {

  if (accessType == TRI_TRANSACTION_WRITE && trxCollection->_accessType != TRI_TRANSACTION_WRITE) {
    // wrong lock type
    LOG_WARNING("logic error. checking wrong lock type");
    return false;
  }

  return trxCollection->_locked;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief add an operation for a transaction collection
////////////////////////////////////////////////////////////////////////////////

int TRI_AddOperationCollectionTransaction (TRI_transaction_collection_t* trxCollection,
                                           TRI_voc_document_operation_e type,
                                           TRI_doc_mptr_t* newHeader,
                                           TRI_doc_mptr_t* oldHeader,
                                           TRI_doc_mptr_t* oldData,
                                           TRI_df_marker_t* marker,
                                           size_t totalSize,
                                           bool syncRequested,
                                           bool* written) {
  TRI_transaction_t* trx;
  TRI_primary_collection_t* primary;
  int res;
  
  trx = trxCollection->_transaction;
  primary = trxCollection->_collection->_collection;

  if (trxCollection->_originalRevision == 0) {
    trxCollection->_originalRevision = primary->base._info._tick;
  }
 
  // the tick value of a marker must always be greater than the tick value of any other
  // existing marker in the collection 
  TRI_SetRevisionDocumentCollection((TRI_document_collection_t*) primary, marker->_tick);
  
  if (trx->_hints & ((TRI_transaction_hint_t) TRI_TRANSACTION_HINT_SINGLE_OPERATION)) {
    // just one operation in the transaction. we can write the marker directly
    // TODO: error checking
    res = TRI_WriteOperationDocumentCollection((TRI_document_collection_t*) primary,
                                               type,
                                               newHeader,
                                               oldHeader,
                                               oldData,
                                               marker, 
                                               totalSize,
                                               syncRequested || trxCollection->_waitForSync || trx->_waitForSync);
    *written = true;
  }
  else {
    trx->_hasOperations = true;

    res = AddCollectionOperation(trxCollection, type, newHeader, oldHeader, oldData, marker, totalSize);
  
    *written = false;
  }
    
  if (syncRequested) {
    trxCollection->_waitForSync = true;
    trx->_waitForSync = true;
  }
  else if (trxCollection->_waitForSync) {
    trx->_waitForSync = true;
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief get a transaction's id
////////////////////////////////////////////////////////////////////////////////

TRI_voc_tid_t TRI_GetIdTransaction (const TRI_transaction_t* trx) {
  return trx->_id;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief get a transaction's id for writing into a marker
/// this will return 0 if the operation is standalone
////////////////////////////////////////////////////////////////////////////////

TRI_voc_tid_t TRI_GetMarkerIdTransaction (const TRI_transaction_t* trx) {
  if (trx->_hints & ((TRI_transaction_hint_t) TRI_TRANSACTION_HINT_SINGLE_OPERATION)) {
    return 0;
  }

  return TRI_GetIdTransaction(trx);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief start a transaction
////////////////////////////////////////////////////////////////////////////////

int TRI_BeginTransaction (TRI_transaction_t* const trx, 
                          TRI_transaction_hint_t hints,
                          const int nestingLevel) {
  int res;

  LOG_TRX(trx, nestingLevel, "%s transaction", "beginning");

  if (nestingLevel == 0) {
    assert(trx->_status == TRI_TRANSACTION_CREATED);

    // get a new id
    trx->_id = TRI_NewTickVocBase();

    // update hints
    if (trx->_collections._length == 1) {
      hints |= (TRI_transaction_hint_t) TRI_TRANSACTION_HINT_SINGLE_COLLECTION;
    }

    if (trx->_type == TRI_TRANSACTION_READ) {
      hints |= (TRI_transaction_hint_t) TRI_TRANSACTION_HINT_READ_ONLY;
    }

    trx->_hints = hints;
  }
  else {
    assert(trx->_status == TRI_TRANSACTION_RUNNING);
  }

  res = UseCollections(trx, nestingLevel);

  if (res == TRI_ERROR_NO_ERROR) {
    // all valid
    if (nestingLevel == 0) {
      UpdateTransactionStatus(trx, TRI_TRANSACTION_RUNNING);
    }
  }
  else {
    // something is wrong
    if (nestingLevel == 0) {
      UpdateTransactionStatus(trx, TRI_TRANSACTION_FAILED);
    }

    // free what we have got so far
    ReleaseCollections(trx, nestingLevel);
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief commit a transaction
////////////////////////////////////////////////////////////////////////////////

int TRI_CommitTransaction (TRI_transaction_t* const trx,
                           const int nestingLevel) {
  int res;

  LOG_TRX(trx, nestingLevel, "%s transaction", "committing");

  assert(trx->_status == TRI_TRANSACTION_RUNNING);

  res = TRI_ERROR_NO_ERROR;

  if (nestingLevel == 0) {
    if (trx->_hasOperations) {
      res = WriteOperations(trx);
      FreeOperations(trx);
    }

    if (res != TRI_ERROR_NO_ERROR) {
      // writing markers has failed
      res = UpdateTransactionStatus(trx, TRI_TRANSACTION_ABORTED);
    }
    else {
      res = UpdateTransactionStatus(trx, TRI_TRANSACTION_COMMITTED);
    }
  }

  ReleaseCollections(trx, nestingLevel);

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief abort and rollback a transaction
////////////////////////////////////////////////////////////////////////////////

int TRI_AbortTransaction (TRI_transaction_t* const trx,
                          const int nestingLevel) {
  int res;

  LOG_TRX(trx, nestingLevel, "%s transaction", "aborting");

  assert(trx->_status == TRI_TRANSACTION_RUNNING);

  if (nestingLevel == 0) {
    if (trx->_hasOperations) {
      RollbackOperations(trx);
      FreeOperations(trx);
    }

    res = UpdateTransactionStatus(trx, TRI_TRANSACTION_ABORTED);
  }
  else {
    res = TRI_ERROR_NO_ERROR;
  }

  ReleaseCollections(trx, nestingLevel);

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                               TRANSACTION HELPERS
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief execute a single operation wrapped in a transaction
/// the actual operation can be specified using a callback function
////////////////////////////////////////////////////////////////////////////////

int TRI_ExecuteSingleOperationTransaction (TRI_vocbase_t* vocbase,
                                           const char* collectionName,
                                           TRI_transaction_type_e accessType,
                                           int (*callback)(TRI_transaction_collection_t*, void*),
                                           void* data) {
  TRI_transaction_t* trx;
  TRI_vocbase_col_t* collection;
  TRI_voc_cid_t cid;
  int res;

  collection = TRI_LookupCollectionByNameVocBase(vocbase, collectionName);

  if (collection == NULL) {
    return TRI_ERROR_ARANGO_COLLECTION_NOT_FOUND;
  }

  cid = collection->_cid;

  // write the data using a one-operation transaction  
  trx = TRI_CreateTransaction(vocbase->_transactionContext, 0.0, false);

  if (trx == NULL) {
    return TRI_ERROR_OUT_OF_MEMORY;
  }

  // add the collection
  res = TRI_AddCollectionTransaction(trx, cid, accessType, TRI_TRANSACTION_TOP_LEVEL);

  if (res == TRI_ERROR_NO_ERROR) {
    res = TRI_BeginTransaction(trx, (TRI_transaction_hint_t) TRI_TRANSACTION_HINT_SINGLE_OPERATION, TRI_TRANSACTION_TOP_LEVEL);

    if (res == TRI_ERROR_NO_ERROR) {
      TRI_transaction_collection_t* trxCollection;

      trxCollection = TRI_GetCollectionTransaction(trx, cid, accessType);

      if (trxCollection != NULL) {
        // execute the callback
        res = callback(trxCollection, data);

        if (res == TRI_ERROR_NO_ERROR) {
          res = TRI_CommitTransaction(trx, TRI_TRANSACTION_TOP_LEVEL);
        }
        else {
          res = TRI_AbortTransaction(trx, TRI_TRANSACTION_TOP_LEVEL);
        }
      }
      else {
        res = TRI_ERROR_ARANGO_COLLECTION_NOT_FOUND;
      }
    }
  }

  TRI_FreeTransaction(trx);

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                               TRANSACTION MARKERS
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief create a "begin" marker
////////////////////////////////////////////////////////////////////////////////

int TRI_CreateMarkerBeginTransaction (TRI_transaction_t* trx,
                                      TRI_doc_begin_transaction_marker_t** result,
                                      uint32_t numCollections) {
  TRI_doc_begin_transaction_marker_t* marker;
  TRI_voc_tick_t tick;

  *result = NULL;
  marker = TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, sizeof(TRI_doc_begin_transaction_marker_t), false);

  if (marker == NULL) {
    return TRI_ERROR_OUT_OF_MEMORY;
  }

  tick = TRI_NewTickVocBase();
  TRI_InitMarker(&marker->base, TRI_DOC_MARKER_BEGIN_TRANSACTION, sizeof(TRI_doc_begin_transaction_marker_t), tick);
  marker->_tid = trx->_id;
  marker->_numCollections = numCollections;

  *result = marker;

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a "commit" marker
////////////////////////////////////////////////////////////////////////////////

int TRI_CreateMarkerCommitTransaction (TRI_transaction_t* trx,
                                       TRI_doc_commit_transaction_marker_t** result) {
  TRI_doc_commit_transaction_marker_t* marker;
  TRI_voc_tick_t tick;

  *result = NULL;
  marker = TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, sizeof(TRI_doc_commit_transaction_marker_t), false);

  if (marker == NULL) {
    return TRI_ERROR_OUT_OF_MEMORY;
  }

  tick = TRI_NewTickVocBase();
  TRI_InitMarker(&marker->base, TRI_DOC_MARKER_COMMIT_TRANSACTION, sizeof(TRI_doc_commit_transaction_marker_t), tick);
  marker->_tid = trx->_id;

  *result = marker;

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create an "abort" marker
////////////////////////////////////////////////////////////////////////////////

int TRI_CreateMarkerAbortTransaction (TRI_transaction_t* trx,
                                      TRI_doc_abort_transaction_marker_t** result) {
  TRI_doc_abort_transaction_marker_t* marker;
  TRI_voc_tick_t tick;

  *result = NULL;
  marker = TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, sizeof(TRI_doc_commit_transaction_marker_t), false);

  if (marker == NULL) {
    return TRI_ERROR_OUT_OF_MEMORY;
  }

  tick = TRI_NewTickVocBase();
  TRI_InitMarker(&marker->base, TRI_DOC_MARKER_ABORT_TRANSACTION, sizeof(TRI_doc_abort_transaction_marker_t), tick);
  marker->_tid = trx->_id;

  *result = marker;

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a "prepare commit" marker
////////////////////////////////////////////////////////////////////////////////

int TRI_CreateMarkerPrepareTransaction (TRI_transaction_t* trx,
                                        TRI_doc_prepare_transaction_marker_t** result) {
  TRI_doc_prepare_transaction_marker_t* marker;
  TRI_voc_tick_t tick;

  *result = NULL;
  marker = TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, sizeof(TRI_doc_prepare_transaction_marker_t), false);

  if (marker == NULL) {
    return TRI_ERROR_OUT_OF_MEMORY;
  }

  tick = TRI_NewTickVocBase();
  TRI_InitMarker(&marker->base, TRI_DOC_MARKER_PREPARE_TRANSACTION, sizeof(TRI_doc_prepare_transaction_marker_t), tick);
  marker->_tid = trx->_id;

  *result = marker;

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
