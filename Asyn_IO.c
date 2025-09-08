#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <aio.h>
#include <errno.h>
#include <time.h>

#define FILE_NAME "testfile.bin"
#define FILE_SIZE (100 * 1024 * 1024) // 100 MB
#define BLOCK_SIZE (4 * 1024)         // 4 KB

static double now_sec()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main()
{
    int fd = open(FILE_NAME, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    char *buf = aligned_alloc(4096, BLOCK_SIZE);
    memset(buf, 'A', BLOCK_SIZE);

    struct aiocb cb;
    memset(&cb, 0, sizeof(cb));
    cb.aio_fildes = fd;
    cb.aio_buf = buf;
    cb.aio_nbytes = BLOCK_SIZE;

    size_t blocks = FILE_SIZE / BLOCK_SIZE;

    // --- Async Write Test ---
    double t1 = now_sec();
    for (size_t i = 0; i < blocks; i++)
    {
        cb.aio_offset = i * BLOCK_SIZE;
        if (aio_write(&cb) < 0)
        {
            perror("aio_write");
            return 1;
        }
        while (aio_error(&cb) == EINPROGRESS)
        {
            // spin until completion
        }
        if (aio_return(&cb) < 0)
        {
            perror("aio_return");
            return 1;
        }
    }
    fsync(fd);
    double t2 = now_sec();

    double write_speed = (double)FILE_SIZE / (t2 - t1) / (1024 * 1024);
    printf("Async write speed: %.2f MB/s\n", write_speed);

    // --- Async Read Test ---
    lseek(fd, 0, SEEK_SET);
    double t3 = now_sec();
    for (size_t i = 0; i < blocks; i++)
    {
        cb.aio_offset = i * BLOCK_SIZE;
        if (aio_read(&cb) < 0)
        {
            perror("aio_read");
            return 1;
        }
        while (aio_error(&cb) == EINPROGRESS)
        {
            // spin until completion
        }
        if (aio_return(&cb) < 0)
        {
            perror("aio_return");
            return 1;
        }
    }
    double t4 = now_sec();

    double read_speed = (double)FILE_SIZE / (t4 - t3) / (1024 * 1024);
    printf("Async read speed: %.2f MB/s\n", read_speed);

    free(buf);
    close(fd);
    unlink(FILE_NAME);
    return 0;
}
