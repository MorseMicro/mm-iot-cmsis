/*
 * Copyright 2021-2023 Morse Micro
 *
 * This file is licensed under terms that can be found in the LICENSE.md file in the root
 * directory of the Morse Micro IoT SDK software package.
 */

/**
 * This File implements the MMCONFIG Persistent Store API.
 */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "mmconfig.h"


extern config_entry_t mm_configs[];

/**
 * This function converts a string representation of a decimal or hexadecimal
 * number to an unsigned integer.
 *
 * @param str       The string representation of a decimal or hexadecimal number
 * @param val       The unsigned integer to return the value in, unchanged on error
 * @return          MMCONFIG_OK on success or an error code on failure
 */
static int mmconfig_str_to_uint(char *str, uint32_t *val)
{
    uint64_t num = 0;
    int ii;

    /* Is it a hexadecimal string? */
    if ((str[0] == '0') && ((str[1] == 'x') || (str[1] == 'X')))
    {
        for (ii = 2; str[ii] != '\0'; ii++) {
            if ((str[ii] >= '0') && (str[ii] <= '9'))
            {
                num = num * 16 + (str[ii] - '0');
            }
            else if ((str[ii] >= 'a') && (str[ii] <= 'f'))
            {
                num = num * 16 + (str[ii] - 'a' + 10);
            }
            else if ((str[ii] >= 'A') && (str[ii] <= 'F'))
            {
                num = num * 16 + (str[ii] - 'A' + 10);
            }
            else
            {
                /* Invalid code, return error */
                return MMCONFIG_ERR_INCORRECT_TYPE;
            }
        }
    }
    else
    {
        for (ii = 0; str[ii] != '\0'; ii++) {
            if ((str[ii] < '0') || (str[ii] > '9'))
            {
                /* Invalid code, return error */
                return MMCONFIG_ERR_INCORRECT_TYPE;
            }

            num = num * 10 + (str[ii] - '0');
        }
    }

    if ((ii > 10) || (num > 4294967295))
    {
        /* Overflow */
        return MMCONFIG_ERR_INCORRECT_TYPE;
    }

    *val = (uint32_t)num;

    return MMCONFIG_OK;
}

/**
 * This function converts a string representation of a signed decimal or hexadecimal
 * number to an signed integer.
 *
 * @param str       The string representation of a signed decimal or hexadecimal number
 * @param val       The signed integer to return the value in, unchanged on error
 * @return          MMCONFIG_OK on success or an error code on failure
 */
static int mmconfig_str_to_int(char *str, int *val)
{
    uint32_t num = 0;
    int sign = 1;

    if (str[0] == '-')
    {
        sign = -1;
        str++;
    }

    int ret = mmconfig_str_to_uint(str, &num);
    if (ret == MMCONFIG_OK)
    {
        /* Check for overflows */
        if ((sign == -1) && (num > 0x80000000))
        {
            return MMCONFIG_ERR_INCORRECT_TYPE;
        }
        if ((sign == 1) && (num > 0x7FFFFFFF))
        {
            return MMCONFIG_ERR_INCORRECT_TYPE;
        }
        *val = sign * num;
    }

    return ret;
}

int mmconfig_validate_key_character(char character)
{
    if (!isalnum(character) && (character != '_') && (character != '.'))
    {
        return MMCONFIG_ERR_INVALID_KEY;
    }

    return MMCONFIG_OK;
}

int mmconfig_validate_key(const char *key)
{
    uint32_t i;

    /* Validate key length */
    size_t key_len = strnlen(key, MMCONFIG_MAX_KEYLEN + 1);
    if ((key_len > MMCONFIG_MAX_KEYLEN) || (key_len == 0))
    {
        return MMCONFIG_ERR_INVALID_KEY;
    }

    /* Validate first char is alpha */
    if (!isalpha((int)key[0]))
    {
        return MMCONFIG_ERR_INVALID_KEY;
    }

    /* Validate rest of key */
    for (i = 1; i < key_len; i++)
    {
        if (mmconfig_validate_key_character(key[i]) != MMCONFIG_OK)
        {
            if ((key[i] == '*') && (i == (key_len - 1)))
            {
                /* Asterisk wildcard currently only supported at end of key */
                return MMCONFIG_ERR_WILDCARD_KEY;
            }
            return MMCONFIG_ERR_INVALID_KEY;
        }
    }

    /* All good */
    return MMCONFIG_OK;
}


/**
 * Reads data identified by the supplied key from persistent memory returning a pointer.
 * @note This is an internal function.
 *
 * @param key           Identifies the data element in persistent storage and is a
 *                      case insensitive alphanumeric (plus underscore) string starting
 *                      with an alpha. Same rules as a C variable name, but case insensitive.
 *                      Must be a null terminated string.
 * @param data          Returns a live pointer to the data in flash memory.  It is the callers
 *                      responsibility to consume it immediately or take a copy as this pointer
 *                      will be invalidated on the next config store write.
 *                      Returns NULL on any error.
 * @return              Returns number of bytes read and allocated on success. On error returns:
 *                          @c MMCONFIG_ERR_INVALID_KEY if key is invalid
 *                          @c MMCONFIG_ERR_NOT_FOUND if the specified key was not found
 *                          Other negative number for other errors.
 */
static int mmconfig_read_data(const char *key, void **data)
{
    /* Ensure key is valid */
    if (mmconfig_validate_key(key) != MMCONFIG_OK)
    {
        return MMCONFIG_ERR_INVALID_KEY;
    }

    for(int i=0;mm_configs[i].key && mm_configs[i].value;i++)
    {
        if(strcmp(key,mm_configs[i].key)==0)
        {
            *data = mm_configs[i].value;
            return strlen(mm_configs[i].value) + 1;
        }
    }

    return MMCONFIG_ERR_NOT_FOUND;
}

int mmconfig_read_string(const char *key, char *buffer, int bufsize)
{
    char *value;
    int retval =  mmconfig_read_data(key, (void**) &value);

    /* Check for error */
    if (retval < 0)
    {
        goto mmconfig_read_string_cleanup;
    }

    /* Treat a NULL data (0 length) as invalid, (not same as a NULL string "") */
    if (retval == 0)
    {
        retval = MMCONFIG_ERR_INCORRECT_TYPE;
        goto mmconfig_read_string_cleanup;
    }

    /* Check for NULL termination */
    if (value[retval - 1] != 0)
    {
        retval = MMCONFIG_ERR_INCORRECT_TYPE;
        goto mmconfig_read_string_cleanup;
    }

    /* If buffer is NULL just return the number of bytes required */
    if (buffer)
    {
        /* Check for sufficient buffer */
        if (retval > bufsize)
        {
            retval = MMCONFIG_ERR_INSUFFICIENT_MEMORY;
            goto mmconfig_read_string_cleanup;
        }

        /* All good, do it */
        memcpy(buffer, value, retval);
    }

mmconfig_read_string_cleanup:

    return retval;
}

int mmconfig_read_int(const char *key, int *value)
{
    /* For maximum compatibility, we are going to represent the integer as a string */
    char *data;
    int retval =  mmconfig_read_data(key, (void**) &data);

    /* Check for error */
    if (retval < 0)
    {
        goto mmconfig_read_int_cleanup;
    }

    /* Treat a NULL data (0 length) as invalid */
    if (retval == 0)
    {
        retval = MMCONFIG_ERR_INCORRECT_TYPE;
        goto mmconfig_read_int_cleanup;
    }

    /* Check for NULL termination */
    if (data[retval - 1] != 0)
    {
        retval = MMCONFIG_ERR_INCORRECT_TYPE;
        goto mmconfig_read_int_cleanup;
    }

    /* Try to convert to int */
    if (mmconfig_str_to_int(data, value) == MMCONFIG_OK)
    {
        retval = MMCONFIG_OK;
        goto mmconfig_read_int_cleanup;
    }

    retval = MMCONFIG_ERR_INCORRECT_TYPE;

mmconfig_read_int_cleanup:

    return retval;
}

int mmconfig_read_uint32(const char *key, uint32_t *value)
{
    /* For maximum compatibility, we are going to represent the value as a string */
    char *data;

    int retval =  mmconfig_read_data(key, (void**) &data);

    /* Check for error */
    if (retval < 0)
    {
        goto mmconfig_read_uint32_cleanup;
    }

    /* Treat a NULL data (0 length) as invalid */
    if (retval == 0)
    {
        retval = MMCONFIG_ERR_INCORRECT_TYPE;
        goto mmconfig_read_uint32_cleanup;
    }

    /* Check for NULL termination */
    if (data[retval - 1] != 0)
    {
        retval = MMCONFIG_ERR_INCORRECT_TYPE;
        goto mmconfig_read_uint32_cleanup;
    }

    /* Is it a hexadecimal or plain numeric string */
    if (mmconfig_str_to_uint(data, value) == MMCONFIG_OK)
    {
        retval = MMCONFIG_OK;
        goto mmconfig_read_uint32_cleanup;
    }

    retval = MMCONFIG_ERR_INCORRECT_TYPE;

mmconfig_read_uint32_cleanup:

    return retval;
}

int mmconfig_read_bool(const char *key, bool *value)
{
    /* For maximum compatibility, we are going to represent the integer as a string */
    char *data;

    int retval =  mmconfig_read_data(key, (void**) &data);

    /* Check for error */
    if (retval < 0)
    {
        goto mmconfig_read_bool_cleanup;
    }

    /* Treat a NULL data (0 length) as invalid */
    if (retval == 0)
    {
        retval = MMCONFIG_ERR_INCORRECT_TYPE;
        goto mmconfig_read_bool_cleanup;
    }

    /* Single byte encoded bool */
    if (retval == 1)
    {
        /* Non zero is true */
        *value = (data[0] != 0);
        goto mmconfig_read_bool_cleanup;
    }

    /* Length > 1, must be a string, check for NULL termination */
    if (data[retval - 1] != 0)
    {
        retval = MMCONFIG_ERR_INCORRECT_TYPE;
        goto mmconfig_read_bool_cleanup;
    }

    if (strcasecmp(data, "true") == 0)
    {
        *value = true;
        retval = MMCONFIG_OK;
        goto mmconfig_read_bool_cleanup;
    }

    if (strcasecmp(data, "false") == 0)
    {
        *value = false;
        retval = MMCONFIG_OK;
        goto mmconfig_read_bool_cleanup;
    }

    /* Try to convert to int */
    int tmp;
    if (mmconfig_str_to_int(data, &tmp) == MMCONFIG_OK)
    {
        *value = (tmp != 0);
        retval = MMCONFIG_OK;
        goto mmconfig_read_bool_cleanup;
    }

    /* Didn't match anything we could interpret as bool */
    retval = MMCONFIG_ERR_INCORRECT_TYPE;

mmconfig_read_bool_cleanup:

    return retval;
}

int mmconfig_write_update_node_list(const struct mmconfig_update_node *node_list)
{
    int retval = MMCONFIG_ERR_FULL;

    return retval;
}

int mmconfig_alloc_and_load(const char *key, void **data)
{
    int length;
    void *livedata;

    /* So we return NULL on error */
    *data = NULL;

    /* Look for the key in config store */
    length = mmconfig_read_data(key, &livedata);

    if (length < 0)
    {
        /* We got an error */
        goto mmconfig_alloc_and_load_cleanup;
    }

    /* Allocate memory */
    *data = malloc(length);
    if (*data == NULL)
    {
        length = MMCONFIG_ERR_INSUFFICIENT_MEMORY;
        goto mmconfig_alloc_and_load_cleanup;
    }

    memcpy(*data, livedata, length);

mmconfig_alloc_and_load_cleanup:
    return length;
}

int mmconfig_read_bytes(const char *key, void *buffer,  uint32_t buffsize, uint32_t offset)
{
    int length;
    uint8_t *data;
    int result;
    int copied;


    /* Look for the key in config store */
    length = mmconfig_read_data(key, (void**)&data);

    if (length < 0)
    {
        /* We got an error */
        result = length;
        goto mmconfig_read_bytes_cleanup;
    }

    if (offset > (uint32_t)length)
    {
        result = MMCONFIG_ERR_OUT_OF_BOUNDS;
        goto mmconfig_read_bytes_cleanup;
    }


    /* If buffer is NULL, just return the length required */
    if (buffer == NULL)
    {
        result = length;
        goto mmconfig_read_bytes_cleanup;
    }

    copied = buffsize < (length - offset) ? buffsize : length - offset;
    memcpy(buffer, &data[offset], copied);
    result = copied;

mmconfig_read_bytes_cleanup:
    return result;
}
