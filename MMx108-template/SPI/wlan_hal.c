/*
 * Copyright 2024 Morse Micro
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

__attribute__((weak)) void mmhal_wlan_spi_cs_assert(void)
{
}

__attribute__((weak)) void mmhal_wlan_spi_cs_deassert(void)
{
}

__attribute__((weak)) uint8_t mmhal_wlan_spi_rw(uint8_t data)
{
    MM_UNUSED(data);
    return 0;
}


__attribute__((weak)) void mmhal_wlan_spi_read_buf(uint8_t *buf, unsigned len)
{
    MM_UNUSED(len);
    MM_UNUSED(buf);
}

__attribute__((weak)) void mmhal_wlan_spi_write_buf(const uint8_t *buf, unsigned len)
{
    MM_UNUSED(len);
    MM_UNUSED(buf);
}


__attribute__((weak)) void mmhal_wlan_send_training_seq(void)
{
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

__attribute__((weak)) void mmhal_wlan_set_busy_irq_enabled(bool enabled)
{
    MM_UNUSED(enabled);
}


/*
 * TODO: implement the BUSY Externel Interrupt handler. for example:
 *  void BUSY_EXTI_IRQ_HANDLER(void)
 *   {
 *       busy_irq_handler();
 *   }
*/

/*
 * TODO: implement the SPI Externel Interrupt handler. for example:
 *  void SPI_EXTI_IRQ_HANDLER(void)
 *   {
 *       spi_irq_handler();
 *   }
*/
