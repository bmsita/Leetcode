void inorder(struct TreeNode* root, struct TreeNode** arr, int* index) {
    if (!root) return;
    inorder(root->left, arr, index);
    arr[*index] = root;
    (*index)++;
    inorder(root->right, arr, index);
}

struct TreeNode* buildBalanced(struct TreeNode** arr, int start, int end) {
    if (start > end) return NULL;
    int mid = start + (end - start) / 2;
    struct TreeNode* root = arr[mid];
    root->left = buildBalanced(arr, start, mid - 1);
    root->right = buildBalanced(arr, mid + 1, end);
    return root;
}

struct TreeNode* balanceBST(struct TreeNode* root) {
    struct TreeNode* arr[10000];
    int index = 0;
    inorder(root, arr, &index);
    return buildBalanced(arr, 0, index - 1);
}
