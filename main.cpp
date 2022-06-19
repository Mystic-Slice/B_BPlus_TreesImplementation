#include "B+Tree.cpp"
#include <bits/stdc++.h>

int main() {
    int maxChildren = 5;
    BPlusTree tree(maxChildren);

    vector<int> insertKeys = { 
        1804,
        846,
        1681,
        1714,
        1957,
        424,
        719,
        1649,
        596,
        1189,
        1025,
        1350,
        783,
        1102,
        2044,
        1967,
        1365,
        1540,
        304,
        1303,
        35,
        521,
    };

    for(auto i: insertKeys) {
        cout<<endl<<endl;
        tree.insertKey(i);
        cout<<"Inserted "<<i<<endl;
        tree.levelOrder();
        cout<<"Leaves ";
        tree.traverseLeaves();
    }

    tree.inorderTraversal();
    cout<<endl;

    vector<int> deleteKeys = {
        1365,
        1714,
        783,
        596,
        304,
        35,
        1957,
        1967,
        1189,
        1102,
        521,
        1025,
        424,
        1681,
        1804,
        719,
        846,
        1540,
        1303,
        1649,
        2044,
        1350
    };

    for(auto i: deleteKeys) {
        cout<<endl<<endl;
        tree.deleteKey(i);
        cout<<"Deleted "<<i<<endl;
        tree.levelOrder();
        cout<<"Leaves ";
        tree.traverseLeaves();
    }
}