/* bm_combined.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#define ITERATIONS 10000
#define NS_PER_SEC 1000000000L

/* Global variables for signal latency benchmark */
volatile sig_atomic_t signal_received = 0;
struct timespec sig_start, sig_end;

/* Global variables for timer benchmark */
timer_t timer_id;
volatile sig_atomic_t timer_expired = 0;
struct timespec timer_start, timer_end;

/* Signal handler for SIGUSR1 (signal latency benchmark) */
void signal_handler(int signum) {
    signal_received = 1;
    clock_gettime(CLOCK_MONOTONIC, &sig_end);
}

/* Signal handler for SIGRTMIN (timer benchmark) */
void timer_handler(int signum) {
    timer_expired = 1;
    clock_gettime(CLOCK_MONOTONIC, &timer_end);
}

/* Configure real-time scheduling with maximum FIFO priority */
void configure_realtime_scheduling() {
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
        perror("sched_setscheduler");
        exit(EXIT_FAILURE);
    }
}

/* Lock memory to avoid paging delays */
void lock_memory() {
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
        perror("mlockall");
        exit(EXIT_FAILURE);
    }
}

/* Benchmark nanosleep() */
void benchmark_nanosleep() {
    long long total_jitter = 0, max_jitter = 0, min_jitter = LLONG_MAX;
    struct timespec start, end;
    struct timespec sleep_time = {0, 1000000}; // 1 ms

    FILE *csv = fopen("nanosleep.csv", "w");
    if (!csv) {
        perror("fopen nanosleep.csv");
        exit(EXIT_FAILURE);
    }
    fprintf(csv, "Iteration,Jitter_ns\n");

    for (int i = 0; i < ITERATIONS; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        nanosleep(&sleep_time, NULL);
        clock_gettime(CLOCK_MONOTONIC, &end);

        long long elapsed = (end.tv_sec - start.tv_sec) * NS_PER_SEC +
                            (end.tv_nsec - start.tv_nsec);
        long long jitter = elapsed - sleep_time.tv_nsec;
        total_jitter += llabs(jitter);
        if (jitter > max_jitter) max_jitter = jitter;
        if (jitter < min_jitter) min_jitter = jitter;
        fprintf(csv, "%d,%lld\n", i, jitter);
    }
    fclose(csv);
    printf("Nanosleep Benchmark:\n");
    printf("  Average jitter: %lld ns\n", total_jitter / ITERATIONS);
    printf("  Max jitter: %lld ns\n", max_jitter);
    printf("  Min jitter: %lld ns\n", min_jitter);
}

/* Benchmark usleep() */
void benchmark_usleep() {
    long long total_jitter = 0, max_jitter = 0, min_jitter = LLONG_MAX;
    struct timespec start, end;
    const unsigned int sleep_us = 1000;  // 1 ms = 1000 microseconds
    long expected_ns = 1000000;          // 1 ms in nanoseconds

    FILE *csv = fopen("usleep.csv", "w");
    if (!csv) {
        perror("fopen usleep.csv");
        exit(EXIT_FAILURE);
    }
    fprintf(csv, "Iteration,Jitter_ns\n");

    for (int i = 0; i < ITERATIONS; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        usleep(sleep_us);
        clock_gettime(CLOCK_MONOTONIC, &end);

        long long elapsed = (end.tv_sec - start.tv_sec) * NS_PER_SEC +
                            (end.tv_nsec - start.tv_nsec);
        long long jitter = elapsed - expected_ns;
        total_jitter += llabs(jitter);
        if (jitter > max_jitter) max_jitter = jitter;
        if (jitter < min_jitter) min_jitter = jitter;
        fprintf(csv, "%d,%lld\n", i, jitter);
    }
    fclose(csv);
    printf("Usleep Benchmark:\n");
    printf("  Average jitter: %lld ns\n", total_jitter / ITERATIONS);
    printf("  Max jitter: %lld ns\n", max_jitter);
    printf("  Min jitter: %lld ns\n", min_jitter);
}

/* Benchmark signal handling latency */
void benchmark_signal_latency() {
    long long total_latency = 0, max_latency = 0, min_latency = LLONG_MAX;

    FILE *csv = fopen("signal_latency.csv", "w");
    if (!csv) {
        perror("fopen signal_latency.csv");
        exit(EXIT_FAILURE);
    }
    fprintf(csv, "Iteration,Latency_ns\n");

    signal(SIGUSR1, signal_handler);

    for (int i = 0; i < ITERATIONS; i++) {
        clock_gettime(CLOCK_MONOTONIC, &sig_start);
        kill(getpid(), SIGUSR1);
        while (!signal_received)
            ;  // Busy-wait until signal is received
        long long latency = (sig_end.tv_sec - sig_start.tv_sec) * NS_PER_SEC +
                            (sig_end.tv_nsec - sig_start.tv_nsec);
        total_latency += latency;
        if (latency > max_latency) max_latency = latency;
        if (latency < min_latency) min_latency = latency;
        fprintf(csv, "%d,%lld\n", i, latency);
        signal_received = 0;
    }
    fclose(csv);
    printf("\nSignal Latency Benchmark:\n");
    printf("  Average latency: %lld ns\n", total_latency / ITERATIONS);
    printf("  Max latency: %lld ns\n", max_latency);
    printf("  Min latency: %lld ns\n", min_latency);
}

/* Benchmark interval timer using timer_create() and SIGRTMIN */
void benchmark_timer() {
    long long total_jitter = 0, max_jitter = 0, min_jitter = LLONG_MAX;

    FILE *csv = fopen("timer.csv", "w");
    if (!csv) {
        perror("fopen timer.csv");
        exit(EXIT_FAILURE);
    }
    fprintf(csv, "Iteration,Jitter_ns\n");

    struct sigevent sev;
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGRTMIN;
    sev.sigev_value.sival_ptr = &timer_id;

    if (timer_create(CLOCK_MONOTONIC, &sev, &timer_id) == -1) {
        perror("timer_create");
        exit(EXIT_FAILURE);
    }

    struct itimerspec its;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 1000000; // 1 ms
    its.it_interval = its.it_value;

    signal(SIGRTMIN, timer_handler);

    if (timer_settime(timer_id, 0, &its, NULL) == -1) {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    }
    clock_gettime(CLOCK_MONOTONIC, &timer_start);
    for (int i = 0; i < ITERATIONS; i++) {
        while (!timer_expired){
        struct timespec ts = {0, 100};
        nanosleep(&ts, NULL);
        }
        // Busy-wait until the timer expires
        long long elapsed = (timer_end.tv_sec - timer_start.tv_sec) * NS_PER_SEC +
                            (timer_end.tv_nsec - timer_start.tv_nsec);
        long long jitter = elapsed - its.it_interval.tv_nsec;
        total_jitter += llabs(jitter);
        if (jitter > max_jitter) max_jitter = jitter;
        if (jitter < min_jitter) min_jitter = jitter;
        fprintf(csv, "%d,%lld\n", i, jitter);
        timer_expired = 0;
        timer_start = timer_end;
    }
    fclose(csv);
    printf("\nTimer Benchmark:\n");
    printf("  Average jitter: %lld ns\n", total_jitter / ITERATIONS);
    printf("  Max jitter: %lld ns\n", max_jitter);
    printf("  Min jitter: %lld ns\n", min_jitter);

    timer_delete(timer_id);
}

int main() {
    configure_realtime_scheduling();
    lock_memory();

    benchmark_nanosleep();
    benchmark_usleep();
    benchmark_signal_latency();
    benchmark_timer();

    return 0;
}
