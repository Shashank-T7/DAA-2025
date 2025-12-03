#include <bits/stdc++.h>
using namespace std;

// ===================== HASHING =====================
// Simple wrapper around std::hash<string>
string hashString(const string &s) {
    size_t h = hash<string>{}(s);
    stringstream ss;
    ss << hex << h;
    return ss.str();
}

// ===================== BLOOM FILTER =================
struct BloomFilter {
    int m;                 // size of bit array
    int k;                 // number of hash functions
    vector<bool> bits;

    BloomFilter(int m_ = 1000, int k_ = 3) {
        init(m_, k_);
    }

    void init(int m_, int k_) {
        m = m_;
        k = k_;
        bits.assign(m, false);
    }

    // k different hash functions from base hash
    int hash_i(const string &s, int i) const {
        size_t h1 = hash<string>{}(s);
        size_t h2 = hash<string>{>("salt" + to_string(i) + s);
        size_t h = h1 + i * h2 + i * i;
        return (int)(h % m);
    }

    void insert(const string &s) {
        for (int i = 0; i < k; ++i) {
            int idx = hash_i(s, i);
            bits[idx] = true;
        }
    }

    bool mayContain(const string &s) const {
        for (int i = 0; i < k; ++i) {
            int idx = hash_i(s, i);
            if (!bits[idx]) return false; // definitely not present
        }
        return true; // maybe present
    }
};

// ===================== MERKLE TREE ==================
struct MerkleTree {
    vector<string> leaves;           // leaf hashes
    vector<vector<string>> levels;   // levels[0] = leaves, levels.back() = root

    void build(const vector<string> &leafHashes) {
        leaves = leafHashes;
        levels.clear();
        if (leaves.empty()) return;

        levels.push_back(leaves);             // level 0

        while (levels.back().size() > 1) {
            const vector<string> &cur = levels.back();
            vector<string> nxt;
            for (size_t i = 0; i < cur.size(); i += 2) {
                if (i + 1 < cur.size()) {
                    // internal node hash = hash(left || right)
                    string combined = cur[i] + cur[i + 1];
                    nxt.push_back(hashString(combined));
                } else {
                    // odd leaf promoted (standard Merkle duplication variant)
                    nxt.push_back(cur[i]);
                }
            }
            levels.push_back(nxt);
        }
    }

    string getRoot() const {
        if (levels.empty()) return "";
        return levels.back()[0];
    }

    // Return Merkle proof (sibling hashes) for leaf index idx
    vector<string> getProof(int idx) const {
        vector<string> proof;
        if (levels.empty() || idx < 0 || idx >= (int)leaves.size()) return proof;

        int pos = idx;
        for (size_t lvl = 0; lvl + 1 < levels.size(); ++lvl) {
            const auto &cur = levels[lvl];
            int sibling = (pos % 2 == 0) ? pos + 1 : pos - 1;
            if (sibling < (int)cur.size()) {
                proof.push_back(cur[sibling]);
            } else {
                proof.push_back(""); // indicates no sibling (odd node)
            }
            pos /= 2;
        }
        return proof;
    }

    // Verify proof for given leaf hash against root
    static bool verifyProof(const string &leafHash,
                            int idx,
                            const vector<string> &proof,
                            const string &root) {
        string cur = leafHash;
        int pos = idx;
        for (const string &sib : proof) {
            if (sib.empty()) {
                // no sibling, just propagate
                cur = cur;
            } else if (pos % 2 == 0) {
                cur = hashString(cur + sib);
            } else {
                cur = hashString(sib + cur);
            }
            pos /= 2;
        }
        return cur == root;
    }
};

// ===================== DOCUMENT MODEL ===============
struct Document {
    string id;        // govt doc ID
    string content;   // simplified text content
    string hash;      // fingerprint
};

vector<Document> docs;                // issued documents
BloomFilter bloom(2000, 4);
MerkleTree merkle;

// ===================== OPS ==========================
void issueDocument() {
    Document d;
    cin.ignore();
    cout << "Enter document ID: ";
    getline(cin, d.id);
    cout << "Enter document content (single line): ";
    getline(cin, d.content);

    d.hash = hashString(d.id + "|" + d.content);
    docs.push_back(d);

    // update bloom filter with doc ID or hash
    bloom.insert(d.id);

    cout << "Document issued. Hash: " << d.hash << "\n";
}

void rebuildMerkle() {
    vector<string> leafHashes;
    for (auto &d : docs) leafHashes.push_back(d.hash);
    merkle.build(leafHashes);
    cout << "Merkle tree rebuilt. Current root: " << merkle.getRoot() << "\n";
}

int findDocIndexById(const string &id) {
    for (int i = 0; i < (int)docs.size(); ++i) {
        if (docs[i].id == id) return i;
    }
    return -1;
}

void verifyDocument() {
    cin.ignore();
    string id, content;
    cout << "Enter presented document ID: ";
    getline(cin, id);

    // 1) Bloom filter quick check
    if (!bloom.mayContain(id)) {
        cout << "Bloom filter says: document definitely DOES NOT exist.\n";
        return;
    } else {
        cout << "Bloom filter says: document MAY exist. Proceeding...\n";
    }

    cout << "Enter presented content (single line): ";
    getline(cin, content);
    string presentedHash = hashString(id + "|" + content);

    // 2) Lookup index
    int idx = findDocIndexById(id);
    if (idx == -1) {
        cout << "Document ID not found in local registry.\n";
        return;
    }

    // 3) Merkle proof-based verification
    vector<string> proof = merkle.getProof(idx);
    string root = merkle.getRoot();
    if (root.empty()) {
        cout << "Merkle tree not built yet. Rebuild and try again.\n";
        return;
    }

    bool ok = MerkleTree::verifyProof(presentedHash, idx, proof, root);
    if (!ok) {
        cout << "Merkle proof FAILED. Document may be forged or altered.\n";
    } else {
        cout << "Merkle proof OK. Document is authentic under root: " << root << "\n";
    }
}

void listDocuments() {
    cout << "---- Issued Documents ----\n";
    for (auto &d : docs) {
        cout << "ID: " << d.id << " | Hash: " << d.hash << "\n";
    }
    cout << "Merkle root: " << merkle.getRoot() << "\n";
}

// ===================== MENU / MAIN ==================
void printMenu() {
    cout << "\n=== Government Document Auth (Merkle + Bloom) ===\n";
    cout << "1. Issue new document\n";
    cout << "2. Rebuild Merkle tree from all documents\n";
    cout << "3. List issued documents & Merkle root\n";
    cout << "4. Verify a document (Bloom + Merkle proof)\n";
    cout << "0. Exit\n";
    cout << "Choice: ";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int choice;
    do {
        printMenu();
        cin >> choice;
        switch (choice) {
            case 1: issueDocument(); break;
            case 2: rebuildMerkle(); break;
            case 3: listDocuments(); break;
            case 4: verifyDocument(); break;
            case 0: cout << "Exiting.\n"; break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != 0);

    return 0;
}
