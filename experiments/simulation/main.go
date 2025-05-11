package main

import (
	"encoding/csv"
	"fmt"
	"net/http"
	"os"
	"strconv"
	"time"
	"sync"
	"encoding/json"
)

const httpPort = 9110

type QueueEntry struct {
	Timeout     float64
	QueueLength int
}

type Message struct {
	Depth  int       `json:"depth"`
	Timestamp time.Time `json:"timestamp"`
}

var (
	QueueData []QueueEntry
	startTimestamp time.Time
	DataMQ []Message
	mu sync.Mutex
)


func LoadCSV(filename string) error {
	file, err := os.Open(filename)
	if err != nil {
		return err
	}
	defer file.Close()

	reader := csv.NewReader(file)
	records, err := reader.ReadAll()
	if err != nil {
		return err
	}

	for _, record := range records {
		if len(record) < 2 {
			continue
		}
		timeout, err := strconv.ParseFloat(record[0], 64)
		if err != nil {
			return fmt.Errorf("invalid timeout value: %v", err)
		}
		queueLen, err := strconv.Atoi(record[1])
		if err != nil {
			return fmt.Errorf("invalid queue length: %v", err)
		}
		QueueData = append(QueueData, QueueEntry{Timeout: timeout, QueueLength: queueLen})
	}
	return nil
}


func handleStart(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	ip := r.URL.Query().Get("ip")
	port := r.URL.Query().Get("port")

	if ip == "" || port == "" {
		http.Error(w, "Missing ip or port parameter", http.StatusBadRequest)
		return
	}

	
	initializeSimulation()

	w.WriteHeader(http.StatusOK)
}

func initializeSimulation() {
	startTimestamp = time.Now()

	for _, q := range QueueData {
		go func () {
			time.Sleep(time.Second * time.Duration(q.Timeout))
			mu.Lock()
			DataMQ = append(DataMQ, Message{
				q.QueueLength,
				startTimestamp.Add(time.Duration(q.Timeout)),
			})
			mu.Unlock()
		} ()
	}
}

func handleQueue(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodGet {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	ip := r.URL.Query().Get("ip")
	port := r.URL.Query().Get("port")

	if ip == "" || port == "" {
		http.Error(w, "Missing ip or port parameter", http.StatusBadRequest)
		return
	}
	
	mu.Lock()
	messages := DataMQ
	DataMQ = nil
	mu.Unlock()
	
	w.Header().Set("Content-Type", "application/json")
	err := json.NewEncoder(w).Encode(messages)
	if err != nil {
		http.Error(w, "Failed to encode JSON", http.StatusInternalServerError)
	}
}

func main() {
	err := LoadCSV("queue.csv")
	if err != nil {
		fmt.Println("Error loading CSV:", err)
		return
	}
	

	http.HandleFunc("/start", handleStart)
	http.HandleFunc("/queue", handleQueue)

	fmt.Printf("Server started on port %d\n", httpPort)
	err = http.ListenAndServe(":" + strconv.Itoa(httpPort), nil)
	if err != nil {
		fmt.Println("HTTP server error:", err)
	}

}


