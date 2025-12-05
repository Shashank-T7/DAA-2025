#include <bits/stdc++.h>
using namespace std;
using db = double;
using ll = long long;
const db INF = 1e18;

struct Edge {
    int u,v;
    db w;
    Edge(){}
    Edge(int a,int b,db c):u(a),v(b),w(c){}
};

struct Graph {
    int n;
    vector<vector<pair<int,db>>> adj;
    vector<Edge> edges;
    Graph(int n_=0){init(n_);}
    void init(int N){
        n=N;
        adj.assign(n,{});
        edges.clear();
    }
    void addEdge(int u,int v,db w){
        adj[u].push_back({v,w});
        edges.emplace_back(u,v,w);
    }
};

vector<db> dijkstra(const Graph&g,int s){
    int n=g.n;
    vector<db>d(n,INF);
    priority_queue<pair<db,int>,vector<pair<db,int>>,greater<pair<db,int>>>pq;
    d[s]=0;pq.push({0,s});
    while(!pq.empty()){
        auto cur=pq.top();pq.pop();
        db cd=cur.first;int u=cur.second;
        if(cd!=d[u]) continue;
        for(auto &pr:g.adj[u]){
            int v=pr.first;db w=pr.second;
            if(cd+w<d[v]){d[v]=cd+w;pq.push({d[v],v});}
        }
    }
    return d;
}

vector<db> bellman_ford(int n,const vector<Edge>&E,int s){
    vector<db>d(n,INF);
    d[s]=0;
    for(int i=0;i<n-1;i++){
        bool upd=false;
        for(auto &e:E){
            if(d[e.u]<INF && d[e.u]+e.w<d[e.v]){
                d[e.v]=d[e.u]+e.w;
                upd=true;
            }
        }
        if(!upd) break;
    }
    for(int i=0;i<n;i++){
        for(auto &e:E){
            if(d[e.u]<INF && d[e.u]+e.w<d[e.v]){
                d[e.v]=-INF;
            }
        }
    }
    return d;
}

struct Fenwick {
    int n;
    vector<db> f;
    Fenwick(int n_=0){init(n_);}
    void init(int n_){
        n=n_;
        f.assign(n+1,0);
    }
    void update(int i, db v){
        for(;i<=n;i+=i&-i) f[i]+=v;
    }
    db query(int i){
        db s=0;
        for(;i>0;i-=i&-i) s+=f[i];
        return s;
    }
    db range(int l,int r){
        if(l>r) return 0;
        return query(r)-query(l-1);
    }
};

struct Battery {
    db cap;
    db cur;
    Battery(){}
    Battery(db c):cap(c),cur(c){}
    db discharge(db x){
        db t = min(cur,x);
        cur-=t;
        return t;
    }
    db charge(db x){
        db sp = min(x, cap-cur);
        cur+=sp;
        return sp;
    }
};

vector<db> build_load(int n,int seed){
    vector<db>L(n);
    mt19937_64 rng(seed);
    uniform_real_distribution<db>ud(10,40);
    for(int i=0;i<n;i++) L[i]=ud(rng);
    return L;
}

vector<db> build_supply(int n,int seed){
    vector<db>S(n);
    mt19937_64 rng(seed);
    uniform_real_distribution<db>ud(5,35);
    for(int i=0;i<n;i++) S[i]=ud(rng);
    return S;
}

db route_cost(const vector<int>&r,const vector<vector<db>>&D){
    db s=0;
    for(int i=0;i+1<r.size();i++) s+=D[r[i]][r[i+1]];
    return s;
}

vector<vector<db>> build_matrix(const vector<pair<db,db>>&pts){
    int n=pts.size();
    vector<vector<db>>D(n,vector<db>(n,INF));
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            if(i==j){D[i][j]=0;continue;}
            db dx=pts[i].first-pts[j].first;
            db dy=pts[i].second-pts[j].second;
            D[i][j]=sqrt(dx*dx+dy*dy);
        }
    }
    return D;
}

vector<int> greedy_discharge(const vector<db>&need, vector<Battery>&B){
    int n=need.size();
    vector<int>order(n);
    vector<pair<db,int>>arr;
    for(int i=0;i<n;i++){
        arr.push_back({need[i],i});
    }
    sort(arr.begin(),arr.end(),[&](auto&a,auto&b){return a.first>b.first;});
    for(int i=0;i<n;i++) order[i]=arr[i].second;
    for(int idx:order){
        db req = need[idx];
        for(auto &bat:B){
            if(req<=0) break;
            req -= bat.discharge(req);
        }
    }
    return order;
}

struct Step {
    db load;
    db supply;
    db net;
};

struct Simulation {
    vector<Step>steps;
    vector<Battery>B;
    Fenwick fw;
    Simulation(int nsteps,int nbatt):fw(nsteps){
        for(int i=0;i<nbatt;i++) B.push_back(Battery(200));
        fw.init(nsteps);
    }
};

Simulation run_sim(int n,int nbatt,const vector<db>&L,const vector<db>&S){
    Simulation sim(n,nbatt);
    for(int i=0;i<n;i++){
        db net = S[i]-L[i];
        sim.steps.push_back({L[i],S[i],net});
        sim.fw.update(i+1, net);
    }
    return sim;
}

vector<int> deficit_slots(const vector<Step>&st){
    vector<int>d;
    for(int i=0;i<st.size();i++){
        if(st[i].net<0) d.push_back(i);
    }
    return d;
}

db total_energy(const vector<Battery>&B){
    db s=0;
    for(auto &b:B) s+=b.cur;
    return s;
}

vector<int> multi_step_assignment(const vector<Step>&steps, vector<Battery>&B){
    int n=steps.size();
    vector<db>need(n);
    for(int i=0;i<n;i++) need[i]=max<db>(0,-steps[i].net);
    return greedy_discharge(need,B);
}

vector<db> prefix_supply(const vector<Step>&s){
    int n=s.size();
    vector<db>P(n);
    db sum=0;
    for(int i=0;i<n;i++){
        sum+=s[i].supply;
        P[i]=sum;
    }
    return P;
}

vector<db> prefix_load(const vector<Step>&s){
    int n=s.size();
    vector<db>P(n);
    db sum=0;
    for(int i=0;i<n;i++){
        sum+=s[i].load;
        P[i]=sum;
    }
    return P;
}

vector<db> sliding_def(const vector<Step>&s,int w){
    int n=s.size();
    vector<db>out(n);
    db sum=0;
    for(int i=0;i<n;i++){
        sum+=s[i].net;
        if(i>=w) sum-=s[i-w].net;
        out[i]=sum;
    }
    return out;
}

vector<pair<int,db>> rank_nodes(const vector<db>&base,const vector<db>&dyn){
    int n=base.size();
    vector<pair<db,int>>arr;
    for(int i=0;i<n;i++){
        arr.push_back({base[i]+dyn[i],i});
    }
    sort(arr.begin(),arr.end(),[&](auto&a,auto&b){return a.first<b.first;});
    vector<pair<int,db>>out;
    for(auto &x:arr) out.push_back({x.second,x.first});
    return out;
}

Graph build_micro(int n,int seed){
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

struct Mission {
    vector<int>path;
    db cost;
};

Mission best_route(int src,int dst,const Graph&g){
    auto d=dijkstra(g,src);
    if(d[dst]>=INF) return {{},INF};
    vector<int>prev(g.n,-1);
    vector<db>dd(d);
    priority_queue<pair<db,int>,vector<pair<db,int>>,greater<pair<db,int>>>pq;
    dd.assign(g.n,INF);
    dd[src]=0; pq.push({0,src});
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
    vector<int>P;
    if(dd[dst]>=INF) return {{},INF};
    int cur=dst;
    while(cur!=-1){ P.push_back(cur); cur=prev[cur]; }
    reverse(P.begin(),P.end());
    return {P,dd[dst]};
}

vector<db> detect_cycles(const Graph&g){
    vector<db>d = bellman_ford(g.n, g.edges, 0);
    return d;
}

vector<pair<int,db>> zone_pressure(const Simulation&sim){
    int n=sim.steps.size();
    vector<db>base(n), dyn(n);
    for(int i=0;i<n;i++){
        base[i]=sim.steps[i].load - sim.steps[i].supply;
        dyn[i]=sim.fw.query(i+1);
    }
    auto rk = rank_nodes(base,dyn);
    vector<pair<int,db>>out;
    for(auto &p:rk) out.push_back({p.first,p.second});
    return out;
}

vector<vector<int>> plan_storage(const vector<Step>&steps, vector<Battery>&B, db cap){
    int n=steps.size();
    vector<vector<int>>missions;
    vector<int>def = deficit_slots(steps);
    if(def.empty()) return missions;
    vector<db>need(n);
    for(int i=0;i<n;i++) need[i]=max<db>(0,-steps[i].net);
    vector<int>critical;
    for(int i:def){
        db t=need[i];
        for(auto &b:B){
            if(t<=0) break;
            t-=b.discharge(t);
        }
        if(need[i]>0) critical.push_back(i);
    }
    missions.push_back(critical);
    return missions;
}

struct Result {
    db total;
    vector<int>order;
    vector<vector<int>>missions;
};

Result simulate(const Graph&g,int nsteps,int nbatt){
    auto L = build_load(nsteps,5);
    auto S = build_supply(nsteps,9);
    Simulation sim = run_sim(nsteps,nbatt,L,S);
    vector<int>ord = multi_step_assignment(sim.steps,sim.B);
    vector<vector<int>>ms = plan_storage(sim.steps,sim.B,200.0);
    db tot = total_energy(sim.B);
    return {tot, ord, ms};
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int mode;
    if(!(cin>>mode)) return 0;

    if(mode==0){
        int n=50;
        Graph g = build_micro(30,3);
        auto res = simulate(g, n, 5);
        cout.setf(std::ios::fixed);
        cout<<setprecision(6);
        cout<<"ENERGY "<<res.total<<"\n";
        cout<<"ORDER "<<res.order.size()<<"\n";
        for(int x:res.order) cout<<x<<" ";
        cout<<"\n";
        cout<<"MS "<<res.missions.size()<<"\n";
        for(auto &m:res.missions){
            cout<<m.size()<<" ";
            for(int x:m) cout<<x<<" ";
            cout<<"\n";
        }
        return 0;
    }

    if(mode==1){
        int n,m;
        cin>>n>>m;
        Graph g(n);
        for(int i=0;i<m;i++){
            int u,v;db w;cin>>u>>v>>w;
            g.addEdge(u,v,w);
        }
        auto cyc = detect_cycles(g);
        for(auto &x:cyc) cout<<x<<" ";
        cout<<"\n";
        return 0;
    }

    if(mode==2){
        int n;
        cin>>n;
        vector<pair<db,db>>pts(n);
        for(int i=0;i<n;i++) cin>>pts[i].first>>pts[i].second;
        auto D = build_matrix(pts);
        vector<int>r(n);
        for(int i=0;i<n;i++) r[i]=i;
        db c=route_cost(r,D);
        cout.setf(std::ios::fixed);
        cout<<setprecision(6);
        cout<<c<<"\n";
        return 0;
    }

    if(mode==3){
        int n,nb;
        cin>>n>>nb;
        auto L = build_load(n,7);
        auto S = build_supply(n,11);
        Simulation sim = run_sim(n,nb,L,S);
        auto z = zone_pressure(sim);
        for(auto &p:z) cout<<p.first<<" "<<p.second<<"\n";
        return 0;
    }

    return 0;
}
