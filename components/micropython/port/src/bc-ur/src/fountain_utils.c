#include "fountain_utils.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Comparison function for qsort
static int compare_size_t(const void *a, const void *b) {
    size_t arg1 = *(const size_t*)a;
    size_t arg2 = *(const size_t*)b;
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

// Simplified Xoshiro256** implementation
static uint64_t rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

void prng_init(prng_state_t *prng, const uint8_t seed[8]) {
    if (!prng || !seed) return;

    // Initialize state from seed - safe for RISC-V alignment
    uint64_t seed_value = 0;
    for (int i = 0; i < 8; i++) {
        seed_value |= ((uint64_t)seed[i]) << (i * 8);
    }

    prng->state[0] = seed_value;
    prng->state[1] = prng->state[0] ^ 0x9E3779B97F4A7C15ULL;
    prng->state[2] = prng->state[1] ^ 0x6A09E667F3BCC908ULL;
    prng->state[3] = prng->state[2] ^ 0xBB67AE8584CAA73BULL;

    // Warm up the generator
    for (int i = 0; i < 12; i++) {
        prng_next_int(prng, 0, 1);
    }
}

static uint64_t prng_next_uint64(prng_state_t *prng) {
    const uint64_t result = rotl(prng->state[1] * 5, 7) * 9;
    const uint64_t t = prng->state[1] << 17;

    prng->state[2] ^= prng->state[0];
    prng->state[3] ^= prng->state[1];
    prng->state[1] ^= prng->state[2];
    prng->state[0] ^= prng->state[3];

    prng->state[2] ^= t;
    prng->state[3] = rotl(prng->state[3], 45);

    return result;
}

uint32_t prng_next_int(prng_state_t *prng, uint32_t min, uint32_t max) {
    if (!prng || min > max) return min;

    uint64_t range = max - min + 1;
    uint64_t val = prng_next_uint64(prng);
    return min + (uint32_t)(val % range);
}

double prng_next_double(prng_state_t *prng) {
    if (!prng) return 0.0;

    uint64_t val = prng_next_uint64(prng);
    return (double)(val >> 11) * 0x1.0p-53;
}

static size_t choose_degree(size_t seq_len, prng_state_t *prng) {
    if (seq_len == 0) return 1;

    // Simplified degree selection using probability 1/i
    double r = prng_next_double(prng);
    double cumulative = 0.0;

    for (size_t i = 1; i <= seq_len; i++) {
        cumulative += 1.0 / i;
        if (r <= cumulative / seq_len) {
            return i;
        }
    }
    return seq_len;
}

bool choose_fragments(uint32_t seq_num, size_t seq_len, uint32_t checksum, part_indexes_t *result) {
    if (!result || seq_len == 0) return false;

    part_indexes_clear(result);

    // The first seq_len parts are pure fragments
    if (seq_num <= seq_len) {
        return part_indexes_add(result, seq_num - 1);
    }

    // Create seed from seq_num and checksum
    uint8_t seed[8];
    seed[0] = (seq_num >> 24) & 0xff;
    seed[1] = (seq_num >> 16) & 0xff;
    seed[2] = (seq_num >> 8) & 0xff;
    seed[3] = seq_num & 0xff;
    seed[4] = (checksum >> 24) & 0xff;
    seed[5] = (checksum >> 16) & 0xff;
    seed[6] = (checksum >> 8) & 0xff;
    seed[7] = checksum & 0xff;

    prng_state_t rng;
    prng_init(&rng, seed);

    size_t degree = choose_degree(seq_len, &rng);

    // Create list of available indexes
    size_t *indexes = safe_malloc(seq_len * sizeof(size_t));
    if (!indexes) return false;

    for (size_t i = 0; i < seq_len; i++) {
        indexes[i] = i;
    }

    // Fisher-Yates shuffle to select degree indexes
    size_t remaining = seq_len;
    for (size_t i = 0; i < degree && remaining > 0; i++) {
        uint32_t idx = prng_next_int(&rng, 0, remaining - 1);
        if (!part_indexes_add(result, indexes[idx])) {
            free(indexes);
            return false;
        }

        // Remove selected index by swapping with last
        indexes[idx] = indexes[remaining - 1];
        remaining--;
    }

    free(indexes);
    return true;
}

bool part_indexes_is_strict_subset(const part_indexes_t *a, const part_indexes_t *b) {
    if (!a || !b) return false;

    if (a->count >= b->count) return false;

    // Check if all elements of a are in b
    for (size_t i = 0; i < a->count; i++) {
        if (!part_indexes_contains(b, a->indexes[i])) {
            return false;
        }
    }

    return true;
}

bool part_indexes_difference(const part_indexes_t *a, const part_indexes_t *b, part_indexes_t *result) {
    if (!a || !b || !result) return false;

    part_indexes_clear(result);

    for (size_t i = 0; i < a->count; i++) {
        if (!part_indexes_contains(b, a->indexes[i])) {
            if (!part_indexes_add(result, a->indexes[i])) {
                return false;
            }
        }
    }
    // After populating result->indexes
    qsort(result->indexes, result->count, sizeof(size_t), compare_size_t);
    return true;
}

bool part_indexes_equal(const part_indexes_t *a, const part_indexes_t *b) {
    if (!a || !b) return false;

    if (a->count != b->count) return false;

    // Check if all elements of a are in b
    for (size_t i = 0; i < a->count; i++) {
        if (!part_indexes_contains(b, a->indexes[i])) {
            return false;
        }
    }

    return true;
}

bool part_indexes_copy(const part_indexes_t *src, part_indexes_t *dst) {
    if (!src || !dst) return false;

    part_indexes_clear(dst);

    for (size_t i = 0; i < src->count; i++) {
        if (!part_indexes_add(dst, src->indexes[i])) {
            return false;
        }
    }

    return true;
}

bool join_fragments(uint8_t **fragments, size_t *fragment_lens, size_t fragment_count,
                   size_t message_len, uint8_t *result) {
    if (!fragments || !fragment_lens || !result || fragment_count == 0) return false;

    // First, concatenate all fragments completely (like Python)
    size_t total_len = 0;
    for (size_t i = 0; i < fragment_count; i++) {
        if (fragments[i] && fragment_lens[i] > 0) {
            total_len += fragment_lens[i];
        }
    }

    // Create temporary buffer for complete concatenation
    uint8_t *temp_buffer = safe_malloc(total_len);
    if (!temp_buffer) return false;

    size_t offset = 0;
    for (size_t i = 0; i < fragment_count; i++) {
        if (fragments[i] && fragment_lens[i] > 0) {
            memcpy(temp_buffer + offset, fragments[i], fragment_lens[i]);
            offset += fragment_lens[i];
        }
    }

    // Then truncate to message_len (like Python's take_first)
    size_t copy_len = (total_len < message_len) ? total_len : message_len;
    memcpy(result, temp_buffer, copy_len);

    free(temp_buffer);
    return true;
}