#include <bits/stdc++.h>

using namespace std;

struct RMQ {
    const vector<int>* values = nullptr;
    vector<int> lg;
    vector<vector<int>> st;

    RMQ() = default;

    explicit RMQ(const vector<int>& a) {
        build(a);
    }

    int better(int i, int j) const {
        if ((*values)[i] != (*values)[j]) return (*values)[i] < (*values)[j] ? i : j;
        return i < j ? i : j;
    }

    void build(const vector<int>& a) {
        values = &a;
        int n = (int)a.size();
        if (n == 0) return;

        lg.assign(n + 1, 0);
        for (int i = 2; i <= n; ++i) lg[i] = lg[i / 2] + 1;

        int k = lg[n] + 1;
        st.assign(k, vector<int>(n));
        for (int i = 0; i < n; ++i) st[0][i] = i;

        for (int j = 1; j < k; ++j) {
            int len = 1 << j;
            int half = len >> 1;
            for (int i = 0; i + len <= n; ++i) {
                st[j][i] = better(st[j - 1][i], st[j - 1][i + half]);
            }
        }
    }

    int queryIndex(int l, int r) const {
        if (l > r) swap(l, r);
        int len = r - l + 1;
        int j = lg[len];
        return better(st[j][l], st[j][r - (1 << j) + 1]);
    }
};

struct LCA {
    int n;
    int root;
    vector<vector<int>> g;
    vector<int> first;
    vector<int> euler;
    vector<int> depth;
    RMQ rmq;

    explicit LCA(int n) : n(n), root(1), g(n + 1), first(n + 1, -1) {}

    void addEdge(int u, int v) {
        g[u].push_back(v);
    }

    bool dfsCheck(int u, vector<int>& state) {
        state[u] = 1;
        for (int v : g[u]) {
            if (state[v] == 1) return false;
            if (state[v] == 0 && !dfsCheck(v, state)) return false;
        }
        state[u] = 2;
        return true;
    }

    void validateTree() {
        vector<int> state(n + 1, 0);
        if (!dfsCheck(root, state)) throw invalid_argument("Invalid tree (cycle)");
        for (int i = 1; i <= n; ++i) {
            if (state[i] == 0) throw invalid_argument("Invalid tree (disconnected)");
        }
    }

    void dfsEuler(int u, int d) {
        if (first[u] == -1) first[u] = (int)euler.size();
        euler.push_back(u);
        depth.push_back(d);
        for (int v : g[u]) {
            dfsEuler(v, d + 1);
            euler.push_back(u);
            depth.push_back(d);
        }
    }

    void build() {
        fill(first.begin(), first.end(), -1);
        euler.clear();
        depth.clear();
        dfsEuler(root, 0);
        rmq.build(depth);
    }

    int query(int u, int v) const {
        if (u > n || v > n) throw invalid_argument("Non existing nodes");
        int pos = rmq.queryIndex(first[u], first[v]);
        return euler[pos];
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    try {
        int n, q;
        cin >> n >> q;

        LCA solver(n);
        vector<int> indeg(n + 1, 0);

        for (int i = 0; i < n - 1; ++i) {
            int u, v;
            cin >> u >> v;
            if (u > n || v > n) throw invalid_argument("Nomera na edin ot vuzlite e po golqm ot N");
            solver.addEdge(u, v);
            indeg[v]++;
        }

        for (int i = 1; i <= n; ++i) {
            if (indeg[i] == 0) {
                solver.root = i;
                break;
            }
        }

        solver.validateTree();
        solver.build();

        cout << "Answer for queries:\n";
        for (int i = 0; i < q; ++i) {
            int u, v;
            cin >> u >> v;
            cout << solver.query(u, v) << '\n';
        }
    } catch (exception& e) {
        cerr << e.what();
        return 1;
    }

    return 0;
}
