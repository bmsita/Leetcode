#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define WAL_FILE "wal.log"
#define DB_FILE  "db.txt"
#define MAX_LINE 256

static int transaction_id = 1;

void append_file(const char *filename, const char *line, int sync) {
    int fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd < 0) { perror("open"); exit(1); }
    size_t len = strlen(line);
    if (write(fd, line, len) != len || write(fd, "\n", 1) != 1) {
        perror("write");
        close(fd);
        exit(1);
    }
    if (sync && fsync(fd) < 0) {
        perror("fsync");
        close(fd);
        exit(1);
    }
    close(fd);
}

void write_transaction(const char *key, const char *value, int sync) {
    char buf[MAX_LINE];
    snprintf(buf, sizeof(buf), "TRANSACTION %d BEGIN", transaction_id);
    append_file(WAL_FILE, buf, sync);
    snprintf(buf, sizeof(buf), "SET %s %s", key, value);
    append_file(WAL_FILE, buf, sync);
    snprintf(buf, sizeof(buf), "TRANSACTION %d COMMIT", transaction_id);
    append_file(WAL_FILE, buf, sync);
    printf("Transaction %d written to WAL%s.\n", transaction_id, sync ? " (fsync)" : "");
    transaction_id++;
}

void crash_after_wal(const char *key, const char *value) {
    write_transaction(key, value, 1);
    printf("Simulating crash BEFORE applying to DB.\n");
    exit(1);
}

void recover() {
    FILE *fp = fopen(WAL_FILE, "r");
    if (!fp) { perror("open wal"); exit(1); }
    char line[MAX_LINE];
    int in_tx = 0;
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;
        if (strncmp(line, "TRANSACTION", 11) == 0 && strstr(line, "BEGIN")) {
            in_tx = 1;
        } else if (strncmp(line, "TRANSACTION", 11) == 0 && strstr(line, "COMMIT")) {
            in_tx = 0;
        } else if (in_tx && strncmp(line, "SET", 3) == 0) {
            append_file(DB_FILE, line, 1);
        }
    }
    fclose(fp);
    printf("Recovery complete. DB updated.\n");
}

void display() {
    char buf[MAX_LINE];
    FILE *fp;
    printf("=== WAL LOG ===\n");
    fp = fopen(WAL_FILE, "r");
    if (fp) {
        while (fgets(buf, sizeof(buf), fp))
            printf("%s", buf);
        fclose(fp);
    }
    printf("\n=== DB FILE ===\n");
    fp = fopen(DB_FILE, "r");
    if (fp) {
        while (fgets(buf, sizeof(buf), fp))
            printf("%s", buf);
        fclose(fp);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args]\n", argv[0]);
        return 1;
    }
    if (strcmp(argv[1], "write-nosync") == 0 && argc == 4)
        write_transaction(argv[2], argv[3], 0);
    else if (strcmp(argv[1], "write-sync") == 0 && argc == 4)
        write_transaction(argv[2], argv[3], 1);
    else if (strcmp(argv[1], "crash-after-wal") == 0 && argc == 4)
        crash_after_wal(argv[2], argv[3]);
    else if (strcmp(argv[1], "recover") == 0)
        recover();
    else if (strcmp(argv[1], "display") == 0)
        display();
    else
        fprintf(stderr, "Invalid command or arguments.\n");
    return 0;
}
