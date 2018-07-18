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

// ns3 - HTTP Server Application class

#include <fstream>

#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/random-variable-stream.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/string.h"
#include "ns3/pointer.h"

#include <stdio.h>

#include "http-server.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("HttpServerApplication");

NS_OBJECT_ENSURE_REGISTERED (HttpServerApplication);





TypeId
HttpServerApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::HttpServerApplication")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<HttpServerApplication> ()
    .AddAttribute ("ListeningAddress",
                   "The listening Address for the inbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&HttpServerApplication::m_listeningAddress),
                   MakeAddressChecker ())
    .AddAttribute ("Port", "Port on which we listen for incoming packets (default: 80).",
                   UintegerValue (80),
                   MakeUintegerAccessor (&HttpServerApplication::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute("ContentDirectory", "The directory of which HttpServerApplication serves the files",
                   StringValue("/"),
                   MakeStringAccessor(&HttpServerApplication::m_contentDir),
                   MakeStringChecker())
    .AddAttribute("MetaDataFile", "The meta data file for virtual files that HttpServerApplication will serve",
                   StringValue("/"),
                   MakeStringAccessor(&HttpServerApplication::m_metaDataFile),
                   MakeStringChecker())
    .AddAttribute("MetaDataDirectory", "The dirctory that virtual payloads from meta data file for virtual files that HttpServerApplication will serve",
                   StringValue("/"),
                   MakeStringAccessor(&HttpServerApplication::m_metaDataContentDirectory),
                   MakeStringChecker())
    .AddAttribute("Hostname", "The (virtual) hostname of this server",
                   StringValue("localhost"),
                   MakeStringAccessor(&HttpServerApplication::m_hostName),
                   MakeStringChecker())
    .AddTraceSource("ThroughputTracer", "Trace Throughput statistics of this server",
                      MakeTraceSourceAccessor(&HttpServerApplication::m_throughputTrace))
                    ;
  ;
  return tid;
}


HttpServerApplication::HttpServerApplication ()
{
  NS_LOG_FUNCTION (this);

  m_active = false;
}

HttpServerApplication::~HttpServerApplication()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
}

void
HttpServerApplication::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}


void
HttpServerApplication::ReportStats()
{
  uint64_t bytes_recv = m_bytes_recv - m_last_bytes_recv;
  uint64_t bytes_sent = m_bytes_sent - m_last_bytes_sent;

  m_throughputTrace(this, bytes_sent, bytes_recv, m_activeClients.size());


  m_last_bytes_recv = m_bytes_recv;
  m_last_bytes_sent = m_bytes_sent;

  if (m_active)
  {
    m_reportStatsTimer = Simulator::Schedule(Seconds(1.0), &HttpServerApplication::ReportStats, this);
  }
}

// this traces the acutal packet size, including header etc...
void
HttpServerApplication::TxTrace(Ptr<Packet const> packet)
{
  m_bytes_sent += packet->GetSize();
}

// this traces the acutal packet size, including header etc...
void
HttpServerApplication::RxTrace(Ptr<Packet const> packet)
{
  m_bytes_recv += packet->GetSize();
}


void
HttpServerApplication::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  m_active = true;

  // trace Physical TX and RX after it has been done (End)
  Ptr<NetDevice> netdevice = GetNode()->GetDevice(0);
  netdevice->TraceConnectWithoutContext("PhyTxEnd", MakeCallback(&HttpServerApplication::TxTrace, this));
  netdevice->TraceConnectWithoutContext("PhyRxEnd", MakeCallback(&HttpServerApplication::RxTrace, this));

  m_last_bytes_recv = 0;
  m_last_bytes_sent = 0;

  m_bytes_recv = 0;
  m_bytes_sent = 0;

  m_lastSocketID = 1;

  if (m_socket == 0)
  {
    TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
    m_socket = Socket::CreateSocket (GetNode (), tid);


    // Fatal error if socket type is not NS3_SOCK_STREAM or NS3_SOCK_SEQPACKET
    if (m_socket->GetSocketType () != Socket::NS3_SOCK_STREAM &&
        m_socket->GetSocketType () != Socket::NS3_SOCK_SEQPACKET)
    {
      NS_FATAL_ERROR ("Using BulkSend with an incompatible socket type. "
                      "BulkSend requires SOCK_STREAM or SOCK_SEQPACKET. "
                      "In other words, use TCP instead of UDP.");
    }

    if (Ipv4Address::IsMatchingType(m_listeningAddress) == true)
    {
      InetSocketAddress local = InetSocketAddress (Ipv4Address::ConvertFrom(m_listeningAddress), m_port);
      NS_LOG_INFO("Listening on Ipv4 " << Ipv4Address::ConvertFrom(m_listeningAddress) << ":" << m_port);
      m_socket->Bind (local);
    } else if (Ipv6Address::IsMatchingType(m_listeningAddress) == true)
    {
      Inet6SocketAddress local6 = Inet6SocketAddress (Ipv6Address::ConvertFrom(m_listeningAddress), m_port);
      NS_LOG_INFO("Listening on Ipv6 " << Ipv6Address::ConvertFrom(m_listeningAddress));
      m_socket->Bind (local6);
    } else {
      NS_LOG_ERROR("Not sure what type the m_listeningaddress is... " << m_listeningAddress);
    }
  }


  // Listen for incoming connections
  m_socket->Listen();
  NS_ASSERT (m_socket != 0);

  // And make sure to handle requests and accepted connections
  m_socket->SetAcceptCallback (MakeCallback(&HttpServerApplication::ConnectionRequested, this),
      MakeCallback(&HttpServerApplication::ConnectionAccepted, this)
  );


  // parse meta data csv file

  // read m_metaDataFile and fill m_fileSizes
  std::ifstream infile(m_metaDataFile.c_str());
  if (!infile.is_open())
  {
    NS_LOG_INFO ("HttpServerFakeClientSocket: Error opening " << m_metaDataFile.c_str());
    return;
  }

  std::string line;

  while (std::getline(infile,line))
  {
    if (line.length() > 2)
    {
      size_t pos = line.find(",");
      if (pos != std::string::npos)
      {
        std::string line_filename = line.substr(0, pos);
        std::string line_filesize = line.substr(pos+1);
        //fprintf(stderr, "First=%s,Second=%s\n", line_filename.c_str(), line_filesize.c_str());
        m_fileSizes[m_contentDir + m_metaDataContentDirectory + line_filename] = atoi(line_filesize.c_str());

        NS_LOG_INFO ("Added '" << (m_contentDir + m_metaDataContentDirectory + line_filename).c_str() << "' to the store!\n");

        m_virtualFiles.push_back(m_contentDir + m_metaDataContentDirectory + line_filename);
      }
    }
  }


  infile.close();

  ReportStats();
}


void
HttpServerApplication::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  m_active = false;

  Simulator::Cancel(m_reportStatsTimer);

  if (m_socket != 0)
  {
    m_socket->Close ();
    m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  }
}


void
HttpServerApplication::OnReadySend(Ptr<Socket> socket, unsigned int txSize)
{
  NS_LOG_INFO ("Server says it is ready to send something now...");
}


bool
HttpServerApplication::ConnectionRequested (Ptr<Socket> socket, const Address& address)
{
  NS_LOG_FUNCTION (this << socket << address);
  NS_LOG_DEBUG (Simulator::Now () << " Socket = " << socket << " " << " Server: ConnectionRequested");
  return true;
}


void
HttpServerApplication::ConnectionAccepted (Ptr<Socket> socket, const Address& address)
{
  NS_LOG_FUNCTION (this << socket << address);


  uint64_t socket_id = RegisterSocket(socket);

  m_activeClients[socket_id] = new HttpServerFakeClientSocket(socket_id, m_contentDir, m_fileSizes, m_virtualFiles,
                  MakeCallback(&HttpServerApplication::FinishedCallback, this));

  NS_LOG_DEBUG (socket << " " << Simulator::Now () << " Successful socket id : " << socket_id << " Connection Accepted From " << address);

  // set callbacks for this socket to be in HttpServerFakeClientSocket class
  socket->SetSendCallback (MakeCallback (&HttpServerFakeClientSocket::HandleReadyToTransmit, m_activeClients[socket_id]));
  socket->SetRecvCallback (MakeCallback (&HttpServerFakeClientSocket::HandleIncomingData, m_activeClients[socket_id]));


  socket->TraceConnectWithoutContext ("State",
    MakeCallback(&HttpServerFakeClientSocket::LogStateChange, m_activeClients[socket_id]));

/*
  socket->TraceConnectWithoutContext ("CongestionWindow",
    MakeCallback(&HttpServerFakeClientSocket::LogCwndChange, m_activeClients[socket_id]));
*/


  socket->SetCloseCallbacks (MakeCallback (&HttpServerFakeClientSocket::ConnectionClosedNormal, m_activeClients[socket_id]),
                             MakeCallback (&HttpServerFakeClientSocket::ConnectionClosedError,  m_activeClients[socket_id]));
}





void
HttpServerApplication::FinishedCallback (uint64_t socket_id)
{
  // create timer to finish this, because if we do it in here, we will crash the app
  Simulator::Schedule(Seconds(1.0), &HttpServerApplication::DoFinishSocket, this, socket_id);
}

void
HttpServerApplication::DoFinishSocket(uint64_t socket_id)
{
  if (m_activeClients.find(socket_id) != m_activeClients.end())
  {
    //HttpServerFakeClientSocket* tmp = m_activeClients[socket_id];
    //m_activeClients.erase(socket_id);
    //delete tmp; // TODO: CHECK
  }
}






uint64_t
HttpServerApplication::RegisterSocket (Ptr<Socket> socket)
{
  this->m_activeSockets[socket] = this->m_lastSocketID;

  return this->m_lastSocketID++;
}





}
