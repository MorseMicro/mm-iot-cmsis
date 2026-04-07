/*
 * Copyright 2025 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __DEMO_MDNS__H__
#define __DEMO_MDNS__H__

#include "lwip/netif.h"

void mdns_link_update(struct netif *mmnetif);

#endif
