#define TABLE_SIZE 1007 // Bigger size to reduce collisions

struct Node
{
    char *key;      // sorted string (hash key)
    char *original; // original string
    struct Node *next;
};

struct Node *hashTable[TABLE_SIZE] = {NULL};

// Simple hash function (djb2 variant)
unsigned int hash(const char *key)
{
    unsigned long hashValue = 5381;
    int c;
    while ((c = *key++))
        hashValue = ((hashValue << 5) + hashValue) + c;
    return hashValue % TABLE_SIZE;
}

// Insert into hash table (grouping by sorted key)
void insert(const char *sortedKey, const char *original)
{
    unsigned int index = hash(sortedKey);
    struct Node *newNode = malloc(sizeof(struct Node));
    newNode->key = strdup(sortedKey);
    newNode->original = strdup(original);
    newNode->next = hashTable[index];
    hashTable[index] = newNode;
}

// Sort string helper
char *sortString(const char *s)
{
    char *sorted = strdup(s);
    int len = strlen(sorted);
    for (int i = 0; i < len - 1; i++)
    {
        for (int j = i + 1; j < len; j++)
        {
            if (sorted[i] > sorted[j])
            {
                char tmp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = tmp;
            }
        }
    }
    return sorted;
}

// Main grouping function
char ***groupAnagrams(char **strs, int strsSize, int *returnSize, int **returnColumnSizes)
{
    // Clear hash table
    for (int i = 0; i < TABLE_SIZE; i++)
        hashTable[i] = NULL;

    // Insert each string into hash table using sorted key
    for (int i = 0; i < strsSize; i++)
    {
        char *sortedKey = sortString(strs[i]);
        insert(sortedKey, strs[i]);
        free(sortedKey); // key is duplicated in insert, so free here
    }

    // Allocate output
    char ***result = NULL;
    *returnColumnSizes = NULL;
    int groups = 0;

    // Traverse hash table to collect groups
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        struct Node *temp = hashTable[i];
        if (temp)
        {
            // Gather all in this bucket
            int count = 0;
            char **group = NULL;

            while (temp)
            {
                group = realloc(group, sizeof(char *) * (count + 1));
                group[count] = strdup(temp->original);
                count++;
                temp = temp->next;
            }

            result = realloc(result, sizeof(char **) * (groups + 1));
            result[groups] = group;

            *returnColumnSizes = realloc(*returnColumnSizes, sizeof(int) * (groups + 1));
            (*returnColumnSizes)[groups] = count;

            groups++;
        }
    }
    *returnSize = groups;
    return result;
}