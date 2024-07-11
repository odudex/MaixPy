
#include "include/font.h"

#include "assert.h"

#include "printf.h"

#include "vfs_spiffs.h"

static uint8_t ascii[] = {};
static uint8_t unicode[] = {
0x01,0x0C,
0x00,0x0A,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x20,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x21,
0x00,0x00,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x10,0x10,0x00,0x00,
0x00,0x22,
0x00,0x24,0x24,0x24,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x23,
0x00,0x00,0x24,0x24,0x24,0x7E,0x24,0x24,0x7E,0x24,0x24,0x24,0x00,0x00,
0x00,0x24,
0x00,0x10,0x10,0x7C,0x92,0x90,0x90,0x7C,0x12,0x12,0x92,0x7C,0x10,0x10,
0x00,0x25,
0x00,0x00,0x64,0x94,0x68,0x08,0x10,0x10,0x20,0x2C,0x52,0x4C,0x00,0x00,
0x00,0x26,
0x00,0x00,0x18,0x24,0x24,0x18,0x30,0x4A,0x44,0x44,0x44,0x3A,0x00,0x00,
0x00,0x27,
0x00,0x10,0x10,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x28,
0x00,0x00,0x08,0x10,0x20,0x20,0x20,0x20,0x20,0x20,0x10,0x08,0x00,0x00,
0x00,0x29,
0x00,0x00,0x20,0x10,0x08,0x08,0x08,0x08,0x08,0x08,0x10,0x20,0x00,0x00,
0x00,0x2A,
0x00,0x00,0x00,0x00,0x00,0x24,0x18,0x7E,0x18,0x24,0x00,0x00,0x00,0x00,
0x00,0x2B,
0x00,0x00,0x00,0x00,0x00,0x10,0x10,0x7C,0x10,0x10,0x00,0x00,0x00,0x00,
0x00,0x2C,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x10,0x20,0x00,
0x00,0x2D,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x2E,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x10,0x00,0x00,
0x00,0x2F,
0x00,0x00,0x04,0x04,0x08,0x08,0x10,0x10,0x20,0x20,0x40,0x40,0x00,0x00,
0x00,0x30,
0x00,0x00,0x3C,0x42,0x42,0x46,0x4A,0x52,0x62,0x42,0x42,0x3C,0x00,0x00,
0x00,0x31,
0x00,0x00,0x08,0x18,0x28,0x08,0x08,0x08,0x08,0x08,0x08,0x3E,0x00,0x00,
0x00,0x32,
0x00,0x00,0x3C,0x42,0x42,0x02,0x04,0x08,0x10,0x20,0x40,0x7E,0x00,0x00,
0x00,0x33,
0x00,0x00,0x3C,0x42,0x42,0x02,0x1C,0x02,0x02,0x42,0x42,0x3C,0x00,0x00,
0x00,0x34,
0x00,0x00,0x02,0x06,0x0A,0x12,0x22,0x42,0x7E,0x02,0x02,0x02,0x00,0x00,
0x00,0x35,
0x00,0x00,0x7E,0x40,0x40,0x40,0x7C,0x02,0x02,0x02,0x42,0x3C,0x00,0x00,
0x00,0x36,
0x00,0x00,0x1C,0x20,0x40,0x40,0x7C,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
0x00,0x37,
0x00,0x00,0x7E,0x02,0x02,0x04,0x04,0x08,0x08,0x10,0x10,0x10,0x00,0x00,
0x00,0x38,
0x00,0x00,0x3C,0x42,0x42,0x42,0x3C,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
0x00,0x39,
0x00,0x00,0x3C,0x42,0x42,0x42,0x42,0x3E,0x02,0x02,0x04,0x38,0x00,0x00,
0x00,0x3A,
0x00,0x00,0x00,0x00,0x00,0x10,0x10,0x00,0x00,0x00,0x10,0x10,0x00,0x00,
0x00,0x3B,
0x00,0x00,0x00,0x00,0x00,0x10,0x10,0x00,0x00,0x00,0x10,0x10,0x20,0x00,
0x00,0x3C,
0x00,0x00,0x00,0x04,0x08,0x10,0x20,0x40,0x20,0x10,0x08,0x04,0x00,0x00,
0x00,0x3D,
0x00,0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,
0x00,0x3E,
0x00,0x00,0x00,0x40,0x20,0x10,0x08,0x04,0x08,0x10,0x20,0x40,0x00,0x00,
0x00,0x3F,
0x00,0x00,0x3C,0x42,0x42,0x42,0x04,0x08,0x08,0x00,0x08,0x08,0x00,0x00,
0x00,0x40,
0x00,0x00,0x7C,0x82,0x9E,0xA2,0xA2,0xA2,0xA6,0x9A,0x80,0x7E,0x00,0x00,
0x00,0x41,
0x00,0x00,0x3C,0x42,0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x42,0x00,0x00,
0x00,0x42,
0x00,0x00,0x7C,0x42,0x42,0x42,0x7C,0x42,0x42,0x42,0x42,0x7C,0x00,0x00,
0x00,0x43,
0x00,0x00,0x3C,0x42,0x42,0x40,0x40,0x40,0x40,0x42,0x42,0x3C,0x00,0x00,
0x00,0x44,
0x00,0x00,0x78,0x44,0x42,0x42,0x42,0x42,0x42,0x42,0x44,0x78,0x00,0x00,
0x00,0x45,
0x00,0x00,0x7E,0x40,0x40,0x40,0x78,0x40,0x40,0x40,0x40,0x7E,0x00,0x00,
0x00,0x46,
0x00,0x00,0x7E,0x40,0x40,0x40,0x78,0x40,0x40,0x40,0x40,0x40,0x00,0x00,
0x00,0x47,
0x00,0x00,0x3C,0x42,0x42,0x40,0x40,0x4E,0x42,0x42,0x42,0x3C,0x00,0x00,
0x00,0x48,
0x00,0x00,0x42,0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x42,0x42,0x00,0x00,
0x00,0x49,
0x00,0x00,0x38,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x38,0x00,0x00,
0x00,0x4A,
0x00,0x00,0x0E,0x04,0x04,0x04,0x04,0x04,0x04,0x44,0x44,0x38,0x00,0x00,
0x00,0x4B,
0x00,0x00,0x42,0x44,0x48,0x50,0x60,0x60,0x50,0x48,0x44,0x42,0x00,0x00,
0x00,0x4C,
0x00,0x00,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x7E,0x00,0x00,
0x00,0x4D,
0x00,0x00,0x82,0xC6,0xAA,0x92,0x92,0x82,0x82,0x82,0x82,0x82,0x00,0x00,
0x00,0x4E,
0x00,0x00,0x42,0x42,0x42,0x62,0x52,0x4A,0x46,0x42,0x42,0x42,0x00,0x00,
0x00,0x4F,
0x00,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
0x00,0x50,
0x00,0x00,0x7C,0x42,0x42,0x42,0x42,0x7C,0x40,0x40,0x40,0x40,0x00,0x00,
0x00,0x51,
0x00,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x4A,0x3C,0x02,0x00,
0x00,0x52,
0x00,0x00,0x7C,0x42,0x42,0x42,0x42,0x7C,0x50,0x48,0x44,0x42,0x00,0x00,
0x00,0x53,
0x00,0x00,0x3C,0x42,0x40,0x40,0x3C,0x02,0x02,0x42,0x42,0x3C,0x00,0x00,
0x00,0x54,
0x00,0x00,0xFE,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x00,
0x00,0x55,
0x00,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
0x00,0x56,
0x00,0x00,0x42,0x42,0x42,0x42,0x42,0x24,0x24,0x24,0x18,0x18,0x00,0x00,
0x00,0x57,
0x00,0x00,0x82,0x82,0x82,0x82,0x82,0x92,0x92,0xAA,0xC6,0x82,0x00,0x00,
0x00,0x58,
0x00,0x00,0x42,0x42,0x24,0x24,0x18,0x18,0x24,0x24,0x42,0x42,0x00,0x00,
0x00,0x59,
0x00,0x00,0x82,0x82,0x44,0x44,0x28,0x10,0x10,0x10,0x10,0x10,0x00,0x00,
0x00,0x5A,
0x00,0x00,0x7E,0x02,0x02,0x04,0x08,0x10,0x20,0x40,0x40,0x7E,0x00,0x00,
0x00,0x5B,
0x00,0x00,0x38,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x38,0x00,0x00,
0x00,0x5C,
0x00,0x00,0x40,0x40,0x20,0x20,0x10,0x10,0x08,0x08,0x04,0x04,0x00,0x00,
0x00,0x5D,
0x00,0x00,0x38,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x38,0x00,0x00,
0x00,0x5E,
0x00,0x10,0x28,0x44,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x5F,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x00,
0x00,0x61,
0x00,0x00,0x00,0x00,0x00,0x3C,0x02,0x3E,0x42,0x42,0x42,0x3E,0x00,0x00,
0x00,0x62,
0x00,0x00,0x40,0x40,0x40,0x7C,0x42,0x42,0x42,0x42,0x42,0x7C,0x00,0x00,
0x00,0x63,
0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x40,0x40,0x40,0x42,0x3C,0x00,0x00,
0x00,0x64,
0x00,0x00,0x02,0x02,0x02,0x3E,0x42,0x42,0x42,0x42,0x42,0x3E,0x00,0x00,
0x00,0x65,
0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x7E,0x40,0x40,0x3C,0x00,0x00,
0x00,0x66,
0x00,0x00,0x0E,0x10,0x10,0x7C,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x00,
0x00,0x67,
0x00,0x00,0x00,0x00,0x00,0x3E,0x42,0x42,0x42,0x42,0x42,0x3E,0x02,0x3C,
0x00,0x68,
0x00,0x00,0x40,0x40,0x40,0x7C,0x42,0x42,0x42,0x42,0x42,0x42,0x00,0x00,
0x00,0x69,
0x00,0x00,0x10,0x10,0x00,0x30,0x10,0x10,0x10,0x10,0x10,0x38,0x00,0x00,
0x00,0x6A,
0x00,0x00,0x04,0x04,0x00,0x0C,0x04,0x04,0x04,0x04,0x04,0x44,0x44,0x38,
0x00,0x6B,
0x00,0x00,0x40,0x40,0x40,0x42,0x44,0x48,0x70,0x48,0x44,0x42,0x00,0x00,
0x00,0x6C,
0x00,0x00,0x30,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x38,0x00,0x00,
0x00,0x6D,
0x00,0x00,0x00,0x00,0x00,0xFC,0x92,0x92,0x92,0x92,0x92,0x92,0x00,0x00,
0x00,0x6E,
0x00,0x00,0x00,0x00,0x00,0x7C,0x42,0x42,0x42,0x42,0x42,0x42,0x00,0x00,
0x00,0x6F,
0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
0x00,0x70,
0x00,0x00,0x00,0x00,0x00,0x7C,0x42,0x42,0x42,0x42,0x42,0x7C,0x40,0x40,
0x00,0x71,
0x00,0x00,0x00,0x00,0x00,0x3E,0x42,0x42,0x42,0x42,0x42,0x3E,0x02,0x02,
0x00,0x72,
0x00,0x00,0x00,0x00,0x00,0x5E,0x60,0x40,0x40,0x40,0x40,0x40,0x00,0x00,
0x00,0x73,
0x00,0x00,0x00,0x00,0x00,0x3E,0x40,0x40,0x3C,0x02,0x02,0x7C,0x00,0x00,
0x00,0x74,
0x00,0x00,0x10,0x10,0x10,0x7C,0x10,0x10,0x10,0x10,0x10,0x0E,0x00,0x00,
0x00,0x75,
0x00,0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x3E,0x00,0x00,
0x00,0x76,
0x00,0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x24,0x24,0x18,0x18,0x00,0x00,
0x00,0x77,
0x00,0x00,0x00,0x00,0x00,0x82,0x82,0x92,0x92,0x92,0x92,0x7C,0x00,0x00,
0x00,0x78,
0x00,0x00,0x00,0x00,0x00,0x42,0x42,0x24,0x18,0x24,0x42,0x42,0x00,0x00,
0x00,0x79,
0x00,0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x3E,0x02,0x3C,
0x00,0x7A,
0x00,0x00,0x00,0x00,0x00,0x7E,0x04,0x08,0x10,0x20,0x40,0x7E,0x00,0x00,
0x00,0x7B,
0x00,0x00,0x0C,0x10,0x10,0x10,0x20,0x10,0x10,0x10,0x10,0x0C,0x00,0x00,
0x00,0x7C,
0x00,0x00,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x00,
0x00,0x7D,
0x00,0x00,0x30,0x08,0x08,0x08,0x04,0x08,0x08,0x08,0x08,0x30,0x00,0x00,
0x00,0x7E,
0x00,0x00,0x00,0x00,0x00,0x00,0x62,0x92,0x8C,0x00,0x00,0x00,0x00,0x00,
0x00,0xA1,
0x00,0x00,0x10,0x10,0x00,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x00,
0x00,0xBF,
0x00,0x00,0x10,0x10,0x00,0x10,0x10,0x20,0x42,0x42,0x42,0x3C,0x00,0x00,
0x00,0xC0,
0x10,0x08,0x00,0x3C,0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x42,0x00,0x00,
0x00,0xC3,
0x32,0x4C,0x00,0x3C,0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x42,0x00,0x00,
0x00,0xC4,
0x24,0x24,0x00,0x3C,0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x42,0x00,0x00,
0x00,0xC7,
0x00,0x00,0x3C,0x42,0x42,0x40,0x40,0x40,0x40,0x42,0x42,0x3C,0x10,0x20,
0x00,0xC9,
0x08,0x10,0x00,0x7E,0x40,0x40,0x40,0x78,0x40,0x40,0x40,0x7E,0x00,0x00,
0x00,0xCC,
0x20,0x10,0x00,0x38,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x38,0x00,0x00,
0x00,0xD3,
0x08,0x10,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
0x00,0xD4,
0x18,0x24,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
0x00,0xD6,
0x24,0x24,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
0x00,0xDC,
0x24,0x24,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
0x00,0xDF,
0x00,0x00,0x38,0x44,0x44,0x48,0x7C,0x42,0x42,0x42,0x62,0x5C,0x00,0x00,
0x00,0xE0,
0x00,0x00,0x10,0x08,0x00,0x3C,0x02,0x3E,0x42,0x42,0x42,0x3E,0x00,0x00,
0x00,0xE1,
0x00,0x00,0x08,0x10,0x00,0x3C,0x02,0x3E,0x42,0x42,0x42,0x3E,0x00,0x00,
0x00,0xE2,
0x00,0x00,0x18,0x24,0x00,0x3C,0x02,0x3E,0x42,0x42,0x42,0x3E,0x00,0x00,
0x00,0xE3,
0x00,0x00,0x32,0x4C,0x00,0x3C,0x02,0x3E,0x42,0x42,0x42,0x3E,0x00,0x00,
0x00,0xE4,
0x00,0x00,0x24,0x24,0x00,0x3C,0x02,0x3E,0x42,0x42,0x42,0x3E,0x00,0x00,
0x00,0xE7,
0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x40,0x40,0x40,0x42,0x3C,0x10,0x20,
0x00,0xE8,
0x00,0x00,0x10,0x08,0x00,0x3C,0x42,0x42,0x7E,0x40,0x40,0x3C,0x00,0x00,
0x00,0xE9,
0x00,0x00,0x08,0x10,0x00,0x3C,0x42,0x42,0x7E,0x40,0x40,0x3C,0x00,0x00,
0x00,0xEA,
0x00,0x00,0x18,0x24,0x00,0x3C,0x42,0x42,0x7E,0x40,0x40,0x3C,0x00,0x00,
0x00,0xEC,
0x00,0x00,0x20,0x10,0x00,0x30,0x10,0x10,0x10,0x10,0x10,0x38,0x00,0x00,
0x00,0xED,
0x00,0x00,0x08,0x10,0x00,0x30,0x10,0x10,0x10,0x10,0x10,0x38,0x00,0x00,
0x00,0xF1,
0x00,0x00,0x32,0x4C,0x00,0x7C,0x42,0x42,0x42,0x42,0x42,0x42,0x00,0x00,
0x00,0xF2,
0x00,0x00,0x10,0x08,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
0x00,0xF3,
0x00,0x00,0x08,0x10,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
0x00,0xF4,
0x00,0x00,0x18,0x24,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
0x00,0xF5,
0x00,0x00,0x32,0x4C,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
0x00,0xF6,
0x00,0x00,0x24,0x24,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
0x00,0xF9,
0x00,0x00,0x10,0x08,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x3E,0x00,0x00,
0x00,0xFA,
0x00,0x00,0x08,0x10,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x3E,0x00,0x00,
0x00,0xFB,
0x00,0x00,0x18,0x24,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x3E,0x00,0x00,
0x00,0xFC,
0x00,0x00,0x24,0x24,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x3E,0x00,0x00,
0x00,0xFD,
0x00,0x00,0x08,0x10,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x3E,0x02,0x3C,
0x01,0x03,
0x00,0x00,0x24,0x18,0x00,0x3C,0x02,0x3E,0x42,0x42,0x42,0x3E,0x00,0x00,
0x01,0x04,
0x00,0x00,0x3C,0x42,0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x42,0x04,0x03,
0x01,0x05,
0x00,0x00,0x00,0x00,0x00,0x3C,0x02,0x3E,0x42,0x42,0x42,0x3E,0x04,0x03,
0x01,0x06,
0x08,0x10,0x00,0x3C,0x42,0x42,0x40,0x40,0x40,0x42,0x42,0x3C,0x00,0x00,
0x01,0x07,
0x00,0x00,0x08,0x10,0x00,0x3C,0x42,0x40,0x40,0x40,0x42,0x3C,0x00,0x00,
0x01,0x10,
0x00,0x00,0x78,0x44,0x42,0x42,0xF2,0x42,0x42,0x42,0x44,0x78,0x00,0x00,
0x01,0x11,
0x00,0x00,0x02,0x0F,0x02,0x3E,0x42,0x42,0x42,0x42,0x42,0x3E,0x00,0x00,
0x01,0x18,
0x00,0x00,0x7E,0x40,0x40,0x40,0x78,0x40,0x40,0x40,0x40,0x7E,0x04,0x03,
0x01,0x19,
0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x7E,0x40,0x40,0x3C,0x08,0x06,
0x01,0x1E,
0x24,0x18,0x00,0x3C,0x42,0x40,0x40,0x4E,0x42,0x42,0x42,0x3C,0x00,0x00,
0x01,0x1F,
0x00,0x00,0x24,0x18,0x00,0x3E,0x42,0x42,0x42,0x42,0x42,0x3E,0x02,0x3C,
0x01,0x30,
0x10,0x10,0x00,0x38,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x38,0x00,0x00,
0x01,0x31,
0x00,0x00,0x00,0x00,0x00,0x30,0x10,0x10,0x10,0x10,0x10,0x38,0x00,0x00,
0x01,0x41,
0x00,0x00,0x40,0x40,0x40,0x40,0x60,0xC0,0x40,0x40,0x40,0x7E,0x00,0x00,
0x01,0x42,
0x00,0x00,0x30,0x10,0x10,0x10,0x18,0x30,0x10,0x10,0x10,0x38,0x00,0x00,
0x01,0x43,
0x08,0x10,0x42,0x42,0x42,0x62,0x52,0x4A,0x46,0x42,0x42,0x42,0x00,0x00,
0x01,0x44,
0x00,0x00,0x08,0x10,0x00,0x7C,0x42,0x42,0x42,0x42,0x42,0x42,0x00,0x00,
0x01,0x5A,
0x08,0x10,0x00,0x3C,0x42,0x40,0x40,0x3C,0x02,0x02,0x42,0x3C,0x00,0x00,
0x01,0x5B,
0x00,0x00,0x04,0x08,0x00,0x3E,0x40,0x40,0x3C,0x02,0x02,0x7C,0x00,0x00,
0x01,0x5E,
0x00,0x00,0x3C,0x42,0x40,0x40,0x3C,0x02,0x02,0x42,0x42,0x3C,0x10,0x20,
0x01,0x5F,
0x00,0x00,0x00,0x00,0x00,0x3E,0x40,0x40,0x3C,0x02,0x02,0x7C,0x10,0x20,
0x01,0x79,
0x08,0x10,0x00,0x7E,0x02,0x04,0x08,0x10,0x20,0x40,0x40,0x7E,0x00,0x00,
0x01,0x7A,
0x00,0x00,0x08,0x10,0x00,0x7E,0x04,0x08,0x10,0x20,0x40,0x7E,0x00,0x00,
0x01,0x7B,
0x10,0x10,0x00,0x7E,0x02,0x04,0x08,0x10,0x20,0x40,0x40,0x7E,0x00,0x00,
0x01,0x7C,
0x00,0x00,0x10,0x10,0x00,0x7E,0x04,0x08,0x10,0x20,0x40,0x7E,0x00,0x00,
0x01,0xA1,
0x00,0x00,0x00,0x00,0x00,0x7A,0x86,0x84,0x84,0x84,0x84,0x78,0x00,0x00,
0x01,0xAF,
0x00,0x02,0x86,0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x78,0x00,0x00,
0x01,0xB0,
0x00,0x00,0x00,0x00,0x02,0x86,0x84,0x84,0x84,0x84,0x84,0x7C,0x00,0x00,
0x04,0x01,
0x24,0x24,0x00,0x7E,0x40,0x40,0x40,0x78,0x40,0x40,0x40,0x7E,0x00,0x00,
0x04,0x10,
0x00,0x00,0x3C,0x42,0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x42,0x00,0x00,
0x04,0x11,
0x00,0x00,0x7C,0x40,0x40,0x7C,0x42,0x42,0x42,0x42,0x42,0x7C,0x00,0x00,
0x04,0x12,
0x00,0x00,0x7C,0x42,0x42,0x42,0x7C,0x42,0x42,0x42,0x42,0x7C,0x00,0x00,
0x04,0x13,
0x00,0x00,0x7E,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x00,0x00,
0x04,0x14,
0x00,0x00,0x3C,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0xFE,0x82,0x00,
0x04,0x15,
0x00,0x00,0x7E,0x40,0x40,0x40,0x78,0x40,0x40,0x40,0x40,0x7E,0x00,0x00,
0x04,0x16,
0x00,0x00,0x92,0x92,0x92,0x54,0x38,0x54,0x92,0x92,0x92,0x92,0x00,0x00,
0x04,0x17,
0x00,0x00,0x3C,0x42,0x42,0x02,0x1C,0x02,0x02,0x42,0x42,0x3C,0x00,0x00,
0x04,0x18,
0x00,0x00,0x42,0x42,0x42,0x46,0x4A,0x52,0x62,0x42,0x42,0x42,0x00,0x00,
0x04,0x19,
0x24,0x18,0x42,0x42,0x42,0x46,0x4A,0x52,0x62,0x42,0x42,0x42,0x00,0x00,
0x04,0x1A,
0x00,0x00,0x42,0x44,0x48,0x50,0x60,0x60,0x50,0x48,0x44,0x42,0x00,0x00,
0x04,0x1B,
0x00,0x00,0x0E,0x12,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x42,0x00,0x00,
0x04,0x1C,
0x00,0x00,0x82,0xC6,0xAA,0x92,0x92,0x82,0x82,0x82,0x82,0x82,0x00,0x00,
0x04,0x1D,
0x00,0x00,0x42,0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x42,0x42,0x00,0x00,
0x04,0x1E,
0x00,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
0x04,0x1F,
0x00,0x00,0x7E,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x00,0x00,
0x04,0x20,
0x00,0x00,0x7C,0x42,0x42,0x42,0x42,0x7C,0x40,0x40,0x40,0x40,0x00,0x00,
0x04,0x21,
0x00,0x00,0x3C,0x42,0x42,0x40,0x40,0x40,0x40,0x42,0x42,0x3C,0x00,0x00,
0x04,0x22,
0x00,0x00,0xFE,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x00,
0x04,0x23,
0x00,0x00,0x42,0x42,0x42,0x42,0x42,0x3E,0x02,0x02,0x02,0x3C,0x00,0x00,
0x04,0x24,
0x00,0x10,0x7C,0x92,0x92,0x92,0x92,0x92,0x92,0x92,0x92,0x7C,0x10,0x00,
0x04,0x25,
0x00,0x00,0x42,0x42,0x24,0x24,0x18,0x18,0x24,0x24,0x42,0x42,0x00,0x00,
0x04,0x26,
0x00,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x3F,0x01,0x01,
0x04,0x27,
0x00,0x00,0x42,0x42,0x42,0x42,0x42,0x3E,0x02,0x02,0x02,0x02,0x00,0x00,
0x04,0x28,
0x00,0x00,0x92,0x92,0x92,0x92,0x92,0x92,0x92,0x92,0x92,0x7E,0x00,0x00,
0x04,0x29,
0x00,0x00,0x92,0x92,0x92,0x92,0x92,0x92,0x92,0x92,0x92,0x7F,0x01,0x01,
0x04,0x2A,
0x00,0x00,0xC0,0x40,0x40,0x7C,0x42,0x42,0x42,0x42,0x42,0x7C,0x00,0x00,
0x04,0x2B,
0x00,0x00,0x82,0x82,0x82,0xF2,0x8A,0x8A,0x8A,0x8A,0x8A,0xF2,0x00,0x00,
0x04,0x2C,
0x00,0x00,0x40,0x40,0x40,0x7C,0x42,0x42,0x42,0x42,0x42,0x7C,0x00,0x00,
0x04,0x2D,
0x00,0x00,0x3C,0x42,0x02,0x02,0x1E,0x02,0x02,0x02,0x42,0x3C,0x00,0x00,
0x04,0x2E,
0x00,0x00,0x8C,0x92,0x92,0x92,0x92,0xF2,0x92,0x92,0x92,0x8C,0x00,0x00,
0x04,0x2F,
0x00,0x00,0x3E,0x42,0x42,0x42,0x42,0x3E,0x0A,0x12,0x22,0x42,0x00,0x00,
0x04,0x30,
0x00,0x00,0x00,0x00,0x00,0x3C,0x02,0x3E,0x42,0x42,0x42,0x3E,0x00,0x00,
0x04,0x31,
0x00,0x00,0x3C,0x40,0x40,0x7C,0x42,0x42,0x42,0x42,0x42,0x7C,0x00,0x00,
0x04,0x32,
0x00,0x00,0x00,0x00,0x00,0x7C,0x42,0x42,0x7C,0x42,0x42,0x7C,0x00,0x00,
0x04,0x33,
0x00,0x00,0x00,0x00,0x00,0x7E,0x40,0x40,0x40,0x40,0x40,0x40,0x00,0x00,
0x04,0x34,
0x00,0x00,0x00,0x00,0x00,0x3C,0x44,0x44,0x44,0x44,0x44,0xFE,0x82,0x00,
0x04,0x35,
0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x7E,0x40,0x40,0x3C,0x00,0x00,
0x04,0x36,
0x00,0x00,0x00,0x00,0x00,0x92,0x92,0x54,0x38,0x54,0x92,0x92,0x00,0x00,
0x04,0x37,
0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x02,0x1C,0x02,0x42,0x3C,0x00,0x00,
0x04,0x38,
0x00,0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x3E,0x00,0x00,
0x04,0x39,
0x00,0x00,0x24,0x18,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x3E,0x00,0x00,
0x04,0x3A,
0x00,0x00,0x00,0x00,0x00,0x42,0x44,0x48,0x70,0x48,0x44,0x42,0x00,0x00,
0x04,0x3B,
0x00,0x00,0x00,0x00,0x00,0x1E,0x22,0x22,0x22,0x22,0x22,0x42,0x00,0x00,
0x04,0x3C,
0x00,0x00,0x00,0x00,0x00,0x82,0xC6,0xAA,0x92,0x82,0x82,0x82,0x00,0x00,
0x04,0x3D,
0x00,0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x00,0x00,
0x04,0x3E,
0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
0x04,0x3F,
0x00,0x00,0x00,0x00,0x00,0x7E,0x42,0x42,0x42,0x42,0x42,0x42,0x00,0x00,
0x04,0x40,
0x00,0x00,0x00,0x00,0x00,0x7C,0x42,0x42,0x42,0x42,0x42,0x7C,0x40,0x40,
0x04,0x41,
0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x40,0x40,0x40,0x42,0x3C,0x00,0x00,
0x04,0x42,
0x00,0x00,0x00,0x00,0x00,0xFE,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x00,
0x04,0x43,
0x00,0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x3E,0x02,0x3C,
0x04,0x44,
0x00,0x00,0x00,0x00,0x10,0x7C,0x92,0x92,0x92,0x92,0x92,0x7C,0x10,0x00,
0x04,0x45,
0x00,0x00,0x00,0x00,0x00,0x42,0x42,0x24,0x18,0x24,0x42,0x42,0x00,0x00,
0x04,0x46,
0x00,0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x3F,0x01,0x01,
0x04,0x47,
0x00,0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x3E,0x02,0x02,0x02,0x00,0x00,
0x04,0x48,
0x00,0x00,0x00,0x00,0x00,0x92,0x92,0x92,0x92,0x92,0x92,0x7E,0x00,0x00,
0x04,0x49,
0x00,0x00,0x00,0x00,0x00,0x92,0x92,0x92,0x92,0x92,0x92,0x7F,0x01,0x01,
0x04,0x4A,
0x00,0x00,0x00,0x00,0x00,0x60,0x20,0x3C,0x22,0x22,0x22,0x3C,0x00,0x00,
0x04,0x4B,
0x00,0x00,0x00,0x00,0x00,0x82,0x82,0xF2,0x8A,0x8A,0x8A,0xF2,0x00,0x00,
0x04,0x4C,
0x00,0x00,0x00,0x00,0x00,0x40,0x40,0x78,0x44,0x44,0x44,0x78,0x00,0x00,
0x04,0x4D,
0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x02,0x1E,0x02,0x42,0x3C,0x00,0x00,
0x04,0x4E,
0x00,0x00,0x00,0x00,0x00,0x8C,0x92,0x92,0xF2,0x92,0x92,0x8C,0x00,0x00,
0x04,0x4F,
0x00,0x00,0x00,0x00,0x00,0x3E,0x42,0x42,0x3E,0x12,0x22,0x42,0x00,0x00,
0x04,0x51,
0x00,0x00,0x24,0x24,0x00,0x3C,0x42,0x42,0x7E,0x40,0x40,0x3C,0x00,0x00,
0x1E,0xA1,
0x00,0x00,0x00,0x00,0x00,0x78,0x04,0x7C,0x84,0x84,0x84,0x7C,0x20,0x20,
0x1E,0xA3,
0x00,0x00,0x70,0x10,0x00,0x78,0x04,0x7C,0x84,0x84,0x84,0x7C,0x00,0x00,
0x1E,0xA5,
0x00,0x04,0x38,0x48,0x00,0x78,0x04,0x7C,0x84,0x84,0x84,0x7C,0x00,0x00,
0x1E,0xA7,
0x00,0x80,0x70,0x48,0x00,0x78,0x04,0x7C,0x84,0x84,0x84,0x7C,0x00,0x00,
0x1E,0xA9,
0x00,0x0C,0x34,0x48,0x00,0x78,0x04,0x7C,0x84,0x84,0x84,0x7C,0x00,0x00,
0x1E,0xAD,
0x00,0x00,0x30,0x48,0x00,0x78,0x04,0x7C,0x84,0x84,0x84,0x7C,0x20,0x20,
0x1E,0xAF,
0x08,0x10,0x48,0x30,0x00,0x78,0x04,0x7C,0x84,0x84,0x84,0x7C,0x00,0x00,
0x1E,0xB7,
0x00,0x00,0x48,0x30,0x00,0x78,0x04,0x7C,0x84,0x84,0x84,0x7C,0x20,0x20,
0x1E,0xBB,
0x00,0x38,0x08,0x10,0x00,0x78,0x84,0x84,0xFC,0x80,0x80,0x78,0x00,0x00,
0x1E,0xBD,
0x00,0x00,0x32,0x4C,0x00,0x3C,0x42,0x42,0x7E,0x40,0x40,0x3C,0x00,0x00,
0x1E,0xBF,
0x04,0x08,0x30,0x48,0x00,0x78,0x84,0x84,0xFC,0x80,0x80,0x78,0x00,0x00,
0x1E,0xC1,
0x10,0x08,0x30,0x48,0x00,0x78,0x84,0x84,0xFC,0x80,0x80,0x78,0x00,0x00,
0x1E,0xC3,
0x0C,0x04,0x38,0x48,0x00,0x78,0x84,0x84,0xFC,0x80,0x80,0x78,0x00,0x00,
0x1E,0xC5,
0x00,0x0A,0x34,0x48,0x00,0x78,0x84,0x84,0xFC,0x80,0x80,0x78,0x00,0x00,
0x1E,0xC7,
0x00,0x00,0x30,0x48,0x00,0x78,0x84,0x84,0xFC,0x80,0x80,0x78,0x20,0x20,
0x1E,0xC9,
0x00,0xE0,0x20,0x40,0x00,0xC0,0x40,0x40,0x40,0x40,0x40,0xE0,0x00,0x00,
0x1E,0xCB,
0x00,0x00,0x10,0x10,0x00,0x30,0x10,0x10,0x10,0x10,0x10,0x38,0x10,0x10,
0x1E,0xCD,
0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x3C,0x10,0x10,
0x1E,0xCF,
0x00,0x38,0x08,0x10,0x00,0x78,0x84,0x84,0x84,0x84,0x84,0x78,0x00,0x00,
0x1E,0xD1,
0x04,0x08,0x30,0x48,0x00,0x78,0x84,0x84,0x84,0x84,0x84,0x78,0x00,0x00,
0x1E,0xD3,
0x10,0x08,0x30,0x48,0x00,0x78,0x84,0x84,0x84,0x84,0x84,0x78,0x00,0x00,
0x1E,0xD5,
0x1C,0x04,0x38,0x48,0x00,0x78,0x84,0x84,0x84,0x84,0x84,0x78,0x00,0x00,
0x1E,0xD7,
0x00,0x0A,0x34,0x48,0x00,0x78,0x84,0x84,0x84,0x84,0x84,0x78,0x00,0x00,
0x1E,0xD9,
0x00,0x00,0x30,0x48,0x00,0x78,0x84,0x84,0x84,0x84,0x84,0x78,0x20,0x20,
0x1E,0xDB,
0x00,0x00,0x08,0x10,0x00,0x7A,0x86,0x84,0x84,0x84,0x84,0x78,0x00,0x00,
0x1E,0xDD,
0x00,0x00,0x20,0x10,0x00,0x7A,0x86,0x84,0x84,0x84,0x84,0x78,0x00,0x00,
0x1E,0xDF,
0x00,0x38,0x08,0x10,0x00,0x7A,0x86,0x84,0x84,0x84,0x84,0x78,0x00,0x00,
0x1E,0xE1,
0x00,0x00,0x14,0x28,0x00,0x7A,0x86,0x84,0x84,0x84,0x84,0x78,0x00,0x00,
0x1E,0xE2,
0x00,0x00,0x7A,0x86,0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x78,0x20,0x20,
0x1E,0xE3,
0x00,0x00,0x00,0x00,0x00,0x7A,0x86,0x84,0x84,0x84,0x84,0x78,0x20,0x20,
0x1E,0xE5,
0x00,0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x3E,0x08,0x08,
0x1E,0xE7,
0x00,0x38,0x08,0x10,0x00,0x84,0x84,0x84,0x84,0x84,0x84,0x7C,0x00,0x00,
0x1E,0xE9,
0x00,0x00,0x08,0x10,0x02,0x86,0x84,0x84,0x84,0x84,0x84,0x7C,0x00,0x00,
0x1E,0xEB,
0x00,0x00,0x20,0x10,0x02,0x86,0x84,0x84,0x84,0x84,0x84,0x7C,0x00,0x00,
0x1E,0xED,
0x00,0x38,0x08,0x10,0x02,0x86,0x84,0x84,0x84,0x84,0x84,0x7C,0x00,0x00,
0x1E,0xEF,
0x00,0x00,0x14,0x28,0x02,0x86,0x84,0x84,0x84,0x84,0x84,0x7C,0x00,0x00,
0x1E,0xF1,
0x00,0x00,0x00,0x00,0x02,0x86,0x84,0x84,0x84,0x84,0x84,0x7C,0x20,0x20,
0x1E,0xF7,
0x00,0x38,0x08,0x10,0x00,0x84,0x84,0x84,0x84,0x84,0x84,0x7C,0x04,0x78,
0x20,0x09,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x20,0xBD,
0x00,0x00,0x3E,0x21,0x21,0x21,0x3E,0x20,0xFC,0x20,0xFC,0x20,0x20,0x00,
0x20,0xBF,
0x28,0x28,0xFC,0x66,0x62,0x66,0x7C,0x62,0x63,0x63,0x63,0xFE,0x28,0x28,
0x21,0xB3,
0xF0,0xF0,0xF0,0xF4,0x66,0x7F,0x66,0x64,0x60,0x64,0x66,0x7F,0x06,0x04,
0x22,0x9A,
0x3C,0x66,0x03,0x79,0xCD,0x85,0xAD,0xB9,0x83,0xEE,0x38,0x83,0xE6,0x3C,
0x25,0x88,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};

struct font
{
  /* data */
  uint8_t width;
  uint8_t high;
  uint8_t index;
  uint8_t source;
  void *this;
} font_config = {
  8, 14, Unicode, BuildIn, unicode
};

static inline void font_init(uint8_t width, uint8_t high, uint8_t index, uint8_t source_type, void *font_offset)
{
  struct font tmp = { width, high, index, source_type, font_offset};
  font_config = tmp;
}

#include "vfs_wrapper.h"
#include "nlr.h"

void font_free()
{
  switch (font_config.index)
  {
    case UTF8:
    case Unicode:
        if (font_config.source == FileIn)
        {
            file_close(font_config.this);
        }
    case GBK:
    case GB2312:
    case ASCII:
    default:
      font_init(8, 12, ASCII, BuildIn, ascii);
      break;
  }
}

void font_load(uint8_t index, uint8_t width, uint8_t high, uint8_t source_type, void *src_addr)
{
    switch (index)
    {
    case UTF8:
        if (src_addr == NULL)
        {
            font_init(8, 12, ASCII, BuildIn, ascii);
            break;
        }
        font_init(width, high, UTF8, source_type, src_addr);
    break;
    default:
    case Unicode:
    case GBK:
    case GB2312:
    case ASCII:
        font_init(8, 12, ASCII, BuildIn, ascii);
    break;
    }
}

int font_width()
{
	return font_config.width;
}

int font_height()
{
	return font_config.high;
}

// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
#define UTF8_ACCEPT 0
#define UTF8_REJECT 1

static const uint8_t utf8d[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
  8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
  0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
  0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
  0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
  1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
  1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
  1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
};

uint32_t font_utf8_decode_codepoint(uint32_t* state, uint32_t* codep, uint32_t byte)
{
  uint32_t type = utf8d[byte];

  *codep = (*state != UTF8_ACCEPT) ?
    (byte & 0x3fu) | (*codep << 6) :
    (0xff >> type) & (byte);

  *state = utf8d[256 + *state*16 + type];
  return *state;
}

int font_utf8_strlen(mp_obj_t str)
{
  int count = 0;
  const uint8_t *s = mp_obj_str_get_str(str);
  uint32_t codepoint;
  uint32_t state = 0;
  for (; *s; ++s)
    if (!font_utf8_decode_codepoint(&state, &codepoint, *s))
      count += 1;
  return count;
}

void imlib_draw_font(image_t *img, int x_off, int y_off, int c, uint8_t font_h, uint8_t font_w, const uint8_t *font)
{
    for (uint8_t y = 0, yy = font_h; y < yy; y++) {
        uint32_t tmp = font[y];
        for (uint8_t i = 1; i < font_w / 8; i++) {
            tmp <<= 8;
            tmp |= font[y + (i * font_h)];
        }
        for (uint8_t x = 0, xx = font_w; x < xx; x++) {
            if (tmp & (1 << (font_w - 1 - x))) {
                imlib_set_pixel(img, (x_off + x), (y_off + y), c);
            }
        }
    }
}

void imlib_draw_string(image_t *img, int x_off, int y_off, mp_obj_t str, int c) {
  const uint8_t font_byte_width = (font_config.width + 7)/8;
  const uint8_t font_len = font_byte_width * font_config.high;
  const uint8_t *s = mp_obj_str_get_str(str);
  uint32_t codepoint;
  uint32_t state = 0;
  uint16_t total_codepoints = (((uint8_t *)font_config.this)[0] << 8) | ((uint8_t *)font_config.this)[1];
  uint16_t cur_codepoint;
  int l;
  int r;
  int m;

  for (; *s; ++s) {
    if (!font_utf8_decode_codepoint(&state, &codepoint, *s)) {
        uint8_t buffer[font_len];
        
        switch (font_config.source)
        {
            case FileIn:
                file_seek_raise(font_config.this, codepoint * font_len, 0);
                read_data_raise(font_config.this, buffer, font_len);
                break;
            case ArrayIn:
                // printk("%d %p %p %p", font_len, buffer, font_config.this, &font_config.this[codepoint * font_len]);
                // memcpy(buffer, &font_config.this[codepoint * font_len], font_len);
                sys_spiffs_read(font_config.this + codepoint * font_len, font_len, buffer);
                break;
            case BuildIn:
                // printk("%d %p %p %d", font_len, buffer, font_config.this, &font_config.this[codepoint * font_len]);
                l = 0;
                r = total_codepoints - 1;
                while (l <= r) {
                    m = l + (r - l) / 2;
                    cur_codepoint = (((uint8_t *)font_config.this)[2 + m * (2 + font_len)] << 8) | ((uint8_t *)font_config.this)[2 + m * (2 + font_len) + 1];
                    if (cur_codepoint == codepoint) {
                        memcpy(buffer, &font_config.this[2 + m * (2 + font_len) + 2], font_len);
                        break;
                    }
                    if (cur_codepoint < codepoint)
                        l = m + 1;
                    else
                        r = m - 1;
                }
                break;
            default:
                break;
        }
        const uint8_t *font = buffer;
        imlib_draw_font(img, x_off, y_off, c, font_config.high, font_byte_width * 8, font);
        x_off += font_config.width;
    }
  }
}
