/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     struct TreeNode *left;
 *     struct TreeNode *right;
 * };
 */
/**
 * Note: The returned array must be malloced, assume caller calls free().
 */
int* inorderTraversal(struct TreeNode* root, int* returnSize) {
    int* result = (int*)malloc(100 * sizeof(int));
    struct TreeNode* stack[100];
    int top = -1;
    *returnSize = 0;
    while (root || top != -1) {
        while (root) {
            stack[++top] = root;
            root = root->left;
        }
        root = stack[top--];
        result[(*returnSize)++] = root->val;
        root = root->right;
    }
    return result;
}