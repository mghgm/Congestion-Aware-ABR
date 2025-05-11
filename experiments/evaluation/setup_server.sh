#!/usr/bin/bash

ETH0="eth0"
NODE_IP="172.31.45.223"
OTHER_IP="172.31.40.188"


VETH0="mininet-veth0"
VETH0_IP="10.0.3.100/16"
VETH1="mininet-veth1"

SERVER_IP="10.0.1.1"
SERVER_MAC="00:00:00:00:01:01"

flush_iptables() {
    sudo iptables -F            
    sudo iptables -X            
    sudo iptables -Z            
    sudo iptables -t nat -F     
    sudo iptables -t nat -X     
    sudo iptables -t mangle -F  
    sudo iptables -t mangle -X  
    sudo iptables -P INPUT ACCEPT
    sudo iptables -P FORWARD ACCEPT
    sudo iptables -P OUTPUT ACCEPT 
}


if ! ip link show $VETH0 &> /dev/null || ! ip link show $VETH1 &> /dev/null; then
    echo "Creating veth pair: $VETH0 and $VETH1"
    sudo ip link add $VETH0 type veth peer name $VETH1
    
    sudo ip link set $VETH0 up
    sudo ip link set $VETH1 up

    sudo ip addr add $VETH0_IP dev $VETH0
    sudo arp -s $SERVER_IP $SERVER_MAC
fi

echo "Enable forwarding"
echo '1' | sudo tee /proc/sys/net/ipv4/conf/$VETH0/forwarding
echo '1' | sudo tee /proc/sys/net/ipv4/conf/$ETH0/forwarding

echo "Disable checksum off-loading"
sudo ethtool -K $VETH0 tx off rx off   
sudo ethtool -K $VETH1 tx off rx off   
sudo ethtool -K $ETH0 tx off rx off   



flush_iptables
echo "Setup iptables"
sudo iptables -t nat -A PREROUTING -d $NODE_IP -p tcp --dport 8080 -j DNAT --to-destination $SERVER_IP:8080
sudo iptables -t nat -A POSTROUTING -s $SERVER_IP -p tcp --sport 8080 -j SNAT --to-source $NODE_IP

#TODO Add a sed command to replace veth0 MAC in s*-commands.txt
#TODO Add a sed command to replace OTHER ip address and MAC as well
