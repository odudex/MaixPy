# Krux Migration Guide: Python urtypes → C uURTypes

This guide shows how to update Krux code to use the C implementation for performance-critical operations.

## Quick Reference

| Python urtypes | C uURTypes | Notes |
|----------------|------------|-------|
| `urtypes.crypto.PSBT` | `uURTypes.PSBT` | ✅ Use C version |
| `urtypes.crypto.BIP39` | `uURTypes.BIP39` | ✅ Use C version |
| `urtypes.Bytes` | `uURTypes.Bytes` | ✅ Use C version |
| `urtypes.bytes.Bytes` | `uURTypes.Bytes` | ✅ Use C version |
| `urtypes.crypto.CRYPTO_PSBT` | `uURTypes.CRYPTO_PSBT` | ✅ Constant available |
| `urtypes.crypto.Output` | `urtypes.crypto.Output` | ⚠️ Keep Python version |
| `urtypes.crypto.Account` | `urtypes.crypto.Account` | ⚠️ Keep Python version |

## File-by-File Changes

### src/krux/psbt.py

```python
# OLD
from urtypes.crypto import CRYPTO_PSBT

# NEW
import uURTypes
CRYPTO_PSBT = uURTypes.CRYPTO_PSBT
```

### src/krux/pages/mnemonic_loader.py

```python
# OLD (line 330)
from urtypes.crypto.bip39 import BIP39
# ...
BIP39.from_cbor(data.cbor).words

# NEW
import uURTypes
# ...
uURTypes.BIP39.from_cbor(data.cbor).words()
```

**Note**: `.words` is now a method `.words()` in the C version.

### src/krux/pages/datum_tool.py

```python
# OLD
from urtypes.crypto.psbt import PSBT
from urtypes.bytes import Bytes

# NEW
import uURTypes
from urtypes.crypto import Output, Account  # Keep Python for these

# In the code:
# OLD: urtypes.crypto.PSBT.from_cbor(psbt_data.cbor).data
# NEW: uURTypes.PSBT.from_cbor(psbt_data.cbor).data()

# OLD: urtypes.Bytes.from_cbor(wallet_data.cbor).data
# NEW: uURTypes.Bytes.from_cbor(wallet_data.cbor).data()

# KEEP PYTHON for Output and Account:
Output.from_cbor(ur_obj.cbor).descriptor()
Account.from_cbor(ur_obj.cbor).output_descriptors[0].descriptor()
```

**Note**: `.data` is now a method `.data()` in the C version.

### tests/pages/test_datum_tool.py

```python
# OLD
from urtypes.crypto.psbt import PSBT
from urtypes.bytes import Bytes

# NEW
import uURTypes
# Reference as uURTypes.PSBT, uURTypes.Bytes
```

### tests/test_qr.py, tests/test_psbt.py

```python
# OLD
from urtypes.crypto.psbt import PSBT

# NEW
import uURTypes
PSBT = uURTypes.PSBT  # or use uURTypes.PSBT directly
```

## Property vs Method Changes

The C implementation uses **methods** instead of **properties** for consistency and C interop:

| Python urtypes | C uURTypes |
|----------------|------------|
| `psbt.data` | `psbt.data()` |
| `bip39.words` | `bip39.words()` |
| `bip39.lang` | `bip39.lang()` |
| `bytes_obj.data` | `bytes_obj.data()` |

## Complete Example: datum_tool.py

### Before

```python
from urtypes.crypto.psbt import PSBT
from urtypes import Bytes
from urtypes.crypto import Output, Account

def process_ur(ur_obj):
    if ur_obj.type == "bytes":
        return urtypes.Bytes.from_cbor(wallet_data.cbor).data
    elif ur_obj.type == "crypto-psbt":
        return urtypes.crypto.PSBT.from_cbor(ur_obj.cbor).data
    elif ur_obj.type == "crypto-account":
        return (
            urtypes.crypto.Account.from_cbor(ur_obj.cbor)
            .output_descriptors[0]
            .descriptor()
        )
    elif ur_obj.type == "crypto-output":
        return urtypes.crypto.Output.from_cbor(ur_obj.cbor).descriptor()
```

### After

```python
import uURTypes
from urtypes.crypto import Output, Account  # Keep Python for these

def process_ur(ur_obj):
    if ur_obj.type == "bytes":
        return uURTypes.Bytes.from_cbor(wallet_data.cbor).data()  # Note: method now
    elif ur_obj.type == "crypto-psbt":
        return uURTypes.PSBT.from_cbor(ur_obj.cbor).data()  # Note: method now
    elif ur_obj.type == "crypto-account":
        # Keep using Python urtypes for complex types
        return (
            Account.from_cbor(ur_obj.cbor)
            .output_descriptors[0]
            .descriptor()
        )
    elif ur_obj.type == "crypto-output":
        # Keep using Python urtypes for complex types
        return Output.from_cbor(ur_obj.cbor).descriptor()
```

## Complete Example: mnemonic_loader.py

### Before

```python
from urtypes.crypto.bip39 import BIP39

def load_mnemonic(data):
    words = BIP39.from_cbor(data.cbor).words
    return words
```

### After

```python
import uURTypes

def load_mnemonic(data):
    words = uURTypes.BIP39.from_cbor(data.cbor).words()  # Note: method now
    return words
```

## Testing Strategy

1. **Build the firmware** with the new module:
   ```bash
   cd firmware/MaixPy
   python3 project.py build
   ```

2. **Test each type individually**:
   ```python
   # On device REPL
   import uURTypes

   # Test PSBT
   psbt = uURTypes.PSBT(b"test_data")
   cbor = psbt.to_cbor()
   psbt2 = uURTypes.PSBT.from_cbor(cbor)
   data = psbt2.data()

   # Test BIP39
   words = ["abandon"] * 12
   bip39 = uURTypes.BIP39(words=words, lang="en")
   cbor = bip39.to_cbor()
   bip39_2 = uURTypes.BIP39.from_cbor(cbor)
   recovered_words = bip39_2.words()
   ```

3. **Test with actual Krux workflows**:
   - Load mnemonic via QR
   - Sign PSBT
   - Display wallet descriptors

## Common Issues

### Issue: AttributeError: 'PSBT' object has no attribute 'data'
**Solution**: Change `psbt.data` to `psbt.data()`

### Issue: AttributeError: 'BIP39' object has no attribute 'words'
**Solution**: Change `bip39.words` to `bip39.words()`

### Issue: Module not found: uURTypes
**Solution**: Ensure `CONFIG_MAIXPY_URTYPES_ENABLE=y` in your project config and rebuild

### Issue: Output/Account errors
**Solution**: Keep using Python `urtypes.crypto` for these types - don't migrate them

## Performance Expectations

Expected improvements with the C implementation:

| Operation | Python urtypes | C uURTypes | Speedup |
|-----------|----------------|------------|---------|
| PSBT decode | ~50ms | ~10ms | 5x |
| BIP39 decode | ~30ms | ~5ms | 6x |
| CBOR encode | ~20ms | ~3ms | 7x |

Actual numbers may vary based on data size.

## Rollback Plan

If issues arise, you can temporarily disable the C module:

1. In `config_defaults.mk`:
   ```makefile
   CONFIG_MAIXPY_URTYPES_ENABLE=n
   ```

2. Revert imports to original Python urtypes

3. Rebuild firmware

## Questions?

See the main [README.md](./README.md) for more details on the implementation.
