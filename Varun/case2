#include <bits/stdc++.h>
using namespace std;
using ll = long long;
const ll INFLL = (ll)4e18;

struct Edge {
    int u, v;
    ll travel_time;
    double energy_cost;
    int congestion;
    ll install_cost;
    int capacity;
    double price;
    Edge() {}
};

struct Adj {
    int to;
    ll travel_time;
    double energy_cost;
    int congestion;
    ll install_cost;
    int capacity;
    double price;
    Adj* next;
    Adj(int t, ll tt, double ec, int cg, ll ic, int cap, double pr)
        : to(t), travel_time(tt), energy_cost(ec), congestion(cg),
          install_cost(ic), capacity(cap), price(pr), next(nullptr) {}
};

struct Graph {
    int n;
    vector<Adj*> head;
    vector<Edge> edges;
    Graph(): n(0) {}
    void init(int N) {
        n = N;
        head.assign(n, nullptr);
        edges.clear();
    }
    void addEdge(int u, int v, ll travel_time, double energy_cost, int congestion, ll install_cost, int capacity, double price) {
        if (u < 0 || v < 0 || u >= n || v >= n) return;
        Edge e;
        e.u = u; e.v = v; e.travel_time = travel_time; e.energy_cost = energy_cost;
        e.congestion = congestion; e.install_cost = install_cost; e.capacity = capacity; e.price = price;
        edges.push_back(e);
        Adj* a = new Adj(v, travel_time, energy_cost, congestion, install_cost, capacity, price);
        a->next = head[u]; head[u] = a;
        Adj* b = new Adj(u, travel_time, energy_cost, congestion, install_cost, capacity, price);
        b->next = head[v]; head[v] = b;
    }
    int size() const { return n; }
    void clear() {
        for (auto p : head) {
            while (p) {
                Adj* tmp = p;
                p = p->next;
                delete tmp;
            }
        }
        head.assign(n, nullptr);
        edges.clear();
    }
    ~Graph() { clear(); }
};

struct DSU {
    int n;
    vector<int> p, r;
    DSU(int n_ = 0) { init(n_); }
    void init(int n_) { n = n_; p.resize(n); r.assign(n, 0); iota(p.begin(), p.end(), 0); }
    int find(int x) { return p[x] == x ? x : p[x] = find(p[x]); }
    bool unite(int a, int b) { a = find(a); b = find(b); if (a == b) return false; if (r[a] < r[b]) swap(a, b); p[b] = a; if (r[a] == r[b]) r[a]++; return true; }
};

struct MinHeap {
    vector<pair<long double,int>> a;
    vector<int> pos;
    MinHeap(int n = 0) { a.reserve(max(4, n)); pos.assign(n, -1); }
    void ensure_pos(int n) { if ((int)pos.size() < n) pos.resize(n, -1); }
    void swap_nodes(int i, int j) {
        swap(a[i], a[j]);
        pos[a[i].second] = i;
        pos[a[j].second] = j;
    }
    void up(int i) {
        while (i > 0) {
            int p = (i - 1) / 2;
            if (a[p].first <= a[i].first) break;
            swap_nodes(p, i);
            i = p;
        }
    }
    void down(int i) {
        int n = a.size();
        while (true) {
            int l = 2*i + 1, r = 2*i + 2, s = i;
            if (l < n && a[l].first < a[s].first) s = l;
            if (r < n && a[r].first < a[s].first) s = r;
            if (s == i) break;
            swap_nodes(i, s);
            i = s;
        }
    }
    void push_or_decrease(int v, long double key) {
        if (v < 0) return;
        if (v >= (int)pos.size()) pos.resize(v+1, -1);
        int p = pos[v];
        if (p == -1) {
            a.emplace_back(key, v);
            pos[v] = (int)a.size() - 1;
            up(pos[v]);
        } else {
            if (key < a[p].first) {
                a[p].first = key;
                up(p);
            }
        }
    }
    bool empty() const { return a.empty(); }
    pair<long double,int> pop() {
        auto ret = a[0];
        pos[ret.second] = -1;
        if (a.size() == 1) { a.pop_back(); return ret; }
        a[0] = a.back();
        a.pop_back();
        pos[a[0].second] = 0;
        down(0);
        return ret;
    }
};

struct MSTResult {
    long long total_cost;
    vector<Edge> chosen;
    MSTResult() { total_cost = 0; }
};

MSTResult kruskal_mst(Graph &g) {
    MSTResult res;
    int n = g.n;
    if (n == 0) return res;
    vector<Edge> es = g.edges;
    sort(es.begin(), es.end(), [](const Edge &a, const Edge &b) {
        if (a.install_cost != b.install_cost) return a.install_cost < b.install_cost;
        return a.travel_time < b.travel_time;
    });
    DSU d(n);
    res.total_cost = 0;
    for (auto &e : es) {
        if (e.u < 0 || e.v < 0) continue;
        if (d.unite(e.u, e.v)) {
            res.chosen.push_back(e);
            res.total_cost += e.install_cost;
            if ((int)res.chosen.size() == n - 1) break;
        }
    }
    if ((int)res.chosen.size() != n - 1) { res.total_cost = INFLL; res.chosen.clear(); }
    return res;
}

MSTResult prim_mst(Graph &g, int start) {
    MSTResult res;
    int n = g.n;
    if (n == 0) return res;
    vector<char> in_mst(n, 0);
    vector<long long> best(n, INFLL);
    vector<int> parent(n, -1);
    MinHeap h(n);
    best[start] = 0;
    h.push_or_decrease(start, 0);
    while (!h.empty()) {
        auto pr = h.pop();
        int u = pr.second;
        if (in_mst[u]) continue;
        in_mst[u] = 1;
        if (parent[u] != -1) {
            Adj* cur = g.head[u];
            bool found = false;
            Edge E;
            while (cur) {
                if (cur->to == parent[u]) {
                    E.u = parent[u];
                    E.v = u;
                    E.travel_time = cur->travel_time;
                    E.energy_cost = cur->energy_cost;
                    E.congestion = cur->congestion;
                    E.install_cost = cur->install_cost;
                    E.capacity = cur->capacity;
                    E.price = cur->price;
                    found = true;
                    break;
                }
                cur = cur->next;
            }
            if (!found) {
                E.u = parent[u]; E.v = u; E.travel_time = 0; E.energy_cost = 0; E.congestion = 0; E.install_cost = best[u]; E.capacity = 0; E.price = 0;
            }
            res.chosen.push_back(E);
            if (res.total_cost < INFLL) res.total_cost += E.install_cost;
        }
        Adj* cur = g.head[u];
        while (cur) {
            int v = cur->to;
            if (!in_mst[v] && cur->install_cost < best[v]) {
                best[v] = cur->install_cost;
                parent[v] = u;
                h.push_or_decrease(v, best[v]);
            }
            cur = cur->next;
        }
    }
    if ((int)res.chosen.size() != n - 1) { res.total_cost = INFLL; res.chosen.clear(); }
    return res;
}

vector<long double> dijkstra(Graph &g, int src) {
    int n = g.n;
    vector<long double> dist(n, numeric_limits<long double>::infinity());
    if (src < 0 || src >= n) return dist;
    MinHeap h(n);
    dist[src] = 0;
    h.push_or_decrease(src, 0);
    while (!h.empty()) {
        auto pr = h.pop();
        int u = pr.second;
        long double du = pr.first;
        if (du > dist[u] + 1e-15) continue;
        Adj* cur = g.head[u];
        while (cur) {
            int v = cur->to;
            long double w = cur->travel_time * (1.0L + cur->congestion / 100.0L) + cur->energy_cost;
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                h.push_or_decrease(v, dist[v]);
            }
            cur = cur->next;
        }
    }
    return dist;
}

void bfs(Graph &g, int start, vector<char> &vis) {
    vis.assign(g.n, 0);
    if (start < 0 || start >= g.n) return;
    queue<int> q;
    q.push(start);
    vis[start] = 1;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        Adj* cur = g.head[u];
        while (cur) {
            if (!vis[cur->to]) { vis[cur->to] = 1; q.push(cur->to); }
            cur = cur->next;
        }
    }
}

bool is_connected(Graph &g) {
    if (g.n == 0) return true;
    int start = -1;
    for (int i = 0; i < g.n; ++i) if (g.head[i] != nullptr) { start = i; break; }
    if (start == -1) return (g.n <= 1);
    vector<char> vis;
    bfs(g, start, vis);
    for (int i = 0; i < g.n; ++i) {
        if (g.head[i] != nullptr && (i >= (int)vis.size() || !vis[i])) return false;
    }
    return true;
}

bool remove_edge_uv(Graph &g, int u, int v) {
    if (u < 0 || v < 0 || u >= g.n || v >= g.n) return false;
    for (size_t i = 0; i < g.edges.size(); ++i) {
        Edge &e = g.edges[i];
        if ((e.u == u && e.v == v) || (e.u == v && e.v == u)) {
            auto remove_one = [&](int A, int B, const Edge &E)->bool {
                Adj** ptr = &g.head[A];
                while (*ptr) {
                    Adj* cur = *ptr;
                    if (cur->to == B && cur->travel_time == E.travel_time && fabs(cur->energy_cost - E.energy_cost) < 1e-12 && cur->congestion == E.congestion && cur->install_cost == E.install_cost && cur->capacity == E.capacity && fabs(cur->price - E.price) < 1e-12) {
                        *ptr = cur->next;
                        delete cur;
                        return true;
                    }
                    ptr = &((*ptr)->next);
                }
                return false;
            };
            bool ok1 = remove_one(e.u, e.v, e);
            bool ok2 = remove_one(e.v, e.u, e);
            e.u = e.v = -1;
            vector<Edge> tmp;
            tmp.reserve(g.edges.size());
            for (auto &ed : g.edges) if (ed.u != -1) tmp.push_back(ed);
            g.edges.swap(tmp);
            return ok1 && ok2;
        }
    }
    return false;
}

int remove_node(Graph &g, int x) {
    if (x < 0 || x >= g.n) return 0;
    int removed = 0;
    Adj* cur = g.head[x];
    while (cur) {
        int nb = cur->to;
        Adj** ptr = &g.head[nb];
        while (*ptr) {
            Adj* c2 = *ptr;
            if (c2->to == x && c2->travel_time == cur->travel_time && fabs(c2->energy_cost - cur->energy_cost) < 1e-12 && c2->congestion == cur->congestion && c2->install_cost == cur->install_cost && c2->capacity == cur->capacity && fabs(c2->price - cur->price) < 1e-12) {
                *ptr = c2->next;
                delete c2;
                removed++;
                break;
            }
            ptr = &((*ptr)->next);
        }
        Adj* tmp = cur;
        cur = cur->next;
        delete tmp;
        removed++;
    }
    g.head[x] = nullptr;
    for (auto &e : g.edges) if (e.u == x || e.v == x) { e.u = e.v = -1; e.travel_time = e.install_cost = e.capacity = 0; e.energy_cost = e.price = 0; e.congestion = 0; }
    vector<Edge> tmp;
    tmp.reserve(g.edges.size());
    for (auto &ed : g.edges) if (ed.u != -1) tmp.push_back(ed);
    g.edges.swap(tmp);
    return removed;
}

void prioritize_edges(Graph &g, const string &attr, const string &order) {
    if (g.edges.empty()) { cout << "No edges to prioritize.\n"; return; }
    vector<Edge> copy = g.edges;
    if (attr == "install_cost") sort(copy.begin(), copy.end(), [](const Edge &a, const Edge &b){ if (a.install_cost != b.install_cost) return a.install_cost < b.install_cost; return a.travel_time < b.travel_time; });
    else if (attr == "travel_time") sort(copy.begin(), copy.end(), [](const Edge &a, const Edge &b){ if (a.travel_time != b.travel_time) return a.travel_time < b.travel_time; return a.install_cost < b.install_cost; });
    else if (attr == "congestion") sort(copy.begin(), copy.end(), [](const Edge &a, const Edge &b){ if (a.congestion != b.congestion) return a.congestion < b.congestion; return a.install_cost < b.install_cost; });
    else if (attr == "price") sort(copy.begin(), copy.end(), [](const Edge &a, const Edge &b){ if (a.price != b.price) return a.price < b.price; return a.install_cost < b.install_cost; });
    else { cout << "Unknown attribute '" << attr << "'\n"; return; }
    if (order == "DESC") reverse(copy.begin(), copy.end());
    for (auto &e : copy) {
        cout << "Edge " << e.u << "-" << e.v << " time=" << e.travel_time << " cost=" << e.install_cost << " cong=" << e.congestion << " price=" << e.price << "\n";
    }
}

void export_mst(const MSTResult &mst, const string &which) {
    if (mst.chosen.empty()) { cout << which << " MST not available or graph disconnected.\n"; return; }
    cout << which << " MST total_install_cost = " << mst.total_cost << "\n";
    for (const auto &e : mst.chosen) {
        cout << e.u << " " << e.v << " " << e.travel_time << " " << fixed << setprecision(3) << e.energy_cost << " " << e.congestion << " " << e.install_cost << " " << e.capacity << " " << fixed << setprecision(2) << e.price << "\n";
    }
}

static inline string trim(const string &s) {
    size_t i = 0, j = s.size();
    while (i < j && isspace((unsigned char)s[i])) ++i;
    while (j > i && isspace((unsigned char)s[j-1])) --j;
    return s.substr(i, j - i);
}

vector<string> split_csv_line(const string &line) {
    vector<string> out;
    string cur;
    bool inq = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (inq) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i+1] == '"') { cur.push_back('"'); ++i; }
                else inq = false;
            } else cur.push_back(c);
        } else {
            if (c == ',') { out.push_back(trim(cur)); cur.clear(); }
            else if (c == '"') { inq = true; }
            else cur.push_back(c);
        }
    }
    out.push_back(trim(cur));
    return out;
}

unordered_map<string,int> map_header(const vector<string> &hdr) {
    unordered_map<string,int> mp;
    for (size_t i = 0; i < hdr.size(); ++i) {
        string key = hdr[i];
        for (auto &ch : key) ch = tolower((unsigned char)ch);
        mp[key] = (int)i;
    }
    return mp;
}

string get_field(const vector<string> &fields, const unordered_map<string,int> &mp, const string &name) {
    string nm = name;
    for (auto &ch : nm) ch = tolower((unsigned char)ch);
    auto it = mp.find(nm);
    if (it == mp.end()) return "";
    int idx = it->second;
    if (idx < 0 || idx >= (int)fields.size()) return "";
    return fields[idx];
}

ll parse_ll(const string &s) {
    if (s.empty()) return 0;
    try {
        size_t pos = 0;
        ll v = stoll(s, &pos);
        return v;
    } catch (...) {
        try {
            double d = stod(s);
            return (ll)llround(d);
        } catch (...) {
            return 0;
        }
    }
}

double parse_double(const string &s) {
    if (s.empty()) return 0.0;
    try {
        return stod(s);
    } catch (...) {
        return 0.0;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<string> lines;
    string row;
    bool any_input = false;
    while (getline(cin, row)) { any_input = true; lines.push_back(row); }
    if (!any_input) return 0;

    int first_nonempty = -1;
    for (size_t i = 0; i < lines.size(); ++i) {
        string t = trim(lines[i]);
        if (!t.empty()) { first_nonempty = (int)i; break; }
    }
    if (first_nonempty == -1) return 0;

    bool is_csv = (lines[first_nonempty].find(',') != string::npos);

    if (!is_csv) {
        cerr << "Non-CSV input detected. Provide CSV with header.\n";
        return 0;
    }

    vector<string> header_fields = split_csv_line(lines[first_nonempty]);
    auto header_map = map_header(header_fields);

    Graph g;
    MSTResult last_kruskal; last_kruskal.total_cost = INFLL;
    MSTResult last_prim; last_prim.total_cost = INFLL;
    vector<long double> last_dij;

    for (size_t li = first_nonempty + 1; li < lines.size(); ++li) {
        string raw = lines[li];
        if (trim(raw).empty()) continue;
        vector<string> fields = split_csv_line(raw);
        string command = get_field(fields, header_map, "command");
        if (command.empty()) {
            string su = get_field(fields, header_map, "u");
            string sv = get_field(fields, header_map, "v");
            string sd = get_field(fields, header_map, "travel_time");
            if (!su.empty() && !sv.empty() && !sd.empty()) command = "ROAD";
            else continue;
        }
        string cmd_upper = command;
        for (auto &c : cmd_upper) c = toupper((unsigned char)c);
        ll u = parse_ll(get_field(fields, header_map, "u"));
        ll v = parse_ll(get_field(fields, header_map, "v"));
        ll travel_time = parse_ll(get_field(fields, header_map, "travel_time"));
        double energy_cost = parse_double(get_field(fields, header_map, "energy_cost"));
        int congestion = (int)parse_ll(get_field(fields, header_map, "congestion"));
        ll install_cost = parse_ll(get_field(fields, header_map, "install_cost"));
        int capacity = (int)parse_ll(get_field(fields, header_map, "capacity"));
        double price = parse_double(get_field(fields, header_map, "price"));
        string extra = get_field(fields, header_map, "extra");

        if (cmd_upper == "INIT") {
            int n = (int)u;
            if (n < 0) n = 0;
            g.init(n);
            last_kruskal = MSTResult(); last_kruskal.total_cost = INFLL;
            last_prim = MSTResult(); last_prim.total_cost = INFLL;
            last_dij.clear();
            cout << "Graph initialized: n=" << g.n << "\n";
        }
        else if (cmd_upper == "ROAD" || cmd_upper == "EDGE" || cmd_upper == "ADD_EDGE") {
            if (g.size() == 0) { cerr << "Graph not initialized before EDGE/ROAD. Skipping.\n"; continue; }
            g.addEdge((int)u, (int)v, travel_time, energy_cost, congestion, install_cost, capacity, price);
            last_kruskal = MSTResult(); last_kruskal.total_cost = INFLL;
            last_prim = MSTResult(); last_prim.total_cost = INFLL;
            last_dij.clear();
        }
        else if (cmd_upper == "RUN_ALL") {
            if (g.size() == 0) { cerr << "Graph not initialized. SKIP RUN_ALL\n"; continue; }
            int source = (int)u;
            if (source < 0 || source >= g.n) source = 0;
            last_kruskal = kruskal_mst(g);
            last_prim = prim_mst(g, 0);
            last_dij = dijkstra(g, source);
            cout << "RUN_ALL completed. Kruskal=" << (last_kruskal.total_cost==INFLL? -1: last_kruskal.total_cost)
                 << " Prim=" << (last_prim.total_cost==INFLL? -1: last_prim.total_cost) << "\n";
        }
        else if (cmd_upper == "SHOW_KRUSKAL") {
            if (g.size() == 0) { cerr << "Graph not initialized. SKIP SHOW_KRUSKAL\n"; continue; }
            if (last_kruskal.chosen.empty()) last_kruskal = kruskal_mst(g);
            export_mst(last_kruskal, "Kruskal");
        }
        else if (cmd_upper == "SHOW_PRIM") {
            if (g.size() == 0) { cerr << "Graph not initialized. SKIP SHOW_PRIM\n"; continue; }
            if (last_prim.chosen.empty()) last_prim = prim_mst(g, 0);
            export_mst(last_prim, "Prim");
        }
        else if (cmd_upper == "SHOW_DIJKSTRA") {
            if (g.size() == 0) { cerr << "Graph not initialized. SKIP SHOW_DIJKSTRA\n"; continue; }
            int src = (int)u;
            if (src < 0 || src >= g.n) src = 0;
            vector<long double> dij = dijkstra(g, src);
            cout << "Dijkstra from " << src << " (cost metric):\n";
            for (int i = 0; i < g.n; ++i) {
                if (!isfinite((double)dij[i])) cout << i << ": INF\n"; else cout << i << ": " << fixed << setprecision(6) << (double)dij[i] << "\n";
            }
        }
        else if (cmd_upper == "CHECK_CONNECTIVITY") {
            if (g.size() == 0) { cerr << "Graph not initialized. SKIP CHECK_CONNECTIVITY\n"; continue; }
            bool ok = is_connected(g);
            cout << "Connectivity: " << (ok ? "Connected" : "Disconnected") << "\n";
        }
        else if (cmd_upper == "REMOVE_EDGE") {
            if (g.size() == 0) { cerr << "Graph not initialized. SKIP REMOVE_EDGE\n"; continue; }
            bool ok = remove_edge_uv(g, (int)u, (int)v);
            cout << "REMOVE_EDGE " << u << "-" << v << " => " << (ok ? "Removed" : "NotFound") << "\n";
            last_kruskal = MSTResult(); last_kruskal.total_cost = INFLL;
            last_prim = MSTResult(); last_prim.total_cost = INFLL;
            last_dij.clear();
        }
        else if (cmd_upper == "REMOVE_NODE") {
            if (g.size() == 0) { cerr << "Graph not initialized. SKIP REMOVE_NODE\n"; continue; }
            int cnt = remove_node(g, (int)u);
            cout << "REMOVE_NODE " << u << " removed approx " << cnt << " items\n";
            last_kruskal = MSTResult(); last_kruskal.total_cost = INFLL;
            last_prim = MSTResult(); last_prim.total_cost = INFLL;
            last_dij.clear();
        }
        else if (cmd_upper == "PRIORITIZE_EDGES") {
            if (g.size() == 0) { cerr << "Graph not initialized. SKIP PRIORITIZE_EDGES\n"; continue; }
            string attr; string ord;
            {
                stringstream ss(extra);
                ss >> attr >> ord;
                if (attr.empty()) { cerr << "PRIORITIZE_EDGES requires extra like: install_cost ASC\n"; continue; }
                if (ord.empty()) ord = "ASC";
                for (auto &c : attr) c = tolower((unsigned char)c);
                for (auto &c : ord) c = toupper((unsigned char)c);
            }
            prioritize_edges(g, attr, ord);
        }
        else if (cmd_upper == "EXPORT_MST") {
            if (g.size() == 0) { cerr << "Graph not initialized. SKIP EXPORT_MST\n"; continue; }
            string which = extra;
            for (auto &c : which) c = tolower((unsigned char)c);
            if (which == "kruskal") {
                if (last_kruskal.chosen.empty()) last_kruskal = kruskal_mst(g);
                export_mst(last_kruskal, "Kruskal");
            } else if (which == "prim") {
                if (last_prim.chosen.empty()) last_prim = prim_mst(g, 0);
                export_mst(last_prim, "Prim");
            } else cout << "EXPORT_MST requires extra 'kruskal' or 'prim'\n";
        }
        else if (cmd_upper == "QUIT") {
            cout << "QUIT encountered. Stopping.\n";
            break;
        }
        else {
            cout << "Unknown command (CSV): " << command << "\n";
        }
    }

    return 0;
}
