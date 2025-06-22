package main

type QueryRequest struct {
	Query string `json:"query"`
}

type QueryResult struct {
	Success       bool            `json:"success"`
	Message       string          `json:"message"`
	ExecutionTime string          `json:"execution_time"`
	Columns       []string        `json:"columns,omitempty"`
	Rows          [][]interface{} `json:"rows,omitempty"`
}

type TableInfo struct {
	Name    string `json:"name"`
	Columns int    `json:"columns"`
	Rows    int    `json:"rows"`
}

type ColumnInfo struct {
	Name     string `json:"name"`
	Type     string `json:"type"`
	Nullable bool   `json:"nullable"`
}

type TableSchema struct {
	Name     string       `json:"name"`
	Columns  []ColumnInfo `json:"columns"`
	RowCount int          `json:"row_count"`
}
