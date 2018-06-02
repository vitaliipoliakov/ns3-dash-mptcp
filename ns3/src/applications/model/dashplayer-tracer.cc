#include "dashplayer-tracer.h"

#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/simulator.h"
#include "ns3/node-list.h"
#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/boolean.h"
#include "ns3/core-module.h"
#include "ns3/trace-source-accessor.h"

#include <boost/shared_ptr.hpp>

#include <fstream>

NS_LOG_COMPONENT_DEFINE("ns3.DASHPlayerTracer");

using namespace std;

namespace ns3 {


static std::list< Ptr< DASHPlayerTracer > > m_allTracers;


void
DASHPlayerTracer::Destroy()
{
  m_allTracers.clear();
}

void
DASHPlayerTracer::InstallAll(const std::string& file)
{
  using namespace std;

  std::list< Ptr< DASHPlayerTracer > > tracers;

  boost::shared_ptr<std::ofstream> os(new std::ofstream());
  os->open(file.c_str(), std::ios_base::out | std::ios_base::trunc);

  if (!os->is_open()) {
    NS_LOG_ERROR("File " << file << " cannot be opened for writing. Tracing disabled");
    return;
  }


  for (NodeList::Iterator node = NodeList::Begin(); node != NodeList::End(); node++) {
    Ptr<DASHPlayerTracer> trace = Install(*node, os);
    tracers.push_back(trace);
    m_allTracers.push_back(trace);
  }

  if (tracers.size() > 0) {
    tracers.front()->PrintHeader(*os);
    *os << "\n";
  }
}



void
DASHPlayerTracer::Install(const NodeContainer& nodes, const std::string& file)
{
  using namespace std;

  std::list< Ptr< DASHPlayerTracer > > tracers;


  // create output stream ONCE
  boost::shared_ptr<std::ofstream> os(new std::ofstream());
  os->open(file.c_str(), std::ios_base::out | std::ios_base::trunc);

  if (!os->is_open()) {
    NS_LOG_ERROR("File " << file << " cannot be opened for writing. Tracing disabled");
    return;
  }

  fprintf(stderr, "Installing tracers on all nodes in nodecontainer...\n");
  // for each node in the node container, install the tracer with that one output stream
  for (NodeContainer::Iterator node = nodes.Begin(); node != nodes.End(); node++) {
    Ptr<DASHPlayerTracer> trace = Install(*node, os);
    tracers.push_back(trace);
    m_allTracers.push_back(trace);
  }
  fprintf(stderr, "Done!\n");

  if (tracers.size() > 0) {
    tracers.front()->PrintHeader(*os);
    *os << "\n";
  }
}

void
DASHPlayerTracer::Install(Ptr<Node> node, const std::string& file)
{
  using namespace std;

  std::list< Ptr< DASHPlayerTracer > > tracers;

  boost::shared_ptr<std::ofstream> os(new std::ofstream());
  os->open(file.c_str(), std::ios_base::out | std::ios_base::trunc);

  if (!os->is_open()) {
    NS_LOG_ERROR("File " << file << " cannot be opened for writing. Tracing disabled");
    return;
  }

  Ptr<DASHPlayerTracer> trace = Install(node, os);
  tracers.push_back(trace);
  m_allTracers.push_back(trace);

  if (tracers.size() > 0) {
    tracers.front()->PrintHeader(*os);
    *os << "\n";
  }
}

Ptr<DASHPlayerTracer>
DASHPlayerTracer::Install(Ptr<Node> node, boost::shared_ptr<std::ofstream> outputStream)
{
  NS_LOG_DEBUG("Node: " << node->GetId());

  Ptr<DASHPlayerTracer> trace = CreateObject<DASHPlayerTracer>(outputStream, node);

  return trace;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

DASHPlayerTracer::DASHPlayerTracer(boost::shared_ptr<std::ofstream> os, Ptr<Node> node)
  : m_nodePtr(node)
  , m_os(os)
{
  std::stringstream node_id_str;
  node_id_str << m_nodePtr->GetId();

  m_node = node_id_str.str();

  // if this is a node with only 1 application, connect directly to the player tracer (if available)
  if (node->GetNApplications() == 1)
  {
    node->GetApplication(0)->TraceConnectWithoutContext ("PlayerTracer", MakeCallback(&DASHPlayerTracer::ConsumeStats,
                                             this));
  } else {
    // else: use normal connect method
    Connect();
  }
}

DASHPlayerTracer::DASHPlayerTracer(boost::shared_ptr<std::ofstream> os, const std::string& node)
  : m_node(node)
  , m_os(os)
{
  Connect();
}

DASHPlayerTracer::~DASHPlayerTracer()
{
  m_os->close();
};

void
DASHPlayerTracer::Connect()
{
  Config::ConnectWithoutContext("/NodeList/" + m_node
                                  + "/ApplicationList/*/PlayerTracer",
                                MakeCallback(&DASHPlayerTracer::ConsumeStats,
                                             this));
}

void
DASHPlayerTracer::PrintHeader(std::ofstream& os) const
{
  os << "Time"
     << "\t"
     << "Node\tUserId"
     << "\t"
     << "SegmentNumber"
     << "\t"
     << "SegmentRepID"
     << "\t"
     << "SegmentExperiencedBitrate(bit/s)"
     << "\t"
     << "BufferLevel(s)"
     << "\t"
     << "StallingTime(msec)"
     << "\t"
     << "SegmentDepIds";
}

void
DASHPlayerTracer::ConsumeStats(Ptr<ns3::Application> app, unsigned int userId,
                               unsigned int segmentNr, std::string representationId,
                               unsigned int segmentExperiencedBitrate,
                               unsigned int stallingTime, unsigned int bufferLevel,
                               std::vector<std::string> dependencyIds)
{
  std::string depIdStr = "";

  for(std::vector<std::string>::iterator it = dependencyIds.begin(); it != dependencyIds.end(); it++ )
  {
    if(depIdStr.compare ("") == 0)
      depIdStr.append(*it);
    else
      depIdStr.append (","+*it);
  }

  (*m_os) << Simulator::Now().ToDouble(Time::S) << "\t" << m_node << "\t" << userId << "\t" /*<< app->GetId() << "\t"*/
        << segmentNr << "\t" << representationId << "\t"
        << segmentExperiencedBitrate << "\t" << bufferLevel << "\t" << stallingTime << "\t" << depIdStr << "\n";
}



} // namespace ns3
