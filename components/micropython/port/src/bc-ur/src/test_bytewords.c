#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "bytewords.h"

void test_basic_encoding() {
    printf("Testing basic bytewords encoding...\n");

    uint8_t test_data[] = {0, 1, 2, 3};
    char *encoded;

    // Test minimal style
    bool success = bytewords_encode(BYTEWORDS_STYLE_MINIMAL, test_data, sizeof(test_data), &encoded);
    if (success) {
        printf("✓ Minimal encoding: %s\n", encoded);
        bytewords_free(encoded);
    } else {
        printf("✗ Minimal encoding failed\n");
    }

    // Test standard style
    success = bytewords_encode(BYTEWORDS_STYLE_STANDARD, test_data, sizeof(test_data), &encoded);
    if (success) {
        printf("✓ Standard encoding: %s\n", encoded);
        bytewords_free(encoded);
    } else {
        printf("✗ Standard encoding failed\n");
    }

    // Test URI style
    success = bytewords_encode(BYTEWORDS_STYLE_URI, test_data, sizeof(test_data), &encoded);
    if (success) {
        printf("✓ URI encoding: %s\n", encoded);
        bytewords_free(encoded);
    } else {
        printf("✗ URI encoding failed\n");
    }
}

void test_encoding_decoding_roundtrip() {
    printf("\nTesting encoding/decoding roundtrip...\n");

    uint8_t original_data[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE};
    size_t original_len = sizeof(original_data);

    bytewords_style_t styles[] = {
        BYTEWORDS_STYLE_MINIMAL,
        BYTEWORDS_STYLE_STANDARD,
        BYTEWORDS_STYLE_URI
    };

    const char *style_names[] = {"minimal", "standard", "uri"};

    for (int i = 0; i < 3; i++) {
        printf("  Testing %s style:\n", style_names[i]);

        // Encode
        char *encoded;
        bool encode_success = bytewords_encode(styles[i], original_data, original_len, &encoded);
        if (!encode_success) {
            printf("  ✗ Encoding failed\n");
            continue;
        }

        printf("    Encoded: %s\n", encoded);

        // Decode
        uint8_t *decoded;
        size_t decoded_len;
        bool decode_success = bytewords_decode(styles[i], encoded, &decoded, &decoded_len);

        if (decode_success) {
            printf("    ✓ Decoding succeeded\n");
            printf("    Original length: %zu, Decoded length: %zu\n", original_len, decoded_len);

            // Verify data matches
            if (decoded_len == original_len && memcmp(original_data, decoded, original_len) == 0) {
                printf("    ✓ Data matches perfectly!\n");
            } else {
                printf("    ✗ Data mismatch\n");
                printf("    Original: ");
                for (size_t j = 0; j < original_len; j++) {
                    printf("%02x ", original_data[j]);
                }
                printf("\n    Decoded:  ");
                for (size_t j = 0; j < decoded_len; j++) {
                    printf("%02x ", decoded[j]);
                }
                printf("\n");
            }

            bytewords_free(decoded);
        } else {
            printf("    ✗ Decoding failed\n");
        }

        bytewords_free(encoded);
    }
}

void test_known_vectors() {
    printf("\nTesting known test vectors...\n");

    // Test vector from UR spec: "Hello world" -> specific bytewords
    uint8_t hello_world[] = "Hello world";
    size_t hello_len = strlen((char*)hello_world);

    char *encoded;
    bool success = bytewords_encode(BYTEWORDS_STYLE_MINIMAL, hello_world, hello_len, &encoded);

    if (success) {
        printf("✓ 'Hello world' encoded to: %s\n", encoded);

        // Try to decode it back
        uint8_t *decoded;
        size_t decoded_len;
        bool decode_success = bytewords_decode(BYTEWORDS_STYLE_MINIMAL, encoded, &decoded, &decoded_len);

        if (decode_success && decoded_len == hello_len &&
            memcmp(hello_world, decoded, hello_len) == 0) {
            printf("✓ Round-trip successful for 'Hello world'\n");
            bytewords_free(decoded);
        } else {
            printf("✗ Round-trip failed for 'Hello world'\n");
        }

        bytewords_free(encoded);
    } else {
        printf("✗ Failed to encode 'Hello world'\n");
    }
}

void test_error_handling() {
    printf("\nTesting error handling...\n");

    uint8_t *decoded;
    size_t decoded_len;

    // Test invalid input
    bool success = bytewords_decode(BYTEWORDS_STYLE_MINIMAL, "invalid", &decoded, &decoded_len);
    printf("  Invalid input: %s\n", success ? "✗ should have failed" : "✓ correctly rejected");

    // Test too short input
    success = bytewords_decode(BYTEWORDS_STYLE_MINIMAL, "ab", &decoded, &decoded_len);
    printf("  Too short input: %s\n", success ? "✗ should have failed" : "✓ correctly rejected");

    // Test corrupted checksum (if we can construct one)
    char *valid_encoded;
    uint8_t test_data[] = {1, 2, 3, 4};
    if (bytewords_encode(BYTEWORDS_STYLE_MINIMAL, test_data, sizeof(test_data), &valid_encoded)) {
        // Corrupt last character
        size_t len = strlen(valid_encoded);
        if (len > 0) {
            char corrupted[len + 1];
            strcpy(corrupted, valid_encoded);
            corrupted[len - 1] = 'x'; // Corrupt last char

            success = bytewords_decode(BYTEWORDS_STYLE_MINIMAL, corrupted, &decoded, &decoded_len);
            printf("  Corrupted checksum: %s\n", success ? "✗ should have failed" : "✓ correctly rejected");
        }
        bytewords_free(valid_encoded);
    }
}

int main() {
    printf("=== Bytewords C Implementation Test ===\n\n");

    test_basic_encoding();
    test_encoding_decoding_roundtrip();
    test_known_vectors();
    test_error_handling();

    printf("\n=== Bytewords Test Complete ===\n");
    return 0;
}