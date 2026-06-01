/*
 * Copyright 2022-2024 Morse Micro
 * SPDX-License-Identifier: Apache-2.0
 *
 * Fixed build configuration for the hostap/wpa_supplicant component.
 * This file is pre-included for all source files in the hostap component via
 * the CMSIS-Pack preIncludeLocal mechanism.
 *
 * FOR DEVELOPERS of MorseMicro CMSIS-pack:
 * Source of truth for this file = framework/mk/wpa_supplicant.mk (unconditional BUILD_DEFINES).
 */

#ifndef HOSTAP_CONFIG_H
#define HOSTAP_CONFIG_H

#define IEEE8021X_EAPOL                     1
#define CONFIG_SME                          1
#define CONFIG_SAE                          1
#define CONFIG_OWE                          1
#define CONFIG_ECC                          1
#define CONFIG_SHA256                       1
#define CONFIG_SHA512                       1
#define CONFIG_SHA384                       1
#define CONFIG_FIPS                         1
#define CONFIG_NO_ACCOUNTING                1
#define CONFIG_NO_BSS_TRANS_MGMT            1
#define CONFIG_NO_CONFIG_BLOBS              1
#define CONFIG_NO_RADIUS                    1
#define CONFIG_NO_RANDOM_POOL               1
#define CONFIG_NO_RC4                       1
#define CONFIG_NO_ROBUST_AV                 1
#define CONFIG_NO_RRM                       1
#define CONFIG_NO_VLAN                      1
#define CONFIG_WNM                          1
#define CONFIG_IEEE80211AH                  1
#define CONFIG_DRIVER_NL80211_MORSE         1
#define CONFIG_NO_CONFIG_WRITE              1
#define OS_NO_C_LIB_DEFINES                 1
#define CONFIG_BGSCAN                       1
#define CONFIG_BGSCAN_SIMPLE                1
#define CONFIG_AUTOSCAN                     1
#define CONFIG_AUTOSCAN_EXPONENTIAL         1
#define CONFIG_S1G_TWT                      1
#define MAX_NUM_MLD_LINKS                   1
#define MAX_NUM_MLO_LINKS                   1
#define WPA_SUPPLICANT_CLEANUP_INTERVAL     120
#define MM_IOT
#define CONFIG_OPENSSL_INTERNAL_AES_WRAP    1
#define MM_IOT_DPP_HEADER                   1

/* Disable stdout debug output by default (matches SDK default behaviour). */
#define CONFIG_NO_WPA_MSG                   1
#define CONFIG_NO_STDOUT_DEBUG              1

#endif /* HOSTAP_CONFIG_H */
