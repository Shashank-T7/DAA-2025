#include <bits/stdc++.h>
using namespace std;

struct Edge { int u,v; double w; Edge(int a,int b,double c):u(a),v(b),w(c){} };
struct NodeDist { int node; double dist; NodeDist(int n,double d):node(n),dist(d){} bool operator<(const NodeDist& o) const { return dist>o.dist; } };

class Graph {
public:
    int n;
    vector<vector<pair<int,double>>> adj;
    vector<Edge> edges;
    Graph(int nn=0){ init(nn); }
    void init(int nn){ n=nn; adj.assign(n,{}); edges.clear(); }
    void addEdge(int u,int v,double w){ adj[u].push_back({v,w}); adj[v].push_back({u,w}); edges.emplace_back(u,v,w); }
    vector<double> dijkstra(int src) const {
        vector<double> dist(n,1e18); priority_queue<NodeDist> pq; dist[src]=0; pq.push(NodeDist(src,0));
        while(!pq.empty()){
            auto cur=pq.top(); pq.pop();
            if(cur.dist!=dist[cur.node]) continue;
            for(auto &pr: adj[cur.node]){
                int v=pr.first; double w=pr.second;
                if(dist[cur.node]+w<dist[v]){ dist[v]=dist[cur.node]+w; pq.push(NodeDist(v,dist[v])); }
            }
        }
        return dist;
    }
    int components() const {
        vector<int> vis(n,0); int c=0;
        for(int i=0;i<n;i++) if(!vis[i]){
            c++; stack<int> st; st.push(i); vis[i]=1;
            while(!st.empty()){
                int u=st.top(); st.pop();
                for(auto &pr: adj[u]) if(!vis[pr.first]){ vis[pr.first]=1; st.push(pr.first); }
            }
        }
        return c;
    }
};

struct Point { double x,y; int id; double demand; Point(){} Point(double a,double b,int i,double d=0):x(a),y(b),id(i),demand(d){} };

double euclid(const Point &a,const Point &b){ double dx=a.x-b.x, dy=a.y-b.y; return sqrt(dx*dx+dy*dy); }

vector<int> kmeans(const vector<Point>& pts, int k, int iter=100){
    int n=pts.size();
    vector<int> assign(n,0);
    if(k<=0) return assign;
    vector<Point> cent; cent.reserve(k);
    std::mt19937 rng((unsigned)chrono::high_resolution_clock::now().time_since_epoch().count());
    unordered_set<int> used;
    while((int)cent.size()<k){
        int idx = rng()%n;
        if(used.insert(idx).second) cent.emplace_back(pts[idx]);
    }
    for(int it=0; it<iter; ++it){
        bool changed=false;
        vector<int> cnt(k,0);
        vector<double> sx(k,0), sy(k,0);
        for(int i=0;i<n;i++){
            double best=1e18; int bi=0;
            for(int j=0;j<k;j++){
                double d=euclid(pts[i], cent[j]);
                if(d<best){ best=d; bi=j; }
            }
            if(assign[i]!=bi){ assign[i]=bi; changed=true; }
            cnt[bi]++; sx[bi]+=pts[i].x; sy[bi]+=pts[i].y;
        }
        for(int j=0;j<k;j++) if(cnt[j]>0){
            cent[j].x=sx[j]/cnt[j]; cent[j].y=sy[j]/cnt[j];
        }
        if(!changed) break;
    }
    return assign;
}

vector<int> nearest_neighbor_tour(const vector<int>& nodes, const vector<vector<double>>& distmat){
    if(nodes.empty()) return {};
    vector<int> tour; tour.reserve(nodes.size());
    unordered_set<int> left(nodes.begin(), nodes.end());
    int cur = nodes[0];
    tour.push_back(cur);
    left.erase(cur);
    while(!left.empty()){
        int best=-1; double bestd=1e18;
        for(int v: left) if(distmat[cur][v]<bestd){ bestd=distmat[cur][v]; best=v; }
        tour.push_back(best);
        left.erase(best);
        cur=best;
    }
    return tour;
}

double tourCost(const vector<int>& tour, const vector<vector<double>>& distmat){
    double s=0;
    for(size_t i=1;i<tour.size();++i) s+=distmat[tour[i-1]][tour[i]];
    if(tour.size()>1) s+=distmat[tour.back()][tour.front()];
    return s;
}

void twoOptImprove(vector<int>& tour, const vector<vector<double>>& distmat){
    int n=tour.size();
    if(n<4) return;
    bool improved=true;
    while(improved){
        improved=false;
        for(int i=0;i<n-2 && !improved;i++){
            for(int j=i+2;j<n && !improved;j++){
                if(i==0 && j==n-1) continue;
                double a=distmat[tour[i]][tour[i+1]];
                double b=distmat[tour[j]][tour[(j+1)%n]];
                double c=distmat[tour[i]][tour[j]];
                double d=distmat[tour[i+1]][tour[(j+1)%n]];
                if(c + d + 1e-9 < a + b){
                    reverse(tour.begin()+i+1, tour.begin()+j+1);
                    improved=true;
                }
            }
        }
    }
}

vector<vector<int>> splitByCapacity(const vector<int>& tourOrder, const vector<double>& demand, double capacity){
    vector<vector<int>> routes;
    vector<int> cur;
    double used=0;
    for(int v: tourOrder){
        if(used + demand[v] <= capacity){ cur.push_back(v); used+=demand[v]; }
        else {
            if(!cur.empty()) routes.push_back(cur);
            cur.clear();
            cur.push_back(v);
            used = demand[v];
        }
    }
    if(!cur.empty()) routes.push_back(cur);
    return routes;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int mode;
    if(!(cin>>mode)) return 0;

    if(mode==1){
        int n,m;
        cin>>n>>m;
        vector<Point> pts; pts.reserve(n);
        for(int i=0;i<n;i++){
            double x,y,d; cin>>x>>y>>d; pts.emplace_back(x,y,i,d);
        }
        Graph road(n);
        for(int i=0;i<m;i++){
            int u,v; double w; cin>>u>>v>>w; road.addEdge(u,v,w);
        }
        int k; double capacity; cin>>k>>capacity;
        if(road.components()>1){
            cout<<"UNREACHABLE\n";
            return 0;
        }
        vector<vector<double>> distmat(n, vector<double>(n,1e18));
        for(int i=0;i<n;i++){
            auto d = road.dijkstra(i);
            for(int j=0;j<n;j++) distmat[i][j]=d[j];
        }
        vector<int> assign = kmeans(pts, k, 200);
        vector<vector<int>> clusters(k);
        for(int i=0;i<n;i++) clusters[assign[i]].push_back(i);
        vector<vector<vector<int>>> allRoutes;
        double totalCost=0;
        for(int ci=0; ci<k; ++ci){
            if(clusters[ci].empty()) continue;
            vector<int> order = nearest_neighbor_tour(clusters[ci], distmat);
            twoOptImprove(order, distmat);
            auto routes = splitByCapacity(order, vector<double>([&]()->vector<double>{ vector<double> tmp(n); for(int i=0;i<n;i++) tmp[i]=pts[i].demand; return tmp; }()), capacity);
            if(routes.empty()){
                routes.push_back(order);
            } else {
                for(auto &r: routes){ twoOptImprove(r, distmat); }
            }
            for(auto &r: routes){
                double c = tourCost(r, distmat);
                totalCost += c;
            }
            allRoutes.push_back(routes);
        }
        cout.setf(std::ios::fixed); cout<<setprecision(6);
        cout<<totalCost<<"\n";
        for(auto &clusterRoutes: allRoutes){
            cout<<clusterRoutes.size()<<"\n";
            for(auto &r: clusterRoutes){
                cout<<r.size();
                for(int v: r) cout<<" "<<v;
                cout<<"\n";
            }
        }
    } else if(mode==2){
        int n,m; cin>>n>>m;
        vector<Point> pts; pts.reserve(n);
        for(int i=0;i<n;i++){ double x,y,d; cin>>x>>y>>d; pts.emplace_back(x,y,i,d); }
        Graph road(n);
        for(int i=0;i<m;i++){ int u,v; double w; cin>>u>>v>>w; road.addEdge(u,v,w); }
        if(road.components()>1){ cout<<"UNREACHABLE\n"; return 0; }
        vector<vector<double>> distmat(n, vector<double>(n,1e18));
        for(int i=0;i<n;i++){ auto d=road.dijkstra(i); for(int j=0;j<n;j++) distmat[i][j]=d[j]; }
        vector<int> nodes(n); iota(nodes.begin(), nodes.end(), 0);
        auto tour = nearest_neighbor_tour(nodes, distmat);
        twoOptImprove(tour, distmat);
        double cap; cin>>cap;
        auto routes = splitByCapacity(tour, vector<double>([&]()->vector<double>{ vector<double> tmp(n); for(int i=0;i<n;i++) tmp[i]=pts[i].demand; return tmp; }()), cap);
        cout<<routes.size()<<"\n";
        for(auto &r: routes){
            cout<<r.size();
            for(int v: r) cout<<" "<<v;
            cout<<"\n";
        }
    } else {
        return 0;
    }

    return 0;
}
