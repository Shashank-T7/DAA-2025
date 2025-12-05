// case08_microgrid.cpp
#include <bits/stdc++.h>
using namespace std;
struct Edge{int u,v; double w;};
bool bellmanFord(int n, vector<Edge>& edges, int src, vector<double>& dist){
    dist.assign(n, 1e18); dist[src]=0;
    for(int i=0;i<n-1;i++) for(auto &e: edges) if(dist[e.u]+e.w < dist[e.v]) dist[e.v]=dist[e.u]+e.w;
    for(auto &e: edges) if(dist[e.u]+e.w < dist[e.v]) return false;
    return true;
}
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    return 0;
}
