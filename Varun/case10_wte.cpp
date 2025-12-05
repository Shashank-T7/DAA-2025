#include <bits/stdc++.h>
using namespace std;

struct Item { int id; double energy; double mass; };
double knapsackGreedy(vector<Item> items, double capacity){
    sort(items.begin(), items.end(), [](const Item&a,const Item&b){ double ra=a.energy/a.mass, rb=b.energy/b.mass; if(ra==rb) return a.energy>b.energy; return ra>rb; });
    double used=0, value=0;
    for(auto &it: items){
        if(used + it.mass <= capacity){ used+=it.mass; value+=it.energy; }
        else { double can = capacity - used; if(can<=0) break; value += it.energy * (can/it.mass); used = capacity; break; }
    }
    return value;
}

struct NodeDist{ int v; double d; NodeDist(int a=0,double b=0):v(a),d(b){} bool operator<(const NodeDist& o) const { return d>o.d; } };

class Graph {
public:
    int n; vector<vector<pair<int,double>>> adj;
    Graph(int nn=0){ init(nn); }
    void init(int nn){ n=nn; adj.assign(n,{}); }
    void addEdge(int u,int v,double w){ adj[u].push_back({v,w}); adj[v].push_back({u,w}); }
    vector<double> dijkstra(int src) const {
        vector<double> dist(n,1e18); priority_queue<NodeDist> pq; dist[src]=0; pq.push(NodeDist(src,0));
        while(!pq.empty()){
            auto cur=pq.top(); pq.pop();
            if(cur.d!=dist[cur.v]) continue;
            for(auto &pr: adj[cur.v]){ int to=pr.first; double w=pr.second; if(dist[cur.v]+w < dist[to]){ dist[to]=dist[cur.v]+w; pq.push(NodeDist(to,dist[to])); } }
        }
        return dist;
    }
};

struct Segment {
    int n; vector<double> seg;
    Segment(int sz=0){ init(sz); }
    void init(int sz){ n=1; while(n<sz) n<<=1; seg.assign(2*n,0.0); }
    void add(int i,double v){ i+=n; seg[i]+=v; for(i>>=1;i;i>>=1) seg[i]=seg[i<<1]+seg[i<<1|1]; }
    double sumRange(int l,int r){ double res=0; for(l+=n,r+=n;l<=r;l>>=1,r>>=1){ if(l&1) res+=seg[l++]; if(!(r&1)) res+=seg[r--]; } return res; }
};

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int mode; if(!(cin>>mode)) return 0;
    if(mode==1){
        int n; cin>>n; vector<Item> items(n);
        for(int i=0;i<n;i++){ int id; double e,m; cin>>id>>e>>m; items[i]={id,e,m}; }
        double cap; cin>>cap;
        cout.setf(std::ios::fixed); cout<<setprecision(6);
        cout<<knapsackGreedy(items, cap)<<"\n";
    } else if(mode==2){
        int nodes, m; cin>>nodes>>m;
        Graph g(nodes);
        for(int i=0;i<m;i++){ int u,v; double w; cin>>u>>v>>w; g.addEdge(u,v,w); }
        int src; cin>>src;
        auto d = g.dijkstra(src);
        for(int i=0;i<nodes;i++) cout<<d[i]<<" ";
        cout<<"\n";
    } else if(mode==3){
        int T; cin>>T;
        Segment seg(T);
        seg.init(T);
        int ops; cin>>ops;
        while(ops--){
            int type; cin>>type;
            if(type==1){
                int t; double add; cin>>t>>add; seg.add(t,add);
            } else {
                int l,r; cin>>l>>r; cout<<seg.sumRange(l,r)<<"\n";
            }
        }
    }
    return 0;
}
