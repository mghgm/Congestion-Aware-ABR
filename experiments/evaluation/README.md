# Evaluation Experiment
![image](https://github.com/user-attachments/assets/9c0bf56f-ce1c-4459-b751-587c5a50f7da)

## Setup
### Both
```bash
p4c -b bmv2 /sdf/P4-TCP-INT/p4/int.p4 -o /sdf/compiled/int
```

### Server
Set variables under `setup_server.sh` as desired and run:
```bash
bash evaluation/setup_server.sh
```
Change values in `*-server-commands.txt`:
```bash
sed -i 's/9a:23:a6:fa:11:b7/<VETH0-MAC>/g' evaluation/*-server-commands.txt
sed -i 's/172.31.40.188/<Client-Public-IP>/g' evaluation/*-server-commands.txt
```

### Client
Set variables under `setup_client.sh` as desired and run:
```bash
bash evaluation/setup_client.sh
```

Change values in `*-client-commands.txt`:
```bash
sed -i 's/f2:b5:d9:4f:b4:a7/<VETH0-MAC>/g' evaluation/*-client-commands.txt
sed -i 's/172.31.45.223/<Server-Public-IP>/g' evaluation/*-client-commands.txt
```

## Test
### Server
```bash
sudo ./run_experiment.py --switch_json /sdf/compiled/int/int.json --topo ./evaluation/server-topology.json --pcap-dir /sdf/ --log-dir /sdf/
```

In mininet CLI:
```bash
h1 nc -l 8080
```

### Client
```bash
sudo ./run_experiment.py --switch_json /sdf/compiled/int/int.json --topo ./evaluation/client-topology.json --pcap-dir /sdf/ --log-dir /sdf/
```

In mininet CLI:
```bash
h1 nc <Server-Public-IP> 8080
```

## Player
### Server
```bash
sudo ./run_experiment.py --switch_json /sdf/compiled/int/int.json --topo ./evaluation/server-topology.json --pcap-dir /sdf/ --log-dir /sdf/
```

Confirm values of defined macros in `reserve.bpf.c` with `bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h`.

Run server (Mininet CLI):
```bash
h1 pushd /home/ubuntu/server && systemd-run --unit=unified --scope --slice=dash_server python3 main.py 8080 &> /sdf/logs/server.log || popd &
```

Attach eBPF program:
```bash
# bpf/sockops-reserve
./unload.sh || ./load.sh
```

Setup load_generator

### Client
```bash
sudo ./run_experiment.py --switch_json /sdf/compiled/int/int.json --topo ./evaluation/client-topology.json --pcap-dir /sdf/ --log-dir /sdf/
```

Run client (Mininet CLI):
```bash
h1 su ubuntu
cd ~/client && systemd-run --unit=unified --scope --slice=dash_client python3 run_client.py --runs 1
```

```bash
./unload.sh || ./load.sh
```

Find map ID with `sudo bpftool map list` and replace it in `pop-server/server.go`. Then `go run server.go` as root.


