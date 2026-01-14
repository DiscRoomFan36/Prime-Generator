

// #define WE_ARE_USING_BESTED_IN_MAIN

#ifdef WE_ARE_USING_BESTED_IN_MAIN
#    include "Bested.h"
#endif // WE_ARE_USING_BESTED_IN_MAIN


#include "Prime_Generator.h"

// under "Prime_Generator.h" so i can see what it depends on
#include <stdio.h>   // for 'printf()'



// replace the helper's i use
#ifndef WE_ARE_USING_BESTED_IN_MAIN
    #define Array_Len(array)    (sizeof(array) / sizeof(array[0]))

    #define NANOSECONDS_PER_SECOND  (1000UL * 1000UL * 1000UL)

    #include <time.h>

    u64 nanoseconds_since_unspecified_epoch(void) {
    #ifdef __unix__
        struct timespec ts;

        #ifndef CLOCK_MONOTONIC
            #define VSCODE_IS_DUMB___THIS_NEVER_HAPPENS
            #define CLOCK_MONOTONIC 69420
        #endif
        // dont worry, this will never trigger.
        assert(CLOCK_MONOTONIC != 69420);

        clock_gettime(CLOCK_MONOTONIC, &ts);

        return NANOSECONDS_PER_SECOND * ts.tv_sec + ts.tv_nsec;
    #else
        #error "Sorry, haven't implemented this yet."
    #endif
    }

#endif // !WE_ARE_USING_BESTED_IN_MAIN



#ifdef WE_ARE_USING_BESTED_IN_MAIN
    // used for easy memory management.
    global_variable Arena arena = {};

    #define CLEAR_ARENA() Arena_Free(&arena)
#else
    // this is just something we can take
    // a pointer to, it is not used
    static int arena = 0;
    // just do nothing.
    #define CLEAR_ARENA()
#endif // WE_ARE_USING_BESTED_IN_MAIN




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


int main(void) {
    bool test_results[NUM_TESTS] = {};

    // run the tests
    #define X(test, run_test) if (run_test) test_results[TEST_NUMBER(test)] = test();
        TESTS
    #undef X


    // max_text_len for formatting.
    size_t max_text_len = 0;
    #define X(test, ...)  {                                     \
        size_t text_len = strlen(#test);                        \
        if (max_text_len < text_len) max_text_len = text_len;   \
    }
        TESTS
    #undef X

    // display results
    #define X(test, run_test)                               \
        printf("TEST %d: "COLOR_YELLOW"%-*s"COLOR_RESET" - STATUS: %s\n",       \
            TEST_NUMBER(test), (int)max_text_len, #test,    \
            !(run_test) ? COLOR_GRAY"MISSED"COLOR_RESET :   \
                (test_results[TEST_NUMBER(test)] ? COLOR_GREEN"PASSED"COLOR_RESET : COLOR_RED"FAILED"COLOR_RESET)       \
        );

        TESTS
    #undef X


    // cleanup.
    CLEAR_ARENA();
    return 0;
}


// helper function, used with 'nanoseconds_since_unspecified_epoch()'
void print_duration(u64 total_time_in_ns) {
    u64 time_in_ns = (total_time_in_ns                             ) % 1000;
    u64 time_in_us = (total_time_in_ns / (                  1000UL)) % 1000;
    u64 time_in_ms = (total_time_in_ns / (         1000UL * 1000UL)) % 1000;
    u64 time_in_s  = (total_time_in_ns / (1000UL * 1000UL * 1000UL)) % 1000;

    printf("%4lds, %4ldms, %4ldus, %4ldns", time_in_s, time_in_ms, time_in_us, time_in_ns);
}



////////////////////////////////////////////////////
//                 The Tests
////////////////////////////////////////////////////


bool test_get_primes_upto_number(void) {
    Prime_Array primes = {};

    get_primes_upto_number(1000, &primes);
    printf("primes.count = %ld\n", primes.count);

    // this is just the nicest way to do
    // thing with the setup I have.
    free(primes.items);
    return primes.count == 168;
}


bool test_get_get_nth_prime_basics(void) {
    CLEAR_ARENA();
    Prime_Generator generator = { .allocator = &arena };

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

    clear_prime_generator(&generator);
    return flag;
}

bool test_greater_and_greater_powers_of_10(void) {
    CLEAR_ARENA();
    Prime_Generator generator = { .allocator = &arena };


    struct {
        u64 n;
        u64 correct;
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
        clear_prime_generator(&generator);
        CLEAR_ARENA();

        u64 n = pows_of_10[i].n;
        u64 correct = pows_of_10[i].correct;

        u64 start_t = nanoseconds_since_unspecified_epoch();
            u64 prime = get_nth_prime(&generator, n);
        u64 end_t   = nanoseconds_since_unspecified_epoch();

        bool was_correct = (prime == correct);

        printf("    %12ld: %12ld (%s) - time: ", n, prime, was_correct ? "Correct" : "Not Correct");
        print_duration(end_t - start_t);
        printf("\n");

        result &= was_correct;
    }

    clear_prime_generator(&generator);
    return result;
}


bool test_get_all_primes_upto_nth_prime(void) {
    CLEAR_ARENA();
    Prime_Generator generator = { .allocator = &arena };

    // test 'get_all_primes_upto_nth_prime()'
    u64 n = 10;
    Prime_Array arr = get_all_primes_upto_nth_prime(&generator, n);
    printf("Test all primes upto %ld-th prime:\n", n);
    for (size_t i = 0; i < arr.count; i++) {
        printf("    %ld: %4ld\n", i, arr.items[i]);
    }

    printf("get_nth_prime(n) == arr.items[arr.count-1]: %ld == %ld\n", get_nth_prime(&generator, n), arr.items[arr.count-1]);

    clear_prime_generator(&generator);
    return get_nth_prime(&generator, n) == arr.items[arr.count-1];
}

bool test_get_all_primes_under_n(void) {
    CLEAR_ARENA();
    Prime_Generator generator = { .allocator = &arena };

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

    clear_prime_generator(&generator);
    return result;
}




bool test_bench_test(void) {
    CLEAR_ARENA();
    Prime_Generator generator = { .allocator = &arena };


    u64 n = 100000000/2; // takes about 1.4 seconds in release build.
    printf("bench test: n = %ld\n", n);
    for (size_t i = 0; i < 10; i++) {
        // reset the generator every time.
        clear_prime_generator(&generator);
        // free all memory so its a fair test
        CLEAR_ARENA();


        u64 start_t = nanoseconds_since_unspecified_epoch();
            u64 prime = get_nth_prime(&generator, n); // big enough number
        u64 end_t   = nanoseconds_since_unspecified_epoch();

        printf("    Prime: %ld, time: ", prime);
        print_duration(end_t - start_t);
        printf("\n");
    }

    clear_prime_generator(&generator);
    return true;
}





////////////////////////////////////////////
//             final includes
////////////////////////////////////////////

#ifdef WE_ARE_USING_BESTED_IN_MAIN
#    define BESTED_IMPLEMENTATION
#    include "Bested.h"
#endif // WE_ARE_USING_BESTED_IN_MAIN


#define PRIME_GENERATOR_IMPLEMENTATION
#include "Prime_Generator.h"
