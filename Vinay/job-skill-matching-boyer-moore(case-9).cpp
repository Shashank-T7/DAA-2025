#include <bits/stdc++.h>
using namespace std;

// ===================== TRIE FOR SKILL / TITLE PREFIX =====================
struct TrieNode {
    bool isEnd;
    unordered_map<char, TrieNode*> next;
    TrieNode() : isEnd(false) {}
};

class Trie {
public:
    Trie() { root = new TrieNode(); }

    void insert(const string &s) {
        TrieNode *cur = root;
        for (char c : s) {
            c = tolower(c);
            if (!cur->next.count(c)) cur->next[c] = new TrieNode();
            cur = cur->next[c];
        }
        cur->isEnd = true;
    }

    // return all words with given prefix from a provided dictionary
    vector<string> startsWith(const string &prefix,
                              const vector<string> &dict) const {
        vector<string> res;
        for (const string &w : dict) {
            if (w.size() >= prefix.size() &&
                equal(prefix.begin(), prefix.end(), w.begin(),
                      [](char a, char b){ return tolower(a)==tolower(b); })) {
                res.push_back(w);
            }
        }
        return res;
    }

private:
    TrieNode *root;
};

// ===================== BOYER–MOORE STRING SEARCH ========================
// bad character heuristic only (good enough for project)
vector<int> buildBadCharTable(const string &pat) {
    const int ALPH = 256;
    vector<int> bad(ALPH, -1);
    for (int i = 0; i < (int)pat.size(); ++i)
        bad[(unsigned char)pat[i]] = i;
    return bad;
}

bool boyerMooreSearch(const string &text, const string &pat) {
    int n = text.size();
    int m = pat.size();
    if (m == 0 || n < m) return false;

    vector<int> bad = buildBadCharTable(pat);
    int shift = 0;

    while (shift <= n - m) {
        int j = m - 1;
        while (j >= 0 && tolower(pat[j]) == tolower(text[shift + j]))
            j--;

        if (j < 0) {
            return true; // match found
        } else {
            int bcIndex = (unsigned char)text[shift + j];
            int lastOcc = bad[bcIndex];
            int s = max(1, j - lastOcc);
            shift += s;
        }
    }
    return false;
}

// ===================== DATA MODELS ======================================
struct Candidate {
    int id;
    string name;
    string skillsText;      // free text skills
    int area;               // location node
};

struct Job {
    int id;
    string title;
    string description;
    string requiredSkillKeyword; // primary skill word to search
    int area;                    // job location node
};

// ranking score = (#skillMatch * 10) - distance
struct RankedCandidate {
    int candId;
    int jobId;
    int score;
};

struct RankCmp {
    bool operator()(const RankedCandidate &a,
                    const RankedCandidate &b) const {
        return a.score < b.score;    // max-heap
    }
};

// ===================== SIMPLE DISTANCE MODEL ============================
// For demo: distance between areas = abs difference of ids
int distanceBetweenAreas(int a, int b) {
    return abs(a - b);
}

// ===================== GLOBAL STATE =====================================
Trie skillTrie;
vector<string> skillDictionary;                  // for prefix listing
unordered_map<int, Candidate> candidates;
unordered_map<int, Job> jobs;
unordered_map<int,
    priority_queue<RankedCandidate, vector<RankedCandidate>, RankCmp>>
    jobHeaps;                                    // jobId -> heap of candidates

int nextCandId = 1, nextJobId = 1;

// ===================== OPERATIONS =======================================
void addSkillToTrie() {
    string skill;
    cin.ignore();
    cout << "Enter skill keyword to add to dictionary: ";
    getline(cin, skill);
    skillDictionary.push_back(skill);
    skillTrie.insert(skill);
    cout << "Skill added.\n";
}

void suggestSkillsByPrefix() {
    string pref;
    cin.ignore();
    cout << "Enter skill prefix: ";
    getline(cin, pref);
    vector<string> res = skillTrie.startsWith(pref, skillDictionary);
    cout << "Skills starting with \"" << pref << "\":\n";
    for (auto &s : res) cout << "- " << s << "\n";
    if (res.empty()) cout << "No matches.\n";
}

void addCandidate() {
    Candidate c;
    c.id = nextCandId++;
    cin.ignore();
    cout << "Enter candidate name: ";
    getline(cin, c.name);
    cout << "Enter candidate skills (free text): ";
    getline(cin, c.skillsText);
    cout << "Enter candidate area (int node id): ";
    cin >> c.area;

    candidates[c.id] = c;
    cout << "Candidate added with ID " << c.id << ".\n";
}

void addJob() {
    Job j;
    j.id = nextJobId++;
    cin.ignore();
    cout << "Enter job title: ";
    getline(cin, j.title);
    cout << "Enter job description: ";
    getline(cin, j.description);
    cout << "Enter primary required skill keyword (single word): ";
    getline(cin, j.requiredSkillKeyword);
    cout << "Enter job area (int node id): ";
    cin >> j.area;

    jobs[j.id] = j;
    cout << "Job added with ID " << j.id << ".\n";
}

int computeMatchScore(const Candidate &c, const Job &j) {
    int matches = 0;

    // Boyer–Moore search of required skill keyword in candidate skills text
    if (boyerMooreSearch(c.skillsText, j.requiredSkillKeyword)) {
        matches += 1;
    }

    // Optionally search in job description as well (double weight)
    if (boyerMooreSearch(j.description, j.requiredSkillKeyword)) {
        matches += 1;
    }

    int dist = distanceBetweenAreas(c.area, j.area);
    int score = matches * 10 - dist;       // simple scoring rule
    return score;
}

void buildRankingsForJob() {
    int jobId;
    cout << "Enter job ID to build candidate ranking: ";
    cin >> jobId;
    if (!jobs.count(jobId)) {
        cout << "Job not found.\n";
        return;
    }
    const Job &j = jobs[jId];

    priority_queue<RankedCandidate, vector<RankedCandidate>, RankCmp> heap;

    for (auto &p : candidates) {
        const Candidate &c = p.second;
        int score = computeMatchScore(c, j);
        if (score > 0) {
            heap.push({c.id, j.id, score});
        }
    }

    jobHeaps[j.id] = heap;
    cout << "Ranking heap built for Job " << j.id << " (" << j.title << ").\n";
}

void showTopKForJob() {
    int jobId, k;
    cout << "Enter job ID and k: ";
    cin >> jobId >> k;
    if (!jobHeaps.count(jobId)) {
        cout << "No ranking heap for this job. Run ranking first.\n";
        return;
    }

    auto heap = jobHeaps[jobId]; // copy for display
    cout << "Top " << k << " candidates for job " << jobId << ":\n";
    while (k-- > 0 && !heap.empty()) {
        RankedCandidate rc = heap.top(); heap.pop();
        Candidate &c = candidates[rc.candId];
        cout << "Candidate " << c.id << " (" << c.name << ")"
             << " | Score: " << rc.score
             << " | Area: " << c.area << "\n";
    }
}

void listJobs() {
    cout << "---- Jobs ----\n";
    for (auto &p : jobs) {
        const Job &j = p.second;
        cout << "ID: " << j.id
             << " | Title: " << j.title
             << " | Skill: " << j.requiredSkillKeyword
             << " | Area: " << j.area << "\n";
    }
}

void listCandidates() {
    cout << "---- Candidates ----\n";
    for (auto &p : candidates) {
        const Candidate &c = p.second;
        cout << "ID: " << c.id
             << " | Name: " << c.name
             << " | Area: " << c.area << "\n";
    }
}

// ===================== MENU / MAIN ======================================
void printMenu() {
    cout << "\n=== Citywide Job & Skill Matching Portal ===\n";
    cout << "1. Add skill keyword to Trie dictionary\n";
    cout << "2. Suggest skills by prefix (Trie)\n";
    cout << "3. Add candidate\n";
    cout << "4. Add job\n";
    cout << "5. List jobs\n";
    cout << "6. List candidates\n";
    cout << "7. Build ranking heap for a job (Boyer–Moore + heap)\n";
    cout << "8. Show top-K candidates for a job\n";
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
            case 1: addSkillToTrie(); break;
            case 2: suggestSkillsByPrefix(); break;
            case 3: addCandidate(); break;
            case 4: addJob(); break;
            case 5: listJobs(); break;
            case 6: listCandidates(); break;
            case 7: buildRankingsForJob(); break;
            case 8: showTopKForJob(); break;
            case 0: cout << "Exiting.\n"; break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != 0);

    return 0;
}
