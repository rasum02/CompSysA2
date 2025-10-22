#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>

#include "job_queue.h"

int job_queue_init(struct job_queue *job_queue, int capacity) {
    job_queue->total_capacity = capacity;
    job_queue->count = 0;
    job_queue->head = 0;
    job_queue->tail = 0;
    job_queue->is_destroyed = 0;
    job_queue->job = malloc(sizeof(void*) * capacity);
    if (!job_queue->job) return -1;

    pthread_mutex_init(&job_queue->mutex, NULL);
    pthread_cond_init(&job_queue->not_empty, NULL);
    pthread_cond_init(&job_queue->not_full, NULL);
    pthread_cond_init(&job_queue->destroyed, NULL);
    return 0;
}

int job_queue_push(struct job_queue *job_queue, void *data) {
    pthread_mutex_lock(&job_queue->mutex);

    // Wait if full
    while (job_queue->count == job_queue->total_capacity && !job_queue->is_destroyed) {
        pthread_cond_wait(&job_queue->not_full, &job_queue->mutex);
    }

    if (job_queue->is_destroyed) {
        pthread_mutex_unlock(&job_queue->mutex);
        return -1;
    }

    job_queue->job[job_queue->tail] = data;
    job_queue->tail = (job_queue->tail + 1) % job_queue->total_capacity;
    job_queue->count++;

    pthread_cond_signal(&job_queue->not_empty);
    pthread_mutex_unlock(&job_queue->mutex);
    return 0;
}

int job_queue_pop(struct job_queue *job_queue, void **data) {
    pthread_mutex_lock(&job_queue->mutex);

    // Wait if empty
    while (job_queue->count == 0 && !job_queue->is_destroyed) {
        pthread_cond_wait(&job_queue->not_empty, &job_queue->mutex);
    }

    // If destroyed and empty, return -1
    if (job_queue->is_destroyed && job_queue->count == 0) {
        pthread_mutex_unlock(&job_queue->mutex);
        return -1;
    }

    *data = job_queue->job[job_queue->head];
    job_queue->head = (job_queue->head + 1) % job_queue->total_capacity;
    job_queue->count--;

    pthread_cond_signal(&job_queue->not_full);
    pthread_mutex_unlock(&job_queue->mutex);
    return 0;
}

int job_queue_destroy(struct job_queue *job_queue) {
    pthread_mutex_lock(&job_queue->mutex);

    // Wait until queue becomes empty
    while (job_queue->count > 0) {
        pthread_cond_wait(&job_queue->not_full, &job_queue->mutex);
    }

    job_queue->is_destroyed = 1;

    // Wake all waiting threads so they can exit
    pthread_cond_broadcast(&job_queue->not_empty);
    pthread_cond_broadcast(&job_queue->not_full);

    pthread_mutex_unlock(&job_queue->mutex);

    free(job_queue->job);

    pthread_mutex_destroy(&job_queue->mutex);
    pthread_cond_destroy(&job_queue->not_empty);
    pthread_cond_destroy(&job_queue->not_full);

    return 0;
}
