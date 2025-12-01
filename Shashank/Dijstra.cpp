<!-- DIJKSTRA CARD (paste into your page, styled like the others) -->
<div class="card fade-up">
  <div class="case-title">Algorithm Used – Dijkstra’s Algorithm</div>

  <p class="case-desc">
    Dijkstra finds shortest paths from a single source to all other nodes when edge weights are non-negative.
    It's ideal for routing problems (maintenance routes, dispatch, navigation) where fast, repeatable shortest-path queries are needed.
  </p>

  <p class="case-text">
    Efficiency & Optimizations: Typical O(E log V) using a binary heap (priority_queue). Use adjacency lists,
    avoid decrease-key by pushing duplicates to the heap (skip stale entries). For extremely large graphs consider
    multi-level heuristics, goal-directed search (A*) or contraction hierarchies.
  </p>

  <details>
    <summary style="cursor:pointer;font-weight:700">View Code</summary>

    <pre id="code-dijkstra">
// Dijkstra's algorithm (C++) - reads: n m, then m edges (u v w) 1-indexed, then source s
// Outputs distances from s to all nodes (1..n), -1 for unreachable.

#include <bits/stdc++.h>
using namespace std;
using ll = long long;
const ll INF = (ll)4e18;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    if (!(cin >> n >> m)) return 0;
    vector<vector<pair<int,int>>> adj(n+1);
    for (int i = 0; i < m; ++i) {
        int u, v; int w;
        cin >> u >> v >> w;
        // if graph is undirected add both; for directed only add (u->v)
        adj[u].push_back({v, w});
        // adj[v].push_back({u, w}); // uncomment for undirected graphs
    }

    int s; cin >> s;

    vector<ll> dist(n+1, INF);
    dist[s] = 0;
    // min-heap of (distance, node)
    priority_queue<pair<ll,int>, vector<pair<ll,int>>, greater<pair<ll,int>>> pq;
    pq.push({0, s});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d != dist[u]) continue; // stale entry
        for (auto &edge : adj[u]) {
            int v = edge.first;
            int w = edge.second;
            if (dist[v] > d + w) {
                dist[v] = d + w;
                pq.push({dist[v], v});
            }
        }
    }

    for (int i = 1; i <= n; ++i) {
        if (dist[i] >= INF/4) cout << -1;
        else cout << dist[i];
        if (i < n) cout << ' ';
    }
    cout << '\n';
    return 0;
}
    </pre>

    <div style="margin-top:8px; display:flex; gap:8px;">
      <button class="btn" onclick="copyCode('code-dijkstra')">Copy</button>
      <button class="btn" onclick="downloadCode('Dijkstra.cpp','code-dijkstra')">Download</button>
    </div>

  </details>
</div>

<!-- Include these helper functions once in your page (if not already present) -->
<script>
function copyCode(id){
  const el = document.getElementById(id);
  if(!el) return alert('Code block not found');
  navigator.clipboard.writeText(el.innerText).then(()=> {
    alert('Code copied to clipboard.');
  }, ()=> alert('Copy failed.'));
}

function downloadCode(filename, id){
  const el = document.getElementById(id);
  if(!el) return alert('Code block not found');
  const data = el.innerText;
  const blob = new Blob([data], {type: 'text/plain'});
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url; a.download = filename;
  document.body.appendChild(a); a.click();
  setTimeout(()=>{ URL.revokeObjectURL(url); a.remove(); }, 200);
}
</script>

