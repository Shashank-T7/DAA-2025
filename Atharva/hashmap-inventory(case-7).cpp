#include <bits/stdc++.h>
using namespace std;

struct Item {
    int id;
    string name;
    string category;
    int quantity;
    int warehouse;
    double weight;
    double score;
};

static vector<vector<string>> read_csv_generic(const string &path){
    ifstream in(path);
    vector<vector<string>> out;
    if(!in.is_open()) return out;
    string line;
    if(!getline(in,line)) return out;
    while(getline(in,line)){
        vector<string> cols;
        string cur; bool inq=false;
        for(char c: line){
            if(c=='"'){ inq=!inq; continue; }
            if(c==',' && !inq){ cols.push_back(cur); cur.clear(); }
            else cur.push_back(c);
        }
        cols.push_back(cur);
        out.push_back(cols);
    }
    return out;
}

static void write_csv_items(const string &path, const vector<Item> &v){
    ofstream out(path);
    out<<"id,name,category,quantity,warehouse,weight,score\n";
    for(auto &x: v){
        out<<x.id<<","<<x.name<<","<<x.category<<","<<x.quantity<<","<<x.warehouse<<","<<x.weight<<","<<x.score<<"\n";
    }
    out.close();
}

static vector<string> split_ws(const string &s){
    vector<string> out; string tmp; stringstream ss(s);
    while(ss>>tmp) out.push_back(tmp);
    return out;
}

static string to_lower(const string &s){
    string r=s;
    for(char &c:r) c=tolower(c);
    return r;
}

struct SkipNode {
    Item val;
    vector<SkipNode*> next;
    SkipNode(const Item &v,int lvl):val(v),next(lvl,nullptr){}
};

struct SkipList {
    int maxlvl;
    double p;
    SkipNode* head;
    mt19937_64 rng;
    SkipList(int mx=20,double pp=0.5):maxlvl(mx),p(pp){
        head=new SkipNode(Item(),maxlvl);
        rng.seed(chrono::high_resolution_clock::now().time_since_epoch().count());
    }
    int random_level(){
        int lvl=1;
        uniform_real_distribution<double> d(0.0,1.0);
        while(d(rng)<p && lvl<maxlvl) lvl++;
        return lvl;
    }
    void insert_item(const Item &it){
        vector<SkipNode*> up(maxlvl,nullptr);
        SkipNode* cur=head;
        for(int lvl=maxlvl-1; lvl>=0; --lvl){
            while(cur->next[lvl] && cur->next[lvl]->val.score < it.score){
                cur = cur->next[lvl];
            }
            up[lvl] = cur;
        }
        int lvl = random_level();
        SkipNode* nn = new SkipNode(it,lvl);
        for(int i=0;i<lvl;++i){
            nn->next[i] = up[i]->next[i];
            up[i]->next[i] = nn;
        }
    }
    bool erase_item(int id){
        SkipNode* cur=head;
        bool found=false;
        for(int lvl=maxlvl-1; lvl>=0; --lvl){
            while(cur->next[lvl] && cur->next[lvl]->val.id < id){
                cur = cur->next[lvl];
            }
            while(cur->next[lvl] && cur->next[lvl]->val.id == id){
                found=true;
                SkipNode* d=cur->next[lvl];
                cur->next[lvl] = d->next[lvl];
            }
        }
        return found;
    }
    vector<Item> top_k(int k){
        vector<Item> out;
        SkipNode* cur=head->next[0];
        while(cur && (int)out.size()<k){
            out.push_back(cur->val);
            cur=cur->next[0];
        }
        return out;
    }
    vector<Item> all_sorted(){
        vector<Item> out;
        SkipNode* cur=head->next[0];
        while(cur){ out.push_back(cur->val); cur=cur->next[0]; }
        return out;
    }
};

class Inventory {
public:
    unordered_map<int, Item> h;
    unordered_map<string, vector<int>> bycat;
    unordered_map<string, vector<int>> byname;
    SkipList sk;
    Inventory():sk(24,0.55){}
    static double score_of(const Item &x){
        return x.quantity*0.6 + x.weight*0.4 + (x.warehouse%7)*0.1 + 1.0;
    }
    void load_csv(const string &path){
        auto rows = read_csv_generic(path);
        for(auto &r: rows){
            if(r.size() < 6) continue;
            Item it;
            it.id = stoi(r[0]);
            it.name = r[1];
            it.category = r[2];
            it.quantity = stoi(r[3]);
            it.warehouse = stoi(r[4]);
            it.weight = stod(r[5]);
            it.score = score_of(it);
            add_item(it);
        }
    }
    void add_item(const Item &it){
        h[it.id] = it;
        bycat[to_lower(it.category)].push_back(it.id);
        byname[to_lower(it.name)].push_back(it.id);
        sk.insert_item(it);
    }
    bool remove_item(int id){
        if(!h.count(id)) return false;
        auto it = h[id];
        auto &vc = bycat[to_lower(it.category)];
        vc.erase(remove(vc.begin(), vc.end(), it.id), vc.end());
        auto &vn = byname[to_lower(it.name)];
        vn.erase(remove(vn.begin(), vn.end(), it.id), vn.end());
        sk.erase_item(id);
        h.erase(id);
        return true;
    }
    vector<Item> list_category(const string &cat){
        vector<Item> out;
        auto key = to_lower(cat);
        if(!bycat.count(key)) return out;
        for(int id: bycat[key]) out.push_back(h[id]);
        return out;
    }
    vector<Item> name_prefix(const string &prefix){
        vector<Item> out;
        string p = to_lower(prefix);
        for(auto &kv: byname){
            if(kv.first.size()>=p.size() && kv.first.compare(0,p.size(),p)==0){
                for(int id: kv.second) out.push_back(h[id]);
            }
        }
        return out;
    }
    vector<Item> warehouse_range(int l,int r){
        vector<Item> out;
        for(auto &kv: h){
            if(kv.second.warehouse >= l && kv.second.warehouse <= r){
                out.push_back(kv.second);
            }
        }
        return out;
    }
    vector<Item> get_top_k(int k){
        return sk.top_k(k);
    }
    vector<Item> sorted_inventory(){ return sk.all_sorted(); }
    void export_all(const string &path){
        auto v = sorted_inventory();
        write_csv_items(path,v);
    }
    vector<Item> global_search(const string &q){
        vector<Item> out;
        string s = to_lower(q);
        for(auto &kv: h){
            auto &it = kv.second;
            string nm = to_lower(it.name);
            string ct = to_lower(it.category);
            if(nm.find(s)!=string::npos || ct.find(s)!=string::npos) out.push_back(it);
        }
        return out;
    }
    vector<Item> multi_index_query(const string &term,int wmin,int wmax){
        vector<Item> namehits = name_prefix(term);
        vector<Item> out;
        for(auto &x: namehits){
            if(x.warehouse>=wmin && x.warehouse<=wmax) out.push_back(x);
        }
        return out;
    }
    void rebalance(){
        vector<Item> v = sorted_inventory();
        sk = SkipList(24,0.55);
        bycat.clear();
        byname.clear();
        h.clear();
        for(auto &x: v){
            add_item(x);
        }
    }
    void stress_ops(int n){
        mt19937_64 rng(chrono::high_resolution_clock::now().time_since_epoch().count());
        uniform_int_distribution<int> d(1,9999999);
        for(int i=0;i<n;++i){
            int op = d(rng) % 5;
            if(op==0){
                Item it;
                it.id = d(rng);
                it.name = "item"+to_string(it.id%1000);
                it.category = "cat"+to_string(it.id%10);
                it.quantity = d(rng)%500;
                it.warehouse = d(rng)%20;
                it.weight = (d(rng)%200)/10.0;
                it.score = score_of(it);
                add_item(it);
            } else if(op==1){
                if(!h.empty()){
                    auto it = h.begin();
                    advance(it, d(rng)%h.size());
                    remove_item(it->first);
                }
            } else if(op==2){
                auto hits = global_search("a");
            } else if(op==3){
                auto v = get_top_k(10);
            } else {
                if(i%200==0) rebalance();
            }
        }
    }
};

struct Timer {
    chrono::high_resolution_clock::time_point s;
    void start(){ s=chrono::high_resolution_clock::now(); }
    long long ms(){ return chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now()-s).count(); }
};

int main(int argc,char**argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(argc<2){
        cerr<<"Usage: "<<argv[0]<<" <inventory.csv>\n";
        return 1;
    }
    string csv = argv[1];
    Inventory inv;
    Timer t; t.start();
    inv.load_csv(csv);
    cerr<<"load "<<t.ms()<<"ms\n";
    cout<<"items="<<inv.h.size()<<"\n";
    cout<<"cmds:\nadd id nm cat q wh wt\nremove id\ncat c\nnamepref p\nrange l r\ntopk k\nexport f.csv\nglob term\nmix term l r\nsorted\nrebalance\nstress n\nquit\n";
    string line;
    while(true){
        cout<<"> ";
        if(!getline(cin,line)) break;
        if(line.empty()) continue;
        auto v = split_ws(line);
        if(v.empty()) continue;
        string c=v[0];
        if(c=="quit"||c=="exit") break;
        if(c=="add"){
            if(v.size()<7){ cout<<"add id nm cat q wh wt\n"; continue; }
            Item it;
            it.id=stoi(v[1]);
            it.name=v[2];
            it.category=v[3];
            it.quantity=stoi(v[4]);
            it.warehouse=stoi(v[5]);
            it.weight=stod(v[6]);
            it.score=Inventory::score_of(it);
            inv.add_item(it);
            cout<<"added\n";
            continue;
        }
        if(c=="remove"){
            if(v.size()<2){ cout<<"remove id\n"; continue; }
            int id=stoi(v[1]);
            cout<<(inv.remove_item(id)?"ok\n":"nf\n");
            continue;
        }
        if(c=="cat"){
            if(v.size()<2){ cout<<"cat c\n"; continue; }
            auto out=inv.list_category(v[1]);
            for(auto &x: out) cout<<x.id<<","<<x.name<<","<<x.score<<"\n";
            continue;
        }
        if(c=="namepref"){
            if(v.size()<2){ cout<<"namepref p\n"; continue; }
            auto out=inv.name_prefix(v[1]);
            for(auto &x: out) cout<<x.id<<","<<x.name<<","<<x.score<<"\n";
            continue;
        }
        if(c=="range"){
            if(v.size()<3){ cout<<"range l r\n"; continue; }
            int l=stoi(v[1]), r=stoi(v[2]);
            auto out=inv.warehouse_range(l,r);
            for(auto &x: out) cout<<x.id<<","<<x.warehouse<<","<<x.score<<"\n";
            continue;
        }
        if(c=="topk"){
            if(v.size()<2){ cout<<"topk k\n"; continue; }
            int k=stoi(v[1]);
            auto out=inv.get_top_k(k);
            for(auto &x: out) cout<<x.id<<","<<x.score<<"\n";
            continue;
        }
        if(c=="export"){
            if(v.size()<2){ cout<<"export f.csv\n"; continue; }
            inv.export_all(v[1]);
            cout<<"done\n";
            continue;
        }
        if(c=="glob"){
            if(v.size()<2){ cout<<"glob term\n"; continue; }
            auto out=inv.global_search(v[1]);
            for(auto &x: out) cout<<x.id<<","<<x.name<<","<<x.category<<"\n";
            continue;
        }
        if(c=="mix"){
            if(v.size()<4){ cout<<"mix term l r\n"; continue; }
            string term=v[1];
            int l=stoi(v[2]), r=stoi(v[3]);
            auto out=inv.multi_index_query(term,l,r);
            for(auto &x: out) cout<<x.id<<","<<x.name<<","<<x.warehouse<<"\n";
            continue;
        }
        if(c=="sorted"){
            auto out=inv.sorted_inventory();
            for(auto &x: out) cout<<x.id<<","<<x.score<<"\n";
            continue;
        }
        if(c=="rebalance"){
            inv.rebalance();
            cout<<"rebuilt\n";
            continue;
        }
        if(c=="stress"){
            if(v.size()<2){ cout<<"stress n\n"; continue; }
            int n=stoi(v[1]);
            t.start();
            inv.stress_ops(n);
            cout<<"done "<<t.ms()<<"ms\n";
            continue;
        }
        cout<<"unknown\n";
    }
    return 0;
}
