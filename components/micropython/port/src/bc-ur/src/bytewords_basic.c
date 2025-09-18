#include "bytewords.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Simplified bytewords implementation
// Note: This is a basic implementation. For production use,
// you should import the full bytewords implementation from the C++ reference.

// Bytewords lookup table (first 16 entries for demonstration)
static const char *bytewords_table[] = {
    "able", "acid", "also", "apex", "arch", "area", "army", "aunt",
    "away", "axis", "back", "bald", "barn", "beta", "bias", "blue",
    // ... (complete table would have 256 entries)
    // For now, we'll implement a basic version
};

static const size_t BYTEWORDS_TABLE_SIZE = 8; // Using only first 8 for demo

static bool find_byteword_index(const char *word, uint8_t *index) {
    if (!word || !index) return false;

    for (size_t i = 0; i < BYTEWORDS_TABLE_SIZE; i++) {
        if (strcmp(word, bytewords_table[i]) == 0) {
            *index = (uint8_t)i;
            return true;
        }
    }
    return false;
}

bool bytewords_decode(bytewords_style_t style, const char *encoded, uint8_t **decoded, size_t *decoded_len) {
    if (!encoded || !decoded || !decoded_len) return false;

    // For minimal style, words are concatenated without separators
    // For standard/URI style, words are separated by spaces or hyphens

    // This is a simplified implementation
    // For now, we'll assume minimal style and 4-character words

    size_t encoded_len = strlen(encoded);
    if (encoded_len % 4 != 0) return false; // Each byte = 4 characters in minimal style

    size_t num_bytes = encoded_len / 4;
    *decoded = safe_malloc(num_bytes);
    if (!*decoded) return false;

    *decoded_len = num_bytes;

    for (size_t i = 0; i < num_bytes; i++) {
        char word[5];
        strncpy(word, encoded + i * 4, 4);
        word[4] = '\0';

        uint8_t byte_val;
        if (!find_byteword_index(word, &byte_val)) {
            free(*decoded);
            *decoded = NULL;
            *decoded_len = 0;
            return false;
        }

        (*decoded)[i] = byte_val;
    }

    return true;
}

bool bytewords_encode(bytewords_style_t style, const uint8_t *data, size_t data_len, char **encoded) {
    if (!data || !encoded || data_len == 0) return false;

    // For minimal style
    size_t encoded_len = data_len * 4; // 4 chars per byte
    *encoded = safe_malloc(encoded_len + 1);
    if (!*encoded) return false;

    char *pos = *encoded;
    for (size_t i = 0; i < data_len; i++) {
        uint8_t byte_val = data[i];
        if (byte_val >= BYTEWORDS_TABLE_SIZE) {
            // For demo purposes, use modulo
            byte_val = byte_val % BYTEWORDS_TABLE_SIZE;
        }

        strcpy(pos, bytewords_table[byte_val]);
        pos += 4;
    }

    return true;
}

void bytewords_free(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}