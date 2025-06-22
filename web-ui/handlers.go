package main

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"strings"
	"time"

	"github.com/gorilla/mux"
	"github.com/gorilla/websocket"
)

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true // Allow all origins for development
	},
}

var clients = make(map[*websocket.Conn]bool)
var broadcast = make(chan []byte)

func init() {
	go handleMessages()
}

func handleMessages() {
	for {
		msg := <-broadcast
		for client := range clients {
			err := client.WriteMessage(websocket.TextMessage, msg)
			if err != nil {
				log.Printf("Websocket error: %v", err)
				client.Close()
				delete(clients, client)
			}
		}
	}
}

func websocketHandler(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Printf("Websocket upgrade error: %v", err)
		return
	}
	defer conn.Close()

	clients[conn] = true
	log.Println("Client connected via WebSocket")

	for {
		_, _, err := conn.ReadMessage()
		if err != nil {
			log.Printf("Websocket read error: %v", err)
			delete(clients, conn)
			break
		}
	}
}

func queryHandler(w http.ResponseWriter, r *http.Request) {
	var req QueryRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "Invalid JSON", http.StatusBadRequest)
		return
	}

	// Execute query using the C++ executable
	result := executeQuery(req.Query)

	// Broadcast query execution to WebSocket clients
	broadcastMsg := map[string]interface{}{
		"type":      "query_executed",
		"query":     req.Query,
		"timestamp": time.Now().Format("15:04:05"),
		"success":   result.Success,
	}

	msgBytes, _ := json.Marshal(broadcastMsg)
	select {
	case broadcast <- msgBytes:
	default:
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(result)
}

func executeQuery(query string) QueryResult {
	// For demo purposes, we'll simulate the C++ database execution
	// In a real implementation, you would:
	// 1. Write the query to a temporary file
	// 2. Execute your C++ binary with the query
	// 3. Parse the output

	// Simulate different query types
	query = strings.TrimSpace(strings.ToUpper(query))

	if strings.HasPrefix(query, "CREATE TABLE") {
		return QueryResult{
			Success:       true,
			Message:       "Table created successfully",
			ExecutionTime: "2ms",
		}
	}

	if strings.HasPrefix(query, "INSERT INTO") {
		return QueryResult{
			Success:       true,
			Message:       "1 row inserted",
			ExecutionTime: "1ms",
		}
	}

	if strings.HasPrefix(query, "SELECT") {
		// Simulate SELECT result
		return QueryResult{
			Success:       true,
			Message:       "Query executed successfully",
			ExecutionTime: "3ms",
			Columns:       []string{"id", "name", "age"},
			Rows: [][]interface{}{
				{1, "John Doe", 25},
				{2, "Jane Smith", 30},
				{3, "Bob Johnson", 35},
			},
		}
	}

	if strings.HasPrefix(query, "DROP TABLE") {
		return QueryResult{
			Success:       true,
			Message:       "Table dropped successfully",
			ExecutionTime: "1ms",
		}
	}

	// For actual C++ execution (uncomment when ready):
	/*
	   cmd := exec.Command("../build/InMemoryPLSQLDB.exe")
	   cmd.Stdin = strings.NewReader(query + "\nexit\n")

	   output, err := cmd.CombinedOutput()
	   if err != nil {
	       return QueryResult{
	           Success: false,
	           Message: fmt.Sprintf("Execution error: %v", err),
	           ExecutionTime: "0ms",
	       }
	   }

	   // Parse the C++ output and return structured result
	   return parseQueryOutput(string(output))
	*/

	return QueryResult{
		Success:       false,
		Message:       "Unknown query type",
		ExecutionTime: "0ms",
	}
}

func parseQueryOutput(output string) QueryResult {
	// Parse the C++ program output
	// This would need to be implemented based on your C++ output format
	lines := strings.Split(output, "\n")

	if strings.Contains(output, "Error:") {
		return QueryResult{
			Success:       false,
			Message:       "Query execution failed",
			ExecutionTime: "0ms",
		}
	}

	// Extract results from output
	var columns []string
	var rows [][]interface{}

	// Simple parsing logic - you'll need to adapt this
	for i, line := range lines {
		line = strings.TrimSpace(line)
		if line == "" {
			continue
		}

		if i == 0 && strings.Contains(line, "\t") {
			// Header line
			columns = strings.Split(line, "\t")
		} else if strings.Contains(line, "\t") {
			// Data line
			parts := strings.Split(line, "\t")
			row := make([]interface{}, len(parts))
			for j, part := range parts {
				row[j] = part
			}
			rows = append(rows, row)
		}
	}

	return QueryResult{
		Success:       true,
		Message:       fmt.Sprintf("%d rows returned", len(rows)),
		ExecutionTime: "5ms",
		Columns:       columns,
		Rows:          rows,
	}
}

func getTablesHandler(w http.ResponseWriter, r *http.Request) {
	// Mock table list - in real implementation, query your C++ database
	tables := []TableInfo{
		{Name: "users", Columns: 3, Rows: 150},
		{Name: "orders", Columns: 5, Rows: 1200},
		{Name: "products", Columns: 4, Rows: 45},
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(tables)
}

func getTableSchemaHandler(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	tableName := vars["name"]

	// Mock schema - in real implementation, query your C++ database
	schema := TableSchema{
		Name: tableName,
		Columns: []ColumnInfo{
			{Name: "id", Type: "INTEGER", Nullable: false},
			{Name: "name", Type: "VARCHAR", Nullable: false},
			{Name: "email", Type: "VARCHAR", Nullable: true},
			{Name: "created_at", Type: "TIMESTAMP", Nullable: false},
		},
		RowCount: 150,
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(schema)
}
