// A reimplemtnation of the random function that Java provides
// https://docs.oracle.com/javase/8/docs/api/java/util/Random.html

// For more info, cross-reference with JDK source
// https://github.com/openjdk/jdk8u-dev/blob/master/jdk/src/share/classes/java/util/Random.java

#pragma once

#include <cstdint>
#include <chrono>
#include <limits>

class JavaRandom {
private:
    static constexpr uint64_t multiplier = 0x5DEECE66DULL;
    static constexpr uint64_t addend     = 0xBULL;
    static constexpr uint64_t mask       = (1ULL << 48) - 1;

    uint64_t seed;

    int32_t next(int bits) {
        seed = (seed * multiplier + addend) & mask;
        return static_cast<int32_t>(seed >> (48 - bits));
    }

public:
    JavaRandom(uint64_t initialSeed) {
        setSeed(initialSeed);
    }

    JavaRandom() {
        // Default seed: current time
        setSeed(static_cast<uint64_t>(
            std::chrono::steady_clock::now().time_since_epoch().count()
        ));
    }

    void setSeed(uint64_t s) {
        seed = (s ^ multiplier) & mask;
    }

    int32_t nextInt() {
        return next(32);
    }

    int32_t nextInt(int bound) {
        if (bound <= 0) throw std::invalid_argument("bound must be positive");

        if ((bound & -bound) == bound) { // power of two
            return static_cast<int32_t>((bound * static_cast<int64_t>(next(31))) >> 31);
        }

        int32_t bits, val;
        do {
            bits = next(31);
            val = bits % bound;
        } while (bits - val + (bound - 1) < 0);
        return val;
    }

    int64_t nextLong() {
        return (static_cast<int64_t>(next(32)) << 32) + next(32);
    }

    double nextDouble() {
        return (((int64_t)next(26) << 27) + next(27)) / static_cast<double>(1LL << 53);
    }

    bool nextBoolean() {
        return next(1) != 0;
    }

    float nextFloat() {
        return next(24) / static_cast<float>(1 << 24);
    }
};
