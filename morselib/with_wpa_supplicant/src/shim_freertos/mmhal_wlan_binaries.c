/*
 * Copyright 2021-2024 Morse Micro
 *
 * This file is licensed under terms that can be found in the LICENSE.md file in the root
 * directory of the Morse Micro IoT SDK software package.
 */

#include <stdio.h>
#include "mmhal.h"
#include "mmconfig.h"
#include "main.h"


#ifdef BCF_DATA_3V3
#ifndef BCF_DATA_3V3_LEN
#error If BCF_DATA_3V3 is defined then BCF_DATA_3V3_LEN must also be defined.
#endif
#else
#define BCF_DATA_3V3     bcf_aw_hm593
#define BCF_DATA_3V3_LEN bcf_aw_hm593_len
#endif


#ifdef BCF_DATA_4V3
#ifndef BCF_DATA_4V3_LEN
#error If BCF_DATA_4V3 is defined then BCF_DATA_4V3_LEN must also be defined.
#endif
#else
#define BCF_DATA_4V3     bcf_aw_hm593_4v3
#define BCF_DATA_4V3_LEN bcf_aw_hm593_4v3_len
#endif

static const uint8_t *bcf_data = NULL;
static size_t bcf_length = 0;

/*
 * ---------------------------------------------------------------------------------------------
 *                                    BCF Selection
 * ---------------------------------------------------------------------------------------------
 */

void select_bcf_3v3(void)
{
    extern unsigned char BCF_DATA_3V3[];
    extern unsigned int BCF_DATA_3V3_LEN;
    bcf_data = BCF_DATA_3V3;
    bcf_length = BCF_DATA_3V3_LEN;
}

void select_bcf_4v3(void)
{
    extern unsigned char BCF_DATA_4V3[];
    extern unsigned int BCF_DATA_4V3_LEN;
    bcf_data = BCF_DATA_4V3;
    bcf_length = BCF_DATA_4V3_LEN;
}

/*
 * ---------------------------------------------------------------------------------------------
 *                                      BCF Retrieval
 * ---------------------------------------------------------------------------------------------
 */


/*
 * The following implementation reads the BCF File from the config store.
 */

void mmhal_wlan_read_bcf_file(uint32_t offset, uint32_t requested_len, struct mmhal_robuf *robuf)
{
    /* Initialise robuf */
    robuf->buf = NULL;
    robuf->len = 0;
    robuf->free_arg = NULL;
    robuf->free_cb = NULL;

    /* If the BCF isn't selected already, select the 3.3V BCF.*/
    if (!bcf_data)
    {
        select_bcf_3v3();
    }

    /* Sanity check */
    if (bcf_length < offset)
    {
        printf("Detected an attempt to start reading off the end of the bcf file.\n");
        return;
    }

    robuf->buf = bcf_data + offset;
    robuf->len = bcf_length - offset;
    robuf->len = (robuf->len < requested_len) ? robuf->len : requested_len;
}

/*
 * ---------------------------------------------------------------------------------------------
 *                                    Firmware Retrieval
 * ---------------------------------------------------------------------------------------------
 */

void mmhal_wlan_read_fw_file(uint32_t offset, uint32_t requested_len, struct mmhal_robuf *robuf)
{
    extern unsigned char fw_mm6108[];
    extern unsigned int fw_mm6108_len;
    uint32_t firmware_len = fw_mm6108_len;
    if (offset > firmware_len)
    {
        printf("Detected an attempt to start read off the end of the firmware file.\n");
        robuf->buf = NULL;
        return;
    }

    robuf->buf = (fw_mm6108 + offset);
    firmware_len -= offset;

    robuf->len = (firmware_len < requested_len) ? firmware_len : requested_len;
}
