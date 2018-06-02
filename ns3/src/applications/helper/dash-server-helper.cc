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
#include "dash-server-helper.h"
#include "ns3/http-server.h"
#include "ns3/http-client.h"
#include "ns3/dash-fake-server.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"
#include "ns3/string.h"

namespace ns3 {

/**********************
* DASH SERVER HELPER  *
**********************/


DASHServerHelper::DASHServerHelper (Address address, uint16_t port, std::string Hostname,
  std::string MPDDirectory, std::string RepresentationsMetaDataFiles,
  std::string RepresentationsSegmentsDirectory)
{
  m_factory.SetTypeId (DASHFakeServerApplication::GetTypeId ());
  SetAttribute ("ListeningAddress", AddressValue (address));
  SetAttribute ("Port", UintegerValue (port));
  SetAttribute ("Hostname", StringValue (Hostname));
  SetAttribute ("MPDDirectory", StringValue (MPDDirectory));
  SetAttribute ("RepresentationsMetaDataFiles", StringValue (RepresentationsMetaDataFiles));
  SetAttribute ("RepresentationsSegmentsDirectory", StringValue (RepresentationsSegmentsDirectory));
}

DASHServerHelper::DASHServerHelper (Ipv4Address address, uint16_t port, std::string Hostname,
  std::string MPDDirectory, std::string RepresentationsMetaDataFiles,
  std::string RepresentationsSegmentsDirectory)
{
  m_factory.SetTypeId (DASHFakeServerApplication::GetTypeId ());
  SetAttribute ("ListeningAddress", AddressValue (address));
  SetAttribute ("Port", UintegerValue (port));
  SetAttribute ("Hostname", StringValue (Hostname));
  SetAttribute ("MPDDirectory", StringValue (MPDDirectory));
  SetAttribute ("RepresentationsMetaDataFiles", StringValue (RepresentationsMetaDataFiles));
  SetAttribute ("RepresentationsSegmentsDirectory", StringValue (RepresentationsSegmentsDirectory));
}

DASHServerHelper::DASHServerHelper (Ipv6Address address, uint16_t port, std::string Hostname,
  std::string MPDDirectory, std::string RepresentationsMetaDataFiles,
  std::string RepresentationsSegmentsDirectory)
{
  m_factory.SetTypeId (DASHFakeServerApplication::GetTypeId ());
  SetAttribute ("ListeningAddress", AddressValue (address));
  SetAttribute ("Port", UintegerValue (port));
  SetAttribute ("Hostname", StringValue (Hostname));
  SetAttribute ("MPDDirectory", StringValue (MPDDirectory));
  SetAttribute ("RepresentationsMetaDataFiles", StringValue (RepresentationsMetaDataFiles));
  SetAttribute ("RepresentationsSegmentsDirectory", StringValue (RepresentationsSegmentsDirectory));
}

void
DASHServerHelper::SetAttribute (
  std::string name,
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
DASHServerHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
DASHServerHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
DASHServerHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
DASHServerHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<DASHFakeServerApplication> ();
  node->AddApplication (app);

  return app;
}




} // namespace ns3
