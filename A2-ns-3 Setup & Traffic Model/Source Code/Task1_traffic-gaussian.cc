#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

#include <fstream>
#include <iomanip>
#include <random>
#include <map>
#include <string>

using namespace ns3;

static std::ofstream g_iat;
static std::ofstream g_rtt;
static uint32_t g_seq = 0;
static std::map<uint32_t, double> g_sendTimes;
static Ptr<Socket> g_sinkSocket;
static uint16_t g_echoPort = 9000;

class GaussianUdpApp : public Application
{
public:
  GaussianUdpApp() {}
  void Setup(Address peer, uint32_t count, uint32_t payloadBytes,
             double meanIat, double stdIat, double minIat, double maxIat,
             uint32_t seed)
  {
    m_peer = peer; m_count = count; m_payloadBytes = payloadBytes;
    m_mean = meanIat; m_std = stdIat; m_min = minIat; m_max = maxIat;
    m_seed = seed;
  }

private:
  void StartApplication() override
  {
    m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    m_socket->Connect(m_peer);
    m_socket->SetRecvCallback(MakeCallback(&GaussianUdpApp::ReadReply, this));
    m_rng.seed(m_seed);
    g_iat << "sequence,send_time_s,inter_arrival_s,payload_bytes\n";
    SendOne();
  }

  void StopApplication() override
  {
    if (m_event.IsPending()) Simulator::Cancel(m_event);
    if (m_socket) m_socket->Close();
  }

  double SampleIat()
  {
    // Rejection sampling: samples outside [minIat, maxIat] are rejected.
    std::normal_distribution<double> normal(m_mean, m_std);
    double x;
    do { x = normal(m_rng); } while (x < m_min || x > m_max);
    return x;
  }

  void SendOne()
  {
    if (m_sent >= m_count) return;
    const double now = Simulator::Now().GetSeconds();
    double iat = 0.0;
    if (m_sent > 0) iat = now - m_lastSend;
    m_lastSend = now;

    uint32_t seq = g_seq++;
    g_sendTimes[seq] = now;
    uint8_t *buf = new uint8_t[m_payloadBytes];
    // Simple sequence marker at the start of the UDP payload.
    if (m_payloadBytes >= 4) {
      buf[0] = (seq >> 24) & 0xff; buf[1] = (seq >> 16) & 0xff;
      buf[2] = (seq >> 8) & 0xff;  buf[3] = seq & 0xff;
    }
    Ptr<Packet> packet = Create<Packet>(buf, m_payloadBytes);
    delete[] buf;
    m_socket->Send(packet);
    g_iat << seq << "," << std::fixed << std::setprecision(9)
          << now << "," << iat << "," << m_payloadBytes << "\n";
    ++m_sent;
    if (m_sent < m_count) m_event = Simulator::Schedule(Seconds(SampleIat()),
                                                         &GaussianUdpApp::SendOne, this);
  }

  void ReadReply(Ptr<Socket> socket)
  {
    Address from;
    Ptr<Packet> p;
    while ((p = socket->RecvFrom(from))) {
      uint8_t seqBytes[4] = {0,0,0,0};
      if (p->GetSize() >= 4) p->CopyData(seqBytes, 4);
      uint32_t seq = (uint32_t(seqBytes[0]) << 24) |
                     (uint32_t(seqBytes[1]) << 16) |
                     (uint32_t(seqBytes[2]) << 8) | uint32_t(seqBytes[3]);
      auto it = g_sendTimes.find(seq);
      if (it != g_sendTimes.end()) {
        g_rtt << seq << "," << std::fixed << std::setprecision(9)
              << it->second << "," << Simulator::Now().GetSeconds() << ","
              << (Simulator::Now().GetSeconds() - it->second) << "\n";
      }
    }
  }

  Ptr<Socket> m_socket;
  Address m_peer;
  uint32_t m_count = 1000, m_payloadBytes = 512, m_sent = 0, m_seed = 12345;
  double m_mean = 0.03, m_std = 0.01, m_min = 0.005, m_max = 0.08, m_lastSend = 0;
  std::mt19937 m_rng;
  EventId m_event;
};

int main(int argc, char *argv[])
{
  uint32_t packets = 1000, payloadBytes = 512, seed = 12345;
  double meanIat = 0.030, stdIat = 0.010, minIat = 0.005, maxIat = 0.080;
  double simStop = 45.0;
  std::string prefix = "traffic";

  CommandLine cmd(__FILE__);
  cmd.AddValue("packets", "Number of UDP packets", packets);
  cmd.AddValue("payloadBytes", "Fixed UDP application payload bytes", payloadBytes);
  cmd.AddValue("meanIat", "Mean inter-arrival time in seconds", meanIat);
  cmd.AddValue("stdIat", "Standard deviation of inter-arrival time in seconds", stdIat);
  cmd.AddValue("minIat", "Minimum allowed inter-arrival time in seconds", minIat);
  cmd.AddValue("maxIat", "Maximum allowed inter-arrival time in seconds", maxIat);
  cmd.AddValue("seed", "Random seed", seed);
  cmd.AddValue("simStop", "Simulation stop time in seconds", simStop);
  cmd.AddValue("prefix", "Output filename prefix", prefix);
  cmd.Parse(argc, argv);

  g_iat.open(prefix + "-iat.csv");
  g_rtt.open(prefix + "-rtt.csv");
  g_rtt << "sequence,send_time_s,reply_time_s,rtt_s\n";

  NodeContainer nodes; nodes.Create(2);
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("20Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("5ms"));
  NetDeviceContainer devices = p2p.Install(nodes);

  InternetStackHelper stack; stack.Install(nodes);
  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign(devices);

  UdpEchoServerHelper server(g_echoPort);
  ApplicationContainer serverApps = server.Install(nodes.Get(1));
  serverApps.Start(Seconds(0.1)); serverApps.Stop(Seconds(simStop));

  UdpEchoClientHelper echoClient(interfaces.GetAddress(1), g_echoPort);
  echoClient.SetAttribute("MaxPackets", UintegerValue(0));
  // The custom application sends packets at Gaussian-truncated intervals.
  Ptr<GaussianUdpApp> app = CreateObject<GaussianUdpApp>();
  app->Setup(InetSocketAddress(interfaces.GetAddress(1), g_echoPort),
             packets, payloadBytes, meanIat, stdIat, minIat, maxIat, seed);
  nodes.Get(0)->AddApplication(app);
  app->SetStartTime(Seconds(1.0));
  app->SetStopTime(Seconds(simStop - 0.1));

  // Capture both directions on the point-to-point devices.
  p2p.EnablePcapAll(prefix, true);

  FlowMonitorHelper flowHelper;
  Ptr<FlowMonitor> monitor = flowHelper.InstallAll();
  Simulator::Stop(Seconds(simStop));
  Simulator::Run();
  monitor->SerializeToXmlFile(prefix + "-flowmon.xml", true, true);
  Simulator::Destroy();
  g_iat.close(); g_rtt.close();
  return 0;
}
