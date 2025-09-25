#include "fountain_decoder.h"
#include "fountain_utils.h"
#include "crc32.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Queue operations
bool queue_init(part_queue_t *queue, size_t capacity) {
    if (!queue || capacity == 0) return false;

    queue->parts = safe_malloc(capacity * sizeof(decoder_part_t));
    if (!queue->parts) return false;

    // Initialize all decoder_part_t structures in the array
    for (size_t i = 0; i < capacity; i++) {
        queue->parts[i].indexes.indexes = NULL;
        queue->parts[i].indexes.count = 0;
        queue->parts[i].indexes.capacity = 0;
        queue->parts[i].data = NULL;
        queue->parts[i].data_len = 0;
    }

    queue->front = 0;
    queue->rear = 0;
    queue->count = 0;
    queue->capacity = capacity;

    return true;
}

void queue_free(part_queue_t *queue) {
    if (!queue) return;

    if (queue->parts) {
        // Free all parts in queue
        for (size_t i = 0; i < queue->count; i++) {
            size_t idx = (queue->front + i) % queue->capacity;
            decoder_part_free(&queue->parts[idx]);
        }
        free(queue->parts);
    }

    memset(queue, 0, sizeof(part_queue_t));
}

bool queue_enqueue(part_queue_t *queue, const decoder_part_t *part) {
    if (!queue || !part || queue->count >= queue->capacity) return false;

    if (!decoder_part_copy(part, &queue->parts[queue->rear])) {
        return false;
    }

    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->count++;

    return true;
}

bool queue_dequeue(part_queue_t *queue, decoder_part_t *part) {
    if (!queue || !part || queue->count == 0) return false;

    if (!decoder_part_copy(&queue->parts[queue->front], part)) {
        return false;
    }

    decoder_part_free(&queue->parts[queue->front]);
    queue->front = (queue->front + 1) % queue->capacity;
    queue->count--;

    return true;
}

bool queue_is_empty(const part_queue_t *queue) {
    return !queue || queue->count == 0;
}

// Part operations
void decoder_part_free(decoder_part_t *part) {
    if (!part) return;

    if (part->indexes.indexes) {
        free(part->indexes.indexes);
        part->indexes.indexes = NULL;
    }
    part->indexes.count = 0;
    part->indexes.capacity = 0;

    if (part->data) {
        free(part->data);
        part->data = NULL;
    }
    part->data_len = 0;
}

bool decoder_part_copy(const decoder_part_t *src, decoder_part_t *dst) {
    if (!src || !dst) return false;

    // Clear destination first
    decoder_part_free(dst);

    // Initialize destination indexes
    dst->indexes.indexes = NULL;
    dst->indexes.count = 0;
    dst->indexes.capacity = 0;

    // Copy indexes
    if (!part_indexes_copy(&src->indexes, &dst->indexes)) {
        return false;
    }

    // Copy data
    if (src->data && src->data_len > 0) {
        dst->data = safe_malloc(src->data_len);
        if (!dst->data) {
            part_indexes_free(&dst->indexes);
            return false;
        }
        memcpy(dst->data, src->data, src->data_len);
        dst->data_len = src->data_len;
    } else {
        dst->data = NULL;
        dst->data_len = 0;
    }

    return true;
}

// Part indexes functions (updated)
part_indexes_t *part_indexes_new(void) {
    part_indexes_t *indexes = safe_malloc(sizeof(part_indexes_t));
    if (indexes) {
        indexes->indexes = NULL;
        indexes->count = 0;
        indexes->capacity = 0;
    }
    return indexes;
}

void part_indexes_free(part_indexes_t *indexes) {
    if (indexes) {
        if (indexes->indexes) {
            free(indexes->indexes);
        }
        free(indexes);
    }
}

bool part_indexes_add(part_indexes_t *indexes, size_t index) {
    if (!indexes) return false;

    // Check if already exists
    if (part_indexes_contains(indexes, index)) {
        return true;
    }

    // Expand array if needed
    if (indexes->count >= indexes->capacity) {
        size_t new_capacity = indexes->capacity == 0 ? 8 : indexes->capacity * 2;
        size_t *new_indexes = safe_realloc(indexes->indexes, sizeof(size_t) * new_capacity);
        if (!new_indexes) return false;

        indexes->indexes = new_indexes;
        indexes->capacity = new_capacity;
    }

    indexes->indexes[indexes->count++] = index;
    return true;
}

bool part_indexes_contains(const part_indexes_t *indexes, size_t index) {
    if (!indexes) return false;

    for (size_t i = 0; i < indexes->count; i++) {
        if (indexes->indexes[i] == index) {
            return true;
        }
    }
    return false;
}

void part_indexes_clear(part_indexes_t *indexes) {
    if (indexes) {
        indexes->count = 0;
    }
}

// Fountain decoder implementation
fountain_decoder_t *fountain_decoder_new(void) {
    fountain_decoder_t *decoder = safe_malloc(sizeof(fountain_decoder_t));
    if (!decoder) return NULL;

    // Initialize part indexes
    decoder->received_part_indexes.indexes = NULL;
    decoder->received_part_indexes.count = 0;
    decoder->received_part_indexes.capacity = 0;

    decoder->last_part_indexes = NULL;
    decoder->processed_parts_count = 0;
    decoder->result = NULL;
    decoder->expected_part_indexes = NULL;
    decoder->expected_fragment_len = 0;
    decoder->expected_message_len = 0;
    decoder->expected_checksum = 0;

    // Initialize parts storage
    decoder->parts = NULL;
    decoder->parts_capacity = 0;

    // Initialize simple parts storage
    decoder->simple_parts.keys = NULL;
    decoder->simple_parts.values = NULL;
    decoder->simple_parts.value_lens = NULL;
    decoder->simple_parts.count = 0;
    decoder->simple_parts.capacity = 0;

    // Initialize mixed parts storage
    decoder->mixed_parts.key_sets = NULL;
    decoder->mixed_parts.values = NULL;
    decoder->mixed_parts.value_lens = NULL;
    decoder->mixed_parts.count = 0;
    decoder->mixed_parts.capacity = 0;

    // Initialize queue
    if (!queue_init(&decoder->queue, 100)) {
        free(decoder);
        return NULL;
    }

    return decoder;
}

void fountain_decoder_free(fountain_decoder_t *decoder) {
    if (!decoder) return;

    // Free part indexes
    if (decoder->received_part_indexes.indexes) {
        free(decoder->received_part_indexes.indexes);
    }

    if (decoder->last_part_indexes) {
        part_indexes_free(decoder->last_part_indexes);
    }

    if (decoder->expected_part_indexes) {
        part_indexes_free(decoder->expected_part_indexes);
    }

    // Free result
    if (decoder->result) {
        if (decoder->result->data) {
            free(decoder->result->data);
        }
        free(decoder->result);
    }

    // Free parts storage
    if (decoder->parts) {
        for (size_t i = 0; i < decoder->parts_capacity; i++) {
            if (decoder->parts[i].data) {
                free(decoder->parts[i].data);
            }
        }
        free(decoder->parts);
    }

    // Free simple parts
    if (decoder->simple_parts.keys) free(decoder->simple_parts.keys);
    if (decoder->simple_parts.value_lens) free(decoder->simple_parts.value_lens);
    if (decoder->simple_parts.values) {
        for (size_t i = 0; i < decoder->simple_parts.count; i++) {
            decoder_part_free(&decoder->simple_parts.values[i]);
        }
        free(decoder->simple_parts.values);
    }

    // Free mixed parts
    if (decoder->mixed_parts.key_sets) {
        for (size_t i = 0; i < decoder->mixed_parts.count; i++) {
            if (decoder->mixed_parts.key_sets[i].indexes) {
                free(decoder->mixed_parts.key_sets[i].indexes);
            }
        }
        free(decoder->mixed_parts.key_sets);
    }
    if (decoder->mixed_parts.value_lens) free(decoder->mixed_parts.value_lens);
    if (decoder->mixed_parts.values) {
        for (size_t i = 0; i < decoder->mixed_parts.count; i++) {
            decoder_part_free(&decoder->mixed_parts.values[i]);
        }
        free(decoder->mixed_parts.values);
    }

    // Free queue
    queue_free(&decoder->queue);

    free(decoder);
}

static bool create_decoder_part_from_encoder_part(const fountain_encoder_part_t *encoder_part, decoder_part_t *decoder_part) {
    printf("DEBUG: create_decoder_part_from_encoder_part entered\n");

    if (!encoder_part || !decoder_part) {
        printf("DEBUG: create_decoder_part_from_encoder_part NULL pointer check failed\n");
        return false;
    }

    // Initialize decoder part
    decoder_part->indexes.indexes = NULL;
    decoder_part->indexes.count = 0;
    decoder_part->indexes.capacity = 0;
    decoder_part->data = NULL;
    decoder_part->data_len = 0;

    // Choose fragments based on sequence number and checksum
    printf("DEBUG: calling choose_fragments with seq_num=%u, seq_len=%u, checksum=%u\n",
           encoder_part->seq_num, encoder_part->seq_len, encoder_part->checksum);
    if (!choose_fragments(encoder_part->seq_num, encoder_part->seq_len, encoder_part->checksum, &decoder_part->indexes)) {
        printf("DEBUG: choose_fragments failed\n");
        return false;
    }
    printf("DEBUG: choose_fragments succeeded, got %u indexes\n", decoder_part->indexes.count);

    // Copy data
    printf("DEBUG: copying data, data_len=%u\n", encoder_part->data_len);
    if (encoder_part->data && encoder_part->data_len > 0) {
        decoder_part->data = safe_malloc(encoder_part->data_len);
        if (!decoder_part->data) {
            printf("DEBUG: failed to allocate decoder part data\n");
            if (decoder_part->indexes.indexes) {
                free(decoder_part->indexes.indexes);
                decoder_part->indexes.indexes = NULL;
            }
            return false;
        }
        memcpy(decoder_part->data, encoder_part->data, encoder_part->data_len);
        decoder_part->data_len = encoder_part->data_len;
        printf("DEBUG: data copied successfully\n");
    } else {
        printf("DEBUG: no data to copy\n");
    }

    printf("DEBUG: create_decoder_part_from_encoder_part returning true\n");
    return true;
}

static bool is_simple_part(const decoder_part_t *part) {
    return part && part->indexes.count == 1;
}

static size_t get_part_index(const decoder_part_t *part) {
    if (!part || part->indexes.count == 0) return 0;
    return part->indexes.indexes[0];
}

static bool add_simple_part(fountain_decoder_t *decoder, const decoder_part_t *part) {
    if (!decoder || !part || !is_simple_part(part)) return false;

    size_t index = get_part_index(part);

    // Check if already exists
    for (size_t i = 0; i < decoder->simple_parts.count; i++) {
        if (decoder->simple_parts.keys[i] == index) {
            return true; // Already exists
        }
    }

    // Expand arrays if needed
    if (decoder->simple_parts.count >= decoder->simple_parts.capacity) {
        size_t new_capacity = decoder->simple_parts.capacity == 0 ? 8 : decoder->simple_parts.capacity * 2;

        size_t *new_keys = safe_realloc(decoder->simple_parts.keys, sizeof(size_t) * new_capacity);
        decoder_part_t *new_values = safe_realloc(decoder->simple_parts.values, sizeof(decoder_part_t) * new_capacity);
        size_t *new_lens = safe_realloc(decoder->simple_parts.value_lens, sizeof(size_t) * new_capacity);

        if (!new_keys || !new_values || !new_lens) {
            return false;
        }

        // Initialize new decoder_part_t structures
        for (size_t i = decoder->simple_parts.capacity; i < new_capacity; i++) {
            new_values[i].indexes.indexes = NULL;
            new_values[i].indexes.count = 0;
            new_values[i].indexes.capacity = 0;
            new_values[i].data = NULL;
            new_values[i].data_len = 0;
        }

        decoder->simple_parts.keys = new_keys;
        decoder->simple_parts.values = new_values;
        decoder->simple_parts.value_lens = new_lens;
        decoder->simple_parts.capacity = new_capacity;
    }

    // Copy part to simple parts storage
    decoder_part_t *stored_part = &decoder->simple_parts.values[decoder->simple_parts.count];
    if (!decoder_part_copy(part, stored_part)) {
        return false;
    }

    decoder->simple_parts.keys[decoder->simple_parts.count] = index;
    decoder->simple_parts.value_lens[decoder->simple_parts.count] = part->data_len;
    decoder->simple_parts.count++;

    return true;
}

static bool reduce_part_by_part(const decoder_part_t *a, const decoder_part_t *b, decoder_part_t *result) {
    if (!a || !b || !result) return false;

    // Check if b's indexes are a strict subset of a's indexes
    if (part_indexes_is_strict_subset(&b->indexes, &a->indexes)) {
        // Initialize result
        result->indexes.indexes = NULL;
        result->indexes.count = 0;
        result->indexes.capacity = 0;
        result->data = NULL;
        result->data_len = 0;

        // Calculate set difference: a - b
        if (!part_indexes_difference(&a->indexes, &b->indexes, &result->indexes)) {
            return false;
        }
        // XOR the data
        result->data = safe_malloc(a->data_len);
        if (!result->data) {
            if (result->indexes.indexes) {
                free(result->indexes.indexes);
            }
            return false;
        }

        result->data_len = a->data_len;
        const size_t count = a->data_len;
        for (size_t i = 0; i < count; i++) {
            result->data[i] = a->data[i] ^ b->data[i];
        }
        return true;
    } else {
        // If b is not a subset of a, return a copy of a
        return decoder_part_copy(a, result);
    }
}

static bool add_mixed_part(fountain_decoder_t *decoder, const decoder_part_t *part) {
    if (!decoder || !part || is_simple_part(part)) return false;
    
    // Check if already exists
    for (size_t i = 0; i < decoder->mixed_parts.count; i++) {
        if (part_indexes_equal(&decoder->mixed_parts.key_sets[i], &part->indexes)) {
            return true; // Already exists
        }
    }
    
    // Expand arrays if needed
    if (decoder->mixed_parts.count >= decoder->mixed_parts.capacity) {
        size_t new_capacity = decoder->mixed_parts.capacity == 0 ? 8 : decoder->mixed_parts.capacity * 2;
        
        part_indexes_t *new_key_sets = safe_realloc(decoder->mixed_parts.key_sets, sizeof(part_indexes_t) * new_capacity);
        decoder_part_t *new_values = safe_realloc(decoder->mixed_parts.values, sizeof(decoder_part_t) * new_capacity);
        size_t *new_lens = safe_realloc(decoder->mixed_parts.value_lens, sizeof(size_t) * new_capacity);
        
        if (!new_key_sets || !new_values || !new_lens) {
            return false;
        }

        // Initialize new key_sets structures
        for (size_t i = decoder->mixed_parts.capacity; i < new_capacity; i++) {
            new_key_sets[i].indexes = NULL;
            new_key_sets[i].count = 0;
            new_key_sets[i].capacity = 0;
        }

        // Initialize new values structures
        for (size_t i = decoder->mixed_parts.capacity; i < new_capacity; i++) {
            new_values[i].indexes.indexes = NULL;
            new_values[i].indexes.count = 0;
            new_values[i].indexes.capacity = 0;
            new_values[i].data = NULL;
            new_values[i].data_len = 0;
        }

        decoder->mixed_parts.key_sets = new_key_sets;
        decoder->mixed_parts.values = new_values;
        decoder->mixed_parts.value_lens = new_lens;
        decoder->mixed_parts.capacity = new_capacity;
    }
    
    // Initialize the new mixed part
    decoder_part_t *mixed_part = &decoder->mixed_parts.values[decoder->mixed_parts.count];
    mixed_part->indexes.indexes = NULL;
    mixed_part->indexes.count = 0;
    mixed_part->indexes.capacity = 0;
    mixed_part->data = NULL;
    mixed_part->data_len = 0;
    
    // Copy part data
    if (!decoder_part_copy(part, mixed_part)) {
        return false;
    }
    
    // Copy indexes for key set
    if (!part_indexes_copy(&part->indexes, &decoder->mixed_parts.key_sets[decoder->mixed_parts.count])) {
        decoder_part_free(mixed_part);
        return false;
    }
    
    decoder->mixed_parts.value_lens[decoder->mixed_parts.count] = part->data_len;
    decoder->mixed_parts.count++;
    
    return true;
}

static void reduce_mixed_by(fountain_decoder_t *decoder, const decoder_part_t *part) {
    if (!decoder || !part) return;

    // Create temporary arrays to hold reduced parts
    decoder_part_t *reduced_parts = safe_malloc(decoder->mixed_parts.count * sizeof(decoder_part_t));
    if (!reduced_parts) return;

    size_t reduced_count = 0;
    size_t original_count = decoder->mixed_parts.count; // Save original count

    // Reduce each mixed part by the given part
    for (size_t i = 0; i < original_count; i++) {
        decoder_part_t *mixed_part = &decoder->mixed_parts.values[i];
        decoder_part_t *reduced_part = &reduced_parts[reduced_count];

        // Initialize reduced part
        reduced_part->indexes.indexes = NULL;
        reduced_part->indexes.count = 0;
        reduced_part->indexes.capacity = 0;
        reduced_part->data = NULL;
        reduced_part->data_len = 0;

        if (reduce_part_by_part(mixed_part, part, reduced_part)) {
            reduced_count++;
        }
    }

    // Clear current mixed parts using original count
    for (size_t i = 0; i < original_count; i++) {
        decoder_part_free(&decoder->mixed_parts.values[i]);
        // Clear key_sets properly
        if (decoder->mixed_parts.key_sets[i].indexes) {
            free(decoder->mixed_parts.key_sets[i].indexes);
            decoder->mixed_parts.key_sets[i].indexes = NULL;
        }
        decoder->mixed_parts.key_sets[i].count = 0;
        decoder->mixed_parts.key_sets[i].capacity = 0;
    }
    decoder->mixed_parts.count = 0; // Reset count after freeing
    
    // Process reduced parts
    for (size_t i = 0; i < reduced_count; i++) {
        if (is_simple_part(&reduced_parts[i])) {
            // Add to queue for processing
            queue_enqueue(&decoder->queue, &reduced_parts[i]);
        } else {
            // Add back to mixed parts
            add_mixed_part(decoder, &reduced_parts[i]);
        }
        decoder_part_free(&reduced_parts[i]);
    }
    
    free(reduced_parts);
}

static void process_simple_part(fountain_decoder_t *decoder, const decoder_part_t *part) {
    if (!decoder || !part || !is_simple_part(part)) return;

    size_t fragment_index = get_part_index(part);

    // Don't process duplicate parts
    if (part_indexes_contains(&decoder->received_part_indexes, fragment_index)) {
        return;
    }

    // Record this part
    if (!add_simple_part(decoder, part)) {
        return;
    }

    if (!part_indexes_add(&decoder->received_part_indexes, fragment_index)) {
        return;
    }

    // Check if we have all expected parts
    if (decoder->expected_part_indexes &&
        part_indexes_equal(&decoder->received_part_indexes, decoder->expected_part_indexes)) {

        // Reconstruct message
        size_t part_count = decoder->simple_parts.count;
        uint8_t **fragments = safe_malloc(part_count * sizeof(uint8_t*));
        size_t *fragment_lens = safe_malloc(part_count * sizeof(size_t));

        if (!fragments || !fragment_lens) {
            if (fragments) free(fragments);
            if (fragment_lens) free(fragment_lens);
            return;
        }

        // Sort parts by index (to match Python implementation)
        // First, create array of (index, fragment, length) tuples
        typedef struct {
            size_t index;
            uint8_t *data;
            size_t len;
        } fragment_info_t;

        fragment_info_t *sorted_fragments = safe_malloc(part_count * sizeof(fragment_info_t));
        if (!sorted_fragments) {
            free(fragments);
            free(fragment_lens);
            return;
        }

        // Populate the array
        for (size_t i = 0; i < part_count; i++) {
            sorted_fragments[i].index = decoder->simple_parts.keys[i];
            sorted_fragments[i].data = decoder->simple_parts.values[i].data;
            sorted_fragments[i].len = decoder->simple_parts.values[i].data_len;
        }

        // Sort by index (simple bubble sort for small arrays)
        for (size_t i = 0; i < part_count - 1; i++) {
            for (size_t j = 0; j < part_count - 1 - i; j++) {
                if (sorted_fragments[j].index > sorted_fragments[j + 1].index) {
                    fragment_info_t temp = sorted_fragments[j];
                    sorted_fragments[j] = sorted_fragments[j + 1];
                    sorted_fragments[j + 1] = temp;
                }
            }
        }

        // Copy sorted fragments to output arrays
        for (size_t i = 0; i < part_count; i++) {
            fragments[i] = sorted_fragments[i].data;
            fragment_lens[i] = sorted_fragments[i].len;
        }

        free(sorted_fragments);

        // Join fragments
        uint8_t *message = safe_malloc(decoder->expected_message_len);
        if (!message) {
            free(fragments);
            free(fragment_lens);
            return;
        }

        if (join_fragments(fragments, fragment_lens, part_count, decoder->expected_message_len, message)) {
            // Verify checksum
            uint32_t checksum = crc32_calculate(message, decoder->expected_message_len);

            if (checksum == decoder->expected_checksum) {
                // Success!
                decoder->result = safe_malloc(sizeof(fountain_decoder_result_t));
                if (decoder->result) {
                    decoder->result->data = message;
                    decoder->result->data_len = decoder->expected_message_len;
                    decoder->result->is_success = true;
                    decoder->result->is_error = false;
                    message = NULL; // Don't free it
                }
            } else {
                // Checksum failure
                decoder->result = safe_malloc(sizeof(fountain_decoder_result_t));
                if (decoder->result) {
                    decoder->result->data = NULL;
                    decoder->result->data_len = 0;
                    decoder->result->is_success = false;
                    decoder->result->is_error = true;
                }
            }
        }

        if (message) free(message);
        free(fragments);
        free(fragment_lens);
    }
}

static void process_mixed_part(fountain_decoder_t *decoder, const decoder_part_t *part) {
    if (!decoder || !part || is_simple_part(part)) return;

    // Check if already exists
    for (size_t i = 0; i < decoder->mixed_parts.count; i++) {
        if (part_indexes_equal(&decoder->mixed_parts.key_sets[i], &part->indexes)) {
            return;
        }
    }

    // Reduce this part by all the simple parts
    decoder_part_t reduced_part = {0};
    reduced_part.indexes.indexes = NULL;
    reduced_part.indexes.count = 0;
    reduced_part.indexes.capacity = 0;
    reduced_part.data = NULL;
    reduced_part.data_len = 0;

    if (!decoder_part_copy(part, &reduced_part)) {
        return;
    }

    // Reduce by all simple parts
    for (size_t i = 0; i < decoder->simple_parts.count; i++) {
        decoder_part_t temp = {0};

        if (reduce_part_by_part(&reduced_part, &decoder->simple_parts.values[i], &temp)) {
            decoder_part_free(&reduced_part);
            reduced_part = temp;
        }
    }

    // Reduce by all mixed parts
    for (size_t i = 0; i < decoder->mixed_parts.count; i++) {
        decoder_part_t temp = {0};

        if (reduce_part_by_part(&reduced_part, &decoder->mixed_parts.values[i], &temp)) {
            decoder_part_free(&reduced_part);
            reduced_part = temp;
        }
    }

    // If the part is now simple, add it to the queue
    if (is_simple_part(&reduced_part)) {
        queue_enqueue(&decoder->queue, &reduced_part);
    } else {
        // Add to mixed parts storage
        add_mixed_part(decoder, &reduced_part);
    }

    decoder_part_free(&reduced_part);
}

static void process_queue_item(fountain_decoder_t *decoder) {
    if (!decoder || queue_is_empty(&decoder->queue)) return;

    decoder_part_t part = {0}; // Initialize to zero
    part.indexes.indexes = NULL;
    part.indexes.count = 0;
    part.indexes.capacity = 0;
    part.data = NULL;
    part.data_len = 0;

    if (!queue_dequeue(&decoder->queue, &part)) return;

    if (is_simple_part(&part)) {
        process_simple_part(decoder, &part);
        // After processing a simple part, reduce all mixed parts by it
        reduce_mixed_by(decoder, &part);
    } else {
        process_mixed_part(decoder, &part);
    }

    decoder_part_free(&part);
}

bool fountain_decoder_receive_part(fountain_decoder_t *decoder, fountain_encoder_part_t *part) {
    printf("DEBUG: fountain_decoder_receive_part entered\n");

    if (!decoder || !part) {
        printf("DEBUG: fountain decoder NULL pointer check failed\n");
        return false;
    }

    // Don't process if already complete
    if (fountain_decoder_is_complete(decoder)) {
        printf("DEBUG: fountain decoder already complete\n");
        return false;
    }

    printf("DEBUG: initializing expected values from first part\n");
    // Initialize expected values from first part
    if (decoder->expected_part_indexes == NULL) {
        printf("DEBUG: creating expected part indexes\n");
        decoder->expected_part_indexes = part_indexes_new();
        if (!decoder->expected_part_indexes) {
            printf("DEBUG: failed to create expected part indexes\n");
            return false;
        }

        printf("DEBUG: adding %u expected parts\n", part->seq_len);
        for (size_t i = 0; i < part->seq_len; i++) {
            if (!part_indexes_add(decoder->expected_part_indexes, i)) {
                printf("DEBUG: failed to add expected part index %u\n", i);
                return false;
            }
        }
        printf("DEBUG: expected parts added successfully\n");

        decoder->expected_checksum = part->checksum;
        decoder->expected_fragment_len = part->data_len;
        decoder->expected_message_len = part->message_len; // Use actual message length from part
        printf("DEBUG: expected values set: checksum=%u, fragment_len=%u, message_len=%u\n",
               part->checksum, part->data_len, part->message_len);
    }

    // Create decoder part from encoder part
    printf("DEBUG: creating decoder part from encoder part\n");
    decoder_part_t decoder_part;
    if (!create_decoder_part_from_encoder_part(part, &decoder_part)) {
        printf("DEBUG: failed to create decoder part from encoder part\n");
        return false;
    }
    printf("DEBUG: decoder part created, indexes count=%u\n", decoder_part.indexes.count);

    // Update last part indexes
    printf("DEBUG: updating last part indexes\n");
    if (decoder->last_part_indexes) {
        part_indexes_free(decoder->last_part_indexes);
    }
    decoder->last_part_indexes = part_indexes_new();
    if (decoder->last_part_indexes) {
        part_indexes_copy(&decoder_part.indexes, decoder->last_part_indexes);
        printf("DEBUG: last part indexes updated\n");
    } else {
        printf("DEBUG: failed to create last part indexes\n");
    }

    // Add to queue
    printf("DEBUG: adding decoder part to queue\n");
    if (!queue_enqueue(&decoder->queue, &decoder_part)) {
        printf("DEBUG: failed to enqueue decoder part\n");
        decoder_part_free(&decoder_part);
        return false;
    }
    printf("DEBUG: decoder part enqueued, queue count=%u\n", decoder->queue.count);

    // Process queue
    printf("DEBUG: starting queue processing loop\n");
    size_t loop_count = 0;
    while (!fountain_decoder_is_complete(decoder) && !queue_is_empty(&decoder->queue)) {
        printf("DEBUG: processing queue item %u\n", loop_count);
        process_queue_item(decoder);
        loop_count++;

        // Safety check to prevent infinite loops
        if (loop_count > 10000) {
            printf("DEBUG: ERROR - infinite loop detected in queue processing\n");
            break;
        }
    }
    printf("DEBUG: queue processing complete, processed %u items\n", loop_count);

    decoder->processed_parts_count++;
    printf("DEBUG: processed parts count now: %u\n", decoder->processed_parts_count);
    decoder_part_free(&decoder_part);

    printf("DEBUG: fountain_decoder_receive_part returning true\n");
    return true;
}

bool fountain_decoder_is_complete(fountain_decoder_t *decoder) {
    return decoder && decoder->result != NULL;
}

bool fountain_decoder_is_success(fountain_decoder_t *decoder) {
    return decoder && decoder->result && decoder->result->is_success;
}

bool fountain_decoder_is_failure(fountain_decoder_t *decoder) {
    return decoder && decoder->result && decoder->result->is_error;
}

size_t fountain_decoder_expected_part_count(fountain_decoder_t *decoder) {
    if (!decoder || !decoder->expected_part_indexes) return 0;
    return decoder->expected_part_indexes->count;
}

double fountain_decoder_estimated_percent_complete(fountain_decoder_t *decoder) {
    if (!decoder) return 0.0;
    if (fountain_decoder_is_complete(decoder)) return 1.0;
    if (!decoder->expected_part_indexes) return 0.0;

    double estimated_input_parts = fountain_decoder_expected_part_count(decoder) * 1.75;
    double progress = (double)decoder->processed_parts_count / estimated_input_parts;
    return progress > 0.99 ? 0.99 : progress;
}

uint8_t *fountain_decoder_result_message(fountain_decoder_t *decoder) {
    if (!decoder || !decoder->result) return NULL;
    return decoder->result->data;
}

size_t fountain_decoder_result_message_len(fountain_decoder_t *decoder) {
    if (!decoder || !decoder->result) return 0;
    return decoder->result->data_len;
}