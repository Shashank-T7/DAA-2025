#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using db = double;
const db INF = 1e18;

struct Node { db x,y,vol; };
struct Graph {
    int n;
    vector<vector<pair<int,db>>> adj;
    Graph(int n_=0){ init(n_); }
    void init(int N){ n=N; adj.assign(n,{}); }
    void addEdge(int u,int v,db w){
        adj[u].push_back({v,w});
        adj[v].push_back({u,w});
    }
};

db dist_euclid(const Node&a,const Node&b){
    db dx=a.x-b.x, dy=a.y-b.y;
    return sqrt(dx*dx + dy*dy);
}

vector<db> dijkstra(const Graph &g,int s){
    int n=g.n;
    vector<db> d(n,INF);
    priority_queue<pair<db,int>,vector<pair<db,int>>,greater<pair<db,int>>>pq;
    d[s]=0; pq.push({0,s});
    while(!pq.empty()){
        auto cur=pq.top(); pq.pop();
        db cd=cur.first; int u=cur.second;
        if(cd!=d[u]) continue;
        for(auto &pr:g.adj[u]){
            int v=pr.first; db w=pr.second;
            if(cd+w < d[v]){ d[v]=cd+w; pq.push({d[v],v}); }
        }
    }
    return d;
}

vector<vector<db>> build_distance_matrix(const Graph &g){
    int n=g.n;
    vector<vector<db>> D(n, vector<db>(n, INF));
    for(int i=0;i<n;i++){
        auto d = dijkstra(g,i);
        for(int j=0;j<n;j++) D[i][j]=d[j];
    }
    return D;
}

db dist_path(const vector<int>&P,const vector<vector<db>>&D){
    if(P.empty()) return 0;
    db s=0;
    for(int i=0;i+1<(int)P.size();i++) s+=D[P[i]][P[i+1]];
    return s;
}

vector<int> tsp_nearest(const vector<int>&pts,int start,const vector<vector<db>>&D){
    int k=pts.size();
    vector<char> used(k,0);
    vector<int> route;
    int cur = start;
    used[start]=1;
    route.push_back(pts[start]);
    for(int i=1;i<k;i++){
        int best=-1; db bd=INF;
        for(int j=0;j<k;j++){
            if(!used[j]){
                db w=D[pts[cur]][pts[j]];
                if(w<bd){ bd=w; best=j; }
            }
        }
        used[best]=1;
        route.push_back(pts[best]);
        cur=best;
    }
    return route;
}

db improve_2opt(vector<int>&R,const vector<vector<db>>&D){
    int n=R.size();
    if(n<4) return dist_path(R,D);
    bool improved=true;
    while(improved){
        improved=false;
        for(int i=1;i<n-2;i++){
            for(int j=i+1;j<n-1;j++){
                db d1=D[R[i-1]][R[i]] + D[R[j]][R[j+1]];
                db d2=D[R[i-1]][R[j]] + D[R[i]][R[j+1]];
                if(d2<d1){
                    reverse(R.begin()+i, R.begin()+j+1);
                    improved=true;
                }
            }
        }
    }
    return dist_path(R,D);
}

struct Cluster {
    vector<int> ids;
    db cx,cy;
};

vector<int> assign_kmeans(const vector<Node>&pts,const vector<Cluster>&C){
    int n=pts.size(), k=C.size();
    vector<int>a(n);
    for(int i=0;i<n;i++){
        db bd=INF; int bc=0;
        for(int j=0;j<k;j++){
            db dx=pts[i].x - C[j].cx;
            db dy=pts[i].y - C[j].cy;
            db w = dx*dx+dy*dy;
            if(w<bd){ bd=w; bc=j; }
        }
        a[i]=bc;
    }
    return a;
}

vector<Cluster> update_centers(const vector<Node>&pts,const vector<int>&a,int k){
    vector<Cluster>C(k);
    vector<int> cnt(k,0);
    for(int i=0;i<k;i++){ C[i].cx=0; C[i].cy=0; }
    for(int i=0;i<(int)pts.size();i++){
        int c=a[i];
        C[c].cx+=pts[i].x;
        C[c].cy+=pts[i].y;
        cnt[c]++;
    }
    for(int j=0;j<k;j++){
        if(cnt[j]==0){ C[j].cx=pts[0].x; C[j].cy=pts[0].y; }
        else{
            C[j].cx/=cnt[j];
            C[j].cy/=cnt[j];
        }
    }
    return C;
}

vector<Cluster> kmeans(const vector<Node>&pts,int k){
    int n=pts.size();
    vector<Cluster>C(k);
    for(int i=0;i<k;i++){
        C[i].cx=pts[i% n].x;
        C[i].cy=pts[i% n].y;
    }
    vector<int> assign(n);
    for(int iter=0;iter<30;iter++){
        assign = assign_kmeans(pts,C);
        C = update_centers(pts, assign, k);
    }
    for(int i=0;i<k;i++) C[i].ids.clear();
    for(int i=0;i<n;i++) C[assign[i]].ids.push_back(i);
    return C;
}

vector<vector<int>> split_capacity(const vector<int>&R,const vector<Node>&pts,db cap){
    vector<vector<int>> S;
    db cur=0;
    vector<int> t;
    for(int u:R){
        if(cur + pts[u].vol > cap){
            S.push_back(t);
            t.clear();
            cur=0;
        }
        t.push_back(u);
        cur += pts[u].vol;
    }
    if(!t.empty()) S.push_back(t);
    return S;
}

struct Plan {
    vector<vector<int>> routes;
    db total;
};

Plan build_plan(const vector<Node>&pts,const vector<vector<db>>&D,const vector<int>&cluster,db cap){
    int k = cluster.size();
    if(k==0) return {{},0};
    vector<int> all = cluster;
    vector<int> idx(k);
    for(int i=0;i<k;i++) idx[i] = i;
    int start=0;
    vector<int> mapped; mapped.reserve(k);
    for(int i: idx) mapped.push_back(i);
    vector<int> R0 = tsp_nearest(mapped,start,D);
    vector<int> R1;
    for(int x: R0) R1.push_back(all[x]);
    db before = dist_path(R1,D);
    db after = improve_2opt(R1,D);
    vector<vector<int>> chunks = split_capacity(R1,pts,cap);
    db tot=0;
    for(auto &c:chunks) tot += dist_path(c,D);
    Plan P; P.routes=chunks; P.total=tot;
    return P;
}

Graph build_graph_from_coords(const vector<Node>&pts){
    int n=pts.size();
    Graph g(n);
    for(int i=0;i<n;i++){
        for(int j=i+1;j<n;j++){
            db w = dist_euclid(pts[i], pts[j]);
            g.addEdge(i,j,w);
        }
    }
    return g;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int mode; 
    if(!(cin>>mode)) return 0;

    if(mode==0){
        int n=50; 
        vector<Node> pts(n);
        mt19937_64 rng(1);
        uniform_real_distribution<db> ux(0,100), uy(0,100), uv(1,5);
        for(int i=0;i<n;i++){
            pts[i].x=ux(rng);
            pts[i].y=uy(rng);
            pts[i].vol=uv(rng);
        }
        int k=5;
        auto C = kmeans(pts,k);
        Graph g = build_graph_from_coords(pts);
        auto D = build_distance_matrix(g);
        db cap = 25.0;
        db total=0;
        for(int i=0;i<k;i++){
            Plan P = build_plan(pts, D, C[i].ids, cap);
            total += P.total;
        }
        cout.setf(std::ios::fixed); 
        cout<<setprecision(6)<<total<<"\n";
        return 0;
    }

    if(mode==1){
        int n,k;
        cin>>n>>k;
        vector<Node> pts(n);
        for(int i=0;i<n;i++){
            cin>>pts[i].x>>pts[i].y>>pts[i].vol;
        }
        auto C = kmeans(pts,k);
        Graph g = build_graph_from_coords(pts);
        auto D = build_distance_matrix(g);
        db cap;
        cin>>cap;
        db total=0;
        for(int i=0;i<k;i++){
            Plan P = build_plan(pts,D,C[i].ids,cap);
            total += P.total;
        }
        cout.setf(std::ios::fixed); 
        cout<<setprecision(6)<<total<<"\n";
        return 0;
    }

    if(mode==2){
        int n; cin>>n;
        vector<Node> pts(n);
        for(int i=0;i<n;i++) cin>>pts[i].x>>pts[i].y>>pts[i].vol;
        Graph g = build_graph_from_coords(pts);
        auto D=build_distance_matrix(g);
        vector<int> order(n);
        for(int i=0;i<n;i++) order[i]=i;
        auto R0 = tsp_nearest(order,0,D);
        improve_2opt(R0,D);
        for(int x:R0) cout<<x<<" ";
        cout<<"\n";
        return 0;
    }

    return 0;
}
