#include <bits/stdc++.h>
using namespace std;
using db = double;
using ll = long long;
const db INF = 1e18;

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
    vector<db>mn,mx,sm;
    void init(int sz){
        n=1;
        while(n<sz) n<<=1;
        mn.assign(2*n,1e18);
        mx.assign(2*n,-1e18);
        sm.assign(2*n,0);
    }
    void build(const vector<db>&a){
        int sz=a.size();
        init(sz);
        for(int i=0;i<sz;i++){
            mn[n+i]=a[i];
            mx[n+i]=a[i];
            sm[n+i]=a[i];
        }
        for(int i=n-1;i>0;i--){
            mn[i]=min(mn[i<<1],mn[i<<1|1]);
            mx[i]=max(mx[i<<1],mx[i<<1|1]);
            sm[i]=sm[i<<1]+sm[i<<1|1];
        }
    }
    void update(int idx,db v){
        idx+=n;
        mn[idx]=v;mx[idx]=v;sm[idx]=v;
        for(idx>>=1;idx;idx>>=1){
            mn[idx]=min(mn[idx<<1],mn[idx<<1|1]);
            mx[idx]=max(mx[idx<<1],mx[idx<<1|1]);
            sm[idx]=sm[idx<<1]+sm[idx<<1|1];
        }
    }
    tuple<db,db,db> query(int l,int r){
        db amn=1e18, amx=-1e18, asm=0;
        l+=n; r+=n;
        while(l<=r){
            if(l&1){
                amn=min(amn,mn[l]);
                amx=max(amx,mx[l]);
                asm+=sm[l];
                l++;
            }
            if(!(r&1)){
                amn=min(amn,mn[r]);
                amx=max(amx,mx[r]);
                asm+=sm[r];
                r--;
            }
            l>>=1; r>>=1;
        }
        return {amn,amx,asm};
    }
};

struct Stream {
    vector<db>v;
    Fenwick fw;
    Seg sg;
    int n;
    Stream(int n_=0){init(n_);}
    void init(int n_){
        n=n_;
        v.assign(n,0);
        fw.init(n);
        sg.init(n);
    }
    void build(const vector<db>&a){
        n=a.size();
        v=a;
        fw.init(n);
        for(int i=1;i<=n;i++) fw.update(i,a[i-1]);
        sg.build(a);
    }
    void push_update(int i, db val){
        fw.update(i+1, val-v[i]);
        sg.update(i, val);
        v[i]=val;
    }
};

vector<db> sliding_max(const vector<db>&v,int w){
    int n=v.size();
    deque<int>dq;
    vector<db>out(n,0);
    for(int i=0;i<n;i++){
        while(!dq.empty() && dq.front()<=i-w) dq.pop_front();
        while(!dq.empty() && v[dq.back()]<=v[i]) dq.pop_back();
        dq.push_back(i);
        out[i]=v[dq.front()];
    }
    return out;
}

vector<db> sliding_min(const vector<db>&v,int w){
    int n=v.size();
    deque<int>dq;
    vector<db>out(n,0);
    for(int i=0;i<n;i++){
        while(!dq.empty() && dq.front()<=i-w) dq.pop_front();
        while(!dq.empty() && v[dq.back()]>=v[i]) dq.pop_back();
        dq.push_back(i);
        out[i]=v[dq.front()];
    }
    return out;
}

vector<db> sliding_sum(const vector<db>&v,int w){
    int n=v.size();
    vector<db>out(n,0);
    db s=0;
    for(int i=0;i<n;i++){
        s+=v[i];
        if(i>=w) s-=v[i-w];
        out[i]=s;
    }
    return out;
}

vector<pair<int,db>> rank_locations(const vector<db>&v){
    vector<pair<db,int>>a;
    for(int i=0;i<v.size();i++) a.push_back({v[i],i});
    sort(a.begin(),a.end(),[&](auto&a,auto&b){return a.first>b.first;});
    vector<pair<int,db>>r;
    for(auto &x:a) r.push_back({x.second,x.first});
    return r;
}

vector<int> anomaly_points(const vector<db>&v,db th){
    vector<int>d;
    for(int i=0;i<v.size();i++){
        if(v[i]>th) d.push_back(i);
    }
    return d;
}

vector<db> random_series(int n,int seed){
    vector<db>a(n);
    mt19937_64 rng(seed);
    uniform_real_distribution<db>ud(40,200);
    for(int i=0;i<n;i++) a[i]=ud(rng);
    return a;
}

vector<db> random_series2(int n,int seed){
    vector<db>a(n);
    mt19937_64 rng(seed);
    uniform_real_distribution<db>ud(10,150);
    for(int i=0;i<n;i++) a[i]=ud(rng);
    return a;
}

vector<db> random_series3(int n,int seed){
    vector<db>a(n);
    mt19937_64 rng(seed);
    uniform_real_distribution<db>ud(5,80);
    for(int i=0;i<n;i++) a[i]=ud(rng);
    return a;
}

vector<vector<db>> build_sensor_matrix(int rows,int cols,int seed){
    vector<vector<db>>M(rows, vector<db>(cols,0));
    mt19937_64 rng(seed);
    uniform_real_distribution<db>ud(20,200);
    for(int i=0;i<rows;i++){
        for(int j=0;j<cols;j++){
            M[i][j]=ud(rng);
        }
    }
    return M;
}

vector<db> flatten(const vector<vector<db>>&M){
    vector<db>r;
    for(auto &row:M) for(auto &x:row) r.push_back(x);
    return r;
}

vector<int> zone_hotspots(const vector<db>&series,db th){
    vector<int>z;
    for(int i=0;i<series.size();i++){
        if(series[i]>th) z.push_back(i);
    }
    return z;
}

struct WindowStat {
    vector<db>mx;
    vector<db>mn;
    vector<db>sm;
};

WindowStat build_window_stats(const vector<db>&v,int w){
    WindowStat ws;
    ws.mx=sliding_max(v,w);
    ws.mn=sliding_min(v,w);
    ws.sm=sliding_sum(v,w);
    return ws;
}

vector<pair<int,db>> detect_peaks(const vector<db>&v){
    int n=v.size();
    vector<pair<db,int>>a;
    for(int i=0;i<n;i++){
        db s=0;
        if(i>0) s+=v[i-1];
        s+=v[i];
        if(i+1<n) s+=v[i+1];
        a.push_back({s,i});
    }
    sort(a.begin(),a.end(),[&](auto&a,auto&b){return a.first>b.first;});
    vector<pair<int,db>>o;
    for(auto &x:a) o.push_back({x.second,x.first});
    return o;
}

vector<int> local_spike(const vector<db>&v){
    vector<int>s;
    for(int i=1;i+1<v.size();i++){
        if(v[i]>v[i-1] && v[i]>v[i+1]) s.push_back(i);
    }
    return s;
}

vector<int> smooth_outliers(const vector<db>&v){
    int n=v.size();
    vector<int>o;
    for(int i=1;i+1<n;i++){
        db a=v[i-1], b=v[i], c=v[i+1];
        db m=(a+c)/2;
        if(b>m*1.5) o.push_back(i);
    }
    return o;
}

struct PollutionSystem {
    Stream st;
    vector<db>data;
    int n;
    PollutionSystem(int n_=0){init(n_);}
    void init(int n_){
        n=n_;
        data.assign(n,0);
        st.init(n);
    }
    void build(const vector<db>&v){
        n=v.size();
        data=v;
        st.build(v);
    }
    void update(int idx,db val){
        st.push_update(idx,val);
        data[idx]=val;
    }
};

struct ZoneResult {
    vector<pair<int,db>> top;
    vector<int> spikes;
    vector<int> outs;
    WindowStat ws;
};

ZoneResult analyse_zone(const vector<db>&v,int w){
    ZoneResult zr;
    zr.top=rank_locations(v);
    zr.spikes=local_spike(v);
    zr.outs=smooth_outliers(v);
    zr.ws=build_window_stats(v,w);
    return zr;
}

struct SystemResult {
    vector<pair<int,db>>ranked;
    vector<int>anom;
    vector<int>zone;
    vector<int>sp;
    vector<int>outl;
    WindowStat ws;
};

SystemResult analyse_system(const vector<db>&v,int w,db th){
    SystemResult R;
    R.ranked=rank_locations(v);
    R.anom=anomaly_points(v,th);
    R.zone=zone_hotspots(v,th);
    R.sp=local_spike(v);
    R.outl=smooth_outliers(v);
    R.ws=build_window_stats(v,w);
    return R;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int mode;
    if(!(cin>>mode)) return 0;

    if(mode==0){
        int n=200;
        auto a = random_series(n,7);
        PollutionSystem sys(n);
        sys.build(a);
        auto R = analyse_system(a,5,150.0);
        cout<<"R "<<R.ranked.size()<<"\n";
        cout<<"A "<<R.anom.size()<<"\n";
        cout<<"Z "<<R.zone.size()<<"\n";
        cout<<"S "<<R.sp.size()<<"\n";
        cout<<"O "<<R.outl.size()<<"\n";
        cout<<"W "<<R.ws.mx.size()<<"\n";
        return 0;
    }

    if(mode==1){
        int n;
        cin>>n;
        vector<db>a(n);
        for(int i=0;i<n;i++) cin>>a[i];
        PollutionSystem sys(n);
        sys.build(a);
        int q;
        cin>>q;
        while(q--){
            int idx; db v;
            cin>>idx>>v;
            sys.update(idx,v);
        }
        auto R = analyse_system(sys.data,4,120.0);
        cout<<R.anom.size()<<"\n";
        return 0;
    }

    if(mode==2){
        int r,c;
        cin>>r>>c;
        auto M = build_sensor_matrix(r,c,3);
        auto f = flatten(M);
        auto R = analyse_system(f,8,180.0);
        cout<<R.ranked.size()<<" "<<R.anom.size()<<"\n";
        return 0;
    }

    if(mode==3){
        int n,w;
        cin>>n>>w;
        vector<db>a(n);
        for(int i=0;i<n;i++) cin>>a[i];
        auto Z = analyse_zone(a,w);
        cout<<Z.top.size()<<" "<<Z.spikes.size()<<" "<<Z.outs.size()<<"\n";
        return 0;
    }

    return 0;
}
