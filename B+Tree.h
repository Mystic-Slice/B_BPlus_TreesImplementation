#ifndef UTILS
#define UTILS
#include "utils.cpp"
#endif

class BPlusNode {
    public:
        bool leaf; // Node is a leaf or not
        BPlusNode* parent;
        BPlusNode* next; // Next node in the linked list (only found in leaves)
        int minChildren, maxChildren, numChildren, numKeys;
        vector<int> keys;
        vector<BPlusNode*> children;

        BPlusNode(int limit);

        bool isLeaf();

        bool canDonate();

        BPlusNode* leftSibling();

        BPlusNode* rightSibling();

        int donateGreatestKey();

        BPlusNode* donateGreatestChild();

        int donateLeastKey();

        BPlusNode* donateLeastChild();

        int getNextKey(int donatedKey);

        int getPrevKey(int donatedKey);
        void replaceDonatedKey(BPlusNode* node);

        int leftMostKey(BPlusNode* node);

        // Merge leaves
        // Just add the keys to a single node and delete a key from parent
        // No need to care about children here
        BPlusNode* mergeLeaves(BPlusNode* rightNode);

        // Merge internal nodes
        // Transfer one node to parent
        // Pull down one node from parent
        // Transfer keys and children to a single node
        BPlusNode* mergeInternal(BPlusNode* rightNode);

        // Adds the key into its correct position in sorted order
        void addKey(int key);

        // Adds the child to the same corresponding index as the key
        void addChild(BPlusNode* child, int key, int offset = 0);

        // Update the previous left neighbour's next pointer to the newly added child node
        void updateNext(int key);

        // Incase the child that was added is the leftmost node of the subtree
        // Its left neighbour has to found by DFS
        void updateByDFS(BPlusNode* child);

        // Splits the node into two if the bound is exceeded
        // Returns root
        // Recursively reaches the root even if nodes along the way
        // are valid...just to get the new root incase the root changes
        BPlusNode* splitIfNeeded();
};

class BPlusTree {
    public:
        BPlusNode* root;
        int minChildren, maxChildren;

        BPlusTree(int limit);

        bool search(int key);

        BPlusNode* searchNode(int key, BPlusNode* node);

        bool insertKey(int key);

        bool deleteKey(int key);

        void manageUnderflowLeaf(BPlusNode* node, int key);

        void manageUnderflowParent(BPlusNode* node);

        void inorderTraversal();

        void inorderTraversal(BPlusNode* node);

        void traverseLeaves();

        void levelOrder();
};