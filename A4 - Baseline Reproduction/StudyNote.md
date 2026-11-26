# Study Note: Analysis of the Influence of PHY Data Rate Variations on QoS in IEEE 802.11e-Based WLAN Networks

> NS-3 experiment documentation for studying the effect of changes in **PHY Data Rate** on **Throughput, Delay, Jitter, and Packet Loss** in an IEEE 802.11e-based WLAN.

## Table of Contents

- [1. Objective](#1-objective)
- [2. Basic Concepts](#2-basic-concepts)
- [3. Network Topology and Scenario](#3-network-topology-and-scenario)
- [4. Simulation Parameters](#4-simulation-parameters)
- [5. PHY Data Rate Variations](#5-phy-data-rate-variations)
- [6. QoS Measurement Methodology](#6-qos-measurement-methodology)
- [7. Results and Analysis](#7-results-and-analysis)
- [8. Creating the Graphs with Python](#8-creating-the-graphs-with-python)
- [9. Result Validity Checks](#9-result-validity-checks)
- [10. Conclusion](#10-conclusion)

---

## 1. Objective

This experiment aims to analyze how changes in PHY Data Rate affect network Quality of Service (QoS) in an IEEE 802.11e WLAN.

The four observed QoS parameters are:

1. **Throughput** — the amount of data successfully received per unit of time.
2. **Delay** — the time required for a packet to travel from the sender to the receiver.
3. **Jitter** — the variation in delay between consecutively received packets.
4. **Packet Loss** — the percentage of transmitted packets that are not received.

The independent variable is PHY Data Rate. The QoS parameters are the dependent variables. Traffic type, packet size, distance, simulation duration, and network configuration should be kept constant so that comparisons between scenarios remain meaningful.

## 2. Basic Concepts

### 2.1 PHY Data Rate

PHY Data Rate is the bit transmission rate at the Physical Layer. This experiment uses four values:

- 6 Mbps
- 12 Mbps
- 24 Mbps
- 54 Mbps

PHY Data Rate is not the same as application throughput. Actual throughput is usually lower because transmission also requires time for headers, preambles, ACKs, contention, inter-frame spacing, retransmissions, and other MAC mechanisms.

### 2.2 IEEE 802.11e and EDCA

IEEE 802.11e introduces QoS mechanisms for WLANs. One of its medium-access mechanisms is **Enhanced Distributed Channel Access (EDCA)**.

EDCA divides traffic into four Access Categories (ACs):

| Access Category | Relative Priority | Common Use |
|---|---:|---|
| AC_VO | Highest | Voice |
| AC_VI | High | Video |
| AC_BE | Best effort | General data traffic |
| AC_BK | Lowest | Background |

EDCA assigns different channel-access parameters to each category. Higher-priority categories generally receive better channel-access opportunities, but this does not mean their packets are always free from delay or loss.

### 2.3 Relationship Between PHY Data Rate and QoS

In general, increasing the PHY Data Rate can reduce the transmission time of a frame. If other factors remain constant, the amount of time the channel is occupied to transmit a payload can become shorter.

However, the final QoS results are also affected by:

- total traffic load;
- the number of STAs contending for the channel;
- EDCA priority;
- packet size and arrival pattern;
- retransmissions and channel conditions;
- queue capacity and drop policy;
- rate-control and basic/control-rate configuration.

Therefore, experimental results do not necessarily form perfectly linear curves.

## 3. Network Topology and Scenario

The topology is an infrastructure WLAN consisting of one Access Point and four Stations.

| Device | Traffic Type in This Experiment | Access Category |
|---|---|---|
| AP | Access Point | Serves four STAs |
| STA 1 | Voice / VoIP | AC_VO |
| STA 2 | Video | AC_VI |
| STA 3 | Web/data | AC_BE |
| STA 4 | Web/data | AC_BK |

All STAs communicate through the AP. The AP-to-STA distance in the described configuration is 5 meters.

> Note: The “Web/data” label in the graphs refers to the name assigned to the experimental traffic. If the traffic is generated using constant-rate UDP, it represents a simplified data-load model rather than a complete simulation of real web browsing behavior based on TCP, request-response exchanges, and bursty traffic.

### Conceptual Diagram

```text
                 +------------------+
                 |   Access Point    |
                 |   IEEE 802.11e    |
                 |       EDCA        |
                 +--------+---------+
                          |
             Wireless infrastructure WLAN
          +---------------+---------------+
          |               |               |
       +--+--+         +--+--+         +--+--+         +--+--+
       |STA 1|         |STA 2|         |STA 3|         |STA 4|
       | VoIP|         |Video|         | Web |         | Web |
       | VO  |         | VI  |         | BE  |         | BK  |
       +-----+         +-----+         +-----+         +-----+
```

## 4. Simulation Parameters

The following parameters are used in the experimental design:

| Parameter | Value |
|---|---|
| Simulator | NS-3 |
| WLAN standard | IEEE 802.11g |
| QoS MAC | IEEE 802.11e / EDCA |
| Network mode | Infrastructure |
| Number of APs | 1 |
| Number of STAs | 4 |
| Channel width | 20 MHz |
| AP-to-STA distance | 5 meters |
| PHY Data Rate | 6, 12, 24, 54 Mbps |
| Packet size | 512 bytes |
| Simulation time | 100 seconds |
| Traffic start time | 5 seconds |
| Traffic stop time | 95 seconds |
| Traffic observation duration | 90 seconds |
| Base random seed | 12345 |
| Planned number of runs | 5 |

### Traffic Configuration

| STA | Traffic Type | Offered Rate | Packet Size |
|---|---|---:|---:|
| STA 1 | VoIP | 0.5 Mbps | 512 bytes |
| STA 2 | Video | 2 Mbps | 512 bytes |
| STA 3 | Web/data | 3 Mbps | 512 bytes |
| STA 4 | Web/data | 1 Mbps | 512 bytes |
| **Total** | | **6.5 Mbps** | |

The total offered traffic is 6.5 Mbps. This value is important when interpreting the 6 Mbps PHY scenario: the total offered load is greater than the nominal PHY Data Rate. This comparison can reveal queueing and packet-drop effects when the offered load is high.

> The parameter values in this section describe the experimental configuration associated with the graphs. For scientific reproducibility, verify every value against the actual NS-3 C++ source code and configuration files used to generate the CSV data.

## 5. PHY Data Rate Variations

Four scenarios are compared:

| Scenario | PHY Data Rate | Wi-Fi Data Mode |
|---|---:|---|
| 1 | 6 Mbps | `ErpOfdmRate6Mbps` |
| 2 | 12 Mbps | `ErpOfdmRate12Mbps` |
| 3 | 24 Mbps | `ErpOfdmRate24Mbps` |
| 4 | 54 Mbps | `ErpOfdmRate54Mbps` |

When using `ConstantRateWifiManager`, the data mode can be explicitly configured. The control mode should remain the same across all scenarios if the purpose of the experiment is to isolate the effect of changing the data mode.

Conceptual configuration example:

```cpp
wifi.SetRemoteStationManager(
    "ns3::ConstantRateWifiManager",
    "DataMode", StringValue(dataMode),
    "ControlMode", StringValue("ErpOfdmRate6Mbps")
);
```

The available mode names depend on the Wi-Fi standard and PHY configuration. Make sure the selected mode is supported by the devices and the NS-3 Wi-Fi configuration being used.

## 6. QoS Measurement Methodology

FlowMonitor can be used to collect flow statistics, such as the number of transmitted and received packets/bytes, delay sum, and jitter sum.

### 6.1 Throughput

Throughput is calculated from the number of bytes received during the observation interval:

\[
Throughput\ (bps)=\frac{ReceivedBytes \times 8}{T}
\]

For a 90-second interval:

\[
Throughput\ (Mbps)=
\frac{ReceivedBytes \times 8}{90 \times 10^6}
\]

Where:

- `ReceivedBytes`: the number of bytes received;
- `T`: the measurement interval in seconds;
- the factor 8 converts bytes to bits.

### 6.2 Average End-to-End Delay

\[
AverageDelay(s)=\frac{DelaySum(s)}{RxPackets}
\]

To convert seconds to milliseconds:

\[
AverageDelay(ms)=
\frac{DelaySum(s)}{RxPackets}\times1000
\]

Average delay can only be calculated when `RxPackets > 0`.

### 6.3 Average Jitter

Using the FlowMonitor jitter-sum definition as the accumulated difference in delay between consecutive packets:

\[
AverageJitter(s)=
\frac{JitterSum(s)}{RxPackets-1}
\]

\[
AverageJitter(ms)=
\frac{JitterSum(s)}{RxPackets-1}\times1000
\]

At least two received packets are required for this calculation. Make sure the same definition is used across all scenarios and is appropriate for the NS-3 version being used.

### 6.4 Packet Loss

\[
PacketLoss(\%)=
\frac{TxPackets-RxPackets}{TxPackets}\times100
\]

Where:

- `TxPackets`: the number of packets transmitted in the flow;
- `RxPackets`: the number of packets received in the flow.

If `TxPackets = 0`, packet loss must not be calculated by dividing by zero. In addition, the difference between Tx and Rx in FlowMonitor must be interpreted according to the measurement scope and flow behavior.

## 7. Results and Analysis

The following four graphs are the images uploaded for this documentation. Values mentioned in the discussion are approximate readings from the plotted points; use the original CSV data for precise values.

### 7.1 Throughput versus PHY Data Rate

<img width="2400" height="1500" alt="QoS_Throughput" src="https://github.com/user-attachments/assets/a7a11392-d55c-4dfa-ae41-7df532e2f4f3" />


**X-axis:** PHY Data Rate (Mbps)  
**Y-axis:** Throughput (Mbps)

Approximate values read from the graph:

| STA | 6 Mbps | 12 Mbps | 24 Mbps | 54 Mbps |
|---|---:|---:|---:|---:|
| STA 1 – VoIP | 0.53 | 0.53 | 0.53 | 0.53 |
| STA 2 – Video | 2.10 | 2.10 | 2.10 | 2.10 |
| STA 3 – Web_STA3 | 0.88 | 2.56 | 3.16 | 3.16 |
| STA 4 – Web_STA4 | 0.22 | 0.80 | 1.05 | 1.05 |

**Interpretation:**

1. STA 1 throughput appears relatively constant at approximately 0.53 Mbps across all PHY Data Rates.
2. STA 2 throughput appears relatively constant at approximately 2.10 Mbps.
3. STA 3 throughput increases as the PHY Data Rate changes from 6 to 24 Mbps, then appears stable at 54 Mbps.
4. STA 4 throughput also increases from 6 to 24 Mbps, then remains relatively stable at 54 Mbps.
5. This increase followed by a plateau may indicate that, at higher rates, the flow has reached a limit determined by the offered traffic or by factors other than PHY Data Rate.

**Why does throughput not always increase in proportion to PHY Data Rate?**

PHY Data Rate is the transmission speed at the physical layer, whereas throughput is the application data successfully received. Once the channel capacity is sufficient to serve the traffic load, increasing the PHY Data Rate does not necessarily increase application throughput.

### 7.2 Delay versus PHY Data Rate

<img width="2400" height="1500" alt="QoS_Delay" src="https://github.com/user-attachments/assets/c370bd6d-5177-4757-afdf-0b5357f09ee9" />

**X-axis:** PHY Data Rate (Mbps)  
**Y-axis:** Average end-to-end delay (ms)

The following patterns can be observed in the graph:

- At 6 Mbps, STA 3 and STA 4 show delays close to 500 ms.
- At 12 Mbps, both STAs still show high delays of approximately 490 ms.
- At 24 and 54 Mbps, the delay values for STA 3 and STA 4 appear very close to zero on the graph scale.
- The STA 1 and STA 2 curves are near the bottom of the graph compared with the delay values of STA 3 and STA 4 at 6 and 12 Mbps.

**Interpretation:**

The high delays at 6 and 12 Mbps may be related to queue waiting time when the traffic load approaches or exceeds the effective channel service capacity. When the PHY Data Rate increases, frame transmission time can decrease, potentially allowing queues to be drained more quickly.

**Important warning:** The change from approximately 490–500 ms to almost 0 ms is very sharp. Do not immediately conclude that the delay is actually zero. Check the raw `delaySum` and `rxPackets` values, time units, flow filters, and the way the program generates the graph. A zero value may also result from invalid data, rounding, or selecting the wrong flow.

### 7.3 Jitter versus PHY Data Rate

![Jitter versus PHY Data Rate](images/QoS_Jitter.png)

**X-axis:** PHY Data Rate (Mbps)  
**Y-axis:** Average jitter (ms)

Approximate values read from the graph:

| STA | 6 Mbps | 12 Mbps | 24 Mbps | 54 Mbps |
|---|---:|---:|---:|---:|
| STA 1 – VoIP | 1.25 | 0.59 | 0.34 | 0.27 |
| STA 2 – Video | 0.98 | 0.60 | 0.53 | 0.44 |
| STA 3 – Web_STA3 | 0.50 | 0.62 | 0.31 | 0.14 |
| STA 4 – Web_STA4 | 1.37 | 1.86 | 1.55 | 0.85 |

**Interpretation:**

1. STA 1 jitter decreases as the PHY Data Rate increases across the plotted points.
2. STA 2 jitter also shows a decreasing trend.
3. STA 3 jitter increases slightly at 12 Mbps compared with 6 Mbps, then decreases at 24 and 54 Mbps.
4. STA 4 has the highest jitter among the four STAs in the graph. Its value peaks at approximately 1.86 ms at 12 Mbps, then decreases.
5. At 54 Mbps, the jitter of all STAs appears lower than their initial values at 6 Mbps.

Jitter does not always decrease monotonically because it is affected by variations in channel-access time, queueing, contention among STAs, and packet arrival patterns. Low jitter means packet arrival times are more consistent; it does not automatically mean that throughput is the highest.

### 7.4 Packet Loss versus PHY Data Rate

![Packet Loss versus PHY Data Rate](images/QoS_Packet_Loss.png)

**X-axis:** PHY Data Rate (Mbps)  
**Y-axis:** Packet loss (%)

Approximate values read from the graph:

| STA | 6 Mbps | 12 Mbps | 24 Mbps | 54 Mbps |
|---|---:|---:|---:|---:|
| STA 1 – VoIP | 0 | 0 | 0 | 0 |
| STA 2 – Video | 0 | 0 | 0 | 0 |
| STA 3 – Web_STA3 | 72.5 | 19 | 0 | 0 |
| STA 4 – Web_STA4 | 79.5 | 24 | 0 | 0 |

**Interpretation:**

1. STA 1 and STA 2 appear to experience no packet loss in any of the displayed scenarios.
2. STA 3 experiences high packet loss at 6 Mbps, which then decreases at 12 Mbps.
3. STA 4 shows the highest packet loss at 6 Mbps, followed by a decrease at 12 Mbps.
4. At 24 and 54 Mbps, packet loss for STA 3 and STA 4 appears to be 0%.
5. These results are consistent with the hypothesis that the service capacity at lower PHY Data Rates is insufficient to handle the entire offered traffic load.

A packet loss value of 0% in the graph means that no lost packets are visible in the plotted data. Verify that this value comes from correct packet counting and is not caused by an undetected flow or an empty CSV data set.

## 8. Creating the Graphs with Python

The graphs can be generated using Python and Matplotlib. If simulation-result CSV files are available, use the CSV values as the data source rather than manually re-entering approximate values read from the images.

### 8.1 Installation

```bash
python3 -m pip install pandas matplotlib
```

### 8.2 Example Code for Generating Four Graphs from the Results

The following example assumes that a summary CSV has already been prepared with these columns:

- `phy_rate`
- `sta`
- `throughput_mbps`
- `delay_ms`
- `jitter_ms`
- `packet_loss_pct`

Example CSV structure:

```csv
phy_rate,sta,throughput_mbps,delay_ms,jitter_ms,packet_loss_pct
6,STA 1 - VoIP,0.53,1.0,1.25,0
12,STA 1 - VoIP,0.53,1.0,0.59,0
24,STA 1 - VoIP,0.53,1.0,0.34,0
54,STA 1 - VoIP,0.53,1.0,0.27,0
```

Plotting code:

```python
import pandas as pd
import matplotlib.pyplot as plt

# Read the simulation summary data
df = pd.read_csv("qos_summary.csv")

# STA order in the legend
sta_order = [
    "STA 1 - VoIP",
    "STA 2 - Video",
    "STA 3 - Web_STA3",
    "STA 4 - Web_STA4"
]

# Generate a separate graph for each parameter
plots = [
    ("throughput_mbps", "Throughput versus PHY Data Rate",
     "Throughput (Mbps)", "QoS_Throughput.png"),
    ("delay_ms", "Delay versus PHY Data Rate",
     "Average end-to-end delay (ms)", "QoS_Delay.png"),
    ("jitter_ms", "Jitter versus PHY Data Rate",
     "Average jitter (ms)", "QoS_Jitter.png"),
    ("packet_loss_pct", "Packet Loss versus PHY Data Rate",
     "Packet loss (%)", "QoS_Packet_Loss.png")
]

for column, title, ylabel, filename in plots:
    fig, ax = plt.subplots(figsize=(12, 7))

    for sta in sta_order:
        subset = df[df["sta"] == sta].sort_values("phy_rate")

        ax.plot(
            subset["phy_rate"],
            subset[column],
            marker="o",
            linewidth=2.5,
            markersize=8,
            label=sta
        )

    ax.set_title(title, fontsize=18)
    ax.set_xlabel("PHY Data Rate (Mbps)", fontsize=14)
    ax.set_ylabel(ylabel, fontsize=14)
    ax.set_xticks([6, 12, 24, 54])
    ax.grid(True, linestyle="--", alpha=0.5)
    ax.legend(fontsize=12)
    fig.tight_layout()
    fig.savefig(filename, dpi=200)
    plt.close(fig)

print("All four QoS graphs have been generated successfully.")
```

> The code above generates graphs from a summary CSV. If your NS-3 output is still in FlowMonitor XML format, first extract and calculate the metrics for each flow, then create a CSV with the same column names. Make sure each row represents one STA, one PHY Data Rate, and one simulation run or clearly defined aggregation result.

### 8.3 Averaging Multiple Runs

If each PHY Data Rate is simulated five times, calculate the average for each STA and PHY Data Rate. Example:

```python
summary = (
    df.groupby(["phy_rate", "sta"], as_index=False)
      .agg({
          "throughput_mbps": "mean",
          "delay_ms": "mean",
          "jitter_ms": "mean",
          "packet_loss_pct": "mean"
      })
)

summary.to_csv("qos_summary.csv", index=False)
```

For stronger reporting, also include the standard deviation or confidence interval. Do not combine run records without ensuring that every run uses the same measurement definition and duration.

## 9. Result Validity Checks

Before using the graphs as final research results, perform the following checks.

### 9.1 Check the Units

- `delaySum` and `jitterSum` in FlowMonitor use time units that must be converted correctly.
- If the values are in seconds, multiply by 1000 to obtain milliseconds.
- Throughput must be expressed consistently in bits per second or Mbps.
- Packet loss must be displayed as a percentage.

### 9.2 Check Which Flows Are Included

Make sure each STA is mapped to the correct flow. Check the source/destination IP addresses, UDP ports, protocol, and traffic direction. Do not accidentally mix statistics from AP flows, STA flows, or reverse-direction traffic.

### 9.3 Check Zero Values

If `RxPackets = 0`, delay and jitter cannot be calculated. Display them as `NaN`/blank or mark them as unavailable rather than automatically assigning 0 ms.

If `RxPackets = 1`, average jitter based on differences between consecutive packets cannot be calculated either.

If packet loss is 0%, verify that `TxPackets` and `RxPackets` are actually present and that their values are consistent.

### 9.4 Check the Delay Spike

A delay of approximately 500 ms at 6 and 12 Mbps, followed by a value close to zero at 24 and 54 Mbps, is a very large change. Check the following:

1. Are queues becoming full, and are packets being dropped?
2. Does the measurement count only successfully received packets?
3. Is the measurement interval consistent?
4. Is `delaySum` divided by `rxPackets` from the same flow?
5. Is the conversion from seconds to milliseconds correct?
6. Are very small graph values visually compressed by the Y-axis scale?

### 9.5 Check the PHY Configuration

Make sure that only the variable intended for investigation is changed. For example, when the data mode changes, the control mode, channel width, distance, transmit power, traffic pattern, packet size, and simulation duration should remain constant.

### 9.6 Reproducibility

Save the following information for every experiment:

- NS-3 version;
- C++ source code;
- command line;
- random seed and run number;
- PHY/MAC configuration;
- CSV/XML files;
- plotting script;
- generated graphs.

This makes the experiment repeatable and verifiable.

## 10. Conclusion

Based on the patterns visible in the four graphs:

1. **Throughput:** STA 3 and STA 4 throughput increases as the PHY Data Rate rises from 6 to 24 Mbps, then tends to stabilize at 54 Mbps. STA 1 and STA 2 remain relatively constant.
2. **Delay:** STA 3 and STA 4 show very high delay at 6 and 12 Mbps, followed by values close to zero at 24 and 54 Mbps. This extreme change must be verified against the raw data before drawing a final conclusion.
3. **Jitter:** jitter generally decreases at higher PHY Data Rates, although STA 3 and STA 4 show increases at certain points.
4. **Packet Loss:** packet loss for STA 3 and STA 4 is high at 6 Mbps, decreases at 12 Mbps, and appears to be zero at 24 and 54 Mbps. STA 1 and STA 2 appear to have zero packet loss in all scenarios.

Overall, the graphs indicate that increasing the PHY Data Rate helps the network handle the offered traffic, particularly for STA 3 and STA 4. However, quantitative conclusions must be based on the original CSV/XML files, validation of the FlowMonitor calculations, and results from multiple simulation runs.

---

## Suggested GitHub Repository Structure

```text
WLAN-QoS-NS3/
│
├── StudyNote.md
│
├── images/
│   ├── QoS_Throughput.png
│   ├── QoS_Delay.png
│   ├── QoS_Jitter.png
│   └── QoS_Packet_Loss.png
│
├── scratch/
│   └── qos-phy-rate-80211e.cc
│
├── results/
│   ├── csv/
│   └── flowmon/
│
└── scripts/
    └── plot-qos-results.py
```

### How to Upload the Project to GitHub

1. Create a new repository, for example, `WLAN-QoS-NS3`.
2. Upload `StudyNote.md` to the repository root.
3. Upload the `images` folder containing the four graphs.
4. Add the NS-3 source code and Python script if you want the repository to support reproducing the experiment.
5. Open `StudyNote.md` on GitHub. The graphs will be displayed automatically because the Markdown uses relative paths such as `images/...`.

**Note:** The numerical values in the analysis tables are approximate readings from the graphs. For publication or a final report, replace these values with results calculated from the original CSV/XML files.
