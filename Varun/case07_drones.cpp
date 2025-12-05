#include <bits/stdc++.h>
using namespace std;
using db = double;
using ll = long long;
const db INF = 1e18;

struct Node {
    db x,y;
    db wind;
    Node(){}
    Node(db a,db b,db c):x(a),y(b),wind(c){}
};

db dist_e(db x1,db y1,db x2,db y2){
    db dx=x1-x2, dy=y1-y2;
    return sqrt(dx*dx+dy*dy);
}

struct Graph {
    int n;
    vector<vector<pair<int,db>>> adj;
    Graph(int n_=0){init(n_);}
    void init(int N){
        n=N;
        adj.assign(n,{});
    }
    void addEdge(int u,int v,db w){
        adj[u].push_back({v,w});
        adj[v].push_back({u,w});
    }
};

vector<db> dijkstra(const Graph &g,int s){
    int n=g.n;
    vector<db>d(n,INF);
    priority_queue<pair<db,int>,vector<pair<db,int>>,greater<pair<db,int>>>pq;
    d[s]=0;pq.push({0,s});
    while(!pq.empty()){
        auto cur=pq.top(); pq.pop();
        db cd=cur.first; int u=cur.second;
        if(cd!=d[u]) continue;
        for(auto &pr:g.adj[u]){
            int v=pr.first; db w=pr.second;
            if(cd+w<d[v]){d[v]=cd+w; pq.push({d[v],v});}
        }
    }
    return d;
}

vector<vector<db>> build_matrix(const vector<Node>&pts){
    int n=pts.size();
    vector<vector<db>>D(n,vector<db>(n,INF));
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            if(i==j){D[i][j]=0;continue;}
            db w=dist_e(pts[i].x,pts[i].y,pts[j].x,pts[j].y);
            w *= (1.0 + pts[j].wind*0.05);
            D[i][j]=w;
        }
    }
    return D;
}

db route_cost(const vector<int>&R,const vector<vector<db>>&D){
    if(R.empty()) return 0;
    db s=0;
    for(int i=0;i+1<R.size();i++) s+=D[R[i]][R[i+1]];
    return s;
}

vector<int> nearest(const vector<int>&v,int start,const vector<vector<db>>&D){
    int k=v.size();
    vector<char>used(k,0);
    vector<int>out;
    used[start]=1;
    out.push_back(v[start]);
    int cur=start;
    for(int i=1;i<k;i++){
        int best=-1; db bd=INF;
        for(int j=0;j<k;j++){
            if(!used[j]){
                db w=D[v[cur]][v[j]];
                if(w<bd){bd=w;best=j;}
            }
        }
        used[best]=1;
        out.push_back(v[best]);
        cur=best;
    }
    return out;
}

db two_opt(vector<int>&R,const vector<vector<db>>&D){
    int n=R.size();
    if(n<4) return route_cost(R,D);
    bool ok=true;
    while(ok){
        ok=false;
        for(int i=1;i<n-2;i++){
            for(int j=i+1;j<n-1;j++){
                db d1=D[R[i-1]][R[i]]+D[R[j]][R[j+1]];
                db d2=D[R[i-1]][R[j]]+D[R[i]][R[j+1]];
                if(d2<d1){
                    reverse(R.begin()+i,R.begin()+j+1);
                    ok=true;
                }
            }
        }
    }
    return route_cost(R,D);
}

vector<vector<int>> split_battery(const vector<int>&R,const vector<vector<db>>&D,db cap){
    vector<vector<int>>out;
    db cur=0;
    vector<int>t;
    for(int u:R){
        if(!t.empty()){
            int pv=t.back();
            db w=D[pv][u];
            if(cur+w>cap){
                out.push_back(t);
                t.clear();
                cur=0;
            } else cur+=w;
        }
        if(t.empty()) t.push_back(u);
        else t.push_back(u);
    }
    if(!t.empty()) out.push_back(t);
    return out;
}

struct Flight {
    vector<int>path;
    db cost;
};

Flight best_flight(const vector<int>&pts,const vector<vector<db>>&D,db cap){
    int n=pts.size();
    vector<int>idx(n);
    for(int i=0;i<n;i++) idx[i]=i;
    vector<int>R=nearest(idx,0,D);
    vector<int>maproute;
    for(int x:R) maproute.push_back(pts[x]);
    two_opt(maproute,D);
    db cst=route_cost(maproute,D);
    if(cst<=cap) return {maproute,cst};
    auto sp=split_battery(maproute,D,cap);
    if(sp.empty()) return {{},0};
    return {sp[0], route_cost(sp[0],D)};
}

vector<vector<int>> full_mission(const vector<int>&pts,const vector<vector<db>>&D,db cap){
    vector<int>idx(pts.size());
    for(int i=0;i<pts.size();i++) idx[i]=i;
    vector<int>R=nearest(idx,0,D);
    vector<int>mapped;
    for(auto &x:R) mapped.push_back(pts[x]);
    two_opt(mapped,D);
    return split_battery(mapped,D,cap);
}

vector<int> emergency_path(int src,int dst,const Graph &g){
    auto d=dijkstra(g,src);
    vector<int>path;
    if(d[dst]>=INF) return path;
    vector<int>prev(g.n,-1);
    auto dd=d;
    priority_queue<pair<db,int>,vector<pair<db,int>>,greater<pair<db,int>>>pq;
    pq.push({0,src});
    dd[src]=0;
    while(!pq.empty()){
        auto cur=pq.top();pq.pop();
        db cd=cur.first; int u=cur.second;
        if(cd!=dd[u]) continue;
        if(u==dst) break;
        for(auto &pr:g.adj[u]){
            int v=pr.first; db w=pr.second;
            if(cd+w<dd[v]){ dd[v]=cd+w; prev[v]=u; pq.push({dd[v],v}); }
        }
    }
    int cur=dst;
    while(cur!=-1){ path.push_back(cur); cur=prev[cur]; }
    reverse(path.begin(),path.end());
    return path;
}

Graph build_ground_graph(int n,int seed){
    Graph g(n);
    mt19937_64 rng(seed);
    uniform_real_distribution<db>ud(1,10);
    for(int i=0;i<n;i++){
        for(int j=i+1;j<n;j++){
            db w=ud(rng);
            g.addEdge(i,j,w);
        }
    }
    return g;
}

vector<Node> random_pts(int n,int seed){
    vector<Node>v(n);
    mt19937_64 rng(seed);
    uniform_real_distribution<db>ux(0,100),uy(0,100),uw(0,5);
    for(int i=0;i<n;i++){
        v[i]=Node(ux(rng),uy(rng),uw(rng));
    }
    return v;
}

vector<int> reorder_by_wind(const vector<Node>&pts){
    int n=pts.size();
    vector<pair<db,int>>v;
    for(int i=0;i<n;i++) v.push_back({pts[i].wind,i});
    sort(v.begin(),v.end(),[&](auto&a,auto&b){return a.first<b.first;});
    vector<int>r;
    for(auto &x:v) r.push_back(x.second);
    return r;
}

vector<vector<int>> mission_by_wind(const vector<Node>&pts,const vector<vector<db>>&D,db cap){
    auto ord = reorder_by_wind(pts);
    vector<int>mapped;
    for(int x:ord) mapped.push_back(x);
    two_opt(mapped,D);
    return split_battery(mapped,D,cap);
}

struct MultiMission {
    vector<vector<int>> missions;
    db total;
};

MultiMission simulate_drone(const vector<Node>&pts,const vector<vector<db>>&D,db cap){
    vector<vector<int>>ms = full_mission(pts,D,cap);
    db s=0;
    for(auto &m:ms) s+=route_cost(m,D);
    return {ms,s};
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int mode;
    if(!(cin>>mode)) return 0;

    if(mode==0){
        int n=70;
        auto pts = random_pts(n,7);
        auto D = build_matrix(pts);
        db cap = 200.0;
        auto mm = simulate_drone(pts,D,cap);
        cout.setf(std::ios::fixed);
        cout<<setprecision(6);
        cout<<"TOTAL "<<mm.total<<"\n";
        cout<<"MISSIONS "<<mm.missions.size()<<"\n";
        for(auto &m:mm.missions){
            cout<<m.size()<<" ";
            for(int x:m) cout<<x<<" ";
            cout<<"\n";
        }
        return 0;
    }

    if(mode==1){
        int n;
        cin>>n;
        vector<Node>pts(n);
        for(int i=0;i<n;i++){
            cin>>pts[i].x>>pts[i].y>>pts[i].wind;
        }
        auto D=build_matrix(pts);
        db cap;cin>>cap;
        auto out=full_mission(pts,D,cap);
        cout<<out.size()<<"\n";
        for(auto &a:out){
            cout<<a.size()<<" ";
            for(int x:a) cout<<x<<" ";
            cout<<"\n";
        }
        return 0;
    }

    if(mode==2){
        int gsz;
        cin>>gsz;
        Graph g=build_ground_graph(gsz,10);
        int s,t;
        cin>>s>>t;
        auto p = emergency_path(s,t,g);
        cout<<p.size()<<"\n";
        for(int x:p) cout<<x<<" ";
        cout<<"\n";
        return 0;
    }

    return 0;
}
