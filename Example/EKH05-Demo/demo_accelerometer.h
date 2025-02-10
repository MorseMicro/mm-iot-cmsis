/*
 * Copyright 2024 Morse Micro
 *
 * This file is licensed under terms that can be found in the LICENSE.md file in
 * the root directory of the Morse Micro IoT SDK software package.
 */

#ifndef DEMO_ACCELEROMETER_H
#define DEMO_ACCELEROMETER_H
#include <stdint.h>
typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} accelerometer_value_t;

void accelerometer_init(void);
void accelerometer_process(void);

accelerometer_value_t get_accelerometer_values();

#endif /* DEMO_ACCELEROMETER_H */
