//
// Prime_Generator.h - generate a list of primes efficiently.
//
// This is a single header file.
//
// Author   - Fletcher M
//
// Created  - 12/01/26
// Modified - 13/01/26
//
// Make sure to...
//      #define PRIME_GENERATOR_IMPLEMENTATION
//      #include "Prime_Generator.h"
// ...somewhere in your project
//


#ifndef PRIME_GENERATOR_H_
#define PRIME_GENERATOR_H_

// TODO
/*
// if you
//     #include "Bested.h"
// before this file, it will use some things from that library
#ifdef BESTED_H
#    define USING_BESTED true
#else
#    define USING_BESTED false
#endif // BESTED_H
*/

// TODO make IF_USING_BESTED setting...
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
void get_primes_upto_number(u64 n, Prime_Array *result);



/////////////////////////////////////////////////
//            PRIME GENERATOR
/////////////////////////////////////////////////

// do not access any members in this struct please, use the functions.
typedef struct Prime_Generator {
    // the inner array,
    union {
        struct {
            _Array_Header_;
            u64 *items;
        };
        Prime_Array inner_prime_array;
    };

    // used when generating the next block.
    u64 last_prime_checked;
} Prime_Generator;

static_assert(sizeof(Prime_Array) == sizeof(Prime_Generator) - sizeof(u64), "check if the union is doing the right thing.");


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





#endif //  PRIME_GENERATOR_H_





#ifdef PRIME_GENERATOR_IMPLEMENTATION

#ifndef PRIME_GENERATOR_IMPLEMENTATION_GUARD_
#define PRIME_GENERATOR_IMPLEMENTATION_GUARD_



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
// returns the number of added primes, maybe that will be useful someday.
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

Prime_Array get_all_primes_upto_nth_prime(Prime_Generator *prime_generator, u64 n) {
    ASSERT(prime_generator);
    ASSERT(n != 0 && "this function is 1 indexed");

    generate_primes_until_nth_prime(prime_generator, n);

    Prime_Array result = prime_generator->inner_prime_array;
    result.count = n;
    return result;
}

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
    ASSERT(low+1 >= result.count || result.items[low+1] >= n);

    result.count = low;
    return result;
}




#endif // PRIME_GENERATOR_IMPLEMENTATION_GUARD_

#endif //  PRIME_GENERATOR_IMPLEMENTATION

