#!/usr/bin/env python3
"""Create packet-size PDF and CDF from a PCAP using tshark."""
import argparse
import csv
import shutil
import subprocess
import sys
from pathlib import Path

def main():
    parser = argparse.ArgumentParser(description="Packet-size PDF/CDF from PCAP")
    parser.add_argument("--pcap", required=True, help="PCAP file created by ns-3")
    parser.add_argument("--out", default="results", help="Output directory")
    parser.add_argument(
        "--size", choices=("payload", "udp", "ip", "frame"), default="payload",
        help="Size metric: UDP payload (default), UDP length, IPv4 length, or captured frame length"
    )
    parser.add_argument("--filter", default="udp", help='Wireshark display filter (default: "udp")')
    parser.add_argument("--bins", type=int, default=20, help="Histogram bins for PDF plot")
    parser.add_argument("--kde", action="store_true", help="Add KDE density curve (requires scipy)")
    args = parser.parse_args()

    pcap = Path(args.pcap)
    if not pcap.is_file():
        sys.exit(f"ERROR: PCAP file not found: {pcap}")
    if not shutil.which("tshark"):
        sys.exit("ERROR: tshark not found. Install Wireshark/tshark first.")

    field_by_size = {
        "payload": "udp.length",
        "udp": "udp.length",
        "ip": "ip.len",
        "frame": "frame.len",
    }
    field = field_by_size[args.size]
    cmd = ["tshark", "-r", str(pcap), "-Y", args.filter, "-T", "fields",
           "-E", "separator=,", "-E", "quote=n", "-e", field]
    try:
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
    except subprocess.CalledProcessError as exc:
        sys.exit("ERROR running tshark: " + (exc.stderr or str(exc)))

    sizes = []
    for line in result.stdout.splitlines():
        try:
            value = int(line.strip().split(",")[0])
        except (ValueError, IndexError):
            continue
        if args.size == "payload":
            value -= 8  # UDP length includes its 8-byte header
        if value > 0:
            sizes.append(value)

    if not sizes:
        sys.exit("ERROR: no packet sizes found. Check the PCAP and --filter.")

    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        sys.exit("ERROR: matplotlib missing; install with: python3 -m pip install matplotlib")

    if args.bins < 1:
        sys.exit("--bins must be at least 1")

    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    sizes.sort()
    n = len(sizes)
    metric_label = {
        "payload": "UDP payload size (bytes)",
        "udp": "UDP datagram length including header (bytes)",
        "ip": "Packet Size (bytes)",
        "frame": "Captured frame length (bytes)",
    }[args.size]

    # Raw observations
    with (out / "packet_sizes.csv").open("w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(["packet_size_bytes"])
        writer.writerows([[v] for v in sizes])

    # Empirical CDF, one row per distinct size
    xs, ys = [], []
    for i, value in enumerate(sizes):
        if i == n - 1 or sizes[i + 1] != value:
            xs.append(value)
            ys.append((i + 1) / n)
    with (out / "packet_size_cdf.csv").open("w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(["packet_size_bytes", "cdf"])
        writer.writerows(zip(xs, ys))

    # PDF and discrete probability mass for constant-size traffic.
    from collections import Counter
    counts = Counter(sizes)
    unique = sorted(counts)
    with (out / "packet_size_density.csv").open("w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["packet_size_bytes", "probability_mass"])
        writer.writerows((x, counts[x] / n) for x in unique)

    plt.figure(figsize=(8, 5))
    if len(unique) == 1:
        x = unique[0]
        plt.vlines(x, 0, 1, linewidth=2)
        plt.scatter([x], [1], zorder=3)
        plt.xlim(x - 20, x + 20)
        plt.ylabel("Probability mass")
        plt.title("Packet Size Distribution (single size)")
    else:
        plt.hist(sizes, bins=args.bins, density=True, edgecolor="black",
                 alpha=0.5, label="Normalized histogram")
        if args.kde:
            try:
                import numpy as np
                from scipy.stats import gaussian_kde
                grid = np.linspace(min(sizes), max(sizes), 500)
                density = gaussian_kde(sizes)(grid)
                plt.plot(grid, density, linewidth=2, label="KDE")
                with (out / "packet_size_kde.csv").open("w", newline="") as f:
                    writer = csv.writer(f)
                    writer.writerow(["packet_size_bytes", "density_per_byte"])
                    writer.writerows(zip(grid, density))
            except Exception as exc:
                print(f"WARNING: KDE unavailable: {exc}", file=sys.stderr)
        plt.ylabel("Probability density (1/byte)")
        plt.title("PDF of Packet Sizes")
        plt.legend()
    plt.xlabel(metric_label)
    plt.grid(True, alpha=0.25)
    plt.tight_layout()
    plt.savefig(out / "packet_size_pdf.png", dpi=160)
    plt.close()

    # Plot empirical CDF, including the case where every packet has the same size.
    plt.figure(figsize=(8, 5))

    if len(xs) == 1:
        # A single unique size produces a jump from 0 to 1 at that size.
        plt.vlines(xs[0], 0, 1, linewidth=2, label="Empirical CDF")
        plt.scatter([xs[0]], [1.0], zorder=3)
        plt.xlim(xs[0] - 20, xs[0] + 20)
    else:
        # Multiple unique sizes: draw the empirical step CDF.
        plt.step(xs, ys, where="post")
        plt.scatter(xs, ys, s=12)
        plt.xlim(min(xs), max(xs))

    plt.xlabel(metric_label)
    plt.ylabel("CDF  F(x) = P(X ≤ x)")
    plt.title("Empirical CDF of Packet Sizes")
    plt.ylim(0, 1.02)
    plt.grid(True, alpha=0.25)
    plt.tight_layout()
    plt.savefig(out / "packet_size_cdf.png", dpi=160)
    plt.close()

    print(f"Packets analyzed : {n}")
    print(f"Size metric      : {args.size}")
    print(f"Minimum size     : {min(sizes)} bytes")
    print(f"Maximum size     : {max(sizes)} bytes")
    print(f"Mean size        : {sum(sizes)/n:.2f} bytes")
    print(f"PDF plot         : {out / 'packet_size_pdf.png'}")
    print(f"CDF plot         : {out / 'packet_size_cdf.png'}")
    print(f"CDF data         : {out / 'packet_size_cdf.csv'}")
    print(f"Raw sizes        : {out / 'packet_sizes.csv'}")
    print(f"Density data     : {out / 'packet_size_density.csv'}")

if __name__ == "__main__":
    main()
