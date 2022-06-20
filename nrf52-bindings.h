/*
 * Copyright (c) 2019, Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_SAMPLES_DRIVERS_LED_WS2812_H_
#define ZEPHYR_SAMPLES_DRIVERS_LED_WS2812_H_

/*
 * At 7 MHz, 1 bit is ~142 ns, so 3 bits is ~426 ns.
 */
#define SPI_FREQ    4000000
#define ZERO_FRAME  0x40
#define ONE_FRAME   0x70

#define STRING_LEN  54

#endif
