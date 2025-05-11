clang -g -O2 -target bpf -c parser.bpf.c -o parser.bpf.o
sudo bpftool prog load parser.bpf.o "/sys/fs/bpf/bpf_sockop"
sudo bpftool cgroup attach "/sys/fs/cgroup/user.slice/" sock_ops pinned "/sys/fs/bpf/bpf_sockop"
