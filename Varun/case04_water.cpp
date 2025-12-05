#include <bits/stdc++.h>
using namespace std;

struct SegmentTree {
    int n;
    vector<double> tmin, tmax;
    SegmentTree(int sz=0){ init(sz); }
    void init(int sz){
        n=1; while(n<sz) n<<=1;
        tmin.assign(2*n, 1e18);
        tmax.assign(2*n, -1e18);
    }
    void update(int i, double v){
        i+=n;
        tmin[i]=tmax[i]=v;
        for(i>>=1; i; i>>=1){
            tmin[i]=min(tmin[i<<1], tmin[i<<1|1]);
            tmax[i]=max(tmax[i<<1], tmax[i<<1|1]);
        }
    }
    double rangeMin(int l,int r){
        double res=1e18;
        for(l+=n, r+=n; l<=r; l>>=1, r>>=1){
            if(l&1) res=min(res, tmin[l++]);
            if(!(r&1)) res=min(res, tmin[r--]);
        }
        return res;
    }
    double rangeMax(int l,int r){
        double res=-1e18;
        for(l+=n, r+=n; l<=r; l>>=1, r>>=1){
            if(l&1) res=max(res, tmax[l++]);
            if(!(r&1)) res=max(res, tmax[r--]);
        }
        return res;
    }
};

struct SparseTable {
    int n, L;
    vector<vector<double>> stMin, stMax;
    vector<int> lg;
    void build(const vector<double>& arr){
        n=arr.size();
        lg.assign(n+1,0);
        for(int i=2;i<=n;i++) lg[i]=lg[i/2]+1;
        L=lg[n];
        stMin.assign(L+1, vector<double>(n));
        stMax.assign(L+1, vector<double>(n));
        for(int i=0;i<n;i++){
            stMin[0][i]=arr[i];
            stMax[0][i]=arr[i];
        }
        for(int k=1;k<=L;k++){
            for(int i=0;i+(1<<k)<=n;i++){
                stMin[k][i]=min(stMin[k-1][i], stMin[k-1][i+(1<<(k-1))]);
                stMax[k][i]=max(stMax[k-1][i], stMax[k-1][i+(1<<(k-1))]);
            }
        }
    }
    double rangeMin(int l,int r){
        int k=lg[r-l+1];
        return min(stMin[k][l], stMin[k][r-(1<<k)+1]);
    }
    double rangeMax(int l,int r){
        int k=lg[r-l+1];
        return max(stMax[k][l], stMax[k][r-(1<<k)+1]);
    }
};

vector<int> buildKMP(const string &p){
    int m=p.size();
    vector<int> lps(m,0);
    for(int i=1,j=0; i<m; ){
        if(p[i]==p[j]) lps[i++]=++j;
        else if(j>0) j=lps[j-1];
        else lps[i++]=0;
    }
    return lps;
}

bool containsPattern(const string &txt, const string &pat){
    if(pat.empty()) return true;
    auto lps=buildKMP(pat);
    int n=txt.size(), m=pat.size();
    for(int i=0,j=0; i<n; ){
        if(txt[i]==pat[j]){
            i++; j++;
            if(j==m) return true;
        } else if(j>0) j=lps[j-1];
        else i++;
    }
    return false;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int mode;
    cin>>mode;

    if(mode==1){
        int n; cin>>n;
        SegmentTree st(n);
        vector<double> arr(n);
        for(int i=0;i<n;i++){
            cin>>arr[i];
            st.update(i, arr[i]);
        }
        int q; cin>>q;
        while(q--){
            int l,r; cin>>l>>r;
            double mn=st.rangeMin(l,r);
            double mx=st.rangeMax(l,r);
            cout<<fixed<<setprecision(6)<<mn<<" "<<mx<<"\n";
        }
    }
    else if(mode==2){
        int n; cin>>n;
        vector<double> arr(n);
        for(int i=0;i<n;i++) cin>>arr[i];
        SparseTable sp; sp.build(arr);
        int q; cin>>q;
        while(q--){
            int l,r; cin>>l>>r;
            cout<<fixed<<setprecision(6)<<sp.rangeMin(l,r)<<" "<<sp.rangeMax(l,r)<<"\n";
        }
    }
    else if(mode==3){
        int k; cin>>k;
        vector<string> reports(k);
        string tmp; getline(cin,tmp);
        for(int i=0;i<k;i++) getline(cin,reports[i]);
        string keyword; getline(cin,keyword);
        int count=0;
        for(auto &r: reports){
            if(containsPattern(r, keyword)) count++;
        }
        cout<<count<<"\n";
    }
    else if(mode==4){
        int n; cin>>n;
        vector<double> arr(n);
        for(int i=0;i<n;i++) cin>>arr[i];
        sort(arr.begin(), arr.end());
        for(double x: arr) cout<<fixed<<setprecision(6)<<x<<" ";
        cout<<"\n";
    }
    else return 0;

    return 0;
}
