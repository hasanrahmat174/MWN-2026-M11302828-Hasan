**Course:** Multimedia Wireless Networks (ET5907701), Fall 2026 · **Student:** Rahmat Munir Hasan (M11302828) · **Path:** Advanced · **Presentation:** Week 7, 10 minutes

> [!NOTE]
> **Rahmat Munir Hasan**  
> M11302828

## Table of Contents

- [1. Setup](#1-setup)
  - [1.1 Platform and Software Versions](#11-platform-and-software-versions)
  - [1.2 Installation Steps](#12-installation-steps)
    - [1.2.1 Installing NS-3](#121-installing-ns-3)
    - [1.2.2 Installing Wireshark](#122-installing-wireshark)
    - [1.2.3 Running NS-3 to Generate PDF and CDF Graphs](#123-running-ns-3-to-generate-pdf-and-cdf-graphs)
  - [1.3 Troubleshooting and Solutions](#13-troubleshooting-and-solutions)
    - [1.3.1 Resolving the `EventId::IsRunning()` Compilation Error](#131-resolving-the-eventidisrunning-compilation-error)

- [2. Traffic Model](#2-traffic-model)

- [3. Running the Model](#3-running-the-model)
  - [3.1 Running the Gaussian Traffic Generator](#31-running-the-gaussian-traffic-generator)
  - [3.2 Generating PCAP Files](#32-generating-pcap-files)
  - [3.3 Analyzing Network Traffic Using Wireshark](#33-analyzing-network-traffic-using-wireshark)
  - [3.4 Generating PDF and CDF Graphs Using Python](#34-generating-pdf-and-cdf-graphs-using-python)
    - [3.4.1 PDF of Inter-Arrival Times](#341-pdf-of-inter-arrival-times)
    - [3.4.2 CDF of Inter-Arrival Times](#342-cdf-of-inter-arrival-times)
    - [3.4.3 PDF of Packet Sizes](#343-pdf-of-packet-sizes)
    - [3.4.4 CDF of Packet Sizes](#344-cdf-of-packet-sizes)

- [4. Analysis](#4-analysis)
  - [4.1 Comprehensive Analysis of Gaussian Traffic Simulation](#41-comprehensive-analysis-of-gaussian-traffic-simulation)
  - [4.2 Analysis of the Inter-Arrival Time CDF](#42-analysis-of-the-inter-arrival-time-cdf)
    - [4.2.1 Interpretation of the Axes](#421-interpretation-of-the-axes)
    - [4.2.2 Observed Results](#422-observed-results)
    - [4.2.3 Statistical Interpretation](#423-statistical-interpretation)
    - [4.2.4 Simulation Validation](#424-simulation-validation)
  - [4.3 Analysis of the Inter-Arrival Time PDF](#43-analysis-of-the-inter-arrival-time-pdf)
    - [4.3.1 Interpretation of the Axes](#431-interpretation-of-the-axes)
    - [4.3.2 Observed Results](#432-observed-results)
    - [4.3.3 Comparison with the Truncated Gaussian Distribution](#433-comparison-with-the-truncated-gaussian-distribution)
    - [4.3.4 Histogram Validation](#434-histogram-validation)
  - [4.4 Analysis of the Packet Size CDF](#44-analysis-of-the-packet-size-cdf)
    - [4.4.1 Interpretation of the Axes](#441-interpretation-of-the-axes)
    - [4.4.2 Observed Results](#442-observed-results)
    - [4.4.3 Calculation of Packet Size](#443-calculation-of-packet-size)
    - [4.4.4 Interpretation of the Results](#444-interpretation-of-the-results)
  - [4.5 Analysis of the Packet Size PDF](#45-analysis-of-the-packet-size-pdf)
    - [4.5.1 Interpretation of the Axes](#451-interpretation-of-the-axes)
    - [4.5.2 Observed Results](#452-observed-results)
    - [4.5.3 Probability Mass Distribution](#453-probability-mass-distribution)
  - [4.6 Overall Results and Discussion](#46-overall-results-and-discussion)
  - [4.7 Recommendations for Further Statistical Validation](#47-recommendations-for-further-statistical-validation)

- [5. References](#5-references)
  
## 1. Setup

### 1.1 Platform and versions

I use an Asus TUF laptop, my operating system is Ubuntu 26. I use NS-3 with version 3.8, Wireshark version 4.6.9, Python version 3.15.0, C++ version 14

### 1.2 Install steps
1.2.1 for the first, we need to install ns-3install ns-3 step by step : 

1. Download ns-3 with git clone 
```bash
git clone https://gitlab.com/nsnam/ns-3-dev.git
```
2. If you are content to work with the tip of the development tree; you need only to cd into ns-3-dev; the master branch is checked out by default.
```bash
cd ns-3-dev
```
3. If instead you want to try the most recent release (version 3.45 as of this writing), you can checkout a branch corresponding to that git tag:
```bash
git checkout -b ns-3.45-branch ns-3.45
```
4. Building and testing
Once you have obtained the source either by downloading a release or by cloning a Git repository, the next step is to configure the build using the CMake build system. The below commands make use of a Python wrapper around CMake, called ns3, that simplifies the command-line syntax, resembling Waf syntax. There are several options to control the build, but enabling the example programs and the tests, for a default build profile (with asserts enabled and and support for ns-3 logging) is what is usually done at first:
```bash
./ns3 configure --enable-examples --enable-tests
```
5. Then, use ns3 to build ns-3:
```bash
./ns3 build
```
6. last, you can try by your own self to run ns3 

1.2.2 for the second, install wireshark : 
1. completely installation 
```bash
sudo apt update
sudo apt install wireshark
sudo usermod -aG wireshark $USER
```
1.2.3 for the third run ns-3 to generate pdf and cdf 

1. Run traffic-gaussian  
```bash
./ns3 run "scratch/traffic-gaussian \
  --packets=1000 \
  --payloadBytes=512 \
  --meanIat=0.030 \
  --stdIat=0.010 \
  --minIat=0.005 \
  --maxIat=0.080 \
  --simStop=45 \
  --prefix=traffic"
```

2. Run wireshark simulation  
```bash
wireshark traffic-0-0.pcap
```

3. generated pdf and cdf vs inter-arival time
```bash
python3 analysis/analyze_interarrival.py \
  --pcap traffic-0-0.pcap \
  --out results
```
4. generated pdf and cdf vs packet size 
```bash
python3 analysis/packet_size_density_pdf_cdf.py \
  --pcap traffic-0-0.pcap \
  --out results2 \
  --kde
```

### 1.3 What broke
1.3.1 This error occurs due to differences in the EventId API in the version of ns-3 you are using. In your version of ns-3, the EventId::IsRunning() method is not available. The compiler also hints that the available method is IsPending().

  > `/home/rahmat/ns-3/scratch/traffic-gaussian.cc: In member function ‘virtual void GaussianUdpApp::StopApplication()’:
/home/rahmat/ns-3/scratch/traffic-gaussian.cc:49:17: error: ‘class ns3::EventId’ has no member named ‘IsRunning’; did you mean ‘IsPending’?
   49 |     if (m_event.IsRunning()) Simulator::Cancel(m_event);
      |                 ^~~~~~~~~
      |                 IsPending
gmake[3]: *** [scratch/CMakeFiles/scratch_traffic-gaussian.dir/build.make:81: scratch/CMakeFiles/scratch_traffic-gaussian.dir/traffic-gaussian.cc.o] Error 1
gmake[2]: *** [CMakeFiles/Makefile2:4692: scratch/CMakeFiles/scratch_traffic-gaussian.dir/all] Error 2
gmake[1]: *** [CMakeFiles/Makefile2:4698: scratch/CMakeFiles/scratch_traffic-gaussian.dir/rule] Error 2
gmake: *** [Makefile:1470: scratch_traffic-gaussian] Error 2`
> 

Solved : 
first, Open file traffic-gaussian and Look for the StopApplication() function which contains the following code:
```bash
void StopApplication() override
{
    if (m_event.IsRunning())
        Simulator::Cancel(m_event);

    if (m_socket)
        m_socket->Close();
}
```
second change the code before to this Change IsRunning() to IsPending(), so the code becomes :
```bash
void StopApplication() override
{
    if (m_event.IsPending())
        Simulator::Cancel(m_event);

    if (m_socket)
        m_socket->Close();
}
```

## 2. Traffic Model 
<img width="786" height="381" alt="image" src="https://github.com/user-attachments/assets/7c2295ac-ebcc-4dbf-bc14-087a15765361" />

<img width="805" height="412" alt="image" src="https://github.com/user-attachments/assets/054a3e33-f678-4687-83b0-bf10b46cdeb7" />




### 3. Running the model

after run traffic-gaussian. there is traffic-0-0.pcap and traffic-1-0.pcap .pcap for analyze with wireshark(open): 

<img width="970" height="758" alt="Task 1_Wireshark simulation" src="https://github.com/user-attachments/assets/73d9ec7e-1541-42ae-ab7d-ddd1f43c1763" />

to get graph pdf and cdf versus packet size and inter-arival time, it can generated from Task 1_analyze_interarrival.py and Task 1_packet_size_density_pdf_cdf.py with traffic-gaussian, the result : 

1. PDF with inter-arival time

<img width="1024" height="768" alt="Task 1_inter_arrival_pdf" src="https://github.com/user-attachments/assets/effe7b37-203a-4442-a9f4-bdf0f7b147b6" />

2. CDF with inter-arival time

   <img width="1024" height="768" alt="Task 1_inter_arrival_cdf" src="https://github.com/user-attachments/assets/f1674953-dedc-4860-99f3-65f2412d557d" />

3. PDF with Packet size

   <img width="1280" height="800" alt="Task 1_packet_size_pdf" src="https://github.com/user-attachments/assets/31a4b089-f1a8-4fce-a50b-1bcae4ec3c20" />
 
4. CDF with Packet size

<img width="1280" height="800" alt="Task 1_packet_size_cdf" src="https://github.com/user-attachments/assets/a85180cf-fa9e-4190-9749-414dfb1cbba6" />

### 4. Analysis 

Comprehensive Analysis of Four Graphs from the Gaussian Traffic Simulation in ns-3
Based on the four uploaded graphs, the simulation demonstrates two different traffic characteristics:
 
 - Inter-arrival time (IAT): Varies and exhibits a pattern resembling a Gaussian distribution.
 - Packet size: Remains constant at 540 bytes, resulting in a distribution concentrated at a single value.

Overall, these graphs indicate that the traffic generator successfully produces variations in packet arrival times while maintaining a fixed packet size. However, statistical conformity to a truncated Gaussian distribution and validity as a 3GPP traffic model still require additional verification.

1. Analysis of the Inter-arrival Time CDF
Graph 1 — Inter-arrival Time CDF (PCAP)

The first graph presents the Empirical Cumulative Distribution Function (ECDF) of inter-arrival times obtained from the PCAP data.

A. Interpretation of the axes
- X-axis: Inter-arrival time in milliseconds (ms).
- Y-axis: Cumulative probability, ranging from 0 to 1.
The CDF represents the probability that the inter-arrival time is less than or equal to a particular value.

<img width="191" height="56" alt="image" src="https://github.com/user-attachments/assets/c2e4cbbd-d3f6-424c-a8f6-8c69ee9ad973" />

B. Observed results
Based on the curve, the following values can be estimated:
Parameter	                                  Estimated value
Minimum observed                              IAT	5–6 ms
Median IAT	                                  Approximately 30 ms
25th percentile (P25)	                      Approximately 22–23 ms
75th percentile (P75)	                      Approximately 36–37 ms
90th percentile (P90)	                      Approximately 43 ms
95th percentile (P95)	                      Approximately 47 ms
Maximum observed IAT	                         Approximately 63 ms

These values are estimated visually from the graph rather than calculated directly from the CSV data.

C. Statistical interpretation
The CDF exhibits an S-shaped curve, with its steepest increase occurring approximately between 25 and 35 ms.
This indicates that:

- Most packets have inter-arrival times close to 30 ms.
- Approximately 50% of the inter-arrival times are less than or equal to 30 ms.
- Approximately 90% of the inter-arrival times are less than or equal to 43 ms.
- Only a small proportion of intervals exceed 50 ms.
This shape is consistent with a probability distribution concentrated around its central value.

D. Was the simulation successful?
Yes, from a visual perspective, the graph demonstrates that the inter-arrival times vary and that the cumulative distribution has been generated correctly.

However, the S-shaped curve alone does not prove that the data follow a truncated Gaussian distribution. Additional statistical validation is required.

2. Analysis of the Inter-arrival Time PDF
Graph 2 — Inter-arrival Time PDF (PCAP)

The second graph presents a probability density histogram of the inter-arrival times.

A. Interpretation of the axes
 - X-axis: Inter-arrival time (ms).
 - Y-axis: Probability density (1/ms).
The values on the Y-axis represent probability density rather than direct probability.

The probability of an inter-arrival time falling within a particular interval is determined by the area under the probability density function.

<img width="265" height="75" alt="image" src="https://github.com/user-attachments/assets/7a24c03e-fe56-42bb-ba26-e0da622b2bed" />

B. Observed results
Parameter	                                 Estimated value
Distribution peak (mode)      	          Approximately 30–32 ms
Maximum probability density	             Approximately 0.043 per ms
Observed IAT range	                      Approximately 5–63 ms
High-density interval	                   Approximately 20–40 ms
Distribution shape	                      Unimodal, approximately bell-shaped

The histogram shows a single dominant peak near 30 ms, followed by decreasing probability density toward shorter and longer inter-arrival times.

C. Relationship with the truncated Gaussian distribution
The previous traffic generator configuration used the following parameters:

<img width="312" height="112" alt="image" src="https://github.com/user-attachments/assets/c49d0ad9-360f-4717-bf09-158bda8d27d2" />

The graph shows a peak near 30 ms, which is consistent with the expected center of the distribution.

The observed inter-arrival times extend only to approximately 63 ms, even though the configured maximum is 80 ms.

This does not necessarily indicate an error. The truncation boundaries define the permitted range of values, but a finite simulation does not have to produce samples exactly at those boundaries.

D. Is the histogram correct?
Visually, the histogram exhibits a reasonable distribution for Gaussian-based inter-arrival times.

However, three additional validation steps are recommended for research purposes:
 1. Compare the empirical histogram with the theoretical truncated Gaussian PDF.
 2. Calculate the empirical mean and standard deviation directly from the CSV data.
 3. Compare the empirical CDF with the theoretical CDF, for example, using the Kolmogorov–Smirnov statistic.
These procedures would support the conclusion that the traffic generator follows the intended truncated Gaussian distribution, rather than relying solely on the visual appearance of the histogram.

3. Analysis of the Packet Size CDF
Graph 3 — Empirical CDF of Packet Sizes

The third graph differs significantly from the inter-arrival time graphs.
The packet size CDF consists of a single vertical line at 540 bytes.

A. Interpretation of the axes
- X-axis: Packet size (bytes).
- Y-axis: Cumulative probability.
  
B. Observed results
The graph indicates that:

<img width="142" height="47" alt="image" src="https://github.com/user-attachments/assets/75f1586c-fc13-4463-b99e-42582f42c267" />

This means that 100% of the analyzed packets have a size of 540 bytes.
The empirical CDF can therefore be expressed as:

<img width="217" height="72" alt="5" src="https://github.com/user-attachments/assets/310d7c3c-92ea-4c8c-b089-a3e982fe171f" />

In other words:
 - For packet sizes smaller than 540 bytes, the cumulative probability is 0.
 - At 540 bytes, the cumulative probability immediately increases to 1.
 - For packet sizes greater than 540 bytes, the cumulative probability remains 1.

C. Why is the packet size 540 bytes?

Based on the previous simulation configuration:
Component	                        Size
UDP payload	                     512 bytes
UDP header	                       8 bytes
IPv4 header	                      20 bytes
Total IPv4 packet size	         540 bytes

The packet size is calculated as:

<img width="300" height="42" alt="image" src="https://github.com/user-attachments/assets/d4235761-f6d0-48d9-9a87-09268739471c" />

This calculation matches the packet size shown in the graph, assuming a standard IPv4 header without additional options.

D. Interpretation of the results
The packet size CDF is mathematically correct if every analyzed packet has the same size.

However, the graph does not demonstrate packet size variability.

Therefore, the result is insufficient for evaluating a variable packet size distribution, such as those used in certain XR traffic models.


4. Analysis of the Packet Size PDF
Graph 4 — Packet Size Distribution (Single Size)
The fourth graph illustrates the packet size distribution using a single vertical line at 540 bytes.

A. Interpretation of the axes
  - X-axis: Packet size (bytes).
  - Y-axis: Probability mass.
Unlike a continuous PDF histogram, this graph represents probability mass concentrated at one specific packet size.

B. Observed results
The entire probability mass is concentrated at:

<img width="132" height="31" alt="image" src="https://github.com/user-attachments/assets/f6eb5163-09b8-449e-8c89-07b097b51d0d" />


Therefore:



<img width="166" height="187" alt="image" src="https://github.com/user-attachments/assets/af07bcc5-999f-4261-a20c-a623c63a0499" />

These values apply to the analyzed packet data, assuming all observed packets have the same size, as indicated by the graph.

## 7. References

1. Chatgpt 
