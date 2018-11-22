// C++ program to find predecessor and successor in a BST 
#include <iostream> 
#include <limits.h>
using namespace std; 
  
// BST Node 
struct Node 
{ 
    int key;
    char *ipport;
    int min;
    int max;
    struct Node *left, *right; 
    Node(){
    	key=0;
    	ipport="";
    	min = INT_MIN;
    	max= INT_MAX;
    	left = NULL;
    	right = NULL;
    }
}; 
  
// This function finds predecessor and successor of key in BST. 
// It sets pre and suc as predecessor and successor respectively 
void findPreSuc(Node* root, Node*& pre, Node*& suc, int key) 
{ 
    // Base case 
    if (root == NULL)  return ; 
  
    // If key is present at root 
    if (root->key == key) 
    { 
        // the maximum value in left subtree is predecessor 
        if (root->left != NULL) 
        { 
            Node* tmp = root->left; 
            while (tmp->right) 
                tmp = tmp->right; 
            pre = tmp ; 
        } 
  
        // the minimum value in right subtree is successor 
        if (root->right != NULL) 
        { 
            Node* tmp = root->right ; 
            while (tmp->left) 
                tmp = tmp->left ; 
            suc = tmp ; 
        } 


        return ; 
    } 
  
    // If key is smaller than root's key, go to left subtree 
    if (root->key > key) 
    { 
        suc = root ; 
        findPreSuc(root->left, pre, suc, key) ; 
    } 
    else // go to right subtree 
    { 
        pre = root ; 
        findPreSuc(root->right, pre, suc, key) ; 
    } 
} 
  
// A utility function to create a newroot->max BST node 
Node *newNode(int id,char *ipport ) 
{ 	
    Node *temp =  new Node; 
    temp->key = id;
    temp->ipport = ipport; 
    temp->left = temp->right = NULL; 
    temp->min = id;
    temp -> max = id;
    return temp; 
} 
  
/* A utility function to insert a new node with given key in BST */
Node* insert(Node* node, int key,char *ipport) 
{ 
    if (node == NULL) return newNode(key,ipport); 
    if (key < node->key){ 
    	if(node->min > key)
    		node->min = key;
        node->left  = insert(node->left, key,ipport); 
    }
    else{
    	if(node -> max < key)
    		node->max = key;
        node->right = insert(node->right, key,ipport); 
    }
    return node; 
} 

void preorder(Node *root,Node **suc){
	if(root->left==NULL){
		// cout<<"value of pre"<<root->key<<endl;
		*suc = root;
		return;
	}
	else if(root->left){
		// cout<<root->left->key<<endl;
		return preorder(root->left,suc);
	}
}
void postorder(Node *root,Node **pre){
	if(root->right==NULL){
		// cout<<"value of pre"<<root->key<<endl;
		*pre = root;
		return;
	}
	else if(root->right){
		// cout<<root->right->key<<endl;
		return postorder(root->right,pre);
	}
}


void inorder(Node *root){
	if(root==NULL){
		
		return;
	}
    else{
        inorder(root->left);
        cout << root->key<<"/"<<root->ipport<<", ";
        inorder(root->right);
    }

}