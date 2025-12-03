#include <bits/stdc++.h>
using namespace std;

using ull = unsigned long long;
using u128 = __uint128_t;

static vector<vector<string>> read_csv_raw(const string &path){
    ifstream in(path);
    vector<vector<string>> out;
    if(!in.is_open()) return out;
    string line;
    if(!getline(in,line)) return out;
    while(getline(in,line)){
        vector<string> cols; string cur; bool inq=false;
        for(char c: line){
            if(c=='"'){ inq = !inq; continue; }
            if(c==',' && !inq){ cols.push_back(cur); cur.clear(); } else cur.push_back(c);
        }
        cols.push_back(cur);
        out.push_back(cols);
    }
    return out;
}

struct Batch {
    int batch_id;
    int product_id;
    string code;
    string manufacture_date;
    string received_date;
    int qty;
    double score;
};

static vector<Batch> load_batches(const string &path){
    vector<Batch> out;
    auto rows = read_csv_raw(path);
    for(auto &r: rows) if(r.size() >= 6){
        Batch b;
        b.batch_id = stoi(r[0]);
        b.product_id = stoi(r[1]);
        b.code = r[2];
        b.manufacture_date = r[3];
        b.received_date = r[4];
        b.qty = stoi(r[5]);
        b.score = 1.0 + b.qty * 0.01;
        out.push_back(b);
    }
    return out;
}

struct RollingHash {
    ull base;
    ull mod;
    vector<ull> powv;
    RollingHash(ull base_=1315423911ull, ull mod_ = (1ull<<61)-1): base(base_), mod(mod_){}
    static inline ull modmul(ull a, ull b){
        u128 z = (u128)a * b;
        ull lo = (ull)z;
        ull hi = (ull)(z >> 64);
        const ull m = (1ull<<61)-1;
        u128 t = (u128)hi * 0xFFFFFFFFFFFFFFFFull + lo;
        ull x = (ull)((t >> 61) + (t & m));
        if(x >= m) x -= m;
        return x;
    }
    void ensure_pow(size_t n){
        if(powv.size() >= n) return;
        size_t cur = powv.size();
        if(cur==0) powv.push_back(1);
        for(size_t i=cur;i<n;++i) powv.push_back(modmul(powv.back(), base));
    }
    ull hash_str(const string &s){
        ull h = 0;
        for(unsigned char c: s){
            h = modmul(h, base) + (ull)c;
            if(h >= mod) h -= mod;
        }
        return h;
    }
    vector<ull> prefix_hash(const string &s){
        int n = (int)s.size();
        vector<ull> pref(n+1);
        pref[0] = 0;
        for(int i=0;i<n;++i){
            pref[i+1] = modmul(pref[i], base) + (unsigned char)s[i];
            if(pref[i+1] >= mod) pref[i+1] -= mod;
        }
        return pref;
    }
    ull range_hash(const vector<ull> &pref, int l, int r){
        if(l>=r) return 0;
        ull res = pref[r];
        ull left = modmul(pref[l], powv[r-l]);
        if(res >= left) res = res - left;
        else res = res + mod - left;
        return res;
    }
};

struct MultiHash {
    vector<RollingHash> rh;
    MultiHash(){
        rh.emplace_back(1315423911ull, (1ull<<61)-1);
        rh.emplace_back(1469598103934665603ull, (1ull<<61)-1);
        rh.emplace_back(1099511628211ull, (1ull<<61)-1);
    }
    vector<ull> hash_str_multi(const string &s){
        vector<ull> out;
        for(auto &r: rh) out.push_back(r.hash_str(s));
        return out;
    }
    vector<vector<ull>> prefix_multi(const string &s){
        vector<vector<ull>> out;
        for(auto &r: rh) out.push_back(r.prefix_hash(s));
        return out;
    }
};

struct InvertedIndex {
    unordered_map<ull, vector<int>> hash_to_batches;
    unordered_map<int, string> id_to_code;
    MultiHash mh;
    void build(const vector<Batch> &batches, int min_sub=4, int max_sub=64){
        hash_to_batches.clear();
        id_to_code.clear();
        for(size_t i=0;i<batches.size(); ++i){
            const Batch &b = batches[i];
            id_to_code[b.batch_id] = b.code;
            string s = b.code;
            for(auto &rh : mh.rh) rh.ensure_pow(s.size()+5);
            auto prefs = mh.prefix_multi(s);
            int n = (int)s.size();
            for(int l=0; l<n; ++l){
                for(int len=min_sub; len<=max_sub && l+len<=n; ++len){
                    vector<ull> hv;
                    for(size_t k=0;k<prefs.size();++k){
                        hv.push_back(mh.rh[k].range_hash(prefs[k], l, l+len));
                    }
                    ull mix = 1469598103934665603ull;
                    for(auto h: hv) mix = mix * 2 + h;
                    hash_to_batches[mix].push_back(b.batch_id);
                }
            }
        }
    }
    vector<int> lookup(const string &pat){
        if(pat.empty()) return {};
        for(auto &rh : mh.rh) rh.ensure_pow(pat.size()+5);
        auto prefs = mh.prefix_multi(pat);
        vector<ull> hv;
        for(size_t k=0;k<prefs.size();++k) hv.push_back(mh.rh[k].range_hash(prefs[k], 0, (int)pat.size()));
        ull mix = 1469598103934665603ull;
        for(auto h: hv) mix = mix * 2 + h;
        if(hash_to_batches.find(mix) == hash_to_batches.end()) return {};
        auto v = hash_to_batches[mix];
        sort(v.begin(), v.end());
        v.erase(unique(v.begin(), v.end()), v.end());
        return v;
    }
    vector<pair<int,int>> batch_positions_in_code(const string &code, const string &pat){
        vector<pair<int,int>> out;
        auto sp = rh_prefix_multi(code());
        return out;
    }
    vector<int> approximate_lookup(const string &pat, int maxResults=100){
        auto v = lookup(pat);
        if((int)v.size() > maxResults) v.resize(maxResults);
        return v;
    }
    string code_of(int batchid){
        if(id_to_code.find(batchid) == id_to_code.end()) return "";
        return id_to_code[batchid];
    }
    vector<vector<ull>> rh_prefix_multi(const string &s){
        vector<vector<ull>> out;
        for(auto &r : mh.rh) out.push_back(r.prefix_hash(s));
        return out;
    }
};

struct RabinKarp {
    ull base;
    ull mod;
    RabinKarp(ull base_=257ull, ull mod_=(1ull<<61)-1): base(base_), mod(mod_){}
    static inline ull modmul(ull a, ull b){
        u128 z = (u128)a * b;
        ull lo = (ull)z;
        ull hi = (ull)(z >> 64);
        const ull m = (1ull<<61)-1;
        u128 t = (u128)hi * 0xFFFFFFFFFFFFFFFFull + lo;
        ull x = (ull)((t >> 61) + (t & m));
        if(x >= m) x -= m;
        return x;
    }
    ull hash_str(const string &s){
        ull h = 0;
        for(unsigned char c: s){
            h = modmul(h, base) + (ull)c;
            if(h >= mod) h -= mod;
        }
        return h;
    }
    vector<int> search(const string &text, const string &pat){
        vector<int> out;
        int n = (int)text.size(), m = (int)pat.size();
        if(m==0 || n < m) return out;
        ull powb = 1;
        ull ph = 0, th = 0;
        for(int i=0;i<m;++i){
            ph = modmul(ph, base) + (unsigned char)pat[i]; if(ph >= mod) ph -= mod;
            th = modmul(th, base) + (unsigned char)text[i]; if(th >= mod) th -= mod;
            if(i) powb = modmul(powb, base);
        }
        for(int i=m;i<=n;++i){
            if(th == ph){
                int st = i-m;
                bool ok = true;
                for(int j=0;j<m;++j) if(text[st+j] != pat[j]) { ok=false; break; }
                if(ok) out.push_back(st);
            }
            if(i==n) break;
            ull left = modmul((unsigned long long)text[i-m], powb);
            if(th >= left) th = th - left; else th = th + mod - left;
            th = modmul(th, base) + (unsigned char)text[i]; if(th >= mod) th -= mod;
        }
        return out;
    }
};

struct RecallSystem {
    vector<Batch> batches;
    unordered_map<int, int> batchid_to_index;
    InvertedIndex idx;
    RabinKarp rk;
    unordered_map<int, string> batch_metadata_line;
    RecallSystem(){}
    void load(const string &csv){
        batches = load_batches(csv);
        batchid_to_index.clear();
        for(size_t i=0;i<batches.size(); ++i) batchid_to_index[batches[i].batch_id] = i;
        idx.build(batches, 3, 128);
        for(auto &b: batches){
            string meta = to_string(b.batch_id) + "," + to_string(b.product_id) + "," + b.code + "," + b.manufacture_date + "," + b.received_date + "," + to_string(b.qty);
            batch_metadata_line[b.batch_id] = meta;
        }
    }
    vector<pair<int, vector<int>>> find_pattern_in_batches(const string &pat){
        vector<pair<int, vector<int>>> out;
        auto cand = idx.lookup(pat);
        for(int bid: cand){
            int pos = -1;
            auto it = batchid_to_index.find(bid);
            if(it==batchid_to_index.end()) continue;
            string code = batches[it->second].code;
            auto res = rk.search(code, pat);
            if(!res.empty()) out.emplace_back(bid, res);
        }
        return out;
    }
    vector<int> recall_batches_by_pattern(const string &pat){
        auto cand = idx.lookup(pat);
        vector<int> out;
        for(int bid: cand){
            auto it = batchid_to_index.find(bid);
            if(it==batchid_to_index.end()) continue;
            string code = batches[it->second].code;
            auto res = rk.search(code, pat);
            if(!res.empty()) out.push_back(bid);
        }
        sort(out.begin(), out.end());
        out.erase(unique(out.begin(), out.end()), out.end());
        return out;
    }
    vector<pair<int,string>> recall_with_context(const string &pat, int ctx){
        vector<pair<int,string>> out;
        auto cand = idx.lookup(pat);
        for(int bid: cand){
            auto it = batchid_to_index.find(bid);
            if(it==batchid_to_index.end()) continue;
            string code = batches[it->second].code;
            auto res = rk.search(code, pat);
            for(int pos: res){
                int L = max(0, pos - ctx);
                int R = min((int)code.size()-1, pos + (int)pat.size() -1 + ctx);
                string snippet = code.substr(L, R-L+1);
                out.emplace_back(bid, snippet);
            }
        }
        return out;
    }
    void export_recall_list(const string &outcsv, const vector<int> &bids){
        ofstream out(outcsv);
        out<<"batch_id,product_id,qty,code\n";
        for(int b: bids){
            auto it = batchid_to_index.find(b);
            if(it==batchid_to_index.end()) continue;
            auto &bb = batches[it->second];
            out<<bb.batch_id<<","<<bb.product_id<<","<<bb.qty<<",\""<<bb.code<<"\"\n";
        }
        out.close();
    }
    void export_recall_with_positions(const string &outcsv, const string &pat){
        auto list = find_pattern_in_batches(pat);
        ofstream out(outcsv);
        out<<"batch_id,positions\n";
        for(auto &p: list){
            out<<p.first<<",\"";
            for(size_t i=0;i<p.second.size(); ++i){ if(i) out<<";"; out<<p.second[i]; }
            out<<"\"\n";
        }
        out.close();
    }
    vector<int> search_by_regex(const string &regex_pat){
        vector<int> out;
        regex re(regex_pat);
        for(auto &b: batches){
            if(regex_search(b.code, re)) out.push_back(b.batch_id);
        }
        return out;
    }
    vector<int> fuzzy_search_by_hamming(const string &pat, int max_ham){
        vector<int> out;
        auto cand = idx.approximate_lookup(pat, 1000);
        for(int bid: cand){
            auto it = batchid_to_index.find(bid);
            if(it==batchid_to_index.end()) continue;
            string code = batches[it->second].code;
            if(code.size() < pat.size()) continue;
            for(size_t i=0;i+pat.size()<=code.size(); ++i){
                int ham=0;
                for(size_t j=0;j<pat.size(); ++j){ if(code[i+j] != pat[j]){ ham++; if(ham>max_ham) break; } }
                if(ham <= max_ham){ out.push_back(bid); break; }
            }
        }
        sort(out.begin(), out.end());
        out.erase(unique(out.begin(), out.end()), out.end());
        return out;
    }
    void batch_recall_and_route(const string &pat, const string &warehouse_map_csv, const string &outcsv){
        vector<int> recalled = recall_batches_by_pattern(pat);
        unordered_map<int, vector<int>> wh_to_batches;
        auto whrows = read_csv_raw(warehouse_map_csv);
        unordered_map<int,int> batch_to_wh;
        for(auto &r: whrows) if(r.size()>=2){
            int bid = stoi(r[0]);
            int wid = stoi(r[1]);
            batch_to_wh[bid] = wid;
        }
        for(int b: recalled){
            int wid = batch_to_wh.find(b) != batch_to_wh.end() ? batch_to_wh[b] : -1;
            wh_to_batches[wid].push_back(b);
        }
        ofstream out(outcsv);
        out<<"warehouse_id,batch_ids\n";
        for(auto &kv: wh_to_batches){
            out<<kv.first<<",\"";
            for(size_t i=0;i<kv.second.size(); ++i){ if(i) out<<";"; out<<kv.second[i]; }
            out<<"\"\n";
        }
        out.close();
    }
    pair<int,int> stats(){
        int total = (int)batches.size();
        int unique_products = 0;
        unordered_set<int> ps;
        for(auto &b: batches) ps.insert(b.product_id);
        unique_products = ps.size();
        return {total, unique_products};
    }
    void build_secondary_index_by_product(){
        unordered_map<int, vector<int>> prodmap;
        for(auto &b: batches) prodmap[b.product_id].push_back(b.batch_id);
        ofstream out("product_index.csv");
        out<<"product_id,batch_ids\n";
        for(auto &kv: prodmap){
            out<<kv.first<<",\"";
            for(size_t i=0;i<kv.second.size(); ++i){ if(i) out<<";"; out<<kv.second[i]; }
            out<<"\"\n";
        }
        out.close();
    }
    void timed_run(function<void()> f, const string &label){
        auto s = chrono::high_resolution_clock::now();
        f();
        auto e = chrono::high_resolution_clock::now();
        cerr<<label<<" "<<chrono::duration_cast<chrono::milliseconds>(e-s).count()<<" ms\n";
    }
};

static vector<string> split_ws(const string &s){
    vector<string> out; string tmp; stringstream ss(s);
    while(ss>>tmp) out.push_back(tmp);
    return out;
}

int main(int argc,char**argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(argc < 2){ cerr<<"Usage: "<<argv[0]<<" <batches.csv>\n"; return 1; }
    string csv = argv[1];
    RecallSystem rs;
    rs.timed_run([&](){ rs.load(csv); }, "load_batches");
    auto st = rs.stats();
    cout<<"batches="<<st.first<<" unique_products="<<st.second<<"\n";
    cout<<"commands:\nfind pat\nrecall pat\nrecall_ctx pat ctx\nexport_list out.csv pat\nexport_positions out.csv pat\nregex pat\nfuzzy pat maxham\nbuild_prod_index\nbatch_recall_route pat whmap.csv out.csv\nstats\nquit\n";
    string line;
    while(true){
        cout<<"> ";
        if(!getline(cin,line)) break;
        if(line.empty()) continue;
        auto parts = split_ws(line);
        if(parts.empty()) continue;
        string cmd = parts[0];
        if(cmd=="quit"||cmd=="exit") break;
        if(cmd=="find"){
            if(parts.size()<2){ cout<<"find pat\n"; continue; }
            string pat = parts[1];
            auto v = rs.find_pattern_in_batches(pat);
            for(auto &p: v){
                cout<<p.first<<":";
                for(size_t i=0;i<p.second.size(); ++i){ if(i) cout<<","; cout<<p.second[i]; }
                cout<<"\n";
            }
            continue;
        }
        if(cmd=="recall"){
            if(parts.size()<2){ cout<<"recall pat\n"; continue; }
            string pat = parts[1];
            auto v = rs.recall_batches_by_pattern(pat);
            for(auto b: v) cout<<b<<"\n";
            continue;
        }
        if(cmd=="recall_ctx"){
            if(parts.size()<3){ cout<<"recall_ctx pat ctx\n"; continue; }
            string pat = parts[1]; int ctx = stoi(parts[2]);
            auto v = rs.recall_with_context(pat, ctx);
            for(auto &p: v) cout<<p.first<<",\"" << p.second <<"\"\n";
            continue;
        }
        if(cmd=="export_list"){
            if(parts.size()<3){ cout<<"export_list out.csv pat\n"; continue; }
            string out = parts[1], pat = parts[2];
            auto v = rs.recall_batches_by_pattern(pat);
            rs.export_recall_list(out, v);
            cout<<"wrote "<<out<<"\n";
            continue;
        }
        if(cmd=="export_positions"){
            if(parts.size()<3){ cout<<"export_positions out.csv pat\n"; continue; }
            string out = parts[1], pat = parts[2];
            rs.export_recall_with_positions(out, pat);
            cout<<"wrote "<<out<<"\n";
            continue;
        }
        if(cmd=="regex"){
            if(parts.size()<2){ cout<<"regex pat\n"; continue; }
            string pat = parts[1];
            auto v = rs.search_by_regex(pat);
            for(auto b: v) cout<<b<<"\n";
            continue;
        }
        if(cmd=="fuzzy"){
            if(parts.size()<3){ cout<<"fuzzy pat maxham\n"; continue; }
            string pat = parts[1]; int maxh = stoi(parts[2]);
            auto v = rs.fuzzy_search_by_hamming(pat, maxh);
            for(auto b: v) cout<<b<<"\n";
            continue;
        }
        if(cmd=="build_prod_index"){
            rs.build_secondary_index_by_product();
            cout<<"wrote product_index.csv\n";
            continue;
        }
        if(cmd=="batch_recall_route"){
            if(parts.size()<4){ cout<<"batch_recall_route pat whmap.csv out.csv\n"; continue; }
            string pat = parts[1]; string whmap = parts[2]; string out = parts[3];
            rs.batch_recall_and_route(pat, whmap, out);
            cout<<"wrote "<<out<<"\n";
            continue;
        }
        if(cmd=="stats"){
            auto s = rs.stats();
            cout<<"batches="<<s.first<<" products="<<s.second<<"\n";
            continue;
        }
        cout<<"unknown\n";
    }
    return 0;
}
