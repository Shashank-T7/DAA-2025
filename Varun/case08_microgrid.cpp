#include <bits/stdc++.h>
using namespace std;
using ll = long long;
const double INF_D = 1e300;

struct Edge {
    int u, v;
    double loss;
    int capacity;
    Edge(): u(-1), v(-1), loss(0.0), capacity(0) {}
    Edge(int a,int b,double l,int c): u(a), v(b), loss(l), capacity(c) {}
};

struct Graph {
    int n;
    vector<vector<pair<int,int>>> adj;
    vector<Edge> edges;
    Graph(): n(0) {}
    void init(int N){ n=N; adj.assign(n,{}); edges.clear(); }
    void addEdge(int u,int v,double loss,int cap){
        if(u<0||v<0) return;
        if(max(u,v) >= n){
            int newn = max(u,v)+1;
            adj.resize(newn);
            n = newn;
        }
        edges.emplace_back(u,v,loss,cap);
        int id = edges.size()-1;
        if((int)adj.size() <= max(u,v)) adj.resize(max(u,v)+1);
        adj[u].push_back({v,id});
        adj[v].push_back({u,id});
    }
};

struct Fenwick {
    int n;
    vector<double> bit;
    Fenwick(): n(0) {}
    void init(int N){ n=N; bit.assign(n+1,0.0); }
    void add(int idx,double val){ idx++; while(idx<=n){ bit[idx]+=val; idx+=idx&-idx; } }
    double sumPrefix(int idx){ idx++; double s=0.0; while(idx>0){ s+=bit[idx]; idx-=idx&-idx; } return s; }
    double rangeSum(int l,int r){ if(r<l) return 0.0; return sumPrefix(r) - (l?sumPrefix(l-1):0.0); }
};

vector<string> split_csv_line(const string &s){
    vector<string> out; string cur; bool inq=false;
    for(size_t i=0;i<s.size();++i){
        char c=s[i];
        if(inq){
            if(c=='"'){ if(i+1<s.size() && s[i+1]=='"'){ cur.push_back('"'); ++i; } else inq=false; }
            else cur.push_back(c);
        } else {
            if(c==','){ out.push_back(cur); cur.clear(); }
            else if(c=='"') inq=true;
            else cur.push_back(c);
        }
    }
    out.push_back(cur);
    for(auto &t: out){ size_t a=0,b=t.size(); while(a<b && isspace((unsigned char)t[a])) ++a; while(b>a && isspace((unsigned char)t[b-1])) --b; t=t.substr(a,b-a); }
    return out;
}

int to_int(const string &s){ if(s.empty()) return 0; try{return stoi(s);}catch(...){return 0;} }
double to_double(const string &s){ if(s.empty()) return 0.0; try{return stod(s);}catch(...){return 0.0;} }
long long to_ll(const string &s){ if(s.empty()) return 0; try{return stoll(s);}catch(...){return 0;} }

struct Dijkstra {
    Graph *g;
    Dijkstra(Graph *gr=nullptr): g(gr) {}
    vector<double> run(int src){
        int n = g->n;
        vector<double> dist(n, INF_D);
        if(src<0||src>=n) return dist;
        using P = pair<double,int>;
        priority_queue<P, vector<P>, greater<P>> pq;
        dist[src]=0; pq.push({0,src});
        while(!pq.empty()){
            auto p = pq.top(); pq.pop();
            double d = p.first; int u = p.second;
            if(d > dist[u] + 1e-12) continue;
            for(auto &pr: g->adj[u]){
                int v = pr.first; int eid = pr.second;
                Edge &e = g->edges[eid];
                double w = e.loss * 1000.0; // scale proxy
                if(dist[u] + w < dist[v]){
                    dist[v] = dist[u] + w;
                    pq.push({dist[v], v});
                }
            }
        }
        return dist;
    }
};

struct BellmanFord {
    int n;
    vector<double> dist;
    BellmanFord(int N=0){ init(N); }
    void init(int N){ n=N; dist.assign(n, INF_D); }
    bool detect_negative_cycle(const vector<tuple<int,int,double>> &edges){
        if(n==0) return false;
        for(int i=0;i<n;++i) dist[i]=0.0;
        for(int iter=0; iter<n-1; ++iter){
            bool any=false;
            for(auto &t: edges){
                int u,v; double w; tie(u,v,w)=t;
                if(u<0||v<0||u>=n||v>=n) continue;
                if(dist[u] + w < dist[v]){ dist[v] = dist[u] + w; any=true; }
            }
            if(!any) break;
        }
        for(auto &t: edges){
            int u,v; double w; tie(u,v,w)=t;
            if(u<0||v<0||u>=n||v>=n) continue;
            if(dist[u] + w < dist[v]) return true;
        }
        return false;
    }
};

struct Battery {
    int id;
    double soc;
    double capacity;
    double max_discharge; 
    double max_charge;
    double efficiency;
    Battery(): id(0), soc(0), capacity(0), max_discharge(0), max_charge(0), efficiency(1.0) {}
};

struct MicrogridManager {
    Graph g;
    Fenwick fen; 
    vector<Battery> batteries;
    vector<pair<long long,double>> demand_times;
    vector<tuple<long long,int,double>> storage_updates;
    MicrogridManager(){}
    void load_from_csv_lines(const vector<string> &lines){
        if(lines.empty()) return;
        int first=0; while(first < (int)lines.size() && lines[first].find(',')==string::npos) ++first;
        if(first >= (int)lines.size()) return;
        auto header = split_csv_line(lines[first]);
        unordered_map<string,int> idx;
        for(int i=0;i<(int)header.size();++i){ string k=header[i]; for(auto &c:k) c=tolower((unsigned char)c); idx[k]=i; }
        for(int i=first+1;i<(int)lines.size();++i){
            string row = lines[i];
            if(row.find_first_not_of(" \t\r\n")==string::npos) continue;
            auto f = split_csv_line(row);
            string cmd = "";
            if(idx.count("command")) cmd = f[idx["command"]];
            if(cmd.empty()){
                if(idx.count("u") && idx.count("v") && idx.count("loss")) cmd = "EDGE";
            }
            string cu = cmd; for(auto &c:cu) c = toupper((unsigned char)c);
            if(cu == "INIT"){
                int n = to_int(f[idx.count("u")?idx["u"]:0]);
                int m = to_int(f[idx.count("v")?idx["v"]:1]);
                if(n<=0) n=0;
                g.init(n);
            } else if(cu == "EDGE"){
                int u = to_int(f[idx.count("u")?idx["u"]:1]);
                int v = to_int(f[idx.count("v")?idx["v"]:2]);
                double loss = to_double(f[idx.count("loss")?idx["loss"]:3]);
                int cap = to_int(f[idx.count("line_capacity")?idx["line_capacity"]:4]);
                g.addEdge(u,v,loss,cap);
            } else if(cu == "STORAGE"){
                long long ts = to_ll(f[idx.count("timestamp")?idx["timestamp"]:5]);
                int batt = to_int(f[idx.count("battery_id")?idx["battery_id"]:6]);
                double change = to_double(f[idx.count("charge_change")?idx["charge_change"]:7]);
                double bcap = to_double(f[idx.count("battery_capacity")?idx["battery_capacity"]:8]);
                double price = to_double(f[idx.count("price")?idx["price"]:9]);
                storage_updates.emplace_back(ts,batt,change);
                if(batt >= (int)batteries.size()){
                    batteries.resize(batt+1);
                    batteries[batt].id = batt;
                    batteries[batt].capacity = bcap;
                    batteries[batt].soc = bcap * 0.5;
                    batteries[batt].max_charge = bcap * 0.5;
                    batteries[batt].max_discharge = bcap * 0.5;
                    batteries[batt].efficiency = 0.95;
                }
            } else if(cu == "DEMAND"){
                long long ts = to_ll(f[idx.count("timestamp")?idx["timestamp"]:5]);
                double d = to_double(f[idx.count("price")?idx["price"]:9]);
                demand_times.emplace_back(ts, d);
            } else {
                // ignore others
            }
        }
        int slots = max(1, (int)max(1, (int)demand_times.size()));
        fen.init(slots + 5);
        for(size_t i=0;i<storage_updates.size();++i){
            long long ts; int bid; double change; tie(ts,bid,change) = storage_updates[i];
            int slot = i % fen.n;
            fen.add(slot, change);
        }
    }
    vector<double> run_dijkstra(int src){ Dijkstra dj(&g); return dj.run(src); }
    bool run_bellmanford_detect(){
        int N = g.n;
        vector<tuple<int,int,double>> ed;
        for(auto &e: g.edges){
            ed.emplace_back(e.u, e.v, e.loss - 0.01);
            ed.emplace_back(e.v, e.u, e.loss - 0.01);
        }
        BellmanFord bf(N); bf.init(N);
        return bf.detect_negative_cycle(ed);
    }
    vector<int> greedy_discharge(double required_kwh){
        vector<pair<double,int>> cand;
        for(auto &b: batteries){
            double avail = max(0.0, min(b.soc, b.max_discharge));
            if(avail>1e-9) cand.emplace_back(avail / (b.capacity + 1e-9), b.id);
        }
        sort(cand.begin(), cand.end(), greater<pair<double,int>>());
        vector<int> used;
        double got = 0;
        for(auto &p: cand){
            if(got >= required_kwh) break;
            int id = p.second;
            double take = min(required_kwh - got, batteries[id].max_discharge);
            if(take>0){
                batteries[id].soc -= take;
                got += take;
                used.push_back(id);
            }
        }
        return used;
    }
    double fenwick_query_range(int l,int r){ return fen.rangeSum(l,r); }
    void fenwick_add_slot(int slot,double val){ if(slot>=0 && slot < fen.n) fen.add(slot,val); }
};

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    vector<string> lines;
    string row;
    while(getline(cin,row)) lines.push_back(row);
    MicrogridManager mgr;
    mgr.load_from_csv_lines(lines);
    cout<<"Loaded graph nodes="<<mgr.g.n<<" edges="<<mgr.g.edges.size()<<"\n";
    cout<<"Batteries loaded="<<mgr.batteries.size()<<"\n";
    cout<<"Storage updates="<<mgr.storage_updates.size()<<" demand_points="<<mgr.demand_times.size()<<"\n";
    cout<<"Default analyses:\n";
    if(mgr.g.n>0){
        int src = 0;
        auto dist = mgr.run_dijkstra(src);
        cout<<"Dijkstra from "<<src<<": sample dist[0]="<<dist[0]<<"\n";
    }
    bool neg = mgr.run_bellmanford_detect();
    cout<<"Bellman-Ford negative cycle detected="<<(neg? "YES":"NO")<<"\n";
    double peak_required = 500.0;
    auto used = mgr.greedy_discharge(peak_required);
    cout<<"Greedy discharge used batteries="<<used.size()<<"\n";
    cout<<"Interactive commands: RUN_DIJKSTRA src dst, FENWICK_ADD slot val, FENWICK_QUERY l r, BELLFORD_RUN, GREEDY_DISCHARGE val, SUMMARY, QUIT\n";
    string cmdline;
    while(true){
        if(!getline(cin, cmdline)) break;
        if(cmdline.find_first_not_of(" \t\r\n")==string::npos) continue;
        stringstream ss(cmdline);
        string cmd; ss>>cmd;
        for(auto &c:cmd) c=toupper((unsigned char)c);
        if(cmd=="QUIT") break;
        else if(cmd=="RUN_DIJKSTRA"){
            int a,b; if(!(ss>>a>>b)){ cout<<"Usage: RUN_DIJKSTRA src dst\n"; continue; }
            vector<double> d = mgr.run_dijkstra(a);
            if(b>=0 && b < (int)d.size()) cout<<"Dist["<<b<<"]="<<d[b]<<"\n"; else cout<<"Invalid dst\n";
        } else if(cmd=="FENWICK_ADD"){
            int slot; double val; if(!(ss>>slot>>val)){ cout<<"Usage: FENWICK_ADD slot val\n"; continue; }
            mgr.fenwick_add_slot(slot,val); cout<<"Added\n";
        } else if(cmd=="FENWICK_QUERY"){
            int l,r; if(!(ss>>l>>r)){ cout<<"Usage: FENWICK_QUERY l r\n"; continue; }
            cout<<"Sum="<<mgr.fenwick_query_range(l,r)<<"\n";
        } else if(cmd=="BELLFORD_RUN"){
            bool negcy = mgr.run_bellmanford_detect();
            cout<<"Negative cycle: "<<(negcy? "YES":"NO")<<"\n";
        } else if(cmd=="GREEDY_DISCHARGE"){
            double val; if(!(ss>>val)){ cout<<"Usage: GREEDY_DISCHARGE kwh\n"; continue; }
            auto u = mgr.greedy_discharge(val);
            cout<<"Used batteries count="<<u.size()<<"\n";
        } else if(cmd=="SUMMARY"){
            cout<<"Nodes="<<mgr.g.n<<" edges="<<mgr.g.edges.size()<<" batteries="<<mgr.batteries.size()<<"\n";
        } else cout<<"Unknown command\n";
    }
    return 0;
}
