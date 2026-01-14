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



// if you
//     #include "Bested.h"
// before this file, it will use some things from that library
#ifdef BESTED_H
    #define USING_BESTED_H true
#else
    #define USING_BESTED_H false
#endif // BESTED_H



#include <stdint.h>     // for 'uint64_t'
#include <stdbool.h>    // for 'bool'


//
// feel free to redefine 'PRIME_GENERATOR_REALLOC',
//
// just make sure to redefine 'PRIME_GENERATOR_FREE' as well
//
#ifndef PRIME_GENERATOR_REALLOC
    #include <stdlib.h>
    #define PRIME_GENERATOR_REALLOC(ptr, old_size, new_size) realloc((ptr), (new_size))
    #define PRIME_GENERATOR_FREE(ptr, old_size) free(ptr)
#else
    #ifndef PRIME_GENERATOR_FREE
        #error "you must also define 'PRIME_GENERATOR_FREE' if you define 'PRIME_GENERATOR_REALLOC'"
    #endif
#endif // PRIME_GENERATOR_REALLOC

//
// if you replace this with your own thing,
// make sure you abort or whatever after.
//
// or else spoooooooky undefined behaviour will kick your ass
//
#ifndef PRIME_GENERATOR_ASSERT
    #include <assert.h>
    #define PRIME_GENERATOR_ASSERT(expr) assert(expr)
#endif // PRIME_GENERATOR_ASSERT


// this is a way to make static_assert without libc, kinda annoyed at this.
#define PRIME_GENERATOR_STATIC_ASSERT(expr, message)   _Static_assert(expr, message)

//
// this relies on memset, witch comes from string.h,
// witch is part of libc, so you can remove libc if you want.
//
#ifndef PRIME_GENERATOR_MEM_ZERO
    #include <string.h>
    // dose what you expect it dose.
    #define PRIME_GENERATOR_MEM_ZERO(ptr, size) memset((ptr), 0, (size))
#endif // PRIME_GENERATOR_MEM_ZERO



// this is just better, also typedefs dont cause warnings. :)
typedef uint64_t u64;

// an array of primes, may change if your USING_BESTED_H.
// always contains:
//   u64 count;  // number of primes,
//   u64 *items; // array of primes
typedef struct Prime_Array Prime_Array;


struct Prime_Array {
    #if USING_BESTED_H
        _Array_Header_;
        u64 *items;
    #else
        u64 count;
        // NOTE we could try to remove this, as 'get_primes_upto_number()'
        // doesn't really need to know its own capacity...
        u64 capacity;
        u64 *items;
    #endif // USING_BESTED_H
};


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

//
// A Prime Number Generator,
//
// zero initialize struct and then begin calling functions on it.
//
// ```c
//     // construct a generator.
//     Prime_Generator generator = {};
//
//     // or construct with an allocator (uses Bested.h arenas)
//     Prime_Generator generator = { .allocator = allocator };
//
//
//     // use it, might have to generate all the primes upto it,
//     // but its pretty fast, and subsequent calls will use cache'd results.
//     u64 prime = get_nth_prime(&generator, 420);
//
//     // clear it (free's backing array) (will save allocator if provided)
//     // (you don't have to do this if you use an allocator, but its
//     // a nice convenience function)
//     clear_prime_generator(&generator);
// ```
//
// please do not access any members in this struct please, use the functions.
//
typedef struct Prime_Generator Prime_Generator;

struct Prime_Generator {

    // the inner array, made this way so its easy to assign an allocator
    #if USING_BESTED_H
        union {
            struct {
                _Array_Header_;
                u64 *items;
            };
            Prime_Array inner_prime_array;
        };
    #else // !USING_BESTED_H
        // TODO this union is dumb
        union {
            struct {
                u64 count;
                u64 capacity;
                u64 *items;
            };
            Prime_Array inner_prime_array;
        };

        // this pointer is not used, it is just for ease of
        // conversion away from Bested.h
        void *allocator;
    #endif // USING_BESTED_H


    // used when generating the next block.
    u64 last_prime_checked;
};


// only for bested.h for allocator alignment reasons
#if USING_BESTED_H
    PRIME_GENERATOR_STATIC_ASSERT(sizeof(Prime_Array) == sizeof(Prime_Generator) - sizeof(u64), "check if the union is doing the right thing.");
#endif // USING_BESTED_H



/////////////////////////////////////////////////
//         PRIME GENERATOR INTERFACE
/////////////////////////////////////////////////

// free's arrays, keeps allocator if provided.
void clear_prime_generator(Prime_Generator *prime_generator);

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


// marks a functions as belonging to this header file only.
#define Prime_Generator_Internal     static



#endif //  PRIME_GENERATOR_H_



#define PRIME_GENERATOR_IMPLEMENTATION

#ifdef PRIME_GENERATOR_IMPLEMENTATION

#ifndef PRIME_GENERATOR_IMPLEMENTATION_GUARD_
#define PRIME_GENERATOR_IMPLEMENTATION_GUARD_


Prime_Generator_Internal void Prime_Array_Append(Prime_Array *array, u64 n) {
    PRIME_GENERATOR_ASSERT(array && "Tried to append to NULL pointer...");

    #if USING_BESTED_H
        // just use Bested.h's one.
        Array_Append(array, n);
    #else
        if (array->count >= array->capacity) {
            // manually allocate
            u64 new_capacity = array->capacity != 0 ? array->capacity * 2 : 32; // double or otherwise 32
            array->items = PRIME_GENERATOR_REALLOC(array->items, array->capacity*sizeof(array->items[0]), new_capacity*sizeof(array->items[0]));
            PRIME_GENERATOR_ASSERT(array->items && "You ran out of memory, how many primes did you just try to make?");
            array->capacity = new_capacity;
        }
        array->items[array->count++] = n;
    #endif // USING_BESTED_H
}



// using the Sieve of Eratosthenes
// Wikipedia: https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes
//
// will append to the array provided, this function cannot be restarted,
// and is slower then the Prime_Generator.
//
// for any significant n, use Prime_Generator
void get_primes_upto_number(u64 n, Prime_Array *result) {
    PRIME_GENERATOR_ASSERT(result && "must pass in a valid result array, got NULL");
    if (n < 2) { return; }


    // TODO this will fail with bigger n, it will run out of stack space
    //
    // TODO make this not have +1,
    bool is_not_prime_array[n+1];
    PRIME_GENERATOR_MEM_ZERO(is_not_prime_array, sizeof(is_not_prime_array));

    // these are not prime.
    is_not_prime_array[0] = true;
    is_not_prime_array[1] = true;

    // special case 2
    for (size_t i = 2*2; i <= n; i += 2) is_not_prime_array[i] = true;

    u64 current_prime = 3;
    while (true) {
        // look for next prime
        for (; current_prime <= n; current_prime++) {
            // if the number is still false, its a prime
            if (!is_not_prime_array[current_prime]) { break; }
        }
        if (current_prime > n) break;

        // set everything thats a multiple of this number to 'not a prime'
        //
        // go 2 at a time because even numbers are allready done.
        for (u64 i = current_prime*3; i <= n; i += current_prime*2) { is_not_prime_array[i] = true; }

        // repeat process, look for next prime
        current_prime += 1;
    }

    // TODO we could use this to perfectly allocate a buffer.
    // u64 prime_count = 0;
    // for (size_t i = 0; i <= n; i++) {
    //     if (!is_not_prime_array[i]) prime_count += 1;
    // }

    for (size_t i = 0; i <= n; i++) {
        if (is_not_prime_array[i]) continue;

        Prime_Array_Append(result, i);
    }
    PRIME_GENERATOR_ASSERT(result->items[0] == 2);
}




// https://en.wikipedia.org/wiki/Integer_square_root
//
// this is pretty fast, but there is a functions that works even faster when provided with a good guess,
// we could guess the actual result the last time we called this,
//
// but i do not think this is the slow part of the '__generate_prime_block()' function.
// the slow part is makein a 8GB array...
Prime_Generator_Internal u64 int_sqrt(u64 n) {
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
Prime_Generator_Internal u64 __generate_prime_block(Prime_Generator *prime_generator) {
    // did a couple of bench tests, bigger number is better here.
    //
    // will make the inital get_primes_upto_number() slower,
    // but that takes < 1ms, so ehh.
    #define PRIME_GENERATOR_BLOCK_SIZE    (1 << 16)

    PRIME_GENERATOR_ASSERT(prime_generator->last_prime_checked % PRIME_GENERATOR_BLOCK_SIZE == 0 && "dont mess with my innards, last_prime_checked was not a multiple of PRIME_GENERATOR_BLOCK_SIZE");

    if (prime_generator->last_prime_checked >= (1UL << 60)) {
        PRIME_GENERATOR_ASSERT(false && "This is getting a little out of hand.");
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
    PRIME_GENERATOR_ASSERT(prime_generator->inner_prime_array.items[0] == 2);


    // remove all even cells with /2, by definition.
    bool is_not_prime_array[PRIME_GENERATOR_BLOCK_SIZE/2];
    PRIME_GENERATOR_MEM_ZERO(is_not_prime_array, sizeof(is_not_prime_array));
    // Mem_Zero(is_not_prime_array, sizeof(is_not_prime_array));

    // do the primes we have
    u64 sqrt_of_ending = int_sqrt(prime_generator->last_prime_checked + PRIME_GENERATOR_BLOCK_SIZE);
    // start from 1, we remove all even cells, so we dont need 2
    for (size_t i = 1; i < prime_generator->inner_prime_array.count; i++) {
        u64 prime = prime_generator->inner_prime_array.items[i];
        if (prime > sqrt_of_ending) break;

        // do a bunch of math to figure of what position we start at in the array
        u64 div_ceil = (prime_generator->last_prime_checked + prime - 1) / prime;
        u64 normal_start = div_ceil * prime;
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
        Prime_Array_Append(&prime_generator->inner_prime_array, new_prime);

        number_of_primes_this_round += 1;
    }

    prime_generator->last_prime_checked += PRIME_GENERATOR_BLOCK_SIZE;
    return number_of_primes_this_round;
}





void clear_prime_generator(Prime_Generator *prime_generator) {
    // this will always exist, and we will always preseve it.
    void *allocator = prime_generator->allocator;

    #if USING_BESTED_H
        // free malloc'd array if not allocator
        if (!allocator) Array_Free(&prime_generator->inner_prime_array);
    #else
        // here we always free the array, even when an allocator exists.
        //
        // because we know 100% that this array was allocated with PRIME_GENERATOR_REALLOC or whatever
        PRIME_GENERATOR_FREE(prime_generator->inner_prime_array.items, prime_generator->inner_prime_array.capacity);

    #endif // USING_BESTED_H

    PRIME_GENERATOR_MEM_ZERO(prime_generator, sizeof(*prime_generator));
    prime_generator->allocator = allocator;
}


// 1 indexed
u64 get_nth_prime(Prime_Generator *prime_generator, u64 n) {
    PRIME_GENERATOR_ASSERT(prime_generator);
    PRIME_GENERATOR_ASSERT(n != 0 && "this function is 1 indexed");
    generate_primes_until_nth_prime(prime_generator, n);
    return prime_generator->inner_prime_array.items[n-1];
}


void generate_primes_under_n(Prime_Generator *prime_generator, u64 n) {
    PRIME_GENERATOR_ASSERT(prime_generator);
    while (prime_generator->last_prime_checked < n)       { __generate_prime_block(prime_generator); }
}

void generate_primes_until_nth_prime(Prime_Generator *prime_generator, u64 n) {
    PRIME_GENERATOR_ASSERT(prime_generator);
    PRIME_GENERATOR_ASSERT(n != 0 && "this function is 1 indexed");

    #if USING_BESTED_H
        // reserve amount needed so we dont have to reallocate.
        Array_Reserve(&prime_generator->inner_prime_array, n);
    #else
        // we could do something here, but im lazy.
    #endif // USING_BESTED_H

    u64 index = n - 1;

    while (prime_generator->inner_prime_array.count <= index) { __generate_prime_block(prime_generator); }
}

Prime_Array get_all_primes_upto_nth_prime(Prime_Generator *prime_generator, u64 n) {
    PRIME_GENERATOR_ASSERT(prime_generator);
    PRIME_GENERATOR_ASSERT(n != 0 && "this function is 1 indexed");

    generate_primes_until_nth_prime(prime_generator, n);

    Prime_Array result = prime_generator->inner_prime_array;
    result.count = n;
    return result;
}

Prime_Array get_all_primes_under_n(Prime_Generator *prime_generator, u64 n) {
    PRIME_GENERATOR_ASSERT(prime_generator);

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
    PRIME_GENERATOR_ASSERT(low+1 >= result.count || result.items[low+1] >= n);

    result.count = low;
    return result;
}




#endif // PRIME_GENERATOR_IMPLEMENTATION_GUARD_

#endif //  PRIME_GENERATOR_IMPLEMENTATION

