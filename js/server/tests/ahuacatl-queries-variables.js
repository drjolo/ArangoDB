////////////////////////////////////////////////////////////////////////////////
/// @brief tests for query language, variable access
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
/// @author Copyright 2012, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

var jsunity = require("jsunity");
var ArangoError = require("org/arangodb/arango-error").ArangoError; 
var QUERY = require("internal").AQL_QUERY;

////////////////////////////////////////////////////////////////////////////////
/// @brief test suite
////////////////////////////////////////////////////////////////////////////////

function ahuacatlQueryVariablesTestSuite () {
  var users1 = null;
  var users2 = null;
  var airports = null;

////////////////////////////////////////////////////////////////////////////////
/// @brief execute a given query
////////////////////////////////////////////////////////////////////////////////

  function executeQuery (query) {
    var cursor = QUERY(query, undefined);
    if (cursor instanceof ArangoError) {
      print(query, cursor.errorMessage);
    }
    assertFalse(cursor instanceof ArangoError);
    return cursor;
  }

////////////////////////////////////////////////////////////////////////////////
/// @brief execute a given query and return the results as an array
////////////////////////////////////////////////////////////////////////////////

  function getQueryResults (query) {
    var result = executeQuery(query).getRows();
    var results = [ ];

    for (var i in result) {
      if (!result.hasOwnProperty(i)) {
        continue;
      }

      results.push(result[i]);
    }

    return results;
  }


  return {

////////////////////////////////////////////////////////////////////////////////
/// @brief set up
////////////////////////////////////////////////////////////////////////////////

    setUp : function () {
      users1 = [ 
        { "id" : 1, "name" : "Max", "hobbies" : [ { "type" : "swimming" }, { "type" : "skating" } ], "friends": [ { "name" : "Peter", "id" : 1 } ] }, 
        { "id" : 2, "name" : "Vanessa", "hobbies" : [ { "type" : "running" }, { "type" : "cycling" } ], "friends" : [ { "name" : "Peter", "id" : 3 }, { "name" : "Max", "id" : 3 } ] }, 
        { "id" : 3, "name" : "Peter", "hobbies": [ ], "friends" : [ { "name" : "Peter" } ] } 
      ];

      users2 = [ 
        { "id" : 1, "name" : "Max", "address" : { "home" : { "street" : "arango road", "zip" : "abcde", "phone" : [ { "type" : "mobile", "number" : "555-1234567" }, { "type" : "fax", "number" : "555-2345678" } ] }, "work": { "street" : "cantaloupe way", "zip" : "xyzab", "phone" : [ { "type" : "landline", "number" : "555-5555555" } ] } } }, 
        { "id" : 2, "name" : "Vanessa", "address" : { "home" : { "street" : "one-way loop", "zip" : "4e2af", "phone" : [ { "type" : "landline", "number" : "555-4352367" } ] }, "work": { "street" : "workers ave", "zip" : "4e2af", "phone" : [ { "type" : "landline", "number" : "555-2214212" } ] } } }, 
        { "id" : 3, "name" : "Peter", "address" : { "home" : { "street" : "theather drive", "zip" : "99998", "phone" : [ { "type" : "mobile", "number" : "555-9624218" }, { "type" : "fax", "number" : "555-4425742" }, { "type" : "landline", "number" : "555-3485385" } ] } } }
      ];

      airports = [
        { "continent" : { "name" : "Europe", "countries" : [ 
          { "name" : "DE", "airports" : [ { "name" : "CGN" }, { "name" : "DTM" }, { "name" : "DUS" } , { "name" : "MUC" }, { "name" : "FRA" }, { "name" : "TXL" }, { "name" : "THF" } ] },
          { "name" : "UK", "airports" : [ { "name" : "LHR" }, { "name" : "LGW" }, { "name" : "STN" } , { "name" : "LCY" }, { "name" : "MAN" } ] }, 
          { "name" : "FR", "airports" : [ { "name" : "CDG" }, { "name" : "ORY" }, { "name" : "LYN" } , { "name" : "MRS" } ] } 
        ] } },
        { "continent" : { "name" : "America", "countries" : [ 
          { "name" : "US", "airports" : [ { "name" : "LGA" }, { "name" : "JFK" }, { "name" : "SFO" }, { "name" : "JER" }, { "name" : "ATL" } ] },
          { "name" : "CA", "airports" : [ { "name" : "YTO" }, { "name" : "YVR" }, { "name" : "YYC" } ] },
          { "name" : "CO", "airports" : [ { "name" : "BOG" } ] }
        ] } }, 
        { "continent" : { "name" : "Asia", "countries" : [ 
          { "name" : "JP", "airports" : [ { "name" : "NRT" }, { "name" : "HND" } , { "name" : "OKD" }, { "name" : "OKA" } ] }
        ] } } 
      ];

    },

////////////////////////////////////////////////////////////////////////////////
/// @brief tear down
////////////////////////////////////////////////////////////////////////////////

    tearDown : function () {
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief return an expanded variable
////////////////////////////////////////////////////////////////////////////////

    testListExpansion1 : function () {
      var query = "FOR u IN " + JSON.stringify(users1) + " RETURN { \"name\" : u.name, \"likes\": u.hobbies[*].type }";
      var expected = [{ "name" : "Max", "likes" : ["swimming", "skating"] }, { "name" : "Vanessa", "likes" : ["running", "cycling"] }, { "name" : "Peter", "likes" : [ ] }]; 

      var actual = getQueryResults(query);
      assertEqual(expected, actual);
    },

                         
////////////////////////////////////////////////////////////////////////////////
/// @brief return an expanded variable
////////////////////////////////////////////////////////////////////////////////

    testListExpansion2 : function () {
      var query = "FOR u IN " + JSON.stringify(users1) + " RETURN { \"name\" : u.name, \"likes\": u.hobbies[*].type, \"friends\": u.friends[*].name }";
      var expected = [{ "name" : "Max", "likes" : ["swimming", "skating"], "friends" : ["Peter"] }, { "name" : "Vanessa", "likes" : ["running", "cycling"], "friends" : ["Peter", "Max"] }, { "name" : "Peter", "likes" : [ ], "friends" : ["Peter"] }];

      var actual = getQueryResults(query);
      assertEqual(expected, actual);
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief return an expanded variable
////////////////////////////////////////////////////////////////////////////////

    testListExpansion3 : function () {
      var query = "FOR u IN " + JSON.stringify(users2) + " RETURN { \"uid\" : u.id, \"phones\" : u.address.home.phone[*].number }";
      var expected = [{ "uid" : 1, "phones" : ["555-1234567", "555-2345678"] }, { "uid" : 2, "phones" : ["555-4352367"] }, { "uid" : 3, "phones" : ["555-9624218", "555-4425742", "555-3485385"] }];

      var actual = getQueryResults(query);
      assertEqual(expected, actual);
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief return an expanded variable
////////////////////////////////////////////////////////////////////////////////

    testListExpansion4 : function () {
      var query = "FOR a IN " + JSON.stringify(airports) + " RETURN a.continent.countries[*].name";
      var expected = [["DE", "UK", "FR"], ["US", "CA", "CO"], ["JP"]];

      var actual = getQueryResults(query);
      assertEqual(expected, actual);
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief return an expanded variable
////////////////////////////////////////////////////////////////////////////////

    testListExpansion5 : function () {
      var query = "FOR a IN " + JSON.stringify(airports) + " RETURN a.continent.countries[*].airports[0].name";
      var expected = [["CGN", "LHR", "CDG"], ["LGA", "YTO", "BOG"], ["NRT"]];

      var actual = getQueryResults(query);
      assertEqual(expected, actual);
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief return an expanded variable
////////////////////////////////////////////////////////////////////////////////

    testListExpansion6 : function () {
      var query = "FOR a IN " + JSON.stringify(airports) + " RETURN a.continent.countries[*].airports[*][0].name";
      var expected = [["CGN", "LHR", "CDG"], ["LGA", "YTO", "BOG"], ["NRT"]];

      var actual = getQueryResults(query);
      assertEqual(expected, actual);
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief return an expanded variable
////////////////////////////////////////////////////////////////////////////////

    testListExpansion7 : function () {
      var query = "FOR a IN " + JSON.stringify(airports) + " RETURN a.continent.countries[*].airports[*][1].name";
      var expected = [["DTM", "LGW", "ORY"], ["JFK", "YVR", null], ["HND"]];

      var actual = getQueryResults(query);
      assertEqual(expected, actual);
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief return a named variable
////////////////////////////////////////////////////////////////////////////////

    testNamedAccess1 : function () {
      var query = "FOR u IN " + JSON.stringify(users2) + " RETURN { \"name\" : u.name, \"addr\" : u.address }";
      var expected = [{ "name" : "Max", "addr" : { "home" : { "street" : "arango road", "zip" : "abcde", "phone" : [{ "type" : "mobile", "number" : "555-1234567" }, { "type" : "fax", "number" : "555-2345678" }] }, "work" : { "street" : "cantaloupe way", "zip" : "xyzab", "phone" : [{ "type" : "landline", "number" : "555-5555555" }] } } }, { "name" : "Vanessa", "addr" : { "home" : { "street" : "one-way loop", "zip" : "4e2af", "phone" : [{ "type" : "landline", "number" : "555-4352367" }] }, "work" : { "street" : "workers ave", "zip" : "4e2af", "phone" : [{ "type" : "landline", "number" : "555-2214212" }] } } }, { "name" : "Peter", "addr" : { "home" : { "street" : "theather drive", "zip" : "99998", "phone" : [{ "type" : "mobile", "number" : "555-9624218" }, { "type" : "fax", "number" : "555-4425742" }, { "type" : "landline", "number" : "555-3485385" }] } } }];

      var actual = getQueryResults(query);
      assertEqual(expected, actual);
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief return a named variable
////////////////////////////////////////////////////////////////////////////////

    testNamedAccess2 : function () {
      var query = "FOR u IN " + JSON.stringify(users2) + " RETURN { \"name\" : u.name, \"home\" : u.address.home }";
      var expected = [{ "name" : "Max", "home" : { "street" : "arango road", "zip" : "abcde", "phone" : [{ "type" : "mobile", "number" : "555-1234567" }, { "type" : "fax", "number" : "555-2345678" }] } }, { "name" : "Vanessa", "home" : { "street" : "one-way loop", "zip" : "4e2af", "phone" : [{ "type" : "landline", "number" : "555-4352367" }] } }, { "name" : "Peter", "home" : { "street" : "theather drive", "zip" : "99998", "phone" : [{ "type" : "mobile", "number" : "555-9624218" }, { "type" : "fax", "number" : "555-4425742" }, { "type" : "landline", "number" : "555-3485385" }] } }] ;

      var actual = getQueryResults(query);
      assertEqual(expected, actual);
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief return a named variable
////////////////////////////////////////////////////////////////////////////////

    testNamedAccess3 : function () {
      var query = "FOR u IN " + JSON.stringify(users2) + " RETURN { \"name\" : u.name, \"str\" : u.address.home.street }";
      var expected = [ { "name" : "Max", "str" : "arango road" }, { "name" : "Vanessa", "str" : "one-way loop" }, { "name" : "Peter", "str" : "theather drive" } ];

      var actual = getQueryResults(query);
      assertEqual(expected, actual);
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief return a named variable
////////////////////////////////////////////////////////////////////////////////

    testNamedAccess4 : function () {
      var query = "FOR a IN " + JSON.stringify(airports) + " RETURN a.continent.name";
      var expected = [ "Europe", "America", "Asia" ];

      var actual = getQueryResults(query);
      assertEqual(expected, actual);
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief return a numerically indexed variable
////////////////////////////////////////////////////////////////////////////////

    testIndexedAccess1 : function () {
      var query = "FOR u IN " + JSON.stringify(users1) + " RETURN u.hobbies[0]";
      var expected = [ { "type": "swimming" }, { "type" : "running" }, null ];

      var actual = getQueryResults(query);
      assertEqual(expected, actual);
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief return a numerically indexed variable
////////////////////////////////////////////////////////////////////////////////

    testIndexedAccess2 : function () {
      var query = "FOR u IN " + JSON.stringify(users1) + " RETURN u.hobbies[0].type";
      var expected = [ "swimming", "running", null ];

      var actual = getQueryResults(query);
      assertEqual(expected, actual);
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief return a numerically indexed variable
////////////////////////////////////////////////////////////////////////////////

    testIndexedAccess3 : function () {
      var query = "FOR u IN " + JSON.stringify(users2) + " RETURN { \"uid\" : u.id, \"phone\" : u.address.home.phone[0] }";
      var expected = [{ "uid" : 1, "phone" : { "type" : "mobile", "number" : "555-1234567" } }, { "uid" : 2, "phone" : { "type" : "landline", "number" : "555-4352367" } }, { "uid" : 3, "phone" : { "type" : "mobile", "number" : "555-9624218" } }];

      var actual = getQueryResults(query);
      assertEqual(expected, actual);
    },


////////////////////////////////////////////////////////////////////////////////
/// @brief return a numerically indexed variable
////////////////////////////////////////////////////////////////////////////////

    testIndexedAccess4 : function () {
      var query = "FOR u IN " + JSON.stringify(users2) + " RETURN { \"uid\" : u.id, \"phone\" : u.address.home.phone[1] }";
      var expected = [{ "uid" : 1, "phone" : { "type" : "fax", "number" : "555-2345678" } }, { "uid" : 2, "phone" : null }, { "uid" : 3, "phone" : { "type" : "fax", "number" : "555-4425742" } }];

      var actual = getQueryResults(query);
      assertEqual(expected, actual);
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief return a numerically indexed variable
////////////////////////////////////////////////////////////////////////////////

    testIndexedAccess5 : function () {
      var query = "FOR u IN " + JSON.stringify(users2) + " RETURN { \"uid\" : u.id, \"phoneType\" : u.address.home.phone[0].type, \"phoneNum\" : u.address.home.phone[0].number }";
      var expected = [{ "uid" : 1, "phoneType" : "mobile", "phoneNum" : "555-1234567" }, { "uid" : 2, "phoneType" : "landline", "phoneNum" : "555-4352367" }, { "uid" : 3, "phoneType" : "mobile", "phoneNum" : "555-9624218" }];

      var actual = getQueryResults(query);
      assertEqual(expected, actual);
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief return a numerically indexed variable
////////////////////////////////////////////////////////////////////////////////

    testIndexedAccess6 : function () {
      var query = "FOR u IN " + JSON.stringify(users2) + " RETURN { \"uid\" : u.id, \"phoneType\" : u.address.home.phone[1].type, \"phoneNum\" : u.address.home.phone[1].number }";
      var expected = [{ "uid" : 1, "phoneType" : "fax", "phoneNum" : "555-2345678" }, { "uid" : 2, "phoneType" : null, "phoneNum" : null }, { "uid" : 3, "phoneType" : "fax", "phoneNum" : "555-4425742" }];

      var actual = getQueryResults(query);
      assertEqual(expected, actual);
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief return a numerically indexed variable
////////////////////////////////////////////////////////////////////////////////

    testIndexedAccess7 : function () {
      var query = "FOR a IN " + JSON.stringify(airports) + " RETURN a.continent.countries[0].airports[0].name";
      var expected = ["CGN", "LGA", "NRT"];

      var actual = getQueryResults(query);
      assertEqual(expected, actual);
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief return data using quoted identifiers
////////////////////////////////////////////////////////////////////////////////

    testEscapedAccess : function () {
      var query = "FOR a IN " + JSON.stringify(airports) + " RETURN `a`.`continent`.`countries`[0].`airports`[0].`name`";
      var expected = ["CGN", "LGA", "NRT"];

      var actual = getQueryResults(query);
      assertEqual(expected, actual);
    }

  };
}

////////////////////////////////////////////////////////////////////////////////
/// @brief executes the test suite
////////////////////////////////////////////////////////////////////////////////

jsunity.run(ahuacatlQueryVariablesTestSuite);

return jsunity.done();

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// @addtogroup\\|// --SECTION--\\|/// @page\\|/// @}\\)"
// End:
