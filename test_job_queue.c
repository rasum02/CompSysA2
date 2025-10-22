#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include "job_queue.h"
#include <unistd.h>

// Tests initialization and destruction of a job-queue
void test_init_destroy() {
    struct job_queue q;
    assert(job_queue_init(&q, 10) == 0);
    assert(job_queue_destroy(&q) == 0);
    printf("test_init_destroy passed\n");
}

// Tests pushing and popping using a single thread
void test_push_pop_single_thread() {
    struct job_queue q;
    job_queue_init(&q, 2);

    int a = 42;
    job_queue_push(&q, &a);

    void *result;
    job_queue_pop(&q, &result);
    assert(*(int *)result == 42);

    job_queue_destroy(&q);
    printf("test_push_pop_single_thread passed\n");
}

// Tests whether push is blocked when queue is full
void *push_blocking(void *arg) {
    struct job_queue *q = (struct job_queue *)arg;
    int *x = malloc(sizeof(int));
    *x = 99;
    job_queue_push(q, x); // Should block until space is available
    return NULL;
}

void test_push_blocks_when_full() {
    struct job_queue q;
    job_queue_init(&q, 1);

    int a = 1;
    job_queue_push(&q, &a); // Fill the queue

    pthread_t t;
    pthread_create(&t, NULL, push_blocking, &q); // This thread should block
    sleep(1); // Give time for thread to reach blocking point

    void *result;
    job_queue_pop(&q, &result); // This should unblock the push

    pthread_join(t, NULL); // Wait for push_blocking to finish

    job_queue_pop(&q, &result); // Pop the job pushed by push_blocking
    assert(*(int *)result == 99);
    free(result); // Clean up

    job_queue_destroy(&q);
    printf("test_push_blocks_when_full passed\n");
}

// Tests whether pop is blocked when queue is empty
void *pop_blocking(void *arg) {
    struct job_queue *q = (struct job_queue *)arg;
    void *result;
    job_queue_pop(q, &result); // Should block until data is available
    assert(*(int *)result == 123);
    return NULL;
}

void test_pop_blocks_when_empty() {
    struct job_queue q;
    job_queue_init(&q, 1);

    pthread_t t;
    pthread_create(&t, NULL, pop_blocking, &q);

    sleep(1); // Give time for thread to block

    int x = 123;
    job_queue_push(&q, &x); // This should unblock the pop

    pthread_join(t, NULL);
    job_queue_destroy(&q);
    printf("test_pop_blocks_when_empty passed\n");
}

// Tests whether destruction wakes blocked threads
void *pop_wait_destroy(void *arg) {
    struct job_queue *q = (struct job_queue *)arg;
    void *result;
    int ret = job_queue_pop(q, &result);
    assert(ret == -1); // Should return -1 after destroy
    return NULL;
}

void test_destroy_wakes_blocked_threads() {
    struct job_queue q;
    job_queue_init(&q, 1);

    pthread_t t;
    pthread_create(&t, NULL, pop_wait_destroy, &q);

    sleep(1); // Let thread block on pop
    job_queue_destroy(&q); // Should wake thread

    pthread_join(t, NULL);
    printf("test_destroy_wakes_blocked_threads passed\n");
}

// Tests concurrent pushing and popping
void *producer(void *arg) {
    struct job_queue *q = (struct job_queue *)arg;
    for (int i = 0; i < 100; i++) {
        int *val = malloc(sizeof(int));
        *val = i;
        job_queue_push(q, val);
    }
    return NULL;
}

void *consumer(void *arg) {
    struct job_queue *q = (struct job_queue *)arg;
    for (int i = 0; i < 100; i++) {
        void *data;
        job_queue_pop(q, &data);
        free(data);
    }
    return NULL;
}

void test_concurrent_push_pop() {
    struct job_queue q;
    job_queue_init(&q, 10);

    pthread_t prod, cons;
    pthread_create(&prod, NULL, producer, &q);
    pthread_create(&cons, NULL, consumer, &q);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    job_queue_destroy(&q);
    printf("test_concurrent_push_pop passed\n");
}

int main() {
    test_init_destroy();
    test_push_pop_single_thread();
    test_push_blocks_when_full();
    test_pop_blocks_when_empty();
    test_destroy_wakes_blocked_threads();
    test_concurrent_push_pop();
    return 0;
}