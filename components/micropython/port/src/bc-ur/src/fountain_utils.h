#ifndef FOUNTAIN_UTILS_H
#define FOUNTAIN_UTILS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "fountain_decoder.h"

/**
 * Simple PRNG for fountain coding (simplified Xoshiro256)
 */
typedef struct {
    uint64_t state[4];
} prng_state_t;

/**
 * Initialize PRNG with seed
 * @param prng PRNG state
 * @param seed 8-byte seed
 */
void prng_init(prng_state_t *prng, const uint8_t seed[8]);

/**
 * Generate next random integer in range [min, max]
 * @param prng PRNG state
 * @param min Minimum value (inclusive)
 * @param max Maximum value (inclusive)
 * @return Random integer
 */
uint32_t prng_next_int(prng_state_t *prng, uint32_t min, uint32_t max);

/**
 * Generate next random double in range [0.0, 1.0)
 * @param prng PRNG state
 * @return Random double
 */
double prng_next_double(prng_state_t *prng);

/**
 * Choose fragments for a fountain encoder part
 * @param seq_num Sequence number
 * @param seq_len Total sequence length
 * @param checksum Message checksum
 * @param result Output part indexes
 * @return true on success
 */
bool choose_fragments(uint32_t seq_num, size_t seq_len, uint32_t checksum, part_indexes_t *result);

/**
 * Check if part_indexes_a is strict subset of part_indexes_b
 * @param a First set
 * @param b Second set
 * @return true if a is strict subset of b
 */
bool part_indexes_is_strict_subset(const part_indexes_t *a, const part_indexes_t *b);

/**
 * Set difference: result = a - b
 * @param a First set
 * @param b Second set
 * @param result Output set (a - b)
 * @return true on success
 */
bool part_indexes_difference(const part_indexes_t *a, const part_indexes_t *b, part_indexes_t *result);

/**
 * Check if two part_indexes are equal
 * @param a First set
 * @param b Second set
 * @return true if equal
 */
bool part_indexes_equal(const part_indexes_t *a, const part_indexes_t *b);

/**
 * Copy part_indexes
 * @param src Source
 * @param dst Destination
 * @return true on success
 */
bool part_indexes_copy(const part_indexes_t *src, part_indexes_t *dst);

/**
 * Join fragments into a single message, taking only message_len bytes
 * @param fragments Array of fragment pointers
 * @param fragment_lens Array of fragment lengths
 * @param fragment_count Number of fragments
 * @param message_len Expected message length
 * @param result Output buffer (allocated by caller)
 * @return true on success
 */
bool join_fragments(uint8_t **fragments, size_t *fragment_lens, size_t fragment_count,
                   size_t message_len, uint8_t *result);

#endif // FOUNTAIN_UTILS_H