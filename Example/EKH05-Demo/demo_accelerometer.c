/*
 * Copyright 2024 Morse Micro
 *
 * This file is licensed under terms that can be found in the LICENSE.md file in
 * the root directory of the Morse Micro IoT SDK software package.
 */

#include "demo_accelerometer.h"
#include "iis328dq_reg.h"
#include "main.h"
#include "mmosal.h"

extern TIM_HandleTypeDef htim4;
extern I2C_HandleTypeDef hi2c1;

/* Private variables ---------------------------------------------------------*/
static stmdev_ctx_t dev_ctx;
static int16_t data_raw_acceleration[3] = {0, 0, 0};
static uint8_t sensor_initialized = 0;
/* Setting iis328dq_i2c_addr to IIS328DQ_I2C_ADD_L, only to have a valid value.
 * This isn't the default address. The detect function will overwrite it anyways.
 */
static uint16_t iis328dq_i2c_addr = IIS328DQ_I2C_ADD_L;

static int32_t IIS328DQ_platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
    reg |= 0x80;
    HAL_I2C_Mem_Read((I2C_HandleTypeDef *)handle, iis328dq_i2c_addr, reg, I2C_MEMADD_SIZE_8BIT,
                     bufp, len, 1000);
    return 0;
}

static int32_t IIS328DQ_platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len)
{
    reg |= 0x80;
    HAL_I2C_Mem_Write((I2C_HandleTypeDef *)handle, iis328dq_i2c_addr, reg, I2C_MEMADD_SIZE_8BIT,
                      (uint8_t *)bufp, len, 1000);

    return 0;
}

static bool IIS328DQ_detect()
{
    uint8_t whoamI;
    whoamI = 0;
    iis328dq_i2c_addr = IIS328DQ_I2C_ADD_L;
    iis328dq_device_id_get(&dev_ctx, &whoamI);

    if (whoamI == IIS328DQ_ID)
    {
        printf("accelerometer ID(0x%x) matches at address: 0x%x \n\r", whoamI, iis328dq_i2c_addr);
        return true;
    }

    whoamI = 0;
    iis328dq_i2c_addr = IIS328DQ_I2C_ADD_H;
    iis328dq_device_id_get(&dev_ctx, &whoamI);

    if (whoamI == IIS328DQ_ID)
    {
        printf("accelerometer ID(0x%x) matches at address: 0x%x \n\r", whoamI, iis328dq_i2c_addr);
        return true;
    }
    return false;
}

/*
 * transform_func transforms the raw sensor value to a pwm value for the leds
 * that looks good when you tilt the board.
 */
int16_t transform_func(int16_t val)
{
    val -= 100;
    if (val < 0)
        val = 0;
    if (val > 1000)
        val = 1000;
    float x = val;
    float y = 1000 * (x / 1000) * (x / 1000) * (x / 1000);
    return (int16_t)y;
}

void accelerometer_process(void)
{
    /* Read output only if new value is available */
    iis328dq_reg_t reg;
    int left, right, down;
    if (sensor_initialized == 0)
    {
        return;
    }
    iis328dq_status_reg_get(&dev_ctx, &reg.status_reg);

    if (reg.status_reg.zyxda)
    {
        /* Read acceleration data */
        iis328dq_acceleration_raw_get(&dev_ctx, data_raw_acceleration);
        if (data_raw_acceleration[1] > 0)
        {
            left = data_raw_acceleration[1] / 16;
            right = 0;
        }
        else
        {
            right = 0 - data_raw_acceleration[1] / 16;
            left = 0;
        }

        if (data_raw_acceleration[0] > 0)
        {
            down = 0;
        }
        else
        {
            down = 0 - data_raw_acceleration[0] / 16;
        }
        htim4.Instance->CCR1 = transform_func(down);
        htim4.Instance->CCR2 = transform_func(left);
        htim4.Instance->CCR3 = transform_func(right);
    }
}

void accelerometer_init(void)
{
    dev_ctx.write_reg = IIS328DQ_platform_write;
    dev_ctx.read_reg = IIS328DQ_platform_read;
    dev_ctx.mdelay = mmosal_task_sleep;
    dev_ctx.handle = &hi2c1;
    /* We use the TIM4 pwm to illuminate RGB LED based on the board's orientation.*/
    HAL_TIM_Base_Start(&htim4);
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
    htim4.Instance->CCR1 = 0;
    htim4.Instance->CCR2 = 0;
    htim4.Instance->CCR3 = 0;

    if (!IIS328DQ_detect())
    {
        printf("Unable to detect IIS328DQ accelerometer.\n\r");
        return;
    }
    /* Enable Block Data Update */
    iis328dq_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
    /* Set full scale */
    iis328dq_full_scale_set(&dev_ctx, IIS328DQ_2g);
    /* Configure filtering chain */
    /* Accelerometer - High Pass / Slope path */
    iis328dq_hp_path_set(&dev_ctx, IIS328DQ_HP_DISABLE);
    /* Set Output Data Rate */
    iis328dq_data_rate_set(&dev_ctx, IIS328DQ_ODR_100Hz);
    sensor_initialized = 1;
}

accelerometer_value_t get_accelerometer_values()
{
    return *(accelerometer_value_t *)&data_raw_acceleration;
}
