#include "BTree.h"

BNode::BNode(int limit): leaf(false), 
                parent(nullptr),
                minChildren(ceil(limit/2.0)), 
                maxChildren(limit),
                numChildren(0), 
                numKeys(0),
                // Initialize vectors filled with INT_MAX/nullptr to 
                // maxsize + 1 to accomodate the one extra 
                // element added before spliting
                keys(limit, INT_MAX), // maxKeys = maxChildren - 1
                children(limit + 1, nullptr) {}

bool BNode::isLeaf() { return leaf; }

bool BNode::canDonate() { return numKeys > (minChildren - 1); }

BNode* BNode::leftSibling() {
    if(not parent) return nullptr;
    int index = find(parent->children.begin(), parent->children.end(), this) - parent->children.begin();
    if(index == 0) return nullptr;
    else return parent->children[index - 1];
}

BNode* BNode::rightSibling() {
    if(not parent) return nullptr;
    int index = find(parent->children.begin(), parent->children.end(), this) - parent->children.begin();
    if(index == parent->children.size() - 1) return nullptr;
    else return parent->children[index + 1];
}

int BNode::donateGreatestKey() {
    int donatedKey = keys[numKeys - 1];
    keys[numKeys - 1] = INT_MAX;
    numKeys--;
    return donatedKey;
}

BNode* BNode::donateGreatestChild() {
    BNode* donatedChild = children[numChildren - 1];
    children[numChildren - 1] = nullptr;
    numChildren--;
    return donatedChild;
}

int BNode::donateLeastKey() {
    int donatedKey = keys[0];
    keys.erase(keys.begin());
    keys.push_back(INT_MAX);
    numKeys--;
    return donatedKey;
}

BNode* BNode::donateLeastChild() {
    BNode* donatedChild = children[0];
    children.erase(children.begin());
    children.push_back(nullptr);
    numChildren--;
    return donatedChild;
}

int BNode::getNextKey(int donatedKey) {
    int index = find(keys.begin(), keys.end(), donatedKey) - keys.begin();
    int nextKey = keys[index + 1];
    keys.erase(keys.begin() + index + 1);
    numKeys--;
    return nextKey;
}

int BNode::getPrevKey(int donatedKey) {
    int index = find(keys.begin(), keys.end(), donatedKey) - keys.begin();
    int prevKey = keys[index - 1];
    keys.erase(keys.begin() + index - 1);
    numKeys--;
    return prevKey;
}

void BNode::replaceDonatedKey(BNode* node) {
    int index = find(children.begin(), children.end(), node) - children.begin();
    if(index >= children.size()) return;
    keys[index] = leftMostKey(children[index+1]);
}

int BNode::leftMostKey(BNode* node) {
    if(node->isLeaf()) {
        return node->keys[0];
    } else {
        return leftMostKey(node->children[0]);
    }
}

// Merge nodes
// Transfer one node to parent
// Pull down one node from parent
// Transfer keys and children to a single node
BNode* BNode::merge(BNode* rightNode) {
    auto currIndex = find(parent->children.begin(), parent->children.end(), this) - parent->children.begin();
    int parentKey = parent->keys[currIndex];
    parent->keys.erase(parent->keys.begin() + currIndex);
    parent->keys.push_back(INT_MAX);
    parent->numKeys--;

    auto itrNode = find(parent->children.begin(), parent->children.end(), rightNode);
    parent->children.erase(itrNode);
    parent->children.push_back(nullptr);
    parent->numChildren--;

    addKey(parentKey);
    for(int i = numKeys; i < numKeys + rightNode->numKeys; i++) {
        keys[i] = rightNode->keys[i - numKeys];
    }
    numKeys += rightNode->numKeys;

    for(int i = numChildren; i<numChildren + rightNode->numChildren; i++) {
        children[i] = rightNode->children[i - numChildren];
        if(children[i]) children[i]->parent = this;
    }
    numChildren += rightNode->numChildren;

    if(parent->numKeys == 0) {
        parent = nullptr;
        return this;
    } else {
        BNode* returnRoot = parent;
        while(returnRoot->parent) returnRoot = returnRoot->parent;
        return returnRoot;
    }
}

// Adds the key into its correct position in sorted order
void BNode::addKey(int key) {
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
void BNode::addChild(BNode* child, int key, int offset /* = 0 */) {
    int index = find(keys.begin(), keys.end(), key) - keys.begin() + offset;
    for(int i = children.size() - 1; i>index; i--) {
        children[i] = children[i-1];
    }
    children[index] = child;
    numChildren++;
}

// Splits the node into two if the bound is exceeded
// Returns root
// Recursively reaches the root even if nodes along the way
// are valid...just to get the new root incase the root changes
BNode* BNode::splitIfNeeded() {

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
    BNode* leftNode = new BNode(maxChildren);
    leftNode->leaf = leaf;

    int startI = median + 1;

    // Copy the elements of leftnode
    for(int i = 0; i<=median; i++) {
        if(i != median) leftNode->keys[i] = keys[i];
        leftNode->children[i] = children[i];
        if(leftNode->children[i])
            leftNode->children[i]->parent = leftNode; // Update parent pointer
    }

    // Shift the elements of right node to the front
    for(int i = startI; i < keys.size(); i++) {
        keys[i - startI] = keys[i];
    }

    for(int i = startI; i < children.size(); i++) {
        children[i - startI] = children[i];
    }

    // Update the attributes of leftnode and rightnode (current node)
    leftNode->numKeys = median;
    numKeys = keys.size() - (startI);

    leftNode->numChildren = median + 1;
    numChildren = children.size() - (startI);

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
        BNode* newParent = new BNode(maxChildren);
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
        leftNode->parent = parent;
        return parent->splitIfNeeded();
    }
}

BTree::BTree(int limit): minChildren(ceil(limit/2.0)), maxChildren(limit), root(nullptr) {}

bool BTree::search(int key) {
    BNode* node = searchNode(key, root);
    if(not node) return false;

    for(auto k: node->keys) if(k == key) return true;
    return false;
}

BNode* BTree::searchNode(int key, BNode* node) {
    if(not node) return nullptr;
    if(node->isLeaf()) return node;

    for(int i = 0; i<node->keys.size(); i++) {
        if(key < node->keys[i]) return searchNode(key, node->children[i]);
        else if(key == node->keys[i]) return node;
    }
    return searchNode(key, node->children[node->numChildren - 1]);
}

bool BTree::insertKey(int key) {
    if(not root) {
        root = new BNode(maxChildren);
        root->addKey(key);
        root->leaf = true;
        return true;
    }
    BNode* node = searchNode(key, root);
    auto itr = find(node->keys.begin(), node->keys.end(), key);
    if(itr != node->keys.end()) {
        cout<<"Key already present"<<endl;
        return false;
    }
    node->addKey(key);
    root = node->splitIfNeeded();
    return true;
}

bool BTree::deleteKey(int key, BNode* rootNode) {
    // Delete key only from leaves
    // Key can remain in the internal nodes
    BNode* node = searchNode(key, rootNode);
    if(not node) {
        cout<<"Node not found: "<<key<<endl;
        return false;
    }
    auto itr = find(node->keys.begin(), node->keys.end(), key);
    if(itr == node->keys.end()) {
        cout<<"Node not found: "<<key<<endl;
        return false;
    }

    int index = itr - node->keys.begin();
    if(node->isLeaf()) {
        for(int i = index; i < node->keys.size() - 1; i++) {
            node->keys[i] = node->keys[i + 1];
        }
        node->keys.back() = INT_MAX;
        node->numKeys--;
        manageUnderflow(node);
    } else {
        int rightChildIndex = find(node->keys.begin(), node->keys.end(), key) - node->keys.begin() + 1;
        int inorderSuccessor = node->leftMostKey(node->children[rightChildIndex]);
        node->keys[index] = inorderSuccessor;
        deleteKey(inorderSuccessor, node->children[rightChildIndex]);
    }
    return true;
}

void BTree::manageUnderflow(BNode* node) {
    if(not node) return;

    if(node->numKeys >= minChildren - 1) return;

    if(node == root) {
        if(node->numKeys == 0) root = nullptr;
        return;
    };

    // Try borrowing from leftSibling
    BNode* leftSibling = node->leftSibling();
    if(leftSibling && leftSibling->canDonate()) {
        int donatedKey = leftSibling->donateGreatestKey();
        BNode* donatedChild = leftSibling->donateGreatestChild();
        node->parent->addKey(donatedKey);
        int nextKey = node->parent->getNextKey(donatedKey);
        node->addKey(nextKey);
        node->addChild(donatedChild, nextKey);
        if(donatedChild) donatedChild->parent = node;
        return;
    }

    // Try borrowing from rightSibling
    BNode* rightSibling = node->rightSibling();
    if(rightSibling && rightSibling->canDonate()) {
        int donatedKey = rightSibling->donateLeastKey();
        BNode* donatedChild = rightSibling->donateLeastChild();
        node->parent->addKey(donatedKey);
        int prevKey = node->parent->getPrevKey(donatedKey);
        node->addKey(prevKey);
        node->addChild(donatedChild, prevKey, 1);
        if(donatedChild) donatedChild->parent = node;
        return;
    }

    // Try merge with leftSibling
    if(leftSibling) {
        root = leftSibling->merge(node);
        return;
    }

    // Try merge with rightSibling
    if(rightSibling) {
        root = node->merge(rightSibling);
        return;
    }

    manageUnderflow(node->parent);
}

void BTree::inorderTraversal() {
    if(not root) {
        cout<<"Tree is empty"<<endl;
        return;
    }
    inorderTraversal(root);
}

void BTree::inorderTraversal(BNode* node) {
    if(not node) return;

    for(int i = 0; i<node->keys.size(); i++) {
        inorderTraversal(node->children[i]);
        // Can stop at the first INT_MAX encountered
        if(node->keys[i] == INT_MAX) break;
        cout<<node->keys[i]<<" ";
    }
    if(node->keys.back() != INT_MAX) inorderTraversal(node->children.back());
}

void BTree::levelOrder() {
    if(not root) {
        cout<<"Tree is empty"<<endl;
        return;
    }

    int colorIndex = 0;

    int prevDepth = 0;
    BNode* prevParent = nullptr;
    queue<pair<BNode*, int>> q;
    q.push({root, 0});
    while(!q.empty()) {
        BNode* node = q.front().first;
        int depth = q.front().second;
        q.pop();
        if(depth != prevDepth) {
            cout<<endl;
            prevDepth =depth;
        }

        // Just to be able to group the children of nodes
        if(node->parent != prevParent || node == root) {
            prevParent = node->parent;
            changeColor(COLORS[colorIndex]);
            colorIndex = (colorIndex + 1)%COLORS.size();
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
    changeColor(WHITE);
    cout<<endl;
}