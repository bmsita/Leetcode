/**
 * Note: The returned array must be malloced, assume caller calls free().
 */
int* maxSlidingWindow(int* nums, int numsSize, int k, int* returnSize) {
    if (numsSize == 0 || k <= 0 || k > numsSize) {
        *returnSize = 0;
        return NULL;
    }
    int outSize = numsSize - k + 1;
    *returnSize = outSize;
    int* result = (int*)malloc(sizeof(int) * outSize);
    int* dq = (int*)malloc(sizeof(int) * numsSize);
    int head = 0, tail = 0;

    for (int i = 0; i < numsSize; i++) {
        if (head < tail && dq[head] <= i - k) head++;
        while (head < tail && nums[dq[tail - 1]] <= nums[i]) tail--;
        dq[tail++] = i;
        if (i >= k - 1) result[i - k + 1] = nums[dq[head]];
    }

    free(dq);
    return result;
}