

#define MAX_LEVEL 32
#define P_FACTOR 0.5

typedef struct Node {
    int val;
    int level;
    struct Node* forward[];
} Node;

typedef struct {
    Node* head;
    int level;
} Skiplist;

static int randomLevel() {
    int lvl = 1;
    while (((double)rand() / RAND_MAX) < P_FACTOR && lvl < MAX_LEVEL) lvl++;
    return lvl;
}

static Node* newNode(int val, int level) {
    Node* n = (Node*)malloc(sizeof(Node) + sizeof(Node*) * level);
    n->val = val;
    n->level = level;
    for (int i = 0; i < level; ++i) n->forward[i] = NULL;
    return n;
}

Skiplist* skiplistCreate() {
    srand(1);
    Skiplist* obj = (Skiplist*)malloc(sizeof(Skiplist));
    obj->level = 1;
    obj->head = newNode(INT_MIN, MAX_LEVEL);
    return obj;
}

bool skiplistSearch(Skiplist* obj, int target) {
    Node* cur = obj->head;
    for (int i = obj->level - 1; i >= 0; --i) {
        while (cur->forward[i] && cur->forward[i]->val < target) cur = cur->forward[i];
    }
    cur = cur->forward[0];
    return cur && cur->val == target;
}

void skiplistAdd(Skiplist* obj, int num) {
    Node* update[MAX_LEVEL];
    Node* cur = obj->head;
    for (int i = obj->level - 1; i >= 0; --i) {
        while (cur->forward[i] && cur->forward[i]->val < num) cur = cur->forward[i];
        update[i] = cur;
    }
    int lvl = randomLevel();
    if (lvl > obj->level) {
        for (int i = obj->level; i < lvl; ++i) update[i] = obj->head;
        obj->level = lvl;
    }
    Node* n = newNode(num, lvl);
    for (int i = 0; i < lvl; ++i) {
        n->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = n;
    }
}

bool skiplistErase(Skiplist* obj, int num) {
    Node* update[MAX_LEVEL];
    Node* cur = obj->head;
    for (int i = obj->level - 1; i >= 0; --i) {
        while (cur->forward[i] && cur->forward[i]->val < num) cur = cur->forward[i];
        update[i] = cur;
    }
    cur = cur->forward[0];
    if (!cur || cur->val != num) return false;
    for (int i = 0; i < obj->level; ++i) {
        if (update[i]->forward[i] != cur) break;
        update[i]->forward[i] = cur->forward[i];
    }
    free(cur);
    while (obj->level > 1 && obj->head->forward[obj->level - 1] == NULL) obj->level--;
    return true;
}

void skiplistFree(Skiplist* obj) {
    Node* cur = obj->head->forward[0];
    while (cur) {
        Node* next = cur->forward[0];
        free(cur);
        cur = next;
    }
    free(obj->head);
    free(obj);
}
