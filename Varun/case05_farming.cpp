// case05_farming.cpp
#include <bits/stdc++.h>
using namespace std;
struct Fenwick{ int n; vector<long long> bit; void init(int sz){ n=sz; bit.assign(n+1,0);} void add(int i,long long v){ for(++i;i<=n;i+=i&-i) bit[i]+=v;} long long sum(int i){ long long r=0; for(++i;i>0;i-=i&-i) r+=bit[i]; return r;} long long range(int l,int r){ if(r<l) return 0; return sum(r)-(l?sum(l-1):0);} };
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    return 0;
}
