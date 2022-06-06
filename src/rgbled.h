
#include <drivers/led_strip.h>

#define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }

int rgbled_init(void);
int rgbled_update(void);
int rgbled_num_pixels();
void rgbled_set_pixels(struct led_rgb color);
void rgbled_set_pixel(int pixel, struct led_rgb color);