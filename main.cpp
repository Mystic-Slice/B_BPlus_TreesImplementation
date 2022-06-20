#include "B+Tree.cpp"
#include "BTree.cpp"
#include <bits/stdc++.h>

int main() {
    int maxChildren = 5;
    BTree tree(maxChildren);

    vector<int> insertKeys = { 
        770,
        449,
        943,
        248,
        765,
        192,
        454,
        743,
        792,
        390,
        104,
        12,
        37,
        85,
        636,
        790,
        503,
        299,
        445,
        926,
        290,
        440,
        755,
        155,
        780,
        65,
        275,
        368,
        311,
        54
    };

    for(auto i: insertKeys) {
        cout<<endl<<endl;
        tree.insertKey(i);
        cout<<"Inserted "<<i<<endl;
        tree.levelOrder();
        // cout<<"Leaves ";
        // tree.traverseLeaves();
    }

    vector<int> deleteKeys = {
        790,
        104,
        454,
        765,
        503,
        248,
        192,
        290,
        12,
        37
    };

    for(auto i: deleteKeys) {
        cout<<endl<<endl;
        tree.deleteKey(i, tree.root);
        cout<<"Deleted "<<i<<endl;
        tree.levelOrder();
        // cout<<"Leaves ";
        // tree.traverseLeaves();
    }
}