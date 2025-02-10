/*
 * Copyright 2024 Morse Micro
 *
 * This file is licensed under terms that can be found in the LICENSE.md file in
 * the root directory of the Morse Micro IoT SDK software package.
 */

#ifndef __SHARED_BUFFER__H__
#define __SHARED_BUFFER__H__

#include "mmosal.h"

#include <stdio.h>
#include <string.h>


#define BUFFER_SIZE 512
#define BUFFER_LOCK_DELAY 500

/* SharedBuffer contains a shared buffer that can be safely used to share data between tasks.*/
typedef struct
{
    char buffer[BUFFER_SIZE];
    int currentIndex;
    struct mmosal_semb *buffer_lock_released;
} SharedBuffer;

bool shared_buffer_lock(SharedBuffer *buf);
void shared_buffer_unlock(SharedBuffer *buf);
void shared_buffer_reset(SharedBuffer *buf);
void shared_buffer_init(SharedBuffer *buf);
bool shared_buffer_append(SharedBuffer *buf, const char *new_data);
const char *shared_buffer_get(SharedBuffer *buf);

#endif
