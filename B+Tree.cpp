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
        void addChild(Node* child, int key) {
            int index = find(keys.begin(), keys.end(), key) - keys.begin();
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

        void insert(int key) {
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
                for(auto child: node->children) {
                    if(not child) break;
                    q.push({child, depth+1});
                }
            }
        }
};

int main() {
    BPlusTree tree(2);
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);
    tree.insert(40);
    tree.insert(50);
    tree.insert(60);
    tree.insert(70);
    tree.insert(80);
    tree.insert(90);
    tree.insert(100);
    tree.insert(110);
    tree.insert(120);
    tree.insert(130);
    tree.insert(140);
    tree.insert(150);
    tree.insert(160);
    tree.insert(170);
    tree.insert(180);
    tree.insert(190);
    tree.insert(200);
    tree.insert(210);
    tree.insert(220);
    tree.insert(230);
    tree.insert(240);
    tree.insert(250);
    tree.insert(260);
    tree.insert(270);
    tree.insert(280);
    tree.insert(290);
    tree.insert(300);
    tree.insert(310);    
    tree.insert(95);

    tree.inorderTraversal();
    cout<<endl;
    cout<<"Leaves"<<endl;
    tree.traverseLeaves();
    cout<<"LevelOrder"<<endl;
    tree.levelOrder();
}