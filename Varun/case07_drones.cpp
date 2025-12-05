// case07_drones.cpp
#include <bits/stdc++.h>
using namespace std;
double tourCost(const vector<int>& tour, const vector<vector<double>>& d){ double s=0; for(size_t i=1;i<tour.size();++i) s+=d[tour[i-1]][tour[i]]; if(tour.size()>1) s+=d[tour.back()][tour.front()]; return s; }
vector<int> nearest(const vector<int>& nodes, const vector<vector<double>>& d){ if(nodes.empty()) return {}; unordered_set<int> left(nodes.begin(), nodes.end()); int cur=nodes[0]; vector<int> tour{cur}; left.erase(cur); while(!left.empty()){ int best=-1; double bd=1e18; for(int v:left) if(d[cur][v]<bd){ bd=d[cur][v]; best=v; } tour.push_back(best); left.erase(best); cur=best; } return tour; }
void twoOpt(vector<int>& tour, const vector<vector<double>>& d){ int n=tour.size(); if(n<4) return; bool improved=true; while(improved){ improved=false; for(int i=0;i<n-2;i++) for(int j=i+2;j<n;j++){ if(i==0 && j==n-1) continue; double a=d[tour[i]][tour[i+1]], b=d[tour[j]][tour[(j+1)%n]], c=d[tour[i]][tour[j]], e=d[tour[i+1]][tour[(j+1)%n]]; if(c+e < a+b){ reverse(tour.begin()+i+1, tour.begin()+j+1); improved=true; } } } }
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    return 0;
}
