#pragma once

#include <cstdint>

class rng {
private:
    uint32_t state;

public:
    // Constructor: Needs a starting seed (must not be 0)
    rng(uint32_t seed);

    // Generate the next random number using 3 bitwise operations
    uint32_t next();

    // Helper function to get a random number between 0 and max-1
    uint32_t next_in_range(uint32_t max);
};