
// C++ program to find predecessor and successor in a BST 
#include <iostream> 
#include <limits.h>
using namespace std; 
  
// BST Node 
struct Node 
{ 
    int key;
    string ipport;
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
Node *newNode(int id,string ipport ) 
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
Node* insert(Node* node, int key, string ipport) 
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
  
// Driver program to test above function 
int main() 
{ 
    int key = 20;    //Key to be searched in BST 

    Node *root = NULL; 
    
    root = insert(root, 50 ,"127.0.0.1:1580"); 
    
    insert(root, 30,"127.0.0.1:1580"); 
    insert(root, 20,"127.0.0.1:1580"); 
    insert(root, 40,"127.0.0.1:1580"); 
    insert(root, 70,"127.0.0.1:1580"); 
    insert(root, 60,"127.0.0.1:1580"); 
    insert(root, 80,"127.0.0.1:1580"); 
  
  
    Node* pre = NULL, *suc = NULL; 
  
    findPreSuc(root, pre, suc, key); 


    if (pre != NULL) 
      cout << "Predecessor is " << pre->key << endl; 
 	else{
      //if no predecessor.........
      postorder(root,&pre);
      cout<< "Predecessor is "<< pre->key<<endl;
    }

    if (suc != NULL) 
      cout << "Successor is " << suc->key<<endl; 
 	else{
 	  //if no successor............	
      preorder(root,&suc);
      cout<< "Successor is "<< suc->key<<endl;
    }
    return 0; 
} 
