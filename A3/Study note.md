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
