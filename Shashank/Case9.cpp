
// Water Treatment & Pipeline Optimization Demo
// Purpose: Manage treatment stages, optimize flow & detect leaks.
//
// Algorithms / DS:
// - BFS/DFS: pipeline flow tracing on graph
// - Dijkstra/Bellman-Ford: lowest pressure-loss paths
// - Union-Find: zone isolation / connectivity
// - Segment Tree: flow/pressure window queries
// - Sparse Table: historical RMQ (min pressure, max flow)
// - Queue: pump operation scheduling
// - Arrays: pipeline node/edge data

#include <bits/stdc++.h>
using namespace std;

// -------------------- Core structures --------------------

struct Node {
    int id;
    string name;
    bool isTreatmentStage; // e.g., sedimentation, filtration, chlorination
};

struct Pipe {
    int u, v;
    double loss;   // pressure loss along this pipe segment
    double length;
};

struct PumpOp {
    int id;
    int pumpId;
    int timeIndex;
    string action; // "START", "STOP", "INCREASE_SPEED", ...
};

// =====================================================================
// BFS / DFS: pipeline flow tracing from a source node
// =====================================================================

void bfsFlow(int start, const vector<vector<int>>& adj) {
    vector<bool> vis(adj.size(), false);
    queue<int> q;
    q.push(start);
    vis[start] = true;
    cout << "BFS flow trace from node " << start << ":\n  ";
    while (!q.empty()) {
        int u = q.front(); q.pop();
        cout << u << " ";
        for (int v : adj[u]) {
            if (!vis[v]) {
                vis[v] = true;
                q.push(v);
            }
        }
    }
    cout << "\n\n";
}

void dfsUtil(int u, const vector<vector<int>>& adj, vector<bool>& vis) {
    vis[u] = true;
    cout << u << " ";
    for (int v : adj[u]) {
        if (!vis[v]) dfsUtil(v, adj, vis);
    }
}

void dfsFlow(int start, const vector<vector<int>>& adj) {
    vector<bool> vis(adj.size(), false);
    cout << "DFS flow trace from node " << start << ":\n  ";
    dfsUtil(start, adj, vis);
    cout << "\n\n";
}

/*
BFS/DFS comments (water context):
- Model tanks, junctions, and treatment stages as nodes; pipes as edges.
- BFS/DFS from a source traces all reachable parts of the network and helps detect isolated or leak-affected segments.
- Useful for quickly understanding which consumer zones depend on a given plant or main line.
*/

// =====================================================================
// Dijkstra: minimum pressure-loss path
// =====================================================================

vector<double> dijkstra(int n, int src, const vector<vector<pair<int,double>>>& adj) {
    const double INF = 1e18;
    vector<double> dist(n, INF);
    dist[src] = 0.0;
    using P = pair<double,int>;
    priority_queue<P, vector<P>, greater<P>> pq;
    pq.push({0.0, src});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        for (auto [v, w] : adj[u]) {
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});
            }
        }
    }
    return dist;
}

/*
Dijkstra comments (water context):
- Treats pipe losses as non-negative weights and finds paths with minimal total pressure loss.
- Guides decisions on which route to supply a given zone from when multiple mains exist.
- Also helpful for planning new branches with minimal additional head loss.
*/

// =====================================================================
// Bellman-Ford: handles negative weights (e.g., modeled pumps/boosts)
// =====================================================================

struct BFEdge {
    int u, v;
    double w; // generalized "cost" (could be loss or negative for boosts)
};

vector<double> bellmanFord(int n, int src, const vector<BFEdge>& edges) {
    const double INF = 1e18;
    vector<double> dist(n, INF);
    dist[src] = 0.0;

    for (int i = 1; i <= n - 1; ++i) {
        for (auto& e : edges) {
            if (dist[e.u] < INF && dist[e.u] + e.w < dist[e.v]) {
                dist[e.v] = dist[e.u] + e.w;
            }
        }
    }
    // Negative cycle detection skipped for brevity.
    return dist;
}

/*
Bellman-Ford comments (water context):
- Allows modeling elements that add pressure (pumps) as negative weights and still compute shortest paths.
- Suitable when the effective path cost can become negative due to boosts or complex energy accounting.
- Also helps detect inconsistent or erroneous network models through negative cycles.
*/

// =====================================================================
// Union-Find: isolate / group pipeline zones (components)
// =====================================================================

class UnionFind {
public:
    vector<int> parent, rk;
    UnionFind(int n = 0) { init(n); }

    void init(int n) {
        parent.resize(n);
        rk.assign(n, 0);
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int x) {
        if (parent[x] == x) return x;
        return parent[x] = find(parent[x]);
    }

    void unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return;
        if (rk[a] < rk[b]) swap(a, b);
        parent[b] = a;
        if (rk[a] == rk[b]) rk[a]++;
    }
};

void demoZones() {
    int n = 6; // 6 nodes
    UnionFind uf(n);

    // Suppose (0-1-2) are in Zone A, (3-4-5) in Zone B
    uf.unite(0, 1);
    uf.unite(1, 2);
    uf.unite(3, 4);
    uf.unite(4, 5);

    unordered_map<int, vector<int>> zones;
    for (int i = 0; i < n; ++i)
        zones[uf.find(i)].push_back(i);

    cout << "Pipeline zones (Union-Find components):\n";
    for (auto& z : zones) {
        cout << "  Zone root " << z.first << " : nodes ";
        for (int v : z.second) cout << v << " ";
        cout << "\n";
    }
    cout << "\n";
}

/*
Union-Find comments (water context):
- Groups nodes into connectivity zones; each component can be isolated with a few valves.
- When a leak is detected between two nodes, affected zone(s) can be quickly identified.
- Supports dynamic reconfiguration as valves open/close or new pipes are added.
*/

// =====================================================================
// Segment Tree: flow / pressure window queries (max here)
// =====================================================================

class SegmentTree {
public:
    int n;
    vector<double> tree;

    SegmentTree(int size = 0) { init(size); }

    void init(int size) {
        n = size;
        tree.assign(4 * n, 0.0);
    }

    void build(const vector<double>& a, int v, int tl, int tr) {
        if (tl == tr) tree[v] = a[tl];
        else {
            int tm = (tl + tr) / 2;
            build(a, v * 2, tl, tm);
            build(a, v * 2 + 1, tm + 1, tr);
            tree[v] = max(tree[v * 2], tree[v * 2 + 1]);
        }
    }

    void build(const vector<double>& a) {
        if (n > 0) build(a, 1, 0, n - 1);
    }

    void update(int v, int tl, int tr, int idx, double val) {
        if (tl == tr) tree[v] = val;
        else {
            int tm = (tl + tr) / 2;
            if (idx <= tm) update(v * 2, tl, tm, idx, val);
            else update(v * 2 + 1, tm + 1, tr, idx, val);
            tree[v] = max(tree[v * 2], tree[v * 2 + 1]);
        }
    }

    void update(int idx, double val) { update(1, 0, n - 1, idx, val); }

    double query(int v, int tl, int tr, int l, int r) {
        if (l > r) return -1e18;
        if (l == tl && r == tr) return tree[v];
        int tm = (tl + tr) / 2;
        return max(
            query(v * 2, tl, tm, l, min(r, tm)),
            query(v * 2 + 1, tm + 1, tr, max(l, tm + 1), r)
        );
    }

    double query(int l, int r) { return query(1, 0, n - 1, l, r); }
};

/*
Segment Tree comments (water context):
- Built over a time-series of flow or pressure readings for one sensor.
- Quickly answers â€œmaximum flow in last N intervalsâ€ or similar safety checks.
- Supports online updates as new readings arrive from SCADA/IoT devices.
*/

// =====================================================================
// Sparse Table: historical min/ max pressure (RMQ) on static logs
// =====================================================================

class SparseTable {
public:
    int n, K;
    vector<vector<double>> st;
    vector<int> lg;

    void build(const vector<double>& a) {
        n = (int)a.size();
        K = 32 - __builtin_clz(n);
        st.assign(K, vector<double>(n));
        lg.assign(n + 1, 0);
        for (int i = 2; i <= n; ++i)
            lg[i] = lg[i / 2] + 1;

        for (int i = 0; i < n; ++i)
            st[0][i] = a[i];

        for (int k = 1; k < K; ++k)
            for (int i = 0; i + (1 << k) <= n; ++i)
                st[k][i] = min(st[k - 1][i], st[k - 1][i + (1 << (k - 1))]);
    }

    double rangeMin(int l, int r) const {
        int len = r - l + 1;
        int k = lg[len];
        return min(st[k][l], st[k][r - (1 << k) + 1]);
    }
};

void demoFlowPressureQueries() {
    // Example pressure series at a node over time
    vector<double> pressure = {5.0, 4.8, 4.7, 3.5, 3.0, 4.0, 4.2, 4.4};

    SegmentTree seg((int)pressure.size());
    seg.build(pressure);
    cout << "SegmentTree: max pressure[1..4] = "
         << seg.query(1, 4) << " bar\n";

    SparseTable st;
    st.build(pressure);
    cout << "SparseTable: min pressure[1..4] = "
         << st.rangeMin(1, 4) << " bar\n\n";
}

/*
Sparse Table comments (water context):
- Built periodically on archived pressure logs to support instant RMQ.
- Answers historical queries like â€œlowest pressure during last nightâ€™s shiftâ€ in O(1).
- Complements segment trees, which focus on the live streaming side.
*/

// =====================================================================
// Queue: pump operations (simple scheduler)
// =====================================================================

void demoPumpQueue() {
    queue<PumpOp> ops;
    ops.push({1001, 1, 10, "START"});
    ops.push({1002, 2, 12, "INCREASE_SPEED"});
    ops.push({1003, 1, 20, "STOP"});

    cout << "Pump operation queue (FIFO):\n";
    while (!ops.empty()) {
        auto op = ops.front(); ops.pop();
        cout << "  Op " << op.id << " pump " << op.pumpId
             << " t=" << op.timeIndex << " action=" << op.action << "\n";
    }
    cout << "\n";
}

/*
Queue comments (water context):
- Represents scheduled pump commands in time order.
- Control center or PLC can dequeue and execute operations in sequence.
- Simple model for automation scripts and manual overrides.
*/

// =====================================================================
// Arrays: pipeline data and routing demo
// =====================================================================

void demoNetworkAndRouting() {
    // Nodes (treatment stages + junctions)
    vector<Node> nodes = {
        {0, "Intake",           true},
        {1, "Sedimentation",    true},
        {2, "Filtration",       true},
        {3, "Chlorination",     true},
        {4, "Reservoir",        false},
        {5, "Consumer_Junction",false}
    };

    cout << "Pipeline nodes:\n";
    for (auto& n : nodes) {
        cout << "  " << n.id << " : " << n.name
             << " stage=" << (n.isTreatmentStage ? "Y" : "N") << "\n";
    }
    cout << "\n";

    // Adjacency for BFS/DFS (undirected for reachability)
    int N = nodes.size();
    vector<vector<int>> adj(N);
    auto addEdgeSimple = [&](int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    };
    addEdgeSimple(0, 1);
    addEdgeSimple(1, 2);
    addEdgeSimple(2, 3);
    addEdgeSimple(3, 4);
    addEdgeSimple(4, 5);

    bfsFlow(0, adj);
    dfsFlow(0, adj);

    // Weighted graph for Dijkstra/Bellman-Ford (pressure loss as weights)
    vector<vector<pair<int,double>>> adjW(N);
    vector<BFEdge> bfEdges;
    auto addPipe = [&](int u, int v, double loss) {
        adjW[u].push_back({v, loss});
        adjW[v].push_back({u, loss});
        bfEdges.push_back({u, v, loss});
        bfEdges.push_back({v, u, loss});
    };

    addPipe(0, 1, 0.2);
    addPipe(1, 2, 0.3);
    addPipe(2, 3, 0.4);
    addPipe(3, 4, 0.1);
    addPipe(4, 5, 0.2);

    int src = 0; // Intake
    auto dDist = dijkstra(N, src, adjW);
    auto bDist = bellmanFord(N, src, bfEdges);

    cout << "Dijkstra: pressure-loss from " << nodes[src].name << ":\n";
    for (int i = 0; i < N; ++i)
        cout << "  to " << nodes[i].name << " : " << dDist[i] << "\n";
    cout << "\n";

    cout << "Bellman-Ford: pressure-loss from " << nodes[src].name << ":\n";
    for (int i = 0; i < N; ++i)
        cout << "  to " << nodes[i].name << " : " << bDist[i] << "\n";
    cout << "\n";
}

// =====================================================================
// MAIN
// =====================================================================

int main() {
    cout << "=== Water Treatment & Pipeline Optimization Demo ===\n\n";

    demoNetworkAndRouting();
    demoZones();
    demoFlowPressureQueries();
    demoPumpQueue();

    return 0;
}
