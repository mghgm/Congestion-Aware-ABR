#!/usr/bin/bash

sudo bpftool cgroup detach "/sys/fs/cgroup/unified/dash_client.slice/unified.scope/" sock_ops pinned "/sys/fs/bpf/bpf_sockop"
sudo rm /sys/fs/bpf/bpf_sockop
