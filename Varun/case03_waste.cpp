// case03_waste.cpp
#include <bits/stdc++.h>
using namespace std;
struct NodeDist{int node; double d; NodeDist(int n=0,double dd=0):node(n),d(dd){} bool operator<(NodeDist const& o) const{return d>o.d;}};
struct Graph{int n; vector<vector<pair<int,double>>> adj; Graph(int nn=0){init(nn);} void init(int nn){n=nn; adj.assign(n,{});} void addEdge(int u,int v,double w){adj[u].push_back({v,w}); adj[v].push_back({u,w});} vector<double> dijkstra(int src) const{ const double INF=1e18; vector<double> dist(n,INF); priority_queue<NodeDist> pq; dist[src]=0; pq.push({src,0}); while(!pq.empty()){ auto cur=pq.top(); pq.pop(); if(cur.d!=dist[cur.node]) continue; for(auto &pr: adj[cur.node]) if(dist[cur.node]+pr.second < dist[pr.first]){ dist[pr.first]=dist[cur.node]+pr.second; pq.push({pr.first, dist[pr.first]}); } } return dist; } int components() const{ vector<int> vis(n,0); int c=0; for(int i=0;i<n;i++) if(!vis[i]){ c++; stack<int> st; st.push(i); vis[i]=1; while(!st.empty()){ int u=st.top(); st.pop(); for(auto &pr: adj[u]) if(!vis[pr.first]){ vis[pr.first]=1; st.push(pr.first); } } } return c; } };
double tourCost(const vector<int>& tour, const vector<vector<double>>& d){ double s=0; for(size_t i=1;i<tour.size();++i) s+=d[tour[i-1]][tour[i]]; if(tour.size()>1) s+=d[tour.back()][tour.front()]; return s; }
vector<int> nearestNeighbour(const vector<int>& nodes, const vector<vector<double>>& d){ if(nodes.empty()) return {}; unordered_set<int> left(nodes.begin(), nodes.end()); int cur=*nodes.begin(); vector<int> tour{cur}; left.erase(cur); while(!left.empty()){ int best=-1; double bd=1e18; for(int v:left) if(d[cur][v]<bd){ bd=d[cur][v]; best=v; } tour.push_back(best); left.erase(best); cur=best; } return tour; }
void twoOpt(vector<int>& tour, const vector<vector<double>>& d){ int n=tour.size(); if(n<4) return; bool improved=true; while(improved){ improved=false; for(int i=0;i<n-2 && !improved;i++) for(int j=i+2;j<n && !improved;j++){ if(i==0 && j==n-1) continue; double a=d[tour[i]][tour[i+1]]; double b=d[tour[j]][tour[(j+1)%n]]; double c=d[tour[i]][tour[j]]; double e=d[tour[i+1]][tour[(j+1)%n]]; if(c+e+1e-9 < a+b){ reverse(tour.begin()+i+1, tour.begin()+j+1); improved=true; } } } }
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    return 0;
}
