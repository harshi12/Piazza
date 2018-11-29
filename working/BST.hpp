// C++ program to find predecessor and successor in a BST
#include <iostream>
#include <limits.h>
#include <vector>
using namespace std;
vector<string> slaves;
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



Node* minValue(Node* node) { 
    Node* current = node; 
    
    /* loop down to find the leftmost leaf */


    while (current->left != NULL) { 
        current = current->left; 
    } 
    return current; 
} 
  

Node* maxValue(Node* node) { 
    Node* current = node; 
    
    /* loop down to find the leftmost leaf */
    while (current->right != NULL) { 
        current = current->right; 
    } 
    return current; 
} 
// A utility function to create a newroot->max BST node 
Node *newNode(int id,string ipport ) 
{   
    Node *temp =  new Node; 

    temp->key = id;
    temp->ipport = ipport;
    temp->left = temp->right = NULL;
    temp->min = id;
    temp->max = id;
    return temp;
}

/* A utility function to insert a new node with given key in BST */
Node *insert(Node *node, int key, string ipport)
{
    // cout << "in insert ipport: " << ipport << "\n";
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


/* Given a non-empty binary search tree, return the node with minimum 
   key value found in that tree. Note that the entire tree does not 
   need to be searched. */
Node *minValueNode(Node *node)
{
    Node *current = node;

    /* loop down to find the leftmost leaf */
    while (current->left != NULL)
        current = current->left;

    return current;
}

/* Given a binary search tree and a key, this function deletes the key 
   and returns the new root */
Node *deleteNode(Node *root, int key)
{
    // base case
    if (root == NULL)
        return root;

    // If the key to be deleted is smaller than the root's key,
    // then it lies in left subtree
    if (key < root->key)
        root->left = deleteNode(root->left, key);

    // If the key to be deleted is greater than the root's key,
    // then it lies in right subtree
    else if (key > root->key)
        root->right = deleteNode(root->right, key);

    // if key is same as root's key, then This is the node
    // to be deleted
    else
    {
        // node with only one child or no child
        if (root->left == NULL)
        {
            Node *temp = root->right;
            free(root);
            return temp;
        }
        else if (root->right == NULL)
        {
            Node *temp = root->left;
            free(root);
            return temp;
        }

        // node with two children: Get the inorder successor (smallest
        // in the right subtree)
        Node *temp = minValueNode(root->right);

        // Copy the inorder successor's content to this node
        root->key = temp->key;

        // Delete the inorder successor
        root->right = deleteNode(root->right, temp->key);
    }
    return root;
}


void preorder(Node *root, int suc)
{

    if (root->left == NULL)
    {
        // cout<<"value of pre"<<root->key<<endl;
        suc = root->key;
        return;
    }
    else if (root->left)
    {
        // cout<<root->left->key<<endl;
        return preorder(root->left, suc);
    }
}
void postorder(Node *root, Node **pre)
{
    if (root->right == NULL)
    {
        // cout<<"value of pre"<<root->key<<endl;
        *pre = root;
        return;
    }
    else if (root->right)
    {
        // cout<<root->right->key<<endl;
        return postorder(root->right, pre);
    }
}

void inorder(Node *root)
{
    if(root==NULL){
        
        return;
    }
    else{

        inorder(root->left);
        cout << root->key << "/" << root->ipport << ", ";
        inorder(root->right);
    }
}