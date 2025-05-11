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
bash setup_server.sh
```
Change values in `*-server-commands.txt`:
```bash
sed -i 's/9a:23:a6:fa:11:b7/<VETH0-MAC>/g' *-server-commands.txt
sed -i 's/172.31.40.188/<Client-Public-IP>/g' *-server-commands.txt
```

### Client
```bash
chmod +x ./experiments/evaluation/setup_client.sh && ./experiments/evaluation/setup_client.sh
```

Change values in `*-client-commands.txt`:
```bash
sed -i 's/f2:b5:d9:4f:b4:a7/<VETH0-MAC>/g' *-client-commands.txt
sed -i 's/172.31.45.223/<Server-Public-IP>/g' *-client-commands.txt
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
