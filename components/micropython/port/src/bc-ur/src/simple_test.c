#include <stdio.h>
#include <stdlib.h>
#include "bytewords.h"

int main() {
    // Test individual word decoding
    printf("Testing individual byteword: 'ab' (first+last chars of 'able')\n");
    uint8_t *decoded;
    size_t decoded_len;

    bool success = bytewords_decode_raw(BYTEWORDS_STYLE_MINIMAL, "ab", &decoded, &decoded_len);
    printf("Result: %s\n", success ? "SUCCESS" : "FAILED");

    if (success) {
        printf("Decoded: %02x (should be 0x00 for 'able')\n", decoded[0]);
        bytewords_free(decoded);
    }

    // Test the 'able' word specifically
    printf("\nTesting full word 'able':\n");
    success = bytewords_decode_raw(BYTEWORDS_STYLE_STANDARD, "able", &decoded, &decoded_len);
    printf("Result: %s\n", success ? "SUCCESS" : "FAILED");

    if (success) {
        printf("Decoded: %02x (should be 0x00)\n", decoded[0]);
        bytewords_free(decoded);
    }

    return 0;
}