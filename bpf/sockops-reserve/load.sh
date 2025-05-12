#!/usr/bin/bash

clang -g -O2 -target bpf -c reserve.bpf.c -o reserve.bpf.o
sudo bpftool prog load /sdf/P4-TCP-INT/bpf/sockops-reserve/reserve.bpf.o "/sys/fs/bpf/bpf_sockop"
sudo bpftool cgroup attach "/sys/fs/cgroup/unified/dash_server.slice/unified.scope/" sock_ops pinned "/sys/fs/bpf/bpf_sockop"
