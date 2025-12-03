#include <bits/stdc++.h>
using namespace std;

struct Edge { int v; double w; };
static vector<tuple<int,int,double>> load_edges_generic(const string &path){
    ifstream in(path);
    vector<tuple<int,int,double>> out;
    if(!in.is_open()) return out;
    string line; if(!getline(in,line)) return out;
    while(getline(in,line)){
        vector<string> cols; string cur; bool inq=false;
        for(char c:line){ if(c=='"'){ inq=!inq; continue; } if(c==',' && !inq){ cols.push_back(cur); cur.clear(); } else cur.push_back(c); }
        cols.push_back(cur);
        if(cols.size()<3) continue;
        int u = stoi(cols[0]); int v = stoi(cols[1]); double w = stod(cols[2]);
        out.emplace_back(u,v,w);
    }
    return out;
}

struct Graph {
    int n;
    vector<vector<Edge>> adj;
    Graph(int n=0): n(n), adj(n) {}
    void add_edge(int u,int v,double w){ if(u>=0 && v>=0){ if(u>=n || v>=n){ int newn = max(n, max(u+1,v+1)); adj.resize(newn); n=newn; } adj[u].push_back({v,w}); adj[v].push_back({u,w}); } }
    pair<vector<double>, vector<int>> dijkstra(int s){
        const double INF = 1e18;
        vector<double> dist(n, INF);
        vector<int> par(n, -1);
        using P = pair<double,int>;
        priority_queue<P, vector<P>, greater<P>> pq;
        dist[s]=0; pq.push({0,s});
        while(!pq.empty()){
            auto [d,u]=pq.top(); pq.pop();
            if(d!=dist[u]) continue;
            for(auto &e: adj[u]){
                int v=e.v; double w=e.w;
                if(dist[v] > d + w){ dist[v]=d+w; par[v]=u; pq.push({dist[v], v}); }
            }
        }
        return {dist, par};
    }
    vector<int> reconstruct(const vector<int>&par, int t){
        vector<int> path; if(t<0||t>=n) return path;
        for(int cur=t; cur!=-1; cur=par[cur]) path.push_back(cur);
        reverse(path.begin(), path.end()); return path;
    }
};

static vector<long long> load_points(const string &path, int &nmax){
    ifstream in(path);
    vector<long long> out; nmax=-1;
    if(!in.is_open()) return out;
    string line; if(!getline(in,line)) return out;
    while(getline(in,line)){
        vector<string> cols; string cur; bool inq=false;
        for(char c:line){ if(c=='"'){ inq=!inq; continue; } if(c==',' && !inq){ cols.push_back(cur); cur.clear(); } else cur.push_back(c); }
        cols.push_back(cur);
        if(cols.size()<3) continue;
        int u = stoi(cols[0]); int v = stoi(cols[1]); double w = stod(cols[2]);
        nmax = max(nmax, max(u,v));
    }
    return out;
}

int main(int argc,char**argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(argc<2){ cerr<<"Usage: "<<argv[0]<<" <csv>\n"; return 1; }
    auto raw = load_edges_generic(argv[1]);
    if(raw.empty()){ cerr<<"no edges\n"; return 1; }
    int maxn=0;
    for(auto &t: raw){ int u,v; double w; tie(u,v,w)=t; maxn = max(maxn, max(u,v)); }
    Graph g(maxn+1);
    for(auto &t: raw){ int u,v; double w; tie(u,v,w)=t; g.add_edge(u,v,w); }
    cout<<"Graph nodes="<<g.n<<"\n";
    cout<<"Commands:\nshortest s t\nmulti s id1 id2 id3 ...\ninspect-tour start k\nexit\n";
    string line;
    while(true){
        cout<<"> ";
        if(!getline(cin,line)) break;
        if(line.empty()) continue;
        stringstream ss(line); string cmd; ss>>cmd;
        if(cmd=="exit"||cmd=="quit") break;
        if(cmd=="shortest"){
            int s,t; ss>>s>>t;
            if(s<0||s>=g.n||t<0||t>=g.n){ cout<<"invalid\n"; continue; }
            auto pr = g.dijkstra(s);
            auto &dist = pr.first; auto &par = pr.second;
            if(dist[t] > 1e17) { cout<<"unreachable\n"; continue; }
            auto path = g.reconstruct(par, t);
            cout<<"cost="<<dist[t]<<" path:";
            for(auto x:path) cout<<" "<<x;
            cout<<"\n";
            continue;
        }
        if(cmd=="multi"){
            int s; ss>>s;
            vector<int> targets; int x;
            while(ss>>x) targets.push_back(x);
            if(s<0||s>=g.n){ cout<<"invalid\n"; continue; }
            auto pr = g.dijkstra(s);
            auto &dist = pr.first;
            for(auto t: targets){
                if(t<0||t>=g.n) cout<<t<<":invalid\n"; else if(dist[t]>1e17) cout<<t<<":unreach\n"; else cout<<t<<":"<<dist[t]<<"\n";
            }
            continue;
        }
        if(cmd=="inspect-tour"){
            int start,k; ss>>start>>k;
            if(start<0||start>=g.n){ cout<<"invalid\n"; continue; }
            vector<int> remaining;
            for(int i=0;i<g.n;++i) remaining.push_back(i);
            vector<int> tour; tour.push_back(start);
            int cur = start;
            for(int step=0; step<k && !remaining.empty(); ++step){
                auto pr = g.dijkstra(cur);
                auto &dist = pr.first;
                int best=-1; double bestd=1e18;
                for(auto node: remaining){
                    if(node==cur) continue;
                    if(dist[node] < bestd){ bestd = dist[node]; best = node; }
                }
                if(best==-1) break;
                tour.push_back(best);
                remaining.erase(find(remaining.begin(), remaining.end(), best));
                cur = best;
            }
            cout<<"tour:";
            for(auto v:tour) cout<<" "<<v;
            cout<<"\n";
            continue;
        }
        cout<<"unknown\n";
    }
    return 0;
}
