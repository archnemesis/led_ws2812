#include <zephyr/bluetooth/uuid.h>

/* bd3664a8-9fc1-4f80-8ba4-1945e4149147 */
#define RGBLED_SERVICE_UUID 0x47, 0x91, 0x14, 0xE4, \
                            0x45, 0x19, 0xA4, 0x8B, \
                            0x80, 0x4F, 0xC1, 0x9F, \
                            0xA8, 0x64, 0x36, 0xBD

/* 7226b7ea-5636-4479-9fe6-5c845ba91b81 */
#define RGBLED_COMMAND_UUID 0x81, 0x1B, 0xA9, 0x5B, \
                            0x84, 0x5C, 0xE6, 0x9F, \
                            0x79, 0x44, 0x36, 0x56, \
                            0xEA, 0xB7, 0x26, 0x72

/* 3d00904c-6e4b-4a2e-88e9-22c9d5706763 */
#define RGBLED_DATA_UUID    0x63, 0x67, 0x70, 0xD5, \
                            0xC9, 0x22, 0xE9, 0x88, \
                            0x2E, 0x4A, 0x4B, 0x6E, \
                            0x4C, 0x90, 0x00, 0x3D

#define BT_UUID_RGBLED      BT_UUID_DECLARE_128(RGBLED_SERVICE_UUID)
#define BT_UUID_RGBLED_CMD  BT_UUID_DECLARE_128(RGBLED_COMMAND_UUID)
#define BT_UUID_RGBLED_DATA BT_UUID_DECLARE_128(RGBLED_DATA_UUID)
