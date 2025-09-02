/**
 * Definition for singly-linked list.
 * struct ListNode {
 *     int val;
 *     struct ListNode *next;
 * };
 */
/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     struct TreeNode *left;
 *     struct TreeNode *right;
 * };
 */
 #include <stdlib.h>




struct TreeNode* buildTree(struct ListNode* start, struct ListNode* end) {
    if (start == end) return NULL;
    
    struct ListNode* slow = start;
    struct ListNode* fast = start;
    
    while (fast != end && fast->next != end) {
        slow = slow->next;
        fast = fast->next->next;
    }
    
    struct TreeNode* root = (struct TreeNode*)malloc(sizeof(struct TreeNode));
    root->val = slow->val;
    root->left = buildTree(start, slow);
    root->right = buildTree(slow->next, end);
    
    return root;
}

struct TreeNode* sortedListToBST(struct ListNode* head) {
    return buildTree(head, NULL);
}
 
