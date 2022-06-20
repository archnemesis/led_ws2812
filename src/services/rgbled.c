#include "rgbled.h"

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(rgbled);

#include <zephyr/zephyr.h>
#include <drivers/led_strip.h>
#include <device.h>
#include <drivers/spi.h>
#include <sys/util.h>

#define RGBLED_STACK_SIZE   1024
#define RGBLED_PRIORITY     5

#define RGBLED_EVENT_SET_PATTERN    0x01
#define RGBLED_EVENT_SET_COLOR_1

static void rgbled_thread_main(void *, void *, void *);
static void pattern_pulse1(void);
static void pattern_pulse2(void);
static void pattern_solid(void);
static void pattern_rainbow(void);

K_THREAD_DEFINE(rgbled_tid,
                RGBLED_STACK_SIZE,
                rgbled_thread_main,
                NULL, NULL, NULL,
                RGBLED_PRIORITY,
                0, 0);

K_EVENT_DEFINE(rgbled_event);

/**
 * RGB LED Driver
 */

#define NUM_COLOR_SLOTS 4

#define STRIP_NODE		DT_ALIAS(led_strip)
#define STRIP_NUM_PIXELS	DT_PROP(DT_ALIAS(led_strip), chain_length)

#define DELAY_TIME K_MSEC(50)

/**
 * @brief This array holds the color value for each pixel in the series.
 */
static struct led_rgb pixels[STRIP_NUM_PIXELS];

/**
 * @brief Colors for patterns are stored in this array of color slots.
 */
static struct led_rgb colors[NUM_COLOR_SLOTS];

/**
 * @brief The current pattern ID.
 */
static int active_pattern = 0;

/**
 * @brief The LED strip device handle.
 */
static const struct device *strip = DEVICE_DT_GET(STRIP_NODE);

static const uint8_t sine_table[256] = {
    128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170,
    173, 176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211,
    213, 215, 218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240,
    241, 243, 244, 245, 246, 248, 249, 250, 250, 251, 252, 253, 253, 254, 254,
    254, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251,
    250, 250, 249, 248, 246, 245, 244, 243, 241, 240, 238, 237, 235, 234, 232,
    230, 228, 226, 224, 222, 220, 218, 215, 213, 211, 208, 206, 203, 201, 198,
    196, 193, 190, 188, 185, 182, 179, 176, 173, 170, 167, 165, 162, 158, 155,
    152, 149, 146, 143, 140, 137, 134, 131, 128, 124, 121, 118, 115, 112, 109,
    106, 103, 100, 97,  93,  90,  88,  85,  82,  79,  76,  73,  70,  67,  65,
    62,  59,  57,  54,  52,  49,  47,  44,  42,  40,  37,  35,  33,  31,  29,
    27,  25,  23,  21,  20,  18,  17,  15,  14,  12,  11,  10,  9,   7,   6,
    5,   5,   4,   3,   2,   2,   1,   1,   1,   0,   0,   0,   0,   0,   0,
    0,   1,   1,   1,   2,   2,   3,   4,   5,   5,   6,   7,   9,   10,  11,
    12,  14,  15,  17,  18,  20,  21,  23,  25,  27,  29,  31,  33,  35,  37,
    40,  42,  44,  47,  49,  52,  54,  57,  59,  62,  65,  67,  70,  73,  76,
    79,  82,  85,  88,  90,  93,  97,  100, 103, 106, 109, 112, 115, 118, 121,
    124};

static const uint8_t gamma_table[256] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,
    1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   2,   2,   2,   3,
    3,   3,   3,   3,   3,   4,   4,   4,   4,   5,   5,   5,   5,   5,   6,
    6,   6,   6,   7,   7,   7,   8,   8,   8,   9,   9,   9,   10,  10,  10,
    11,  11,  11,  12,  12,  13,  13,  13,  14,  14,  15,  15,  16,  16,  17,
    17,  18,  18,  19,  19,  20,  20,  21,  21,  22,  22,  23,  24,  24,  25,
    25,  26,  27,  27,  28,  29,  29,  30,  31,  31,  32,  33,  34,  34,  35,
    36,  37,  38,  38,  39,  40,  41,  42,  42,  43,  44,  45,  46,  47,  48,
    49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
    64,  65,  66,  68,  69,  70,  71,  72,  73,  75,  76,  77,  78,  80,  81,
    82,  84,  85,  86,  88,  89,  90,  92,  93,  94,  96,  97,  99,  100, 102,
    103, 105, 106, 108, 109, 111, 112, 114, 115, 117, 119, 120, 122, 124, 125,
    127, 129, 130, 132, 134, 136, 137, 139, 141, 143, 145, 146, 148, 150, 152,
    154, 156, 158, 160, 162, 164, 166, 168, 170, 172, 174, 176, 178, 180, 182,
    184, 186, 188, 191, 193, 195, 197, 199, 202, 204, 206, 209, 211, 213, 215,
    218, 220, 223, 225, 227, 230, 232, 235, 237, 240, 242, 245, 247, 250, 252,
    255};

int rgbled_init(void)
{
	if (device_is_ready(strip)) {
		LOG_INF("Found LED strip device %s", strip->name);
	} else {
		LOG_ERR("LED strip device %s is not ready", strip->name);
		return 1;
	}

    active_pattern = 4;

    memset((void *)&pixels[0], 0x00, STRIP_NUM_PIXELS * sizeof(struct led_rgb));
    memset((void *)&colors[0], 0x00, NUM_COLOR_SLOTS * sizeof(struct led_rgb));

    colors[0] = (struct led_rgb)RGB(0x00, 0xFF, 0x00);
    colors[1] = (struct led_rgb)RGB(0xFF, 0x00, 0x00);

    return 0;
}

int rgbled_num_pixels()
{
    return STRIP_NUM_PIXELS;
}

int rgbled_update(void)
{
    int err = 0;

    err = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
    if (err) {
        LOG_ERR("couldn't update strip: %d", err);
    }

    return err;
}

void rgbled_set_pixels(struct led_rgb color)
{
    for (int i = 0; i < STRIP_NUM_PIXELS; i++) {
        rgbled_set_pixel(i, color);
    }
}

void rgbled_set_pixel(int pixel, struct led_rgb color)
{
    pixels[pixel] = color;
}

void rgbled_set_pattern(int pattern)
{
    active_pattern = pattern;
}

/**
 * @brief Set the value of a color slot.
 * 
 * @param index Index of the color slot to set
 * @param color Color to set in the slot
 */
void rgbled_set_color(int index, struct led_rgb color)
{
    colors[index] = color;
}

void rgbled_thread_main(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    uint32_t events;
    
    while (1) {
        events = k_event_wait(&rgbled_event, 0xFFF, true, K_NO_WAIT);
        if (events != 0) {
            // TODO: process event (change pattern, color, etc)
        }

        switch (active_pattern) {
            case 1:
                pattern_rainbow();
                break;
            case 2:
                pattern_solid();
                break;
            case 3:
                pattern_pulse1();
                break;
            case 4:
                pattern_pulse2();
                break;
        }

        k_sleep(DELAY_TIME);
    }
}

void pattern_pulse1(void)
{
    static int i = 0;
    static bool upcount = true;

    for (int j = 0; j < rgbled_num_pixels(); j++) {
        uint8_t r = i == 0 ? 0 : ((colors[0].r * i) >> 8) + 1;
        uint8_t g = i == 0 ? 0 : ((colors[0].g * i) >> 8) + 1;
        uint8_t b = i == 0 ? 0 : ((colors[0].b * i) >> 8) + 1;
        rgbled_set_pixel(j, (struct led_rgb)RGB(r, g, b));
    }

    if (upcount) {
        i++;
        if (i == 255) {
            upcount = false;
        }
    }
    else {
        i--;
        if (i == 0) {
            upcount = true;
        }
    }

    rgbled_update();
}

void pattern_pulse2(void)
{
    static int i = 0;

    for (int j = 0; j < rgbled_num_pixels(); j++) {
        uint8_t alpha = sine_table[i];
        uint8_t r = alpha == 0 ? 0 : ((colors[0].r * alpha) >> 8) + 1;
        uint8_t g = alpha == 0 ? 0 : ((colors[0].g * alpha) >> 8) + 1;
        uint8_t b = alpha == 0 ? 0 : ((colors[0].b * alpha) >> 8) + 1;
        rgbled_set_pixel(j, (struct led_rgb)RGB(r, g, b));
    }

    if (i++ == 256) {
        i = 0;
    }

    rgbled_update();
}

static void pattern_solid(void)
{
    for (int j = 0; j < STRIP_NUM_PIXELS; j++) {
        rgbled_set_pixel(j, colors[0]);
    }
    rgbled_update();
}

static uint32_t color_hsv(uint16_t hue, uint8_t sat, uint8_t val)
{
    uint8_t r, g, b;

    // Remap 0-65535 to 0-1529. Pure red is CENTERED on the 64K rollover;
    // 0 is not the start of pure red, but the midpoint...a few values above
    // zero and a few below 65536 all yield pure red (similarly, 32768 is the
    // midpoint, not start, of pure cyan). The 8-bit RGB hexcone (256 values
    // each for red, green, blue) really only allows for 1530 distinct hues
    // (not 1536, more on that below), but the full unsigned 16-bit type was
    // chosen for hue so that one's code can easily handle a contiguous color
    // wheel by allowing hue to roll over in either direction.
    hue = (hue * 1530L + 32768) / 65536;
    // Because red is centered on the rollover point (the +32768 above,
    // essentially a fixed-point +0.5), the above actually yields 0 to 1530,
    // where 0 and 1530 would yield the same thing. Rather than apply a
    // costly modulo operator, 1530 is handled as a special case below.

    // So you'd think that the color "hexcone" (the thing that ramps from
    // pure red, to pure yellow, to pure green and so forth back to red,
    // yielding six slices), and with each color component having 256
    // possible values (0-255), might have 1536 possible items (6*256),
    // but in reality there's 1530. This is because the last element in
    // each 256-element slice is equal to the first element of the next
    // slice, and keeping those in there this would create small
    // discontinuities in the color wheel. So the last element of each
    // slice is dropped...we regard only elements 0-254, with item 255
    // being picked up as element 0 of the next slice. Like this:
    // Red to not-quite-pure-yellow is:        255,   0, 0 to 255, 254,   0
    // Pure yellow to not-quite-pure-green is: 255, 255, 0 to   1, 255,   0
    // Pure green to not-quite-pure-cyan is:     0, 255, 0 to   0, 255, 254
    // and so forth. Hence, 1530 distinct hues (0 to 1529), and hence why
    // the constants below are not the multiples of 256 you might expect.

    // Convert hue to R,G,B (nested ifs faster than divide+mod+switch):
    if (hue < 510) { // Red to Green-1
        b = 0;
        if (hue < 255) { //   Red to Yellow-1
            r = 255;
            g = hue;       //     g = 0 to 254
        } else {         //   Yellow to Green-1
            r = 510 - hue; //     r = 255 to 1
            g = 255;
        }
    } else if (hue < 1020) { // Green to Blue-1
        r = 0;
        if (hue < 765) { //   Green to Cyan-1
            g = 255;
            b = hue - 510;  //     b = 0 to 254
        } else {          //   Cyan to Blue-1
            g = 1020 - hue; //     g = 255 to 1
            b = 255;
        }
    } else if (hue < 1530) { // Blue to Red-1
        g = 0;
        if (hue < 1275) { //   Blue to Magenta-1
            r = hue - 1020; //     r = 0 to 254
            b = 255;
        } else { //   Magenta to Red-1
            r = 255;
            b = 1530 - hue; //     b = 255 to 1
        }
    } else { // Last 0.5 Red (quicker than % operator)
        r = 255;
        g = b = 0;
    }

    // Apply saturation and value to R,G,B, pack into 32-bit result:
    uint32_t v1 = 1 + val;  // 1 to 256; allows >>8 instead of /255
    uint16_t s1 = 1 + sat;  // 1 to 256; same reason
    uint8_t s2 = 255 - sat; // 255 to 0
    return ((((((r * s1) >> 8) + s2) * v1) & 0xff00) << 8) |
            (((((g * s1) >> 8) + s2) * v1) & 0xff00) |
            (((((b * s1) >> 8) + s2) * v1) >> 8);
}

static uint8_t gamma8(uint8_t x) {
    return gamma_table[x];
}

static uint32_t gamma32(uint32_t x) {
    uint8_t *y = (uint8_t *)&x;
    // All four bytes of a 32-bit value are filtered even if RGB (not WRGB),
    // to avoid a bunch of shifting and masking that would be necessary for
    // properly handling different endianisms (and each byte is a fairly
    // trivial operation, so it might not even be wasting cycles vs a check
    // and branch for the RGB case). In theory this might cause trouble *if*
    // someone's storing information in the unused most significant byte
    // of an RGB value, but this seems exceedingly rare and if it's
    // encountered in reality they can mask values going in or coming out.
    for (uint8_t i = 0; i < 4; i++)
        y[i] = gamma8(y[i]);
    return x; // Packed 32-bit return
}

static void pattern_rainbow(void)
{
    static int firstPixelHue = 0;
    static int a = 0;
    static int b = 0;

    rgbled_set_pixels((struct led_rgb)RGB(0, 0, 0));

    if (a < 30) {
        if (b < 3) {
            for (int c = b; c < STRIP_NUM_PIXELS; c++) {
                int hue = firstPixelHue + c * 65536L / STRIP_NUM_PIXELS;
                uint32_t rgb = gamma32(color_hsv(hue, 255, 255));
                pixels[c].r = (0x00FF0000 & rgb) >> 16;
                pixels[c].g = (0x0000FF00 & rgb) >> 8;
                pixels[c].b = (0x000000FF & rgb);
            }
            b++;
            firstPixelHue += 65536 / 90;
            rgbled_update();
        }
        else {
            a++;
            b = 0;
        }
    }
    else {
        a = 0;
    }
}