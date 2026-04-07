/*
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Dirk Ziegelmeier <dziegel@gmx.de>
 *
 */

#include "demo_mdns.h"
#include "lwip/apps/mdns.h"
#include "lwip/tcpip.h"

static bool mdns_initialized = false;

#if LWIP_MDNS_RESPONDER
static void srv_txt(struct mdns_service *service, void *txt_userdata)
{
    err_t res;
    LWIP_UNUSED_ARG(txt_userdata);
    const char *const service_txt = "lwip-rangetest-api=v1.0";

    res = mdns_resp_add_service_txtitem(service, service_txt, strlen(service_txt));
    LWIP_ERROR("mdns add service txt failed\n", (res == ERR_OK), return );
}

static void mdns_report(struct netif *netif, u8_t result, s8_t service)
{
    LWIP_PLATFORM_DIAG(("mdns status[netif %d][service %d]: %d\n", netif->num, service, result));
}
#endif

void mdns_init(struct netif *mmnetif)
{
#if LWIP_MDNS_RESPONDER
    char ekh05_domain[11];
    sprintf(ekh05_domain, "ekh05-%02x%02x", mmnetif->hwaddr[4], mmnetif->hwaddr[5]);
    mdns_resp_register_name_result_cb(mdns_report);
    mdns_resp_init();
    mdns_resp_add_netif(mmnetif, ekh05_domain);
    mdns_resp_add_service(mmnetif, ekh05_domain, "_http", DNSSD_PROTO_TCP, 80, srv_txt, NULL);
    mdns_resp_announce(mmnetif);
#endif
}


void mdns_link_update(struct netif *mmnetif)
{
#if LWIP_MDNS_RESPONDER
    LOCK_TCPIP_CORE();
    if (!mdns_initialized)
    {
        mdns_initialized = true;
        mdns_init(mmnetif);
    }
    else
    {
        mdns_resp_announce(mmnetif);
    }
    UNLOCK_TCPIP_CORE();
#endif
}