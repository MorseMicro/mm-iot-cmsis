/*
 * Copyright 2022-2023 Morse Micro
 *
 * This file is licensed under terms that can be found in the LICENSE.md file in the root
 * directory of the Morse Micro IoT SDK software package.
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

static char cgi_buffer[128] = {0};

void update_ble_ip_report()
{
    char ip[51], gw[48]; /* 3 more chars for IP to show netmask: /xx at the end. */
    uint8_t mask_counter = 0;
    char *res;
    struct netif *netif;
    /* Search through all the interfaces and find the Morse netif*/
    for (netif = netif_list; netif != NULL; netif = netif->next)
    {
        if (netif->name[0] == 'M' && netif->name[1] == 'M')
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
 * Get the sensors values in a json string.
 *
 * @param fil   File to write to.
 */
static void rest_ep_get_sensors(struct restfs_file* fil)
{
    restfs_alloc_buffer(fil, sizeof(cgi_buffer));
    accelerometer_value_t xyz = get_accelerometer_values();
    th_value_t th = get_th_values();
    sprintf(cgi_buffer,
            "{\"x\":\"%d\",\"y\":\"%d\",\"z\":\"%d\",\"T\":\"%ld\",\"H\":\"%ld\",\"ping_running\":%s}",
            xyz.x, xyz.y, xyz.z, th.temperature_milli_degC, th.humidity_milli_RH,
            ping_get_in_progress() ? "true" : "false");
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

    int i;
    uint32_t u32_value=0;

    (void)index;

    for (i = 0; i < nparams; i++)
    {
        if (!strncmp(params[i], "ip", sizeof("ip")))
        {
            ping_set_IP(values[i]);
            printf("setting ping target to: %s",values[i]);
        }

        if (!strncmp(params[i], "count", sizeof("count")))
        {
            u32_value = (uint32_t)strtoul(values[i], NULL, 10);
            ping_set_count(u32_value);
            printf("setting ping count to: %ld",u32_value);
        }
    }
    ping_start();

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
    {"/rest/get_image", rest_ep_get_image},
    {"/rest/get_saved_image", rest_ep_get_saved_image},
    {"/rest/get_sensors", rest_ep_get_sensors},
    {"/rest/get_configs", rest_ep_get_configs},
    {"/rest/toggle_leds", rest_ep_toggle_leds},
    {"/rest/get_terminal", rest_ep_get_terminal},
    {"/rest/stop_operation", rest_ep_stop_operation},
};

/**
 * Vector table of LWIP CGI endpoints. Declare the URI and handlers for CGI endpoints here
 *
 * Will pass query parameters to function call.
 * For example, `<ip_address>/rest/<endpoint>?queryname=queryval&queryname2=queryval2` ... etc.
 */

static const tCGI cgi_endpoints[] = {
    {"/rest/trigger_ping", cgi_set_ping_start},
};

/**
 * Main entry point to the application. This will be invoked in a thread once operating system
 * and hardware initialization has completed. It may return, but it does not have to.
 */
void app_init(void)
{
    printf("\n\nEKH05 Demo Example(Built " __DATE__ " " __TIME__ ")\n\n");
    periphs_start();

    ping_init();
    shared_buffer_init(&http_terminal_buffer);

    /* Initialize and connect to WiFi, blocks till connected */
    app_wlan_init();
    app_wlan_start();

    /* Update BLE reported IP and Gateway*/
    update_ble_ip_report();

    LOCK_TCPIP_CORE();
    rest_init_endpoints(rest_endpoints, LWIP_ARRAYSIZE(rest_endpoints));
    http_set_cgi_handlers(cgi_endpoints, LWIP_ARRAYSIZE(cgi_endpoints));
    httpd_init();
    UNLOCK_TCPIP_CORE();

    /* We idle till we get a connection */
}
