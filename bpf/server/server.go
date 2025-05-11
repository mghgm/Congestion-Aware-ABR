package main

import (
	"encoding/binary"
	"encoding/hex"
	"errors"
	"fmt"
	"log"
	"net"
	"net/http"

	"github.com/cilium/ebpf"
)

func lookupBPFMap(bpfMap *ebpf.Map, ip string, port uint16) ([]byte, error) {
	parsedIP := net.ParseIP(ip)
	if parsedIP == nil {
		return nil, errors.New("invalid IP address")
	}

	ipv4 := parsedIP.To4()
	if ipv4 == nil {
		return nil, errors.New("only IPv4 is supported")
	}

	var key [8]byte
	binary.LittleEndian.PutUint32(key[:4], binary.BigEndian.Uint32(ipv4))
	binary.LittleEndian.PutUint16(key[4:6], port)
	binary.LittleEndian.PutUint16(key[6:], 0)

	var value [12]byte
	if err := bpfMap.Lookup(&key, &value); err != nil {
		return nil, fmt.Errorf("failed to lookup key: %w", err)
	}

	return value[len(value)-4:len(value)-3], nil
}

func handleLookup(bpfMap *ebpf.Map) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		ip := r.URL.Query().Get("ip")
		portStr := r.URL.Query().Get("port")
		if ip == "" || portStr == "" {
			http.Error(w, "Missing ip or port parameter", http.StatusBadRequest)
			return
		}

		port, err := parsePort(portStr)
		if err != nil {
			http.Error(w, "Invalid port", http.StatusBadRequest)
			return
		}

		value, err := lookupBPFMap(bpfMap, ip, port)
		if err != nil {
			http.Error(w, fmt.Sprintf("Lookup failed: %v", err), http.StatusInternalServerError)
			return
		}

		w.WriteHeader(http.StatusOK)
		w.Write([]byte(hex.EncodeToString(value)))
	}
}

func parsePort(portStr string) (uint16, error) {
	var port uint16
	_, err := fmt.Sscanf(portStr, "%d", &port)
	return port, err
}

func main() {
	mapID := ebpf.MapID(23)
	bpfMap, err := ebpf.NewMapFromID(mapID)
	if err != nil {
		log.Fatalf("Failed to open BPF map by ID: %v", err)
	}
	defer bpfMap.Close()

	http.HandleFunc("/lookup", handleLookup(bpfMap))
	log.Println("Server listening on :9100")
	http.ListenAndServe(":9100", nil)
}

