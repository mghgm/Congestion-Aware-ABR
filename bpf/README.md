# BPF program

## Setup
- `cd bpf && make && sudo ./loader`
- `sudo go run server.go` 

## Usage
- `curl "http://localhost:9100/lookup?ip=192.168.50.11&port=9090"`
