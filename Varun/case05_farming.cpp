#include <bits/stdc++.h>
using namespace std;
using db = double;
using ll = long long;

struct Fenwick {
    int n;
    vector<db> f;
    Fenwick(int n_=0){init(n_);}
    void init(int n_){
        n=n_;
        f.assign(n+1,0);
    }
    void update(int i, db v){
        for(; i<=n; i+=i&-i) f[i]+=v;
    }
    db query(int i){
        db s=0;
        for(; i>0; i-=i&-i) s+=f[i];
        return s;
    }
    db range(int l,int r){
        if(l>r) return 0;
        return query(r)-query(l-1);
    }
};

struct Seg {
    int n;
    vector<db> mn, mx;
    void init(int sz){
        n=1;
        while(n<sz) n<<=1;
        mn.assign(2*n, 1e18);
        mx.assign(2*n, -1e18);
    }
    void build(const vector<db>&a){
        int sz=a.size();
        init(sz);
        for(int i=0;i<sz;i++){
            mn[n+i]=a[i];
            mx[n+i]=a[i];
        }
        for(int i=n-1;i>0;i--){
            mn[i]=min(mn[i<<1],mn[i<<1|1]);
            mx[i]=max(mx[i<<1],mx[i<<1|1]);
        }
    }
    void update(int idx, db v){
        idx+=n;
        mn[idx]=v; mx[idx]=v;
        for(idx>>=1; idx; idx>>=1){
            mn[idx]=min(mn[idx<<1],mn[idx<<1|1]);
            mx[idx]=max(mx[idx<<1],mx[idx<<1|1]);
        }
    }
    pair<db,db> query(int l,int r){
        db amn=1e18, amx=-1e18;
        l+=n; r+=n;
        while(l<=r){
            if(l&1){
                amn=min(amn,mn[l]);
                amx=max(amx,mx[l]);
                l++;
            }
            if(!(r&1)){
                amn=min(amn,mn[r]);
                amx=max(amx,mx[r]);
                r--;
            }
            l>>=1; r>>=1;
        }
        return {amn,amx};
    }
};

struct Plot {
    db x,y;
    db moisture;
    db growth;
    db yieldv;
};

db dist_e(db x1,db y1,db x2,db y2){
    db dx=x1-x2;
    db dy=y1-y2;
    return sqrt(dx*dx+dy*dy);
}

vector<int> assign_kmeans(const vector<Plot>&p,const vector<pair<db,db>>&C){
    int n=p.size(), k=C.size();
    vector<int>a(n);
    for(int i=0;i<n;i++){
        db bd=1e18; int bc=0;
        for(int j=0;j<k;j++){
            db d=dist_e(p[i].x,p[i].y,C[j].first,C[j].second);
            if(d<bd){ bd=d; bc=j; }
        }
        a[i]=bc;
    }
    return a;
}

vector<pair<db,db>> update_centers(const vector<Plot>&p,const vector<int>&a,int k){
    vector<pair<db,db>>C(k,{0,0});
    vector<int>cnt(k,0);
    for(int i=0;i<p.size();i++){
        C[a[i]].first+=p[i].x;
        C[a[i]].second+=p[i].y;
        cnt[a[i]]++;
    }
    for(int j=0;j<k;j++){
        if(cnt[j]==0){
            C[j]={p[0].x,p[0].y};
        } else{
            C[j].first/=cnt[j];
            C[j].second/=cnt[j];
        }
    }
    return C;
}

vector<vector<int>> cluster_plots(const vector<Plot>&p,int k){
    int n=p.size();
    vector<pair<db,db>>C(k);
    for(int i=0;i<k;i++){
        C[i]={p[i].x,p[i].y};
    }
    vector<int>a(n);
    for(int it=0;it<20;it++){
        a = assign_kmeans(p,C);
        C = update_centers(p,a,k);
    }
    vector<vector<int>>out(k);
    for(int i=0;i<n;i++) out[a[i]].push_back(i);
    return out;
}

db sum_vec(const vector<db>&v){
    db s=0;
    for(db x:v) s+=x;
    return s;
}

db avg_vec(const vector<db>&v){
    if(v.empty()) return 0;
    return sum_vec(v)/v.size();
}

db stdv(const vector<db>&v){
    if(v.empty()) return 0;
    db m=avg_vec(v);
    db s=0;
    for(db x:v){ db d=x-m; s+=d*d; }
    return sqrt(s/v.size());
}

vector<db> moving_avg(const vector<db>&v,int w){
    int n=v.size();
    vector<db>out(n,0);
    db s=0;
    for(int i=0;i<n;i++){
        s+=v[i];
        if(i>=w) s-=v[i-w];
        if(i>=w-1) out[i]=s/w;
        else out[i]=s/(i+1);
    }
    return out;
}

vector<int> rank_yield(const vector<Plot>&p){
    int n=p.size();
    vector<pair<db,int>>v;
    for(int i=0;i<n;i++) v.push_back({p[i].yieldv,i});
    sort(v.begin(),v.end(),[&](auto&a,auto&b){return a.first>b.first;});
    vector<int>out;
    for(auto &x:v) out.push_back(x.second);
    return out;
}

vector<int> water_priority(const vector<Plot>&p){
    vector<pair<db,int>> v;
    for(int i=0;i<p.size();i++){
        db score = (1.0/(p[i].moisture+1)) + p[i].growth*0.5;
        v.push_back({score,i});
    }
    sort(v.begin(),v.end(),[&](auto&a,auto&b){return a.first>b.first;});
    vector<int>out;
    for(auto&s:v) out.push_back(s.second);
    return out;
}

vector<db> moisture_series(int steps,db base){
    vector<db>v(steps);
    mt19937_64 rng(1);
    uniform_real_distribution<db>ud(-0.5,0.5);
    db cur=base;
    for(int i=0;i<steps;i++){
        cur += ud(rng);
        if(cur<0) cur=0;
        v[i]=cur;
    }
    return v;
}

vector<db> growth_series(int steps,db base){
    vector<db>v(steps);
    mt19937_64 rng(2);
    uniform_real_distribution<db>ud(-0.2,0.2);
    db cur=base;
    for(int i=0;i<steps;i++){
        cur += ud(rng);
        v[i]=cur;
    }
    return v;
}

pair<db,db> stress_window(const vector<db>&g,int l,int r){
    db mn=1e18, mx=-1e18;
    for(int i=l;i<=r;i++){
        mn=min(mn,g[i]);
        mx=max(mx,g[i]);
    }
    return {mn,mx};
}

vector<int> detect_stress(const vector<db>&growth, db th_lo, db th_hi){
    vector<int>out;
    for(int i=0;i<growth.size();i++){
        if(growth[i]<th_lo || growth[i]>th_hi) out.push_back(i);
    }
    return out;
}

struct ZonePlan {
    vector<int> zone_plots;
    db avg_moist;
    db avg_growth;
    vector<int>priority;
};

ZonePlan build_zone(
    const vector<Plot>&p,
    const vector<int>&ids,
    const vector<db>&moist,
    const vector<db>&growth
){
    vector<db> mv, gv;
    for(int i:ids){
        mv.push_back(moist[i]);
        gv.push_back(growth[i]);
    }
    db am = avg_vec(mv);
    db ag = avg_vec(gv);
    vector<pair<db,int>>score;
    for(int i:ids){
        db s = (1.0/(p[i].moisture+1)) + p[i].growth*0.7;
        score.push_back({s,i});
    }
    sort(score.begin(),score.end(),[&](auto&a,auto&b){return a.first>b.first;});
    vector<int>pri;
    for(auto &x:score) pri.push_back(x.second);
    ZonePlan Z;
    Z.zone_plots = ids;
    Z.avg_moist = am;
    Z.avg_growth = ag;
    Z.priority = pri;
    return Z;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int mode;
    if(!(cin>>mode)) return 0;

    if(mode==0){
        int n=120;
        vector<Plot>p(n);
        mt19937_64 rng(7);
        uniform_real_distribution<db>ux(0,100), uy(0,100), um(10,40), ug(1,10), uyv(5,20);

        for(int i=0;i<n;i++){
            p[i].x=ux(rng);
            p[i].y=uy(rng);
            p[i].moisture=um(rng);
            p[i].growth=ug(rng);
            p[i].yieldv=uyv(rng);
        }

        auto clusters = cluster_plots(p,6);

        vector<db> moisture;
        vector<db> growth;

        for(int i=0;i<n;i++){
            auto ms = moisture_series(1, p[i].moisture);
            auto gs = growth_series(1, p[i].growth);
            moisture.push_back(ms[0]);
            growth.push_back(gs[0]);
        }

        Fenwick fw(n);
        fw.init(n);
        for(int i=1;i<=n;i++) fw.update(i, moisture[i-1]);

        Seg sg;
        sg.build(growth);

        vector<ZonePlan> Z;
        for(auto &ids: clusters){
            ZonePlan z = build_zone(p, ids, moisture, growth);
            Z.push_back(z);
        }

        auto ranked = rank_yield(p);
        auto wp = water_priority(p);

        int L=10,R=40;
        db sm = fw.range(L+1, R+1);
        auto qr = sg.query(L,R);

        cout<<fixed<<setprecision(6);
        cout<<"SUM_MOIST "<<sm<<"\n";
        cout<<"GROW_RANGE "<<qr.first<<" "<<qr.second<<"\n";
        cout<<"TOP_YIELD "<<ranked[0]<<" "<<ranked[1]<<" "<<ranked[2]<<"\n";
        cout<<"WATER_FIRST "<<wp[0]<<" "<<wp[1]<<" "<<wp[2]<<"\n";

        for(int i=0;i<Z.size();i++){
            cout<<"ZONE "<<i<<" "<<Z[i].avg_moist<<" "<<Z[i].avg_growth<<" "<<Z[i].priority[0]<<"\n";
        }

        return 0;
    }

    if(mode==1){
        int n;
        cin>>n;
        vector<db>a(n),b(n);
        for(int i=0;i<n;i++) cin>>a[i];
        for(int i=0;i<n;i++) cin>>b[i];
        Fenwick fw(n);
        fw.init(n);
        for(int i=1;i<=n;i++) fw.update(i,a[i-1]);
        Seg sg;
        sg.build(b);
        int q;
        cin>>q;
        while(q--){
            string t;
            cin>>t;
            if(t=="sum"){
                int l,r;
                cin>>l>>r;
                cout<<fw.range(l+1,r+1)<<"\n";
            } else if(t=="range"){
                int l,r;
                cin>>l>>r;
                auto qr = sg.query(l,r);
                cout<<qr.first<<" "<<qr.second<<"\n";
            }
        }
        return 0;
    }

    return 0;
}
