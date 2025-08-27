int firstUniqChar(char* s) {
    int freq[26] = {0};
    int len = strlen(s);
    for (int i = 0; i < len; i++) {
        freq[s[i] - 'a']++;
    }
    for (int i = 0; i < len; i++) {
        if (freq[s[i] - 'a'] == 1)
            return i;
    }
    return -1;
}
    