# P4 TCP-INT
A simple (but complete not yet!) implementation of TCP-INT based on [In-band Network Telemetry (INT) Dataplane Specification](https://p4.org/p4-spec/docs/INT_v2_1.pdf). 

## Setup
### BPF-Testbed-Able
```bash
sudo ip a add 192.168.50.11/32 dev enp7s0
sudo ip link set enp7s0 up
sudo ip route add 192.168.50.12/32 via 192.168.50.11 dev enp7s0
sudo arp -i enp7s0 -s 192.168.50.12 52:54:00:5c:a6:94
sudo ethtool --offload enp7s0 rx off tx off
```
### BPF-Testbed-Baker
```bash
sudo ip a add 192.168.50.12/32 dev enp7s0
sudo ip link set enp7s0 up
sudo ip route add 192.168.50.11/32 via 192.168.50.12 dev enp7s0
sudo arp -i enp7s0 -s 192.168.50.11 52:54:00:6c:5e:ce
sudo ethtool --offload enp7s0 rx off tx off
```
### P4-Utils
```bash
sudo ip link set enp7s0 up
sudo ip link set enp8s0 up
```
```bash
sudo simple_switch --device-id 1 --thrift-port 9090 --interface 0@enp7s0 --interface 1@enp8s0   /sdf/int/int.json
```
```bash
cat table.txt | simple_switch_CLI
table_set_default int_record add_int_record  1
table_add ipv4_lpm ipv4_forward 192.168.50.11/32 => 52:54:00:6c:5e:ce 0
table_add ipv4_lpm ipv4_forward 192.168.50.12/32 => 52:54:00:5c:a6:94 1
```


## InBand Network Telemetry metadata

## TODO
- [ ] Put real data in INT-Headers
- [ ] Handle Endiannes issue in BPF parser
- [ ] Support ipv6
- [ ] Support ipv4/ipv6 options
- [ ] Support multiple INT headers
