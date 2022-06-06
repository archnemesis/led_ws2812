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
#define SPI_FREQ    7000000
#define ZERO_FRAME  0x60
#define ONE_FRAME   0x7E

#endif
