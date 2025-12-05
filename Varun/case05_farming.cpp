#include <bits/stdc++.h>
using namespace std;

struct Fenwick {
    int n;
    vector<long long> bit;
    Fenwick(int sz=0){ init(sz); }
    void init(int sz){ n=sz; bit.assign(n+1,0); }
    void add(int idx,long long val){ for(++idx; idx<=n; idx+=idx&-idx) bit[idx]+=val; }
    long long sumPrefix(int idx){ long long r=0; for(++idx; idx>0; idx-=idx&-idx) r+=bit[idx]; return r; }
    long long rangeSum(int l,int r){ if(r<l) return 0; return sumPrefix(r) - (l?sumPrefix(l-1):0); }
};

struct Segment {
    int n;
    vector<long long> tree;
    Segment(int sz=0){ init(sz); }
    void init(int sz){ n=1; while(n<sz) n<<=1; tree.assign(2*n, LLONG_MAX); }
    void update(int i,long long v){ i+=n; tree[i]=v; for(i>>=1;i;i>>=1) tree[i]=min(tree[i<<1], tree[i<<1|1]); }
    long long queryMin(int l,int r){ long long res=LLONG_MAX; for(l+=n,r+=n; l<=r; l>>=1,r>>=1){ if(l&1) res=min(res,tree[l++]); if(!(r&1)) res=min(res,tree[r--]); } return res; }
};

struct Plot { int id; double x,y; long long yield; int lastUpdate; Plot():id(0),x(0),y(0),yield(0),lastUpdate(0){} };
double dist(double ax,double ay,double bx,double by){ double dx=ax-bx, dy=ay-by; return sqrt(dx*dx+dy*dy); }

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int mode;
    if(!(cin>>mode)) return 0;

    if(mode==1){
        int n; cin>>n;
        vector<Plot> plots(n);
        for(int i=0;i<n;i++){ double x,y; long long yld; cin>>x>>y>>yld; plots[i].id=i; plots[i].x=x; plots[i].y=y; plots[i].yield=yld; }
        Fenwick fw(n);
        for(int i=0;i<n;i++) fw.add(i, plots[i].yield);
        int q; cin>>q;
        while(q--){
            int t; cin>>t;
            if(t==1){
                int idx; long long add; cin>>idx>>add;
                plots[idx].yield += add;
                fw.add(idx, add);
            } else if(t==2){
                int l,r; cin>>l>>r;
                cout<<fw.rangeSum(l,r)<<"\n";
            } else if(t==3){
                vector<pair<long long,int>> v;
                for(int i=0;i<n;i++) v.push_back({plots[i].yield, plots[i].id});
                sort(v.begin(), v.end(), greater<pair<long long,int>>());
                int k; cin>>k;
                for(int i=0;i<k && i<(int)v.size(); ++i) cout<<v[i].second<<" "<<v[i].first<<"\n";
            }
        }
    } else if(mode==2){
        int n; cin>>n;
        vector<Plot> plots(n);
        for(int i=0;i<n;i++){ double x,y; long long yld; cin>>x>>y>>yld; plots[i].id=i; plots[i].x=x; plots[i].y=y; plots[i].yield=yld; }
        Segment seg(n);
        for(int i=0;i<n;i++) seg.update(i, plots[i].yield);
        int q; cin>>q;
        while(q--){
            int t; cin>>t;
            if(t==1){
                int idx; long long val; cin>>idx>>val;
                seg.update(idx,val);
            } else if(t==2){
                int l,r; cin>>l>>r;
                cout<<seg.queryMin(l,r)<<"\n";
            }
        }
    } else if(mode==3){
        int n; cin>>n;
        vector<Plot> plots(n);
        for(int i=0;i<n;i++){ double x,y; long long yld; cin>>x>>y>>yld; plots[i].id=i; plots[i].x=x; plots[i].y=y; plots[i].yield=yld; }
        int k; cin>>k;
        vector<int> centers;
        centers.reserve(k);
        centers.push_back(0);
        while((int)centers.size()<k){
            int best=-1; double bestd=1e18;
            for(int i=0;i<n;i++){
                bool found=false; for(int c:centers) if(c==i) { found=true; break; } if(found) continue;
                double mind=1e18;
                for(int c:centers) mind=min(mind, dist(plots[i].x,plots[i].y,plots[c].x,plots[c].y));
                if(mind>bestd) continue;
                best=i; bestd=mind;
            }
            if(best==-1) break;
            centers.push_back(best);
        }
        for(int c: centers) cout<<c<<" ";
        cout<<"\n";
    }
    return 0;
}
