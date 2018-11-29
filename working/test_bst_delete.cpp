
// C++ program to find predecessor and successor in a BST
#include <iostream>
#include <limits.h>
#include "BST.hpp"
using namespace std;

// Driver program to test above function
int main()
{
    int key = 20; //Key to be searched in BST

    Node *root = NULL;

    root = insert(root, 50, "127.0.0.1:1580");

    insert(root, 30, "127.0.0.1:1580");
    insert(root, 20, "127.0.0.1:1580");
    insert(root, 40, "127.0.0.1:1580");
    insert(root, 70, "127.0.0.1:1580");
    insert(root, 60, "127.0.0.1:1580");
    insert(root, 80, "127.0.0.1:1580");

    root = deleteNode(root, 20);
    root = deleteNode(root, 70);

    inorder(root);
    return 0;
}
