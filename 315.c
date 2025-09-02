struct Node {
    int val;
    int count;
    struct Node *left;
    struct Node *right;
};

struct Node* insert(struct Node* root, int val, int* smallerCount, int count) {
    if (!root) {
        struct Node* node = (struct Node*)malloc(sizeof(struct Node));
        node->val = val;
        node->count = 0;
        node->left = node->right = NULL;
        *smallerCount = count;
        return node;
    }
    if (val <= root->val) {
        root->count++;
        root->left = insert(root->left, val, smallerCount, count);
    } else {
        root->right = insert(root->right, val, smallerCount, count + root->count + 1);
    }
    return root;
}

int* countSmaller(int* nums, int numsSize, int* returnSize) {
    *returnSize = numsSize;
    int* res = (int*)malloc(sizeof(int) * numsSize);
    struct Node* root = NULL;
    for (int i = numsSize - 1; i >= 0; i--) {
        int count = 0;
        root = insert(root, nums[i], &count, 0);
        res[i] = count;
    }
    return res;
}
