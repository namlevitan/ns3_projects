#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/stats-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Congestion Control");

//====================================================
//
//            node 0                     node 1
//   +---------------------+   +----------------------+
//   |     ns-3 TCP        |   |       ns-3 TCP       |
//   +---------------------+   +----------------------+
//   |     10.1.1.1        |   |        10.1.1.2      |
//   +---------------------+   +----------------------+
//   |   point-to-point    |   |  point-to-point      |
//   +---------------------+   +----------------------+
//              |                           |
//              +---------------------------+
//                     5 Mbps, 2 ms
// We want to look at the changes in TCP congestion window. We need
// to crank up a flow and hook the CongestionWindow attribute on the
// socket of the sender. Normally one would use an on-off applicationsto
// generate the flow, but this has a couple of problems. First, the socket
// of the on-off application is not created until Application Start time
// so we wouldn't be able to hook the socket (now) at the configuration time, the
// socket is not public so we couldn't get at it.

//
//
//
//

//
//
//
//
//======================================================================

class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp();

  /*Register this type.
  return The TypeId.
  static TypeId GetTypeId (void);
  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);
  */
private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SenPacket (void);

  Ptr<Socket>      m_socket;
  Address          m_peer;
  uint32_t         m_packetSize;
  uint32_t         m_nPackets;
  DataRate         m_dataRate;
  EventId          m_sendEvent;
  bool             m_running;
  uint32_t         m_packetsSent;

};

//constructor

MyApp::MyApp ()
   : m_socket (0),
     m_peer (),
     m_packetSize(0),
     m_nPackets(0),
     m_dataRate(0),
     m_sendEvent(),
     m_running(false),
     m_packetsSent(0)
{}

MyApp::~MyApp ()
{
  m_socket = 0;
}

/* static */

TypeId MyApp::GetTypeId(void)
{
  static TypeId tid = TypeId ("My application");
  .SetParent<Application>()
  .SetGroupName ("Demonstration")
  .AddConstructor<MyApp>()
  ;
  return tid;

}
void MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate datarate )
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;

}

void MyApp::StartApplication(void)
{
  m_running = true;
  m_packetsSent = 0;
  if (InetSocketAddress::IsMatchingType (m_peer))
  {
    m_socket->Bind();
  }
  else
  {
    m_socket->Bind6();
  }
  m_socket->Connect (m_peer);
  SendPacket ();
}

void MyApp::StopApplication(void)
{
  m_running = false;
  if (m_sendEvent.IsRunning())
  {
    Simulator::Cancel (m_sendEvent);
  }
  if (m_socket)
  {
    m_socket->Close();
  }
}

void MyApp::SendPacket(void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send(Packet);
  if (++m_packetsSent < m_nPackets)
  {
    ScheduleTx();
  }
}

void MyApp::ScheduleTx(void)
{
  if (m_running)
  {
    Time tNext (Second (m_packetSize*8/static_cast<double>(m_dataRate.GetBitRate())));
    m_sendEvent = Simulator::ScheduleTx(tNext,&MyApp::SendPacket,this)
  }
}

static void CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  NS_LOG_UNCOND (Simulator::Now().GetSeconds()<<"\t"<<newCwnd);
  *stream->GetStream() << Simulator::Now().GetSeconds()<<"\t"<<oldCwnd<<"\t"<<newCwnd<<"\t"<<std::endl;
}

static void RxDrop (Ptr<PcapFileWrapper> file, Ptr<const Packet> p)
{
  NS_LOG_UNCOND ("RxDrop at "<<Simulator::Now().GetSeconds());
  file->Write(Simulator::Now(),p);
}
