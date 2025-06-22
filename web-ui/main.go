package main

import (
	"fmt"
	"log"
	"net/http"
	"os"
	"path/filepath"

	"github.com/gorilla/mux"
)

func main() {
	// Initialize router
	router := mux.NewRouter()

	// Static files
	staticDir := "./static/"
	router.PathPrefix("/static/").Handler(http.StripPrefix("/static/", http.FileServer(http.Dir(staticDir))))

	// Routes
	router.HandleFunc("/", indexHandler).Methods("GET")
	router.HandleFunc("/api/query", queryHandler).Methods("POST")
	router.HandleFunc("/api/tables", getTablesHandler).Methods("GET")
	router.HandleFunc("/api/table/{name}", getTableSchemaHandler).Methods("GET")
	router.HandleFunc("/ws", websocketHandler)

	// CORS middleware
	router.Use(corsMiddleware)

	port := ":8080"
	fmt.Printf("🚀 ExtreemeDB Web UI starting on http://localhost%s\n", port)
	fmt.Println("📊 Access your in-memory database through the web interface")

	log.Fatal(http.ListenAndServe(port, router))
}

func corsMiddleware(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("Access-Control-Allow-Origin", "*")
		w.Header().Set("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
		w.Header().Set("Access-Control-Allow-Headers", "Content-Type, Authorization")

		if r.Method == "OPTIONS" {
			w.WriteHeader(http.StatusOK)
			return
		}

		next.ServeHTTP(w, r)
	})
}

func indexHandler(w http.ResponseWriter, r *http.Request) {
	indexPath := filepath.Join("static", "index.html")
	if _, err := os.Stat(indexPath); os.IsNotExist(err) {
		http.Error(w, "index.html not found", http.StatusNotFound)
		return
	}
	http.ServeFile(w, r, indexPath)
}
