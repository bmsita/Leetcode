long mergeCount(long* sums, int left, int right, int lower, int upper) {
    if (right - left <= 1) return 0;
    int mid = (left + right) / 2;
    long count = mergeCount(sums, left, mid, lower, upper) + mergeCount(sums, mid, right, lower, upper);
    
    int j = mid, k = mid, t = mid;
    long* cache = (long*)malloc(sizeof(long) * (right - left));
    int r = 0;
    
    for (int i = left; i < mid; i++) {
        while (k < right && sums[k] - sums[i] < lower) k++;
        while (j < right && sums[j] - sums[i] <= upper) j++;
        while (t < right && sums[t] < sums[i]) cache[r++] = sums[t++];
        cache[r++] = sums[i];
        count += (j - k);
    }
    while (t < right) cache[r++] = sums[t++];
    for (int i = 0; i < r; i++) sums[left + i] = cache[i];
    free(cache);
    return count;
}

int countRangeSum(int* nums, int numsSize, int lower, int upper) {
    long* sums = (long*)malloc(sizeof(long) * (numsSize + 1));
    sums[0] = 0;
    for (int i = 0; i < numsSize; i++) sums[i + 1] = sums[i] + nums[i];
    
    long ans = mergeCount(sums, 0, numsSize + 1, lower, upper);
    free(sums);
    return (int)ans;
}
