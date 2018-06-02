/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//
// Copyright (c) 2015 Christian Kreuzberger, Alpen-Adria-Universitaet Klagenfurt
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: Christian Kreuzberger <christian.kreuzberger@itec.aau.at>
//
#ifndef DASH_SERVER_HELPER_H
#define DASH_SERVER_HELPER_H

#include <stdint.h>
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"

namespace ns3 {

/**
 * \ingroup Http
 * \brief Create a server application which hosts DASH content, and waits for input TCP packets
 *        and sends them back to the original sender.
 */
class DASHServerHelper
{
public:
  /**
   * Create HttpServerHelper which will make life easier for people trying
   * to set up simulations with Http Servers.
   *
   * \param port The port the server will wait on for incoming packets
   * \param ContentDirectory The directory the server will serve files from
   * \param Hostname The (virtual) hostname of the server
   */
  DASHServerHelper (Address ip, uint16_t port, std::string Hostname,
  std::string MPDDirectory, std::string RepresentationsMetaDataFiles,
  std::string RepresentationsSegmentsDirectory);
  DASHServerHelper (Ipv4Address ip, uint16_t port, std::string Hostname,
  std::string MPDDirectory, std::string RepresentationsMetaDataFiles,
  std::string RepresentationsSegmentsDirectory);
  DASHServerHelper (Ipv6Address ip, uint16_t port, std::string Hostname,
  std::string MPDDirectory, std::string RepresentationsMetaDataFiles,
  std::string RepresentationsSegmentsDirectory);

  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
   * Create a HttpServerApplication on the specified Node.
   *
   * \param node The node on which to create the Application.  The node is
   *             specified by a Ptr<Node>.
   *
   * \returns An ApplicationContainer holding the Application created,
   */
  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * Create a HttpServerApplication on specified node
   *
   * \param nodeName The node on which to create the application.  The node
   *                 is specified by a node name previously registered with
   *                 the Object Name Service.
   *
   * \returns An ApplicationContainer holding the Application created.
   */
  ApplicationContainer Install (std::string nodeName) const;

  /**
   * \param c The nodes on which to create the Applications.  The nodes
   *          are specified by a NodeContainer.
   *
   * Create one udp echo server application on each of the Nodes in the
   * NodeContainer.
   *
   * \returns The applications created, one Application per Node in the
   *          NodeContainer.
   */
  ApplicationContainer Install (NodeContainer c) const;

private:
  /**
   * Install an ns3::HttpServer on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an HttpServer will be installed.
   * \returns Ptr to the application installed.
   */
  Ptr<Application> InstallPriv (Ptr<Node> node) const;

  ObjectFactory m_factory; //!< Object factory.
};

} // namespace ns3

#endif /* HTTP_HELPER_H */
