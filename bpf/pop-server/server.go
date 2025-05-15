package main

import (
	"encoding/binary"
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"sync"
	"time"

	"github.com/cilium/ebpf"
)

type CongestionInfo struct {
	Timestamp 		uint64	`json:"timestamp"`
	QueueLength 	uint8	`json:"queue_length"` 
	Timedelta		uint16  `json:"timedelta"` 
}

var CongestionInfoQueue []CongestionInfo
var QueueLock sync.Mutex

const (
	serverPort = 9110
)

func handleQeueu(w http.ResponseWriter, r *http.Request) {
	ip := r.URL.Query().Get("ip")
	portStr := r.URL.Query().Get("port")
	if ip == "" || portStr == "" {
		http.Error(w, "Missing ip or port parameter", http.StatusBadRequest)
		return
	}

	QueueLock.Lock()
	tmpQueue := CongestionInfoQueue[:]
	CongestionInfoQueue = make([]CongestionInfo, 0)
	QueueLock.Unlock()
	
	data, err := json.Marshal(tmpQueue)
	if err != nil {
		http.Error(w, "Faild pop queue", http.StatusBadGateway)
	}

	w.WriteHeader(http.StatusOK)
	w.Write(data)
}

func popQueue(bpfMap *ebpf.Map) {
	var err error
	
	var v [11]byte;
	for err == nil {
		err = bpfMap.LookupAndDelete(nil, &v)
		if err == nil {
			QueueLock.Lock()
			CongestionInfoQueue = append(CongestionInfoQueue, CongestionInfo{
				Timestamp: binary.LittleEndian.Uint64(v[:8]),
				QueueLength: uint8(v[8]),
				Timedelta: binary.LittleEndian.Uint16(v[9:]),
			})
			QueueLock.Unlock()
		} else {
			fmt.Printf("Err: %v\n", err)
		}
	}
}

func main() {
	CongestionInfoQueue = make([]CongestionInfo, 0)

	mapID := ebpf.MapID(11)
	bpfMap, err := ebpf.NewMapFromID(mapID)
	if err != nil {
		log.Fatalf("Failed to open BPF map by ID: %v", err)
	}
	defer bpfMap.Close()

	ticker := time.NewTicker(100 * time.Millisecond)	
	done := make(chan bool)
	
	go func() {
		for {
			select {
			case <- done:
				return
			case <- ticker.C:
				fmt.Println("Poping queue")
				popQueue(bpfMap)
			}
		}
	}()
	

	http.HandleFunc("/queue", handleQeueu)
	http.ListenAndServe(fmt.Sprintf(":%v", serverPort), nil)
}
