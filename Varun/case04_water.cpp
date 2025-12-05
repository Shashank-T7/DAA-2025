// case04_water.cpp
#include <bits/stdc++.h>
using namespace std;
struct Seg {
    int n; vector<double> mn, mx;
    void init(int sz){ n=1; while(n<sz) n<<=1; mn.assign(2*n,1e18); mx.assign(2*n,-1e18); }
    void update(int i,double v){ i+=n; mn[i]=mx[i]=v; for(i>>=1;i;i>>=1){ mn[i]=min(mn[i<<1], mn[i<<1|1]); mx[i]=max(mx[i<<1], mx[i<<1|1]); } }
    pair<double,double> query(int l,int r){ double a=1e18,b=-1e18; for(l+=n,r+=n;l<=r;l>>=1,r>>=1){ if(l&1) a=min(a,mn[l++]), b=max(b,mx[l-1]); if(!(r&1)) a=min(a,mn[r--]), b=max(b,mx[r+1]); } return {a,b}; }
};
vector<int> buildLPS(const string &p){ int m=p.size(); vector<int> lps(m); for(int i=1,j=0;i<m;){ if(p[i]==p[j]) lps[i++]=++j; else if(j>0) j=lps[j-1]; else lps[i++]=0; } return lps; }
bool kmpFind(const string &t,const string &p){ if(p.empty()) return true; auto lps=buildLPS(p); int n=t.size(), m=p.size(); for(int i=0,j=0;i<n;){ if(t[i]==p[j]){ i++; j++; if(j==m) return true; } else if(j>0) j=lps[j-1]; else i++; } return false; }
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    return 0;
}
