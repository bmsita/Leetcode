typedef struct Node {
    int key;
    int value;
    struct Node* prev;
    struct Node* next;
    struct Node* hnext;
} Node;

typedef struct {
    int capacity;
    int size;
    Node* head;
    Node* tail;
    Node** map;
    int map_size;
} LRUCache;

Node* createNode(int key, int value) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->key = key;
    node->value = value;
    node->prev = node->next = node->hnext = NULL;
    return node;
}

unsigned int hash(int key, int map_size) {
    return key % map_size;
}

void removeNode(LRUCache* cache, Node* node) {
    if (node->prev) node->prev->next = node->next;
    else cache->head = node->next;
    if (node->next) node->next->prev = node->prev;
    else cache->tail = node->prev;
}

void addToHead(LRUCache* cache, Node* node) {
    node->next = cache->head;
    node->prev = NULL;
    if (cache->head) cache->head->prev = node;
    cache->head = node;
    if (!cache->tail) cache->tail = node;
}

Node* getNode(LRUCache* cache, int key) {
    unsigned int idx = hash(key, cache->map_size);
    Node* node = cache->map[idx];
    while (node) {
        if (node->key == key) return node;
        node = node->hnext;
    }
    return NULL;
}

void putNode(LRUCache* cache, Node* node) {
    unsigned int idx = hash(node->key, cache->map_size);
    node->hnext = cache->map[idx];
    cache->map[idx] = node;
}

void removeFromMap(LRUCache* cache, int key) {
    unsigned int idx = hash(key, cache->map_size);
    Node* node = cache->map[idx];
    Node* prev = NULL;
    while (node) {
        if (node->key == key) {
            if (prev) prev->hnext = node->hnext;
            else cache->map[idx] = node->hnext;
            break;
        }
        prev = node;
        node = node->hnext;
    }
}

LRUCache* lRUCacheCreate(int capacity) {
    LRUCache* cache = (LRUCache*)malloc(sizeof(LRUCache));
    cache->capacity = capacity;
    cache->size = 0;
    cache->head = cache->tail = NULL;
    cache->map_size = 10007;
    cache->map = (Node**)calloc(cache->map_size, sizeof(Node*));
    return cache;
}

int lRUCacheGet(LRUCache* cache, int key) {
    Node* node = getNode(cache, key);
    if (!node) return -1;
    removeNode(cache, node);
    addToHead(cache, node);
    return node->value;
}

void lRUCachePut(LRUCache* cache, int key, int value) {
    Node* node = getNode(cache, key);
    if (node) {
        node->value = value;
        removeNode(cache, node);
        addToHead(cache, node);
    } else {
        if (cache->size == cache->capacity) {
            Node* tail = cache->tail;
            removeNode(cache, tail);
            removeFromMap(cache, tail->key);
            free(tail);
            cache->size--;
        }
        Node* newNode = createNode(key, value);
        addToHead(cache, newNode);
        putNode(cache, newNode);
        cache->size++;
    }
}

void lRUCacheFree(LRUCache* cache) {
    Node* node = cache->head;
    while (node) {
        Node* tmp = node;
        node = node->next;
        free(tmp);
    }
    free(cache->map);
    free(cache);
}






