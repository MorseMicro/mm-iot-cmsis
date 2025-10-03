/*
 * Copyright 2024 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef DEMO_TEMPERATURE_H
#define DEMO_TEMPERATURE_H
#include <stdint.h>
typedef struct
{
    int32_t temperature_milli_degC;
    int32_t humidity_milli_RH;
} th_value_t;

void temperature_init(void);
void temperature_process(void);

th_value_t get_th_values();

#endif /* DEMO_TEMPERATURE_H */
