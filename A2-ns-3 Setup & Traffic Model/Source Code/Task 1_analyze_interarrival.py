#!/usr/bin/env python3
"""Analyze inter-arrival time from the simulator CSV and/or a Wireshark PCAP.

The primary PDF/CDF outputs are for inter-arrival time only. Packet-size PDF/CDF
plots are intentionally not generated. PCAP timestamps are parsed by tshark.
"""
import argparse, csv, os, shutil, subprocess, sys
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

def load_iat_csv(path):
    vals = []
    with open(path, newline="") as f:
        for row in csv.DictReader(f):
            x = float(row["inter_arrival_s"])
            if x > 0:
                vals.append(x)
    return np.asarray(vals, dtype=float)

def load_pcap_iat(path, tshark):
    fields = ["frame.time_epoch", "ip.src", "ip.dst", "udp.srcport", "udp.dstport"]
    cmd = [tshark, "-r", path, "-Y", "udp", "-T", "fields"]
    for field in fields:
        cmd += ["-e", field]
    cmd += ["-E", "separator=,", "-E", "quote=d", "-E", "occurrence=f"]
    proc = subprocess.run(cmd, capture_output=True, text=True)
    if proc.returncode:
        raise RuntimeError(proc.stderr.strip() or "tshark failed")
    # Keep one direction (first observed source/destination/ports tuple) to
    # avoid mixing request and echo reply timestamps.
    records = []
    for line in proc.stdout.splitlines():
        parts = next(csv.reader([line]))
        if len(parts) < 5 or not parts[0]:
            continue
        try:
            ts = float(parts[0])
        except ValueError:
            continue
        key = tuple(parts[1:5])
        records.append((ts, key))
    if not records:
        raise RuntimeError("No UDP packets found in PCAP.")
    # Select the most common direction; echo request and reply are separate.
    from collections import Counter
    counts = Counter(k for _, k in records)
    direction = counts.most_common(1)[0][0]
    times = sorted(ts for ts, key in records if key == direction)
    return np.diff(times)

def make_plots(values, outdir, label):
    os.makedirs(outdir, exist_ok=True)
    values = np.asarray(values, dtype=float)
    values = values[np.isfinite(values) & (values > 0)]
    if len(values) == 0:
        raise ValueError("No positive inter-arrival samples available.")
    # PDF estimated with a normalized density histogram; CDF is empirical.
    fig, ax = plt.subplots()
    ax.hist(values * 1000, bins="fd", density=True)
    ax.set_xlabel("Inter-arrival time (ms)")
    ax.set_ylabel("Probability density (1/ms)")
    ax.set_title(f"Inter-arrival Time PDF ({label})")
    ax.grid(True, alpha=0.3); fig.tight_layout()
    fig.savefig(os.path.join(outdir, "inter_arrival_pdf.png"), dpi=160); plt.close(fig)

    xs = np.sort(values) * 1000
    ys = np.arange(1, len(xs) + 1) / len(xs)
    fig, ax = plt.subplots()
    ax.step(xs, ys, where="post")
    ax.set_xlabel("Inter-arrival time (ms)")
    ax.set_ylabel("Empirical CDF")
    ax.set_ylim(0, 1.02)
    ax.set_title(f"Inter-arrival Time CDF ({label})")
    ax.grid(True, alpha=0.3); fig.tight_layout()
    fig.savefig(os.path.join(outdir, "inter_arrival_cdf.png"), dpi=160); plt.close(fig)

    with open(os.path.join(outdir, "inter_arrival_summary.txt"), "w") as f:
        f.write(f"Source: {label}\nSamples: {len(values)}\n")
        f.write(f"Mean (ms): {np.mean(values)*1000:.6f}\n")
        f.write(f"Std dev (ms): {np.std(values, ddof=1)*1000 if len(values)>1 else 0:.6f}\n")
        f.write(f"Min (ms): {np.min(values)*1000:.6f}\n")
        f.write(f"Median (ms): {np.median(values)*1000:.6f}\n")
        f.write(f"Max (ms): {np.max(values)*1000:.6f}\n")
    np.savetxt(os.path.join(outdir, "inter_arrival_samples_ms.csv"),
               values * 1000, delimiter=",", header="inter_arrival_ms", comments="")

def main():
    p = argparse.ArgumentParser()
    p.add_argument("--iat-csv", help="Simulator output, e.g. traffic-iat.csv")
    p.add_argument("--pcap", help="PCAP file captured by ns-3")
    p.add_argument("--tshark", default="tshark", help="Path to tshark executable")
    p.add_argument("--out", default="results", help="Output directory")
    args = p.parse_args()
    if not args.iat_csv and not args.pcap:
        p.error("Provide --iat-csv and/or --pcap")
    if args.iat_csv:
        values = load_iat_csv(args.iat_csv)
        make_plots(values, os.path.join(args.out, "from_simulator_csv"), "simulator CSV")
        print("Wrote simulator-CSV inter-arrival PDF/CDF.")
    if args.pcap:
        if not shutil.which(args.tshark) and not os.path.isfile(args.tshark):
            sys.exit("tshark not found. Install Wireshark/tshark or pass --tshark PATH.")
        values = load_pcap_iat(args.pcap, args.tshark)
        make_plots(values, os.path.join(args.out, "from_pcap"), "PCAP")
        print("Wrote PCAP-derived inter-arrival PDF/CDF.")

if __name__ == "__main__":
    main()
