#include <bits/stdc++.h>
using namespace std;

struct Edge { int u,v; double w; Edge(int a=0,int b=0,double c=0):u(a),v(b),w(c){} };
struct DSU { vector<int> p; vector<int> r; DSU(int n=0){ p.resize(n); r.assign(n,0); for(int i=0;i<n;i++)p[i]=i; } int findSet(int x){ while(p[x]!=x){ p[x]=p[p[x]]; x=p[x]; } return x; } bool unite(int a,int b){ int A=findSet(a), B=findSet(b); if(A==B) return false; if(r[A]<r[B]) p[A]=B; else if(r[B]<r[A]) p[B]=A; else { p[B]=A; r[A]++; } return true; } };

class Landscape {
public:
    int n;
    vector<pair<double,double>> coords;
    vector<Edge> candidates;
    Landscape(int nn=0){ init(nn); }
    void init(int nn){ n=nn; coords.assign(n,{0,0}); candidates.clear(); }
    void addCoord(int id,double x,double y){ coords[id]={x,y}; }
    void addCandidate(int u,int v,double w){ candidates.emplace_back(u,v,w); }
    double kruskalBackbone(vector<Edge>* out=nullptr){
        auto e=candidates;
        sort(e.begin(), e.end(), [](const Edge&a,const Edge&b){ return a.w<b.w; });
        DSU d(n);
        double total=0; int used=0;
        for(auto &ed: e){
            if(d.unite(ed.u, ed.v)){ total+=ed.w; if(out) out->push_back(ed); if(++used==n-1) break; }
        }
        if(used<n-1) return numeric_limits<double>::infinity();
        return total;
    }
    int components(){
        vector<vector<int>> g(n);
        for(auto &e: candidates){ g[e.u].push_back(e.v); g[e.v].push_back(e.u); }
        vector<int> vis(n,0); int c=0;
        for(int i=0;i<n;i++) if(!vis[i]){ c++; stack<int> st; st.push(i); vis[i]=1; while(!st.empty()){ int u=st.top(); st.pop(); for(int v: g[u]) if(!vis[v]){ vis[v]=1; st.push(v); } } }
        return c;
    }
    vector<int> greedySelectByBenefit(int k, const vector<double>& benefit, const vector<double>& cost, double budget){
        int m = benefit.size();
        vector<int> idx(m);
        iota(idx.begin(), idx.end(), 0);
        sort(idx.begin(), idx.end(), [&](int a,int b){ double ra = benefit[a]/max(1e-9,cost[a]); double rb = benefit[b]/max(1e-9,cost[b]); if(ra==rb) return benefit[a]>benefit[b]; return ra>rb; });
        vector<int> sel;
        double used=0;
        for(int id: idx){
            if(used + cost[id] <= budget && (int)sel.size()<k){ sel.push_back(id); used += cost[id]; }
        }
        return sel;
    }
};

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n,m;
    if(!(cin>>n>>m)) return 0;
    Landscape L(n);
    for(int i=0;i<n;i++){ double x,y; cin>>x>>y; L.addCoord(i,x,y); }
    for(int i=0;i<m;i++){ int u,v; double w; cin>>u>>v>>w; L.addCandidate(u,v,w); }
    double backbone = L.kruskalBackbone();
    int comps = L.components();
    cout.setf(std::ios::fixed); cout<<setprecision(6);
    if(backbone==numeric_limits<double>::infinity()) cout<<"INF\n"; else cout<<backbone<<"\n";
    cout<<comps<<"\n";
    int k; cin>>k;
    vector<double> benefit(k), cost(k);
    for(int i=0;i<k;i++) cin>>benefit[i]>>cost[i];
    double budget; cin>>budget;
    auto sel = L.greedySelectByBenefit(k, benefit, cost, budget);
    for(int x: sel) cout<<x<<" ";
    cout<<"\n";
    return 0;
}
