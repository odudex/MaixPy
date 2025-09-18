#include "fountain_decoder.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

// Part indexes functions
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

bool part_indexes_contains(part_indexes_t *indexes, size_t index) {
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

// Fountain decoder functions
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

    // Initialize parts storage for sequential reconstruction
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
            if (decoder->simple_parts.values[i]) {
                free(decoder->simple_parts.values[i]);
            }
        }
        free(decoder->simple_parts.values);
    }

    // Free mixed parts
    if (decoder->mixed_parts.key_sets) {
        for (size_t i = 0; i < decoder->mixed_parts.count; i++) {
            part_indexes_free(&decoder->mixed_parts.key_sets[i]);
        }
        free(decoder->mixed_parts.key_sets);
    }
    if (decoder->mixed_parts.value_lens) free(decoder->mixed_parts.value_lens);
    if (decoder->mixed_parts.values) {
        for (size_t i = 0; i < decoder->mixed_parts.count; i++) {
            if (decoder->mixed_parts.values[i]) {
                free(decoder->mixed_parts.values[i]);
            }
        }
        free(decoder->mixed_parts.values);
    }

    free(decoder);
}

bool fountain_decoder_receive_part(fountain_decoder_t *decoder, fountain_encoder_part_t *part) {
    if (!decoder || !part) return false;

    // Don't process if already complete
    if (fountain_decoder_is_complete(decoder)) {
        return false;
    }

    // This is a simplified implementation of the fountain decoding algorithm
    // For a complete implementation, you would need to implement:
    // 1. Fragment selection based on sequence number and checksum
    // 2. XOR operations for mixed parts
    // 3. Queue processing and reduction algorithms

    // For now, we'll implement basic single-part processing
    decoder->processed_parts_count++;

    // Store in received parts
    if (!part_indexes_add(&decoder->received_part_indexes, part->seq_num)) {
        return false;
    }

    // For demonstration, we'll assume single part decoding
    if (decoder->expected_part_indexes == NULL) {
        decoder->expected_part_indexes = part_indexes_new();
        if (!decoder->expected_part_indexes) return false;

        for (size_t i = 0; i < part->seq_len; i++) {
            part_indexes_add(decoder->expected_part_indexes, i);
        }
    }

    // Store the part data for reconstruction
    if (!decoder->parts) {
        decoder->parts = safe_malloc(sizeof(stored_part_t) * part->seq_len);
        if (!decoder->parts) return false;
        decoder->parts_capacity = part->seq_len;
        // Initialize all parts as not received
        for (size_t i = 0; i < part->seq_len; i++) {
            decoder->parts[i].data = NULL;
            decoder->parts[i].data_len = 0;
            decoder->parts[i].received = false;
        }
    }

    // Store this part's data (seq_num is 1-based, convert to 0-based)
    size_t part_index = part->seq_num - 1;
    if (part_index < decoder->parts_capacity && !decoder->parts[part_index].received) {
        decoder->parts[part_index].data = safe_malloc(part->data_len);
        if (!decoder->parts[part_index].data) return false;

        memcpy(decoder->parts[part_index].data, part->data, part->data_len);
        decoder->parts[part_index].data_len = part->data_len;
        decoder->parts[part_index].received = true;
    }

    // Check if we have all parts for reconstruction
    if (decoder->received_part_indexes.count >= part->seq_len) {
        // Calculate total data length
        size_t total_len = 0;
        for (size_t i = 0; i < part->seq_len; i++) {
            if (decoder->parts[i].received) {
                total_len += decoder->parts[i].data_len;
            } else {
                // Missing part - cannot reconstruct
                return true;
            }
        }

        // Create result by concatenating all parts in sequence
        decoder->result = safe_malloc(sizeof(fountain_decoder_result_t));
        if (!decoder->result) return false;

        decoder->result->data = safe_malloc(total_len);
        if (!decoder->result->data) {
            free(decoder->result);
            decoder->result = NULL;
            return false;
        }

        // Concatenate parts in order
        size_t offset = 0;
        for (size_t i = 0; i < part->seq_len; i++) {
            memcpy(decoder->result->data + offset, decoder->parts[i].data, decoder->parts[i].data_len);
            offset += decoder->parts[i].data_len;
        }

        decoder->result->data_len = total_len;
        decoder->result->is_success = true;
        decoder->result->is_error = false;
    }

    return true;
}

bool fountain_decoder_is_complete(fountain_decoder_t *decoder) {
    if (!decoder) return false;
    return decoder->result != NULL;
}

bool fountain_decoder_is_success(fountain_decoder_t *decoder) {
    if (!decoder || !decoder->result) return false;
    return decoder->result->is_success;
}

bool fountain_decoder_is_failure(fountain_decoder_t *decoder) {
    if (!decoder || !decoder->result) return false;
    return decoder->result->is_error;
}

size_t fountain_decoder_expected_part_count(fountain_decoder_t *decoder) {
    if (!decoder || !decoder->expected_part_indexes) return 0;
    return decoder->expected_part_indexes->count;
}

double fountain_decoder_estimated_percent_complete(fountain_decoder_t *decoder) {
    if (!decoder) return 0.0;

    if (fountain_decoder_is_complete(decoder)) {
        return 1.0;
    }

    if (!decoder->expected_part_indexes || decoder->expected_part_indexes->count == 0) {
        return 0.0;
    }

    double estimated_input_parts = decoder->expected_part_indexes->count * 1.75;
    double progress = decoder->processed_parts_count / estimated_input_parts;
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