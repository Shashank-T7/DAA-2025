#include <bits/stdc++.h>
using namespace std;

// ============ CONFIG ============
const int MAX_N = 50;
const int INF = 1e9;

// ============ GLOBALS ============
int N;                          // number of hubs/stations
vector<vector<int>> distFW;     // Floyd–Warshall distance matrix

// For timetable: for each directed edge u->v, a sorted list of departures (minutes from 0..1439)
vector<int> departures[MAX_N+1][MAX_N+1];

// ============ FLOYD–WARSHALL ============
void floydWarshall() {
    for (int k = 1; k <= N; ++k) {
        for (int i = 1; i <= N; ++i) {
            if (distFW[i][k] == INF) continue;
            for (int j = 1; j <= N; ++j) {
                if (distFW[k][j] == INF) continue;
                if (distFW[i][j] > distFW[i][k] + distFW[k][j]) {
                    distFW[i][j] = distFW[i][k] + distFW[k][j];
                }
            }
        }
    }
}

// ============ INPUT GRAPH & TIMETABLE ============
void setupNetwork() {
    cout << "Enter number of hubs/stations (<= " << MAX_N << "): ";
    cin >> N;
    if (N < 1 || N > MAX_N) {
        cout << "Invalid, setting N = 5.\n";
        N = 5;
    }

    // init dist matrix
    distFW.assign(N+1, vector<int>(N+1, INF));
    for (int i = 1; i <= N; ++i) distFW[i][i] = 0;

    // clear departures
    for (int i = 1; i <= N; ++i)
        for (int j = 1; j <= N; ++j)
            departures[i][j].clear();

    int m;
    cout << "Enter number of direct routes (edges): ";
    cin >> m;
    cout << "For each route, enter: u v travel_time number_of_departures then that many departure times (minutes 0..1439, sorted or unsorted).\n";
    for (int k = 0; k < m; ++k) {
        int u, v, t, d;
        cin >> u >> v >> t >> d;
        if (u < 1 || u > N || v < 1 || v > N) {
            cout << "Invalid hubs, skipping.\n";
            for (int i = 0; i < d; ++i) { int tmp; cin >> tmp; }
            continue;
        }
        distFW[u][v] = min(distFW[u][v], t); // in-vehicle travel time
        departures[u][v].reserve(departures[u][v].size() + d);
        for (int i = 0; i < d; ++i) {
            int dep;
            cin >> dep;
            departures[u][v].push_back(dep);
        }
    }

    // sort all departure arrays
    for (int i = 1; i <= N; ++i)
        for (int j = 1; j <= N; ++j)
            sort(departures[i][j].begin(), departures[i][j].end());

    floydWarshall();
    cout << "Network and timetable loaded. All-pairs shortest travel times computed.\n";
}

// ============ NEXT DEPARTURE (BINARY SEARCH) ============
int nextDeparture(int u, int v, int currentTime) {
    auto &vec = departures[u][v];
    if (vec.empty()) return -1;
    auto it = lower_bound(vec.begin(), vec.end(), currentTime);
    if (it != vec.end()) return *it;
    // if no later departure, optionally wrap to next day; here we return -1
    return -1;
}

// ============ SIMPLE QUERY: DIRECT OR SINGLE TRANSFER ============
void queryDirectOrOneTransfer() {
    int src, dst, startT;
    cout << "Enter source hub, destination hub, and start time (minutes 0..1439): ";
    cin >> src >> dst >> startT;
    if (src < 1 || src > N || dst < 1 || dst > N) {
        cout << "Invalid hubs.\n";
        return;
    }

    int bestArr = INF;
    string bestPlan;

    // 1) Direct route
    for (int v = 1; v <= N; ++v) {/*dummy loop to avoid unused warnings*/}

    if (!departures[src][dst].empty()) {
        int dep = nextDeparture(src, dst, startT);
        if (dep != -1) {
            int arr = dep + distFW[src][dst]; // using direct edge time (distFW[src][dst] == edge or better path)
            if (arr < bestArr) {
                bestArr = arr;
                bestPlan = "Direct: " + to_string(src) + " -> " + to_string(dst) +
                           " (depart " + to_string(dep) + ", arrive " + to_string(arr) + ")";
            }
        }
    }

    // 2) One-transfer options: src -> k -> dst
    for (int k = 1; k <= N; ++k) {
        if (k == src || k == dst) continue;
        // first leg src->k
        if (departures[src][k].empty()) continue;
        int dep1 = nextDeparture(src, k, startT);
        if (dep1 == -1) continue;
        int arr1 = dep1 + distFW[src][k];

        // second leg k->dst, starting from arr1
        if (departures[k][dst].empty()) continue;
        int dep2 = nextDeparture(k, dst, arr1);
        if (dep2 == -1) continue;
        int arr2 = dep2 + distFW[k][dst];

        if (arr2 < bestArr) {
            bestArr = arr2;
            bestPlan = "Transfer at " + to_string(k) + ": " +
                       to_string(src) + " -> " + to_string(k) +
                       " (depart " + to_string(dep1) + ", arrive " + to_string(arr1) + "), then " +
                       to_string(k) + " -> " + to_string(dst) +
                       " (depart " + to_string(dep2) + ", arrive " + to_string(arr2) + ")";
        }
    }

    if (bestArr == INF) {
        cout << "No feasible route (with <=1 transfer) found from " << src << " to " << dst << ".\n";
    } else {
        cout << "Best found plan (<=1 transfer):\n" << bestPlan << "\n";
    }
}

// ============ SHOW DIST MATRIX ============
void showAllPairsTimes() {
    cout << "All-pairs minimum in-vehicle travel times (INF=1e9 means no path):\n";
    for (int i = 1; i <= N; ++i) {
        for (int j = 1; j <= N; ++j) {
            if (distFW[i][j] >= INF) cout << "INF ";
            else cout << distFW[i][j] << " ";
        }
        cout << "\n";
    }
}

// ============ MENU / MAIN ============
void printMenu() {
    cout << "\n=== Dynamic Public Transport Timetable & Transfer Optimizer ===\n";
    cout << "1. Setup network & timetable (Floyd–Warshall + departures)\n";
    cout << "2. Show all-pairs shortest in-vehicle times\n";
    cout << "3. Query best route with at most one transfer (binary search on departures)\n";
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
            case 1: setupNetwork(); break;
            case 2: showAllPairsTimes(); break;
            case 3: queryDirectOrOneTransfer(); break;
            case 0: cout << "Exiting.\n"; break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != 0);

    return 0;
}
