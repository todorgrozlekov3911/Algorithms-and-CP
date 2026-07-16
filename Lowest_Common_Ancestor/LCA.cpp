#include <bits/stdc++.h>
#include<vector>
#include<exception>
#include<unordered_set>

using namespace std;


struct PlusMinusOneRMQ {

    struct MicroTable {
        int b = 0;                 
        vector<uint16_t> table; // 3D predstava: [mask][l-query][r-query]  
    } micro; // no e "spleskano" => moje da se orientirame s: (mask*B + l) + r

    const vector<int>* A = nullptr;   
    int n = 0;
    int sizeOfBlock = 1;                        
    int nb = 0;                       

    vector<int> blockSize;            
    vector<int> blockMask;            
    vector<int> blockMinPos;          

    vector<int> logarithmTable;                   
    vector<vector<int>> SparseTable;           

    // samo za naglednost
    public:
    PlusMinusOneRMQ() = default;
    PlusMinusOneRMQ(const vector<int>& arr) { build(arr); }

    static int floor_log2_int(int x) {
        return (x <= 1) ? 0 : (31 - __builtin_clz((unsigned)x));
    }

    inline int betterPos(int p, int q) const {
        if (p == -1) return q;
        if (q == -1) return p;
        const auto& a = *A;
        if (a[p] != a[q]) return (a[p] < a[q]) ? p : q;
        return (p < q) ? p : q; 
    }

    void buildMicroTableForB() {
        micro.b = sizeOfBlock;
        int types = 1 << max(0, sizeOfBlock - 1); 
        micro.table.assign((size_t)types * sizeOfBlock * sizeOfBlock, 0);

        vector<int> rel(sizeOfBlock); // rel = relative profile za validnite permutacii na "maskata"
        for (int mask = 0; mask < types; mask++) {
            rel[0] = 0;
            for (int i = 1; i < sizeOfBlock; i++) {
                int step = ((mask >> (i - 1)) & 1) ? +1 : -1; // maskata se razglejda ot lest significant bit
                rel[i] = rel[i - 1] + step;
            }

            for (int l = 0; l < sizeOfBlock; l++) {
                int minIdx = l;
                for (int r = l; r < sizeOfBlock; r++) {
                    if (rel[r] < rel[minIdx]) minIdx = r;
                    micro.table[((mask * sizeOfBlock + l) * sizeOfBlock + r)] = (uint16_t)minIdx;
                }
            }
        }
    }

    int queryInBlock(int blockId, int l, int r) const {
        int start = blockId * sizeOfBlock; // startov index na bloka
        int s = blockSize[blockId];

        if (s == sizeOfBlock) {
            int mask = blockMask[blockId];
            uint16_t idx = micro.table[((mask * sizeOfBlock + l) * sizeOfBlock + r)];
            return start + (int)idx; // globalen index
        }

        // general case kogato N ne e mnojitel na B, togava imame posleden blok s po maluk razmer
        // Pone v statiqta (do kolkoto az razbrah), e okei za posledniq blok da se prejalim i da namerim po "standartniq" nachin RMQ-to
        int best = start + l;
        for (int i = start + l + 1; i <= start + r; i++) {
            best = betterPos(best, i);
        }
        return best;
    }

    int queryBlocks(int lBlock, int rBlock) const {
        if (lBlock > rBlock) return -1;
        int len = rBlock - lBlock + 1;
        int k = logarithmTable[len]; // 2^k <= len < 2^(k+1)
        int p1 = SparseTable[k][lBlock];
        int p2 = SparseTable[k][rBlock - (1 << k) + 1];
        return betterPos(p1, p2);
    }
    //postroqvame standartniq RMQ  SparceTable model, samo che tuk go pravim za minimumite na vseki blok
    void buildSparseTableOnBlocks() {
        logarithmTable.assign(nb + 1, 0);
        logarithmTable[1] = 0;
        for (int i = 2; i <= nb; i++)
            logarithmTable[i] = logarithmTable[i >> 1] + 1;

        int K = logarithmTable[nb] + 1;
        SparseTable.assign(K, vector<int>(nb, -1));

        for (int i = 0; i < nb; i++) 
            SparseTable[0][i] = blockMinPos[i]; // nai maluk el za dadeniq blok i

        for (int k = 1; k < K; k++) {
            int span = 1 << k;
            int half = span >> 1;
            for (int i = 0; i + span <= nb; i++) {
                SparseTable[k][i] = betterPos(SparseTable[k - 1][i], SparseTable[k - 1][i + half]);
            }
        }
    }

    void buildBlocks() {
        nb = (n + sizeOfBlock - 1) / sizeOfBlock;
        blockSize.assign(nb, 0);
        blockMask.assign(nb, 0);
        blockMinPos.assign(nb, -1);

        const auto& a = *A;
        
        int fullBlocks = n / sizeOfBlock;
        int rem = n % sizeOfBlock;

    
        for (int bid = 0; bid < fullBlocks; bid++) {
            int start = bid * sizeOfBlock;
            blockSize[bid] = sizeOfBlock;

            int mask = 0;
            for (int i = 1; i < sizeOfBlock; i++) {
                int diff = a[start + i] - a[start + i - 1];
                if (diff == +1) mask |= (1 << (i - 1));
            }
            blockMask[bid] = mask;

            blockMinPos[bid] = queryInBlock(bid, 0, sizeOfBlock - 1);
        }

        
        if (rem != 0) {
            int bid = fullBlocks;
            int start = bid * sizeOfBlock;
            blockSize[bid] = rem;

            int mask = 0;
            for (int i = 1; i < rem; i++) {
                int diff = a[start + i] - a[start + i - 1];
                if (diff == +1) mask |= (1 << (i - 1));
            }
            blockMask[bid] = mask;

            blockMinPos[bid] = queryInBlock(bid, 0, rem - 1);
        }
    }

    void build(const vector<int>& arr) {
        A = &arr;
        n = (int)arr.size();
        if (n == 0) return;

       
        int lgN = floor_log2_int(n);
        sizeOfBlock = max(1, lgN / 2);

        buildMicroTableForB();
        buildBlocks();
        buildSparseTableOnBlocks();
    }

    // RMQ index of minimum in [l..r]
    int rmqIndex(int l, int r) const {
        if (l > r) std::swap(l, r);
        int bl = l / sizeOfBlock;
        int br = r / sizeOfBlock;

        if (bl == br) {
            return queryInBlock(bl, l - bl * sizeOfBlock, r - bl * sizeOfBlock);
        }

        int leftPos = queryInBlock(bl, l - bl * sizeOfBlock, blockSize[bl] - 1);
        int rightPos = queryInBlock(br, 0, r - br * sizeOfBlock);

        int ans = betterPos(leftPos, rightPos);

        if (bl + 1 <= br - 1) {
            int midPos = queryBlocks(bl + 1, br - 1);
            ans = betterPos(ans, midPos);
        }

        return ans;
    }
};

struct LCA {
    int n = 0;
    int root;
    vector<vector<int>> g;
    vector<int> first;       // purvo sreshtane v oilerovata razhodka
    vector<int> euler;       // oilerova razhodka
    vector<int> depthEuler;  // dulbochina v durvoto bazirana na oilerova razhodka

    PlusMinusOneRMQ rmq;

    explicit LCA(int n = 0) : n(n), g(n + 1), first(n + 1, -1), root(1) {}

    void addEdge(int u, int v) { g[u].push_back(v); }

    void build() {

        euler.clear();
        depthEuler.clear();
        fill(first.begin(), first.end(), -1);

        struct Frame { int u; int idx; int depth; };
        vector<Frame> st;
        st.push_back({root, 0, 0});

        auto record = [&](int u, int d) {
            if (first[u] == -1) first[u] = (int)euler.size();
            euler.push_back(u);
            depthEuler.push_back(d);
        };

        record(root, 0);

        while (!st.empty()) {
            auto &top = st.back();
            int u = top.u;
            if (top.idx < (int)g[u].size()) {
                int v = g[u][top.idx++];
                st.push_back({v, 0, top.depth + 1});
                record(v, top.depth + 1);
            } else {
                st.pop_back();
                if (!st.empty()) {
                    record(st.back().u, st.back().depth);
                }
            }
        }

        rmq = PlusMinusOneRMQ(depthEuler);
    }

    int lca(int u, int v) const {
        if(u>n || v>n)
            throw invalid_argument("Non existing nodes");

        int i = first[u], j = first[v];
        int pos = rmq.rmqIndex(i, j);
        return euler[pos];
    }
     void isValideTree()
    {
         vector<int> seen(n + 1, 0);
        if (!dfs(root, seen))
            throw invalid_argument("Invalid tree (cycle)");
        for (int i = 1; i <= n; ++i) {
            if (seen[i] == 0)
                throw invalid_argument("Invalid tree (disconnected)");
        }
    }
    private:

    bool dfs(int u, vector<int>& seen)
    {
        seen[u] = 1; 
        for (int v : g[u]) {
            if (seen[v] == 1) return false; 
            if (seen[v] == 0 && !dfs(v, seen)) 
                return false;
        }
        seen[u] = 2; 
        return true;
    }

};



int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    
    
    try{
        int n, q;
        cin >> n >> q;

        LCA solver(n);

        vector<int> indeg(n + 1, 0);
        for (int i = 0; i < n - 1; i++) {
            int u, v;
            cin >> u >> v;
            if(u >n || v>n)
                throw invalid_argument("Nomera na edin ot vuzlite e po golqm ot N");
            solver.addEdge(u, v);
            indeg[v]++;
        }

        int root = 1;
        for (int i = 1; i <= n; i++) {
            if (indeg[i] == 0) { root = i; break; }
        }
        solver.root = root;
        solver.isValideTree();
        solver.build();

        cout<<"Answer for queries:"<<"\n";
        for (int i = 0; i < q; i++) {
            int u, v;
            cin >> u >> v;
            cout<<solver.lca(u, v) << "\n";
        }
    }catch(exception& e)
    {
        std::cerr<<e.what();
        return 1;
    }    

    return 0;
}

