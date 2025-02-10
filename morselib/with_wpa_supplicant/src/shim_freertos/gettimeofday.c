/*
 * Copyright 2021-2024 Morse Micro
 *
 * This file is licensed under terms that can be found in the LICENSE.md file in the root
 * directory of the Morse Micro IoT SDK software package.
 */

#include <sys/types.h>
#include <mmosal.h>

int _gettimeofday(struct timeval *tv, void *ptz)
{
    int time_ms = mmosal_get_time_ms();
    if (tv != NULL) {
        tv->tv_sec = (time_ms / 1000);
        tv->tv_usec = (time_ms % 1000) * 1000;
        return 0;
    }

    return -1;
}