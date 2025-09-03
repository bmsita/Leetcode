typedef enum {red, black} Color;

typedef struct Node{
    int value;
    Color color;
    int size;
    struct Node *left, *right, *parent;
}Node;

Node* createNode(int value){
    Node *newNode = (Node *)malloc(sizeof(Node));
    if(newNode==NULL) return NULL;

    newNode->value = value;
    newNode->size = 1;
    newNode->color = red;
    newNode->left = newNode->right = newNode->parent = NULL;

    return newNode;
}

int getSize(Node *root){
    return (root ? root->size : 0);
}

void updateSize(Node* root){
    if(root==NULL) return;
    root->size = getSize(root->left) + getSize(root->right) + 1;
}

void leftRotate(Node **root, Node *x){
    if(x==NULL || x->right==NULL) return;
    Node *y = x->right;
    x->right = y->left;
    if(y->left!=NULL) y->left->parent = x;

    if(y->left!=NULL){
        y->left->parent = x;
    }

    y->parent = x->parent;
    if(x->parent==NULL){
        *root = y;
    }
    else if(x==x->parent->left){
        x->parent->left = y;
    }
    else{
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;

    updateSize(x);
    updateSize(y);
}

void rightRotate(Node **root, Node *x){
    if(x==NULL || x->left==NULL) return;
    Node *y = x->left;
    x->left = y->right;
    if(y->right!=NULL) y->right->parent = x;

    y->parent = x->parent;
    if(x->parent==NULL){
        *root = y;
    }
    else if(x==x->parent->left){
        x->parent->left = y;
    }
    else{
        x->parent->right = y;
    }

    y->right = x;
    x->parent = y;

    updateSize(x);
    updateSize(y);
}

void fixInsert(Node **root, Node *z){
    while(z!=*root && z->parent->color == red){
        Node *parent = z->parent;
        Node *grandParent = parent->parent;
        if(grandParent && (parent == grandParent->left)){
            Node *uncle = grandParent->right;
            if(uncle && uncle->color==red){
                parent->color = black;
                uncle->color = black;
                grandParent->color = red;

                z = grandParent;
            }
            else{
                if(z == parent->right){
                    z = parent;
                    leftRotate(root, z);
                }
                else{
                    z->parent->color = black;
                    grandParent->color = red;
                    rightRotate(root, grandParent);
                }
            }
        }
        else{
            Node *uncle = grandParent?grandParent->right:NULL;
            if(uncle && uncle->color==red){
                parent->color = black;
                uncle->color = black;
                grandParent->color = red;

                z = grandParent;
            }
            else{
                if(z == parent->left){
                    z = parent;
                    rightRotate(root, z);
                }

                parent->color = black;
                if(grandParent!=NULL)
                 grandParent->color = red;
                leftRotate(root, grandParent);
            }
        }
    }
    if(*root){
        (*root)->color = black;
    }
}

Node* insertValue(Node **root, int val) {
    Node *y = NULL, *x = *root;

    // Search position
    while (x != NULL) {
        y = x;
        if (val == x->value) return *root; // skip duplicates
        if (val < x->value) x = x->left;
        else x = x->right;
    }

    Node *z = createNode(val);
    z->parent = y;

    if (y == NULL) *root = z;
    else if (val < y->value) y->left = z;
    else y->right = z;

    // Update sizes while going up
    Node *p = z;
    while (p != NULL) {
        updateSize(p);
        p = p->parent;
    }

    fixInsert(root, z);
    return *root;
}

int findKthLargest(Node *root, int k){
    if(root==NULL) return -1;

    int rootsize = root->right ? root->right->size:0;
    if(rootsize + 1 == k) return root->value;
    if(k<=rootsize){
        return findKthLargest(root->right, k);
    }
    else{
        return findKthLargest(root->left, k-rootsize-1);
    }
}
int thirdMax(int* nums, int numsSize) {
    Node *root = NULL;
    for(int i=0; i<numsSize; i++){
        insertValue(&root, nums[i]);
    }

    int totalNodes = root->size;
    if(totalNodes<3) return findKthLargest(root, 1);
    return findKthLargest(root, 3);
}
