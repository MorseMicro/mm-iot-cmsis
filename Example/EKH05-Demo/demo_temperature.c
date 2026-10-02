/*
 * Copyright 2024 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "demo_temperature.h"
#include "main.h"
#include "mmosal.h"
#include "sensirion_common.h"
#include "sht4x_i2c.h"

static struct mmosal_mutex *values_mutex = NULL;
static int32_t temperature_milli_degC = 0;
static int32_t humidity_milli_RH = 0;
static uint8_t sensor_initialized = 0;

void temperature_process(void)
{
    int16_t error = NO_ERROR;
    if (sensor_initialized == 0)
    {
        return;
    }
    int32_t temperature;
    int32_t humidity;

    error = sht4x_measure_lowest_precision(&temperature, &humidity);
    if (error != NO_ERROR)
    {
        printf("error executing sht4x_measure_lowest_precision(): %i\n\r", error);
        return;
    }

    MMOSAL_MUTEX_GET_INF(values_mutex);
    temperature_milli_degC = temperature;
    humidity_milli_RH = humidity;
    MMOSAL_MUTEX_RELEASE(values_mutex);
}

void temperature_init()
{
    int16_t error = NO_ERROR;
    uint32_t serial_number = 0;

    MMOSAL_ASSERT(values_mutex == NULL);
    values_mutex = mmosal_mutex_create("temperature_values");
    MMOSAL_ASSERT(values_mutex != NULL);

    sht4x_init(SHT40_I2C_ADDR_44 << 1);
    sht4x_soft_reset();
    mmosal_task_sleep(10);
    error = sht4x_serial_number(&serial_number);
    if (error != NO_ERROR)
    {
        printf("error executing sht4x_serial_number(): %i\n", error);
        return;
    }
    printf("SHT4x serial_number: %u\n\r", (unsigned int)serial_number);
    sensor_initialized = 1;
}

th_value_t get_th_values()
{
    th_value_t values;

    MMOSAL_MUTEX_GET_INF(values_mutex);
    values = (th_value_t){temperature_milli_degC, humidity_milli_RH};
    MMOSAL_MUTEX_RELEASE(values_mutex);

    return values;
}
