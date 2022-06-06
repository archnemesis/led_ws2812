#include "rgbled.h"

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(rgbled);

#include <drivers/led_strip.h>
#include <device.h>
#include <drivers/spi.h>
#include <sys/util.h>

#define STRIP_NODE		DT_ALIAS(led_strip)
#define STRIP_NUM_PIXELS	DT_PROP(DT_ALIAS(led_strip), chain_length)

#define DELAY_TIME K_MSEC(5)

struct led_rgb pixels[STRIP_NUM_PIXELS];

static const struct device *strip = DEVICE_DT_GET(STRIP_NODE);

int rgbled_init(void)
{
	int i = 0;
	int err;

	if (device_is_ready(strip)) {
		LOG_INF("Found LED strip device %s", strip->name);
	} else {
		LOG_ERR("LED strip device %s is not ready", strip->name);
		return 1;
	}

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