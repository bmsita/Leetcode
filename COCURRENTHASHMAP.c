#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct node {
    char *key;
    void *value;
    struct node *next;
} node_t;

typedef struct bucket {
    pthread_mutex_t lock;
    node_t *head;
} bucket_t;

typedef struct concurrentHashMap {
    size_t nbuckets;
    bucket_t *buckets;
} concurrentHashMap_t;

static unsigned long hash_str(const char *s) {
    unsigned long hash = 5381;
    int c;
    while ((c = *s++)) {
        hash = ((hash << 5) + hash) + (unsigned char)c;
    }
    return hash;
}

concurrentHashMap_t *map_create(size_t nbuckets) {
    concurrentHashMap_t *m = malloc(sizeof(*m));
    m->nbuckets = nbuckets;
    m->buckets = calloc(nbuckets, sizeof(bucket_t));
    for (size_t i = 0; i < nbuckets; i++) {
        pthread_mutex_init(&m->buckets[i].lock, NULL);
    }
    return m;
}

void *map_insert(concurrentHashMap_t *m, const char *key, void *value) {
    unsigned long h = hash_str(key) % m->nbuckets;
    bucket_t *b = &m->buckets[h];
    pthread_mutex_lock(&b->lock);
    node_t *cur = b->head;
    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            void *old = cur->value;
            cur->value = value;
            pthread_mutex_unlock(&b->lock);
            return old;
        }
        cur = cur->next;
    }
    node_t *n = malloc(sizeof(*n));
    n->key = strdup(key);
    n->value = value;
    n->next = b->head;
    b->head = n;
    pthread_mutex_unlock(&b->lock);
    return NULL;
}

void *map_get(concurrentHashMap_t *m, const char *key) {
    unsigned long h = hash_str(key) % m->nbuckets;
    bucket_t *b = &m->buckets[h];
    pthread_mutex_lock(&b->lock);
    node_t *cur = b->head;
    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            void *val = cur->value;
            pthread_mutex_unlock(&b->lock);
            return val;
        }
        cur = cur->next;
    }
    pthread_mutex_unlock(&b->lock);
    return NULL;
}

void *map_remove(concurrentHashMap_t *m, const char *key) {
    unsigned long h = hash_str(key) % m->nbuckets;
    bucket_t *b = &m->buckets[h];
    pthread_mutex_lock(&b->lock);
    node_t *cur = b->head, *prev = NULL;
    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            if (prev) prev->next = cur->next;
            else b->head = cur->next;
            void *val = cur->value;
            free(cur->key);
            free(cur);
            pthread_mutex_unlock(&b->lock);
            return val;
        }
        prev = cur;
        cur = cur->next;
    }
    pthread_mutex_unlock(&b->lock);
    return NULL;
}

void map_destroy(concurrentHashMap_t *m, void (*free_val)(void *)) {
    for (size_t i = 0; i < m->nbuckets; i++) {
        bucket_t *b = &m->buckets[i];
        pthread_mutex_lock(&b->lock);
        node_t *cur = b->head;
        while (cur) {
            node_t *next = cur->next;
            if (free_val) free_val(cur->value);
            free(cur->key);
            free(cur);
            cur = next;
        }
        pthread_mutex_unlock(&b->lock);
        pthread_mutex_destroy(&b->lock);
    }
    free(m->buckets);
    free(m);
}

typedef struct {
    concurrentHashMap_t *map;
    int tid;
} arg_t;

void *worker(void *p) {
    arg_t *a = p;
    char key[32];
    snprintf(key, sizeof(key), "key-%d", a->tid);
    int *val = malloc(sizeof(int));
    *val = a->tid * 100;
    map_insert(a->map, key, val);
    printf("Thread %d inserted %s -> %d\n", a->tid, key, *val);
    int *got = map_get(a->map, key);
    if (got) printf("Thread %d got %s -> %d\n", a->tid, key, *got);
    return NULL;
}

int main() {
    concurrentHashMap_t *m = map_create(64);
    const int N = 4;
    pthread_t th[N];
    arg_t args[N];
    for (int i = 0; i < N; i++) {
        args[i].map = m;
        args[i].tid = i + 1;
        pthread_create(&th[i], NULL, worker, &args[i]);
    }
    for (int i = 0; i < N; i++) pthread_join(th[i], NULL);
    int *removed = map_remove(m, "key-2");
    if (removed) {
        printf("Removed key-2 -> %d\n", *removed);
        free(removed);
    }
    map_destroy(m, free);
    return 0;
}
