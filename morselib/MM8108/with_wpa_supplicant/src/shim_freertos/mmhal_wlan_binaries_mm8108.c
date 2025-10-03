/*
 * Copyright 2025 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include "mmhal.h"
#include "main.h"


#ifdef BCF_DATA
#ifndef BCF_DATA_LEN
#error If BCF_DATA is defined then BCF_DATA_LEN must also be defined.
#endif
#else
#define BCF_DATA     bcf_mf15457
#define BCF_DATA_LEN bcf_mf15457_len
#endif

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

    extern const unsigned char BCF_DATA[];
    extern const unsigned int BCF_DATA_LEN;
    robuf->buf = NULL;
    robuf->len = 0;
    robuf->free_arg = NULL;
    robuf->free_cb = NULL;


    /* Sanity check */
    if (BCF_DATA_LEN < offset)
    {
        printf("Detected an attempt to start reading off the end of the bcf file.\n");
        return;
    }

    robuf->buf = BCF_DATA + offset;
    robuf->len = BCF_DATA_LEN - offset;
    robuf->len = (robuf->len < requested_len) ? robuf->len : requested_len;
}

/*
 * ---------------------------------------------------------------------------------------------
 *                                    Firmware Retrieval
 * ---------------------------------------------------------------------------------------------
 */

void mmhal_wlan_read_fw_file(uint32_t offset, uint32_t requested_len, struct mmhal_robuf *robuf)
{
    extern const unsigned char fw_mm8108b2_rl[];
    extern const unsigned int fw_mm8108b2_rl_len;
    uint32_t firmware_len = fw_mm8108b2_rl_len;
    if (offset > firmware_len)
    {
        printf("Detected an attempt to start read off the end of the firmware file.\n");
        robuf->buf = NULL;
        return;
    }

    robuf->buf = (fw_mm8108b2_rl + offset);
    firmware_len -= offset;

    robuf->len = (firmware_len < requested_len) ? firmware_len : requested_len;
}
