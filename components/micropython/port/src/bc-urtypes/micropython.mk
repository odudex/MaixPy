# bc-urtypes MicroPython module makefile

URTYPES_MOD_DIR := $(USERMOD_DIR)

# Add all C source files
SRC_USERMOD += $(URTYPES_MOD_DIR)/uURTypes.c
SRC_USERMOD += $(URTYPES_MOD_DIR)/src/utils.c
SRC_USERMOD += $(URTYPES_MOD_DIR)/src/cbor_data.c
SRC_USERMOD += $(URTYPES_MOD_DIR)/src/cbor_encoder.c
SRC_USERMOD += $(URTYPES_MOD_DIR)/src/cbor_decoder.c
SRC_USERMOD += $(URTYPES_MOD_DIR)/src/registry.c
SRC_USERMOD += $(URTYPES_MOD_DIR)/src/bytes_type.c
SRC_USERMOD += $(URTYPES_MOD_DIR)/src/psbt.c
SRC_USERMOD += $(URTYPES_MOD_DIR)/src/bip39.c

# Add include paths
CFLAGS_USERMOD += -I$(URTYPES_MOD_DIR)
CFLAGS_USERMOD += -I$(URTYPES_MOD_DIR)/src

# Optional: Add optimization flags
CFLAGS_USERMOD += -O2
