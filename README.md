
# Prime Generator

Generate a list of primes up to n or until the *n*th number.

Allows for fast search for the *n*th prime.

## Features

- Available in a Single Header Library
  - just drag the file into your project and run.
    - optionally you can include `Bested.h`, (my suite of c helper tools), ***BEFORE*** `Prime_Generator.h`, and get access to arena allocators that work with the Prime_Generator struct to help manage memory.
    - NOTE: if you include it after, it behaves as if you never included it.

- Can generate all primes up to the ***100,000,000***th in just `2.66` seconds!
  - how many more primes do you really need?

## Quick Start

```bash
# compile the build script once.
$ cc -o nob nob.c

# compile and run tests
$ ./nob release && ./build/main_release
```

## How to use

```c
// construct a generator.
Prime_Generator generator = {};

// or construct with an allocator (uses Bested.h arenas) (only useable if you include `Bested.h` *BEFORE* `Prime_Generator.h`)
Prime_Generator generator = { .allocator = allocator };


// use it, might have to generate all the primes up to *n*,
// but its pretty fast, and subsequent calls will use cache'd results.
u64 prime = get_nth_prime(&generator, 420);

// clear it (free's backing array) (will save allocator if provided)
// (you don't have to do this if you use an allocator, but its
// a nice convenience function)
clear_prime_generator(&generator);
```


## Expected Output

```console
$ ./nob release && ./build/main_release

# ignore this line
[INFO] copying ~/Programming/C-things/Bested.h/Bested.h -> ./thirdparty/Bested.h

[INFO] directory `./build/` already exists
[INFO] CMD: clang -std=gnu11 -Wall -Wextra -Wno-initializer-overrides -I./thirdparty/ -O2 -o ./build/main_release ./src/main.c
primes.count = 168
test get_get_nth_prime basics:
    get_nth_prime( 1) =  2 (Correct)
    get_nth_prime( 2) =  3 (Correct)
    get_nth_prime( 3) =  5 (Correct)
    get_nth_prime( 4) =  7 (Correct)
    get_nth_prime( 5) = 11 (Correct)
testing greater and greater powers of 10:
               1:            2 (Correct) - time:    0s,    0ms,  171us,  445ns
              10:           29 (Correct) - time:    0s,    0ms,  156us,  243ns
             100:          541 (Correct) - time:    0s,    0ms,  153us,  509ns
            1000:         7919 (Correct) - time:    0s,    0ms,  151us,  696ns
           10000:       104729 (Correct) - time:    0s,    0ms,  286us,  902ns
          100000:      1299709 (Correct) - time:    0s,    2ms,  337us,  209ns
         1000000:     15485863 (Correct) - time:    0s,   24ms,   46us,   56ns
        10000000:    179424673 (Correct) - time:    0s,  262ms,   42us,  858ns
       100000000:   2038074743 (Correct) - time:    2s,  911ms,  770us,  567ns
Test all primes upto 10-th prime:
    0:    2
    1:    3
    2:    5
    3:    7
    4:   11
    5:   13
    6:   17
    7:   19
    8:   23
    9:   29
DEBUG: arr.count = 10
test get_all_primes_under_n(100):
    arr[0] = 2
    arr[1] = 3
    arr[2] = 5
    arr[3] = 7
    arr[4] = 11
    arr[5] = 13
    arr[6] = 17
    arr[7] = 19
    arr[8] = 23
    arr[9] = 29
    arr[10] = 31
    arr[11] = 37
    arr[12] = 41
    arr[13] = 43
    arr[14] = 47
    arr[15] = 53
    arr[16] = 59
    arr[17] = 61
    arr[18] = 67
    arr[19] = 71
    arr[20] = 73
    arr[21] = 79
    arr[22] = 83
    arr[23] = 89
    arr[24] = 97
get_nth_prime(arr.count+1) = 101
testing when n is prime:
    arr2[0] = 2
    arr2[1] = 3
    arr2[2] = 5
    arr2[3] = 7
get_nth_prime(arr.count+1) = 11
bench test: n = 50000000
    time:    1s,  397ms,  985us,  867ns
    time:    1s,  407ms,  940us,  584ns
    time:    1s,  403ms,  106us,  153ns
    time:    1s,  391ms,  544us,  601ns
    time:    1s,  407ms,  985us,  483ns
    time:    1s,  388ms,  308us,  657ns
    time:    1s,  396ms,  150us,  794ns
    time:    1s,  396ms,  808us,  914ns
    time:    1s,  390ms,  182us,  307ns
    time:    1s,  400ms,  385us,  532ns
TEST 0: test_get_primes_upto_number           - STATUS: PASSED
TEST 1: test_get_get_nth_prime_basics         - STATUS: PASSED
TEST 2: test_greater_and_greater_powers_of_10 - STATUS: PASSED
TEST 3: test_get_all_primes_upto_nth_prime    - STATUS: PASSED
TEST 4: test_get_all_primes_under_n           - STATUS: PASSED
TEST 5: test_bench_test                       - STATUS: PASSED
```

