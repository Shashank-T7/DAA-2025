#include <bits/stdc++.h>
using namespace std;
using ll = long long;
struct WP { int id; double x,y; double wind; int importance; };
double euclid(const WP &a,const WP &b){ double dx=a.x-b.x, dy=a.y-b.y; return sqrt(dx*dx+dy*dy); }
vector<string> split_csv(const string &s){
    vector<string> out; string cur; bool inq=false;
    for(size_t i=0;i<s.size();++i){
        char c=s[i];
        if(inq){
            if(c=='"'){ if(i+1<s.size() && s[i+1]=='"'){ cur.push_back('"'); ++i; } else inq=false; } else cur.push_back(c);
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

vector<int> nearest_neighbor(const vector<WP>&wps){
    int n = wps.size();
    if(n==0) return {};
    vector<int> ids(n);
    iota(ids.begin(), ids.end(), 0);
    vector<char> used(n,0);
    vector<int> tour;
    tour.reserve(n);
    int cur = 0;
    used[cur]=1; tour.push_back(cur);
    for(int step=1; step<n; ++step){
        double best = 1e300; int bi = -1;
        for(int j=0;j<n;++j) if(!used[j]){
            double d = euclid(wps[cur], wps[j]) * (1.0 + wps[j].wind/30.0);
            if(d < best){ best = d; bi = j; }
        }
        if(bi==-1) break;
        used[bi]=1; tour.push_back(bi); cur=bi;
    }
    return tour;
}
double tour_len(const vector<int>&tour,const vector<WP>&wps){
    if(tour.empty()) return 0.0;
    double s=0;
    for(size_t i=0;i+1<tour.size();++i) s += euclid(wps[tour[i]], wps[tour[i+1]]) * (1.0 + wps[tour[i+1]].wind/30.0);
    s += euclid(wps[tour.back()], wps[tour.front()]) * (1.0 + wps[tour.front()].wind/30.0);
    return s;
}
void two_opt(vector<int>&tour,const vector<WP>&wps){
    int n=tour.size();
    if(n<4) return;
    bool improved=true;
    while(improved){
        improved=false;
        for(int i=0;i<n-1 && !improved;++i){
            for(int k=i+2;k<n && !improved;++k){
                int a=tour[i], b=tour[(i+1)%n], c=tour[k% n], d=tour[(k+1)%n];
                double before = euclid(wps[a],wps[b])*(1+wps[b].wind/30.0) + euclid(wps[c],wps[d])*(1+wps[d].wind/30.0);
                double after  = euclid(wps[a],wps[c])*(1+wps[c].wind/30.0) + euclid(wps[b],wps[d])*(1+wps[d].wind/30.0);
                if(after + 1e-9 < before){ reverse(tour.begin()+i+1, tour.begin()+k+1); improved=true; }
            }
        }
    }
}
vector<vector<int>> split_by_battery(const vector<int>&tour,const vector<WP>&wps,double max_dist){
    vector<vector<int>> missions;
    double cur=0; vector<int> part;
    if(tour.empty()) return missions;
    for(size_t i=0;i<tour.size();++i){
        int id = tour[i];
        double leg = (i==0 ? 0 : euclid(wps[tour[i-1]], wps[id]) * (1 + wps[id].wind/30.0));
        if(!part.empty() && cur + leg > max_dist){
            missions.push_back(part); part.clear(); cur = 0;
        }
        part.push_back(id); cur += leg;
    }
    if(!part.empty()) missions.push_back(part);
    return missions;
}
struct PQItem{ double d; int v; bool operator<(const PQItem&other)const{return d>other.d;} };
vector<double> dijkstra_weighted(const vector<WP>&wps,int src){
    int n=wps.size();
    vector<double> dist(n, 1e300);
    priority_queue<PQItem>pq;
    dist[src]=0; pq.push({0,src});
    while(!pq.empty()){
        auto it=pq.top(); pq.pop();
        double d=it.d; int u=it.v;
        if(d>dist[u]+1e-12) continue;
        for(int v=0; v<n; ++v){
            if(v==u) continue;
            double w = euclid(wps[u], wps[v]) * (1.0 + wps[v].wind/30.0);
            if(dist[u] + w < dist[v]){ dist[v]=dist[u]+w; pq.push({dist[v], v}); }
        }
    }
    return dist;
}
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    vector<string> lines; string row;
    while(getline(cin,row)) lines.push_back(row);
    if(lines.empty()) return 0;
    int first=0; while(first<(int)lines.size() && lines[first].find(',')==string::npos) first++;
    if(first>=(int)lines.size()) return 0;
    auto header = split_csv(lines[first]);
    unordered_map<string,int> h;
    for(int i=0;i<(int)header.size();++i){ string t=header[i]; for(auto &c:t) c=tolower((unsigned char)c); h[t]=i; }
    vector<WP> wps;
    int base_index = -1;
    for(int i=first+1;i<(int)lines.size();++i){
        if(lines[i].find_first_not_of(" \t\r\n")==string::npos) continue;
        auto f = split_csv(lines[i]);
        string cmd = "";
        if(h.count("command")) cmd = f[h["command"]];
        if(cmd.empty()){
            if(h.count("node_id") && h.count("x") && h.count("y")) cmd = "WAYPOINT";
        }
        string up = cmd; for(auto &c:up) c=toupper((unsigned char)c);
        if(up=="BASE"){
            WP b; b.id = to_int(f[h.count("node_id")?h["node_id"]:1]); b.x = to_double(f[h.count("x")?h["x"]:2]); b.y = to_double(f[h.count("y")?h["y"]:3]); b.wind = to_double(f[h.count("wind")?h["wind"]:4]); b.importance = to_int(f[h.count("importance")?h["importance"]:5]);
            wps.push_back(b);
            base_index = 0;
        } else if(up=="WAYPOINT"){
            WP p; p.id = to_int(f[h.count("node_id")?h["node_id"]:1]); p.x = to_double(f[h.count("x")?h["x"]:2]); p.y = to_double(f[h.count("y")?h["y"]:3]); p.wind = to_double(f[h.count("wind")?h["wind"]:4]); p.importance = to_int(f[h.count("importance")?h["importance"]:5]);
            wps.push_back(p);
        }
    }
    if(wps.empty()) return 0;
    int n = wps.size();
    cout<<"Loaded "<<n<<" waypoints (including base if present)\n";
    vector<int> tour = nearest_neighbor(wps);
    cout<<"Initial NN tour size "<<tour.size()<<" length "<<fixed<<setprecision(3)<<tour_len(tour,wps)<<"\n";
    two_opt(tour,wps);
    cout<<"After 2-opt tour length "<<fixed<<setprecision(3)<<tour_len(tour,wps)<<"\n";
    double battery_km = 30000.0;
    auto missions = split_by_battery(tour,wps,battery_km);
    cout<<"Split into "<<missions.size()<<" missions by battery limit "<<battery_km<<"\n";
    if(base_index<0) base_index = 0;
    cout<<"Computing Dijkstra emergency paths from each waypoint back to base (this may take time)\n";
    int src = base_index;
    auto dists = dijkstra_weighted(wps, src);
    cout<<"Distance from base to waypoint 0 = "<<dists[0]<<"\n";
    cout<<"Interactive commands: RUN_TSP, SPLIT val, EMERGENCY id, DIJKSTRA src dst, QUIT\n";
    string linecmd;
    while(getline(cin,linecmd)){
        if(linecmd.find_first_not_of(" \t\r\n")==string::npos) continue;
        stringstream ss(linecmd); string cmd; ss>>cmd;
        for(auto &c:cmd) c=toupper((unsigned char)c);
        if(cmd=="QUIT") break;
        else if(cmd=="RUN_TSP"){
            tour = nearest_neighbor(wps);
            two_opt(tour,wps);
            cout<<"Recomputed TSP length "<<tour_len(tour,wps)<<"\n";
        } else if(cmd=="SPLIT"){
            double val; if(!(ss>>val)){ cout<<"Usage: SPLIT max_distance\n"; continue; }
            missions = split_by_battery(tour,wps,val);
            cout<<"Split into "<<missions.size()<<" missions by battery "<<val<<"\n";
        } else if(cmd=="EMERGENCY"){
            int id; if(!(ss>>id)){ cout<<"Usage: EMERGENCY node_id\n"; continue; }
            if(id<0 || id>= (int)wps.size()){ cout<<"Invalid node\n"; continue; }
            auto dist_back = dijkstra_weighted(wps, id);
            cout<<"Emergency path length from "<<id<<" to base "<<dist_back[base_index]<<"\n";
        } else if(cmd=="DIJKSTRA"){
            int a,b; if(!(ss>>a>>b)){ cout<<"Usage: DIJKSTRA src dst\n"; continue; }
            if(a<0||a>=n||b<0||b>=n){ cout<<"Invalid nodes\n"; continue; }
            auto dd = dijkstra_weighted(wps,a);
            cout<<"Shortest approx distance from "<<a<<" to "<<b<<" = "<<dd[b]<<"\n";
        } else cout<<"Unknown command\n";
    }
    return 0;
}
