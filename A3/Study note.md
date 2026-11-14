*A3-Literature review**

---
---

## 1. Introduction & Goal & Overview

### 1. Problem
Wireless Local Area Networks (WLANs) originally relied on the Distributed Coordination Function (DCF), which operates as a best-effort medium access protocol. Because DCF treats all data packets equally regardless of their content or timing constraints, real-time applications such as Voice over IP (VoIP) and interactive video streaming must contend for channel access on equal terms with low-priority background data transfers.

### 2. Importance
As Wi-Fi networks became primary access points for multimedia applications, providing Quality of Service (QoS) guarantees became essential. Without MAC-layer prioritization, heavy background data transfers saturate the shared medium, introducing unpredictable delays, high packet loss, and severe jitter that render voice and video communications unusable. IEEE 802.11e introduced Enhanced Distributed Channel Access (EDCA) to solve this by creating prioritized Access Categories ($AC\_VO$, $AC\_VI$, $AC\_BE$, $AC\_BK$).
