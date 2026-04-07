/*
 * Copyright 2025 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
 */



#include "mmosal.h"
#include "demo_http.h"
#include "lwip/netif.h"
#include "gatt_db.h"
#include "peripherals.h"
#include "demo_ping.h"
#include "demo_iperf.h"
#include "demo_mdns.h"
#include "mmregdb.h"
#include "mm_app_common.h"
#include "main.h"
#include "demo_cli.h"
#include "mmagic_cli.h"

#define AUTO_CONNECT_DELAY  5
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)


extern struct mmagic_cli *mmagic_cli_ctx;
void mmagic_cli_wlan_connect(EmbeddedCli *cli, char *args, void *context);

static struct mmosal_semb *link_status_changed_semb = NULL;
struct mmosal_task *link_watch_task_p;


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
        LWIP_ASSERT("GW buf too short", res != NULL);
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

static void link_status_callback(const struct mmipal_link_status *link_status)
{
    mmosal_semb_give(link_status_changed_semb);
}

static void link_watch_task(void *arg)
{
    while (true)
    {
        if (mmosal_semb_wait(link_status_changed_semb, UINT32_MAX))
        {
            struct netif *mmnetif;
            /* let the mdns and ble now that the link status is update.*/
            mmnetif = get_mmnetif();
            if (mmnetif)
            {
                if(mmipal_get_link_state() == MMIPAL_LINK_UP)
                {
                    /* Update BLE reported IP and Gateway*/
                    update_ble_ip_report(mmnetif);

                    /* Update mdns*/
                    mdns_link_update(mmnetif);
                }
                else
                {
                    update_ble_ip_gw("NOT-CONNECTED", "NOT-CONNECTED");
                }
            }
        }
    }
}


/**
 * Main entry point to the application. This will be invoked in a thread once operating system
 * and hardware initialization has completed. It may return, but it does not have to.
 */
void app_init(void)
{
    periphs_start();
    ping_init();
    iperf_init();
#ifndef LIMITED_DEMO_EXAMPLE
    int ac_delay; /* Auto Connect Delay*/
    MMOSAL_ASSERT(link_status_changed_semb == NULL);
    link_status_changed_semb = mmosal_semb_create("link_status_changed");
    link_watch_task_p = mmosal_task_create(link_watch_task, NULL, MMOSAL_TASK_PRI_NORM, 512, "LinkWatch");
    if (link_watch_task_p == NULL)
    {
        printf("Unable to Link status watcher task\n\r");
    }
    cli_init();
    mmipal_set_link_status_callback(link_status_callback);
    embeddedCliPrint(mmagic_cli_ctx->cli, "EKH05 Demo Example(Built " __DATE__ " " __TIME__ ")\n\n");

    /* Wait for AUTO_CONNECT_DELAY seconds and if there were no uart input,
    * start connecting to AP.*/
    embeddedCliPrint(mmagic_cli_ctx->cli, "Auto connecting in " TOSTRING(AUTO_CONNECT_DELAY) " seconds. Press any key to skip.");
    ac_delay = AUTO_CONNECT_DELAY * 10; /*multiply by a number to reduce the between checks*/
    do
    {
        HAL_GPIO_TogglePin(GPIO_LED_GREEN_GPIO_Port, GPIO_LED_GREEN_Pin);
        if(cli_get_uart_data_received())
        {
            embeddedCliPrint(mmagic_cli_ctx->cli, "Skipping auto connect.");
            break;
        }
        mmosal_task_sleep(100);
    } while (--ac_delay);

    if(ac_delay == 0)
    {
        cli_set_auto_connect_in_progress(true);
        HAL_GPIO_WritePin(GPIO_LED_GREEN_GPIO_Port, GPIO_LED_GREEN_Pin, GPIO_PIN_SET);
        mmagic_cli_wlan_connect(mmagic_cli_ctx->cli, NULL, NULL);
        cli_set_auto_connect_in_progress(false);
    }

    HAL_GPIO_WritePin(GPIO_LED_GREEN_GPIO_Port, GPIO_LED_GREEN_Pin, GPIO_PIN_RESET);
    http_init();
#else
    LL_GPIO_ResetOutputPin(RESET_N_GPIO_Port, RESET_N_Pin);
#endif
    /* We idle till we get a connection */
}
