#include <bits/stdc++.h>
using namespace std;

struct Edge {
    int u;
    int v;
    double w;
    int id;
    Edge() : u(0), v(0), w(0.0), id(-1) {}
    Edge(int _u,int _v,double _w,int _id):u(_u),v(_v),w(_w),id(_id){}
};

struct DSU {
    int n;
    vector<int> p;
    vector<int> r;
    vector<int> sz;
    DSU():n(0){}
    DSU(int n_){ init(n_); }
    void init(int n_){
        n = n_;
        p.resize(n);
        r.assign(n,0);
        sz.assign(n,1);
        for(int i=0;i<n;++i) p[i]=i;
    }
    int find(int a){
        while(p[a]!=a){ p[a]=p[p[a]]; a=p[a]; }
        return a;
    }
    bool unite(int a,int b){
        a=find(a); b=find(b);
        if(a==b) return false;
        if(r[a]<r[b]) swap(a,b);
        p[b]=a;
        sz[a]+=sz[b];
        if(r[a]==r[b]) r[a]++;
        return true;
    }
    int size(int a){ return sz[find(a)]; }
};

struct GraphLoader {
    string path;
    int declared_n;
    GraphLoader(const string &p=""):path(p),declared_n(-1){}
    vector<Edge> load_edges(int &n_out){
        vector<Edge> edges;
        ifstream in(path);
        if(!in.is_open()){ n_out = 0; return edges; }
        string line;
        if(!getline(in, line)){ n_out=0; return edges; }
        bool first_is_header = false;
        {
            string tmp=line;
            bool isnum=true;
            for(char c: tmp) if(!isdigit((unsigned char)c) && c!=' ' && c!=',' && c!='\t' && c!='-'){ isnum=false; break; }
            first_is_header = !isnum;
        }
        int idcnt=0;
        if(!first_is_header){
            {
                stringstream ss(line);
                vector<string> tokens;
                string tok;
                while(getline(ss, tok, ',')) tokens.push_back(tok);
                if(tokens.size()>=3){
                    int a = stoi(tokens[0]);
                    int b = stoi(tokens[1]);
                    double w = stod(tokens[2]);
                    edges.emplace_back(a,b,w,idcnt++);
                }
            }
        }
        while(getline(in, line)){
            if(line.empty()) continue;
            vector<string> cols;
            string cur; bool inq=false;
            for(char ch: line){
                if(ch=='"'){ inq=!inq; continue; }
                if(ch==',' && !inq){ cols.push_back(cur); cur.clear(); } else cur.push_back(ch);
            }
            if(!cur.empty()) cols.push_back(cur);
            if(cols.size() < 3) continue;
            int u = stoi(cols[0]);
            int v = stoi(cols[1]);
            double w = stod(cols[2]);
            edges.emplace_back(u,v,w,idcnt++);
        }
        int maxnode = -1;
        for(auto &e: edges) maxnode = max(maxnode, max(e.u, e.v));
        n_out = maxnode + 1;
        return edges;
    }
};

struct KruskalSolver {
    vector<Edge> edges;
    int n;
    KruskalSolver():n(0){}
    void load_from_csv(const string &path){
        GraphLoader gl(path);
        int nout=0;
        edges = gl.load_edges(nout);
        n = nout;
    }
    pair<vector<Edge>, double> mst_kruskal(){
        vector<Edge> out;
        double total=0.0;
        if(n<=0) return {out,total};
        vector<int> ord(edges.size());
        for(size_t i=0;i<edges.size(); ++i) ord[i]=i;
        sort(ord.begin(), ord.end(), [&](int a,int b){ if(edges[a].w==edges[b].w) return edges[a].id < edges[b].id; return edges[a].w < edges[b].w; });
        DSU dsu(n);
        for(int idx: ord){
            auto &e = edges[idx];
            if(dsu.unite(e.u, e.v)){
                out.push_back(e);
                total += e.w;
                if((int)out.size() == n-1) break;
            }
        }
        return {out, total};
    }
    vector<vector<pair<int,double>>> build_adj_from_mst(const vector<Edge> &mst){
        vector<vector<pair<int,double>>> adj(n);
        for(auto &e: mst){
            if(e.u >=0 && e.u < n && e.v >=0 && e.v < n){
                adj[e.u].push_back({e.v, e.w});
                adj[e.v].push_back({e.u, e.w});
            }
        }
        return adj;
    }
    vector<vector<pair<int,double>>> build_adj_full(){
        vector<vector<pair<int,double>>> adj(n);
        for(auto &e: edges){
            if(e.u>=0 && e.u<n && e.v>=0 && e.v<n){
                adj[e.u].push_back({e.v, e.w});
                adj[e.v].push_back({e.u, e.w});
            }
        }
        return adj;
    }
    vector<pair<int,int>> cluster_by_threshold(double threshold){
        DSU dsu(n);
        for(auto &e: edges){
            if(e.w <= threshold) dsu.unite(e.u, e.v);
        }
        unordered_map<int,int> compmap;
        int cnt=0;
        vector<pair<int,int>> compSizes;
        for(int i=0;i<n;++i){
            int r = dsu.find(i);
            if(!compmap.count(r)) compmap[r] = cnt++;
        }
        vector<int> sizes(cnt,0);
        for(int i=0;i<n;++i) sizes[compmap[dsu.find(i)]]++;
        for(int i=0;i<cnt;++i) compSizes.push_back({i, sizes[i]});
        return compSizes;
    }
    vector<Edge> k_shortest_mst_variants(int k){
        vector<Edge> base = mst_kruskal().first;
        vector<Edge> result = base;
        for(int attempt=0; attempt<k-1; ++attempt){
            if(edges.empty()) break;
            int ridx = attempt % edges.size();
            Edge blocked = edges[ridx];
            vector<Edge> cand;
            double total=0.0;
            vector<int> ord(edges.size());
            for(size_t i=0;i<edges.size(); ++i) ord[i]=i;
            sort(ord.begin(), ord.end(), [&](int a,int b){ if(edges[a].id==blocked.id) return false; if(edges[b].id==blocked.id) return true; if(edges[a].w==edges[b].w) return edges[a].id < edges[b].id; return edges[a].w < edges[b].w; });
            DSU dsu(n);
            for(int idx: ord){
                auto &e = edges[idx];
                if(e.id == blocked.id) continue;
                if(dsu.unite(e.u, e.v)){
                    cand.push_back(e);
                    total += e.w;
                    if((int)cand.size() == n-1) break;
                }
            }
            if((int)cand.size()==n-1){
                for(auto &e: cand) result.push_back(e);
            }
        }
        return result;
    }
};

struct ShortestPath {
    int n;
    vector<vector<pair<int,double>>> adj;
    ShortestPath():n(0){}
    void build_from_adj(const vector<vector<pair<int,double>>> &adj_){ adj = adj_; n = adj.size(); }
    pair<vector<double>, vector<int>> dijkstra(int s){
        const double INF = 1e18;
        vector<double> dist(n, INF);
        vector<int> par(n, -1);
        using P = pair<double,int>;
        priority_queue<P, vector<P>, greater<P>> pq;
        if(s<0 || s>=n) return {dist,par};
        dist[s]=0.0;
        pq.push({0.0, s});
        while(!pq.empty()){
            auto [d,u] = pq.top(); pq.pop();
            if(d != dist[u]) continue;
            for(auto &pr: adj[u]){
                int v = pr.first;
                double w = pr.second;
                if(dist[v] > d + w){
                    dist[v] = d + w;
                    par[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }
        return {dist, par};
    }
    vector<int> path_from_par(const vector<int> &par, int t){
        vector<int> path;
        if(t<0 || t>=n) return path;
        for(int cur=t; cur!=-1; cur=par[cur]) path.push_back(cur);
        reverse(path.begin(), path.end());
        return path;
    }
};

struct IOHelper {
    static vector<string> split_ws(const string &s){
        vector<string> out; string tmp; stringstream ss(s);
        while(ss>>tmp) out.push_back(tmp);
        return out;
    }
    static string now_iso(){
        auto t = chrono::system_clock::now();
        time_t tt = chrono::system_clock::to_time_t(t);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&tt));
        return string(buf);
    }
    static void write_mst_to_csv(const string &path, const vector<Edge> &mst){
        ofstream out(path);
        out<<"u,v,w,id\n";
        for(auto &e: mst) out<<e.u<<","<<e.v<<","<<e.w<<","<<e.id<<"\n";
        out.close();
    }
    static void write_components_csv(const string &path, const vector<pair<int,int>> &comps){
        ofstream out(path);
        out<<"component_id,size\n";
        for(auto &p: comps) out<<p.first<<","<<p.second<<"\n";
        out.close();
    }
    static void write_edges_csv(const string &path, const vector<Edge> &edges){
        ofstream out(path);
        out<<"u,v,w,id\n";
        for(auto &e: edges) out<<e.u<<","<<e.v<<","<<e.w<<","<<e.id<<"\n";
        out.close();
    }
};

struct Bench {
    static long long now_ms(){
        return chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
    }
    static void time_fn(function<void()> fn, const string &label){
        long long s = now_ms();
        fn();
        long long e = now_ms();
        cerr<<label<<" "<<(e-s)<<" ms\n";
    }
};

struct Simulator {
    KruskalSolver solver;
    vector<Edge> mst;
    vector<vector<pair<int,double>>> adj_mst;
    vector<vector<pair<int,double>>> adj_full;
    ShortestPath sp;
    Simulator(){}
    void init(const string &edges_csv){
        solver.load_from_csv(edges_csv);
        auto tmp = solver.mst_kruskal();
        mst = tmp.first;
        adj_mst = solver.build_adj_from_mst(mst);
        adj_full = solver.build_adj_full();
        sp.build_from_adj(adj_full);
    }
    void refresh(){
        auto tmp = solver.mst_kruskal();
        mst = tmp.first;
        adj_mst = solver.build_adj_from_mst(mst);
        adj_full = solver.build_adj_full();
        sp.build_from_adj(adj_full);
    }
    void run_routing_batch(const vector<pair<int,int>> &pairs, const string &outcsv){
        ofstream out(outcsv);
        out<<"s,t,cost,path\n";
        for(auto &pr: pairs){
            int s = pr.first; int t = pr.second;
            auto res = sp.dijkstra(s);
            auto &dist = res.first; auto &par = res.second;
            if(t >=0 && t < (int)dist.size() && dist[t] < 1e17){
                auto path = sp.path_from_par(par, t);
                out<<s<<","<<t<<","<<dist[t]<<",\"";
                for(size_t i=0;i<path.size();++i){ if(i) out<<"-"; out<<path[i]; }
                out<<"\"\n";
            } else {
                out<<s<<","<<t<<",INF,\n";
            }
        }
        out.close();
    }
    vector<pair<int,double>> nearest_k_from(int s, int k){
        auto res = sp.dijkstra(s);
        auto &dist = res.first;
        vector<pair<int,double>> nodes;
        for(int i=0;i<(int)dist.size();++i) if(i!=s && dist[i] < 1e17) nodes.push_back({i, dist[i]});
        sort(nodes.begin(), nodes.end(), [](auto &a, auto &b){ if(a.second==b.second) return a.first < b.first; return a.second < b.second; });
        if((int)nodes.size() > k) nodes.resize(k);
        return nodes;
    }
    vector<pair<int,int>> generate_random_pairs(int m){
        vector<pair<int,int>> out;
        int n = max(1, solver.n);
        mt19937_64 rng((uint64_t)chrono::high_resolution_clock::now().time_since_epoch().count());
        uniform_int_distribution<int> d(0, n-1);
        for(int i=0;i<m;++i){
            int a = d(rng);
            int b = d(rng);
            out.push_back({a,b});
        }
        return out;
    }
    void stress_test(int ops){
        mt19937_64 rng((uint64_t)chrono::high_resolution_clock::now().time_since_epoch().count());
        uniform_int_distribution<int> dd(1,100);
        for(int i=0;i<ops;++i){
            int t = dd(rng)%4;
            if(t==0){
                auto p = generate_random_pairs(50);
                run_routing_batch(p, "/tmp/kruskal_batch.csv");
            } else if(t==1){
                solver.edges.insert(solver.edges.end(), {0,0, (double)(dd(rng)%100), (int)solver.edges.size()});
                refresh();
            } else if(t==2){
                auto comps = solver.cluster_by_threshold(10.0);
                IOHelper::write_components_csv("/tmp/components.csv", comps);
            } else {
                auto nn = nearest_k_from(0, 5);
            }
            if(i%50==0) refresh();
        }
    }
};

int main(int argc,char**argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(argc < 2){
        cerr<<"Usage: "<<argv[0]<<" <edges-csv>\n";
        return 1;
    }
    string csv = argv[1];
    Simulator sim;
    Bench::time_fn([&](){ sim.init(csv); }, "init");
    cout<<"Nodes="<<sim.solver.n<<" Edges="<<sim.solver.edges.size()<<"\n";
    cout<<"Commands:\nshowmst\nwritemst out.csv\ncomponents thresh\ncluster thresh\nkshort k\nroute s t\nbatch n out.csv\nnearest s k\nstress n\nexport edges out.csv\nexit\n";
    string line;
    while(true){
        cout<<"> ";
        if(!getline(cin, line)) break;
        if(line.empty()) continue;
        vector<string> parts = IOHelper::split_ws(line);
        if(parts.empty()) continue;
        string cmd = parts[0];
        if(cmd=="exit" || cmd=="quit") break;
        if(cmd=="showmst"){
            for(auto &e: sim.mst) cout<<e.u<<","<<e.v<<","<<e.w<<","<<e.id<<"\n";
            continue;
        }
        if(cmd=="writemst"){
            if(parts.size()<2){ cout<<"writemst out.csv\n"; continue; }
            IOHelper::write_mst_to_csv(parts[1], sim.mst);
            cout<<"wrote "<<parts[1]<<"\n";
            continue;
        }
        if(cmd=="components"){
            if(parts.size()<2){ cout<<"components thresh\n"; continue; }
            double thr = stod(parts[1]);
            auto comps = sim.solver.cluster_by_threshold(thr);
            for(auto &c: comps) cout<<c.first<<","<<c.second<<"\n";
            continue;
        }
        if(cmd=="cluster"){
            if(parts.size()<2){ cout<<"cluster thresh\n"; continue; }
            double thr = stod(parts[1]);
            auto comps = sim.solver.cluster_by_threshold(thr);
            IOHelper::write_components_csv("components.csv", comps);
            cout<<"wrote components.csv\n";
            continue;
        }
        if(cmd=="kshort"){
            if(parts.size()<2){ cout<<"kshort k\n"; continue; }
            int k = stoi(parts[1]);
            auto v = sim.solver.k_shortest_mst_variants(k);
            IOHelper::write_edges_csv("kshort_edges.csv", v);
            cout<<"wrote kshort_edges.csv count="<<v.size()<<"\n";
            continue;
        }
        if(cmd=="route"){
            if(parts.size()<3){ cout<<"route s t\n"; continue; }
            int s = stoi(parts[1]); int t = stoi(parts[2]);
            auto res = sim.sp.dijkstra(s);
            auto &dist = res.first; auto &par = res.second;
            if(t<0 || t>= (int)dist.size() || dist[t] > 1e17){ cout<<"unreachable\n"; continue; }
            auto path = sim.sp.path_from_par(par, t);
            cout<<"cost="<<dist[t]<<" path:";
            for(size_t i=0;i<path.size(); ++i){ if(i) cout<<"-"; cout<<path[i]; }
            cout<<"\n";
            continue;
        }
        if(cmd=="batch"){
            if(parts.size()<3){ cout<<"batch n out.csv\n"; continue; }
            int n = stoi(parts[1]);
            string out = parts[2];
            auto pairs = sim.generate_random_pairs(n);
            sim.run_routing_batch(pairs, out);
            cout<<"wrote "<<out<<"\n";
            continue;
        }
        if(cmd=="nearest"){
            if(parts.size()<3){ cout<<"nearest s k\n"; continue; }
            int s = stoi(parts[1]); int k = stoi(parts[2]);
            auto v = sim.nearest_k_from(s, k);
            for(auto &p: v) cout<<p.first<<","<<p.second<<"\n";
            continue;
        }
        if(cmd=="stress"){
            if(parts.size()<2){ cout<<"stress n\n"; continue; }
            int n = stoi(parts[1]);
            Bench::time_fn([&](){ sim.stress_test(n); }, "stress");
            cout<<"stress done\n";
            continue;
        }
        if(cmd=="export"){
            if(parts.size()<3){ cout<<"export edges out.csv\n"; continue; }
            if(parts[1]=="edges"){ IOHelper::write_edges_csv(parts[2], sim.solver.edges); cout<<"wrote "<<parts[2]<<"\n"; }
            else cout<<"unknown export\n";
            continue;
        }
        cout<<"unknown\n";
    }
    return 0;
}
