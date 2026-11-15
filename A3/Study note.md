*A3-Literature review**
> **Paper:** Islam Obaidat, Mohammad Alsmirat, and Yaser Jararweh, *"Completing IEEE 802.11e Implementation in NS-3,"* IEEE Wireless Communications, 2003.

---
---

## 1. Introduction & Goal & Overview

### 1. Problem
Nowadays, very large percentage of data communications approaches have converged toward wireless communication. The IEEE 802.11 standard [1] is one of the most adopted wireless communication standards. It is commonly used for wireless local area networks (WLAN) in infrastructure mode
due to its easy deployment and use. However, due to the characteristics of the wireless connections, wireless communication sustains performance drawbacks. The IEEE 802.11 thus, should provide appropriate wireless services to overcome these drawbacks, especially for time sensitive applications. The Wireless network connection should guarantee Quality of Service (QoS) for real time applications such as multimedia applications.

### 2. Importance
IEEE 802.11e is a Wi-Fi standard that supports Quality of Service (QoS) mechanisms. One of the main mechanisms is EDCA (Enhanced Distributed Channel Access), which assigns different priorities to traffic types such as voice, video, best effort, and background traffic.This research discusses the implementation of the EDCA (Enhanced Distributed Channel Access) mechanism on ns-3 to support QoS.The author found that the EDCA implementation on ns-3 at that time did not fully support the TXOP function. Therefore, they developed an implementation of TXOP and compared the simulation results of ns-3 with OPNET.

The four traffic categories used in IEEE 802.11e are:
- AC_VO: Voice
- AC_VI: Videos
- AC_BE: Best Effort
- AC_BK: Background
The EDCA mechanism uses CWmin, CWmax, AIFS, and TXOP parameters to differentiate traffic priorities.

### 3. Challenges
802.11 EDCA is fully implemented in OPNET. On the other hand, not all EDCA components are implemented in NS-3. NS-3 supports only the use of CWmin, CWmax, and AIFS. TXOP is considered to be a main component for EDCA but, unfortunately, it is not implemented in NS-31.
In this work, we complete the NS-3 EDCA implementation by implementing the TXOP full functionalities. In order to
implement TXOP in NS-3, many changes must be done to the current implementation and many testing is needed to verify the implementation correctness.

## 2. System Model 

### 1. Environment 
Describes the network environment used in the research.

| Parameter | Configuration |
| --- | --- |
| Network Simulator | ns-3 version 3.22 |
| Network Type | 	Wireless LAN (WLAN) |
| Network Topology | Infrastructure Mode |
| Access Point | 1 AP |
| Wireless Stations | 2, 4, 6, 8, 10, 12 |
| Wireless Standard| IEEE 802.11b / 802.11g |
| MAC Protocol | IEEE 802.11e EDCA |
| Communication| Wireless Station → Access Point |

Explanation:
This study uses a WLAN network with one Access Point and several Wireless Stations. Each station sends data to the AP. The EDCA mechanism is used to prioritize channel access based on traffic category.

### 2. Input 
Describes the network environment used in the research.

| Parameter | Configuration |
| --- | --- |
| Traffic Type | 	Voice |
| Traffic Condition | Saturated Traffic |
| Packet Size | 1024 bytes |
| Access Point | 1 AP |
| Application Rate | Physical Rate / Number of Stations |
| Source Start Time| Random 1–5 seconds |
| Buffer Size | 256 packets |
| Beacon Interval| 0.02 seconds |
| PHY Data Rate | 11 Mbps / 54 Mbps |
| CWmin| Based on Access Category |
| CWmax |Based on Access Category |
| AIFSN| Based on Access Category |
| TXOP | Based on Access Category |
| Simulation Time| 240 seconds |

Explanation:
The input parameters are used to test how the IEEE 802.11e EDCA mechanism affects voice traffic delivery. The number of stations and EDCA parameters affect a station's access to the transmission medium.

### 3. Output 
Outputs are data obtained after the ns-3 simulation is run.
| Output | Explanation |
| --- | --- |
| Packets Transmitted | Number of packets sent by the station |
| Packets Received | Number of packets received by AP |
| Total Received Bytes | Total size of data received by AP |
| Packet Transmission Time | Packet transmission time|
| Throughput Results | Throughput calculation results |
| Delay Results| Delay calculation results |

After the simulation is complete, ns-3 generates packet transmission and reception data. This data is used to determine the success of communication between the station and the AP.

### 4. Performance Metrics
Performance metrics are indicators used to evaluate network performance.
The study used two main metrics:

A. Average Throughput
Measuring the average amount of data successfully received per unit of time.

<img width="274" height="60" alt="image" src="https://github.com/user-attachments/assets/de44200f-801c-4aaa-ab49-a4467cfad63b" />

Units: bps or Mbps. The higher the throughput, the greater the amount of data the AP can successfully receive per second.

B. Average Delay
Measuring the average time it takes for a packet to travel from the station to the AP.

<img width="186" height="64" alt="image" src="https://github.com/user-attachments/assets/f2d005eb-9e55-461b-be7a-e09a80af74d3" />

Units: ms. The lower the delay, the faster the packet is received by the AP.
