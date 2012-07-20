////////////////////////////////////////////////////////////////////////////////
/// @brief connection endpoint specification
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
/// @author Jan Steemann
/// @author Copyright 2012, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_FYN_REST_ENDPOINT_SPECIFICATION_H
#define TRIAGENS_FYN_REST_ENDPOINT_SPECIFICATION_H 1

#include <Basics/Common.h>

// -----------------------------------------------------------------------------
// --SECTION--                                             EndpointSpecification
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Rest
/// @{
////////////////////////////////////////////////////////////////////////////////

namespace triagens {
  namespace rest {

////////////////////////////////////////////////////////////////////////////////
/// @brief endpoint types
////////////////////////////////////////////////////////////////////////////////

    enum EndpointType {
      ENDPOINT_UNKNOWN = 0,
      ENDPOINT_TCP,     
      ENDPOINT_UNIX
    };

////////////////////////////////////////////////////////////////////////////////
/// @brief endpoint specification
////////////////////////////////////////////////////////////////////////////////

    class EndpointSpecification {

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Rest
/// @{
////////////////////////////////////////////////////////////////////////////////

    protected:

////////////////////////////////////////////////////////////////////////////////
/// @brief creates an endpoint
////////////////////////////////////////////////////////////////////////////////

      EndpointSpecification (EndpointType type, const string& specification);

////////////////////////////////////////////////////////////////////////////////
/// @brief destroys an endpoint
////////////////////////////////////////////////////////////////////////////////
      
      virtual ~EndpointSpecification ();

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Rest
/// @{
////////////////////////////////////////////////////////////////////////////////
    
    public:

////////////////////////////////////////////////////////////////////////////////
/// @brief creates an endpoint from a string value
////////////////////////////////////////////////////////////////////////////////

      static EndpointSpecification* factory (const string&);

////////////////////////////////////////////////////////////////////////////////
/// @brief compare two endpoints
////////////////////////////////////////////////////////////////////////////////

      bool operator== (EndpointSpecification const &) const;

////////////////////////////////////////////////////////////////////////////////
/// @brief get the type of an endpoint
////////////////////////////////////////////////////////////////////////////////

      EndpointType getType () const {
        return _type;
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief get the original endpoint specification
////////////////////////////////////////////////////////////////////////////////

      string getSpecification () const {
        return _specification;
      }

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                  public variables
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Rest
/// @{
////////////////////////////////////////////////////////////////////////////////

    public:

////////////////////////////////////////////////////////////////////////////////
/// @brief default port number if none specified
////////////////////////////////////////////////////////////////////////////////

      static const uint32_t _defaultPort;

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Rest
/// @{
////////////////////////////////////////////////////////////////////////////////

    private:

////////////////////////////////////////////////////////////////////////////////
/// @brief endpoint type
////////////////////////////////////////////////////////////////////////////////
      
      EndpointType _type;

////////////////////////////////////////////////////////////////////////////////
/// @brief original endpoint specification
////////////////////////////////////////////////////////////////////////////////

      string _specification;

    };

// -----------------------------------------------------------------------------
// --SECTION--                                         EndpointSpecificationUnix
// -----------------------------------------------------------------------------

    class EndpointSpecificationUnix : public EndpointSpecification {

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Rest
/// @{
////////////////////////////////////////////////////////////////////////////////

    public:

////////////////////////////////////////////////////////////////////////////////
/// @brief creates an endpoint
////////////////////////////////////////////////////////////////////////////////

      EndpointSpecificationUnix (string const&, string const&);

////////////////////////////////////////////////////////////////////////////////
/// @brief destroys an endpoint
////////////////////////////////////////////////////////////////////////////////
      
      virtual ~EndpointSpecificationUnix ();

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Rest
/// @{
////////////////////////////////////////////////////////////////////////////////
    
    private:

////////////////////////////////////////////////////////////////////////////////
/// @brief socket file
////////////////////////////////////////////////////////////////////////////////

      string _socket;

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

    };

// -----------------------------------------------------------------------------
// --SECTION--                                          EndpointSpecificationTcp
// -----------------------------------------------------------------------------

    class EndpointSpecificationTcp : public EndpointSpecification {

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Rest
/// @{
////////////////////////////////////////////////////////////////////////////////

    public:

////////////////////////////////////////////////////////////////////////////////
/// @brief creates an endpoint
////////////////////////////////////////////////////////////////////////////////

      EndpointSpecificationTcp (string const&, string const& host, const uint32_t port);

////////////////////////////////////////////////////////////////////////////////
/// @brief destroys an endpoint
////////////////////////////////////////////////////////////////////////////////
      
      ~EndpointSpecificationTcp ();

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Rest
/// @{
////////////////////////////////////////////////////////////////////////////////
    
    private:

////////////////////////////////////////////////////////////////////////////////
/// @brief host name / address (IPv4 or IPv6)
////////////////////////////////////////////////////////////////////////////////

      string _host;

////////////////////////////////////////////////////////////////////////////////
/// @brief port number
////////////////////////////////////////////////////////////////////////////////

      uint32_t _port;

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

    };

  }
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

#endif

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|// --SECTION--\\|/// @\\}\\)"
// End:
