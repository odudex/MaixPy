# Blockchain Commons UR for MicroPython

This is a pure C implementation of Blockchain Commons UR (Uniform Resources) for use with MicroPython, specifically designed for the Krux project.

## Structure

```
bc-ur/
├── bc_ur.c                    # MicroPython wrapper module
├── src/                       # Pure C implementation
│   ├── ur_decoder.h/.c        # Main UR decoder
│   ├── fountain_decoder.h/.c  # Fountain code decoder
│   ├── bytewords.h/.c         # Bytewords encoding/decoding
│   ├── utils.h/.c             # Utility functions
│   ├── test_ur.c              # Test program
│   └── Makefile               # Build system
└── README.md                  # This file
```

## Features

This implementation provides:

- **URDecoder**: Equivalent to the Python `URDecoder` class
- **Multi-part UR support**: Handles animated QR codes with fountain codes
- **Single-part UR support**: Basic UR decoding
- **MicroPython integration**: Direct replacement for Python UR decoder
- **Memory efficient**: Designed for embedded systems

## C API Usage

### Basic Usage

```c
#include "ur_decoder.h"

// Create decoder
ur_decoder_t *decoder = ur_decoder_new();

// Process UR parts
bool success = ur_decoder_receive_part(decoder, "ur:crypto-psbt/1-2/...");

// Check completion
if (ur_decoder_is_complete(decoder)) {
    ur_result_t *result = ur_decoder_get_result(decoder);
    printf("Type: %s, Data: %zu bytes\n", result->type, result->cbor_len);
    ur_result_free(result);
}

// Clean up
ur_decoder_free(decoder);
```

### Single-part UR Decoding

```c
ur_result_t *result;
ur_decoder_error_t error = ur_decoder_decode_single("ur:crypto-psbt/data", &result);
if (error == UR_DECODER_OK) {
    // Use result
    ur_result_free(result);
}
```

## MicroPython API Usage

The MicroPython wrapper provides the same interface as the original Python implementation:

```python
import bc_ur

# Create decoder
decoder = bc_ur.URDecoder()

# Process parts
success = decoder.receive_part("ur:crypto-psbt/1-2/...")

# Check progress
print(f"Progress: {decoder.estimated_percent_complete():.1%}")
print(f"Complete: {decoder.is_complete()}")

# Get result
if decoder.is_complete():
    result = decoder.result
    print(f"Type: {result.type}")
    print(f"CBOR data: {len(result.cbor)} bytes")
```

## Integration with Krux

To use this in the main Krux application, modify `src/krux/qr.py` around line 181:

```python
# Instead of:
# from ur.ur_decoder import URDecoder

# Use:
try:
    import bc_ur
    URDecoder = bc_ur.URDecoder
except ImportError:
    # Fallback to Python implementation
    from ur.ur_decoder import URDecoder
```

## Building and Testing

### Build the C library:

```bash
cd src/
make
```

### Run tests:

```bash
cd src/
make test
./test_ur
```

## Current Status

This is a **basic implementation** with the following limitations:

1. **Simplified Fountain Decoder**: The fountain code implementation is basic and may not handle all edge cases
2. **Basic Bytewords**: Only supports minimal style with limited word dictionary
3. **No CRC validation**: Checksum validation is not fully implemented
4. **Limited error handling**: Some edge cases may not be handled

## Next Steps for Production Use

1. **Complete Fountain Decoder**: Implement full fountain code algorithm from the C++ reference
2. **Full Bytewords**: Import complete bytewords implementation with all 256 words
3. **Add CRC32**: Implement proper checksum validation
4. **Memory optimization**: Add memory pool allocation for embedded use
5. **Extensive testing**: Add comprehensive test suite
6. **Performance tuning**: Optimize for MicroPython constraints

## Implementation Notes

- The C implementation follows the same structure as the Python version
- Memory management is explicit with `_new()` and `_free()` functions
- Error handling uses return codes rather than exceptions
- The MicroPython wrapper translates between C and Python semantics

## Dependencies

The C implementation has minimal dependencies:
- Standard C library (stdlib, string, etc.)
- No external libraries required

The MicroPython wrapper requires:
- MicroPython development headers
- The C UR implementation