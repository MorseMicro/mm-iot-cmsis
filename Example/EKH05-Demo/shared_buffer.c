/*
 * Copyright 2024 Morse Micro
 *
 * This file is licensed under terms that can be found in the LICENSE.md file in
 * the root directory of the Morse Micro IoT SDK software package.
 */

#include "shared_buffer.h"

bool shared_buffer_lock(SharedBuffer *buf)
{
    return mmosal_semb_wait(buf->buffer_lock_released, BUFFER_LOCK_DELAY);
}

void shared_buffer_unlock(SharedBuffer *buf)
{
    mmosal_semb_give(buf->buffer_lock_released);
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

    /* Semaphore flag to indicate the access (read/write) process is allowed
     * When the flag is given (1) = allow to access to the buffer
     * When the flag is taken (0) = read/write in progress
     */
    buf->buffer_lock_released = mmosal_semb_create("string_buffer_accessible");
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
    int new_data_len = strlen(new_data);
    if (new_data_len == 0)
    {
        return true;
    }

    if (new_data_len > BUFFER_SIZE - buf->currentIndex)
    {
        /* If the buffer does not able to fit, skip pushing in */
        return false;
    }

    if (shared_buffer_lock(buf))
    {
        /* The buffer must have enough space to fit the new string */
        strcat(buf->buffer, new_data);
        buf->currentIndex += new_data_len;

        /* ensure the buffer contains '\0' */
        buf->buffer[buf->currentIndex] = '\0';

        shared_buffer_unlock(buf);
    }

    return true;
}

const char *shared_buffer_get(SharedBuffer *buf) { return buf->buffer; }
