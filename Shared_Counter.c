#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define CHILD_COUNT 4
#define LOOP_COUNT 100

typedef struct MemBlock {
    atomic_int arr[CHILD_COUNT];
} MemBlock;

int throwError(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    int shmid = shmget(IPC_PRIVATE, sizeof(MemBlock), IPC_CREAT | 0600);
    if (shmid < 0) {
        throwError("shmget failed");
    }

    MemBlock *ptr = (MemBlock *)shmat(shmid, NULL, 0);
    if (ptr == (void *)-1) {
        throwError("shmat failed");
    }

    for (int a = 0; a < CHILD_COUNT; a++) {
        atomic_store(&ptr->arr[a], 0);
    }

    for (int id = 0; id < CHILD_COUNT; id++) {
        int pid = fork();
        if (pid < 0) {
            throwError("fork error");
        }
        if (pid == 0) {
            MemBlock *cptr = (MemBlock *)shmat(shmid, NULL, 0);
            if (cptr == (void *)-1) {
                throwError("child shmat error");
            }
            for (int t = 0; t < LOOP_COUNT; t++) {
                atomic_fetch_add(&cptr->arr[id], 1);
            }
            shmdt(cptr);
            _exit(0);
        }
    }

    for (int c = 0; c < CHILD_COUNT; c++) {
        wait(NULL);
    }

    int grand_total = 0;
    for (int p = 0; p < CHILD_COUNT; p++) {
        int val = atomic_load(&ptr->arr[p]);
        printf("Child %d : counter = %d\n", p, val);
        grand_total += val;
    }

    printf("Final Total = %d\n", grand_total);

    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
