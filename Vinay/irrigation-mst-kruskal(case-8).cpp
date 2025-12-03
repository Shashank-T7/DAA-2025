#include <bits/stdc++.h>
using namespace std;

// ===================== UNION-FIND (DISJOINT SET) =====================
struct UnionFind {
    vector<int> parent, rank;
    
    UnionFind(int n) {
        parent.resize(n + 1);
        rank.resize(n + 1, 0);
        for (int i = 1; i <= n; ++i) parent[i] = i;
    }
    
    int find(int x) {
        if (parent[x] != x) parent[x] = find(parent[x]);
        return parent[x];
    }
    
    bool unite(int x, int y) {
        int px = find(x), py = find(y);
        if (px == py) return false;
        
        if (rank[px] < rank[py]) swap(px, py);
        parent[py] = px;
        if (rank[px] == rank[py]) rank[px]++;
        return true;
    }
};

// ===================== EDGE STRUCTURE ============================
struct Edge {
    int u, v;
    int cost;           // piping cost (distance * terrain factor)
    
    bool operator<(const Edge &other) const {
        return cost < other.cost;  // for sorting
    }
};

// ===================== FARM MODEL =================================
struct Farm {
    int id;
    string name;
    double lat, lon;    // for realistic distance calculation
};

vector<Farm> farms;
vector<Edge> allEdges;
int numFarms;

// ===================== DISTANCE CALCULATION =======================
double haversine(double lat1, double lon1, double lat2, double lon2) {
    // Simplified distance (actual haversine is more complex)
    double dx = abs(lon1 - lon2) * 111 * cos((lat1 + lat2) / 2 * M_PI / 180);
    double dy = abs(lat1 - lat2) * 111;
    return sqrt(dx*dx + dy*dy);
}

// terrain factor: 1.0 flat, 1.5 hilly, 2.0 mountainous
int terrainCost(double dist, int terrain) {
    double factors[] = {1.0, 1.5, 2.0};
    return (int)(dist * factors[terrain - 1]);
}

// ===================== MST (KRUSKAL) ===============================
pair<bool, int> kruskalMST() {
    if (allEdges.empty()) {
        cout << "No edges available. Add farm connections first.\n";
        return {false, 0};
    }
    
    // Sort edges by cost (QuickSort/MergeSort via std::sort)
    sort(allEdges.begin(), allEdges.end());
    
    UnionFind uf(numFarms);
    int totalCost = 0;
    int edgesUsed = 0;
    
    cout << "Building MST - edges sorted by cost:\n";
    for (auto &e : allEdges) {
        if (uf.unite(e.u, e.v)) {
            totalCost += e.cost;
            edgesUsed++;
            cout << "Added edge " << e.u << "-" << e.v
                 << " (cost: " << e.cost << ")\n";
        }
    }
    
    bool connected = (edgesUsed == numFarms - 1);
    return {connected, totalCost};
}

// ===================== INPUT OPERATIONS ============================
void addFarm() {
    Farm f;
    cout << "Enter farm id: ";
    cin >> f.id;
    cin.ignore();
    cout << "Enter farm name: ";
    getline(cin, f.name);
    cout << "Enter latitude: ";
    cin >> f.lat;
    cout << "Enter longitude: ";
    cin >> f.lon;
    
    farms.push_back(f);
    numFarms = farms.size();
    cout << "Farm added.\n";
}

void generateConnections() {
    allEdges.clear();
    cout << "Generating possible pipe connections between farms...\n";
    
    for (int i = 0; i < numFarms; ++i) {
        for (int j = i + 1; j < numFarms; ++j) {
            double dist = haversine(farms[i].lat, farms[i].lon,
                                  farms[j].lat, farms[j].lon);
            
            int terrain;
            cout << "Terrain between farm " << farms[i].id << " and "
                 << farms[j].id << " (1=flat, 2=hilly, 3=mountain): ";
            cin >> terrain;
            
            int cost = terrainCost(dist, terrain);
            allEdges.push_back({farms[i].id, farms[j].id, cost});
        }
    }
    cout << allEdges.size() << " possible connections generated.\n";
}

void addManualEdge() {
    Edge e;
    cout << "Enter edge u v cost (farm ids and piping cost): ";
    cin >> e.u >> e.v >> e.cost;
    allEdges.push_back(e);
    cout << "Manual edge added.\n";
}

// ===================== DISPLAY ====================================
void showFarms() {
    cout << "\n=== Registered Farms ===\n";
    for (auto &f : farms) {
        cout << "ID: " << f.id << " | " << f.name
             << " | (" << f.lat << ", " << f.lon << ")\n";
    }
}

void showMSTResult(const pair<bool, int> &result) {
    cout << "\n=== Irrigation Network Planning Result ===\n";
    if (!result.first) {
        cout << "❌ Network is NOT fully connected!\n";
    } else {
        cout << "✅ All farms connected with MINIMUM piping cost!\n";
    }
    cout << "Total piping cost: Rs " << result.second << "\n";
    cout << "Edges used: " << (numFarms - 1) << " / " << allEdges.size() << "\n";
}

// ===================== MENU / MAIN ================================
void printMenu() {
    cout << "\n=== Smart Agriculture: Irrigation Network Planning ===\n";
    cout << "1. Add farm (with GPS coordinates)\n";
    cout << "2. Generate all possible pipe connections\n";
    cout << "3. Add manual pipe connection\n";
    cout << "4. Show all farms\n";
    cout << "5. Plan optimal irrigation network (Kruskal MST)\n";
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
            case 1: addFarm(); break;
            case 2: generateConnections(); break;
            case 3: addManualEdge(); break;
            case 4: showFarms(); break;
            case 5: {
                auto result = kruskalMST();
                showMSTResult(result);
                break;
            }
            case 0: cout << "Exiting.\n"; break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != 0);
    
    return 0;
}
