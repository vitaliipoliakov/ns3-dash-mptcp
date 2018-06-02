#include "node-throughput-tracer.h"

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

NS_LOG_COMPONENT_DEFINE("ns3.NodeThroughputTracer");

using namespace std;

namespace ns3 {


static std::list< Ptr< NodeThroughputTracer > > m_allTracers;


void
NodeThroughputTracer::Destroy()
{
  m_allTracers.clear();
}

void
NodeThroughputTracer::InstallAll(const std::string& file)
{
  using namespace std;

  std::list< Ptr< NodeThroughputTracer > > tracers;

  boost::shared_ptr<std::ofstream> os(new std::ofstream());
  os->open(file.c_str(), std::ios_base::out | std::ios_base::trunc);

  if (!os->is_open()) {
    NS_LOG_ERROR("File " << file << " cannot be opened for writing. Tracing disabled");
    return;
  }


  for (NodeList::Iterator node = NodeList::Begin(); node != NodeList::End(); node++) {
    Ptr<NodeThroughputTracer> trace = Install(*node, os);
    tracers.push_back(trace);
    m_allTracers.push_back(trace);
  }

  if (tracers.size() > 0) {
    tracers.front()->PrintHeader(*os);
    *os << "\n";
  }
}

void
NodeThroughputTracer::Install(const NodeContainer& nodes, const std::string& file)
{
  using namespace std;

  std::list< Ptr< NodeThroughputTracer > > tracers;

  boost::shared_ptr<std::ofstream> os(new std::ofstream());
  os->open(file.c_str(), std::ios_base::out | std::ios_base::trunc);

  if (!os->is_open()) {
    NS_LOG_ERROR("File " << file << " cannot be opened for writing. Tracing disabled");
    return;
  }


  for (NodeContainer::Iterator node = nodes.Begin(); node != nodes.End(); node++) {
    Ptr<NodeThroughputTracer> trace = Install(*node, os);
    tracers.push_back(trace);
    m_allTracers.push_back(trace);
  }

  if (tracers.size() > 0) {
    tracers.front()->PrintHeader(*os);
    *os << "\n";
  }
}

void
NodeThroughputTracer::Install(Ptr<Node> node, const std::string& file)
{
  using namespace std;

  std::list< Ptr< NodeThroughputTracer > > tracers;

  boost::shared_ptr<std::ofstream> os(new std::ofstream());
  os->open(file.c_str(), std::ios_base::out | std::ios_base::trunc);

  if (!os->is_open()) {
    NS_LOG_ERROR("File " << file << " cannot be opened for writing. Tracing disabled");
    return;
  }

  Ptr<NodeThroughputTracer> trace = Install(node, os);
  tracers.push_back(trace);
  m_allTracers.push_back(trace);

  if (tracers.size() > 0) {
    tracers.front()->PrintHeader(*os);
    *os << "\n";
  }
}

Ptr<NodeThroughputTracer>
NodeThroughputTracer::Install(Ptr<Node> node, boost::shared_ptr<std::ofstream> outputStream)
{
  NS_LOG_DEBUG("Node: " << node->GetId());

  Ptr<NodeThroughputTracer> trace = CreateObject<NodeThroughputTracer>(outputStream, node);

  return trace;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

NodeThroughputTracer::NodeThroughputTracer(boost::shared_ptr<std::ofstream> os, Ptr<Node> node)
  : m_nodePtr(node)
  , m_os(os)
{
  std::stringstream node_id_str;
  node_id_str << m_nodePtr->GetId();

  m_node = node_id_str.str();

  Connect();
}

NodeThroughputTracer::NodeThroughputTracer(boost::shared_ptr<std::ofstream> os, const std::string& node)
  : m_node(node)
  , m_os(os)
{
  Connect();
}

NodeThroughputTracer::~NodeThroughputTracer()
{
  m_os->close();
};

void
NodeThroughputTracer::Connect()
{
  Config::ConnectWithoutContext("/NodeList/" + m_node
                                  + "/ApplicationList/*/ThroughputTracer",
                                MakeCallback(&NodeThroughputTracer::ThroughputStats,
                                             this));
}

void
NodeThroughputTracer::PrintHeader(std::ofstream& os) const
{
  os << "Time\tNode\tTxBytes\tRxBytes\tOpenSockets";
}

void
NodeThroughputTracer::ThroughputStats(Ptr<ns3::Application> app, uint64_t txBytes, uint64_t rxBytes, uint32_t openSockets)
{
  (*m_os) << Simulator::Now().ToDouble(Time::S) << "\t" << m_node << "\t"
        << txBytes << "\t" << rxBytes << "\t" << openSockets << "\n";


  (*m_os).flush();
}



} // namespace ns3
