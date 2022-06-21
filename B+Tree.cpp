#include "B+Tree.h"

BPlusNode::BPlusNode(int limit): leaf(false), 
                parent(nullptr),
                next(nullptr), 
                minChildren(ceil(limit/2.0)), 
                maxChildren(limit),
                numChildren(0), 
                numKeys(0),
                // Initialize vectors filled with INT_MAX/nullptr to 
                // maxsize + 1 to accomodate the one extra 
                // element added before spliting
                keys(limit, INT_MAX), // maxKeys = maxChildren - 1
                children(limit + 1, nullptr) {}

bool BPlusNode::isLeaf() { return leaf; }

bool BPlusNode::canDonate() { return numKeys > (minChildren - 1); }

BPlusNode* BPlusNode::leftSibling() {
    if(not parent) return nullptr;
    int index = find(parent->children.begin(), parent->children.end(), this) - parent->children.begin();
    if(index == 0) return nullptr;
    else return parent->children[index - 1];
}

BPlusNode* BPlusNode::rightSibling() {
    if(not parent) return nullptr;
    int index = find(parent->children.begin(), parent->children.end(), this) - parent->children.begin();
    if(index == parent->children.size() - 1) return nullptr;
    else return parent->children[index + 1];
}

int BPlusNode::donateGreatestKey() {
    int donatedKey = keys[numKeys - 1];
    keys[numKeys - 1] = INT_MAX;
    numKeys--;
    return donatedKey;
}

BPlusNode* BPlusNode::donateGreatestChild() {
    BPlusNode* donatedChild = children[numChildren - 1];
    children[numChildren - 1] = nullptr;
    numChildren--;
    return donatedChild;
}

int BPlusNode::donateLeastKey() {
    int donatedKey = keys[0];
    keys.erase(keys.begin());
    keys.push_back(INT_MAX);
    numKeys--;
    return donatedKey;
}

BPlusNode* BPlusNode::donateLeastChild() {
    BPlusNode* donatedChild = children[0];
    children.erase(children.begin());
    children.push_back(nullptr);
    numChildren--;
    return donatedChild;
}

int BPlusNode::getNextKey(int donatedKey) {
    int index = find(keys.begin(), keys.end(), donatedKey) - keys.begin();
    int nextKey = keys[index + 1];
    keys.erase(keys.begin() + index + 1);
    numKeys--;
    return nextKey;
}

int BPlusNode::getPrevKey(int donatedKey) {
    int index = find(keys.begin(), keys.end(), donatedKey) - keys.begin();
    int prevKey = keys[index - 1];
    keys.erase(keys.begin() + index - 1);
    numKeys--;
    return prevKey;
}

void BPlusNode::replaceDonatedKey(BPlusNode* node) {
    int index = find(children.begin(), children.end(), node) - children.begin();
    if(index >= children.size()) return;
    keys[index] = leftMostKey(children[index+1]);
}

int BPlusNode::leftMostKey(BPlusNode* node) {
    if(node->isLeaf()) {
        return node->keys[0];
    } else {
        return leftMostKey(node->children[0]);
    }
}

// Merge leaves
// Just add the keys to a single node and delete a key from parent
// No need to care about children here
BPlusNode* BPlusNode::mergeLeaves(BPlusNode* rightNode) {

    auto currIndex = find(parent->children.begin(), parent->children.end(), this) - parent->children.begin();
    parent->keys.erase(parent->keys.begin() + currIndex);
    parent->keys.push_back(INT_MAX);
    parent->numKeys--;
    
    auto itrNode = find(parent->children.begin(), parent->children.end(), rightNode);
    parent->children.erase(itrNode);
    parent->children.push_back(nullptr);
    parent->numChildren--;

    
    for(int i = numKeys; i < numKeys + rightNode->numKeys; i++) {
        keys[i] = rightNode->keys[i - numKeys];
    }
    numKeys += rightNode->numKeys;

    next = rightNode->next;

    if(parent->numKeys == 0) {
        parent = nullptr;
        return this;
    } else {
        BPlusNode* returnRoot = parent;
        while(returnRoot->parent) returnRoot = returnRoot->parent;
        return returnRoot;
    }
}

// Merge internal nodes
// Transfer one node to parent
// Pull down one node from parent
// Transfer keys and children to a single node
BPlusNode* BPlusNode::mergeInternal(BPlusNode* rightNode) {
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
        children[i]->parent = this;
    }
    numChildren += rightNode->numChildren;

    next = rightNode->next;

    if(parent->numKeys == 0) {
        parent = nullptr;
        return this;
    } else {
        BPlusNode* returnRoot = parent;
        while(returnRoot->parent) returnRoot = returnRoot->parent;
        return returnRoot;
    }
}

// Adds the key into its correct position in sorted order
void BPlusNode::addKey(int key) {
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
void BPlusNode::addChild(BPlusNode* child, int key, int offset /* = 0 */) {
    int index = find(keys.begin(), keys.end(), key) - keys.begin() + offset;
    for(int i = children.size() - 1; i>index; i--) {
        children[i] = children[i-1];
    }
    children[index] = child;
    numChildren++;
}

// Update the previous left neighbour's next pointer to the newly added child node
void BPlusNode::updateNext(int key) {
    int index = find(keys.begin(), keys.end(), key) - keys.begin();
    if(index == 0) return updateByDFS(children[0]);
    // if(not index) return;
    children[index-1]->next = children[index];
}

// Incase the child that was added is the leftmost node of the subtree
// Its left neighbour has to found by DFS
void BPlusNode::updateByDFS(BPlusNode* child) {
    BPlusNode* root = this;
    while(root->parent) root = root->parent;

    BPlusNode* prevLeaf = nullptr;
    stack<BPlusNode*> st;
    st.push(root);
    while(!st.empty()) {
        BPlusNode* node = st.top(); st.pop();
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
BPlusNode* BPlusNode::splitIfNeeded() {

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
    BPlusNode* leftNode = new BPlusNode(maxChildren);
    leftNode->leaf = leaf;
    leftNode->next = this;

    // Retain the median element in the right node only for leaves
    // This is taken care by the value of startI            
    int startI = median + 1;
    if(leaf) startI--;

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
        BPlusNode* newParent = new BPlusNode(maxChildren);
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

BPlusTree::BPlusTree(int limit): minChildren(ceil(limit/2.0)), maxChildren(limit), root(nullptr) {}

bool BPlusTree::search(int key) {
    BPlusNode* node = searchNode(key, root);
    if(not node) return false;

    for(auto k: node->keys) if(k == key) return true;
    return false;
}

BPlusNode* BPlusTree::searchNode(int key, BPlusNode* node) {
    if(not node) return nullptr;
    if(node->isLeaf()) return node;

    for(int i = 0; i<node->keys.size(); i++) {
        if(key < node->keys[i]) return searchNode(key, node->children[i]);
    }
    return searchNode(key, node->children[node->numChildren - 1]);
}

void BPlusTree::insertKey(int key) {
    if(not root) {
        root = new BPlusNode(maxChildren);
        root->addKey(key);
        root->leaf = true;
        return;
    }
    BPlusNode* node = searchNode(key, root);
    node->addKey(key);
    root = node->splitIfNeeded();
}

void BPlusTree::deleteKey(int key) {
    // Delete key only from leaves
    // Key can remain in the internal nodes
    BPlusNode* node = searchNode(key, root);
    auto itr = find(node->keys.begin(), node->keys.end(), key);
    if(itr == node->keys.end()) {
        cout<<"Node not found: "<<key<<endl;
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

void BPlusTree::manageUnderflowLeaf(BPlusNode* node, int key) {
    if(node->numKeys >= minChildren - 1) return;
    if(node == root) {
        if(node->numKeys == 0) root = nullptr;
        return;
    };

    // Try borrowing from leftSibling
    BPlusNode* leftSibling = node->leftSibling();
    if(leftSibling && leftSibling->canDonate()) {
        int donatedKey = leftSibling->donateGreatestKey();
        node->addKey(donatedKey);
        node->parent->replaceDonatedKey(leftSibling);
        return;
    }

    // Try borrowing from rightSibling
    BPlusNode* rightSibling = node->rightSibling();
    if(rightSibling && rightSibling->canDonate()) {
        int donatedKey = rightSibling->donateLeastKey();
        node->addKey(donatedKey);
        node->parent->replaceDonatedKey(node);
        return;
    }

    // Try merge with leftSibling
    if(leftSibling) {
        root = leftSibling->mergeLeaves(node);
        return;
    }

    // Try merge with rightSibling
    if(rightSibling) {
        root = node->mergeLeaves(rightSibling);
        return;
    }
}

void BPlusTree::manageUnderflowParent(BPlusNode* node) {
    if(not node) return;

    if(node->numKeys >= minChildren - 1) return;

    if(node == root) {
        if(node->numKeys == 0) root = nullptr;
        return;
    };

    // Try borrowing from leftSibling
    BPlusNode* leftSibling = node->leftSibling();
    if(leftSibling && leftSibling->canDonate()) {
        int donatedKey = leftSibling->donateGreatestKey();
        BPlusNode* donatedChild = leftSibling->donateGreatestChild();
        node->parent->addKey(donatedKey);
        int nextKey = node->parent->getNextKey(donatedKey);
        node->addKey(nextKey);
        node->addChild(donatedChild, nextKey);
        donatedChild->parent = node;
        return;
    }

    // Try borrowing from rightSibling
    BPlusNode* rightSibling = node->rightSibling();
    if(rightSibling && rightSibling->canDonate()) {
        int donatedKey = rightSibling->donateLeastKey();
        BPlusNode* donatedChild = rightSibling->donateLeastChild();
        node->parent->addKey(donatedKey);
        int prevKey = node->parent->getPrevKey(donatedKey);
        node->addKey(prevKey);
        node->addChild(donatedChild, prevKey, 1);
        donatedChild->parent = node;
        return;
    }

    // Try merge with leftSibling
    if(leftSibling) {
        root = leftSibling->mergeInternal(node);
        return;
    }

    // Try merge with rightSibling
    if(rightSibling) {
        root = node->mergeInternal(rightSibling);
        return;
    }

    manageUnderflowParent(node->parent);
}

void BPlusTree::inorderTraversal() {
    if(not root) {
        cout<<"Tree is empty"<<endl;
        return;
    }
    inorderTraversal(root);
}

void BPlusTree::inorderTraversal(BPlusNode* node) {
    if(not node) return;

    for(int i = 0; i<node->keys.size(); i++) {
        inorderTraversal(node->children[i]);
        // Can stop at the first INT_MAX encountered
        if(node->keys[i] == INT_MAX) break;
        cout<<node->keys[i]<<" ";
    }
    if(node->keys.back() != INT_MAX) inorderTraversal(node->children.back());
}

void BPlusTree::traverseLeaves() {
    if(not root) {
        cout<<"Tree is empty"<<endl;
        return;
    }

    // Find the leftmost node
    BPlusNode* node = root;
    while(not node->isLeaf()) node = node->children[0];

    // Use the `next` pointer to traverse through the elements
    while(node){
        cout<<node->keys;
        node = node->next;
    }
    cout<<endl;
}

void BPlusTree::levelOrder() {
    if(not root) {
        cout<<"Tree is empty"<<endl;
        return;
    }

    int colorIndex = 0;

    int prevDepth = 0;
    BPlusNode* prevParent = nullptr;
    queue<pair<BPlusNode*, int>> q;
    q.push({root, 0});
    while(!q.empty()) {
        BPlusNode* node = q.front().first;
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