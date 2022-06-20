#include "B+Tree.cpp"
#include "BTree.cpp"
#include <bits/stdc++.h>

void printMenu(bool bplus = false) {
    cout<<"Menu:"<<endl;
    cout<<"1. Search key <S key>"<<endl;
    cout<<"2. Insert key <I key>"<<endl;
    cout<<"3. Delete key <D key>"<<endl;
    cout<<"4. Level order traversal <LT>"<<endl;
    cout<<"5. Inorder traversal <IT>"<<endl;
    if(bplus) cout<<"6. Traverse leaves <TL>"<<endl;
    cout<<"Exit <E>"<<endl;
    cout<<endl<<endl;
}

int main() {
    cout<<"Choose tree: "<<endl;
    cout<<"1. BTree"<<endl;
    cout<<"2. B+Tree"<<endl;

    int choice;
    cin>>choice;

    cout<<"Enter maximum number of children per node (order of tree): "<<endl;
    int numChildren;
    cin>>numChildren;

    if(choice == 1) {
        BTree tree(numChildren);
        string input;
        int inputKey;
        while(true) {
            // printMenu();        
            cin>>input;
            if(input == "S") {
                cin>>inputKey;
                cout<<tree.search(inputKey)<<endl;
            } else if(input == "I") {
                cin>>inputKey;
                tree.insertKey(inputKey);
                cout<<"Inserted "<<inputKey<<endl;
                tree.levelOrder();
            } else if(input == "D") {
                cin>>inputKey;
                tree.deleteKey(inputKey, tree.root);;
                cout<<"Deleted "<<inputKey<<endl;
                tree.levelOrder();
            } else if(input == "LT") {
                cout<<"Level order Traversal :"<<endl;
                tree.levelOrder();
            } else if(input == "IT") {
                cout<<"Inorder Traversal :"<<endl;
                tree.inorderTraversal();
                cout<<endl;
            } else if(input == "E") {
                break;
            } else {
                cout<<"Invalid input"<<endl;
            }
            cout<<endl<<endl;
        }
    } else if(choice == 2) {
        BPlusTree tree(numChildren);
        string input;
        int inputKey;
        while(true) {
            // printMenu();
            cin>>input;
            if(input == "S") {
                cin>>inputKey;
                cout<<tree.search(inputKey)<<endl;
            } else if(input == "I") {
                cin>>inputKey;
                tree.insertKey(inputKey);
                cout<<"Inserted "<<inputKey<<endl;
                tree.levelOrder();
            } else if(input == "D") {
                cin>>inputKey;
                tree.deleteKey(inputKey);
                cout<<"Deleted "<<inputKey<<endl;
                tree.levelOrder();
            } else if(input == "LT") {
                cout<<"Level order Traversal :"<<endl;
                tree.levelOrder();
            } else if(input == "IT") {
                cout<<"Inorder Traversal :"<<endl;
                tree.inorderTraversal();
                cout<<endl;
            } else if(input == "E") {
                break;
            } else if(input == "TL") {
                cout<<"Leaves Traversal: "<<endl;
                tree.traverseLeaves();
            } else {
                cout<<"Invalid input"<<endl;
            }
            cout<<endl<<endl;
        }
    } else {
        cout<<"Invalid choice"<<endl;
    }
    return 0;
}