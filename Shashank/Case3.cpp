

// Warehouse & Distribution Routing Demo
// Purpose: Optimize depot connections (network) and delivery batches (order groups).
//
// Algorithms / DS used:
// - Kruskal / Prim: build MST (cheapest depot/route connectivity)
// - Union-Find (Disjoint Set Union): component tracking for Kruskal
// - Sorting: batch ordering by priority, deadline, or region
// - List (std::list / std::vector): route sequences (stops in visiting order)

#include <bits/stdc++.h>
using namespace std;

// -------------------- Basic Graph Structures --------------------

struct Edge {
    int u, v;
    int cost; // cost to connect depot u and depot v (distance, fuel, tolls, etc.)
};

struct Depot {
    int id;
    double x, y; // coordinates for illustration (not used directly in MST here)
};

// -------------------- Union-Find / Disjoint Set --------------------
// Used in Kruskal to track which depots are in which connected component.

class UnionFind {
public:
    vector<int> parent, rankVec;

    UnionFind(int n = 0) { init(n); }

    void init(int n) {
        parent.resize(n);
        rankVec.assign(n, 0);
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int x) {
        if (parent[x] == x) return x;
        return parent[x] = find(parent[x]); // path compression
    }

    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (rankVec[a] < rankVec[b]) swap(a, b);
        parent[b] = a;
        if (rankVec[a] == rankVec[b]) rankVec[a]++;
        return true;
    }
};

/*
Union-Find comments (warehouse context):
- Each depot starts as its own component (separate region).
- As edges are added in Kruskal, components merge when a new cheap link is chosen.
- Prevents cycles and helps stop when all depots are connected in one network.
*/

// -------------------- Kruskal's Algorithm (MST) --------------------
// Build a minimum-cost network of depots using edge list + Union-Find.

vector<Edge> kruskalMST(int numDepots, vector<Edge> edges) {
    // Sort edges by increasing cost.
    sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b) {
        return a.cost < b.cost;
    });

    UnionFind uf(numDepots);
    vector<Edge> mst;
    int totalCost = 0;

    for (const auto& e : edges) {
        if (uf.unite(e.u, e.v)) { // if this edge connects two different components
            mst.push_back(e);
            totalCost += e.cost;
        }
    }

    cout << "Kruskal MST total connection cost = " << totalCost << "\n";
    cout << "Edges in MST (u - v : cost):\n";
    for (auto& e : mst) {
        cout << "  " << e.u << " - " << e.v << " : " << e.cost << "\n";
    }
    cout << "\n";
    return mst;
}

/*
Kruskal comments (warehouse context):
- Model depots or hubs as nodes, transport links as weighted edges (cost/distance).
- Sort all potential links by cost, then greedily add the cheapest that doesn’t create a cycle.
- Resulting MST is a low-cost backbone for depot-to-depot connections or linehaul network design.
*/

// -------------------- Prim's Algorithm (MST using adjacency list) --------------------
// Alternative MST algorithm starting from a chosen depot and growing the tree.

vector<Edge> primMST(int numDepots, const vector<vector<pair<int,int>>>& adj) {
    const int INF = 1e9;
    vector<int> key(numDepots, INF);     // best cost edge to connect each vertex
    vector<int> parent(numDepots, -1);   // parent depot in MST
    vector<bool> inMST(numDepots, false);

    // Start from depot 0 (could be main warehouse).
    key[0] = 0;
    // min-heap of (key, vertex)
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
    pq.push({0, 0});

    while (!pq.empty()) {
        int u = pq.top().second;
        pq.pop();
        if (inMST[u]) continue;
        inMST[u] = true;

        for (auto& [v, w] : adj[u]) {
            if (!inMST[v] && w < key[v]) {
                key[v] = w;
                parent[v] = u;
                pq.push({key[v], v});
            }
        }
    }

    vector<Edge> mst;
    int totalCost = 0;
    for (int v = 1; v < numDepots; ++v) {
        if (parent[v] != -1) {
            mst.push_back({parent[v], v, key[v]});
            totalCost += key[v];
        }
    }

    cout << "Prim MST total connection cost = " << totalCost << "\n";
    cout << "Edges in MST (u - v : cost):\n";
    for (auto& e : mst) {
        cout << "  " << e.u << " - " << e.v << " : " << e.cost << "\n";
    }
    cout << "\n";
    return mst;
}

/*
Prim comments (warehouse context):
- Begins from a chosen “central” depot, such as the main DC.
- Gradually adds the cheapest edge that expands the current connected region.
- Also builds a low-cost shipping network, useful when adjacency lists are already known.
*/

// -------------------- Sorting for Batch Ordering --------------------
// Orders can be batched by priority, due time, route region, etc.

struct Order {
    int id;
    int depotId;
    int priority;   // smaller = higher priority
    int dueTime;    // time index when order should be delivered
};

void demoBatchSorting() {
    vector<Order> orders = {
        {101, 0, 2,  15},
        {102, 1, 1,  10},
        {103, 0, 3,  20},
        {104, 2, 1,  8}
    };

    // Example: sort by priority then dueTime.
    sort(orders.begin(), orders.end(), [](const Order& a, const Order& b) {
        if (a.priority != b.priority) return a.priority < b.priority;
        return a.dueTime < b.dueTime;
    });

    cout << "Orders sorted for batch creation (priority, then dueTime):\n";
    for (const auto& o : orders) {
        cout << "  Order " << o.id
             << " depot " << o.depotId
             << " priority " << o.priority
             << " dueTime " << o.dueTime << "\n";
    }
    cout << "\n";
}

/*
Sorting comments (warehouse context):
- Orders are sorted by business rules (priority, delivery deadline, region).
- Sorted lists drive how orders are grouped into batches and in what sequence trucks are loaded.
- Different comparison functions allow flexible batching strategies (e.g., zone-based, time-based).
*/

// -------------------- Lists for Route Sequences --------------------
// Represent the sequence of stops a truck makes when serving a batch.

void demoRouteLists() {
    // Using std::list to model a route sequence of customer stops.
    list<int> route; // each int is a customer location ID in visiting order

    // Build an initial route: DC (0) -> A (1) -> B (2) -> C (3) -> back to DC (0)
    route.push_back(0);
    route.push_back(1);
    route.push_back(2);
    route.push_back(3);
    route.push_back(0);

    cout << "Initial route sequence of stops:\n  ";
    for (int stop : route) cout << stop << " ";
    cout << "\n";

    // Suppose a new urgent customer (4) needs to be inserted after stop 1.
    auto it = route.begin();
    advance(it, 2); // move iterator to position after stop 1 (0,1,here,2,...)
    route.insert(it, 4);

    cout << "Route after inserting urgent stop 4 after depot 1:\n  ";
    for (int stop : route) cout << stop << " ";
    cout << "\n\n";
}

/*
List comments (warehouse context):
- Represents an ordered route a vehicle follows through depots or customer locations.
- std::list allows insertion/removal of stops in the middle of the route without reallocation.
- Useful when routes are adjusted on the fly (adding urgent deliveries or removing canceled ones).
*/

// -------------------- Main: Small Warehouse Network Example --------------------
int main() {
    int numDepots = 5; // depots 0..4

    // Example edges between depots with “cost” (distance / expense).
    vector<Edge> edges = {
        {0, 1, 4},
        {0, 2, 2},
        {1, 2, 1},
        {1, 3, 7},
        {2, 3, 3},
        {2, 4, 6},
        {3, 4, 2}
    };

    cout << "=== Kruskal-based depot network (MST) ===\n";
    vector<Edge> mstKruskal = kruskalMST(numDepots, edges);

    // Build adjacency list for Prim from the same edges.
    vector<vector<pair<int,int>>> adj(numDepots);
    for (const auto& e : edges) {
        adj[e.u].push_back({e.v, e.cost});
        adj[e.v].push_back({e.u, e.cost});
    }

    cout << "=== Prim-based depot network (MST) ===\n";
    vector<Edge> mstPrim = primMST(numDepots, adj);

    cout << "=== Order batch sorting ===\n";
    demoBatchSorting();

    cout << "=== Route sequence using list ===\n";
    demoRouteLists();

    return 0;
}
