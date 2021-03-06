/*
 * Copyright (c) 2012-2015, Brian Watling and other contributors
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "lockfree_ring_buffer.h"
#include "test_helper.h"
#include <pthread.h>

#define PER_THREAD_COUNT 10000000
#define NUM_THREADS 4

lockfree_ring_buffer_t* rb;
char counters[PER_THREAD_COUNT] = {};
pthread_barrier_t barrier;

void* run_function(void* param)
{
    pthread_barrier_wait(&barrier);
    intptr_t i;
    for(i = 1; i <= PER_THREAD_COUNT; ++i) {
        lockfree_ring_buffer_push(rb, (void*)i);
        const size_t size = lockfree_ring_buffer_size(rb);
        test_assert(size <= 128);
        intptr_t j = (intptr_t)lockfree_ring_buffer_pop(rb);
        test_assert(j > 0 && j <= PER_THREAD_COUNT);
        __sync_add_and_fetch(&counters[j - 1], 1);
    }
    return NULL;
}

int main()
{
    rb = lockfree_ring_buffer_create(7);
    pthread_barrier_init(&barrier, NULL, NUM_THREADS);

    pthread_t threads[NUM_THREADS];
    int i;
    for(i = 0; i < NUM_THREADS; ++i) {
        pthread_create(&threads[i], NULL, &run_function, NULL);
    }

    for(i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    lockfree_ring_buffer_destroy(rb);

    for(i = 0; i < PER_THREAD_COUNT; ++i) {
        test_assert(counters[i] == NUM_THREADS);
    }

    return 0;
}

