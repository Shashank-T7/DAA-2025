#include <bits/stdc++.h>
using namespace std;
using ll = long long;
const ll INFLL = (ll)4e18;
struct Edge {
    int u;
    int v;
    ll cost;
    double benefit;
    bool active;
    Edge(): u(-1), v(-1), cost(0), benefit(0.0), active(true) {}
    Edge(int _u,int _v,ll _c,double _b): u(_u), v(_v), cost(_c), benefit(_b), active(true) {}
};
struct Graph {
    int n;
    vector<vector<int>> adj;
    vector<Edge> edges;
    Graph(): n(0) {}
    void init(int N) { n = N; adj.assign(n, {}); edges.clear(); }
    void addEdge(int u,int v,ll cost,double benefit) {
        if(u<0||v<0||u>=n||v>=n) return;
        edges.emplace_back(u,v,cost,benefit);
        int id = (int)edges.size()-1;
        adj[u].push_back(id);
        adj[v].push_back(id);
    }
};
struct DSU {
    int n;
    vector<int> p;
    vector<int> r;
    DSU(): n(0) {}
    void init(int N) { n = N; p.resize(n=N); r.assign(n,0); iota(p.begin(), p.end(), 0); }
    int find(int x) { return p[x]==x?x:p[x]=find(p[x]); }
    bool unite(int a,int b) { a=find(a); b=find(b); if(a==b) return false; if(r[a]<r[b]) swap(a,b); p[b]=a; if(r[a]==r[b]) r[a]++; return true; }
};
vector<string> split_csv(const string &s) {
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
    for(auto &t: out){
        size_t a=0,b=t.size();
        while(a<b && isspace((unsigned char)t[a])) ++a;
        while(b>a && isspace((unsigned char)t[b-1])) --b;
        t = t.substr(a, b-a);
    }
    return out;
}
int to_int(const string &s){ if(s.empty()) return 0; try{return stoi(s);}catch(...){ return 0; } }
ll to_ll(const string &s){ if(s.empty()) return 0; try{return stoll(s);}catch(...){ try{ double d=stod(s); return (ll)llround(d); }catch(...){return 0;} } }
double to_double(const string &s){ if(s.empty()) return 0.0; try{return stod(s);}catch(...){return 0.0;} }
void print_edge(const Edge &e){ cout<<"Edge "<<e.u<<"-"<<e.v<<" cost="<<e.cost<<" benefit="<<e.benefit<<"\n"; }
vector<int> bfs_component(Graph &g, int start, vector<char> &vis) {
    vector<int> comp;
    queue<int> q;
    q.push(start);
    vis[start]=1;
    while(!q.empty()){
        int u=q.front(); q.pop();
        comp.push_back(u);
        for(int id: g.adj[u]){
            Edge &e = g.edges[id];
            int v = e.u==u?e.v:e.u;
            if(!vis[v]){ vis[v]=1; q.push(v); }
        }
    }
    return comp;
}
vector<vector<int>> all_components(Graph &g) {
    vector<char> vis(g.n,0);
    vector<vector<int>> comps;
    for(int i=0;i<g.n;++i){
        if(!vis[i]){
            auto c = bfs_component(g, i, vis);
            if(!c.empty()) comps.push_back(c);
        }
    }
    return comps;
}
struct Candidate { int id; double ratio; ll cost; double benefit; Candidate(int i=0,double r=0,ll c=0,double b=0):id(i),ratio(r),cost(c),benefit(b){} };
vector<int> kruskal_mst(Graph &g, ll &total_cost_out) {
    vector<int> chosen;
    total_cost_out = 0;
    int n = g.n;
    if(n==0) return chosen;
    vector<int> idx(g.edges.size());
    iota(idx.begin(), idx.end(), 0);
    sort(idx.begin(), idx.end(), [&](int a,int b){ if(g.edges[a].cost!=g.edges[b].cost) return g.edges[a].cost < g.edges[b].cost; return g.edges[a].benefit > g.edges[b].benefit; });
    DSU d; d.init(n);
    for(int id: idx){
        Edge &e = g.edges[id];
        if(d.unite(e.u, e.v)){ chosen.push_back(id); total_cost_out += e.cost; if((int)chosen.size()==n-1) break; }
    }
    if((int)chosen.size() != n-1) { total_cost_out = INFLL; }
    return chosen;
}
vector<int> kruskal_mst_subset(Graph &g, const vector<int>&edge_ids, ll &total_cost_out) {
    vector<int> chosen;
    total_cost_out = 0;
    int n = g.n;
    vector<int> idx = edge_ids;
    sort(idx.begin(), idx.end(), [&](int a,int b){ if(g.edges[a].cost!=g.edges[b].cost) return g.edges[a].cost < g.edges[b].cost; return g.edges[a].benefit > g.edges[b].benefit; });
    DSU d; d.init(n);
    for(int id: idx){
        Edge &e = g.edges[id];
        if(d.unite(e.u, e.v)){ chosen.push_back(id); total_cost_out += e.cost; }
    }
    return chosen;
}
vector<int> greedy_select_by_ratio(Graph &g, ll budget) {
    vector<Candidate> cand;
    for(int i=0;i<(int)g.edges.size();++i){
        Edge &e = g.edges[i];
        if(e.cost <= 0) continue;
        double r = e.benefit / (double)e.cost;
        cand.emplace_back(i, r, e.cost, e.benefit);
    }
    sort(cand.begin(), cand.end(), [](const Candidate &a,const Candidate &b){
        if(fabs(a.ratio - b.ratio) > 1e-12) return a.ratio > b.ratio;
        if(a.benefit != b.benefit) return a.benefit > b.benefit;
        return a.cost < b.cost;
    });
    vector<int> selected;
    ll used = 0;
    for(auto &c: cand){
        if(used + c.cost <= budget){
            selected.push_back(c.id);
            used += c.cost;
        }
    }
    return selected;
}
vector<int> greedy_maximal_connectivity(Graph &g, ll budget) {
    vector<int> order(g.edges.size());
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a,int b){
        double ra = g.edges[a].benefit / (double)max<ll>(1,g.edges[a].cost);
        double rb = g.edges[b].benefit / (double)max<ll>(1,g.edges[b].cost);
        if(fabs(ra-rb) > 1e-12) return ra > rb;
        if(g.edges[a].benefit != g.edges[b].benefit) return g.edges[a].benefit > g.edges[b].benefit;
        return g.edges[a].cost < g.edges[b].cost;
    });
    DSU d; d.init(g.n);
    ll used = 0;
    vector<int> picked;
    for(int id: order){
        Edge &e = g.edges[id];
        if(used + e.cost > budget) continue;
        int a = d.find(e.u), b = d.find(e.v);
        if(a != b){
            d.unite(a,b);
            picked.push_back(id);
            used += e.cost;
        }
    }
    return picked;
}
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    vector<string> lines;
    string row;
    while(getline(cin,row)) lines.push_back(row);
    if(lines.empty()) return 0;
    int first_nonempty = 0;
    while(first_nonempty < (int)lines.size()){
        string t = lines[first_nonempty];
        bool allws = true;
        for(char c: t) if(!isspace((unsigned char)c)){ allws=false; break; }
        if(!allws) break;
        ++first_nonempty;
    }
    if(first_nonempty >= (int)lines.size()) return 0;
    vector<string> header = split_csv(lines[first_nonempty]);
    unordered_map<string,int> hmap;
    for(int i=0;i<(int)header.size();++i){ string key=header[i]; for(auto &c:key) c=tolower((unsigned char)c); hmap[key]=i; }
    Graph g;
    ll budget = 1000000;
    for(int i=first_nonempty+1;i<(int)lines.size();++i){
        string s = lines[i];
        if(s.find_first_not_of(" \t\r\n")==string::npos) continue;
        auto f = split_csv(s);
        string cmd = "";
        if(hmap.count("command")) cmd = f[hmap["command"]];
        if(cmd.empty()){
            if(hmap.count("u") && hmap.count("v") && hmap.count("cost")) cmd = "EDGE";
        }
        string cu = cmd;
        for(auto &c: cu) c = toupper((unsigned char)c);
        if(cu == "INIT"){
            int n = to_int(f[hmap.count("u")?hmap["u"]:0]);
            int m = to_int(f[hmap.count("v")?hmap["v"]:0]);
            if(n < 0) n = 0;
            g.init(n);
            cout<<"INIT n="<<n<<" m_est="<<m<<"\n";
        } else if(cu=="EDGE"){
            int u = to_int(f[hmap.count("u")?hmap["u"]:1]);
            int v = to_int(f[hmap.count("v")?hmap["v"]:2]);
            ll cost = to_ll(f[hmap.count("cost")?hmap["cost"]:3]);
            double benefit = to_double(f[hmap.count("benefit")?hmap["benefit"]:4]);
            if(g.n==0){
                int maxnode = max(u,v);
                g.init(maxnode+1);
            }
            g.addEdge(u,v,cost,benefit);
        } else if(cu=="BUDGET"){
            budget = to_ll(f[hmap.count("u")?hmap["u"]:0]);
            cout<<"Budget set to "<<budget<<"\n";
        } else if(cu=="RUN_KRUSKAL"){
            ll tot=0;
            auto chosen = kruskal_mst(g, tot);
            if(tot==INFLL) cout<<"Graph disconnected; MST not available\n"; else {
                cout<<"Kruskal MST total_cost="<<tot<<" edges="<<chosen.size()<<"\n";
            }
        } else if(cu=="COMPONENTS"){
            auto comps = all_components(g);
            cout<<"Detected "<<comps.size()<<" components\n";
            for(size_t ci=0;ci<comps.size();++ci){
                cout<<"Component "<<ci<<" size="<<comps[ci].size()<<"\n";
            }
        } else if(cu=="GREEDY_RATIO"){
            ll b = budget;
            auto picked = greedy_select_by_ratio(g, b);
            ll used=0; double totben=0;
            for(int id: picked){ used += g.edges[id].cost; totben += g.edges[id].benefit; }
            cout<<"Greedy ratio selected "<<picked.size()<<" edges used_cost="<<used<<" total_benefit="<<totben<<"\n";
        } else if(cu=="GREEDY_CONNECT"){
            auto picked = greedy_maximal_connectivity(g, budget);
            ll used=0; double totben=0;
            for(int id: picked){ used += g.edges[id].cost; totben += g.edges[id].benefit; }
            cout<<"Greedy connect selected "<<picked.size()<<" edges used_cost="<<used<<" total_benefit="<<totben<<"\n";
        } else if(cu=="EXPORT"){
            string mode = "";
            if(hmap.count("extra")) mode = f[hmap["extra"]];
            for(int i=0;i<(int)g.edges.size();++i){
                auto &e = g.edges[i];
                cout<<e.u<<" "<<e.v<<" "<<e.cost<<" "<<e.benefit<<"\n";
            }
        } else if(cu=="QUIT"){
            cout<<"QUIT\n";
            break;
        } else {
            // ignore unknown
        }
    }
    if(g.n==0){ cout<<"No graph loaded\n"; return 0; }
    cout<<"Summary: nodes="<<g.n<<" edges="<<g.edges.size()<<"\n";
    cout<<"Running default analyses: MST, components, greedy selection\n";
    ll mstcost=0;
    auto mst = kruskal_mst(g, mstcost);
    if(mstcost==INFLL) cout<<"MST not available (disconnected)\n"; else cout<<"MST cost="<<mstcost<<" edges="<<mst.size()<<"\n";
    auto comps = all_components(g);
    cout<<"Components: "<<comps.size()<<"\n";
    for(size_t ci=0; ci<comps.size(); ++ci) cout<<"Comp "<<ci<<" size="<<comps[ci].size()<<"\n";
    auto greedy1 = greedy_select_by_ratio(g, budget);
    ll used1=0; double ben1=0;
    for(int id: greedy1){ used1 += g.edges[id].cost; ben1 += g.edges[id].benefit; }
    cout<<"Greedy ratio: selected="<<greedy1.size()<<" used="<<used1<<" benefit="<<ben1<<"\n";
    auto greedy2 = greedy_maximal_connectivity(g, budget);
    ll used2=0; double ben2=0;
    for(int id: greedy2){ used2 += g.edges[id].cost; ben2 += g.edges[id].benefit; }
    cout<<"Greedy connectivity: selected="<<greedy2.size()<<" used="<<used2<<" benefit="<<ben2<<"\n";
    return 0;
}
