/*
 * Copyright 2024 Morse Micro
 *
 * This file is licensed under terms that can be found in the LICENSE.md file in
 * the root directory of the Morse Micro IoT SDK software package.
 */

#ifndef __DEMO_PING__H__
#define __DEMO_PING__H__

#include <string.h>
#include <stdlib.h>

#include "mmutils.h"
#include "mmosal.h"
#include "mmconfig.h"
#include "mmping.h"
#include "mmipal.h"
#include "shared_buffer.h"


extern SharedBuffer http_terminal_buffer;

void ping_start(void);
void ping_stop(void);
void ping_init(void);
bool ping_get_in_progress(void);
const char *ping_get_target(void);
uint32_t ping_get_count(void);
void ping_set_IP(const char *ip);
void ping_set_count(uint32_t count);

#endif
