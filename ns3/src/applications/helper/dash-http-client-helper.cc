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
#include "dash-http-client-helper.h"
#include "ns3/http-multimedia-consumer.h"
#include "ns3/http-client.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"
#include "ns3/string.h"

namespace ns3 {
typedef MultimediaConsumer<HttpClientApplication> HTTPMultimediaConsumer;


DASHHttpClientHelper::DASHHttpClientHelper (std::string mpdFileURI)
{
  m_factory.SetTypeId (HTTPMultimediaConsumer::GetTypeId ());
  SetAttribute ("MpdFileToRequest", StringValue(mpdFileURI));
}



void
DASHHttpClientHelper::SetAttribute (
  std::string name,
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}


ApplicationContainer
DASHHttpClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
DASHHttpClientHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
DASHHttpClientHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
DASHHttpClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<HTTPMultimediaConsumer> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
