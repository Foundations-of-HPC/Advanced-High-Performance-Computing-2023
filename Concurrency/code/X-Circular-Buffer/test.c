#include <stdio.h>
#include <stdlib.h>

#include "circbuf.h"

#define DEFAULT_LEN 10
#define MULTIPLICITY_FACTOR 10

#ifdef DEBUG
#define OFFSET_ENABLED 1
#else
#define OFFSET_ENABLED 0
#endif //DEBUG

void print_test_result(size_t n, const char* name, bool passed)
{
    printf("TEST %2lu: %-50s [%.6s]\n",
           n,
           name,
           passed ? "PASSED" : "FAILED");
}


bool test_push_until_full(struct circbuf* cb,
                          size_t* written,
                          double* checkbuf)
{
    const size_t capacity = cb_capacity(cb);
    const size_t size = cb_size(cb);
    const size_t empty = size < capacity ? capacity - size : 0;
    *written = 0;

    double el;
    for (size_t i = 0; i < empty; ++i) {
        el = i;
        if (!cb_push(cb, &el)) {
            return false;
        } else {
            checkbuf[*written] = el;
            ++*written;
        }
    }

    ++el; 
    if (cb_push(cb, &el))
        return false;

    return empty == *written;
}


bool test_pop_until_empty(struct circbuf* cb,
                          const size_t* written,
                          const double* checkbuf)
{
    const size_t size = cb_size(cb);

    if (*written != size)
        return false;

    double el;
    for (size_t i = 0; i < size; ++i) {
        if (!cb_pop(cb, &el))
            return false;

        if (el != checkbuf[i])
            return false;
    }

    if (cb_pop(cb, &el))
        return false;

    return !cb_size(cb);
}


bool test_push_then_pop(struct circbuf* cb)
{
    const size_t capacity = cb_capacity(cb);
    const size_t steps = capacity * MULTIPLICITY_FACTOR;

    double el;
    while (cb_size(cb))
        if (!cb_pop(cb, &el))
            return false;

    for (size_t i = 0; i < steps; ++i) {
        el = i;
        if (!cb_push(cb, &el))
            return false;

        if (!cb_pop(cb, &el))
            return false;

        if (el != i)
            return false;

#ifdef DEBUG
        if (cb_head_offset(cb) != cb_tail_offset(cb))
            return false;
#endif //DEBUG
    }

    return true;
}


bool test_multiple_push_then_pop(struct circbuf* cb,
                                 size_t batches,
                                 const size_t* push_n,
                                 const double* push_v,
                                 const size_t* pop_n,
                                 bool* inserted)
{
    size_t push_idx = 0;
    size_t pop_idx = 0;

    for (size_t batch = 0; batch < batches; ++batch)
    {
        // push
        for (size_t i = 0; i < push_n[batch]; ++i) {
            if (cb_push(cb, &push_v[push_idx]))
                inserted[push_idx] = true;
            else
                inserted[push_idx] = false;
        
        ++push_idx;
        }

        // pop
        for (size_t i = 0; i < pop_n[batch]; ++i) {
            double el;
            if (!cb_pop(cb, &el))
                continue;

            if (inserted[pop_idx]) {
                if (el != push_v[pop_idx])
                    return false;
            }

            ++pop_idx;
        }
    }

    return true;
}



int main(int argc, char* argv[])
{
    int retcode = EXIT_FAILURE;

    int len = DEFAULT_LEN;
    if (argc == 2)
        len = atoi(argv[1]);

    double* buffer = (double*)malloc(len * sizeof(double));
    if (!buffer) {
        fprintf(stderr, "Impossible to allocate buffer.\n");
        return retcode;
    }

    double* checkbuf = (double*)malloc(len * sizeof(double));
    if (!checkbuf) {
        fprintf(stderr, "Impossible to allocate checkbuf.\n");
        goto free_buffer;
    }

    size_t written;

    retcode = EXIT_SUCCESS;

    struct circbuf cb;
    cb_init(&cb, buffer, sizeof(double), len);

    printf("circbuf test suite (offset tests %s)\n\n",
            OFFSET_ENABLED ? "ENABLED" : "DISABLED");

    {
        // TEST 0: empty at the beginning
        bool passed = cb_size(&cb) == 0;
#ifdef DEBUG
        passed &= cb_head_offset(&cb) == 0;
        passed &= cb_tail_offset(&cb) == 0;
#endif //DEBUG
        print_test_result(0, "empty at the beginning", passed);
    }

    {
        // TEST 1: full circbuf from empty
        bool ret_test = test_push_until_full(&cb, &written, checkbuf);
        bool ok_written = written == (size_t)len;
        bool ok_inserted = true;
        for (size_t i = 0; i < written; ++i) {
            if (checkbuf[i] != i) {
                ok_inserted = false;
                break;
            }
        }

        bool passed = ret_test && ok_written && ok_inserted;
#ifdef DEBUG
        passed &= cb_head_offset(&cb) == 0;
        passed &= cb_tail_offset(&cb) == 0;
#endif //DEBUG
        print_test_result(1, "full circbuf from empty", passed);
    }

    {
        // TEST 2: empty circbuf from full
        bool passed = test_pop_until_empty(&cb, &written, checkbuf);
#ifdef DEBUG
        passed &= cb_head_offset(&cb) == 0;
        passed &= cb_head_offset(&cb) == 0;
#endif //DEBUG
        print_test_result(2, "empty circbuf from full", passed);
    }

    {
        // TEST 3: push then pop
        bool passed = test_push_then_pop(&cb);
        print_test_result(3, "push then pop", passed);
    }

    {
        // TEST 4: multiple push then pop
        size_t batches = 5;
        size_t push_n[] = {3, 4, 5, 0, 2};
        size_t pop_n[] = {3, 3, 3, 3, 3};
        double push_v[] = { 0.0,  1.0,  2.0,  3.0,  4.0,
                            5.0,  6.0,  7.0,  8.0,  9.0,
                           10.0, 11.0, 12.0, 13.0, 14.0};
        bool inserted[15];

        bool passed = test_multiple_push_then_pop(&cb,
                                                  batches,
                                                  push_n,
                                                  push_v,
                                                  pop_n,
                                                  inserted);
        print_test_result(4, "multiple push then pop", passed);
    }


    cb_unset(&cb);
    free(checkbuf);
free_buffer:
    free(buffer);

    return retcode;
}
