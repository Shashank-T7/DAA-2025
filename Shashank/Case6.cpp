
// Predictive Maintenance & Alerting Demo
// Purpose: Predict equipment failures and trigger early alerts.
//
// Data Structures / Algorithms:
// - Segment Tree: sensor window queries (e.g., max vibration in last N readings)
// - Fenwick Tree (BIT): incremental updates / prefix sums of readings or events
// - Sparse Table: historical RMQ (Range Minimum/Maximum Query) with static data
// - Heap (priority_queue): high-priority alerts (risk level)
// - Queue: maintenance task buffer
// - Hashing (unordered_map): asset -> sensor list / mapping
// - Arrays/Structures: machine metadata and sensor series

#include <bits/stdc++.h>
using namespace std;

// -------------------- Basic structures --------------------

struct Machine {
    int id;
    string name;
    string location;
};

struct SensorReading {
    int timeIndex;   // discrete time index
    double value;    // reading (e.g., temperature, vibration)
};

struct Alert {
    int machineId;
    string message;
    int severity;    // higher = more severe
    int timeIndex;
};

// =================================================================
// SEGMENT TREE: sensor window queries (max / min / sum)
// Here we implement a max segment tree over a sensor time-series.
// =================================================================
class SegmentTree {
public:
    int n;
    vector<double> tree; // store max in each segment

    SegmentTree(int size = 0) { init(size); }

    void init(int size) {
        n = size;
        tree.assign(4 * n, 0.0);
    }

    void build(const vector<double>& a, int v, int tl, int tr) {
        if (tl == tr) {
            tree[v] = a[tl];
        } else {
            int tm = (tl + tr) / 2;
            build(a, v * 2, tl, tm);
            build(a, v * 2 + 1, tm + 1, tr);
            tree[v] = max(tree[v * 2], tree[v * 2 + 1]);
        }
    }

    void build(const vector<double>& a) {
        if (n > 0) build(a, 1, 0, n - 1);
    }

    // point update: update index idx to new value val
    void update(int v, int tl, int tr, int idx, double val) {
        if (tl == tr) {
            tree[v] = val;
        } else {
            int tm = (tl + tr) / 2;
            if (idx <= tm)
                update(v * 2, tl, tm, idx, val);
            else
                update(v * 2 + 1, tm + 1, tr, idx, val);
            tree[v] = max(tree[v * 2], tree[v * 2 + 1]);
        }
    }

    void update(int idx, double val) {
        update(1, 0, n - 1, idx, val);
    }

    // query max in [l, r]
    double query(int v, int tl, int tr, int l, int r) {
        if (l > r) return -1e18;
        if (l == tl && r == tr) return tree[v];
        int tm = (tl + tr) / 2;
        return max(
            query(v * 2, tl, tm, l, min(r, tm)),
            query(v * 2 + 1, tm + 1, tr, max(l, tm + 1), r)
        );
    }

    double query(int l, int r) {
        return query(1, 0, n - 1, l, r);
    }
};

/*
Segment Tree comments (predictive-maintenance context):
- Maintains rolling sensor readings (e.g., vibration, temperature).
- Supports fast queries like “max vibration in the last 100 samples” for anomaly checks.
- Point updates handle new incoming samples in near real time.
*/

// =================================================================
// FENWICK TREE (BIT): incremental updates / prefix sums
// Example: count how many times a reading exceeded a threshold up to each time.
// =================================================================
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
Fenwick Tree comments (predictive-maintenance context):
- Tracks cumulative counts or sums, e.g., number of threshold breaches over time.
- Each new reading updates a few BIT cells in O(log n).
- Range queries reveal how frequently a sensor has been abnormal within a time window.
*/

// =================================================================
// SPARSE TABLE: historical RMQ (Range Min/Max Query) on static data
// Once built (offline), queries are O(1). Good for long-term history.
// Here we store min values (could store max similarly).
// =================================================================
class SparseTable {
public:
    int n, K;
    vector<vector<double>> st; // st[k][i] => min on [i, i+2^k-1]
    vector<int> log2v;

    SparseTable() : n(0), K(0) {}

    void build(const vector<double>& a) {
        n = (int)a.size();
        K = 32 - __builtin_clz(n);
        st.assign(K, vector<double>(n));
        log2v.assign(n + 1, 0);
        for (int i = 2; i <= n; ++i)
            log2v[i] = log2v[i / 2] + 1;

        for (int i = 0; i < n; ++i)
            st[0][i] = a[i];

        for (int k = 1; k < K; ++k) {
            for (int i = 0; i + (1 << k) <= n; ++i) {
                st[k][i] = min(st[k - 1][i], st[k - 1][i + (1 << (k - 1))]);
            }
        }
    }

    // Min on [l, r]
    double rangeMin(int l, int r) const {
        int len = r - l + 1;
        int k = log2v[len];
        return min(st[k][l], st[k][r - (1 << k) + 1]);
    }
};

/*
Sparse Table comments (predictive-maintenance context):
- Built on historical sensor data snapshots (e.g., last 30 days).
- After building, answers queries like “minimum temperature during shift X” in O(1).
- Ideal when historical data is mostly read-only and frequently queried for trends/thresholds.
*/

// =================================================================
// HEAP: high-priority alerts (max-heap by severity)
// =================================================================
struct AlertHeapItem {
    Alert alert;
    bool operator<(const AlertHeapItem& other) const {
        if (alert.severity != other.alert.severity)
            return alert.severity < other.alert.severity; // higher severity first
        return alert.timeIndex > other.alert.timeIndex;   // earlier alert first
    }
};

void demoAlertHeap() {
    priority_queue<AlertHeapItem> pq;

    pq.push({{1, "High vibration",  9, 100}});
    pq.push({{2, "Overheat",       10, 102}});
    pq.push({{1, "Mild anomaly",    3, 101}});

    cout << "Processing alerts in priority order:\n";
    while (!pq.empty()) {
        auto ah = pq.top(); pq.pop();
        cout << "  Machine " << ah.alert.machineId
             << " severity=" << ah.alert.severity
             << " msg=" << ah.alert.message << "\n";
    }
    cout << "\n";
}

/*
Heap comments (predictive-maintenance context):
- Orders alerts by severity and recency so operators see the most critical issues first.
- Useful when many machines continuously raise events, but human attention is limited.
- Can also be used to feed an automated escalation or paging system.
*/

// =================================================================
// QUEUE: maintenance task buffer
// =================================================================
struct Task {
    int id;
    int machineId;
    string description;
};

void demoTaskQueue() {
    queue<Task> tasks;

    tasks.push({1001, 1, "Inspect bearing on motor A"});
    tasks.push({1002, 2, "Check coolant level on pump B"});
    tasks.push({1003, 1, "Tighten loose coupling on motor A"});

    cout << "Maintenance task queue (FIFO execution):\n";
    while (!tasks.empty()) {
        Task t = tasks.front(); tasks.pop();
        cout << "  Task " << t.id << " for machine " << t.machineId
             << " : " << t.description << "\n";
    }
    cout << "\n";
}

/*
Queue comments (predictive-maintenance context):
- Holds work orders generated by alerts or periodic inspections.
- Tasks are typically executed in FIFO or scheduled priority order by a planner.
- Simple structure for routing tasks to technicians or CMMS systems.
*/

// =================================================================
// HASHING: asset → sensor mapping
// =================================================================
void demoAssetSensorMap() {
    // assetId -> list of sensor IDs
    unordered_map<int, vector<int>> assetToSensors;

    assetToSensors[1] = {10, 11};   // machine 1 has sensors 10,11
    assetToSensors[2] = {20};      // machine 2 has sensor 20
    assetToSensors[3] = {30, 31, 32};

    cout << "Asset to sensor mapping (hash map):\n";
    for (auto& p : assetToSensors) {
        cout << "  Machine " << p.first << " sensors: ";
        for (int sid : p.second) cout << sid << " ";
        cout << "\n";
    }

    int queryAsset = 3;
    auto it = assetToSensors.find(queryAsset);
    if (it != assetToSensors.end()) {
        cout << "Lookup: Machine " << queryAsset << " has sensors: ";
        for (int sid : it->second) cout << sid << " ";
        cout << "\n\n";
    } else {
        cout << "Lookup: Machine " << queryAsset << " not found\n\n";
    }
}

/*
Hashing comments (predictive-maintenance context):
- unordered_map lets the system quickly find which sensors belong to a given asset.
- Used when an alert references a sensor and the system needs to locate its machine, or vice versa.
- Scales well when managing thousands of assets and tens of thousands of sensors.
*/

// =================================================================
// ARRAYS / STRUCTURES: machine metadata and sensor series
// =================================================================
void demoMetadataAndSensors() {
    // Machine metadata
    vector<Machine> machines = {
        {1, "Compressor A", "Line 1"},
        {2, "Pump B",       "Line 2"},
        {3, "Fan C",        "Line 3"}
    };

    cout << "Machine metadata:\n";
    for (const auto& m : machines) {
        cout << "  Machine " << m.id << " : " << m.name
             << " at " << m.location << "\n";
    }
    cout << "\n";

    // Example time-series for one sensor (vibration)
    vector<double> vibration = {0.2, 0.3, 0.5, 0.9, 1.2, 0.8, 0.4, 0.3};

    // Build Segment Tree on this series
    SegmentTree seg((int)vibration.size());
    seg.build(vibration);
    cout << "SegmentTree: max vibration[2..5] = "
         << seg.query(2, 5) << "\n";

    // Build Fenwick: count points above threshold 0.7
    FenwickTree fw((int)vibration.size());
    double th = 0.7;
    for (int i = 0; i < (int)vibration.size(); ++i)
        if (vibration[i] > th) fw.update(i, 1);

    cout << "Fenwick: count of vibration > " << th
         << " in [0..7] = " << fw.rangeSum(0, 7) << "\n";

    // Build Sparse Table for min queries
    SparseTable st;
    st.build(vibration);
    cout << "SparseTable: min vibration[1..4] = "
         << st.rangeMin(1, 4) << "\n\n";
}

/*
Arrays/Structures comments (predictive-maintenance context):
- Vectors/arrays hold machine metadata and sensor readings in a simple, cache-friendly layout.
- Structs like Machine and SensorReading give clear semantics to the stored data.
- Other structures (segment tree, Fenwick, sparse table) are built on top of these base arrays.
*/

// =================================================================
// MAIN: run all demos
// =================================================================
int main() {
    cout << "=== Predictive Maintenance & Alerting Demo ===\n\n";

    demoMetadataAndSensors();
    demoAlertHeap();
    demoTaskQueue();
    demoAssetSensorMap();

    return 0;
}
