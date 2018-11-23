// C++ program to find predecessor and successor in a BST 
#include <iostream> 
#include <limits.h>
using namespace std; 
vector<string>slaves;
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
// Node* findPreSuc(Node* root, int key) 
// { 
//     // Base case 
//     if (root == NULL)  return NULL; 
  
//     // If key is present at root 
//     if (root->key == key) 
//     { 
//         Node* tmp = root->right; 
//         // the minimum value in right subtree is successor 
//         if (root->right != NULL) 
//         { 
            
//             while (tmp->left) 
//                 tmp = tmp->left ; 
//             // suc = tmp ; 
//         } 


//         return tmp ; 
//     } 
  
//     // If key is smaller than root's key, go to left subtree 
//     if (root->key > key) 
//     { 
//         // suc = root ; 
//         return findPreSuc(root->left, key) ; 
//     } 
//     else // go to right subtree 
//     { 
//         // pre = root ; 
//         return findPreSuc(root->right, key) ; 
//     } 
// } 

Node* findPreSuc(Node* root,int key){
    if(!root)
    {
        printf("BST empty, no successor for key: %u\n", key);
        return NULL;
    }
    Node* p_succ = NULL;
    while(root)
    {
        if(root->key > key)
        {
            p_succ = root;
            root = root->left;
        }
        else
        {
            root = root->right;
        }
    }
    if(!p_succ)
        return NULL;
    else
        return p_succ;
}
Node* minValue(Node* node) { 
    Node* current = node; 
    
    /* loop down to find the leftmost leaf */
    while (current->left != NULL) { 
        current = current->left; 
    } 
    return current; 
} 
  
// void findPreSuc(Node* root, Node*& pre, Node*& suc, int key) 
// { 
//     // Base case 
//     if (root == NULL)  return ; 
  
//     // If key is present at root 
//     if (root->key == key) 
//     { 
//         // the maximum value in left subtree is predecessor 
//         if (root->left != NULL) 
//         { 
//             Node* tmp = root->left; 
//             while (tmp->right) 
//                 tmp = tmp->right; 
//             pre = tmp ; 
//         } 
  
//         // the minimum value in right subtree is successor 
//         if (root->right != NULL) 
//         { 
//             Node* tmp = root->right ; 
//             while (tmp->left) 
//                 tmp = tmp->left ; 
//             suc = tmp ; 
//         } 


//         return ; 
//     } 
  
//     // If key is smaller than root's key, go to left subtree 
//     if (root->key > key) 
//     { 
//         suc = root ; 
//         findPreSuc(root->left, pre, suc, key) ; 
//     } 
//     else // go to right subtree 
//     { 
//         pre = root ; 
//         findPreSuc(root->right, pre, suc, key) ; 
//     } 
// } 
  
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
{   cout<<"in insert ipport: "<<ipport<<"\n";
    // cout<<"dddddddddddddddddddd"
    if (node == NULL) return node = newNode(key,ipport); 
    cout<<key<<":key:"<<endl;
    cout<<node->key<<" node-> key : "<<endl;
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

Node* preorder(Node *root,int suc){

	if(root->left==NULL){
		// cout<<"value of pre"<<root->key<<endl;
		suc = root->key;
		return root;
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