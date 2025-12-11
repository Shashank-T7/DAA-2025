#include <bits/stdc++.h>
using namespace std;
using ll = long long;
struct Plot {
    int id;
    double x;
    double y;
    double moisture;
    double growth;
    double temp;
    double humidity;
    double light;
    double co2;
    double yield_val;
};
struct Fenwick {
    int n;
    vector<double> bit;
    Fenwick(): n(0) {}
    void init(int n_) { n = n_; bit.assign(n+1, 0.0); }
    void add(int idx, double val) {
        idx++;
        while (idx <= n) { bit[idx] += val; idx += idx & -idx; }
    }
    double sumPrefix(int idx) {
        idx++;
        double res = 0.0;
        while (idx > 0) { res += bit[idx]; idx -= idx & -idx; }
        return res;
    }
    double rangeSum(int l, int r) {
        if (r < l) return 0.0;
        return sumPrefix(r) - (l? sumPrefix(l-1): 0.0);
    }
};
struct SegTree {
    int n;
    vector<double> segMin;
    vector<double> segMax;
    SegTree(): n(0) {}
    void init(int N) {
        n = 1;
        while (n < N) n <<= 1;
        segMin.assign(2*n, 1e300);
        segMax.assign(2*n, -1e300);
    }
    void build(const vector<double>&arr) {
        int N = arr.size();
        init(N);
        for (int i = 0; i < N; ++i) {
            segMin[n+i] = arr[i];
            segMax[n+i] = arr[i];
        }
        for (int i = n-1; i >= 1; --i) {
            segMin[i] = min(segMin[2*i], segMin[2*i+1]);
            segMax[i] = max(segMax[2*i], segMax[2*i+1]);
        }
    }
    void update(int idx, double val) {
        int p = n + idx;
        segMin[p] = val;
        segMax[p] = val;
        p >>= 1;
        while (p >= 1) {
            segMin[p] = min(segMin[2*p], segMin[2*p+1]);
            segMax[p] = max(segMax[2*p], segMax[2*p+1]);
            p >>= 1;
        }
    }
    pair<double,double> queryRange(int l, int r) {
        if (l > r) return {1e300, -1e300};
        double mn = 1e300, mx = -1e300;
        l += n; r += n;
        while (l <= r) {
            if (l & 1) { mn = min(mn, segMin[l]); mx = max(mx, segMax[l]); ++l; }
            if (!(r & 1)) { mn = min(mn, segMin[r]); mx = max(mx, segMax[r]); --r; }
            l >>= 1; r >>= 1;
        }
        return {mn, mx};
    }
};
vector<string> split_csv_line(const string &s) {
    vector<string> out;
    string cur;
    bool inq = false;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (inq) {
            if (c == '"') {
                if (i+1 < s.size() && s[i+1] == '"') { cur.push_back('"'); ++i; }
                else inq = false;
            } else cur.push_back(c);
        } else {
            if (c == ',') { out.push_back(cur); cur.clear(); }
            else if (c == '"') inq = true;
            else cur.push_back(c);
        }
    }
    out.push_back(cur);
    for (auto &t : out) {
        size_t a=0, b=t.size();
        while (a<b && isspace((unsigned char)t[a])) ++a;
        while (b>a && isspace((unsigned char)t[b-1])) --b;
        t = t.substr(a, b-a);
    }
    return out;
}
int parse_int(const string &s) {
    if (s.empty()) return 0;
    try { return stoi(s); } catch(...) { return 0; }
}
double parse_double(const string &s) {
    if (s.empty()) return 0.0;
    try { return stod(s); } catch(...) { return 0.0; }
}
double dist2(const Plot &a, const Plot &b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return dx*dx + dy*dy;
}
vector<int> kmeans(const vector<Plot> &plots, int K, int maxIter=100) {
    int n = plots.size();
    if (n == 0) return {};
    if (K <= 0) K = 1;
    vector<int> assign(n, 0);
    vector<pair<double,double>> cent(K);
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> uid(0, n-1);
    for (int i = 0; i < K; ++i) {
        int idx = uid(gen);
        cent[i].first = plots[idx].x;
        cent[i].second = plots[idx].y;
    }
    for (int it = 0; it < maxIter; ++it) {
        bool changed = false;
        for (int i = 0; i < n; ++i) {
            double best = 1e300; int bi = 0;
            for (int j = 0; j < K; ++j) {
                double dx = plots[i].x - cent[j].first;
                double dy = plots[i].y - cent[j].second;
                double d = dx*dx + dy*dy;
                if (d < best) { best = d; bi = j; }
            }
            if (assign[i] != bi) { assign[i] = bi; changed = true; }
        }
        if (!changed) break;
        vector<double> sx(K,0), sy(K,0);
        vector<int> cnt(K,0);
        for (int i = 0; i < n; ++i) { int c = assign[i]; sx[c] += plots[i].x; sy[c] += plots[i].y; cnt[c]++; }
        for (int j = 0; j < K; ++j) {
            if (cnt[j] > 0) {
                cent[j].first = sx[j] / cnt[j];
                cent[j].second = sy[j] / cnt[j];
            } else {
                int idx = uid(gen);
                cent[j].first = plots[idx].x;
                cent[j].second = plots[idx].y;
            }
        }
    }
    return assign;
}
vector<int> nearest_neighbor_tour(const vector<int> &ids, const vector<Plot> &plots) {
    int m = ids.size();
    if (m == 0) return {};
    vector<int> tour;
    tour.reserve(m);
    vector<char> used(m, 0);
    int cur = 0;
    tour.push_back(ids[cur]); used[cur] = 1;
    for (int step = 1; step < m; ++step) {
        double best = 1e300;
        int bi = -1;
        for (int j = 0; j < m; ++j) if (!used[j]) {
            double d = dist2(plots[ids[cur]], plots[ids[j]]);
            if (d < best) { best = d; bi = j; }
        }
        if (bi == -1) break;
        cur = bi;
        used[cur] = 1;
        tour.push_back(ids[cur]);
    }
    return tour;
}
double tour_length(const vector<int> &tour, const vector<Plot> &plots) {
    if (tour.empty()) return 0.0;
    double s = 0.0;
    for (size_t i = 0; i + 1 < tour.size(); ++i) {
        Plot a = plots[tour[i]], b = plots[tour[i+1]];
        double dx = a.x - b.x, dy = a.y - b.y;
        s += sqrt(dx*dx + dy*dy);
    }
    Plot a = plots[tour.back()], b = plots[tour.front()];
    double dx = a.x - b.x, dy = a.y - b.y;
    s += sqrt(dx*dx + dy*dy);
    return s;
}
void two_opt(vector<int> &tour, const vector<Plot> &plots) {
    int n = tour.size();
    if (n < 4) return;
    bool improved = true;
    while (improved) {
        improved = false;
        for (int i = 0; i < n-1 && !improved; ++i) {
            for (int k = i+2; k < n && !improved; ++k) {
                int a = tour[i];
                int b = tour[(i+1)%n];
                int c = tour[k% n];
                int d = tour[(k+1)%n];
                double before = sqrt((plots[a].x-plots[b].x)*(plots[a].x-plots[b].x)+(plots[a].y-plots[b].y)*(plots[a].y-plots[b].y))
                              + sqrt((plots[c].x-plots[d].x)*(plots[c].x-plots[d].x)+(plots[c].y-plots[d].y)*(plots[c].y-plots[d].y));
                double after = sqrt((plots[a].x-plots[c].x)*(plots[a].x-plots[c].x)+(plots[a].y-plots[c].y)*(plots[a].y-plots[c].y))
                             + sqrt((plots[b].x-plots[d].x)*(plots[b].x-plots[d].x)+(plots[b].y-plots[d].y)*(plots[b].y-plots[d].y));
                if (after + 1e-9 < before) {
                    reverse(tour.begin()+i+1, tour.begin()+k+1);
                    improved = true;
                }
            }
        }
    }
}
vector<vector<int>> split_tour_by_capacity(const vector<int> &tour, const vector<Plot> &plots, double capacity) {
    vector<vector<int>> parts;
    double cur_load = 0.0;
    vector<int> cur;
    for (int id : tour) {
        double w = plots[id].yield_val;
        if (!cur.empty() && cur_load + w > capacity) {
            parts.push_back(cur);
            cur.clear();
            cur_load = 0.0;
        }
        cur.push_back(id);
        cur_load += w;
    }
    if (!cur.empty()) parts.push_back(cur);
    return parts;
}
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    vector<string> lines;
    string line;
    while (getline(cin, line)) lines.push_back(line);
    if (lines.empty()) return 0;
    int first_nonempty = 0;
    while (first_nonempty < (int)lines.size()) {
        string t = lines[first_nonempty];
        bool onlyws = true;
        for (char c : t) if (!isspace((unsigned char)c)) { onlyws = false; break; }
        if (!onlyws) break;
        ++first_nonempty;
    }
    if (first_nonempty >= (int)lines.size()) return 0;
    vector<string> header = split_csv_line(lines[first_nonempty]);
    unordered_map<string,int> idx;
    for (int i = 0; i < (int)header.size(); ++i) {
        string key = header[i];
        for (auto &c : key) c = tolower((unsigned char)c);
        idx[key] = i;
    }
    vector<Plot> plots;
    plots.reserve(6000);
    for (int i = first_nonempty + 1; i < (int)lines.size(); ++i) {
        string s = lines[i];
        if (s.find_first_not_of(" \t\r\n") == string::npos) continue;
        vector<string> fields = split_csv_line(s);
        string cmd = "";
        if (idx.count("command")) cmd = fields[idx["command"]];
        if (cmd.empty()) {
            if (idx.count("plot_id")) cmd = "PLOT";
        }
        string up = cmd;
        for (auto &c : up) c = toupper((unsigned char)c);
        if (up == "PLOT") {
            Plot p;
            p.id = parse_int(fields[idx.count("plot_id")?idx["plot_id"]:0]);
            p.x = parse_double(fields[idx.count("x")?idx["x"]:0]);
            p.y = parse_double(fields[idx.count("y")?idx["y"]:0]);
            p.moisture = parse_double(fields[idx.count("moisture")?idx["moisture"]:0]);
            p.growth = parse_double(fields[idx.count("growth")?idx["growth"]:0]);
            p.temp = parse_double(fields[idx.count("temp")?idx["temp"]:0]);
            p.humidity = parse_double(fields[idx.count("humidity")?idx["humidity"]:0]);
            p.light = parse_double(fields[idx.count("light")?idx["light"]:0]);
            p.co2 = parse_double(fields[idx.count("co2")?idx["co2"]:0]);
            p.yield_val = parse_double(fields[idx.count("yield")?idx["yield"]:0]);
            plots.push_back(p);
        } else if (up == "QUIT") {
            break;
        } else {
            // ignore unknown commands for now
        }
    }
    int n = plots.size();
    cout << "Loaded " << n << " plots\n";
    if (n == 0) return 0;
    Fenwick fen_moist, fen_yield;
    fen_moist.init(n);
    fen_yield.init(n);
    vector<double> growths(n);
    for (int i = 0; i < n; ++i) {
        fen_moist.add(i, plots[i].moisture);
        fen_yield.add(i, plots[i].yield_val);
        growths[i] = plots[i].growth;
    }
    SegTree seg_growth;
    seg_growth.build(growths);
    int K = max(1, (int)round(sqrt((double)n)));
    vector<int> assignment = kmeans(plots, K, 50);
    vector<vector<int>> clusters(K);
    for (int i = 0; i < n; ++i) {
        int c = assignment[i];
        if (c < 0) c = 0;
        if (c >= K) c = K-1;
        clusters[c].push_back(i);
    }
    cout << "Kmeans clustered into " << K << " clusters\n";
    for (int c = 0; c < K; ++c) {
        cout << "Cluster " << c << " size " << clusters[c].size() << "\n";
    }
    vector<pair<double,int>> ranked;
    ranked.reserve(n);
    for (int i = 0; i < n; ++i) ranked.emplace_back(-plots[i].yield_val, plots[i].id);
    sort(ranked.begin(), ranked.end());
    cout << "Top 10 plots by yield:\n";
    for (int i = 0; i < min(10, (int)ranked.size()); ++i) {
        cout << i+1 << ". plot " << ranked[i].second << " yield=" << -ranked[i].first << "\n";
    }
    double cap = 50.0;
    cout << "Using truck capacity threshold (for demo split) = " << cap << "\n";
    for (int c = 0; c < K; ++c) {
        auto &ids = clusters[c];
        if (ids.empty()) continue;
        vector<int> tour = nearest_neighbor_tour(ids, plots);
        two_opt(tour, plots);
        auto parts = split_tour_by_capacity(tour, plots, cap);
        cout << "Cluster " << c << " tour length " << fixed << setprecision(3) << tour_length(tour, plots) << " parts " << parts.size() << "\n";
    }
    cout << "Fenwick moisture sum [0," << n-1 << "] = " << fen_moist.rangeSum(0, n-1) << "\n";
    cout << "Fenwick yield sum [0," << n-1 << "] = " << fen_yield.rangeSum(0, n-1) << "\n";
    auto gm = seg_growth.queryRange(0, n-1);
    cout << "Growth min=" << gm.first << " max=" << gm.second << "\n";
    cout << "Ready. Enter interactive commands (UPDATE, FENWICK_QUERY, SEG_QUERY, KMEANS k, RANK topk, QUIT)\n";
    string cmdline;
    while (true) {
        if (!getline(cin, cmdline)) break;
        if (cmdline.find_first_not_of(" \t\r\n") == string::npos) continue;
        vector<string> toks;
        {
            string tmp; stringstream ss(cmdline);
            while (ss >> tmp) toks.push_back(tmp);
        }
        if (toks.empty()) continue;
        string c = toks[0];
        for (auto &ch : c) ch = toupper((unsigned char)ch);
        if (c == "QUIT") break;
        else if (c == "UPDATE") {
            if (toks.size() < 4) { cout << "Usage: UPDATE id field value\n"; continue; }
            int id = stoi(toks[1]);
            string field = toks[2];
            double val = stod(toks[3]);
            int idxplot = -1;
            for (int i = 0; i < n; ++i) if (plots[i].id == id) { idxplot = i; break; }
            if (idxplot == -1) { cout << "Plot not found\n"; continue; }
            if (field == "moisture") {
                double old = plots[idxplot].moisture;
                plots[idxplot].moisture = val;
                fen_moist.add(idxplot, val - old);
                cout << "Updated moisture\n";
            } else if (field == "yield") {
                double old = plots[idxplot].yield_val;
                plots[idxplot].yield_val = val;
                fen_yield.add(idxplot, val - old);
                cout << "Updated yield\n";
            } else if (field == "growth") {
                plots[idxplot].growth = val;
                seg_growth.update(idxplot, val);
                cout << "Updated growth and segment tree\n";
            } else cout << "Unknown field\n";
        } else if (c == "FENWICK_QUERY") {
            if (toks.size() < 3) { cout << "Usage: FENWICK_QUERY l r field\n"; continue; }
            int l = stoi(toks[1]), r = stoi(toks[2]);
            string field = (toks.size() >= 4 ? toks[3] : "moisture");
            if (l < 0) l = 0; if (r >= n) r = n-1;
            if (field == "moisture") cout << "moisture sum = " << fen_moist.rangeSum(l, r) << "\n";
            else if (field == "yield") cout << "yield sum = " << fen_yield.rangeSum(l, r) << "\n";
            else cout << "Unknown field\n";
        } else if (c == "SEG_QUERY") {
            if (toks.size() < 3) { cout << "Usage: SEG_QUERY l r\n"; continue; }
            int l = stoi(toks[1]), r = stoi(toks[2]);
            if (l < 0) l = 0; if (r >= n) r = n-1;
            auto pr = seg_growth.queryRange(l, r);
            cout << "growth min=" << pr.first << " max=" << pr.second << "\n";
        } else if (c == "KMEANS") {
            if (toks.size() < 2) { cout << "Usage: KMEANS k\n"; continue; }
            int k = stoi(toks[1]);
            auto assign2 = kmeans(plots, k, 100);
            cout << "Recomputed kmeans with k=" << k << "\n";
        } else if (c == "RANK") {
            int topk = 10;
            if (toks.size() >= 2) topk = stoi(toks[1]);
            vector<pair<double,int>> rr;
            for (int i = 0; i < n; ++i) rr.emplace_back(-plots[i].yield_val, plots[i].id);
            sort(rr.begin(), rr.end());
            cout << "Top " << topk << " by yield:\n";
            for (int i = 0; i < min(topk, (int)rr.size()); ++i) cout << i+1 << ". plot " << rr[i].second << " yield=" << -rr[i].first << "\n";
        } else cout << "Unknown command\n";
    }
    return 0;
}
