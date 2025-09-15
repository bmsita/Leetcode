#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>

static long mutex_counter = 0;
atomic_long atomic_counter = 0;

pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    long iterations;
} benchmark_arg_t;

static void* worker_atomic(void *arg) {
    benchmark_arg_t *barg = (benchmark_arg_t *)arg;
    long iterations = barg->iterations;

    for (long i = 0; i < iterations; i++) {
        atomic_fetch_add(&atomic_counter, 1);
    }
    return NULL;
}

static void* worker_mutex(void *arg) {
    benchmark_arg_t *barg = (benchmark_arg_t *)arg;
    long iterations = barg->iterations;

    for (long i = 0; i < iterations; i++) {
        pthread_mutex_lock(&counter_mutex);
        mutex_counter++;
        pthread_mutex_unlock(&counter_mutex);
    }
    return NULL;
}

static double now_sec() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

static void run_benchmark(const char *label,
                          void *(*worker)(void *),
                          long num_threads,
                          long iterations) {
    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    benchmark_arg_t args = { iterations };

    double start = now_sec();

    for (long i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker, &args);
    }
    for (long i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    double end = now_sec();
    double elapsed = end - start;

    if (worker == worker_mutex) {
        printf("%s: counter = %ld, time = %.6f sec, throughput = %.2f Mops/sec\n",
               label, mutex_counter, elapsed,
               (num_threads * iterations) / (elapsed * 1e6));
    } else {
        printf("%s: counter = %ld, time = %.6f sec, throughput = %.2f Mops/sec\n",
               label, atomic_counter, elapsed,
               (num_threads * iterations) / (elapsed * 1e6));
    }

    free(threads);
}

int main(int argc, char **argv) {
    long num_threads = (argc > 1) ? atol(argv[1]) : 64;
    long iterations = (argc > 2) ? atol(argv[2]) : 2000000;

    printf("Running with %ld threads, %ld iterations each\n", num_threads, iterations);

    // Mutex benchmark
    mutex_counter = 0;
    run_benchmark("Mutex", worker_mutex, num_threads, iterations);

    // Atomic benchmark
    atomic_store(&atomic_counter, 0);
    run_benchmark("Atomic", worker_atomic, num_threads, iterations);

    return 0;
}
