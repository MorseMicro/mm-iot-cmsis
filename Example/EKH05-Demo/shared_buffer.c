/*
 * Copyright 2024 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "shared_buffer.h"
#include <stdarg.h>

#define TEMP_BUFFER_SIZE 256
static char local_output_str[TEMP_BUFFER_SIZE];

bool shared_buffer_lock(SharedBuffer *buf)
{
    return mmosal_mutex_get(buf->mutex, UINT32_MAX);
}

void shared_buffer_unlock(SharedBuffer *buf)
{
    mmosal_mutex_release(buf->mutex);
}

void shared_buffer_reset(SharedBuffer *buf)
{
    if (shared_buffer_lock(buf))
    {
        buf->buffer[0] = '\0';
        buf->currentIndex = 0;
        shared_buffer_unlock(buf);
    }
}

/* Function to initialize the buffer with empty strings and create the semaphore */
void shared_buffer_init(SharedBuffer *buf)
{
    buf->currentIndex = 0;

    buf->mutex = mmosal_mutex_create("shared_buffer");
    shared_buffer_unlock(buf);

    /* Zero out the buffer initially */
    memset(buf->buffer, 0, BUFFER_SIZE);
}

/* Function to push new data to the top of the buffer
 * Return (true) = push success/no data appended
 * Return (false) = push fail/new data can not fit into the buffer
 */
bool shared_buffer_append(SharedBuffer *buf, const char *new_data)
{
    size_t new_data_len = strlen(new_data);

    if (new_data_len == 0)
    {
        return true;
    }

    if (!shared_buffer_lock(buf))
    {
        return false;
    }

    if (buf->currentIndex + new_data_len + 1 > BUFFER_SIZE)
    {
        shared_buffer_unlock(buf);
        return false;
    }

    memcpy(buf->buffer + buf->currentIndex, new_data, new_data_len);
    buf->currentIndex += new_data_len;
    buf->buffer[buf->currentIndex] = '\0';
    shared_buffer_unlock(buf);

    return true;
}

const char *shared_buffer_get(SharedBuffer *buf) { return buf->buffer; }

/* dual_print prints into uart and http console. return false if the http console buffer can't take
 * more data.*/
bool dual_print(SharedBuffer *sb, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(local_output_str, TEMP_BUFFER_SIZE, format, args);
    va_end(args);
    /* Print on uart console and to http console.*/
    printf("%s", local_output_str);
    return shared_buffer_append(sb, local_output_str);
}
