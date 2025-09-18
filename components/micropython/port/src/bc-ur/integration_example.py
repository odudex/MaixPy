# Example of how to integrate the C UR decoder in Krux
# This shows the modification needed in src/krux/qr.py

def create_ur_decoder():
    """
    Factory function to create a URDecoder, preferring C implementation
    """
    try:
        # Try to use the C implementation first
        import bc_ur
        return bc_ur.URDecoder()
    except ImportError:
        # Fallback to Python implementation
        from ur.ur_decoder import URDecoder
        return URDecoder()

# Usage example (similar to what's in src/krux/qr.py:180-184)
def process_ur_part_example(data):
    """
    Example of how the URDecoder would be used in QRPartParser
    """
    # In QRPartParser.__init__:
    decoder = create_ur_decoder()

    # In QRPartParser.parse():
    if not decoder:
        decoder = create_ur_decoder()

    # Process the UR part
    success = decoder.receive_part(data)

    if success and decoder.is_complete():
        if decoder.is_success():
            result = decoder.result
            print("Decoded UR: type={}, data_len={}".format(result.type, len(result.cbor)))
            return result
        else:
            print("UR decoding failed")
            return None

    # Show progress
    progress = decoder.estimated_percent_complete()
    processed = decoder.processed_parts_count()
    print("UR Progress: {:.1%} ({} parts processed)".format(progress, processed))

    return None

if __name__ == "__main__":
    # Test the integration
    decoder = create_ur_decoder()
    print("Created decoder: {}".format(type(decoder)))

    # Test with a simple UR part
    test_part = "ur:crypto-psbt/1-2/ableacidalsoapex"
    success = decoder.receive_part(test_part)
    print("Processed part: {}".format(success))
    print("Progress: {:.1%}".format(decoder.estimated_percent_complete()))
    print("Complete: {}".format(decoder.is_complete()))