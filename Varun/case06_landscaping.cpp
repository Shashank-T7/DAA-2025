// case06_landscaping.cpp
#include <bits/stdc++.h>
using namespace std;
struct Edge{int u,v; double w;};
struct DSU{vector<int> p; DSU(int n=0){ p.resize(n); for(int i=0;i<n;i++) p[i]=i;} int f(int x){ return p[x]==x?x:p[x]=f(p[x]); } bool unite(int a,int b){ a=f(a); b=f(b); if(a==b) return false; p[b]=a; return true; } };
double kruskal(int n, vector<Edge>& edges){
    sort(edges.begin(), edges.end(), [](auto&a,auto&b){ return a.w<b.w; });
    DSU d(n); double total=0; int used=0;
    for(auto &e: edges) if(d.unite(e.u, e.v)){ total+=e.w; if(++used==n-1) break; }
    return (used==n-1?total:-1.0);
}
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    return 0;
}
