#include<vector>
template<typename T, typename MergeFunc>
class SeggmentTree{
public:
    SeggmentTree(const std::vector<T>& v): n(v.size()), N(1)
    {
        while(N<n) N<<=1;
        tree.assign(2*N, MergeFunc::dummy);
        
        for(int i=0;i<n;i++) tree[N+i] = v[i];
        for(int i = N-1;i>=1;i--) tree[i] = MergeFunc{}(tree[2*i], tree[2*i+1]); 
    }

    T query(int l, int r)
    {
        return query_rec(l,r, 0,N-1, 1);
    }

    void update(int i,const T& new_val)
    {
        return update(i,i,new_val);
    }

    void update(int l, int r, const T& new_val)
    {
        return update_rec(l,r, 0, N-1, new_val, 1);
    }

private:

    T query_rec(int ql, int qr, int tl, int tr, int node)
    {
        if(ql<=tl && tr<=qr) return tree[node];
        if(qr<tl || ql>tr)   return MergeFunc::dummy;

        int mid = (tr-tl)/2+tl;
        return MergeFunc{}(query_rec(ql, qr, tl, mid, 2*node), query_rec(ql, qr, mid+1, tr, 2*node+1));
    }

    void update_rec(int ql, int qr, int tl, int tr,int node, const T& new_val)
    {
        if(ql == tl && qr == tr)
        {
            tree[node] = new_val;
            return;
        }

        if(ql > tr || qr < tl) return;

        int mid = (tr- trl) / 2 + tl;
        update_rec(ql, qr, tl,      mid, 2 * node,     val);
        update_rec(ql, qr, mid + 1, tr,  2 * node + 1, val);
        tree[node] = MergeFunc{}(2*node, 2*node+1);

    }


    int n, N;

    std::vector<T> tree;
};