
#include <string.h> // for strlen

// #include "Bested.h" this is in Prime_Generator

#include "Prime_Generator.h"


// tests, (1 == run, 0 == dont run)
#define TESTS                                   \
    X(test_get_primes_upto_number,           1) \
    X(test_get_get_nth_prime_basics,         1) \
    X(test_greater_and_greater_powers_of_10, 1) \
    X(test_get_all_primes_upto_nth_prime,    1) \
    X(test_get_all_primes_under_n,           1) \
                                                \
    X(test_bench_test,                       1)


// predefine tests
#define X(test, ...) bool test(void);
    TESTS
#undef X

// calculate the number of tests, also number them.
enum {
    #define X(test, ...) __enum__##test,
        TESTS
    #undef X

    NUM_TESTS,
};
#define TEST_NUMBER(test) (__enum__##test)



// terminal color codes.
#define COLOR_RESET     "\033[1;0m"
#define COLOR_GREEN     "\033[1;32m"
#define COLOR_RED       "\033[1;31m"
#define COLOR_YELLOW    "\033[1;33m"
#define COLOR_GRAY      "\033[1;90m"


// used for easy memory management.
global_variable Arena arena = {};

int main(void) {
    bool test_results[NUM_TESTS] = {};

    // run the tests
    #define X(test, run_test) if (run_test) test_results[TEST_NUMBER(test)] = test();
        TESTS
    #undef X


    // max_text_len for formatting.
    size_t max_text_len = 0;
    #define X(test, ...) max_text_len = Max(max_text_len, strlen(#test));
        TESTS
    #undef X

    // display results
    #define X(test, run_test)                               \
        printf("TEST %d: "COLOR_YELLOW"%-*s"COLOR_RESET" - STATUS: %s\n",              \
            TEST_NUMBER(test), (int)max_text_len, #test,    \
            !(run_test) ? COLOR_GRAY"MISSED"COLOR_RESET :   \
                (test_results[TEST_NUMBER(test)] ? COLOR_GREEN"PASSED"COLOR_RESET : COLOR_RED"FAILED"COLOR_RESET)       \
        );

        TESTS
    #undef X


    // cleanup.
    Arena_Free(&arena);
    return 0;
}



////////////////////////////////////////////////////
//                 The Tests
////////////////////////////////////////////////////


bool test_get_primes_upto_number(void) {
    Arena_Free(&arena); // get a clean slate

    Prime_Array primes = { .allocator = &arena };

    get_primes_upto_number(1000, &primes);
    printf("primes.count = %ld\n", primes.count);

    // Array_Free(&primes);
    return primes.count == 168;
}

bool test_get_get_nth_prime_basics(void) {
    Arena_Free(&arena); // get a clean slate

    // TODO make this easier, maybe have prime generator *be* a
    // dynamic array... so generator.allocator works
    Prime_Generator generator = { .inner_prime_array.allocator = &arena };

    // warning calls the functions twice
    #define DEBUG_THEN_ASSERT(a, b) do { debug(a); ASSERT((a) == (b)); } while (0)

    struct {
        u64 n; u64 correct;
    } tests[] = {
        {1,  2},
        {2,  3},
        {3,  5},
        {4,  7},
        {5, 11},
    };

    bool flag = true;
    printf("test get_get_nth_prime basics:\n");
    for (size_t i = 0; i < Array_Len(tests); i++) {
        u64 n = tests[i].n;
        u64 correct = tests[i].correct;

        u64 result = get_nth_prime(&generator, n);
        printf("    get_nth_prime(%2ld) = %2ld (%s)\n", n, result, (result == correct) ? "Correct" : "Not Correct");

        if (result != correct) flag = false;
    }

    reset_prime_generator(&generator);
    return flag;
}

bool test_greater_and_greater_powers_of_10(void) {
    Arena_Free(&arena); // get a clean slate

    // TODO make this easier, maybe have prime generator *be* a
    // dynamic array... so generator.allocator works
    Prime_Generator generator = { .inner_prime_array.allocator = &arena };


    struct {
        u64 n;
        u64 prime;
    } pows_of_10[] = {
        {        1,          2},
        {       10,         29},
        {      100,        541},
        {     1000,       7919},
        {    10000,     104729},
        {   100000,    1299709},
        {  1000000,   15485863},
        { 10000000,  179424673},
        {100000000, 2038074743}, // 2.968 seconds is my best time. might crash vscode.
    };

    bool result = true;

    printf("testing greater and greater powers of 10:\n");
    for (size_t i = 0; i < Array_Len(pows_of_10); i++) {
        // reset the generator every time.
        reset_prime_generator(&generator);
        Arena_Clear(&arena);

        u64 n = pows_of_10[i].n;
        u64 real_prime = pows_of_10[i].prime;

        u64 start_t = nanoseconds_since_unspecified_epoch();
            u64 prime = get_nth_prime(&generator, n);
        u64 end_t   = nanoseconds_since_unspecified_epoch();

        u64 total_time = end_t - start_t;

        u64 time_in_ns = (total_time           ) % 1000;
        u64 time_in_us = (total_time / THOUSAND) % 1000;
        u64 time_in_ms = (total_time / MILLION ) % 1000;
        u64 time_in_s  = (total_time / BILLION ) % 1000;
        const char *time = temp_sprintf("%4lds, %4ldms, %4ldus, %4ldns", time_in_s, time_in_ms, time_in_us, time_in_ns);

        bool correct = (prime == real_prime);
        printf("    %12ld: %12ld (%s) - time: %s\n", n, prime, correct ? "Correct" : "Not Correct", time);

        result &= correct;
    }

    reset_prime_generator(&generator);
    return result;
}

bool test_get_all_primes_upto_nth_prime(void) {
    Arena_Free(&arena); // get a clean slate

    // TODO make this easier, maybe have prime generator *be* a
    // dynamic array... so generator.allocator works
    Prime_Generator generator = { .inner_prime_array.allocator = &arena };

    // test 'get_all_primes_upto_nth_prime()'
    u64 n = 10;
    Prime_Array arr = get_all_primes_upto_nth_prime(&generator, n);
    printf("Test all primes upto %ld-th prime:\n", n);
    for (size_t i = 0; i < arr.count; i++) {
        printf("    %ld: %4ld\n", i, arr.items[i]);
    }

    debug(arr.count);
    reset_prime_generator(&generator);
    return arr.count == 10;
}

bool test_get_all_primes_under_n(void) {
    Arena_Free(&arena); // get a clean slate

    // TODO make this easier, maybe have prime generator *be* a
    // dynamic array... so generator.allocator works
    Prime_Generator generator = { .inner_prime_array.allocator = &arena };

    bool result = true;

    {
        u64 n = 100;
        printf("test get_all_primes_under_n(%ld):\n", n);
        Prime_Array arr = get_all_primes_under_n(&generator, n);

        for (size_t i = 0; i < arr.count; i++) {
            printf("    arr[%ld] = %ld\n", i, arr.items[i]);
        }

        u64 next_prime = get_nth_prime(&generator, arr.count+1);
        printf("get_nth_prime(arr.count+1) = %ld\n", next_prime);

        result &= (arr.items[arr.count-1] < n && n <= next_prime);
    }


    {
        u64 n = 11;
        Prime_Array arr = get_all_primes_under_n(&generator, n);
        printf("testing when n is prime:\n");
        for (size_t i = 0; i < arr.count; i++) {
            printf("    arr2[%ld] = %ld\n", i, arr.items[i]);
        }

        u64 next_prime = get_nth_prime(&generator, arr.count+1);
        printf("get_nth_prime(arr.count+1) = %ld\n", next_prime);
        result &= arr.items[arr.count-1] < n && n <= next_prime && next_prime == n;
    }

    reset_prime_generator(&generator);
    return result;
}




bool test_bench_test(void) {
    Arena_Free(&arena); // get a clean slate

    // TODO make this easier, maybe have prime generator *be* a
    // dynamic array... so generator.allocator works
    Prime_Generator generator = { .inner_prime_array.allocator = &arena };


    u64 n = 100000000/2; // takes about 1.4 seconds in release build.
    printf("bench test: n = %ld\n", n);
    for (size_t i = 0; i < 10; i++) {
        // reset the generator every time.
        reset_prime_generator(&generator);
        // free all memory so its a fair test
        Arena_Free(&arena);


        u64 start_t = nanoseconds_since_unspecified_epoch();
            get_nth_prime(&generator, n); // big enough number
        u64 end_t   = nanoseconds_since_unspecified_epoch();

        u64 total_time = end_t - start_t;

        u64 time_in_ns = (total_time           ) % 1000;
        u64 time_in_us = (total_time / THOUSAND) % 1000;
        u64 time_in_ms = (total_time / MILLION ) % 1000;
        u64 time_in_s  = (total_time / BILLION ) % 1000;
        const char *time = temp_sprintf("%4lds, %4ldms, %4ldus, %4ldns", time_in_s, time_in_ms, time_in_us, time_in_ns);

        printf("    time: %s\n", time);
    }

    reset_prime_generator(&generator);
    return true;
}







////////////////////////////////////////////
//             final includes
////////////////////////////////////////////

#define BESTED_IMPLEMENTATION
#include "Bested.h"


#define PRIME_GENERATOR_IMPLEMENTATION
#include "Prime_Generator.h"
