#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/dashplayer-tracer.h"
#include "ns3/node-throughput-tracer.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-nix-vector-helper.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
// #include "ns3/openflow-module.h"
//#include "ns3/mpls-module.h"

#include "ns3/uinteger.h"

#include <fstream>
#include <string>
#include <sstream>

/*
Topology:

          PtP 10M, 2ms
10.0.0.1 <-------------->  10.0.0.2  
 Client                     Server (DASH)
10.0.1.1 <-------------->  10.0.1.2
          PtP 10M, 2ms

*/



using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("dash-mptcp");




int
main (int argc, char *argv[])
{
  // Config::SetDefault ("ns3::TcpSocketBase::EnableMpTcp", BooleanValue(true));
  // Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue("ns3::MpTcpCongestionLia"));

  // LogComponentEnable ("HttpClientApplication", LOG_LEVEL_ALL);
  // LogComponentEnable ("DASHFakeServerApplication", LOG_LEVEL_ALL);
  LogComponentEnable ("dash-mptcp", LOG_LEVEL_ALL); 
  // LogComponentEnable ("MultimediaConsumer", LOG_LEVEL_ALL);
  // LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_ALL);
  // LogComponentEnable ("mpls::MplsProtocol", LOG_LEVEL_DEBUG);
  // LogComponentEnable ("mpls::Ipv4Routing", LOG_LEVEL_DEBUG);
  // LogComponentEnable ("MplsNetworkDiscoverer", LOG_LEVEL_DEBUG); 
  // LogComponentEnable ("TcpL4Protocol", LOG_LEVEL_ALL); 
  // LogComponentEnable ("MpTcpSocketBase", LOG_LEVEL_ALL);
  // LogComponentEnable ("PointToPointNetDevice", LOG_LEVEL_ALL);
  // LogComponentEnable ("OpenFlowSwitchHelper", LOG_LEVEL_ALL); 
  // LogComponentEnable ("OpenFlowInterface", LOG_LEVEL_ALL);
  // LogComponentEnable ("OpenFlowSwitchNetDevice", LOG_LEVEL_ALL);
  // LogComponentEnable ("FlowMonitor", LOG_LEVEL_ALL);
  // LogComponentEnable ("Ipv4StaticRouting", LOG_LEVEL_ALL);


  Config::SetDefault("ns3::Ipv4GlobalRouting::FlowEcmpRouting", BooleanValue(true));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1400));
  Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(0));
  Config::SetDefault("ns3::DropTailQueue::Mode", StringValue("QUEUE_MODE_PACKETS"));
  Config::SetDefault("ns3::DropTailQueue::MaxPackets", UintegerValue(100));
  //setting TcpL4Protocol::SocketType attr IS DAMN IMPORTANT, IF YOU DON'T SET IT MPTCP'S GONNA SIGSEGV!!!!1111 >:(
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(MpTcpSocketBase::GetTypeId())); 
  Config::SetDefault("ns3::MpTcpSocketBase::MaxSubflows", UintegerValue(8)); // Sink

  ns3::RngSeedManager::SetSeed(3); 
  ns3::SeedManager::SetRun(1);



  NodeContainer nodes;
  nodes.Create(2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));
  
  PointToPointHelper pointToPoint1;
  pointToPoint1.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
  pointToPoint1.SetChannelAttribute("Delay", StringValue("2ms"));

  std::vector<NetDeviceContainer> devices;
  NetDeviceContainer d0 = pointToPoint.Install(nodes);
  devices.push_back(d0);
  NetDeviceContainer d1 = pointToPoint1.Install(nodes);
  devices.push_back(d1);

  InternetStackHelper internet;
  internet.Install(nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer i0 = ipv4.Assign(devices[0]);
  ipv4.SetBase("10.0.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i1 = ipv4.Assign(devices[1]);



  
  std::string srv_ip = "10.0.0.2";


  std::string AdaptationLogicToUse = "dash::player::BufferBasedAdaptationLogic";

  ApplicationContainer serverApps;
  std::string representationStrings = "/content/representations/netflix_vid1.csv";
  fprintf(stderr, "representations = %s\n", representationStrings.c_str ());
  DASHServerHelper server (Ipv4Address::GetAny (), 80,  srv_ip, 
                           "/content/mpds/", representationStrings, "/content/segments/");
  serverApps = server.Install (nodes.Get (1));
  serverApps.Start (Seconds(0.1));
  serverApps.Stop (Seconds(100));


  // client stuff
  std::stringstream mpd_baseurl;
  mpd_baseurl << "http://" << srv_ip << "/content/mpds/";
  int videoId = 0;
  // int userId = 0;
  int screenWidth = 1920;
  int screenHeight = 1080;

  std::stringstream ssMPDURL;
  ssMPDURL << mpd_baseurl.str () << "vid" << videoId+1 << ".mpd.gz";
  DASHHttpClientHelper player (ssMPDURL.str ());
  player.SetAttribute("AdaptationLogic", StringValue(AdaptationLogicToUse));
  player.SetAttribute("StartUpDelay", StringValue("0.5"));
  player.SetAttribute("ScreenWidth", UintegerValue(screenWidth));
  player.SetAttribute("ScreenHeight", UintegerValue(screenHeight));
  player.SetAttribute("UserId", UintegerValue(0));
  player.SetAttribute("AllowDownscale", BooleanValue(true));
  player.SetAttribute("AllowUpscale", BooleanValue(true));
  player.SetAttribute("MaxBufferedSeconds", StringValue("1600"));
  // player.SetAttribute("WriteOutfile", StringValue("/home/dashout"));

  ApplicationContainer clientApps;
  clientApps = player.Install (nodes.Get (0));
  clientApps.Start (Seconds (5));
  clientApps.Stop (Seconds (30));
  // fprintf (stderr, "Client starts at %d and stops at %d\n", startsAt, stopsAt);
  



  // Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Ipv4GlobalRoutingHelper g;
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("dynamic-global-routing.routes", std::ios::out);
  // g.PrintRoutingTableAllAt (Seconds (1), routingStream);
  g.PrintRoutingTableAllAt (Seconds (2.1), routingStream);
  // g.PrintRoutingTableAllAt (Seconds (5.1), routingStream);

  pointToPoint.EnablePcapAll ("dash-mptcp");

  // AsciiTraceHelper ascii;
  // pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("dash-mptcp.tr"));

  // mpls_network.DiscoverNetwork ();
  // mpls_network.ShowConfig ();

  Simulator::Stop (Seconds(60));
  Simulator::Run ();
  Simulator::Destroy ();

  ns3::DASHPlayerTracer::Destroy ();

  return 0;
}


