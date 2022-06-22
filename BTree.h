#ifndef UTILS
#define UTILS
#include "utils.cpp"
#endif

class BNode {
    public:
        bool leaf; // Node is a leaf or not
        BNode* parent;
        int minChildren, maxChildren, numChildren, numKeys;
        vector<int> keys;
        vector<BNode*> children;

        BNode(int limit);

        bool isLeaf();

        bool canDonate();

        BNode* leftSibling();

        BNode* rightSibling();

        int donateGreatestKey();

        BNode* donateGreatestChild();

        int donateLeastKey();

        BNode* donateLeastChild();

        int getNextKey(int donatedKey);

        int getPrevKey(int donatedKey);

        void replaceDonatedKey(BNode* node);

        int leftMostKey(BNode* node);

        // Merge nodes
        // Transfer one node to parent
        // Pull down one node from parent
        // Transfer keys and children to a single node
        BNode* merge(BNode* rightNode);

        // Adds the key into its correct position in sorted order
        void addKey(int key);

        // Adds the child to the same corresponding index as the key
        void addChild(BNode* child, int key, int offset = 0);

        // Splits the node into two if the bound is exceeded
        // Returns root
        // Recursively reaches the root even if nodes along the way
        // are valid...just to get the new root incase the root changes
        BNode* splitIfNeeded();
};

class BTree {
    public:
        BNode* root;
        int minChildren, maxChildren;

        BTree(int limit);

        bool search(int key);

        BNode* searchNode(int key, BNode* node);

        bool insertKey(int key);

        bool deleteKey(int key, BNode* rootNode);

        void manageUnderflow(BNode* node);

        void inorderTraversal();

        void inorderTraversal(BNode* node);

        void levelOrder();
};