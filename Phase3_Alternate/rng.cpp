#include "rng.h"

// Constructor: Needs a starting seed (must not be 0)
rng::rng(uint32_t seed) : state(seed) {
    if (state == 0)
        state = 1;
}

// Generate the next random number using 3 bitwise operations
uint32_t rng::next() {
    uint32_t x = state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    state = x;
    return x;
}

// Helper function to get a random number between 0 and max-1
uint32_t rng::next_in_range(uint32_t max) {
    // Modulo is slightly biased, but perfectly fine for work-stealing
    return next() % max;
}