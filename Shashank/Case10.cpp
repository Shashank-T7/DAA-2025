
// Traffic Signal Optimization Demo
// Purpose: Adaptive signal timing and corridor-level traffic control.
//
// Algorithms / DS:
// - Segment Tree / Fenwick Tree: live traffic counts on approaches
// - Sparse Table: peak volume/time range queries on historical data
// - Heap: prioritize congested junctions
// - Queue: vehicle queues per approach
// - Hashing: intersection lookup
// - BFS/DFS: corridor grouping of intersections

#include <bits/stdc++.h>
using namespace std;

// -------------------- Core structures --------------------

struct Intersection {
    int id;
    string name;
    double lat, lon;
};

struct ApproachCount {
    int timeIndex;
    int count; // vehicles observed in this interval
};

struct CongestionInfo {
    int intersectionId;
    double delayScore; // larger = more congested
};

// =====================================================================
// Segment Tree: live traffic count window queries (max here)
// =====================================================================

class SegmentTree {
public:
    int n;
    vector<int> tree;

    SegmentTree(int size = 0) { init(size); }

    void init(int size) {
        n = size;
        tree.assign(4 * n, 0);
    }

    void build(const vector<int>& a, int v, int tl, int tr) {
        if (tl == tr) tree[v] = a[tl];
        else {
            int tm = (tl + tr) / 2;
            build(a, v * 2, tl, tm);
            build(a, v * 2 + 1, tm + 1, tr);
            tree[v] = max(tree[v * 2], tree[v * 2 + 1]);
        }
    }

    void build(const vector<int>& a) {
        if (n > 0) build(a, 1, 0, n - 1);
    }

    void update(int v, int tl, int tr, int idx, int val) {
        if (tl == tr) tree[v] = val;
        else {
            int tm = (tl + tr) / 2;
            if (idx <= tm) update(v * 2, tl, tm, idx, val);
            else update(v * 2 + 1, tm + 1, tr, idx, val);
            tree[v] = max(tree[v * 2], tree[v * 2 + 1]);
        }
    }

    void update(int idx, int val) { update(1, 0, n - 1, idx, val); }

    int query(int v, int tl, int tr, int l, int r) {
        if (l > r) return 0;
        if (l == tl && r == tr) return tree[v];
        int tm = (tl + tr) / 2;
        return max(
            query(v * 2, tl, tm, l, min(r, tm)),
            query(v * 2 + 1, tm + 1, tr, max(l, tm + 1), r)
        );
    }

    int query(int l, int r) { return query(1, 0, n - 1, l, r); }
};

/*
Segment Tree comments (traffic context):
- Maintains per-interval vehicle counts for an approach.
- Quickly answers “max flow in the last N intervals” to detect sudden surges.
- Supports online updates as new detector samples are ingested.
*/

// =====================================================================
// Fenwick Tree: incremental sums of vehicle counts
// =====================================================================

class FenwickTree {
public:
    int n;
    vector<int> bit;

    FenwickTree(int size = 0) { init(size); }

    void init(int size) {
        n = size;
        bit.assign(n + 1, 0);
    }

    void update(int idx, int delta) {
        for (int i = idx + 1; i <= n; i += i & -i)
            bit[i] += delta;
    }

    int prefixSum(int idx) {
        int res = 0;
        for (int i = idx + 1; i > 0; i -= i & -i)
            res += bit[i];
        return res;
    }

    int rangeSum(int l, int r) {
        if (l > r) return 0;
        return prefixSum(r) - (l ? prefixSum(l - 1) : 0);
    }
};

/*
Fenwick Tree comments (traffic context):
- Tracks cumulative vehicle counts over time.
- Allows fast updates when each new interval’s count arrives.
- Range sums give total volume in a period, helping compute cycle timings or saturation.
*/

// =====================================================================
// Sparse Table: historical peak ranges (RMQ – max vehicles)
// =====================================================================

class SparseTable {
public:
    int n, K;
    vector<vector<int>> st;
    vector<int> lg;

    void build(const vector<int>& a) {
        n = (int)a.size();
        K = 32 - __builtin_clz(n);
        st.assign(K, vector<int>(n));
        lg.assign(n + 1, 0);
        for (int i = 2; i <= n; ++i)
            lg[i] = lg[i / 2] + 1;

        for (int i = 0; i < n; ++i)
            st[0][i] = a[i];

        for (int k = 1; k < K; ++k)
            for (int i = 0; i + (1 << k) <= n; ++i)
                st[k][i] = max(st[k - 1][i], st[k - 1][i + (1 << (k - 1))]);
    }

    int rangeMax(int l, int r) const {
        int len = r - l + 1;
        int k = lg[len];
        return max(st[k][l], st[k][r - (1 << k) + 1]);
    }
};

/*
Sparse Table comments (traffic context):
- Built on historical per-interval counts to query peak volumes within time windows.
- Answers “maximum flow between 8–9 AM over last week” in O(1) after preprocessing.
- Rocks for mostly read-only archives used in signal plan optimization.
*/

// =====================================================================
// Heap: prioritize congested junctions
// =====================================================================

struct CongHeapItem {
    CongestionInfo info;
    bool operator<(const CongHeapItem& other) const {
        return info.delayScore < other.info.delayScore; // max-heap by delay
    }
};

void demoCongestionHeap() {
    priority_queue<CongHeapItem> pq;
    pq.push({{1,  8.5}});
    pq.push({{2, 15.0}});
    pq.push({{3,  5.0}});

    cout << "Intersections by congestion priority:\n";
    while (!pq.empty()) {
        auto c = pq.top(); pq.pop();
        cout << "  Intersection " << c.info.intersectionId
             << " delayScore=" << c.info.delayScore << "\n";
    }
    cout << "\n";
}

/*
Heap comments (traffic context):
- Ranks intersections by congestion or delay score.
- Controllers or operators can focus tuning or incident response on the worst locations first.
- Also useful for selecting a corridor seed intersection to retime during peaks.
*/

// =====================================================================
// Queue: vehicle queues per approach
// =====================================================================

struct VehicleArrival {
    string plate;
    int timeIndex;
};

void demoVehicleQueues() {
    queue<VehicleArrival> northQueue;
    northQueue.push({"KA01AB1234", 10});
    northQueue.push({"KA02XY9999", 11});

    cout << "North approach vehicle queue (FIFO):\n";
    while (!northQueue.empty()) {
        auto v = northQueue.front(); northQueue.pop();
        cout << "  Vehicle " << v.plate << " arrived at t=" << v.timeIndex << "\n";
    }
    cout << "\n";
}

/*
Queue comments (traffic context):
- Models the physical queue of vehicles waiting at a red light.
- New arrivals join the back; departures leave from the front when there is green.
- Queue length and delay estimates feed back into timing optimization.
*/

// =====================================================================
// Hashing: intersection lookup (id/name -> data)
// =====================================================================

void demoIntersectionHash() {
    unordered_map<int, Intersection> idToInt;
    idToInt[1] = {1, "Main&1st", 12.90, 77.60};
    idToInt[2] = {2, "Main&2nd", 12.91, 77.61};

    cout << "Intersection lookup by ID:\n";
    for (auto& p : idToInt) {
        cout << "  " << p.first << " : " << p.second.name << "\n";
    }

    int qid = 2;
    auto it = idToInt.find(qid);
    if (it != idToInt.end())
        cout << "Lookup: ID " << qid << " is " << it->second.name << "\n\n";
    else
        cout << "Lookup: ID " << qid << " not found\n\n";
}

/*
Hashing comments (traffic context):
- unordered_map provides O(1)-average access from intersection ID (or code) to metadata.
- Used by optimization logic to quickly fetch coordinates, lane counts, and controller IDs.
- Also useful for mapping corridor/group IDs to lists of intersections.
*/

// =====================================================================
// BFS/DFS: corridor grouping on a network of intersections
// =====================================================================

void bfsCorridor(int start, const vector<vector<int>>& adj) {
    vector<bool> vis(adj.size(), false);
    queue<int> q;
    q.push(start);
    vis[start] = true;
    cout << "BFS corridor group from intersection " << start << ":\n  ";
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

void dfsCorridorUtil(int u, const vector<vector<int>>& adj, vector<bool>& vis) {
    vis[u] = true;
    cout << u << " ";
    for (int v : adj[u])
        if (!vis[v]) dfsCorridorUtil(v, adj, vis);
}

void dfsCorridor(int start, const vector<vector<int>>& adj) {
    vector<bool> vis(adj.size(), false);
    cout << "DFS corridor group from intersection " << start << ":\n  ";
    dfsCorridorUtil(start, adj, vis);
    cout << "\n\n";
}

/*
BFS/DFS comments (traffic context):
- Treats intersections as nodes and arterial links as edges.
- From a seed junction, BFS/DFS discovers the contiguous corridor or subnetwork to coordinate.
- Supports grouping for green-wave plans and coordinated control strategies.
*/

// =====================================================================
// Demo tying everything together
// =====================================================================

int main() {
    cout << "=== Traffic Signal Optimization Demo ===\n\n";

    // Example historical counts for one approach (8 intervals)
    vector<int> counts = {5, 7, 10, 20, 18, 9, 6, 4};

    SegmentTree seg((int)counts.size());
    seg.build(counts);
    cout << "SegmentTree: max volume[2..5] = "
         << seg.query(2, 5) << " vehicles\n";

    FenwickTree fw((int)counts.size());
    for (int i = 0; i < (int)counts.size(); ++i)
        fw.update(i, counts[i]);
    cout << "Fenwick: total volume[0..7] = "
         << fw.rangeSum(0, 7) << " vehicles\n";

    SparseTable st;
    st.build(counts);
    cout << "SparseTable: peak volume[1..4] = "
         << st.rangeMax(1, 4) << " vehicles\n\n";

    cout << "=== Congestion heap ===\n";
    demoCongestionHeap();

    cout << "=== Vehicle queues ===\n";
    demoVehicleQueues();

    cout << "=== Intersection hash ===\n";
    demoIntersectionHash();

    // Simple corridor graph: intersections 0..4 along an arterial
    int n = 5;
    vector<vector<int>> adj(n);
    adj[0] = {1};
    adj[1] = {0, 2};
    adj[2] = {1, 3};
    adj[3] = {2, 4};
    adj[4] = {3};

    cout << "=== Corridor grouping (BFS/DFS) ===\n";
    bfsCorridor(0, adj);
    dfsCorridor(0, adj);

    return 0;
}
