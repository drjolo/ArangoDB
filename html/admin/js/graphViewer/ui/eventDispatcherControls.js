/*jslint indent: 2, nomen: true, maxlen: 100, white: true  plusplus: true */
/*global $, _, d3*/
/*global document*/
/*global modalDialogHelper, uiComponentsHelper */
/*global EventDispatcher, EventLibrary*/
////////////////////////////////////////////////////////////////////////////////
/// @brief Graph functionality
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
/// @author Michael Hackstein
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////
function EventDispatcherControls(list, nodeShaper, edgeShaper, dispatcherConfig) {
  "use strict";
  
  if (list === undefined) {
    throw "A list element has to be given.";
  }
  if (nodeShaper === undefined) {
    throw "The NodeShaper has to be given.";
  }
  if (edgeShaper === undefined) {
    throw "The EdgeShaper has to be given.";
  }
  
  var self = this,
    baseClass = "event",
    eventlib = new EventLibrary(),
    dispatcher = new EventDispatcher(nodeShaper, edgeShaper, dispatcherConfig),
    
    
    createButton = function(title, callback) {
      uiComponentsHelper.createButton(
        baseClass,
        list,
        title,
        "control_" + title,
        callback
      );
    },
    rebindNodes = function(actions) {
      dispatcher.rebind("nodes", actions);
    },
    rebindEdges = function(actions) {
      dispatcher.rebind("edges", actions);
    };
  
  this.addControlDrag = function() {
    var prefix = "control_drag",
      idprefix = prefix + "_",
      callback = function() {
        rebindNodes( {
          drag: dispatcher.events.DRAG
        });
        rebindEdges();
        
        
      };
    createButton("drag", callback);
  };
  
  this.addControlEdit = function() {
    var prefix = "control_edit",
      idprefix = prefix + "_",
      nodeCallback = function(n) {
        modalDialogHelper.createModalEditDialog(
          "Edit Node " + n._id,
          "control_node_edit_",
          n,
          function(newData) {
            dispatcher.events.PATCHNODE(n, newData, function() {
              $("#control_node_edit_modal").modal('hide');
            })();
          }
        );
      },
      edgeCallback = function(e) {
        modalDialogHelper.createModalEditDialog(
          "Edit Edge " + e.source._id + "->" + e.target._id,
          "control_edge_edit_",
          e,
          function(newData) {
            dispatcher.events.PATCHEDGE(e, newData, function() {
              $("#control_edge_edit_modal").modal('hide');
            })();
          }
        );
      },
      callback = function() {
        rebindNodes({click: nodeCallback});
        rebindEdges({click: edgeCallback});
      };
    createButton("edit", callback);
  };
  
  this.addControlExpand = function() {
    var prefix = "control_expand",
      idprefix = prefix + "_",
      callback = function() {
        rebindNodes({click: dispatcher.events.EXPAND});
        rebindEdges();
      };
    createButton("expand", callback);
  };
  
  this.addControlDelete = function() {
    var prefix = "control_delete",
      idprefix = prefix + "_",
      callback = function() {
        rebindNodes({click: dispatcher.events.DELETENODE(function() {
          nodeShaper.reshapeNodes();
        })});
        rebindEdges({click: dispatcher.events.DELETEEDGE(function() {
          edgeShaper.reshapeEdges();
        })});
      };
    createButton("delete", callback);
  };
  
  this.addControlConnect = function() {
    var prefix = "control_connect",
      idprefix = prefix + "_",
      callback = function() {
        
        rebindNodes({
          mousedown: dispatcher.events.STARTCREATEEDGE(),
          mouseup: dispatcher.events.FINISHCREATEEDGE(function(){
            edgeShaper.reshapeEdges();
          })
        });
        rebindEdges();
      };
    createButton("connect", callback);
  };
  
  
  
  this.addAll = function () {
    self.addControlDrag();
    self.addControlEdit();
    self.addControlExpand();
    self.addControlDelete();
    self.addControlConnect();
  };
  
}