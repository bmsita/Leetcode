#include <liburing.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define MAX_FILES 3
#define BUF_SIZE  8192   // chunk size for reading

struct file_ctx {
    int fd;
    char *buf;
    const char *name;
};


int main() {
    struct io_uring ring;
    io_uring_queue_init(MAX_FILES, &ring, 0);

    const char *input_files[MAX_FILES] = {"file1.txt", "file2.txt", "file3.txt"};
    struct file_ctx ctxs[MAX_FILES];
    int output_fd = open("merged_output.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);

    if (output_fd < 0) {
        perror("open output");
        return 1;
    }

    // Submit read requests
    for (int i = 0; i < MAX_FILES; i++) {
        ctxs[i].fd = open(input_files[i], O_RDONLY);
        if (ctxs[i].fd < 0) {
            perror("open input");
            return 1;
        }

        ctxs[i].buf = malloc(BUF_SIZE);
        ctxs[i].name = input_files[i];
        if (!ctxs[i].buf) {
            perror("malloc");
            return 1;
        }

        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
        io_uring_prep_read(sqe, ctxs[i].fd, ctxs[i].buf, BUF_SIZE, 0);
        io_uring_sqe_set_data(sqe, &ctxs[i]);
    }

    io_uring_submit(&ring);

    // Collect completions
    for (int i = 0; i < MAX_FILES; i++) {
        struct io_uring_cqe *cqe;
        io_uring_wait_cqe(&ring, &cqe);

        struct file_ctx *ctx = (struct file_ctx *) io_uring_cqe_get_data(cqe);

        if (cqe->res < 0) {
            fprintf(stderr, "Read failed for %s: %s\n", ctx->name, strerror(-cqe->res));
        }
        else if (cqe->res > 0) {
          struct file_ctx *ctx = (struct file_ctx *) io_uring_cqe_get_data(cqe);

          // prepare async write
          struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
          io_uring_prep_write(sqe, output_fd, ctx->buf, cqe->res, -1);
          io_uring_sqe_set_data(sqe, ctx);  // reuse same context if needed

          io_uring_submit(&ring);
        }
        io_uring_cqe_seen(&ring, cqe);
    }

    // Cleanup
    for (int i = 0; i < MAX_FILES; i++) {
        close(ctxs[i].fd);
        free(ctxs[i].buf);
    }

    close(output_fd);
    io_uring_queue_exit(&ring);

    printf("Merged into merged_output.txt in completion order.\n");
    return 0;
}
