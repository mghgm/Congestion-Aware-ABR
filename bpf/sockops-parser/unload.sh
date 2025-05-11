sudo bpftool cgroup detach "/sys/fs/cgroup/user.slice/" sock_ops pinned "/sys/fs/bpf/bpf_sockop"
sudo rm "/sys/fs/bpf/bpf_sockop"
