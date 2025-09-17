#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_KEYS 128
#define MAX_TX 32

typedef enum { READ, WRITE } LockType;

// Version of a key
typedef struct Version {
    char *value;
    int tx_id;          // transaction that created this version
    int committed;      // 1 if committed, 0 if tentative
    int commit_ts;      // commit timestamp
    struct Version *next;
} Version;

// Key with multiple versions
typedef struct KVPair {
    char *key;
    Version *versions;
} KVPair;

// Lock for deadlock detection
typedef struct Lock {
    char *key;
    LockType type;
    int tx_id;
    struct Lock *next;
} Lock;

// Transaction structure
typedef struct {
    int id;
    int active;
    int start_ts;       // snapshot timestamp
    Lock *locks;
} Transaction;

KVPair db[MAX_KEYS];
int db_size = 0;
Transaction transactions[MAX_TX];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int global_ts = 1;      // global logical timestamp

// Wait-for graph
int wait_for[MAX_TX][MAX_TX];


char* mvcc_read(Transaction *tx, const char *key) {
    for (int i = 0; i < db_size; i++) {
        if (strcmp(db[i].key, key) == 0) {
            Version *ver = db[i].versions;
            Version *latest = NULL;
            while (ver) {
                if (ver->committed && ver->commit_ts <= tx->start_ts) {
                    latest = ver;
                    break; // versions are in newest-first order
                }
                if (!ver->committed && ver->tx_id == tx->id) {
                    // read own uncommitted version
                    latest = ver;
                    break;
                }
                ver = ver->next;
            }
            return latest ? latest->value : NULL;
        }
    }
    return NULL;
}

void mvcc_write(Transaction *tx, const char *key, const char *value) {
    KVPair *kv = NULL;
    for (int i = 0; i < db_size; i++) {
        if (strcmp(db[i].key, key) == 0) {
            kv = &db[i];
            break;
        }
    }
    if (!kv) {
        kv = &db[db_size++];
        kv->key = strdup(key);
        kv->versions = NULL;
    }

    Version *ver = malloc(sizeof(Version));
    ver->value = strdup(value);
    ver->tx_id = tx->id;
    ver->committed = 0;   // tentative
    ver->commit_ts = 0;
    ver->next = kv->versions;
    kv->versions = ver;
}

int acquire_lock(Transaction *tx, const char *key, LockType type) {
    for (int i = 0; i < MAX_TX; i++) {
        if (transactions[i].active) {
            for (Lock *l = transactions[i].locks; l; l = l->next) {
                if (strcmp(l->key, key) == 0 && l->tx_id != tx->id) {
                    if (l->type == WRITE || type == WRITE) {
                        wait_for[tx->id][l->tx_id] = 1;
                        return 0;
                    }
                }
            }
        }
    }
    Lock *new_lock = malloc(sizeof(Lock));
    new_lock->key = strdup(key);
    new_lock->type = type;
    new_lock->tx_id = tx->id;
    new_lock->next = tx->locks;
    tx->locks = new_lock;
    return 1;
}


void release_locks(Transaction *tx) {
    Lock *l = tx->locks;
    while (l) {
        Lock *tmp = l;
        l = l->next;
        free(tmp->key);
        free(tmp);
    }
    tx->locks = NULL;
}


int visited[MAX_TX], stack[MAX_TX];
int detect_cycle_util(int v, int n) {
    visited[v] = 1; stack[v] = 1;
    for (int u = 0; u < n; u++) {
        if (wait_for[v][u]) {
            if (!visited[u] && detect_cycle_util(u, n)) return 1;
            else if (stack[u]) return 1;
        }
    }
    stack[v] = 0;
    return 0;
}

int detect_deadlock(int n) {
    memset(visited, 0, sizeof(visited));
    memset(stack, 0, sizeof(stack));
    for (int i = 0; i < n; i++)
        if (!visited[i] && detect_cycle_util(i, n)) return 1;
    return 0;
}

Transaction* begin_tx(int id) {
    transactions[id].id = id;
    transactions[id].active = 1;
    transactions[id].start_ts = global_ts++; // snapshot timestamp
    transactions[id].locks = NULL;
    return &transactions[id];
}

// Commit: mark versions as committed
void commit_tx(Transaction *tx) {
    int commit_ts = global_ts++;
    for (int i = 0; i < db_size; i++) {
        Version *ver = db[i].versions;
        while (ver) {
            if (ver->tx_id == tx->id) {
                ver->committed = 1;
                ver->commit_ts = commit_ts;
            }
            ver = ver->next;
        }
    }
    release_locks(tx);
    tx->active = 0;
    printf("Transaction %d committed (ts=%d)\n", tx->id, commit_ts);
}

// Abort: remove uncommitted versions
void abort_tx(Transaction *tx) {
    for (int i = 0; i < db_size; i++) {
        Version **prev = &db[i].versions;
        Version *ver = db[i].versions;
        while (ver) {
            if (ver->tx_id == tx->id) {
                *prev = ver->next;
                free(ver->value);
                free(ver);
                ver = *prev;
            } else {
                prev = &ver->next;
                ver = ver->next;
            }
        }
    }
    release_locks(tx);
    tx->active = 0;
    printf("Transaction %d aborted\n", tx->id);
}


int main() {
    pthread_mutex_lock(&lock);

    Transaction *t1 = begin_tx(1);
    Transaction *t2 = begin_tx(2);

    acquire_lock(t1, "x", WRITE);
    acquire_lock(t2, "y", WRITE);

    mvcc_write(t1, "x", "100"); 
    mvcc_write(t2, "y", "200"); 

    if (!acquire_lock(t1, "y", WRITE)) printf("T1 waiting for y\n");
    if (!acquire_lock(t2, "x", WRITE)) printf("T2 waiting for x\n");

    if (detect_deadlock(MAX_TX)) {
        printf("Deadlock detected!\n");
        abort_tx(t2);
    }

    if (t1->active) commit_tx(t1);

    char *val = mvcc_read(t2, "x");
    printf("T2 reads x = %s\n", val ? val : "NULL");

    pthread_mutex_unlock(&lock);
    return 0;
}
