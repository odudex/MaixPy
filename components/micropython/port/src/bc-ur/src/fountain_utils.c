#include "fountain_utils.h"
#include "utils.h"
#include "sha256/sha256.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

// Proper SHA256 hash function to match Python's hashlib behavior
static void compute_sha256(const uint8_t *input, size_t len, uint8_t output[32]) {
    CRYAL_SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, input, len);
    sha256_final(&ctx, output);
}

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

void prng_init_from_bytes(prng_state_t *prng, const uint8_t *seed, size_t seed_len) {
    if (!prng || !seed) return;

    uint8_t hash[32];
    compute_sha256(seed, seed_len, hash);


    // Initialize state from hash - use first 32 bytes as 4 uint64_t values
    for (int i = 0; i < 4; i++) {
        prng->state[i] = 0;
        for (int j = 0; j < 8; j++) {
            prng->state[i] <<= 8;
            prng->state[i] |= hash[i * 8 + j];
        }
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

    // int(next_double() * (high - low + 1) + low)
    double range = (double)(max - min + 1);
    double rand_val = prng_next_double(prng);
    uint64_t result = (uint64_t)(rand_val * range + min);

    // Ensure we stay within bounds and match Python's & MAX_UINT64
    return (uint32_t)(result & 0xFFFFFFFFULL);
}

double prng_next_double(prng_state_t *prng) {
    if (!prng) return 0.0;

    uint64_t val = prng_next_uint64(prng);
    return (double)val / (double)(0xFFFFFFFFFFFFFFFFULL + 1.0);
}

// RandomSampler implementation (alias method)
bool random_sampler_init(random_sampler_t *sampler, double *probs, size_t count) {
    if (!sampler || !probs || count == 0) return false;

    // Normalize probabilities and scale
    double total = 0.0;
    for (size_t i = 0; i < count; i++) {
        if (probs[i] <= 0.0) return false;
        total += probs[i];
    }
    if (total <= 0.0) return false;

    // Allocate memory
    sampler->probs = safe_malloc(count * sizeof(double));
    sampler->aliases = safe_malloc(count * sizeof(int));
    if (!sampler->probs || !sampler->aliases) {
        random_sampler_free(sampler);
        return false;
    }
    sampler->count = count;

    // Create scaled probability array P
    double *P = safe_malloc(count * sizeof(double));
    if (!P) {
        random_sampler_free(sampler);
        return false;
    }

    for (size_t i = 0; i < count; i++) {
        P[i] = (probs[i] * count) / total;
    }

    // Create small and large lists
    int *small = safe_malloc(count * sizeof(int));
    int *large = safe_malloc(count * sizeof(int));
    if (!small || !large) {
        free(P);
        free(small);
        free(large);
        random_sampler_free(sampler);
        return false;
    }

    size_t small_size = 0, large_size = 0;

    // Populate small and large lists (reverse order like Python)
    for (int i = (int)count - 1; i >= 0; i--) {
        if (P[i] < 1.0) {
            small[small_size++] = i;
        } else {
            large[large_size++] = i;
        }
    }

    // Process alias method
    while (small_size > 0 && large_size > 0) {
        int a = small[--small_size];  // Schwarz's l
        int g = large[--large_size];  // Schwarz's g

        sampler->probs[a] = P[a];
        sampler->aliases[a] = g;
        P[g] = P[g] + P[a] - 1.0;

        if (P[g] < 1.0) {
            small[small_size++] = g;
        } else {
            large[large_size++] = g;
        }
    }

    // Handle remaining items
    while (large_size > 0) {
        sampler->probs[large[--large_size]] = 1.0;
    }
    while (small_size > 0) {
        sampler->probs[small[--small_size]] = 1.0;
    }

    free(P);
    free(small);
    free(large);
    return true;
}

void random_sampler_free(random_sampler_t *sampler) {
    if (!sampler) return;
    free(sampler->probs);
    free(sampler->aliases);
    sampler->probs = NULL;
    sampler->aliases = NULL;
    sampler->count = 0;
}

int random_sampler_next(random_sampler_t *sampler, prng_state_t *rng) {
    if (!sampler || !sampler->probs || !sampler->aliases || sampler->count == 0) return 0;

    double r1 = prng_next_double(rng);
    double r2 = prng_next_double(rng);
    int i = (int)((double)sampler->count * r1);

    // Ensure i is within bounds
    if (i >= (int)sampler->count) i = (int)sampler->count - 1;

    return (r2 < sampler->probs[i]) ? i : sampler->aliases[i];
}

static size_t choose_degree(size_t seq_len, prng_state_t *prng) {
    printf("DEBUG: choose_degree entered with seq_len=%zu\n", seq_len);

    if (seq_len == 0) {
        printf("DEBUG: choose_degree seq_len is 0, returning 1\n");
        return 1;
    }

    printf("DEBUG: creating degree probabilities array\n");
    // Create degree probabilities array (1/i for i from 1 to seq_len)
    double *degree_probs = safe_malloc(seq_len * sizeof(double));
    if (!degree_probs) {
        printf("DEBUG: failed to allocate degree_probs\n");
        return 1;
    }

    printf("DEBUG: populating degree probabilities\n");
    for (size_t i = 0; i < seq_len; i++) {
        degree_probs[i] = 1.0 / (i + 1);
    }

    printf("DEBUG: initializing random sampler\n");
    // Create and use RandomSampler
    random_sampler_t sampler = {0};
    if (!random_sampler_init(&sampler, degree_probs, seq_len)) {
        printf("DEBUG: random_sampler_init failed\n");
        free(degree_probs);
        return 1;
    }
    printf("DEBUG: random sampler initialized successfully\n");

    printf("DEBUG: calling random_sampler_next\n");
    int degree_index = random_sampler_next(&sampler, prng);
    printf("DEBUG: random_sampler_next returned %d\n", degree_index);

    size_t degree = degree_index + 1;  // Convert 0-based index to 1-based degree
    printf("DEBUG: calculated degree=%zu\n", degree);

    random_sampler_free(&sampler);
    free(degree_probs);

    printf("DEBUG: choose_degree returning degree=%zu\n", degree);
    return degree;
}

bool choose_fragments(uint32_t seq_num, size_t seq_len, uint32_t checksum, part_indexes_t *result) {
    printf("DEBUG: choose_fragments entered with seq_num=%u, seq_len=%zu, checksum=%u\n", seq_num, seq_len, checksum);

    if (!result || seq_len == 0) {
        printf("DEBUG: choose_fragments invalid parameters\n");
        return false;
    }

    part_indexes_clear(result);

    // The first seq_len parts are pure fragments
    if (seq_num <= seq_len) {
        printf("DEBUG: choosing pure fragment %u\n", seq_num - 1);
        return part_indexes_add(result, seq_num - 1);
    }

    printf("DEBUG: creating mixed fragment\n");

    // int_to_bytes(seq_num) + int_to_bytes(checksum)
    // Each int_to_bytes produces 4 bytes in big-endian format
    uint8_t seed[8];
    seed[0] = (seq_num >> 24) & 0xff;
    seed[1] = (seq_num >> 16) & 0xff;
    seed[2] = (seq_num >> 8) & 0xff;
    seed[3] = seq_num & 0xff;
    seed[4] = (checksum >> 24) & 0xff;
    seed[5] = (checksum >> 16) & 0xff;
    seed[6] = (checksum >> 8) & 0xff;
    seed[7] = checksum & 0xff;

    printf("DEBUG: initializing PRNG\n");
    prng_state_t rng;
    prng_init_from_bytes(&rng, seed, 8);

    printf("DEBUG: calling choose_degree\n");
    size_t degree = choose_degree(seq_len, &rng);
    printf("DEBUG: chosen degree=%zu\n", degree);

    // Create result array for shuffled indexes
    size_t *shuffled_indexes = safe_malloc(seq_len * sizeof(size_t));
    if (!shuffled_indexes) return false;

    // Create working copy of indexes
    size_t *remaining_indexes = safe_malloc(seq_len * sizeof(size_t));
    if (!remaining_indexes) {
        free(shuffled_indexes);
        return false;
    }

    // Initialize with 0, 1, 2, ..., seq_len-1
    for (size_t i = 0; i < seq_len; i++) {
        remaining_indexes[i] = i;
    }

    // repeatedly remove random items
    size_t remaining_count = seq_len;
    for (size_t i = 0; i < seq_len && remaining_count > 0; i++) {
        uint32_t idx = prng_next_int(&rng, 0, remaining_count - 1);
        shuffled_indexes[i] = remaining_indexes[idx];

        // Remove selected item by shifting remaining items
        for (size_t j = idx; j < remaining_count - 1; j++) {
            remaining_indexes[j] = remaining_indexes[j + 1];
        }
        remaining_count--;
    }

    // Take first 'degree' indexes
    for (size_t i = 0; i < degree && i < seq_len; i++) {
        if (!part_indexes_add(result, shuffled_indexes[i])) {
            free(shuffled_indexes);
            free(remaining_indexes);
            return false;
        }
    }

    free(shuffled_indexes);
    free(remaining_indexes);
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