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
#include "http-helper.h"
#include "ns3/http-server.h"
#include "ns3/http-client.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"
#include "ns3/string.h"

namespace ns3 {

HttpServerHelper::HttpServerHelper (Address address, uint16_t port, std::string ContentDirectory, std::string Hostname)
{
  m_factory.SetTypeId (HttpServerApplication::GetTypeId ());
  SetAttribute ("ListeningAddress", AddressValue (address));
  SetAttribute ("Port", UintegerValue (port));
  SetAttribute ("ContentDirectory", StringValue (ContentDirectory));
  SetAttribute ("Hostname", StringValue (Hostname));
}

HttpServerHelper::HttpServerHelper (Ipv4Address address, uint16_t port, std::string ContentDirectory, std::string Hostname)
{
  m_factory.SetTypeId (HttpServerApplication::GetTypeId ());
  SetAttribute ("ListeningAddress", AddressValue (address));
  SetAttribute ("Port", UintegerValue (port));
  SetAttribute ("ContentDirectory", StringValue (ContentDirectory));
  SetAttribute ("Hostname", StringValue (Hostname));
}

HttpServerHelper::HttpServerHelper (Ipv6Address address, uint16_t port, std::string ContentDirectory, std::string Hostname)
{
  m_factory.SetTypeId (HttpServerApplication::GetTypeId ());
  SetAttribute ("ListeningAddress", AddressValue (address));
  SetAttribute ("Port", UintegerValue (port));
  SetAttribute ("ContentDirectory", StringValue (ContentDirectory));
  SetAttribute ("Hostname", StringValue (Hostname));
}

void
HttpServerHelper::SetAttribute (
  std::string name,
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
HttpServerHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
HttpServerHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
HttpServerHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
HttpServerHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<HttpServerApplication> ();
  node->AddApplication (app);

  return app;
}

HttpClientHelper::HttpClientHelper (Address address, uint16_t port, std::string FileToRequest, std::string ServerHostname)
{
  m_factory.SetTypeId (HttpClientApplication::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
  SetAttribute ("FileToRequest", StringValue(FileToRequest));
  SetAttribute ("RemoteHostName", StringValue(ServerHostname));
}

HttpClientHelper::HttpClientHelper (Ipv4Address address, uint16_t port, std::string FileToRequest, std::string ServerHostname)
{
  m_factory.SetTypeId (HttpClientApplication::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (Address(address)));
  SetAttribute ("RemotePort", UintegerValue (port));
  SetAttribute ("FileToRequest", StringValue(FileToRequest));
  SetAttribute ("RemoteHostName", StringValue(ServerHostname));
}

HttpClientHelper::HttpClientHelper (Ipv6Address address, uint16_t port, std::string FileToRequest, std::string ServerHostname)
{
  m_factory.SetTypeId (HttpClientApplication::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (Address(address)));
  SetAttribute ("RemotePort", UintegerValue (port));
  SetAttribute ("FileToRequest", StringValue(FileToRequest));
  SetAttribute ("RemoteHostName", StringValue(ServerHostname));
}

void
HttpClientHelper::SetAttribute (
  std::string name,
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}


ApplicationContainer
HttpClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
HttpClientHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
HttpClientHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
HttpClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<HttpClientApplication> ();
  node->AddApplication (app);

  return app;
}


} // namespace ns3
