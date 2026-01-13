
#include "Bested.h"



typedef struct {
    _Array_Header_;
    u64 *items;
} Prime_Array;


// using the Sieve of Eratosthenes
// Wikipedia: https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes
//
// will append to the array provided, this function cannot be restarted,
// and is slower then the Prime_Generator.
//
// for any significant n, use Prime_Generator
void get_primes_upto_number(u64 n, Prime_Array *result) {
    ASSERT(result && "must pass in a valid result array, got NULL");
    if (n < 2) { return; }


    // TODO this will fail with bigger n, it will run out of stack space
    bool is_not_prime_array[n+1];
    Mem_Zero(is_not_prime_array, sizeof(is_not_prime_array));

    // special case 2
    Array_Append(result, 2);
    for (size_t i = 2; i <= n; i += 2) is_not_prime_array[i] = true;

    u64 current_prime = 3;
    while (true) {
        // look for next prime
        for (; current_prime <= n; current_prime++) {
            // if the number is still false, its a prime
            if (!is_not_prime_array[current_prime]) { break; }
        }
        if (current_prime > n) break;

        // add to the result array
        Array_Append(result, current_prime);

        // set everything thats a multiple of this number to 'not a prime'
        //
        // go 2 at a time because even numbers are allready done.
        for (u64 i = current_prime*3; i <= n; i += current_prime*2) { is_not_prime_array[i] = true; }

        // repeat process, look for next prime
        current_prime += 1;
    }
}




// do not access any members in this struct please, use the functions.
typedef struct Prime_Generator {
    Prime_Array inner_prime_array;
    // used when generating the next block.
    u64 last_prime_checked;
} Prime_Generator;


// https://en.wikipedia.org/wiki/Integer_square_root
//
// this is pretty fast, but there is a functions that works even faster when provided with a good guess,
// we could guess the actual result the last time we called this,
//
// but i do not think this is the slow part of the '__generate_prime_block()' function.
// the slow part is makein a 8GB array...
internal u64 int_sqrt(u64 n) {
    u64 L = 1, R = n;
    while (L < R) {
        R = L + ((R - L) / 2);
        L = n / R;
    }
    return R;
}

// private function, generates the next block of primes.
//
// returns the number of added primes, maybe it will be useful someday.
internal u64 __generate_prime_block(Prime_Generator *prime_generator) {
    // did a couple of bench tests, bigger number is better here.
    //
    // will make the inital get_primes_upto_number() slower,
    // but that takes at most 150us, so ehh.
    #define PRIME_GENERATOR_BLOCK_SIZE    (1 << 16)

    ASSERT(prime_generator->last_prime_checked % PRIME_GENERATOR_BLOCK_SIZE == 0 && "dont mess with my innards, last_prime_checked was not a multiple of PRIME_GENERATOR_BLOCK_SIZE");

    if (prime_generator->last_prime_checked >= (1UL << 60)) {
        PANIC("This is getting a little out of hand.");
    }

    if (prime_generator->last_prime_checked == 0) {
        // we special case 0, because it has 2 as the first prime,
        // and we can do some optimizations knowing we have 2.
        //
        // also knowing all numbers in the block cannot effect the block itself is good.
        // 
        // so we just use a simple Sieve, im a little
        // worried this might be slow if we crank the block size
        get_primes_upto_number(PRIME_GENERATOR_BLOCK_SIZE, &prime_generator->inner_prime_array);
        prime_generator->last_prime_checked = PRIME_GENERATOR_BLOCK_SIZE;
        return prime_generator->inner_prime_array.count;
    }

    // we *know* that we have 2 so lets do a little optimization.
    ASSERT(prime_generator->inner_prime_array.items[0] == 2);


    // remove all even cells with /2, by definition.
    bool is_not_prime_array[PRIME_GENERATOR_BLOCK_SIZE/2];
    Mem_Zero(is_not_prime_array, sizeof(is_not_prime_array));

    // do the primes we have
    u64 sqrt_of_ending = int_sqrt(prime_generator->last_prime_checked + PRIME_GENERATOR_BLOCK_SIZE);
    // start from 1, we remove all even cells, so we dont need 2
    for (size_t i = 1; i < prime_generator->inner_prime_array.count; i++) {
        u64 prime = prime_generator->inner_prime_array.items[i];
        if (prime > sqrt_of_ending) break;

        // do a bunch of math to figure of what position we start at in the array
        u64 normal_start = Div_Ceil(prime_generator->last_prime_checked, prime) * prime;
        u64 regular_array_start = normal_start - prime_generator->last_prime_checked;
        // make sure its not even, we removed all of those cells.
        if (regular_array_start % 2 == 0) { regular_array_start += prime; }
        u64 start = regular_array_start / 2;

        // technically were iterating by 'prime * 2 / 2'
        //     / 2 because we removed the even cells
        //     * 2 because all multiples of 2 are gone. and we dont need to check them.
        for (u64 j = start; j < PRIME_GENERATOR_BLOCK_SIZE/2; j += prime) {
            is_not_prime_array[j] = true;
        }
    }

    // none of the numbers in this block can effect each other,
    // (because we did a first block, and any numbers in here
    // are bigger than PRIME_GENERATOR_BLOCK_SIZE)
    u64 number_of_primes_this_round = 0;
    for (size_t i = 0; i < PRIME_GENERATOR_BLOCK_SIZE/2; i++) {
        if (is_not_prime_array[i]) continue;
        // i*2+1 to account for the array.
        u64 new_prime = prime_generator->last_prime_checked + (i*2+1);
        // add it to the list
        Array_Append(&prime_generator->inner_prime_array, new_prime);

        number_of_primes_this_round += 1;
    }

    prime_generator->last_prime_checked += PRIME_GENERATOR_BLOCK_SIZE;
    return number_of_primes_this_round;
}

/////////////////////////////////////////////////
//         PRIME GENERATOR INTERFACE
/////////////////////////////////////////////////

// convience function for testing. keeps the provided allocator. (not the array)
void reset_prime_generator(Prime_Generator *prime_generator);

// 1 indexed
u64 get_nth_prime(Prime_Generator *prime_generator, u64 n);

// may generate more than primes under n, performance reasons
void generate_primes_under_n(Prime_Generator *prime_generator, u64 n);
// may generate more than n primes, performance reasons
void generate_primes_until_nth_prime(Prime_Generator *prime_generator, u64 n);


// returns an array with all primes upto the nth prime.
//
// *WARNING* do not mess with the items in this array,
// this is the real backing array for the generator,
// and messing with it will have unintended behaviour on future calls to these functions.
Prime_Array get_all_primes_upto_nth_prime(Prime_Generator *prime_generator, u64 n);

// returns an array with all primes under n
//
// *WARNING* do not mess with the items in this array,
// this is the real backing array for the generator,
// and messing with it will have unintended behaviour on future calls to these functions.
Prime_Array get_all_primes_under_n(Prime_Generator *prime_generator, u64 n);





void reset_prime_generator(Prime_Generator *prime_generator) {
    Arena *allocator = prime_generator->inner_prime_array.allocator;

    // free malloc'd array.
    if (!allocator) Array_Free(&prime_generator->inner_prime_array);

    Mem_Zero(prime_generator, sizeof(*prime_generator));
    prime_generator->inner_prime_array.allocator = allocator;
}


// 1 indexed
u64 get_nth_prime(Prime_Generator *prime_generator, u64 n) {
    ASSERT(prime_generator);
    ASSERT(n != 0 && "this function is 1 indexed");
    generate_primes_until_nth_prime(prime_generator, n);
    return prime_generator->inner_prime_array.items[n-1];
}


void generate_primes_under_n(Prime_Generator *prime_generator, u64 n) {
    ASSERT(prime_generator);
    while (prime_generator->last_prime_checked < n)       { __generate_prime_block(prime_generator); }
}

void generate_primes_until_nth_prime(Prime_Generator *prime_generator, u64 n) {
    ASSERT(prime_generator);
    ASSERT(n != 0 && "this function is 1 indexed");
    u64 index = n - 1;
    while (prime_generator->inner_prime_array.count <= index) { __generate_prime_block(prime_generator); }
}

// TODO test
Prime_Array get_all_primes_upto_nth_prime(Prime_Generator *prime_generator, u64 n) {
    ASSERT(prime_generator);
    ASSERT(n != 0 && "this function is 1 indexed");

    generate_primes_until_nth_prime(prime_generator, n);

    Prime_Array result = prime_generator->inner_prime_array;
    result.count = n;
    return result;
}

// TODO test
Prime_Array get_all_primes_under_n(Prime_Generator *prime_generator, u64 n) {
    ASSERT(prime_generator);

    generate_primes_under_n(prime_generator, n);

    Prime_Array result = prime_generator->inner_prime_array;

    // binary search to fund biggest prime smaller than n
    u64 low = 0;
    u64 high = result.count-1;

    while (low < high) {
        // if this tries to overflow, you computer will allready be at 0.0001 FPS
        u64 mid = (low + high) / 2;
        if      (result.items[mid] < n) low  = mid + 1;
        else if (result.items[mid] > n) high = mid - 1;
        else                            break;
    }
    // ASSERT(result.items[low] < n);
    ASSERT(low+1 >= result.count || result.items[low+1] >= n);

    result.count = low;
    return result;
}


// used for easy memory management.
global_variable Arena arena = {};


// tests, 1 == run, 0 == dont run
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


enum {
    #define X(test, ...) __enum__##test,
        TESTS
    #undef X

    NUM_TESTS,
};

#define TEST_NUMBER(test) (__enum__##test)

#include <string.h>

int main(void) {
    bool test_results[NUM_TESTS] = {};

    // run the tests
    #define X(test, run_test) if (run_test) test_results[TEST_NUMBER(test)] = test();
        TESTS
    #undef X


    size_t max_text_len = 0;
    #define X(test, ...) max_text_len = Max(max_text_len, strlen(#test));
        TESTS
    #undef X

    #define COLOR_RESET     "\033[1;0m"
    #define COLOR_GREEN     "\033[1;32m"
    #define COLOR_RED       "\033[1;31m"
    #define COLOR_YELLOW    "\033[1;33m"
    #define COLOR_GRAY      "\033[1;90m"

    // display results
    #define X(test, run_test)                               \
        printf("TEST %d: "COLOR_YELLOW"%-*s"COLOR_RESET" - STATUS: %s\n",              \
            TEST_NUMBER(test), (int)max_text_len, #test,    \
            !(run_test) ? COLOR_GRAY"MISSED"COLOR_RESET :   \
                (test_results[TEST_NUMBER(test)] ? COLOR_GREEN"PASSED"COLOR_RESET : COLOR_RED"FAILED"COLOR_RESET)       \
        );

        TESTS
    #undef X

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

    return result;
}

bool test_bench_test(void) {
    Arena_Free(&arena); // get a clean slate

    // TODO make this easier, maybe have prime generator *be* a
    // dynamic array... so generator.allocator works
    Prime_Generator generator = { .inner_prime_array.allocator = &arena };


    u64 n = 100000000/2;
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

    return true;
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

    return result;
}









////////////////////////////////////////////
//             final includes
////////////////////////////////////////////

#define BESTED_IMPLEMENTATION
#include "Bested.h"

