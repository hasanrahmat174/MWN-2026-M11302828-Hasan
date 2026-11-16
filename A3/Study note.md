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

## 2. Experimental Result  
Following are the parameters that can be taken from the journal to reproduce the experiment.

### 1. Topology 
The research uses Wi-Fi Infrastructure Mode topology.
| Parameter | Value |
| --- | --- |
| Network topology| Infrastructure Mode |
| Access Point | 1 AP |
| Number of stations | 2, 4, 6, 8, 10, 12 |
| Communication | STA → AP|
| Network architecture | Centralized AP|

This means that one Access Point serves as the central hub for receiving data from multiple wireless stations. The number of stations was changed to determine the impact of user numbers on network performance.

### 2. Traffic Model
The research used Voice traffic with saturated traffic conditions.
| Parameter | Value |
| --- | --- |
| Traffic type | Voice |
| Traffic condition | Saturated |
| Packet size | 1024 bytes |
| Application rate | Physical rate / S|
| Source start time | Random (1–5 seconds) |
| Buffer size | 256 packets|
| Beacon interval | 0.02 seconds |

Description:
- Voice: Traffic consists of voice data.
- Saturated traffic: Stations always have packets to send.
- Packet size: Each packet is 1024 bytes.
- Application rate: Application speed is calculated by dividing the physical rate by the number of stations (S).
- Source start time: The transmission start time is randomly generated between 1–5 seconds.

### 3. PHY/MAC Parameters
- PHY (Physical Layer)

| Parameter | Value |
| --- | --- |
| Wireless standard | IEEE 802.11b dan 802.11g |
| Data rate 802.11b | 11 Mbps |
| Data rate 802.11g | 54 Mbps |

- MAC (Medium Access Control)
The study used the following EDCA parameters:

| Access Category | CWmin (802.11b / g) | CWmax (802.11b / g) | AIFSN | TXOP (802.11b / g)| 
| --- | --- | --- | --- | --- | 
| AC_BK | 31 / 15 | 1023 / 1023	| 7 | 0 / 0 | 
| AC_BE | 31 / 15 | 1023 / 1023	| 3 | 0 / 0 | 
| AC_VI | 15 / 7 | 31 / 15| 2 | 6.016 / 3.008 ms | 
| AC_VO | 7 / 3 | 15 / 7 | 2 | 3.264 / 1.504 ms | 

These values ​​are the default EDCA parameters listed in the journal.

### 4. Simulation Time

| Parameter | Value |
| --- | --- |
| Simulation time | 4 minutes |
| In seconds | 240 seconds |
| Source start time | Random 1–5 seconds |

This means that each experiment was run for 240 seconds.

### 5. Seeds (Random Seed)

The journal states that the station start time is generated randomly between 1–5 seconds.

## 3. Revise from A1 
1. Changed the title from Analysis of the Influence of Bandwidth on QoS in IP Networks Using NS-3 to Analysis of the Influence of PHY Data Rate Variations on Quality of Service (QoS) in IEEE 802.11e-Based WLAN Networks Using NS-3.
   
2. References to the previous summary of the system model: Environment, Inputs, Outputs, Performance Metrics and experimental results of topology, traffic model, PHY/MAC parameters, simulation time, seeds were added to the final project.

3. The research objective changes to the variation of PHY Data Rate on QoS in IEEE 802.11e-Based WLAN Networks Using NS-3 which has an impact on the following 4 parameters: throughput, jitter, delay, packet loss.


