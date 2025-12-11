#include <bits/stdc++.h>
using namespace std;
using ll = long long;

struct Waypoint {
    int id;
    double x;
    double y;
    double wind;
    int importance;
    Waypoint(): id(0), x(0), y(0), wind(0), importance(0) {}
};

static inline double euclid(const Waypoint &a, const Waypoint &b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx*dx + dy*dy);
}

static inline vector<string> split_csv_line(const string &s) {
    vector<string> out;
    string cur;
    bool inq = false;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (inq) {
            if (c == '"') {
                if (i + 1 < s.size() && s[i+1] == '"') {
                    cur.push_back('"');
                    ++i;
                } else {
                    inq = false;
                }
            } else {
                cur.push_back(c);
            }
        } else {
            if (c == ',') {
                out.push_back(cur);
                cur.clear();
            } else if (c == '"') {
                inq = true;
            } else {
                cur.push_back(c);
            }
        }
    }
    out.push_back(cur);
    for (auto &t : out) {
        size_t a = 0, b = t.size();
        while (a < b && isspace((unsigned char)t[a])) ++a;
        while (b > a && isspace((unsigned char)t[b-1])) --b;
        t = t.substr(a, b - a);
    }
    return out;
}

static inline int to_int(const string &s) {
    if (s.empty()) return 0;
    try { return stoi(s); } catch (...) { return 0; }
}

static inline double to_double(const string &s) {
    if (s.empty()) return 0.0;
    try { return stod(s); } catch (...) { return 0.0; }
}

struct NN_TSP {
    static vector<int> nearest_neighbor(const vector<Waypoint> &wps, int start_idx = 0) {
        int n = (int)wps.size();
        if (n == 0) return {};
        vector<int> tour;
        tour.reserve(n);
        vector<char> used(n, 0);
        int cur = start_idx;
        used[cur] = 1;
        tour.push_back(cur);
        for (int step = 1; step < n; ++step) {
            double best = numeric_limits<double>::infinity();
            int bi = -1;
            for (int j = 0; j < n; ++j) if (!used[j]) {
                double d = euclid(wps[cur], wps[j]) * (1.0 + wps[j].wind / 30.0);
                if (d < best) { best = d; bi = j; }
            }
            if (bi == -1) break;
            used[bi] = 1;
            tour.push_back(bi);
            cur = bi;
        }
        return tour;
    }

    static double tour_length(const vector<int> &tour, const vector<Waypoint> &wps) {
        double s = 0.0;
        int m = (int)tour.size();
        if (m == 0) return 0.0;
        for (int i = 0; i + 1 < m; ++i) {
            int a = tour[i], b = tour[i+1];
            s += euclid(wps[a], wps[b]) * (1.0 + wps[b].wind / 30.0);
        }
        if (m >= 2) {
            int a = tour.back(), b = tour.front();
            s += euclid(wps[a], wps[b]) * (1.0 + wps[b].wind / 30.0);
        }
        return s;
    }

    static void two_opt(vector<int> &tour, const vector<Waypoint> &wps) {
        int n = (int)tour.size();
        if (n < 4) return;
        bool improved = true;
        while (improved) {
            improved = false;
            for (int i = 0; i < n - 1 && !improved; ++i) {
                for (int k = i + 2; k < n && !improved; ++k) {
                    int a = tour[i];
                    int b = tour[(i + 1) % n];
                    int c = tour[k % n];
                    int d = tour[(k + 1) % n];
                    double before = euclid(wps[a], wps[b]) * (1.0 + wps[b].wind / 30.0)
                                  + euclid(wps[c], wps[d]) * (1.0 + wps[d].wind / 30.0);
                    double after  = euclid(wps[a], wps[c]) * (1.0 + wps[c].wind / 30.0)
                                  + euclid(wps[b], wps[d]) * (1.0 + wps[d].wind / 30.0);
                    if (after + 1e-12 < before) {
                        reverse(tour.begin() + i + 1, tour.begin() + k + 1);
                        improved = true;
                    }
                }
            }
        }
    }

    static vector<vector<int>> split_by_battery(const vector<int> &tour, const vector<Waypoint> &wps, double max_distance) {
        vector<vector<int>> missions;
        if (tour.empty()) return missions;
        vector<int> cur;
        double accumulated = 0.0;
        for (size_t i = 0; i < tour.size(); ++i) {
            int id = tour[i];
            double leg = 0.0;
            if (!cur.empty()) {
                int prev = cur.back();
                leg = euclid(wps[prev], wps[id]) * (1.0 + wps[id].wind / 30.0);
            } else {
                leg = 0.0;
            }
            if (!cur.empty() && accumulated + leg > max_distance) {
                missions.push_back(cur);
                cur.clear();
                accumulated = 0.0;
            }
            cur.push_back(id);
            accumulated += leg;
        }
        if (!cur.empty()) missions.push_back(cur);
        return missions;
    }
};

struct GraphDijkstra {
    const vector<Waypoint> *wps;
    int n;
    GraphDijkstra(const vector<Waypoint> *wps_ptr = nullptr) {
        wps = wps_ptr;
        n = wps ? (int)wps->size() : 0;
    }
    vector<double> run(int src) {
        int N = n;
        vector<double> dist(N, numeric_limits<double>::infinity());
        if (src < 0 || src >= N) return dist;
        struct Item { double d; int v; };
        struct Cmp { bool operator()(const Item &a, const Item &b) const { return a.d > b.d; } };
        priority_queue<Item, vector<Item>, Cmp> pq;
        dist[src] = 0.0;
        pq.push({0.0, src});
        while (!pq.empty()) {
            Item it = pq.top(); pq.pop();
            double d = it.d;
            int u = it.v;
            if (d > dist[u] + 1e-12) continue;
            for (int v = 0; v < N; ++v) {
                if (v == u) continue;
                double w = euclid((*wps)[u], (*wps)[v]) * (1.0 + (*wps)[v].wind / 30.0);
                double nd = dist[u] + w;
                if (nd + 1e-12 < dist[v]) {
                    dist[v] = nd;
                    pq.push({nd, v});
                }
            }
        }
        return dist;
    }
};

static inline void print_tour_summary(const vector<int> &tour, const vector<Waypoint> &wps) {
    double len = NN_TSP::tour_length(tour, wps);
    cout << "Tour nodes: " << tour.size() << " length: " << fixed << setprecision(6) << len << "\n";
}

static inline void print_missions(const vector<vector<int>> &missions, const vector<Waypoint> &wps) {
    cout << "Missions: " << missions.size() << "\n";
    for (size_t i = 0; i < missions.size(); ++i) {
        cout << "Mission " << i << " size " << missions[i].size();
        double mlen = 0.0;
        for (size_t j = 1; j < missions[i].size(); ++j) {
            int a = missions[i][j-1], b = missions[i][j];
            mlen += euclid(wps[a], wps[b]) * (1.0 + wps[b].wind / 30.0);
        }
        cout << " est_len " << mlen << "\n";
    }
}

static inline void interactive_loop(vector<Waypoint> &wps, vector<int> &tour, vector<vector<int>> &missions, int base_index) {
    cout << "Interactive commands:\n";
    cout << "  RUN_TSP [start_id]\n";
    cout << "  TWO_OPT\n";
    cout << "  SPLIT max_distance\n";
    cout << "  EMERGENCY node_id  (compute shortest path back to base)\n";
    cout << "  DIJKSTRA src dst\n";
    cout << "  PRINT_TOUR\n";
    cout << "  PRINT_MISSIONS\n";
    cout << "  QUIT\n";
    string line;
    GraphDijkstra gdi(&wps);
    while (true) {
        if (!getline(cin, line)) break;
        if (line.find_first_not_of(" \t\r\n") == string::npos) continue;
        stringstream ss(line);
        string cmd;
        ss >> cmd;
        for (auto &c : cmd) c = toupper((unsigned char)c);
        if (cmd == "QUIT") break;
        else if (cmd == "RUN_TSP") {
            int start = 0;
            if (ss >> start) {
                if (start < 0 || start >= (int)wps.size()) {
                    cout << "Invalid start, using 0\n";
                    start = 0;
                }
            }
            tour = NN_TSP::nearest_neighbor(wps, start);
            cout << "Computed NN tour\n";
            print_tour_summary(tour, wps);
        }
        else if (cmd == "TWO_OPT") {
            NN_TSP::two_opt(tour, wps);
            cout << "Applied 2-opt\n";
            print_tour_summary(tour, wps);
        }
        else if (cmd == "SPLIT") {
            double maxd;
            if (!(ss >> maxd)) {
                cout << "Usage: SPLIT max_distance\n";
                continue;
            }
            missions = NN_TSP::split_by_battery(tour, wps, maxd);
            cout << "Split into " << missions.size() << " missions for max_distance " << maxd << "\n";
        }
        else if (cmd == "EMERGENCY") {
            int nid;
            if (!(ss >> nid)) {
                cout << "Usage: EMERGENCY node_id\n";
                continue;
            }
            if (nid < 0 || nid >= (int)wps.size()) {
                cout << "Invalid node_id\n";
                continue;
            }
            vector<double> dist = gdi.run(nid);
            if (base_index >= 0 && base_index < (int)wps.size()) {
                double d = dist[base_index];
                if (!isfinite(d)) cout << "No path to base\n"; else cout << "Emergency distance from " << nid << " to base(" << base_index << ") = " << d << "\n";
            } else {
                cout << "Base not defined\n";
            }
        }
        else if (cmd == "DIJKSTRA") {
            int a, b;
            if (!(ss >> a >> b)) {
                cout << "Usage: DIJKSTRA src dst\n";
                continue;
            }
            if (a < 0 || a >= (int)wps.size() || b < 0 || b >= (int)wps.size()) {
                cout << "Invalid nodes\n"; continue;
            }
            vector<double> dist = gdi.run(a);
            double d = dist[b];
            if (!isfinite(d)) cout << "No path\n"; else cout << "Distance approx " << d << "\n";
        }
        else if (cmd == "PRINT_TOUR") {
            print_tour_summary(tour, wps);
        }
        else if (cmd == "PRINT_MISSIONS") {
            print_missions(missions, wps);
        }
        else {
            cout << "Unknown command\n";
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<string> raw;
    string line;
    while (getline(cin, line)) raw.push_back(line);
    if (raw.empty()) return 0;

    int first = 0;
    while (first < (int)raw.size()) {
        if (raw[first].find(',') != string::npos) break;
        ++first;
    }
    if (first >= (int)raw.size()) return 0;

    vector<string> header = split_csv_line(raw[first]);
    unordered_map<string,int> hmap;
    for (int i = 0; i < (int)header.size(); ++i) {
        string key = header[i];
        for (auto &c : key) c = tolower((unsigned char)c);
        hmap[key] = i;
    }

    vector<Waypoint> wps;
    wps.reserve(6000);
    int base_index = -1;
    for (int i = first + 1; i < (int)raw.size(); ++i) {
        string &row = raw[i];
        if (row.find_first_not_of(" \t\r\n") == string::npos) continue;
        vector<string> fields = split_csv_line(row);
        string cmd;
        if (hmap.count("command")) cmd = fields[hmap["command"]];
        if (cmd.empty()) {
            if (hmap.count("node_id") && hmap.count("x") && hmap.count("y")) cmd = "WAYPOINT";
        }
        string up = cmd;
        for (auto &c : up) c = toupper((unsigned char)c);
        if (up == "BASE") {
            Waypoint b;
            b.id = to_int(fields[hmap.count("node_id")?hmap["node_id"]:1]);
            b.x = to_double(fields[hmap.count("x")?hmap["x"]:2]);
            b.y = to_double(fields[hmap.count("y")?hmap["y"]:3]);
            b.wind = to_double(fields[hmap.count("wind")?hmap["wind"]:4]);
            b.importance = to_int(fields[hmap.count("importance")?hmap["importance"]:5]);
            base_index = (int)wps.size();
            wps.push_back(b);
        } else if (up == "WAYPOINT" || up == "POINT" || up == "NODE") {
            Waypoint p;
            p.id = to_int(fields[hmap.count("node_id")?hmap["node_id"]:1]);
            p.x = to_double(fields[hmap.count("x")?hmap["x"]:2]);
            p.y = to_double(fields[hmap.count("y")?hmap["y"]:3]);
            p.wind = to_double(fields[hmap.count("wind")?hmap["wind"]:4]);
            p.importance = to_int(fields[hmap.count("importance")?hmap["importance"]:5]);
            wps.push_back(p);
        } else {
            // ignore other commands at load time
        }
    }

    if (wps.empty()) {
        cout << "No waypoints loaded\n";
        return 0;
    }

    cout << "Loaded " << wps.size() << " waypoints\n";
    if (base_index < 0) base_index = 0;

    vector<int> tour = NN_TSP::nearest_neighbor(wps, base_index);
    cout << "Initial NN tour computed. Size " << tour.size() << "\n";
    cout << "Tour length before 2-opt: " << fixed << setprecision(6) << NN_TSP::tour_length(tour, wps) << "\n";

    NN_TSP::two_opt(tour, wps);
    cout << "Tour length after 2-opt: " << fixed << setprecision(6) << NN_TSP::tour_length(tour, wps) << "\n";

    double default_battery = 30000.0;
    vector<vector<int>> missions = NN_TSP::split_by_battery(tour, wps, default_battery);
    cout << "Split into " << missions.size() << " missions with default battery " << default_battery << "\n";

    GraphDijkstra gdi(&wps);
    cout << "Computing Dijkstra from base for emergency references (may take a while) ...\n";
    vector<double> base_dists = gdi.run(base_index);
    cout << "Base distances computed. Dist to first waypoint: " << base_dists[0] << "\n";

    interactive_loop(wps, tour, missions, base_index);

    cout << "Exiting\n";
    return 0;
}
