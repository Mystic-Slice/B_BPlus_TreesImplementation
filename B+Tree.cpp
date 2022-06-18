#include <bits/stdc++.h>
using namespace std;

// Just me being lazy to write a for loop
template <typename T>
ostream& operator<<(ostream& out, const vector<T>& v) {
    for (auto& element : v){
        // if(not element) continue;
        if(element == INT_MAX) continue;
        out << element << " ";
    }
    return out;
}

class Node {
    public:
        bool leaf; // Node is a leaf or not
        Node* parent;
        Node* next; // Next node in the linked list (only found in leaves)
        int minChildren, maxChildren, numChildren, numKeys;
        vector<int> keys;
        vector<Node*> children;

        Node(int limit): leaf(false), 
                        parent(nullptr),
                        next(nullptr), 
                        minChildren(limit), 
                        maxChildren(limit*2),
                        numChildren(0), 
                        numKeys(0),
                        // Initialize vectors filled with INT_MAX/nullptr to 
                        // maxsize + 1 to accomodate the one extra 
                        // element added before spliting
                        keys(limit*2, INT_MAX), // maxKeys = maxChildren - 1
                        children(limit*2 + 1, nullptr) {}

        bool isLeaf() { return leaf; }

        bool canDonate() { return numKeys > (minChildren - 1); }

        // bool isBoundsRespected() { return numKeys >= minChildren - 1 && numKeys <= maxChildren - 1; }

        Node* leftSibling() {
            if(not parent) return nullptr;
            int index = find(parent->children.begin(), parent->children.end(), this) - parent->children.begin();
            if(index == 0) return nullptr;
            else return parent->children[index - 1];
        }

        Node* rightSibling() {
            if(not parent) return nullptr;
            int index = find(parent->children.begin(), parent->children.end(), this) - parent->children.begin();
            if(index == parent->children.size() - 1) return nullptr;
            else return parent->children[index + 1];
        }

        int donateGreatestKey() {
            int donatedKey = keys[numKeys - 1];
            keys[numKeys - 1] = INT_MAX;
            numKeys--;
            return donatedKey;
        }

        Node* donateGreatestChild() {
            Node* donatedChild = children[numChildren - 1];
            children[numChildren - 1] = nullptr;
            numChildren--;
            return donatedChild;
        }

        int donateLeastKey() {
            int donatedKey = keys[0];
            keys.erase(keys.begin());
            keys.push_back(INT_MAX);
            numKeys--;
            return donatedKey;
        }

        Node* donateLeastChild() {
            Node* donatedChild = children[0];
            children.erase(children.begin());
            children.push_back(nullptr);
            numChildren--;
            return donatedChild;
        }

        int getNextKey(int donatedKey) {
            int index = find(keys.begin(), keys.end(), donatedKey) - keys.begin();
            int nextKey = keys[index + 1];
            keys.erase(keys.begin() + index + 1);
            numKeys--;
            return nextKey;
        }

        int getPrevKey(int donatedKey) {
            int index = find(keys.begin(), keys.end(), donatedKey) - keys.begin();
            int prevKey = keys[index - 1];
            keys.erase(keys.begin() + index - 1);
            numKeys--;
            return prevKey;
        }

        void replaceDonatedKey(int donatedKey) {
            int index = find(keys.begin(), keys.end(), donatedKey) - keys.begin();
            if(index >= keys.size()) return;
            keys[index] = rightMostKey(children[index]);
        }

        int rightMostKey(Node* node) {
            if(node->isLeaf()) {
                return node->keys[node->numKeys - 1];
            } else {
                return rightMostKey(node->children[node->numChildren - 1]);
            }
        }

        void merge(Node* rightNode) {

            auto currIndex = find(parent->children.begin(), parent->children.end(), this) - parent->children.begin();
            parent->keys.erase(parent->keys.begin() + currIndex);
            parent->numKeys--;
            
            auto itrNode = find(parent->children.begin(), parent->children.end(), rightNode);
            parent->children.erase(itrNode);
            parent->children.push_back(nullptr);
            parent->numChildren--;

            
            for(int i = numKeys; i < numKeys + rightNode->numKeys; i++) {
                keys[i] = rightNode->keys[i - numKeys];
            }
            numKeys += rightNode->numKeys;
        }

        // Adds the key into its correct position in sorted order
        void addKey(int key) {
            int index;
            for(index = 0; index<keys.size(); index++) {
                if(key < keys[index]) break;
            }
            for(int i = keys.size() - 1; i>index; i--) {
                keys[i] = keys[i - 1];
            }
            keys[index] = key;
            numKeys++;
        }

        // Adds the child to the same corresponding index as the key
        void addChild(Node* child, int key, int offset = 0) {
            int index = find(keys.begin(), keys.end(), key) - keys.begin() + offset;
            for(int i = children.size() - 1; i>index; i--) {
                children[i] = children[i-1];
            }
            children[index] = child;
            numChildren++;
        }

        // Update the previous left neighbour's next pointer to the newly added child node
        void updateNext(int key) {
            int index = find(keys.begin(), keys.end(), key) - keys.begin();
            if(index == 0) return updateByDFS(children[0]);
            // if(not index) return;
            children[index-1]->next = children[index];
        }

        // Incase the child that was added is the leftmost node of the subtree
        // Its left neighbour has to found by DFS
        void updateByDFS(Node* child) {
            Node* root = this;
            while(root->parent) root = root->parent;

            Node* prevLeaf = nullptr;
            stack<Node*> st;
            st.push(root);
            while(!st.empty()) {
                Node* node = st.top(); st.pop();
                if(node->isLeaf()) {
                    if(node == child){
                        if(prevLeaf) prevLeaf->next = child;
                        return;
                    } else {
                        prevLeaf = node;
                    }
                } else {
                    // Should add in reverse to get leaves in left to right order
                    for(int i = node->numChildren-1; i >= 0; i--) {
                        if(node->children[i]) st.push(node->children[i]);
                    }
                }
            }
        }

        // Splits the node into two if the bound is exceeded
        // Returns root
        // Recursively reaches the root even if nodes along the way
        // are valid...just to get the new root incase the root changes
        Node* splitIfNeeded() {

            // Take the median to the parent node
            // Split the current node into two halves
            // Let the current node just retain the keys
            // greater than the median
            // The current node will become the right child

            // Dont do anything if the bound is respected
            if(numKeys <= maxChildren - 1) {
                if(parent) return parent->splitIfNeeded();
                else return this;
            }

            int median = numKeys/2;
            int medianKey = keys[median];
            
            // Create a new node for left child
            Node* leftNode = new Node(minChildren);
            leftNode->leaf = leaf;
            leftNode->next = this;

            // Retain the median element in the left node only for leaves
            // This is taken care by the value of maxI            
            int maxI = median;
            // Only point of difference between b-tree and b+tree implementation :)
            if(leaf) maxI++;

            // Copy the elements of leftnode
            for(int i = 0; i<=maxI; i++) {
                if(i != maxI) leftNode->keys[i] = keys[i];
                leftNode->children[i] = children[i];
                if(leftNode->children[i])
                    leftNode->children[i]->parent = leftNode; // Update parent pointer
            }

            // Shift the elements of right node to the front
            for(int i = median + 1; i < keys.size(); i++) {
                keys[i - median - 1] = keys[i];
            }

            for(int i = median + 1; i < children.size(); i++) {
                children[i - median - 1] = children[i];
            }

            // Update the attributes of leftnode and rightnode (current node)
            leftNode->numKeys = maxI;
            numKeys = keys.size() - (median + 1);

            leftNode->numChildren = maxI + 1;
            numChildren = children.size() - (median + 1);

            // Clean the rest of the vectors
            for(int i = leftNode->numKeys; i<leftNode->keys.size(); i++) {
                leftNode->keys[i] = INT_MAX;
            }

            for(int i = leftNode->numChildren; i<leftNode->children.size(); i++) {
                leftNode->children[i] = nullptr;
            }

            for(int i = numKeys; i<keys.size(); i++) {
                keys[i] = INT_MAX;
            }

            for(int i = numChildren; i<children.size(); i++) {
                children[i] = nullptr;
            }

            // Add the median to the parent
            if(not parent) {
                // Current node is the root
                // Has to be split and new root is created
                Node* newParent = new Node(minChildren);
                newParent->addKey(medianKey);
                newParent->addChild(leftNode, medianKey);
                newParent->children[1] = this;
                newParent->numChildren++;

                this->parent = newParent;
                leftNode->parent = newParent;
                return parent;
            } else {
                // Splitting propagated to the parent                
                parent->addKey(medianKey);
                parent->addChild(leftNode, medianKey);
                parent->updateNext(medianKey);
                leftNode->parent = parent;
                return parent->splitIfNeeded();
            }
        }
};

class BPlusTree {
    public:
        // !TODO: Make private
        Node* root;
        int minChildren, maxChildren;

        BPlusTree(int limit): minChildren(limit), maxChildren(limit*2), root(nullptr) {}

        /*
            Return True or False depending on whether
            the key exists in the tree
        */
        bool search(int key) {
            Node* node = searchNode(key, root);
            if(not node) return false;

            for(auto k: node->keys) if(k == key) return true;
            return false;
        }

        Node* searchNode(int key, Node* node) {
            if(not node) return nullptr;
            if(node->isLeaf()) return node;

            for(int i = 0; i<node->keys.size(); i++) {
                if(key <= node->keys[i]) return searchNode(key, node->children[i]);
            }
            return searchNode(key, node->children.back());
        }

        void insertKey(int key) {
            if(not root) {
                root = new Node(minChildren);
                root->addKey(key);
                root->leaf = true;
                return;
            }
            Node* node = searchNode(key, root);
            node->addKey(key);
            root = node->splitIfNeeded();
        }

        void deleteKey(int key) {
            // If data present in leaf and only in the leaf
            // delete that no problem

            // If data present in leaf and internal node
            // but no problem with bounds
            // delete both and replace the internal node with the least value from the right subtree

            // If deleting data causes problems with bounds
            // Try borrow from siblings
            // else merge with siblings
            Node* node = searchNode(key, root);
            auto itr = find(node->keys.begin(), node->keys.end(), key);
            if(itr == node->keys.end()) {
                cout<<"Node not found"<<endl;
                return;
            }

            int index = itr - node->keys.begin();
            for(int i = index; i < node->keys.size() - 1; i++) {
                node->keys[i] = node->keys[i + 1];
            }
            node->keys.back() = INT_MAX;
            node->numKeys--;

            manageUnderflowLeaf(node, key);
            manageUnderflowParent(node->parent);
        }

        void manageUnderflowLeaf(Node* node, int key) {
            if(node->numKeys >= minChildren - 1) return;
            if(node == root) {
                if(node->numKeys == 0) root = nullptr;
                return;
            };

            // Try borrowing from leftSibling
            Node* leftSibling = node->leftSibling();
            if(leftSibling && leftSibling->canDonate()) {
                int donatedKey = leftSibling->donateGreatestKey();
                node->addKey(donatedKey);
                node->parent->replaceDonatedKey(donatedKey);
                return;
            }

            // Try borrowing from rightSibling
            Node* rightSibling = node->rightSibling();
            if(rightSibling && rightSibling->canDonate()) {
                int donatedKey = rightSibling->donateLeastKey();
                node->addKey(donatedKey);
                node->parent->replaceDonatedKey(donatedKey);
                return;
            }

            // Try merge with leftSibling
            if(leftSibling) {
                leftSibling->merge(node);
                return;
            }

            // Try merge with rightSibling
            if(rightSibling) {
                node->merge(rightSibling);
                return;
            }
        }

        void manageUnderflowParent(Node* node) {
            cout<<"Here"<<endl;
            if(not node) return;

            if(node->numKeys >= minChildren - 1) return;

            if(node == root) {
                if(node->numKeys == 0) root = nullptr;
                return;
            };

            // Try borrowing from leftSibling
            Node* leftSibling = node->leftSibling();
            if(leftSibling && leftSibling->canDonate()) {
                int donatedKey = leftSibling->donateGreatestKey();
                Node* donatedChild = leftSibling->donateGreatestChild();
                node->parent->addKey(donatedKey);
                int nextKey = node->parent->getNextKey(donatedKey);
                node->addKey(nextKey);
                node->addChild(donatedChild, nextKey);
                return;
            }

            // Try borrowing from rightSibling
            Node* rightSibling = node->rightSibling();
            if(rightSibling && rightSibling->canDonate()) {
                int donatedKey = rightSibling->donateLeastKey();
                Node* donatedChild = rightSibling->donateLeastChild();
                node->parent->addKey(donatedKey);
                int prevKey = node->parent->getPrevKey(donatedKey);
                node->addKey(prevKey);
                node->addChild(donatedChild, prevKey, 1);
                return;
            }

            // !TODO: Merging of internal nodes
            // Try merge with leftSibling
            if(leftSibling) {
                leftSibling->merge(node);
                return;
            }

            // Try merge with rightSibling
            if(rightSibling) {
                node->merge(rightSibling);
                return;
            }

            manageUnderflowParent(node->parent);
        }

        void inorderTraversal() {
            if(not root) {
                cout<<"Tree is empty"<<endl;
                return;
            }
            inorderTraversal(root);
        }

        void inorderTraversal(Node* node) {
            if(not node) return;

            for(int i = 0; i<node->keys.size(); i++) {
                inorderTraversal(node->children[i]);
                // Can stop at the first INT_MAX encountered
                if(node->keys[i] == INT_MAX) break;
                cout<<node->keys[i]<<" ";
            }
            if(node->keys.back() != INT_MAX) inorderTraversal(node->children.back());
        }

        // !TODO: Maybe have a separate pointer for the leftmost node
        void traverseLeaves() {
            if(not root) {
                cout<<"Tree is empty"<<endl;
                return;
            }

            // Find the leftmost node
            Node* node = root;
            while(not node->isLeaf()) node = node->children[0];

            // Use the `next` pointer to traverse through the elements
            while(node){
                cout<<node->keys;
                node = node->next;
            }
            cout<<endl;
        }

        void levelOrder() {
            int prevDepth = 0;
            queue<pair<Node*, int>> q;
            q.push({root, 0});
            while(!q.empty()) {
                Node* node = q.front().first;
                int depth = q.front().second;
                q.pop();
                if(depth != prevDepth) {
                    cout<<endl;
                    prevDepth =depth;
                }

                cout<<node->keys<<" | ";

                // cout<<node->keys<<" ("<<node<<")"<<" | ";
                // cout<<node->keys<<" ("<<node->parent<<")"<<" | ";

                // cout<<"("<<node<<") ";
                // for(auto child: node->children) {
                //     if(not child) break;
                //     cout<<child<<" ";
                // }
                // cout<<"("<<node->parent<<")"<<" | ";
                for(auto child: node->children) {
                    if(not child) break;
                    q.push({child, depth+1});
                }
            }
            cout<<endl;
        }
};

int main() {
    freopen("error.txt", "w", stderr);
    BPlusTree tree(3);
    tree.insertKey(10);
    tree.insertKey(20);
    tree.insertKey(30);
    tree.insertKey(40);
    tree.insertKey(50);
    tree.insertKey(60);
    tree.insertKey(70);
    tree.insertKey(80);
    tree.insertKey(90);
    tree.insertKey(100);
    tree.insertKey(110);
    tree.insertKey(120);
    tree.insertKey(130);
    tree.insertKey(140);
    tree.insertKey(150);
    tree.insertKey(160);
    tree.insertKey(170);
    tree.insertKey(180);
    tree.insertKey(190);
    tree.insertKey(200);
    tree.insertKey(210);
    tree.insertKey(220);
    tree.insertKey(230);
    tree.insertKey(240);
    tree.insertKey(250);
    tree.insertKey(260);
    tree.insertKey(270);
    tree.insertKey(280);
    tree.insertKey(290);
    tree.insertKey(300);
    tree.insertKey(310);    
    tree.insertKey(95);

    tree.inorderTraversal();
    // cout<<endl;
    // cout<<"Leaves"<<endl;
    // tree.traverseLeaves();
    // cout<<"LevelOrder"<<endl;

    cout<<"Before deletion"<<endl;
    tree.levelOrder();

        // tree.deleteKey(50);
        tree.deleteKey(60);
        tree.deleteKey(90);
        tree.deleteKey(95);
        tree.deleteKey(100);
        tree.deleteKey(30);
        tree.deleteKey(40);
        tree.deleteKey(20);
        tree.deleteKey(70);
        tree.deleteKey(80);
        tree.deleteKey(50);

    cout<<endl<<endl<<endl;
    cout<<"After deletion"<<endl;
    tree.levelOrder();
}