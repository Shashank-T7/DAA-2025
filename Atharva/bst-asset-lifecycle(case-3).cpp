#include <bits/stdc++.h>
using namespace std;

struct Asset {
    int device_id;
    string model;
    string received_date;
    string expiry_date;
    int condition;
    int location_id;
    double score;
};

static vector<vector<string>> read_csv_generic(const string &path) {
    ifstream in(path);
    vector<vector<string>> rows;
    if(!in.is_open()) return rows;
    string header;
    if(!getline(in, header)) return rows;
    string line;
    while(getline(in, line)) {
        vector<string> cols;
        string cur;
        bool inq=false;
        for(char ch: line) {
            if(ch=='"'){ inq=!inq; continue; }
            if(ch==',' && !inq){ cols.push_back(cur); cur.clear(); }
            else cur.push_back(ch);
        }
        cols.push_back(cur);
        rows.push_back(cols);
    }
    return rows;
}

static string trim(const string &s){
    size_t a=0,b=s.size();
    while(a<b && isspace((unsigned char)s[a])) ++a;
    while(b>a && isspace((unsigned char)s[b-1])) --b;
    return s.substr(a,b-a);
}

static long long date_to_int(const string &d){
    if(d.size()<8) return 0;
    int y=0,m=0,day=0;
    sscanf(d.c_str(), "%d-%d-%d", &y, &m, &day);
    return (long long)y*10000 + m*100 + day;
}

struct TreapNode {
    Asset val;
    int pr;
    TreapNode *l,*r;
    int sz;
    long long key;
    TreapNode(const Asset &a): val(a), pr((rand()<<16) ^ rand()), l(nullptr), r(nullptr), sz(1), key(date_to_int(a.expiry_date)){}
};

static int sz(TreapNode* t){ return t ? t->sz : 0; }
static void pull(TreapNode* t){ if(t) t->sz = 1 + sz(t->l) + sz(t->r); }

static void split_by_key(TreapNode* t, long long k, TreapNode* &a, TreapNode* &b){
    if(!t){ a=b=nullptr; return; }
    if(t->key <= k){ split_by_key(t->r, k, t->r, b); a=t; pull(a); }
    else{ split_by_key(t->l, k, a, t->l); b=t; pull(b); }
}

static TreapNode* meld(TreapNode* a, TreapNode* b){
    if(!a) return b;
    if(!b) return a;
    if(a->pr < b->pr){ a->r = meld(a->r, b); pull(a); return a; }
    else{ b->l = meld(a, b->l); pull(b); return b; }
}

static TreapNode* insert_node(TreapNode* t, TreapNode* node){
    if(!t) return node;
    if(node->pr < t->pr){
        split_by_key(t, node->key, node->l, node->r);
        pull(node);
        return node;
    }
    if(node->key <= t->key) t->l = insert_node(t->l, node);
    else t->r = insert_node(t->r, node);
    pull(t);
    return t;
}

static TreapNode* erase_key(TreapNode* t, long long key, int device_id, bool &erased){
    if(!t) return nullptr;
    if(t->key == key && t->val.device_id == device_id){
        erased = true;
        TreapNode* res = meld(t->l, t->r);
        delete t;
        return res;
    }
    if(key <= t->key) t->l = erase_key(t->l, key, device_id, erased);
    else t->r = erase_key(t->r, key, device_id, erased);
    pull(t);
    return t;
}

static void inorder_collect(TreapNode* t, vector<Asset> &out){
    if(!t) return;
    inorder_collect(t->l, out);
    out.push_back(t->val);
    inorder_collect(t->r, out);
}

static void range_collect(TreapNode* &root, long long a, long long b, vector<Asset> &out){
    TreapNode *t1, *t2, *t3;
    split_by_key(root, b, t2, t3);
    split_by_key(t2, a-1, t1, t2);
    inorder_collect(t2, out);
    root = meld(meld(t1, t2), t3);
}

struct MinHeapItem {
    long long key;
    int device_id;
    double score;
    bool operator<(const MinHeapItem &o) const { return key > o.key; }
};

class AssetManager {
    TreapNode* root;
    unordered_map<int, Asset> by_id;
    priority_queue<MinHeapItem> pq;
    unordered_map<int, bool> in_heap;
public:
    AssetManager(): root(nullptr){}
    void bulk_load(const string &csv){
        auto rows = read_csv_generic(csv);
        for(auto &r: rows){
            if(r.size() < 5) continue;
            Asset a;
            a.device_id = stoi(trim(r[0]));
            a.model = trim(r[1]);
            a.received_date = trim(r[2]);
            a.expiry_date = trim(r[3]);
            a.condition = stoi(trim(r[4]));
            a.location_id = (r.size()>5? stoi(trim(r[5])) : 0);
            a.score = compute_score(a);
            by_id[a.device_id] = a;
            TreapNode* node = new TreapNode(a);
            root = insert_node(root, node);
            pq.push({date_to_int(a.expiry_date), a.device_id, a.score});
            in_heap[a.device_id] = true;
        }
    }
    double compute_score(const Asset &a){
        double s = a.condition * 0.6 + max(0.0, 100.0 - (double)abs((int)(date_to_int(a.expiry_date) % 10000 - 1000))) * 0.1;
        return s + 1.0;
    }
    bool remove(int device_id){
        if(by_id.find(device_id)==by_id.end()) return false;
        Asset a = by_id[device_id];
        bool erased=false;
        root = erase_key(root, date_to_int(a.expiry_date), device_id, erased);
        by_id.erase(device_id);
        in_heap[device_id] = false;
        return erased;
    }
    bool add(const Asset &a){
        if(by_id.find(a.device_id)!=by_id.end()) return false;
        Asset copy = a;
        copy.score = compute_score(copy);
        by_id[copy.device_id] = copy;
        TreapNode* node = new TreapNode(copy);
        root = insert_node(root, node);
        pq.push({date_to_int(copy.expiry_date), copy.device_id, copy.score});
        in_heap[copy.device_id] = true;
        return true;
    }
    Asset peek_next_expiry(){
        while(!pq.empty() && (!in_heap[pq.top().device_id] || by_id.find(pq.top().device_id)==by_id.end())) pq.pop();
        if(pq.empty()) return Asset();
        int id = pq.top().device_id;
        return by_id[id];
    }
    bool pop_next_expiry(Asset &out){
        while(!pq.empty() && (!in_heap[pq.top().device_id] || by_id.find(pq.top().device_id)==by_id.end())) pq.pop();
        if(pq.empty()) return false;
        int id = pq.top().device_id;
        pq.pop();
        if(by_id.find(id)==by_id.end()) return false;
        out = by_id[id];
        remove(id);
        return true;
    }
    vector<Asset> list_all_sorted(){
        vector<Asset> out;
        inorder_collect(root, out);
        return out;
    }
    vector<Asset> query_expiry_range(const string &a, const string &b){
        vector<Asset> out;
        range_collect(root, date_to_int(a), date_to_int(b), out);
        return out;
    }
    vector<Asset> top_k_by_score(int k){
        vector<pair<double, int>> vec;
        for(auto &kv: by_id) vec.push_back({kv.second.score, kv.first});
        sort(vec.begin(), vec.end(), greater<>());
        vector<Asset> out;
        for(int i=0;i<k && i<(int)vec.size(); ++i) out.push_back(by_id[vec[i].second]);
        return out;
    }
    void export_csv(const string &path){
        ofstream out(path);
        out<<"device_id,model,received_date,expiry_date,condition,location_id,score\n";
        vector<Asset> all = list_all_sorted();
        for(auto &a: all) out<<a.device_id<<","<<a.model<<","<<a.received_date<<","<<a.expiry_date<<","<<a.condition<<","<<a.location_id<<","<<a.score<<"\n";
        out.close();
    }
    size_t total_count() const { return by_id.size(); }
    void rebuild_balance(){
        vector<Asset> all = list_all_sorted();
        function<void(TreapNode*& , int, int)> build = [&](TreapNode*& node, int l, int r){
            if(l>r){ node=nullptr; return; }
            int m = (l+r)/2;
            node = new TreapNode(all[m]);
            build(node->l, l, m-1);
            build(node->r, m+1, r);
            pull(node);
        };
        // delete old treap nodes
        function<void(TreapNode*)> cleanup = [&](TreapNode* t){
            if(!t) return;
            cleanup(t->l); cleanup(t->r); delete t;
        };
        cleanup(root);
        root = nullptr;
        build(root, 0, (int)all.size()-1);
        // rebuild heap
        priority_queue<MinHeapItem> empty;
        swap(pq, empty);
        in_heap.clear();
        for(auto &kv: by_id){
            pq.push({date_to_int(kv.second.expiry_date), kv.first, kv.second.score});
            in_heap[kv.first] = true;
        }
    }
    void stress_random_updates(int n){
        mt19937_64 rng((uint64_t)chrono::high_resolution_clock::now().time_since_epoch().count());
        uniform_int_distribution<int> d(0,999999);
        for(int i=0;i<n;++i){
            int op = d(rng) % 3;
            if(op==0 && !by_id.empty()){
                auto it = by_id.begin();
                advance(it, d(rng) % by_id.size());
                remove(it->first);
            } else {
                Asset a;
                a.device_id = d(rng);
                a.model = "GEN";
                int yy = 2025; int mm = 1 + (d(rng)%12); int dd = 1 + (d(rng)%28);
                char buf[32]; sprintf(buf, "%04d-%02d-%02d", yy, mm, dd);
                a.received_date = buf;
                int exy = 2025 + (d(rng)%3);
                int exm = 1 + (d(rng)%12);
                int exd = 1 + (d(rng)%28);
                sprintf(buf, "%04d-%02d-%02d", exy, exm, exd);
                a.expiry_date = buf;
                a.condition = 30 + (d(rng)%70);
                a.location_id = d(rng)%10;
                add(a);
            }
            if(i%1000==0) rebuild_balance();
        }
    }
};

static void timed(function<void()> f, const string &label){
    auto t1 = chrono::high_resolution_clock::now();
    f();
    auto t2 = chrono::high_resolution_clock::now();
    auto ms = chrono::duration_cast<chrono::milliseconds>(t2-t1).count();
    cerr<<label<<" "<<ms<<"ms\n";
}

static vector<string> split_ws(const string &s){
    vector<string> out; string tmp; stringstream ss(s);
    while(ss>>tmp) out.push_back(tmp);
    return out;
}

static string now_iso(){
    auto t=chrono::system_clock::now();
    time_t tt = chrono::system_clock::to_time_t(t);
    char buf[64]; strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&tt));
    return string(buf);
}

int main(int argc,char**argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    srand((unsigned)chrono::high_resolution_clock::now().time_since_epoch().count());
    if(argc < 2){
        cerr<<"Usage: "<<argv[0]<<" <csv-path>\n";
        return 1;
    }
    string csv = argv[1];
    AssetManager mgr;
    timed([&](){ mgr.bulk_load(csv); }, "load");
    cout<<"Assets loaded="<<mgr.total_count()<<"\n";
    cout<<"Commands:\nnext\npop\nadd device model rdate edate cond loc\nremove id\nrange rstart rend\ntopk k\nexport out.csv\nrebuild\nstress n\nstats\nquit\n";
    string line;
    while(true){
        cout<<"> ";
        if(!getline(cin, line)) break;
        if(line.empty()) continue;
        auto parts = split_ws(line);
        if(parts.empty()) continue;
        string cmd = parts[0];
        if(cmd=="quit" || cmd=="exit") break;
        if(cmd=="next"){
            Asset a = mgr.peek_next_expiry();
            if(mgr.total_count()==0) cout<<"no assets\n"; else cout<<a.device_id<<","<<a.model<<","<<a.expiry_date<<","<<a.condition<<"\n";
            continue;
        }
        if(cmd=="pop"){
            Asset a;
            if(mgr.pop_next_expiry(a)) cout<<"popped "<<a.device_id<<"\n"; else cout<<"none\n";
            continue;
        }
        if(cmd=="add"){
            if(parts.size()<7){ cout<<"add device model rdate edate cond loc\n"; continue; }
            Asset a;
            a.device_id = stoi(parts[1]);
            a.model = parts[2];
            a.received_date = parts[3];
            a.expiry_date = parts[4];
            a.condition = stoi(parts[5]);
            a.location_id = stoi(parts[6]);
            if(mgr.add(a)) cout<<"added\n"; else cout<<"exists\n";
            continue;
        }
        if(cmd=="remove"){
            if(parts.size()<2){ cout<<"remove id\n"; continue; }
            int id = stoi(parts[1]);
            if(mgr.remove(id)) cout<<"removed\n"; else cout<<"notfound\n";
            continue;
        }
        if(cmd=="range"){
            if(parts.size()<3){ cout<<"range rstart rend\n"; continue; }
            string a = parts[1], b = parts[2];
            auto res = mgr.query_expiry_range(a,b);
            for(auto &x: res) cout<<x.device_id<<","<<x.model<<","<<x.expiry_date<<","<<x.condition<<"\n";
            continue;
        }
        if(cmd=="topk"){
            if(parts.size()<2){ cout<<"topk k\n"; continue; }
            int k = stoi(parts[1]);
            auto res = mgr.top_k_by_score(k);
            for(auto &x: res) cout<<x.device_id<<","<<x.model<<","<<x.score<<"\n";
            continue;
        }
        if(cmd=="export"){
            if(parts.size()<2){ cout<<"export out.csv\n"; continue; }
            mgr.export_csv(parts[1]); cout<<"exported\n"; continue;
        }
        if(cmd=="rebuild"){
            timed([&](){ mgr.rebuild_balance(); }, "rebuild");
            cout<<"rebuilt\n"; continue;
        }
        if(cmd=="stress"){
            if(parts.size()<2){ cout<<"stress n\n"; continue; }
            int n = stoi(parts[1]);
            timed([&](){ mgr.stress_random_updates(n); }, "stress");
            cout<<"stressdone\n"; continue;
        }
        if(cmd=="stats"){
            cout<<"count="<<mgr.total_count()<<"\n";
            continue;
        }
        cout<<"unknown\n";
    }
    return 0;
}
