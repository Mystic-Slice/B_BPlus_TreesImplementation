#include <bits/stdc++.h>
using namespace std;

template <typename T>
ostream& operator<<(ostream& out, const vector<T>& v) {
    for (auto& element : v){
        if(element == INT_MAX) continue;
        out << element << " ";
    }
    return out;
}

class Node {
	public:
		bool leaf;
		Node* parent;
        Node* next;
		int minChildren, maxChildren, numChildren, numKeys;
		vector<int> keys;
		vector<Node*> children;

		Node(int limit): minChildren(limit), 
                            maxChildren(limit*2), 
                            keys(limit*2, INT_MAX), 
                            children(limit*2 + 1, nullptr), 
                            numChildren(0), 
                            numKeys(0),
                            parent(nullptr),
                            leaf(false),
                            next(nullptr) {}

		bool isLeaf() { return leaf; }

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

		void addChild(Node* child, int key) {
			int index = find(keys.begin(), keys.end(), key) - keys.begin();
			for(int i = children.size() - 1; i>index; i--) {
				children[i] = children[i-1];
			}
			children[index] = child;
			numChildren++;
		}

		Node* splitIfNeeded() {
			if(numKeys <= maxChildren - 1) {
				if(parent) return parent->splitIfNeeded();
				else return this;
			}

            // Take the median to the parent node
            // Split the current node into two halves
            // Let the current node just retain the keys
            // greater than the median
            // Create a new node for left child
			int median = numKeys/2;
			int medianKey = keys[median];
			Node* leftNode = new Node(minChildren);
			leftNode->leaf = leaf;
            leftNode->next = this;

            // Copy the elements of leftnode
            // `i` goes till `median+1` because the median element
            // is retained in the leftNode
            // for b-tree i goes till median
            //// Only point of difference between b-tree and b+tree implementation :)
			for(int i = 0; i<=median+1; i++) {
				if(i != median+1) leftNode->keys[i] = keys[i];
				leftNode->children[i] = children[i];
			}

            // Shift the elements of right node to the front
			for(int i = median + 1; i < keys.size(); i++) {
				keys[i - median - 1] = keys[i];
			}

			children[median] = nullptr;
			for(int i = median + 1; i < children.size(); i++) {
				children[i - median - 1] = children[i];
			}

            // Update the attributes of leftnode and rightnode(current node)
			leftNode->numKeys = median+1;
			numKeys = keys.size() - leftNode->numKeys;

			leftNode->numChildren = median + 2;
			numChildren = children.size() - leftNode->numChildren;

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
				if(node->keys[i] == INT_MAX) break;
				cout<<node->keys[i]<<" ";
			}
			inorderTraversal(node->children.back());
		}

        void traverseLeaves() {
            if(not root) {
                cout<<"Tree is empty"<<endl;
                return;
            }

            // Find the leftmost node
            Node* node = root;
            while(not node->isLeaf()) node = node->children[0];

            while(node){
                cout<<node->keys;
                node = node->next;
            }
        }
};

int main() {
	BPlusTree tree(3);
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);
    tree.insert(9);
    tree.insert(10);
    tree.insert(11);
    tree.insert(12);
    tree.insert(4);
    tree.insert(5);
    tree.insert(6);
    tree.insert(7);
	tree.inorderTraversal();
    cout<<endl;
    tree.traverseLeaves();
}