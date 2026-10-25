#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

using namespace ns3;

namespace {
constexpr uint8_t TYPE1 = 1;
constexpr uint8_t TYPE2 = 2;

struct TrafficConfig {
  uint8_t type;
  double peakMbps;
  double meanMbps;
  double peakDuration;
  uint32_t packetBytes;
  double meanOnOffPeriod;
};

struct QueuedPacket {
  Ptr<Packet> packet;
  uint8_t type;
  uint32_t bytes;
  Time enqueued;
};

class TwoClassAtmModel : public Application {
public:
  void Configure(Ipv4Address destination, uint16_t port, double linkMbps,
                 double reservedMbps, uint32_t k1, uint32_t k2,
                 TrafficConfig t1, TrafficConfig t2, double stopTime) {
    m_destination = destination;
    m_port = port;
    m_capacityMbps = linkMbps;
    m_reservedMbps = reservedMbps;
    m_k1 = k1;
    m_k2 = k2;
    m_cfg[0] = t1;
    m_cfg[1] = t2;
    m_stopTime = stopTime;
  }

private:
  void StartApplication() override {
    m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    m_socket->Connect(InetSocketAddress(m_destination, m_port));
    m_csv.open("atm_trace.csv");
    m_csv << "time_s,event,class,q1_packets,q2_packets,delta_q_packets_s,pl_est,"
             "Ca_Mbps,Ce_Mbps,action_y,decision_z,throughput_Mbps,delay_ms,dropped\n";
    m_stats.open("atm_summary.txt");
    m_start = Simulator::Now();
    m_lastQueueUpdate = Simulator::Now();
    // Each class is admitted independently at setup.
   // Admission decision for Type-1
    std::string decision1 = Admission(TYPE1);
    PrintDecision(TYPE1, decision1);
    // Admission decision for Type-2
    std::string decision2 = Admission(TYPE2);
    PrintDecision(TYPE2, decision2);

    ScheduleNextPacket(0, Seconds(0.001));
    ScheduleNextPacket(1, Seconds(0.002));
    m_serviceEvent = Simulator::Schedule(MicroSeconds(1), &TwoClassAtmModel::Service, this);
    m_monitorEvent = Simulator::Schedule(MilliSeconds(10), &TwoClassAtmModel::Monitor, this);
  }

  void StopApplication() override {
    if (m_serviceEvent.IsPending()) Simulator::Cancel(m_serviceEvent);
    if (m_monitorEvent.IsPending()) Simulator::Cancel(m_monitorEvent);
    for (auto &e : m_generateEvents) if (e.IsPending()) Simulator::Cancel(e);
    if (m_csv.is_open()) m_csv.close();

    double elapsed = (Simulator::Now() - m_start).GetSeconds();
    double throughput = elapsed > 0 ? (8.0 * m_rxBytes / elapsed / 1e6) : 0;
    m_stats << std::fixed << std::setprecision(6)
            << "Simulation duration (s): " << elapsed << "\n"
            << "Output link capacity (Mbps): " << m_capacityMbps << "\n"
            << "Reserved Type-1 capacity Cr (Mbps): " << m_reservedMbps << "\n"
            << "Type-1 generated/admitted/dropped: " << m_generated[0] << "/"
            << m_admittedPackets[0] << "/" << m_dropped[0] << "\n"
            << "Type-2 generated/admitted/dropped: " << m_generated[1] << "/"
            << m_admittedPackets[1] << "/" << m_dropped[1] << "\n"
            << "Overall packet loss probability: "
            << (m_generated[0] + m_generated[1] > 0 ?
                double(m_dropped[0] + m_dropped[1]) /
                double(m_generated[0] + m_generated[1]) : 0) << "\n"
            << "Received throughput (Mbps): " << throughput << "\n"
            << "Mean queueing delay (ms): "
            << (m_servedPackets > 0 ? 1000.0 * m_delaySum / m_servedPackets : 0) << "\n"
            << "QoS target (loss <= 1e-5): "
            << ((m_generated[0] + m_generated[1] > 0 &&
                 double(m_dropped[0] + m_dropped[1]) /
                 double(m_generated[0] + m_generated[1]) <= 1e-5) ? "MET" : "NOT MET")
            << "\n";
    m_stats.close();
  }

  // Equivalent capacity estimate: mean rate plus a burst allowance.
  double EquivalentCapacity(const TrafficConfig &c) const {
    double burstFactor = std::min(1.0, c.peakDuration / std::max(0.001, m_stopTime));
    return c.meanMbps + std::max(0.0, c.peakMbps - c.meanMbps) * burstFactor;
  }

  std::string Admission(uint8_t type) {
    const unsigned i = type - 1;
    double ce = EquivalentCapacity(m_cfg[i]);
    double used = EquivalentCapacity(m_cfg[0]) * (m_admitted[0] ? 1.0 : 0.0)
                + EquivalentCapacity(m_cfg[1]) * (m_admitted[1] ? 1.0 : 0.0);
    // Initial decisions: Type-1 can use reserved capacity; Type-2 cannot consume it.
    double allowed = (type == TYPE1) ? m_capacityMbps : (m_capacityMbps - m_reservedMbps);
    double projected = used + ce;
    if (projected <= allowed) return "Accept";
    if (projected <= allowed * 1.10) return "WA"; // Warning / admit with warning
    if (projected <= allowed * 1.25) return "WR"; // Warning / rate-reduce then retry
    return "Reject";
  }

  void PrintDecision(uint8_t type, const std::string &decision) {
    const unsigned i = type - 1;
    m_ce[i] = EquivalentCapacity(m_cfg[i]);
    m_admitted[i] = (decision == "Accept" || decision == "WA" || decision == "WR");
    if (decision == "WR") m_rateScale[i] = 0.75;
    if (decision == "Reject") m_rateScale[i] = 0.0;
    std::cout << "Admission Type-" << unsigned(type)
              << ": Ce=" << m_ce[i] << " Mbps, z=" << decision
              << ", admitted=" << (m_admitted[i] ? "yes" : "no") << "\n";
    Log("admission", type, 0, 0, 0, 0.0, AvailableCapacity(), m_ce[i],
        CongestionAction(), decision, 0.0, 0.0, false);
  }

  double AvailableCapacity() const {
    double queuedMbps = (m_q[0] + m_q[1]) * 8.0 * m_cfg[0].packetBytes / 1e6;
    return std::max(0.0, m_capacityMbps - queuedMbps);
  }

  std::string CongestionAction() const {
    double q = double(m_q[0] + m_q[1]);
    double dq = (q - m_previousQ) / 0.01;
    double pl = double(m_dropped[0] + m_dropped[1]) /
                std::max(1.0, double(m_generated[0] + m_generated[1]));
    if (pl > 1e-5 || q > 0.8 * (m_k1 + m_k2) || dq > 50.0) return "DM";
    if (q > 0.6 * (m_k1 + m_k2) || dq > 10.0) return "DS";
    if (q < 0.2 * (m_k1 + m_k2) && dq < 0.0) return "IM";
    if (q < 0.4 * (m_k1 + m_k2)) return "IS";
    return "NC";
  }

  void ScheduleNextPacket(unsigned i, Time delay) {
    if (Simulator::Now() + delay < Seconds(m_stopTime)) {
      m_generateEvents[i] = Simulator::Schedule(delay, &TwoClassAtmModel::Generate, this, i);
    }
  }

  void Generate(unsigned i) {
    const TrafficConfig &c = m_cfg[i];
    uint8_t type = c.type;
    ++m_generated[i];

    if (!m_admitted[i] || m_rateScale[i] <= 0.0) {
      Log("source_blocked", type, m_q[0], m_q[1], 0, CurrentLoss(), AvailableCapacity(),
          m_ce[i], CongestionAction(), "Reject", 0, 0, true);
      ScheduleNextPacket(i, NextInterarrival(i));
      return;
    }

    Ptr<Packet> p = Create<Packet>(c.packetBytes);
    uint32_t limit = (type == TYPE1) ? m_k1 : m_k2;
    if (m_q[i] >= limit) {
      ++m_dropped[i];
      Log("buffer_drop", type, m_q[0], m_q[1], 0, CurrentLoss(), AvailableCapacity(),
          m_ce[i], CongestionAction(), "Accept", 0, 0, true);
    } else {
      m_queues[i].push_back({p, type, c.packetBytes, Simulator::Now()});
      ++m_q[i];
      ++m_admittedPackets[i];
      Log("enqueue", type, m_q[0], m_q[1], 0, CurrentLoss(), AvailableCapacity(),
          m_ce[i], CongestionAction(), "Accept", 0, 0, false);
    }
    ScheduleNextPacket(i, NextInterarrival(i));
  }

  Time NextInterarrival(unsigned i) {
    const TrafficConfig &c = m_cfg[i];
    // Alternate peak and non-peak intervals. Rm/Rp determine average packet spacing.
    double rate = c.meanMbps;
    double phase = std::fmod(Simulator::Now().GetSeconds(), std::max(0.01, 2*c.peakDuration));
    if (phase < c.peakDuration) rate = c.peakMbps;
    rate = std::max(0.001, rate * std::max(0.0, m_rateScale[i]));
    double seconds = double(c.packetBytes) * 8.0 / (rate * 1e6);
    return Seconds(std::max(0.0001, seconds));
  }

  void Service() {
    if (Simulator::Now() >= Seconds(m_stopTime)) return;
    // Strict priority for Type-1 while preserving Type-2 access to unreserved capacity.
    int selected = -1;
    if (!m_queues[0].empty()) {
      selected = 0;
    } else if (!m_queues[1].empty()) {
      selected = 1;
    }
    if (selected >= 0) {
      QueuedPacket item = m_queues[selected].front();
      m_queues[selected].pop_front();
      --m_q[selected];
      m_socket->Send(item.packet);
      ++m_servedPackets;
      m_rxBytes += item.bytes;
      m_delaySum += (Simulator::Now() - item.enqueued).GetSeconds();
      double serviceSeconds = double(item.bytes) * 8.0 / (m_capacityMbps * 1e6);
      Log("dequeue_tx", item.type, m_q[0], m_q[1], 0, CurrentLoss(),
          AvailableCapacity(), m_ce[item.type - 1], CongestionAction(), "Accept",
          m_capacityMbps, 1000.0*(Simulator::Now()-item.enqueued).GetSeconds(), false);
      m_serviceEvent = Simulator::Schedule(Seconds(std::max(1e-7, serviceSeconds)),
                                            &TwoClassAtmModel::Service, this);
    } else {
      m_serviceEvent = Simulator::Schedule(MicroSeconds(10), &TwoClassAtmModel::Service, this);
    }
  }

  double CurrentLoss() const {
    uint64_t total = m_generated[0] + m_generated[1];
    return total ? double(m_dropped[0] + m_dropped[1]) / total : 0.0;
  }

  void Monitor() {
    if (Simulator::Now() >= Seconds(m_stopTime)) return;
    double q = double(m_q[0] + m_q[1]);
    double dq = (q - m_previousQ) / 0.01;
    m_previousQ = q;
    std::string action = CongestionAction();
    // Congestion control actions are applied to Type-2 first to protect real-time traffic.
    if (action == "DM") { m_rateScale[1] = std::max(0.25, m_rateScale[1] * 0.75); }
    else if (action == "DS") { m_rateScale[1] = std::max(0.40, m_rateScale[1] * 0.90); }
    else if (action == "IS") { m_rateScale[1] = std::min(1.0, m_rateScale[1] + 0.02); }
    else if (action == "IM") { m_rateScale[1] = std::min(1.0, m_rateScale[1] + 0.05); }

    Log("monitor", 0, m_q[0], m_q[1], dq, CurrentLoss(), AvailableCapacity(),
        std::max(m_ce[0], m_ce[1]), action, "-", m_capacityMbps, 0.0, false);
    m_monitorEvent = Simulator::Schedule(MilliSeconds(10), &TwoClassAtmModel::Monitor, this);
  }

  void Log(std::string event, uint8_t type, uint32_t q1, uint32_t q2, double dq,
           double pl, double ca, double ce, std::string y, std::string z,
           double throughput, double delayMs, bool dropped) {
    if (!m_csv.is_open()) return;
    m_csv << std::fixed << std::setprecision(8) << Simulator::Now().GetSeconds()
          << "," << event << "," << unsigned(type) << "," << q1 << "," << q2
          << "," << dq << "," << pl << "," << ca << "," << ce << "," << y
          << "," << z << "," << throughput << "," << delayMs << ","
          << (dropped ? 1 : 0) << "\n";
  }

  Ipv4Address m_destination;
  uint16_t m_port{9000};
  double m_capacityMbps{10.0}, m_reservedMbps{2.0}, m_stopTime{10.0};
  uint32_t m_k1{100}, m_k2{200};
  TrafficConfig m_cfg[2]{};
  Ptr<Socket> m_socket;
  std::ofstream m_csv, m_stats;
  Time m_start, m_lastQueueUpdate;
  EventId m_serviceEvent, m_monitorEvent, m_generateEvents[2];
  std::deque<QueuedPacket> m_queues[2];
  uint32_t m_q[2]{0,0};
  uint64_t m_generated[2]{0,0}, m_admittedPackets[2]{0,0}, m_dropped[2]{0,0};
  bool m_admitted[2]{false,false};
  double m_ce[2]{0,0}, m_rateScale[2]{1.0,1.0};
  uint64_t m_rxBytes{0}, m_servedPackets{0};
  double m_delaySum{0.0}, m_previousQ{0.0};
};
} // namespace

int main(int argc, char *argv[]) {
  double capacityMbps = 10.0;
  double reservedMbps = 2.0;
  uint32_t k1 = 100, k2 = 200;
  double stopTime = 20.0;
  double r1Peak = 2.0, r1Mean = 1.0, tp1 = 0.5;
  double r2Peak = 8.0, r2Mean = 3.0, tp2 = 0.3;
  uint32_t packetBytes = 1000;

  CommandLine cmd(__FILE__);
  cmd.AddValue("C", "Total output-link capacity in Mbps", capacityMbps);
  cmd.AddValue("Cr", "Capacity reserved for Type-1 in Mbps", reservedMbps);
  cmd.AddValue("K1", "Type-1 buffer capacity in packets", k1);
  cmd.AddValue("K2", "Type-2 buffer capacity in packets", k2);
  cmd.AddValue("Rp1", "Type-1 peak bit rate in Mbps", r1Peak);
  cmd.AddValue("Rm1", "Type-1 mean bit rate in Mbps", r1Mean);
  cmd.AddValue("Tp1", "Type-1 peak duration in seconds", tp1);
  cmd.AddValue("Rp2", "Type-2 peak bit rate in Mbps", r2Peak);
  cmd.AddValue("Rm2", "Type-2 mean bit rate in Mbps", r2Mean);
  cmd.AddValue("Tp2", "Type-2 peak duration in seconds", tp2);
  cmd.AddValue("packetBytes", "Payload packet size in bytes", packetBytes);
  cmd.AddValue("simTime", "Simulation duration in seconds", stopTime);
  cmd.Parse(argc, argv);

  if (capacityMbps <= 0 || reservedMbps < 0 || reservedMbps >= capacityMbps ||
      k1 == 0 || k2 == 0 || packetBytes == 0 || r1Peak <= 0 || r1Mean <= 0 ||
      r2Peak <= 0 || r2Mean <= 0 || tp1 <= 0 || tp2 <= 0 || stopTime <= 0) {
    NS_FATAL_ERROR("Invalid parameters: require C>Cr>=0, positive rates, buffers, durations and simTime.");
  }

  NodeContainer nodes;
  nodes.Create(2);
  InternetStackHelper internet;
  internet.Install(nodes);

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue(std::to_string(capacityMbps) + "Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));
  NetDeviceContainer devices = p2p.Install(nodes);
  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign(devices);

  uint16_t port = 9000;
  PacketSinkHelper sinkHelper("ns3::UdpSocketFactory",
      InetSocketAddress(Ipv4Address::GetAny(), port));
  ApplicationContainer sinkApps = sinkHelper.Install(nodes.Get(1));
  sinkApps.Start(Seconds(0.0));
  sinkApps.Stop(Seconds(stopTime + 1.0));

  TrafficConfig t1{TYPE1, r1Peak, r1Mean, tp1, packetBytes, 0.0};
  TrafficConfig t2{TYPE2, r2Peak, r2Mean, tp2, packetBytes, 0.0};
  Ptr<TwoClassAtmModel> model = CreateObject<TwoClassAtmModel>();
  model->Configure(interfaces.GetAddress(1), port, capacityMbps, reservedMbps,
                   k1, k2, t1, t2, stopTime);
  nodes.Get(0)->AddApplication(model);
  model->SetStartTime(Seconds(0.0));
  model->SetStopTime(Seconds(stopTime));

  p2p.EnablePcapAll("atm-two-class", true);
  Simulator::Stop(Seconds(stopTime + 0.1));
  Simulator::Run();
  Simulator::Destroy();
  std::cout << "\nCreated: atm_trace.csv, atm_summary.txt, atm-two-class-*.pcap\n";
  return 0;
}
