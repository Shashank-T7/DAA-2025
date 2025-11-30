
// Purpose:
// - Maintain a searchable database of product/food batch records (batch codes, dates, source, location).
// - Provide fast substring/pattern search across many batch codes using Rabin-Karp rolling hash.
// - Support multi-pattern queries, bulk simulation, CSV export of recall lists, and a small interactive CLI.
//
// Design notes:
// - Uses a 64-bit rolling hash (modulus implicit in uint64 overflow) plus verification to avoid false positives.
// - Patterns and texts are treated as plain ASCII strings; code normalisation trims whitespace.
// - The system is intentionally practical and readable; no unnecessary comments or fluff.

#include <bits/stdc++.h>
using namespace std;
using u64 = unsigned long long;

struct BatchRecord {
    int id;
    string code;        // alphanumeric batch code or composed string
    string product;
    string facility;    // where produced
    string receivedDate; // YYYY-MM-DD
    int quantity;
};

class BatchDB {
public:
    void addBatch(const BatchRecord &b) {
        db.push_back(b);
        index.push_back(b.code);
    }

    size_t size() const { return db.size(); }

    const BatchRecord& getByIndex(size_t i) const { return db[i]; }

    // Return all indices where record code contains pattern (using Rabin-Karp)
    vector<int> searchPattern(const string &pattern) const {
        vector<int> res;
        if (pattern.empty()) return res;
        u64 patHash = rollingHash(pattern);
        int m = (int)pattern.size();
        u64 power = computePower(m);
        for (size_t i = 0; i < index.size(); ++i) {
            const string &text = index[i];
            if ((int)text.size() < m) continue;
            u64 h = 0;
            for (int j = 0; j < m; ++j) h = h * BASE + (unsigned char)text[j];
            if (h == patHash) {
                if (text.compare(0, m, pattern) == 0) res.push_back((int)i);
            }
            for (int j = m; j < (int)text.size(); ++j) {
                h = h * BASE + (unsigned char)text[j];
                u64 remove = (u64)(unsigned char)text[j - m] * power;
                h -= remove;
                if (h == patHash) {
                    if (text.compare(j - m + 1, m, pattern) == 0) res.push_back((int)i);
                }
            }
        }
        return res;
    }

    // Search multiple patterns, return union of indices
    vector<int> searchAnyPattern(const vector<string> &patterns) const {
        unordered_set<int> seen;
        for (auto &p : patterns) {
            auto r = searchPattern(p);
            for (int idx : r) seen.insert(idx);
        }
        vector<int> out(seen.begin(), seen.end());
        sort(out.begin(), out.end());
        return out;
    }

    // Export list of batch records by indices to CSV
    bool exportCSV(const string &filename, const vector<int> &indices) const {
        ofstream out(filename);
        if (!out.is_open()) return false;
        out << "id,code,product,facility,receivedDate,quantity\n";
        for (int i : indices) {
            if (i < 0 || i >= (int)db.size()) continue;
            const auto &b = db[i];
            out << b.id << "," << escapeCsv(b.code) << "," << escapeCsv(b.product) << ","
                << escapeCsv(b.facility) << "," << b.receivedDate << "," << b.quantity << "\n";
        }
        out.close();
        return true;
    }

    // Bulk generate random batch records (for simulation/testing)
    void simulateBulk(int startId, int count) {
        static const vector<string> products = {
            "Rice","Wheat","Groundnut","Dal","MilkPowder","CannedBeans","Spices","Biscuits"
        };
        static const vector<string> facilities = {
            "Central Mill","North Plant","East Agro","South Packhouse","West Store"
        };
        mt19937 rng((unsigned)chrono::high_resolution_clock::now().time_since_epoch().count());
        uniform_int_distribution<int> p(0, (int)products.size()-1);
        uniform_int_distribution<int> f(0, (int)facilities.size()-1);
        uniform_int_distribution<int> qty(10, 2000);
        for (int i = 0; i < count; ++i) {
            BatchRecord b;
            b.id = startId + i;
            b.product = products[p(rng)];
            b.facility = facilities[f(rng)];
            b.quantity = qty(rng);
            b.receivedDate = randomDateString(rng);
            b.code = makeRandomCode(b.product, b.facility, b.id, rng);
            addBatch(b);
        }
    }

    // Helper: find by exact batch code (full string match)
    int findBatchByCode(const string &code) const {
        for (size_t i = 0; i < db.size(); ++i) {
            if (db[i].code == code) return (int)i;
        }
        return -1;
    }

private:
    vector<BatchRecord> db;
    vector<string> index;

    static constexpr u64 BASE = 1315423911ULL; // big odd base; works with 64-bit overflow

    static u64 rollingHash(const string &s) {
        u64 h = 0;
        for (unsigned char c : s) h = h * BASE + c;
        return h;
    }

    static u64 computePower(int m) {
        u64 p = 1;
        for (int i = 0; i < m; ++i) p = p * BASE;
        return p;
    }

    static string randomDateString(mt19937 &rng) {
        uniform_int_distribution<int> y(2023, 2025);
        uniform_int_distribution<int> mo(1,12);
        uniform_int_distribution<int> d(1,28);
        int yy = y(rng), mm = mo(rng), dd = d(rng);
        char buf[16];
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d", yy, mm, dd);
        return string(buf);
    }

    static string makeRandomCode(const string &product, const string &facility, int id, mt19937 &rng) {
        // Example: RICE-CENTRAL-000123-XL
        string prod = product;
        for (auto &c : prod) if (isspace((unsigned char)c)) c = '_';
        string fac = facility;
        for (auto &c : fac) if (isspace((unsigned char)c)) c = '_';
        uniform_int_distribution<int> sufx(100,999);
        int s = sufx(rng);
        char buf[64];
        snprintf(buf, sizeof(buf), "%s-%s-%06d-%03d", prod.c_str(), fac.c_str(), id, s);
        return string(buf);
    }

    static string escapeCsv(const string &s) {
        if (s.find_first_of(",\"\n") == string::npos) return s;
        string out = "\"";
        for (char c : s) {
            if (c == '"') out += "\"\"";
            else out += c;
        }
        out += "\"";
        return out;
    }
};

// ===========================
// CLI and recall management
// ===========================

static string readLineTrim(const string &prompt) {
    cout << prompt;
    string s;
    if (!getline(cin, s)) return string();
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return string();
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static int readInt(const string &prompt, int def) {
    string s = readLineTrim(prompt);
    if (s.empty()) return def;
    try { return stoi(s); } catch (...) { return def; }
}

static void printRecord(const BatchRecord &b) {
    cout << "id=" << b.id << " code=" << b.code << " product=" << b.product
         << " facility=" << b.facility << " date=" << b.receivedDate
         << " qty=" << b.quantity << "\n";
}

class RecallManager {
public:
    RecallManager(BatchDB &db) : database(db) { }

    // find batches matching pattern and mark recalled (logical)
    vector<int> recallByPattern(const string &pattern) {
        auto idxs = database.searchPattern(pattern);
        for (int i : idxs) recalledIndices.insert(i);
        return idxs;
    }

    // recall if any of the provided patterns match (OR)
    vector<int> recallByAnyPattern(const vector<string> &patterns) {
        auto idxs = database.searchAnyPattern(patterns);
        for (int i : idxs) recalledIndices.insert(i);
        return idxs;
    }

    // recall exact batch codes
    bool recallByExactCode(const string &code) {
        int idx = database.findBatchByCode(code);
        if (idx >= 0) {
            recalledIndices.insert(idx);
            return true;
        }
        return false;
    }

    // list currently recalled batches
    vector<int> listRecalled() const {
        vector<int> out(recalledIndices.begin(), recalledIndices.end());
        sort(out.begin(), out.end());
        return out;
    }

    // export recalled batches CSV
    bool exportRecalled(const string &filename) const {
        vector<int> idxs = listRecalled();
        return database.exportCSV(filename, idxs);
    }

private:
    BatchDB &database;
    unordered_set<int> recalledIndices;
};

// ===========================
// Small interactive driver
// ===========================

static void showMenu() {
    cout << "\nRabin-Karp Batch Recall — Options\n";
    cout << " 1) Add a batch record\n";
    cout << " 2) Bulk simulate N records\n";
    cout << " 3) Search batches by pattern\n";
    cout << " 4) Recall batches by pattern\n";
    cout << " 5) Recall batches by multiple patterns (OR)\n";
    cout << " 6) Recall exact batch code\n";
    cout << " 7) List recalled batches\n";
    cout << " 8) Export recalled batches to CSV\n";
    cout << " q) Quit\n";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    BatchDB db;
    RecallManager rm(db);

    // small seeded data so interactive session has some content
    db.simulateBulk(1000, 150);

    bool stop = false;
    while (!stop) {
        showMenu();
        string cmd = readLineTrim("Choice> ");
        if (cmd == "1") {
            BatchRecord b;
            b.id = readInt("Batch id: ", (int)db.size() + 1001);
            b.code = readLineTrim("Batch code: ");
            b.product = readLineTrim("Product name: ");
            b.facility = readLineTrim("Facility: ");
            b.receivedDate = readLineTrim("Received date (YYYY-MM-DD): ");
            b.quantity = readInt("Quantity: ", 0);
            if (b.code.empty()) {
                cout << "Code required. Skipping.\n";
            } else {
                db.addBatch(b);
                cout << "Added batch.\n";
            }
        } else if (cmd == "2") {
            int cnt = readInt("How many to generate? ", 100);
            int startId = (int)db.size() + 1000;
            db.simulateBulk(startId, cnt);
            cout << "Generated " << cnt << " records.\n";
        } else if (cmd == "3") {
            string pat = readLineTrim("Pattern to search: ");
            if (pat.empty()) { cout << "Empty pattern.\n"; continue; }
            auto idxs = db.searchPattern(pat);
            cout << "Found in " << idxs.size() << " records:\n";
            for (int i : idxs) printRecord(db.getByIndex(i));
        } else if (cmd == "4") {
            string pat = readLineTrim("Pattern to recall: ");
            if (pat.empty()) { cout << "Empty pattern.\n"; continue; }
            auto idxs = rm.recallByPattern(pat);
            cout << "Recalled " << idxs.size() << " batches.\n";
        } else if (cmd == "5") {
            string line = readLineTrim("Enter patterns separated by commas: ");
            vector<string> patterns;
            string cur;
            for (char c : line) {
                if (c == ',') { if (!cur.empty()) { patterns.push_back(trim(cur)); cur.clear(); } }
                else cur.push_back(c);
            }
            if (!cur.empty()) patterns.push_back(trim(cur));
            if (patterns.empty()) { cout << "No patterns.\n"; continue; }
            auto idxs = rm.recallByAnyPattern(patterns);
            cout << "Recalled " << idxs.size() << " batches for given patterns.\n";
        } else if (cmd == "6") {
            string code = readLineTrim("Exact batch code: ");
            if (code.empty()) { cout << "Empty.\n"; continue; }
            bool ok = rm.recallByExactCode(code);
            cout << (ok ? "Recalled exact batch.\n" : "No batch with that code.\n");
        } else if (cmd == "7") {
            auto rec = rm.listRecalled();
            cout << "Currently recalled (" << rec.size() << "):\n";
            for (int i : rec) printRecord(db.getByIndex(i));
        } else if (cmd == "8") {
            string fname = readLineTrim("CSV filename (default=recalled.csv): ");
            if (fname.empty()) fname = "recalled.csv";
            bool ok = rm.exportRecalled(fname);
            cout << (ok ? "Exported to " + fname + "\n" : "Export failed\n");
        } else if (cmd == "q" || cmd == "quit") {
            stop = true;
        } else {
            cout << "Unknown option\n";
        }
    }

    cout << "Exiting.\n";
    return 0;
}
