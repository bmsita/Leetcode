
typedef enum { RED, BLACK } Color;

typedef struct RBTNode {
    int val;
    Color color;
    int size;  
    struct RBTNode *left, *right, *parent;
} RBTNode;

typedef struct {
    RBTNode* root;
} MedianFinder;


int nodeSize(RBTNode* node) {
    return node ? node->size : 0;
}


RBTNode* newNode(int val) {
    RBTNode* node = (RBTNode*)malloc(sizeof(RBTNode));
    node->val = val;
    node->color = RED;
    node->size = 1;
    node->left = node->right = node->parent = NULL;
    return node;
}


RBTNode* leftRotate(RBTNode* root, RBTNode* x) {
    RBTNode* y = x->right;
    x->right = y->left;
    if(y->left) y->left->parent = x;
    y->parent = x->parent;
    if(!x->parent) root = y;
    else if(x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;

    x->size = 1 + nodeSize(x->left) + nodeSize(x->right);
    y->size = 1 + nodeSize(y->left) + nodeSize(y->right);

    return root;
}


RBTNode* rightRotate(RBTNode* root, RBTNode* y) {
    RBTNode* x = y->left;
    y->left = x->right;
    if(x->right) x->right->parent = y;
    x->parent = y->parent;
    if(!y->parent) root = x;
    else if(y == y->parent->left) y->parent->left = x;
    else y->parent->right = x;
    x->right = y;
    y->parent = x;

    y->size = 1 + nodeSize(y->left) + nodeSize(y->right);
    x->size = 1 + nodeSize(x->left) + nodeSize(x->right);

    return root;
}


RBTNode* insertFixup(RBTNode* root, RBTNode* z) {
    while(z->parent && z->parent->color == RED) {
        RBTNode* gp = z->parent->parent;
        if(z->parent == gp->left) {
            RBTNode* y = gp->right;
            if(y && y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                gp->color = RED;
                z = gp;
            } else {
                if(z == z->parent->right) {
                    z = z->parent;
                    root = leftRotate(root, z);
                }
                z->parent->color = BLACK;
                gp->color = RED;
                root = rightRotate(root, gp);
            }
        } else {
            RBTNode* y = gp->left;
            if(y && y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                gp->color = RED;
                z = gp;
            } else {
                if(z == z->parent->left) {
                    z = z->parent;
                    root = rightRotate(root, z);
                }
                z->parent->color = BLACK;
                gp->color = RED;
                root = leftRotate(root, gp);
            }
        }
    }
    root->color = BLACK;
    return root;
}


RBTNode* insertRBT(RBTNode* root, int val) {
    RBTNode* z = newNode(val);
    RBTNode* y = NULL;
    RBTNode* x = root;

    while(x) {
        x->size++;  
        y = x;
        if(val < x->val) x = x->left;
        else x = x->right;
    }
    z->parent = y;
    if(!y) root = z;
    else if(val < y->val) y->left = z;
    else y->right = z;

    root = insertFixup(root, z);
    return root;
}


int kthSmallest(RBTNode* node, int k) {
    int leftSize = nodeSize(node->left);
    if(k == leftSize + 1) return node->val;
    if(k <= leftSize) return kthSmallest(node->left, k);
    return kthSmallest(node->right, k - leftSize - 1);
}


MedianFinder* medianFinderCreate() {
    MedianFinder* mf = (MedianFinder*)malloc(sizeof(MedianFinder));
    mf->root = NULL;
    return mf;
}


void medianFinderAddNum(MedianFinder* obj, int num) {
    obj->root = insertRBT(obj->root, num);
}


double medianFinderFindMedian(MedianFinder* obj) {
    int n = nodeSize(obj->root);
    if(n % 2 == 1) return (double)kthSmallest(obj->root, n/2 + 1);
    int a = kthSmallest(obj->root, n/2);
    int b = kthSmallest(obj->root, n/2 + 1);
    return (a + b) / 2.0;
}


void freeRBT(RBTNode* node) {
    if(!node) return;
    freeRBT(node->left);
    freeRBT(node->right);
    free(node);
}


void medianFinderFree(MedianFinder* obj) {
    freeRBT(obj->root);
    free(obj);
}
