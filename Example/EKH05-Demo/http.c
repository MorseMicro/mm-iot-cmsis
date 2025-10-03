/*
 * Copyright 2022-2023 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief HTTP server example, with APIs for RESTful interfaces.
 *
 * @note It is assumed that you have followed the steps in the @ref GETTING_STARTED guide and are
 * therefore familiar with how to build, flash, and monitor an application using the MM-IoT-SDK
 * framework.
 *
 * This file contains examples on how to use the lwIP http server.
 * It supports static and dynamically generated pages.
 *
 * Static pages are specified in the @c fs/ directory, and on compilation will be automatically
 * converted to C arrays which will be embedded into the binary.
 * Currently the file converter is compiled to run only on x86 linux systems. To support other
 * systems, @c htmlgen can be recompiled on your development machine from the mainline lwIP sources.
 *
 * Dynamic pages are specified through the custom file system, @c restfs, and are registered by
 * specifying the URI and handler function in the @c rest_endpoints array.
 *
 * Note that only GET requests are supported at this time.
 *
 * To view the web-page, navigate to
 * @code
 * http://<device_ip>/index.html
 * @endcode
 * on a computer with a route to the Morse Micro IoT device, over HaLow. Alternatively, you can run
 * @code
 * curl http://<device_ip>/index.html
 * @endcode
 * from the AP that the device is connected to.
 *
 * See @ref APP_COMMON_API for details of WLAN and IP stack configuration.
 */

#include <string.h>
#include "mmosal.h"
#include "mmwlan.h"
#include "mmutils.h"

#include "mmipal.h"
#include "lwip/apps/httpd.h"

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/ip_addr.h"

#include "restfs.h"
#include "mm_app_common.h"
#include "shared_buffer.h"
#include "demo_accelerometer.h"
#include "demo_temperature.h"
#include "demo_ping.h"
#include "gatt_db.h"
#include "peripherals.h"
#include "demo_iperf.h"
#include "demo_mdns.h"
#include "mm_app_regdb.h"
#include "main.h"

static char cgi_buffer[256] = {0};

/* http_terminal_buffer is a shared buffer to hold the output of ping or iperf task, so http can
 * read it from.*/
SharedBuffer http_terminal_buffer;

static struct netif *get_mmnetif()
{
    struct netif *netif;
    /* Search through all the interfaces and find the Morse netif*/
    for (netif = netif_list; netif != NULL; netif = netif->next)
    {
        if (netif->name[0] == 'M' && netif->name[1] == 'M')
        {
            return netif;
        }
    }
    return NULL;
}

static void update_ble_ip_report(struct netif *netif)
{
    char ip[51], gw[48]; /* 3 more chars for IP to show netmask: /xx at the end. */
    uint8_t mask_counter = 0;
    char *res;
    if (netif)
    {
        res = ipaddr_ntoa_r(&netif->ip_addr, ip, sizeof(ip));
        LWIP_ASSERT("IP buf too short", res != NULL);

        res = ipaddr_ntoa_r(&netif->gw, gw, sizeof(gw));
        LWIP_ASSERT("IP buf too short", res != NULL);
        /* get CIDR notation of the netmask and add it to the IP*/
        for (int i = 0; i < 32; i++)
        {
            if (netif->netmask.u_addr.ip4.addr & (1 << i))
            {
                mask_counter++;
            }
        }
        sprintf(ip + strlen(ip), "/%d", mask_counter);
        update_ble_ip_gw(ip, gw);
    }
}

/**
 * Example endpoint to return fixed html string
 *
 * @param fil   File to write to.
 */
static void rest_ep_success(struct restfs_file *fil)
{
    static const char successhtml[] = "<html><body>Success</body></html>";

    restfs_write_const(fil, successhtml, strlen(successhtml));
}

/**
 * Example endpoint to return fixed html string
 *
 * @param fil   File to write to.
 */
static void rest_ep_failed(struct restfs_file *fil)
{
    static const char failedhtml[] = "<html><body>Failed</body></html>";

    restfs_write_const(fil, failedhtml, strlen(failedhtml));
}

static void rest_ep_toggle_leds(struct restfs_file *fil)
{
    uint8_t color=periphs_toggle_leds();
    switch (color)
    {
    case 0:
        sprintf(cgi_buffer, "{\"color\":\"red\"}");
        break;
    case 1:
        sprintf(cgi_buffer, "{\"color\":\"blue\"}");
        break;
    case 2:
        sprintf(cgi_buffer, "{\"color\":\"green\"}");
        break;

    default:
        break;
    }
    restfs_alloc_buffer(fil, strlen(cgi_buffer));
    restfs_write(fil, (const uint8_t*)cgi_buffer, strlen(cgi_buffer));
}

/**
 * returns true or false string based on input.
 */
static const char *b2s(bool b)
{
    if (b)
        return "true";
    else
        return "false";
}

/**
 * Get the sensors values in a json string.
 *
 * @param fil   File to write to.
 */
static void rest_ep_get_status(struct restfs_file* fil)
{
    restfs_alloc_buffer(fil, sizeof(cgi_buffer));
    accelerometer_value_t xyz = get_accelerometer_values();
    th_value_t th = get_th_values();
    sprintf(cgi_buffer,
            "{"
            "\"x\":\"%d\","
            "\"y\":\"%d\","
            "\"z\":\"%d\","
            "\"T\":\"%ld\","
            "\"H\":\"%ld\","
            "\"ping_running\":%s,"
            "\"iperf_client_running\":%s,"
            "\"iperf_serv_t\":%s,"
            "\"iperf_serv_u\":%s"
            "}",
            xyz.x, xyz.y, xyz.z, th.temperature_milli_degC, th.humidity_milli_RH,
            b2s(ping_get_in_progress()), b2s(iperf_is_client_in_progress()),
            b2s(iperf_is_tcp_server_up()), b2s(iperf_is_udp_server_up()));
    restfs_write(fil, (const uint8_t *)cgi_buffer, strlen(cgi_buffer));
}
static void rest_ep_get_configs(struct restfs_file* fil)
{
    restfs_alloc_buffer(fil, sizeof(cgi_buffer));
    sprintf(cgi_buffer, "{\"pingtarget\":\"%s\",\"pingcount\":\"%ld\"}", ping_get_target(),
            ping_get_count());
    restfs_write(fil, (const uint8_t *)cgi_buffer, strlen(cgi_buffer));
}

/**
 * Get the content of JPEG image
 *
 * @param fil   File to write to.
 */
static void rest_ep_get_image(struct restfs_file* fil)
{
    if(!periphs_jpeg_buffer_lock())
    {
        rest_ep_failed(fil);
        return;
    }
    if(!periphs_get_live_jpeg_size())
    {
        periphs_jpeg_buffer_unlock();
        return;
    }
    restfs_write_const(fil, (const char*)periphs_get_jpeg_buffer(), (uint16_t)periphs_get_live_jpeg_size());
}

/**
 * Get the content of JPEG image that is saved in the QSPI flash
 *
 * @param fil   File to write to.
 */
static void rest_ep_get_saved_image(struct restfs_file *fil)
{

    if (!periphs_qspi_flash_lock())
    {
        rest_ep_failed(fil);
        return;
    }
    /* If we can acquire the lock, it means that the QSPI flash is not being written and it's in
     * memory mapped mode. So, no need to check memory mapped before accessing it.*/
    if (!SAVED_IMAGE_DATA_SIZE || !(SAVED_IMAGE_DATA_SIZE < (64 * 1024 - 4)))
    {
        periphs_qspi_flash_unlock();
        rest_ep_failed(fil);
        return;
    }
    restfs_write_const(fil, (const char *)(OCTOSPI1_BASE + sizeof(uint32_t)),
                       (uint16_t)SAVED_IMAGE_DATA_SIZE);
}

/**
 * Searching through the parameters to find the index of the one of interest.
 *
 * @param nparams       Number of elements in @p params.
 * @param params        Parameter names.
 * @param target        Target parameter name.
 * @returns Index of the of the target parameter, -1 if not found.
 *
 */
static int get_parameter_index(int nparams, char *params[], const char* target)
{
    int i;
    int target_len=strlen(target);
    for (i = 0; i < nparams; i++)
    {
        if (!strncmp(params[i], target, target_len))
        {
            return i;
        }
    }
    return -1;
}

/**
 * Searching through the parameters to find the indexes of all the ones of interest.
 *
 * @param params                   Parameter names to search in.
 * @param params_of_interest       Name of the parameters that are going to be searched.
 * @param indexes                  The array that the found parameter indexes are going to be stored in.
 * @param nparams                  Number of elements in @p params_of_interest.
 * @returns 0 if all the parameters of interrest are found, -1 otherwise.
 *
 */
static int find_param_indexes(char * params[], const char *const params_of_interest[],
                              uint8_t *indexes, uint8_t nparams)
{
    int idx;
    for (int i = 0; i < nparams; i++)
    {
        idx = get_parameter_index(nparams, params, params_of_interest[i]);
        if (idx < 0)
        {
            printf("ERROR: parameter not found: %s\n", params_of_interest[i]);
            return -1;
        }
        else
        {
            indexes[i] = (uint8_t)idx;
        }
    }
    return 0;
}

/**
 * CGI handler to set a global variable based on query parameters.
 *
 * @param index         The index of the endpoint in the CGI handlers table.
 * @param nparams       Number of elements in @p params provided in the URI.
 * @param params        Parameter names provided in the URI.
 * @param values        Values corresponding to each parameter given in the URI.
 * @returns the URI of the response to return (which can be a REST endpoint).
 *
 * @note Strictly speaking GET requests should not be altering state on the server,
 * but for this example we will use it as a lightweight alternative to POST.
 *
 */

static const char *cgi_set_ping_start(int index, int nparams, char *params[], char *values[])
{
    printf("Ping starting ... \n");

    uint32_t count=0;

    (void)index;

    const char *const params_of_interest[] = {"ip", "count"};
    uint8_t indexes[2];
    if (find_param_indexes(params, params_of_interest, indexes, sizeof(indexes)))
    {
        return "failed.html";
    }

    count = (uint32_t)strtoul(values[indexes[1]], NULL, 10);

    ping_set_IP(values[indexes[0]]);
    printf("setting ping target to: %s", values[0]);
    ping_set_count(count);
    printf("setting ping count to: %ld",count);
    ping_start();

    return "success.html";
}

static const char *cgi_start_iperf_server_udp(int index, int nparams, char *params[], char *values[])
{
    int i;
    iperf_cmd_args_t iperf_cmd_args;
    (void)index;

    i = get_parameter_index(nparams, params, "port");
    if (i < 0)
    {
        printf("ERROR: parameter not found: port\n");
        return "failed.html";
    }
    iperf_cmd_args.port = (uint16_t)strtoul(values[i], NULL, 10);
    if(!iperf_cmd_args.port)
    {
        return "failed.html";
    }
    iperf_cmd_args.mode = IPERF_MODE_UDP_SERVER;
    if( !iperf_push_cmd(iperf_cmd_args))
    {
        return "failed.html";
    }
    return "success.html";
}

static const char *cgi_start_iperf_server_tcp(int index, int nparams, char *params[], char *values[])
{
    int i;
    iperf_cmd_args_t iperf_cmd_args;
    (void)index;

    i = get_parameter_index(nparams, params, "port");
    if (i < 0)
    {
        printf("ERROR: parameter not found: port\n");
        return "failed.html";
    }
    iperf_cmd_args.port = (uint16_t)strtoul(values[i], NULL, 10);
    if(!iperf_cmd_args.port)
    {
        return "failed.html";
    }
    iperf_cmd_args.mode = IPERF_MODE_TCP_SERVER;
    if( !iperf_push_cmd(iperf_cmd_args))
    {
        return "failed.html";
    }
    return "success.html";
}

static const char *cgi_start_iperf_client(int index, int nparams, char *params[], char *values[])
{
    iperf_cmd_args_t iperf_cmd_args;
    (void)index;

    const char * const params_of_interest[] = {"proto", "port", "ip", "len"};
    uint8_t indexes[4];
    if (find_param_indexes(params, params_of_interest, indexes, sizeof(indexes)))
    {
        return "failed.html";
    }

    if (!strcmp(values[indexes[0]], "UDP"))
    {
        iperf_cmd_args.mode = IPERF_MODE_UDP_CLIENT;
    }
    else if (!strcmp(values[indexes[0]], "TCP"))
    {
        iperf_cmd_args.mode = IPERF_MODE_TCP_CLIENT;
    }
    else
    {
        printf("invalid proto\n");
        return "failed.html";
    }

    iperf_cmd_args.port = (uint16_t)strtoul(values[indexes[1]], NULL, 10);
    if(!iperf_cmd_args.port)
    {
        return "failed.html";
    }

    strncpy(iperf_cmd_args.target_ip,values[indexes[2]],sizeof(iperf_cmd_args.target_ip));
    iperf_cmd_args.amount = (int)strtol(values[indexes[3]], NULL, 10) * -100; //minus to make it time, and the unit is 10ms
    if(!iperf_cmd_args.amount)
    {
        return "failed.html";
    }

    if( !iperf_push_cmd(iperf_cmd_args))
    {
        return "failed.html";
    }
    return "success.html";
}

static void rest_ep_get_terminal(struct restfs_file *fil)
{
    if (shared_buffer_lock(&http_terminal_buffer))
    {
        /* dump the whole buffer */
        restfs_alloc_buffer(fil, strlen(shared_buffer_get(&http_terminal_buffer)) + 1);
        restfs_write(fil, (const uint8_t *)shared_buffer_get(&http_terminal_buffer),
                     strlen(shared_buffer_get(&http_terminal_buffer)) + 1);
        shared_buffer_unlock(&http_terminal_buffer);
        /* clear the buffer */
        shared_buffer_reset(&http_terminal_buffer);
    }
}

static void rest_ep_stop_operation(struct restfs_file *fil)
{
    ping_stop();
    rest_ep_success(fil);
}

/**
 * Vector table of rest endpoints. Declare the URI and handlers for REST endpoints here.
 *
 * For example, HTTP GET on `<ip address>/rest/example_endpoint`
 */
static const struct rest_endpoint rest_endpoints[] = {
    {"success.html", rest_ep_success},
    {"failed.html", rest_ep_failed},
    {"/api/get_image", rest_ep_get_image},
    {"/api/get_saved_image", rest_ep_get_saved_image},
    {"/api/get_status", rest_ep_get_status},
    {"/api/get_configs", rest_ep_get_configs},
    {"/api/toggle_leds", rest_ep_toggle_leds},
    {"/api/get_terminal", rest_ep_get_terminal},
    {"/api/ping_stop", rest_ep_stop_operation},
};

/**
 * Vector table of LWIP CGI endpoints. Declare the URI and handlers for CGI endpoints here
 *
 * Will pass query parameters to function call.
 * For example, `<ip_address>/rest/<endpoint>?queryname=queryval&queryname2=queryval2` ... etc.
 */

static const tCGI cgi_endpoints[] = {
    {"/api/ping_start", cgi_set_ping_start},
    {"/api/iperf_server_udp", cgi_start_iperf_server_udp},
    {"/api/iperf_server_tcp", cgi_start_iperf_server_tcp},
    {"/api/iperf_client", cgi_start_iperf_client},
};

/**
 * Main entry point to the application. This will be invoked in a thread once operating system
 * and hardware initialization has completed. It may return, but it does not have to.
 */
void app_init(void)
{
    struct netif *mmnetif;
    char strval[8];
    const struct mmwlan_s1g_channel_list *channel_list;

    printf("\n\nEKH05 Demo Example(Built " __DATE__ " " __TIME__ ")\n\n");
    periphs_start();

    ping_init();
    iperf_init();
    shared_buffer_init(&http_terminal_buffer);
#ifndef LIMITED_DEMO_EXAMPLE
    /* Check if the country code is set correctly by checking the channel list.
     * and don't call wlan start and init function if not. (This is pretty much
     * same as load_channel_list() but without assertion.)*/
    (void)mmosal_safer_strcpy(strval, "??", sizeof(strval));
    (void)mmconfig_read_string("wlan.country_code", strval, sizeof(strval));
    channel_list = mmwlan_lookup_regulatory_domain(get_regulatory_db(), strval);
    if (channel_list == NULL)
    {
        printf("Could not find specified regulatory domain matching country code %s\n", strval);
        printf("Please set the configuration key wlan.country_code to the correct country code.\n");
        return;
    }
    /* Initialize and connect to WiFi, blocks till connected */
    app_wlan_init();
    app_wlan_start();
    mmnetif = get_mmnetif();

    /* Update BLE reported IP and Gateway*/
    update_ble_ip_report(mmnetif);

    LOCK_TCPIP_CORE();
    /* Start mdns*/
    mdns_init(mmnetif);

    /* Setup HTTP server. */
    rest_init_endpoints(rest_endpoints, LWIP_ARRAYSIZE(rest_endpoints));
    http_set_cgi_handlers(cgi_endpoints, LWIP_ARRAYSIZE(cgi_endpoints));
    httpd_init();
    UNLOCK_TCPIP_CORE();
#else
    LL_GPIO_ResetOutputPin(RESET_N_GPIO_Port, RESET_N_Pin);
#endif
    /* We idle till we get a connection */
}
