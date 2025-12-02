#include <bits/stdc++.h>
using namespace std;

// --- Basic graph structures ---
struct Edge {
    int to;
    double weight;
    Edge(int t = 0, double w = 1.0) : to(t), weight(w) {}
};

struct Graph {
    int n;
    vector<vector<Edge>> adj;
    Graph(int n = 0) { init(n); }
    void init(int n_) { n = n_; adj.assign(n, {}); }
    void add_edge(int u, int v, double w = 1.0, bool directed = false) {
        adj[u].emplace_back(v, w);
        if (!directed) adj[v].emplace_back(u, w);
    }
};

// --- Dijkstra: shortest paths from a source (non-negative weights) ---
struct DijkstraResult {
    vector<double> dist;
    vector<int> parent;
    DijkstraResult(int n = 0) : dist(n, numeric_limits<double>::infinity()), parent(n, -1) {}
};

DijkstraResult dijkstra(const Graph &g, int src) {
    int n = g.n;
    DijkstraResult R(n);
    using PDI = pair<double,int>;
    priority_queue<PDI, vector<PDI>, greater<PDI>> pq;
    R.dist[src] = 0;
    pq.push({0, src});
    while (!pq.empty()) {
        auto [d,u] = pq.top(); pq.pop();
        if (d > R.dist[u]) continue;
        for (auto &e : g.adj[u]) {
            int v = e.to;
            double w = e.weight;
            if (R.dist[v] > d + w) {
                R.dist[v] = d + w;
                R.parent[v] = u;
                pq.push({R.dist[v], v});
            }
        }
    }
    return R;
}

// --- Bellman-Ford: handles negative edges, detects negative cycles ---
struct BellmanResult {
    vector<double> dist;
    vector<int> parent;
    bool has_negative_cycle;
    BellmanResult(int n = 0) : dist(n, numeric_limits<double>::infinity()), parent(n, -1), has_negative_cycle(false) {}
};

BellmanResult bellmanFord(const Graph &g, int src) {
    int n = g.n;
    BellmanResult R(n);
    R.dist[src] = 0;
    for (int iter = 0; iter < n - 1; ++iter) {
        bool changed = false;
        for (int u = 0; u < n; ++u) {
            if (R.dist[u] == numeric_limits<double>::infinity()) continue;
            for (auto &e : g.adj[u]) {
                int v = e.to;
                double w = e.weight;
                if (R.dist[v] > R.dist[u] + w) {
                    R.dist[v] = R.dist[u] + w;
                    R.parent[v] = u;
                    changed = true;
                }
            }
        }
        if (!changed) break;
    }
    // check negative cycle
    for (int u = 0; u < n; ++u) {
        if (R.dist[u] == numeric_limits<double>::infinity()) continue;
        for (auto &e : g.adj[u]) {
            int v = e.to;
            double w = e.weight;
            if (R.dist[v] > R.dist[u] + w) {
                R.has_negative_cycle = true;
                return R;
            }
        }
    }
    return R;
}

// --- BFS / DFS for exploring nearby drivers (unweighted neighborhood search) ---
vector<int> bfs_nearby(const Graph &g, int startNode, int maxDepth) {
    vector<int> found;
    vector<int> depth(g.n, -1);
    queue<int> q;
    depth[startNode] = 0;
    q.push(startNode);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        found.push_back(u);
        if (depth[u] >= maxDepth) continue;
        for (auto &e : g.adj[u]) {
            int v = e.to;
            if (depth[v] == -1) {
                depth[v] = depth[u] + 1;
                q.push(v);
            }
        }
    }
    return found;
}

void dfs_collect(const Graph &g, int node, int maxDepth, vector<int> &depth, vector<int> &out) {
    out.push_back(node);
    if (depth[node] >= maxDepth) return;
    for (auto &e : g.adj[node]) {
        int v = e.to;
        if (depth[v] == -1) {
            depth[v] = depth[node] + 1;
            dfs_collect(g, v, maxDepth, depth, out);
        }
    }
}

vector<int> dfs_nearby(const Graph &g, int startNode, int maxDepth) {
    vector<int> depth(g.n, -1);
    vector<int> out;
    depth[startNode] = 0;
    dfs_collect(g, startNode, maxDepth, depth, out);
    return out;
}

// --- Basic entities: Driver, Rider, Request ---
struct Driver {
    int id;
    int node; // location node in the graph
    double rating; // 0..5
    bool available;
    double speed; // relative speed factor (km/h etc.)
    Driver(int i=0,int n=0,double r=5.0,bool a=true,double s=1.0) : id(i), node(n), rating(r), available(a), speed(s) {}
};

struct Rider {
    int id;
    int node; // pickup node
    int dest; // destination node
    Rider(int i=0,int n=0,int d=0) : id(i), node(n), dest(d) {}
};

struct Request {
    int request_id;
    int rider_id;
    int pickup_node;
    int destination_node;
    Request(int rid=0,int r=0,int p=0,int d=0) : request_id(rid), rider_id(r), pickup_node(p), destination_node(d) {}
};

// --- Dispatcher with hashing, queue, heap priority ---
struct Candidate {
    int driver_id;
    double eta; // estimated time to pickup (lower better)
    double score; // can incorporate rating, surge, distance
    Candidate(int did=0,double e=0,double s=0) : driver_id(did), eta(e), score(s) {}
};

struct CandidateCmp {
    bool operator()(Candidate const& a, Candidate const& b) const {
        if (a.eta != b.eta) return a.eta > b.eta;
        return a.score < b.score;
    }
};

class Dispatcher {
public:
    Dispatcher(Graph &g) : G(g), next_request_id(1) {}

    void add_driver(const Driver &d) {
        drivers[d.id] = d;
        drivers_by_node[d.node].push_back(d.id);
    }

    void update_driver_location(int driver_id, int new_node) {
        auto it = drivers.find(driver_id);
        if (it == drivers.end()) return;
        int old_node = it->second.node;
        it->second.node = new_node;
        // rebuild node lists lazily; for simplicity remove from old list and add to new
        auto &vecOld = drivers_by_node[old_node];
        vecOld.erase(remove(vecOld.begin(), vecOld.end(), driver_id), vecOld.end());
        drivers_by_node[new_node].push_back(driver_id);
    }

    void set_driver_availability(int driver_id, bool available) {
        if (drivers.count(driver_id)) drivers[driver_id].available = available;
    }

    void add_rider(const Rider &r) {
        riders[r.id] = r;
    }

    int enqueue_request(int rider_id) {
        if (!riders.count(rider_id)) return -1;
        Request req(next_request_id++, rider_id, riders[rider_id].node, riders[r.id].dest);
        requests.push(req);
        return req.request_id;
    }

    // Immediately try to match top request (FIFO) using multi-step approach:
    // 1) find nearby candidates via BFS within depth limit
    // 2) compute ETA (Dijkstra) for each candidate
    // 3) push to heap, pick best
    bool process_next_request(int bfsDepth = 4) {
        if (requests.empty()) return false;
        Request req = requests.front(); requests.pop();

        if (!riders.count(req.rider_id)) {
            cout << "Rider " << req.rider_id << " not found.\n";
            return false;
        }

        // 1) find nodes within bfsDepth
        vector<int> nearbyNodes = bfs_nearby(G, req.pickup_node, bfsDepth);

        // 2) gather driver candidates who are available
        vector<int> candidate_driver_ids;
        unordered_set<int> seenDrivers;
        for (int node : nearbyNodes) {
            auto it = drivers_by_node.find(node);
            if (it == drivers_by_node.end()) continue;
            for (int did : it->second) {
                if (drivers.count(did) && drivers[did].available) {
                    if (!seenDrivers.count(did)) {
                        candidate_driver_ids.push_back(did);
                        seenDrivers.insert(did);
                    }
                }
            }
        }

        if (candidate_driver_ids.empty()) {
            cout << "No available drivers within BFS depth " << bfsDepth << " for request " << req.request_id << "\n";
            return false;
        }

        // 3) For each candidate compute ETA using Dijkstra from driver node to pickup node
        // To avoid running Dijkstra many times, we could run multi-source; here we run Dijkstra per driver for clarity.
        priority_queue<Candidate, vector<Candidate>, CandidateCmp> pq;
        for (int did : candidate_driver_ids) {
            Driver &drv = drivers[did];
            DijkstraResult dr = dijkstra(G, drv.node);
            double distance = dr.dist[req.pickup_node];
            if (distance == numeric_limits<double>::infinity()) continue;
            // simple ETA model: time = distance / speed (we assume graph weight is distance)
            double eta = distance / max(0.0001, drv.speed);
            double score = drv.rating; // higher rating improves tie-break
            pq.push(Candidate(did, eta, score));
        }

        if (pq.empty()) {
            cout << "No reachable drivers for request " << req.request_id << "\n";
            return false;
        }

        // 4) pick the best candidate
        Candidate best = pq.top(); pq.pop();
        assign_driver_to_request(best.driver_id, req);

        return true;
    }

    // A more advanced matching: run single Dijkstra from pickup node to get distances to all nodes,
    // then evaluate drivers by distance/ETA using driver speeds. This is better if many drivers.
    bool process_next_request_single_source(int bfsDepth = 8) {
        if (requests.empty()) return false;
        Request req = requests.front(); requests.pop();

        DijkstraResult fromPickup = dijkstra(G, req.pickup_node);

        // find all available drivers within node distance limit (by hops or by graph distance)
        vector<int> candidate_driver_ids;
        for (auto &p : drivers) {
            if (!p.second.available) continue;
            int did = p.first;
            int dnode = p.second.node;
            double dist = fromPickup.dist[dnode]; // distance from pickup to driver (reverse but same undirected distances)
            if (dist == numeric_limits<double>::infinity()) continue;
            // consider driver if within some distance threshold (derived from bfsDepth)
            if (dist <= bfsDepth * 1.0) candidate_driver_ids.push_back(did);
        }

        if (candidate_driver_ids.empty()) {
            cout << "No available drivers near pickup for request " << req.request_id << "\n";
            return false;
        }

        priority_queue<Candidate, vector<Candidate>, CandidateCmp> pq;
        for (int did : candidate_driver_ids) {
            Driver &drv = drivers[did];
            double dist = fromPickup.dist[drv.node];
            if (dist == numeric_limits<double>::infinity()) continue;
            double eta = dist / max(0.0001, drv.speed);
            double score = drv.rating;
            pq.push(Candidate(did, eta, score));
        }

        if (pq.empty()) {
            cout << "No reachable drivers for request " << req.request_id << "\n";
            return false;
        }

        Candidate best = pq.top(); pq.pop();
        assign_driver_to_request(best.driver_id, req);
        return true;
    }

    // Simple assignment: mark driver unavailable, print summary
    void assign_driver_to_request(int driver_id, const Request &req) {
        if (!drivers.count(driver_id)) {
            cout << "Driver " << driver_id << " missing\n";
            return;
        }
        Driver &drv = drivers[driver_id];
        drv.available = false;
        cout << "Assigned Driver " << driver_id << " (node " << drv.node << ") to Request " << req.request_id
             << " (Rider " << req.rider_id << " pickup node " << req.pickup_node << ")\n";
    }

    // utilities
    void print_driver_status() const {
        cout << "Drivers:\n";
        for (auto &p : drivers) {
            const Driver &d = p.second;
            cout << "  id=" << d.id << ", node=" << d.node << ", rating=" << d.rating << ", avail=" << (d.available? "Y":"N") << ", speed=" << d.speed << '\n';
        }
    }

    void print_requests() const {
        queue<Request> temp = requests;
        cout << "Pending requests:\n";
        while (!temp.empty()) {
            auto r = temp.front(); temp.pop();
            cout << "  req=" << r.request_id << " rider=" << r.rider_id << " pickup=" << r.pickup_node << '\n';
        }
    }

private:
    Graph &G;
    unordered_map<int, Driver> drivers;
    unordered_map<int, Rider> riders;
    unordered_map<int, vector<int>> drivers_by_node;
    queue<Request> requests;
    int next_request_id;
};

// --- Demo / Simulation main ---
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Build a demo road graph (nodes are intersections)
    // We'll create a small grid like graph (5x5)
    int rows = 5, cols = 5;
    int N = rows * cols;
    Graph G(N);
    auto nodeId = [&](int r, int c) { return r * cols + c; };
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int u = nodeId(r, c);
            if (r + 1 < rows) G.add_edge(u, nodeId(r+1, c), 1.0);
            if (c + 1 < cols) G.add_edge(u, nodeId(r, c+1), 1.0);
        }
    }

    // Place drivers and riders
    Dispatcher D(G);
    D.add_driver(Driver(1, nodeId(0,0), 4.8, true, 1.0));
    D.add_driver(Driver(2, nodeId(1,2), 4.2, true, 1.2));
    D.add_driver(Driver(3, nodeId(4,4), 3.9, true, 0.9));
    D.add_driver(Driver(4, nodeId(2,1), 4.9, false, 1.1)); // busy

    D.add_rider(Rider(101, nodeId(0,1), nodeId(4,0)));
    D.add_rider(Rider(102, nodeId(2,2), nodeId(0,4)));
    D.add_rider(Rider(103, nodeId(3,0), nodeId(1,4)));

    // Create requests (enqueued in FIFO order)
    D.enqueue_request(101);
    D.enqueue_request(102);
    D.enqueue_request(103);

    cout << "Initial driver status:\n";
    D.print_driver_status();
    cout << '\n';

    // Process requests one by one using single-source optimization
    while (true) {
        D.print_requests();
        bool ok = D.process_next_request_single_source(6);
        if (!ok) break;
        cout << '\n';
    }

    cout << "Final driver status:\n";
    D.print_driver_status();

    // Demo: show Dijkstra from nodeId(0,1)
    cout << "\nDijkstra distances from rider at (0,1):\n";
    DijkstraResult dr = dijkstra(G, nodeId(0,1));
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int u = nodeId(r,c);
            double dist = dr.dist[u];
            if (dist == numeric_limits<double>::infinity()) cout << "inf ";
            else cout << (int)dist << ' ';
        }
        cout << '\n';
    }

    // Bellman-Ford demo with an added negative edge (illustrative)
    Graph G2 = G;
    // introduce a negative weight edge between two nodes to show detection
    G2.add_edge(0, 1, -5.0, true);
    BellmanResult br = bellmanFord(G2, 0);
    if (br.has_negative_cycle) cout << "\nBellman-Ford detected a negative cycle in G2.\n";
    else cout << "\nBellman-Ford did not detect a negative cycle in G2.\n";

    return 0;
}
