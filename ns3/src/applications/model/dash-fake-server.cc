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
#include <iostream>

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
#include "ns3/double.h"
#include "ns3/random-variable-stream.h"
#include "ns3/mp-tcp-socket-base.h"

#include <stdio.h>

#include "dash-fake-server.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DASHFakeServerApplication");

NS_OBJECT_ENSURE_REGISTERED (DASHFakeServerApplication);





TypeId
DASHFakeServerApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DASHFakeServerApplication")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<DASHFakeServerApplication> ()
    .AddAttribute ("ListeningAddress",
                   "The listening Address for the inbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&DASHFakeServerApplication::m_listeningAddress),
                   MakeAddressChecker ())
    .AddAttribute ("Port", "Port on which we listen for incoming packets (default: 80).",
                   UintegerValue (80),
                   MakeUintegerAccessor (&DASHFakeServerApplication::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute("MPDDirectory", "The directory which DASHFakeServerApplication fakes to serves the MPD files",
                   StringValue("/"),
                   MakeStringAccessor(&DASHFakeServerApplication::m_mpdDirectory),
                   MakeStringChecker())
    .AddAttribute("RepresentationsMetaDataFiles", "The meta data csv file(s) containing the videos and representations this server will serve",
                   StringValue("./representations.csv"),
                   MakeStringAccessor(&DASHFakeServerApplication::m_mpdMetaDataFiles),
                   MakeStringChecker())
    .AddAttribute("RepresentationsSegmentsDirectory", "The directory that virtual segments of representations are served",
                   StringValue("/"),
                   MakeStringAccessor(&DASHFakeServerApplication::m_metaDataContentDirectory),
                   MakeStringChecker())
    .AddAttribute("Hostname", "The (virtual) hostname of this server",
                   StringValue("localhost"),
                   MakeStringAccessor(&DASHFakeServerApplication::m_hostName),
                   MakeStringChecker())
    .AddTraceSource("ThroughputTracer", "Trace Throughput statistics of this server",
                      MakeTraceSourceAccessor(&DASHFakeServerApplication::m_throughputTrace))
                    ;
  ;
  return tid;
}


DASHFakeServerApplication::DASHFakeServerApplication ()
{
  NS_LOG_FUNCTION (this);
}

DASHFakeServerApplication::~DASHFakeServerApplication()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
}

void
DASHFakeServerApplication::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}



std::string /* mpd string */
DASHFakeServerApplication::ImportDASHRepresentations (std::string mpdMetaDataFilename, int video_id)
{
  NS_LOG_FUNCTION(mpdMetaDataFilename << video_id);
// read m_mpdMetaDataFiles and fill m_fileSizes
  std::ifstream infile(mpdMetaDataFilename.c_str());
  if (!infile.is_open())
  {
    NS_LOG_ERROR("Error opening " << mpdMetaDataFilename);
    return "";
  }

  int segment_duration = 0;
  int number_of_segments = 0;

  std::string line;

  /*** this is an example of how the file looks like:
  segmentDuration=2
  numberOfSegments=1800
  reprId,screenWidth,screenHeight,bitrate
  1,640,360,317
  2,640,360,399
  10,960,540,755
  31,1920,1080,624
*/


  // get first line: segmentDuration
  std::getline(infile,line);
  std::string prefix("segmentDuration=");
  if (!line.compare(0, prefix.size(), prefix))
    segment_duration = atoi(line.substr(prefix.size()).c_str());

  std::getline(infile,line);
  prefix = "numberOfSegments=";
  if (!line.compare(0, prefix.size(), prefix))
    number_of_segments = atoi(line.substr(prefix.size()).c_str());
 
  //JEREMIE
  double avgsigma_mu = -1;
  //read bitrate variation
  std::getline(infile,line);
  prefix = "AvgSigma/mu=";
  if (!line.compare(0, prefix.size(), prefix))
    avgsigma_mu = atof(line.substr(prefix.size()).c_str());

  std::stringstream mpdData;
  mpdData << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl
          << "<MPD xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << std::endl
          << "xmlns=\"urn:mpeg:DASH:schema:MPD:2011\" xsi:schemaLocation=\"urn:mpeg:DASH:schema:MPD:2011\"" << std::endl
          << "profiles=\"urn:mpeg:dash:profile:isoff-main:2011\" type=\"static\"" << std::endl;


  int totalVideoDuration = number_of_segments * segment_duration;

  int videoDurationInSeconds = totalVideoDuration % 60;
  totalVideoDuration -= videoDurationInSeconds;
  totalVideoDuration = totalVideoDuration / 60;
  int videoDurationInMinutes = totalVideoDuration % 60;
  totalVideoDuration -= videoDurationInMinutes;
  int videoDurationInHours = totalVideoDuration / 60;


  mpdData << "mediaPresentationDuration=\"PT" << videoDurationInHours << "H" <<
          videoDurationInMinutes << "M" <<
          videoDurationInSeconds << "S\" ";
  mpdData << "minBufferTime=\"PT2.0S\">" << std::endl;
  mpdData << "<BaseURL>http://" << m_hostName << m_metaDataContentDirectory  << "vid" << video_id << "/</BaseURL>" << std::endl
          << "<Period start=\"PT0S\">" << std::endl << "<AdaptationSet bitstreamSwitching=\"true\">" << std::endl;

  // get header and ignore
  if(avgsigma_mu!=-1) std::getline(infile,line); // reprId,screenWidth,screenHeight,bitrate

  //create file with segment properties
  std::ofstream segments_file;
  segments_file.open("segments", std::ofstream::app);

  while (std::getline(infile,line))
  {
    if (line.length() > 2) // line must not be empty
    {
      size_t pos1 = line.find(",");
      if (pos1 != std::string::npos)
      {
        std::string repr_id = line.substr(0, pos1);
        line = line.substr(pos1+1);

        pos1 = line.find(",");
        if (pos1 != std::string::npos)
        {
         std::string quality_index = line.substr(0, pos1);
         line = line.substr(pos1+1);

         pos1 = line.find(",");
         if (pos1 != std::string::npos)
         {
          std::string repr_width = line.substr(0, pos1);
          line = line.substr(pos1+1);

          pos1 = line.find(",");

          if (pos1 != std::string::npos)
          {
            std::string repr_height = line.substr(0, pos1);
	    line = line.substr(pos1+1);

	    pos1 = line.find(",");

	    std::string repr_bitrate; 
	    std::string repr_sigma_mu;
	    std::string repr_avgchuncksize;
	    double bitrate;
	    double sigma_mu=-1;
	    double avgchunksize;

            if (pos1 != std::string::npos)
            {
               repr_bitrate  = line.substr(0, pos1);
	       line = line.substr(pos1+1);
	       pos1 = line.find(",");
               repr_sigma_mu  = line.substr(0, pos1);
               repr_avgchuncksize = line.substr(pos1+1);
	       sigma_mu=atof(repr_sigma_mu.c_str());
	       avgchunksize=atof(repr_avgchuncksize.c_str());	
	    }else{
	       repr_bitrate  = line.substr(pos1+1);
	    }
	    bitrate=atof(repr_bitrate.c_str());
	    int iBitrate = atoi(repr_bitrate.c_str()); // read bitrate in kilobit/s

            NS_LOG_DEBUG ("Representation ID = "<<repr_id.c_str()<<", height = "<<repr_height.c_str()<<", bitrate = " <<repr_bitrate.c_str());
            mpdData << "<Representation id=\"" << repr_id << "\" codecs=\"avc1\" mimeType=\"video/mp4\"" <<
                 " width=\"" << repr_width << "\" height=\"" << repr_height << "\" startWithSAP=\"1\" bandwidth=\"" << (iBitrate*1000) << "\">" << std::endl;
            mpdData << "<SegmentList duration=\"" << segment_duration << "\">" << std::endl;


	    //JEREMIE: modify the size of each segment according to bit rate variation 
	    Ptr<LogNormalRandomVariable> x = CreateObject<LogNormalRandomVariable> ();
	    if(sigma_mu>=0){
		double delta=std::sqrt(1.0/(sigma_mu*sigma_mu)+2*std::log(avgchunksize));
		double sigma=-1/sigma_mu+delta; 
		double mu=sigma/sigma_mu;
		//double mu=std::log(bitrate*bitrate/std::sqrt(bitrate*bitrate+sigma_mu*bitrate));
		//double sigma=std::sqrt(std::log(sigma_mu*bitrate/(bitrate*bitrate)+1));
	    	NS_LOG_INFO( bitrate << " " << sigma_mu << " " << sigma << " " << mu );
	    	x->SetAttribute("Mu", DoubleValue (mu));
	   	x->SetAttribute("Sigma", DoubleValue (sigma));
	    	NS_LOG_INFO( quality_index << " " << avgchunksize << " " << iBitrate << " " << mu << " " << sigma << " " << x->GetValue() << " " << bitrate << " " << sigma_mu*bitrate << " " << sigma_mu << " " << x->GetValue());
		//exit(0);
	    }
            
	    long iSegmentSize = (double)iBitrate/8.0 * (double)segment_duration * 1024; // in byte
            
	    for (int i = 0; i < number_of_segments; i++)
            {
              std::ostringstream segmentFileName;
              segmentFileName << "vid" << video_id << "/repr_" << repr_id << "_seg_" << i << ".264";
	      if(sigma_mu>=0){
			iBitrate=(int)x->GetValue()/ (double)segment_duration;
			iSegmentSize = (double)iBitrate/8.0 * (double)segment_duration * 1024; // in byte
	      }
	      segments_file  << video_id << " " << repr_id << " " << i << " " << iSegmentSize << " " << quality_index << "\n";
              m_fileSizes[m_metaDataContentDirectory + segmentFileName.str()] = iSegmentSize;

              m_virtualFiles.push_back(m_metaDataContentDirectory + segmentFileName.str());
              mpdData << "<SegmentURL media=\"" <<  "repr_" << repr_id << "_seg_" << i << ".264" << "\"/> " << std::endl;
              //fprintf(stderr, "SegmentName=%s\n", (m_metaDataContentDirectory + segmentFileName.str()).c_str());
            }

            mpdData << "</SegmentList>" << std::endl << "</Representation>" << std::endl;
          }
        }
       }
      }
    }
  }
  segments_file.close();
  mpdData << "</AdaptationSet></Period></MPD>" << std::endl;

  infile.close();

  return mpdData.str();
}




void
DASHFakeServerApplication::ReportStats()
{
  uint64_t bytes_recv = m_bytes_recv - m_last_bytes_recv;
  uint64_t bytes_sent = m_bytes_sent - m_last_bytes_sent;

  m_throughputTrace(this, bytes_sent, bytes_recv, m_activeClients.size());


  m_last_bytes_recv = m_bytes_recv;
  m_last_bytes_sent = m_bytes_sent;

  if (m_active)
  {
    m_reportStatsTimer = Simulator::Schedule(Seconds(1.0), &DASHFakeServerApplication::ReportStats, this);
  }
}


// this traces the acutal packet size, including header etc...
void
DASHFakeServerApplication::TxTrace(Ptr<Packet const> packet)
{
  m_bytes_sent += packet->GetSize();
}

// this traces the acutal packet size, including header etc...

void
DASHFakeServerApplication::RxTrace(Ptr<Packet const> packet)
{
  m_bytes_recv += packet->GetSize();
}




void
DASHFakeServerApplication::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  m_lastSocketID = 1;

  m_active = true;

  Ptr<NetDevice> netdevice = GetNode()->GetDevice(0);
  netdevice->TraceConnectWithoutContext("PhyTxEnd", MakeCallback(&DASHFakeServerApplication::TxTrace, this));
  netdevice->TraceConnectWithoutContext("PhyRxEnd", MakeCallback(&DASHFakeServerApplication::RxTrace, this));

  m_last_bytes_recv = 0;
  m_last_bytes_sent = 0;

  m_bytes_recv = 0;
  m_bytes_sent = 0;

  if (m_socket == 0)
  {
    TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory"); // that's a correct TypeID for MPTCP
    // m_socket = Socket::CreateSocket (GetNode (), tid);
    m_socket = DynamicCast<MpTcpSocketBase>(Socket::CreateSocket (GetNode (), tid));


    //Fatal error if socket type is not NS3_SOCK_STREAM or NS3_SOCK_SEQPACKET
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
      NS_LOG_UNCOND("Listening on Ipv4 " << Ipv4Address::ConvertFrom(m_listeningAddress) << ":" << m_port);
      m_socket->Bind (Address(local));
    } else if (Ipv6Address::IsMatchingType(m_listeningAddress) == true)
    {
      NS_FATAL_ERROR ("IPv6 is not handled nor tested in this MPTCP conversion");
      // Inet6SocketAddress local6 = Inet6SocketAddress (Ipv6Address::ConvertFrom(m_listeningAddress), m_port);
      // NS_LOG_INFO("Listening on Ipv6 " << Ipv6Address::ConvertFrom(m_listeningAddress));
      // m_socket->Bind (local6);
    } else {
      NS_LOG_ERROR("Not sure what type the m_listeningaddress is... " << m_listeningAddress);
    }
  }


  // Listen for incoming connections
  m_socket->Listen();
  NS_ASSERT (m_socket != 0);

  // And make sure to handle requests and accepted connections
  m_socket->SetAcceptCallback (MakeCallback(&DASHFakeServerApplication::ConnectionRequested, this),
      MakeCallback(&DASHFakeServerApplication::ConnectionAccepted, this)
  );

  // vitalii: for some reason, m_mpdMetaDataFiles is empty and there isn't any explicit way to construct or fetch it. Hence, next line:
  std::string m_mpdMetaDataFiles = "../content/representations/netflix_vid1.csv,../content/representations/netflix_vid2.csv,../content/representations/netflix_vid3.csv"; // makes it only for three videoIDs, add if need more
  
  // parse m_mpdMetaDataFiles, could be comma separated list
  std::vector<std::string> metaDataRepresentations;
  size_t pos = m_mpdMetaDataFiles.find(",");



  if (pos == std::string::npos)
  {
    // parse one file
    metaDataRepresentations.push_back(m_mpdMetaDataFiles);
  } else {
    // parse multiple files
    size_t start_pos = 0;
    std::string parseString = m_mpdMetaDataFiles;
    while (pos != std::string::npos)
    {
      std::string tmpStr = parseString.substr(start_pos, pos);

      metaDataRepresentations.push_back(tmpStr);

      parseString = parseString.substr(pos+1);
      pos = parseString.find(",");
    }

    std::string tmpStr = parseString;

    metaDataRepresentations.push_back(tmpStr);
  }

  int video_id = 1;

  // parse all files in metaDataRepresentations
  for (std::vector<std::string>::iterator it = metaDataRepresentations.begin(); it != metaDataRepresentations.end(); ++it)
  {
    std::string mmm = *it;
    std::string mpdData = ImportDASHRepresentations(mmm, video_id);

    // compress
    std::string compressedMpdData = zlib_compress_string(mpdData);
    NS_LOG_INFO ("Size of compressed = " << compressedMpdData.length() << ", uncompressed = " << mpdData.length());


    std::stringstream SSMpdFilename;

    SSMpdFilename << m_mpdDirectory << "vid" << video_id << ".mpd.gz";

    m_fileSizes[SSMpdFilename.str()] = compressedMpdData.size();

    NS_LOG_INFO ("Adding " << SSMpdFilename.str().c_str() << " to m_fileSizes with size " << compressedMpdData.size());


    m_mpdFileContents[SSMpdFilename.str()] = compressedMpdData;

    video_id++;
  }


  ReportStats();

}


void
DASHFakeServerApplication::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
  {
    m_socket->Close ();
    m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  }

  this->m_virtualFiles.clear (); // Vitalii: let's see if it helps with memory leaking
}


void
DASHFakeServerApplication::OnReadySend(Ptr<Socket> socket, unsigned int txSize)
{
  NS_LOG_INFO ("Server says it is ready to send something now...");
}


bool
DASHFakeServerApplication::ConnectionRequested (Ptr<Socket> socket, const Address& address)
{
  NS_LOG_FUNCTION (this << socket << address);
  NS_LOG_DEBUG (Simulator::Now () << " Socket = " << socket << " " << " Server: ConnectionRequested");
  return true;
}


void
DASHFakeServerApplication::ConnectionAccepted (Ptr<Socket> s, const Address& address)
{
  NS_LOG_FUNCTION (this << s << address);
  NS_LOG_INFO ("DASH Fake Server: Connection Accepted!");
  Ptr<MpTcpSocketBase> socket = DynamicCast<MpTcpSocketBase> (s);

  uint64_t socket_id = RegisterSocket(socket);

  m_activeClients[socket_id] = new HttpServerFakeVirtualClientSocket(socket_id, "/", m_fileSizes, m_virtualFiles, m_mpdFileContents,
                  MakeCallback(&DASHFakeServerApplication::FinishedCallback, this));

  NS_LOG_DEBUG (socket << " " << Simulator::Now () << " Successful socket id : " << socket_id << " Connection Accepted From " << address);

  // set callbacks for this socket to be in HttpServerFakeClientSocket class
  socket->SetSendCallback (MakeCallback (&HttpServerFakeVirtualClientSocket::HandleReadyToTransmit, m_activeClients[socket_id]));
  socket->SetRecvCallback (MakeCallback (&HttpServerFakeVirtualClientSocket::HandleIncomingData, m_activeClients[socket_id]));


  socket->TraceConnectWithoutContext ("State",
    MakeCallback(&HttpServerFakeVirtualClientSocket::LogStateChange, m_activeClients[socket_id]));

  socket->SetCloseCallbacks (MakeCallback (&HttpServerFakeVirtualClientSocket::ConnectionClosedNormal, m_activeClients[socket_id]),
                             MakeCallback (&HttpServerFakeVirtualClientSocket::ConnectionClosedError,  m_activeClients[socket_id]));
}





void
DASHFakeServerApplication::FinishedCallback (uint64_t socket_id)
{
  // create timer to finish this, because if we do it in here, we will crash the app
  Simulator::Schedule(Seconds(1.0), &DASHFakeServerApplication::DoFinishSocket, this, socket_id);
}

void
DASHFakeServerApplication::DoFinishSocket(uint64_t socket_id)
{
  if (m_activeClients.find(socket_id) != m_activeClients.end())
  {
    HttpServerFakeClientSocket* tmp = m_activeClients[socket_id];
    NS_LOG_INFO ("DASH virtual server (not sure which exactly :/ ) is going to delete a terminated socket " << socket_id);
    m_activeClients.erase(socket_id);
    delete tmp; // TODO: CHECK
  }
}






uint64_t
DASHFakeServerApplication::RegisterSocket (Ptr<MpTcpSocketBase> socket)
{
  this->m_activeSockets[socket] = this->m_lastSocketID;

  return this->m_lastSocketID++;
}





}
