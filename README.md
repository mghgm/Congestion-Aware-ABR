# Congestion-Aware Adaptive Bitrate

## Overview

This project implements an in-band network telemetry system using P4-programmable data planes and eBPF-based endpoint monitoring. The framework enables real-time network congestion detection and analysis through TCP-INT (In-band Network Telemetry) extensions, providing fine-grained visibility into network queue dynamics and latency metrics.

![image](https://github.com/user-attachments/assets/9c0bf56f-ce1c-4459-b751-587c5a50f7da)

## Directory Structure

- **`bpf/pop-server/`**: Go-based HTTP server that exposes collected telemetry data via REST API
- **`bpf/sockops-parser/`**: eBPF program that parses TCP-INT options from socket operations  
- **`bpf/sockops-reserve/`**: eBPF program that reserves space for TCP-INT options in outgoing packets
- **`experiments/evaluation/`**: Multi-switch cascade topology for system evaluation
- **`linear/`**: Linear topology experiments with multiple hops
- **`p4/`**: P4 source code for programmable switches

## Reference

This work have been published as short paper in Würzburg Workshop on Next-Generation Communication Networks (WueWoWAS '26).

```bibtex
@inproceedings{ghasemi2025receiving,
  title={Receiving kernel-level insights via eBPF: Can ABR algorithms adapt smarter?},
  author={Ghasemi, Mohsen and Lorenzi, Daniele and Dolati, Mahdi and Tashtarian, Farzad and Gorinsky, Sergey and Timmerer, Christian and others},
  booktitle={W{\"u}rzburg Workshop on Next-Generation Communication Networks (WueWoWAS)},
  pages={1--4},
  year={2025}
}
```
