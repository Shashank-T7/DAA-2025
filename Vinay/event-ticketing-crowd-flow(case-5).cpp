#include <bits/stdc++.h>
using namespace std;

// ================== CONFIG ==================
const int MAX_ROWS = 30;
const int MAX_COLS = 30;
const int MAX_NODES = 50;
const int INF = 1e9;

// ================== SEAT / ZONE MAPS (ARRAYS) ==========
int R, C;                         // rows, cols of stadium zone grid
char grid[MAX_ROWS][MAX_COLS];   // '.' = free, '#' = blocked/unsafe, 'G' = gate

// Directions for BFS (4-neighbour)
int dr[4] = {-1, 1, 0, 0};
int dc[4] = {0, 0, -1, 1};

// ================== TICKET VALIDATION (HASHING) =========
unordered_map<string, bool> ticketUsed;  // ticketId -> used or not
unordered_set<string> validTickets;      // all valid tickets

void loadTickets() {
    int n;
    cout << "Enter number of valid tickets: ";
    cin >> n;
    validTickets.clear();
    ticketUsed.clear();
    cout << "Enter ticket IDs:\n";
    for (int i = 0; i < n; ++i) {
        string id;
        cin >> id;
        validTickets.insert(id);
        ticketUsed[id] = false;
    }
    cout << "Ticket list loaded.\n";
}

void validateTicket() {
    string id;
    cout << "Enter ticket ID at gate: ";
    cin >> id;
    if (!validTickets.count(id)) {
        cout << "INVALID ticket. Access denied.\n";
        return;
    }
    if (ticketUsed[id]) {
        cout << "Ticket ALREADY USED. Possible duplicate/fraud.\n";
        return;
    }
    ticketUsed[id] = true;
    cout << "Ticket valid. Access granted.\n";
}

// ================== GRID / CROWD FLOW (BFS) =============
void loadGrid() {
    cout << "Enter grid rows and cols (<= " << MAX_ROWS << "): ";
    cin >> R >> C;
    if (R < 1 || R > MAX_ROWS || C < 1 || C > MAX_COLS) {
        cout << "Invalid size, setting to 5x5.\n";
        R = C = 5;
    }
    cout << "Enter grid (each row as string with '.', '#', 'G'):\n";
    for (int i = 0; i < R; ++i) {
        string row;
        cin >> row;
        for (int j = 0; j < C; ++j) {
            grid[i][j] = row[j];
        }
    }
}

bool inBounds(int r, int c) {
    return r >= 0 && r < R && c >= 0 && c < C;
}

// BFS from a starting cell to nearest gate 'G'
void findSafeRouteBFS() {
    int sr, sc;
    cout << "Enter start cell (row col, 0-indexed): ";
    cin >> sr >> sc;
    if (!inBounds(sr, sc) || grid[sr][sc] == '#') {
        cout << "Start is invalid or blocked.\n";
        return;
    }

    queue<pair<int,int>> q;
    vector<vector<int>> dist(R, vector<int>(C, -1));
    vector<vector<pair<int,int>>> parent(R, vector<pair<int,int>>(C, {-1,-1}));

    q.push({sr, sc});
    dist[sr][sc] = 0;

    pair<int,int> gate = {-1,-1};

    while (!q.empty()) {
        auto [r, c] = q.front();
        q.pop();

        if (grid[r][c] == 'G') {
            gate = {r, c};
            break;
        }

        for (int k = 0; k < 4; ++k) {
            int nr = r + dr[k];
            int nc = c + dc[k];
            if (inBounds(nr, nc) && grid[nr][nc] != '#' && dist[nr][nc] == -1) {
                dist[nr][nc] = dist[r][c] + 1;
                parent[nr][nc] = {r, c};
                q.push({nr, nc});
            }
        }
    }

    if (gate.first == -1) {
        cout << "No safe path to any gate found.\n";
        return;
    }

    cout << "Safe path found to gate at (" << gate.first << ", "
         << gate.second << ") with " << dist[gate.first][gate.second]
         << " steps.\n";

    // reconstruct path
    vector<pair<int,int>> path;
    pair<int,int> cur = gate;
    while (cur.first != -1) {
        path.push_back(cur);
        cur = parent[cur.first][cur.second];
    }
    reverse(path.begin(), path.end());
    cout << "Path cells:\n";
    for (auto &p : path) {
        cout << "(" << p.first << "," << p.second << ") ";
    }
    cout << "\n";
}

// ================== AREA GRAPH + DIJKSTRA ================
int numNodes;                          // distinct zones/nodes around stadium
vector<pair<int,int>> adj[MAX_NODES+1]; // (neighbor, cost)
int mainGate;                          // reference node (e.g., main entry)

void loadAreaGraph() {
    cout << "Enter number of areas/nodes (<= " << MAX_NODES << "): ";
    cin >> numNodes;
    if (numNodes < 1 || numNodes > MAX_NODES) {
        cout << "Invalid, setting numNodes = 8.\n";
        numNodes = 8;
    }
    int m;
    cout << "Enter number of paths/edges: ";
    cin >> m;
    for (int i = 1; i <= numNodes; ++i) adj[i].clear();
    cout << "Enter edges as: u v cost (time or distance)\n";
    for (int i = 0; i < m; ++i) {
        int u, v, w;
        cin >> u >> v >> w;
        if (u < 1 || v < 1 || u > numNodes || v > numNodes) continue;
        adj[u].push_back({v, w});
        adj[v].push_back({u, w});
    }
    cout << "Enter main gate node id (1.." << numNodes << "): ";
    cin >> mainGate;
    if (mainGate < 1 || mainGate > numNodes) mainGate = 1;
}

vector<int> dijkstra(int src) {
    vector<int> dist(numNodes + 1, INF);
    priority_queue<pair<int,int>, vector<pair<int,int>>,
                   greater<pair<int,int>>> pq;
    dist[src] = 0;
    pq.push({0, src});
    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (d != dist[u]) continue;
        for (auto &e : adj[u]) {
            int v = e.first, w = e.second;
            if (dist[v] > dist[u] + w) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});
            }
        }
    }
    return dist;
}

void optimalEntryPath() {
    if (numNodes == 0) {
        cout << "Load area graph first.\n";
        return;
    }
    int origin;
    cout << "Enter visitor origin node id: ";
    cin >> origin;
    if (origin < 1 || origin > numNodes) {
        cout << "Invalid origin.\n";
        return;
    }
    vector<int> dist = dijkstra(origin);
    if (dist[mainGate] >= INF) {
        cout << "No path from origin to main gate.\n";
    } else {
        cout << "Optimal travel cost from node " << origin
             << " to main gate " << mainGate
             << " = " << dist[mainGate] << "\n";
    }
}

// ================== MENU / MAIN ===========================
void printMenu() {
    cout << "\n=== Urban Event Ticketing & Crowd Flow Optimizer ===\n";
    cout << "1. Load valid ticket IDs (hashing)\n";
    cout << "2. Validate a ticket at gate\n";
    cout << "3. Load stadium zone grid (arrays)\n";
    cout << "4. Find safe route to nearest gate using BFS\n";
    cout << "5. Load area graph around stadium (for Dijkstra)\n";
    cout << "6. Compute optimal entry path (origin -> main gate)\n";
    cout << "0. Exit\n";
    cout << "Choice: ";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int choice;
    do {
        printMenu();
        cin >> choice;
        switch (choice) {
            case 1: loadTickets(); break;
            case 2: validateTicket(); break;
            case 3: loadGrid(); break;
            case 4: findSafeRouteBFS(); break;
            case 5: loadAreaGraph(); break;
            case 6: optimalEntryPath(); break;
            case 0: cout << "Exiting.\n"; break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != 0);

    return 0;
}
