class ExtreemeDBUI {
    constructor() {
        this.ws = null;
        this.queryHistory = [];
        this.init();
    }

    init() {
        this.setupEventListeners();
        this.connectWebSocket();
        this.loadTables();
    }

    setupEventListeners() {
        const executeBtn = document.getElementById('executeBtn');
        const clearBtn = document.getElementById('clearBtn');
        const queryInput = document.getElementById('queryInput');

        executeBtn.addEventListener('click', () => this.executeQuery());
        clearBtn.addEventListener('click', () => this.clearQuery());
        
        // Execute query on Ctrl+Enter
        queryInput.addEventListener('keydown', (e) => {
            if (e.ctrlKey && e.key === 'Enter') {
                this.executeQuery();
            }
        });
    }

    connectWebSocket() {
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const wsUrl = `${protocol}//${window.location.host}/ws`;
        
        this.ws = new WebSocket(wsUrl);
        
        this.ws.onopen = () => {
            this.updateConnectionStatus(true);
            console.log('WebSocket connected');
        };
        
        this.ws.onclose = () => {
            this.updateConnectionStatus(false);
            console.log('WebSocket disconnected');
            // Attempt to reconnect after 3 seconds
            setTimeout(() => this.connectWebSocket(), 3000);
        };
        
        this.ws.onmessage = (event) => {
            const data = JSON.parse(event.data);
            this.handleWebSocketMessage(data);
        };
    }

    updateConnectionStatus(connected) {
        const statusElement = document.getElementById('connectionStatus');
        if (connected) {
            statusElement.className = 'status-connected';
            statusElement.innerHTML = '<i class="fas fa-circle"></i> Connected';
        } else {
            statusElement.className = 'status-disconnected';
            statusElement.innerHTML = '<i class="fas fa-circle"></i> Disconnected';
        }
    }

    handleWebSocketMessage(data) {
        if (data.type === 'query_executed') {
            this.addToQueryHistory(data.query, data.timestamp, data.success);
        }
    }

    async executeQuery() {
        const queryInput = document.getElementById('queryInput');
        const query = queryInput.value.trim();
        
        if (!query) {
            this.showError('Please enter a query');
            return;
        }

        this.showLoading(true);
        
        try {
            const response = await fetch('/api/query', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ query })
            });

            const result = await response.json();
            this.displayResults(result);
            
        } catch (error) {
            this.showError(`Network error: ${error.message}`);
        } finally {
            this.showLoading(false);
        }
    }

    displayResults(result) {
        const resultsContent = document.getElementById('resultsContent');
        const executionInfo = document.getElementById('executionInfo');
        
        executionInfo.textContent = `Execution time: ${result.execution_time}`;
        
        if (!result.success) {
            resultsContent.innerHTML = `
                <div class="error-message">
                    <i class="fas fa-exclamation-triangle"></i>
                    <strong>Error:</strong> ${result.message}
                </div>
            `;
            return;
        }

        if (result.columns && result.rows) {
            // Display table results
            const table = this.createResultsTable(result.columns, result.rows);
            resultsContent.innerHTML = '';
            resultsContent.appendChild(table);
        } else {
            // Display success message
            resultsContent.innerHTML = `
                <div class="success-message">
                    <i class="fas fa-check-circle"></i>
                    <strong>Success:</strong> ${result.message}
                </div>
            `;
        }
    }

    createResultsTable(columns, rows) {
        const table = document.createElement('table');
        table.className = 'results-table';
        
        // Create header
        const thead = document.createElement('thead');
        const headerRow = document.createElement('tr');
        
        columns.forEach(column => {
            const th = document.createElement('th');
            th.textContent = column;
            headerRow.appendChild(th);
        });
        
        thead.appendChild(headerRow);
        table.appendChild(thead);
        
        // Create body
        const tbody = document.createElement('tbody');
        
        rows.forEach(row => {
            const tr = document.createElement('tr');
            
            row.forEach(cell => {
                const td = document.createElement('td');
                td.textContent = cell;
                tr.appendChild(td);
            });
            
            tbody.appendChild(tr);
        });
        
        table.appendChild(tbody);
        return table;
    }

    clearQuery() {
        document.getElementById('queryInput').value = '';
        document.getElementById('queryInput').focus();
    }

    showError(message) {
        const resultsContent = document.getElementById('resultsContent');
        resultsContent.innerHTML = `
            <div class="error-message">
                <i class="fas fa-exclamation-triangle"></i>
                <strong>Error:</strong> ${message}
            </div>
        `;
    }

    showLoading(show) {
        const overlay = document.getElementById('loadingOverlay');
        overlay.style.display = show ? 'flex' : 'none';
    }

    async loadTables() {
        try {
            const response = await fetch('/api/tables');
            const tables = await response.json();
            this.displayTables(tables);
        } catch (error) {
            console.error('Failed to load tables:', error);
        }
    }

    displayTables(tables) {
        const tablesList = document.getElementById('tablesList');
        
        if (tables.length === 0) {
            tablesList.innerHTML = '<div class="no-history">No tables found</div>';
            return;
        }
        
        tablesList.innerHTML = '';
        
        tables.forEach(table => {
            const tableItem = document.createElement('div');
            tableItem.className = 'table-item';
            tableItem.innerHTML = `
                <div class="table-name">${table.name}</div>
                <div class="table-info">${table.columns} columns, ${table.rows} rows</div>
            `;
            
            tableItem.addEventListener('click', () => {
                this.selectTable(table.name);
            });
            
            tablesList.appendChild(tableItem);
        });
    }

    selectTable(tableName) {
        const queryInput = document.getElementById('queryInput');
        queryInput.value = `SELECT * FROM ${tableName};`;
        queryInput.focus();
    }

    addToQueryHistory(query, timestamp, success) {
        this.queryHistory.unshift({ query, timestamp, success });
        
        // Keep only last 10 queries
        if (this.queryHistory.length > 10) {
            this.queryHistory = this.queryHistory.slice(0, 10);
        }
        
        this.updateQueryHistoryDisplay();
    }

    updateQueryHistoryDisplay() {
        const historyContainer = document.getElementById('queryHistory');
        
        if (this.queryHistory.length === 0) {
            historyContainer.innerHTML = '<div class="no-history">No queries executed yet</div>';
            return;
        }
        
        historyContainer.innerHTML = '';
        
        this.queryHistory.forEach(item => {
            const historyItem = document.createElement('div');
            historyItem.className = 'history-item';
            historyItem.innerHTML = `
                <div class="query-text">${item.query.substring(0, 50)}${item.query.length > 50 ? '...' : ''}</div>
                <div class="query-time">${item.timestamp}</div>
            `;
            
            historyItem.addEventListener('click', () => {
                document.getElementById('queryInput').value = item.query;
            });
            
            historyContainer.appendChild(historyItem);
        });
    }
}

// Initialize the application when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    new ExtreemeDBUI();
});