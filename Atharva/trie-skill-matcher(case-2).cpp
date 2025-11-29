#include <bits/stdc++.h>
using namespace std;

// A Trie node representing one character in a skill word.
struct SkillNode {
    array<SkillNode*, 26> next;
    bool isEnd;
    int wordCount;
    vector<int> linkedProfiles;

    SkillNode() : isEnd(false), wordCount(0) {
        next.fill(nullptr);
    }
};

// The main trie class for storing skills and performing prefix queries.
class SkillTrie {
public:
    SkillTrie() {
        root = new SkillNode();
    }

    void insertSkill(const string &skill, int profileId) {
        SkillNode* cur = root;
        for (char c : skill) {
            if (isalpha(c) == false) continue;
            char x = tolower(c);
            int idx = x - 'a';
            if (idx < 0 || idx >= 26) continue;
            if (cur->next[idx] == nullptr) {
                cur->next[idx] = new SkillNode();
            }
            cur = cur->next[idx];
            cur->wordCount++;
        }
        cur->isEnd = true;
        cur->linkedProfiles.push_back(profileId);
    }

    bool containsSkill(const string &skill) {
        SkillNode* cur = root;
        for (char c : skill) {
            if (!isalpha(c)) continue;
            char x = tolower(c);
            int idx = x - 'a';
            if (idx < 0 || idx >= 26) return false;
            if (cur->next[idx] == nullptr) return false;
            cur = cur->next[idx];
        }
        return cur->isEnd;
    }

    vector<string> skillsWithPrefix(const string &prefix) {
        SkillNode* node = findNode(prefix);
        vector<string> result;
        if (!node) return result;
        string current = prefix;
        explore(node, current, result);
        return result;
    }

    vector<int> profileMatches(const string &skill) {
        SkillNode* cur = root;
        for (char c : skill) {
            if (!isalpha(c)) continue;
            int idx = tolower(c) - 'a';
            if (idx < 0 || idx >= 26) return {};
            if (cur->next[idx] == nullptr) return {};
            cur = cur->next[idx];
        }
        if (cur->isEnd == false) return {};
        return cur->linkedProfiles;
    }

private:
    SkillNode* root;

    SkillNode* findNode(const string &s) {
        SkillNode* cur = root;
        for (char c : s) {
            if (!isalpha(c)) continue;
            int idx = tolower(c) - 'a';
            if (idx < 0 || idx >= 26) return nullptr;
            if (cur->next[idx] == nullptr) return nullptr;
            cur = cur->next[idx];
        }
        return cur;
    }

    void explore(SkillNode* node, string &current, vector<string> &out) {
        if (node->isEnd) out.push_back(current);
        for (int i = 0; i < 26; i++) {
            if (node->next[i]) {
                current.push_back('a' + i);
                explore(node->next[i], current, out);
                current.pop_back();
            }
        }
    }
};

// Represents a profile in Aroha Nagar (student, worker, startup).
struct Profile {
    int id;
    string name;
    vector<string> skills;
};

class SkillDirectory {
public:
    SkillDirectory() { }

    void addProfile(const Profile &p) {
        profiles[p.id] = p;
        for (const string &s : p.skills) {
            skillTrie.insertSkill(s, p.id);
        }
    }

    bool hasSkill(const string &s) {
        return skillTrie.containsSkill(s);
    }

    vector<string> suggestions(const string &prefix) {
        return skillTrie.skillsWithPrefix(prefix);
    }

    vector<Profile> profilesWithSkill(const string &skill) {
        vector<int> ids = skillTrie.profileMatches(skill);
        vector<Profile> res;
        for (int id : ids) {
            if (profiles.count(id)) {
                res.push_back(profiles[id]);
            }
        }
        return res;
    }

private:
    unordered_map<int, Profile> profiles;
    SkillTrie skillTrie;
};

// Simulates real-time updates where new skills are registered over time.
class SkillStreamSimulator {
public:
    SkillStreamSimulator(SkillDirectory &dir, vector<Profile> baseData)
        : directory(dir), data(move(baseData)) { }

    void streamUpdates(int intervalMs, int cycles) {
        int idx = 0;
        for (int i = 0; i < cycles; i++) {
            if (idx >= (int)data.size()) idx = 0;
            directory.addProfile(data[idx]);
            idx++;
            this_thread::sleep_for(chrono::milliseconds(intervalMs));
        }
    }

private:
    SkillDirectory &directory;
    vector<Profile> data;
};

// Utility printing
void printProfiles(const vector<Profile> &v) {
    for (const auto &p : v) {
        cout << "Profile ID: " << p.id << " | Name: " << p.name << " | Skills: ";
        for (auto &s : p.skills) cout << s << " ";
        cout << "\n";
    }
}

void printStrings(const vector<string> &v) {
    for (const auto &s : v) cout << s << "\n";
}

int main() {
    SkillDirectory directory;

    vector<Profile> inputs = {
        {1, "Ayesha", {"python", "pytorch", "pandas"}},
        {2, "Raghav", {"java", "javascript", "json"}},
        {3, "Meera", {"networking", "netsec"}},
        {4, "Kabir", {"cpp", "c", "cuda"}},
        {5, "Jia", {"react", "redux", "ruby"}},
        {6, "Nirav", {"python", "django"}},
        {7, "Fiza", {"html", "css", "js"}},
        {8, "Shlok", {"typescript", "tailwind", "threejs"}},
        {9, "Ishan", {"go", "graphql"}},
        {10, "Dev", {"ml", "maths", "matlab"}}
    };

    for (auto &p : inputs) directory.addProfile(p);

    cout << "\n--- Skill Prefix Query: 'py' ---\n";
    auto prefixes = directory.suggestions("py");
    printStrings(prefixes);

    cout << "\n--- Profiles With Skill 'python' ---\n";
    auto pythonUsers = directory.profilesWithSkill("python");
    printProfiles(pythonUsers);

    cout << "\n--- Contains Skill 'redux'? ---\n";
    cout << (directory.hasSkill("redux") ? "Yes\n" : "No\n");

    cout << "\n--- Running Skill-Stream Simulator ---\n";
    vector<Profile> newData = {
        {11, "Riya", {"rust", "rocket"}},
        {12, "Arjun", {"kotlin", "kserve"}},
        {13, "Diya", {"swift", "spritekit"}},
        {14, "Farhan", {"perl", "php"}}
    };

    SkillStreamSimulator simulator(directory, newData);
    simulator.streamUpdates(50, 4);

    cout << "\n--- Profiles With Skill 'rust' After Streaming ---\n";
    auto rustUsers = directory.profilesWithSkill("rust");
    printProfiles(rustUsers);

    return 0;
}
