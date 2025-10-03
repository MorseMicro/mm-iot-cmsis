/*
 * Copyright 2021-2024 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mmhal_wlan.h"
#include "mmutils.h"


/** SPI hw interrupt handler. Must be set before enabling irq */
static mmhal_irq_handler_t spi_irq_handler = NULL;

/** busy interrupt handler. Must be set before enabling irq */
static mmhal_irq_handler_t busy_irq_handler = NULL;

__attribute__((weak)) void mmhal_wlan_hard_reset(void)
{
}

#if defined(ENABLE_EXT_XTAL_INIT) && ENABLE_EXT_XTAL_INIT
__attribute__((weak)) bool mmhal_wlan_ext_xtal_init_is_required(void)
{
    return true;
}
#endif

__attribute__((weak)) int mmhal_wlan_sdio_cmd(uint8_t cmd_idx, uint32_t arg, uint32_t *rsp)
{
    MM_UNUSED(rsp);
    MM_UNUSED(arg);
    MM_UNUSED(cmd_idx);
    return 0;
}

__attribute__((weak)) int mmhal_wlan_sdio_startup(void)
{
    return 0;
}

__attribute__((weak)) void mmhal_wlan_register_spi_irq_handler(mmhal_irq_handler_t handler)
{
    spi_irq_handler = handler;
}

__attribute__((weak)) bool mmhal_wlan_spi_irq_is_asserted(void)
{
    return false;
}

__attribute__((weak)) void mmhal_wlan_set_spi_irq_enabled(bool enabled)
{
    MM_UNUSED(enabled);
}


__attribute__((weak)) void mmhal_wlan_init(void)
{
}

__attribute__((weak)) void mmhal_wlan_deinit(void)
{
}

__attribute__((weak)) void mmhal_wlan_wake_assert(void)
{
}

__attribute__((weak)) void mmhal_wlan_wake_deassert(void)
{
}

__attribute__((weak)) bool mmhal_wlan_busy_is_asserted(void)
{
    return false;
}

__attribute__((weak)) void mmhal_wlan_register_busy_irq_handler(mmhal_irq_handler_t handler)
{
    busy_irq_handler = handler;
}

__attribute__((weak)) void mmhal_wlan_clear_spi_irq(void)
{
}

__attribute__((weak)) void mmhal_wlan_set_busy_irq_enabled(bool enabled)
{
    MM_UNUSED(enabled);
}



__attribute__((weak)) int mmhal_wlan_sdio_cmd53_write(const struct mmhal_wlan_sdio_cmd53_write_args *args)
{
    MM_UNUSED(args);
    return 0;
}

__attribute__((weak)) int mmhal_wlan_sdio_cmd53_read(const struct mmhal_wlan_sdio_cmd53_read_args *args)
{
    MM_UNUSED(args);
    return 0;
}

/* 
 * TODO: implement the BUSY Externel Interrupt handler. for example:
 *  v__attribute__((weak)) oid BUSY_EXTI_IRQ_HANDLER(void)
 *   {
 *       busy_irq_handler();
 *   }
*/

/* 
 * TODO: implement SDIO Interrupt handler. for example:
 *  v__attribute__((weak)) oid WLAN_SDMMC_IRQHandler(void)
 *   {
 *       spi_irq_handler();
 *   }
*/

