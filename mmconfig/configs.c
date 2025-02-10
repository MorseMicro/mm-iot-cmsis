/*
 * Copyright 2021-2023 Morse Micro
 *
 * This file is licensed under terms that can be found in the LICENSE.md file in the root
 * directory of the Morse Micro IoT SDK software package.
 */

#include "mmconfig.h"

__attribute__((weak)) config_entry_t mm_configs[]={
         /* tcp_client */
        {"iperf.mode", "udp_server"},
        /* IP address of server to connect to when in client mode */
        {"iperf.server", "192.168.1.1"},
        /* Specifies the port to listen on in server mode. */
        {"iperf.port", "5001"},
        /* Duration for client transfers specified either in seconds or bytes. If this is
         * negative, it specifies a time in seconds; if positive, it specifies the number
         * of bytes to transmit. */
        {"iperf.amount", "-10"},

        {"ping.target", "192.168.1.1"},
        {"ping.count", "10"},
        {"ping.interval", "1000"},
        {"ping.size", "56"},
        {"ping.post_ping_delay_ms", "60"},

        /* The following setting is required only for the wnm_sleep example */
        {"wlan.wnm_sleep_duration_ms", "20000"},

        /* The WiFi SSID */
        {"wlan.ssid", "MorseMicro"},
        /* The WiFi password, not required if wlan.security is open */
        {"wlan.password", "12345678"},
        /* The WiFi security to use, valid values are sae, owe and open */
        {"wlan.security", "sae"},
        /* The 2 letter country code to ensure the correct regulatory domain is used */
        {"wlan.country_code","US"},

        /* If true use DHCP, else the static IP configuration will be used */
        {"ip.dhcp_enabled", "true"},

        /* These settings are ignored if DHCP is used, but mandatory if using static IP */
        {"ip.ip_addr", "192.168.1.2"},
        {"ip.netmask", "255.255.255.0"},
        {"ip.gateway", "192.168.1.1"},

        /* These settings are for IPv6, ip6.ip_addr is only required if ip6.autoconfig is
         * false */
        {"ip6.autoconfig", "true"},
        // "ip6.ip_addr": "FE80::2"

        /* Note: The following settings should be used only when explicitly required */
        /* If specified, use this MAC address for the WLAN interface */
        // "wlan.macaddr": "02:00:12:34:56:78"
        /* BSSID of the AP to connect to. If non-zero, the STA will only connect to an AP
         * that matches this value. */
        // "wlan.bssid": "00:00:00:00:00:00"
        /* S1G non-AP STA type, valid values are sensor and non_sensor */
        // "wlan.station_type": "non_sensor"
        /* Whether Centralized Authentication Controlled is enabled on the STA. */
        // "wlan.cac_enabled": "true"
        /* Priority used by the AP to assign a STA to a Restricted Access Window (RAW)
         * group. Valid range is 0 - 7, or -1 to disable RAW. */
        // "wlan.raw_priority": "-1"
        // "wlan.power_save_mode": "enabled"
        // "wlan.subbands_enabled": "true"
        // "wlan.sgi_enabled": "true"
        // "wlan.ampdu_enabled": "true"
        // "wlan.fragment_threshold": "0"
        // "wlan.rts_threshold": "0"

        /* The following settings are required only for the sslclient app */
        // {"sslclient.server", "www.google.com"},
        // {"sslclient.port", "443"},

        /* The following settings are required only for the mqttdemo app */
        /* IP address of the MQTT broker to connect to.  test.mosquitto.org is a publicly
         * available MQTT broker. */
        // {"mqtt.server", "test.mosquitto.org"},
        /* Specifies the MQTT port on the server to connect to, the default is 1883 */
        // {"mqtt.port", "1883"},
        /* The amount of time in ms to wait before publishing */
        // {"mqtt.publish_delay", "1000"},
        /* The client id string uniquely identifies this device to the broker */
        // {"mqtt.clientid", "<Put your clientid here>"},
        /* The topic to publish/subscribe to.  To keep this unique, insert your clientid in
         * it. Not doing so may cause you to publish to someone elses device and annoy
         * them. */
        // {"mqtt.topic", "/MorseMicro/<Put your clientid here>/topic"},
        /* A test message you wish to send */
        // {"mqtt.message", "G'day World!"},

        /*
        {"aws.thingname", "Put your thing name here"},
        {"aws.endpoint", "<your aws endpoint here>.amazonaws.com"},

        {"aws.rootca",
            "-----BEGIN CERTIFICATE-----\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF                                0123456789ABCDEF\n"
            "0123456789ABCDEF       AmazonRootCA1.pem        0123456789ABCDEF\n"
            "0123456789ABCDEF                                0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789AB\n"
            "-----END CERTIFICATE-----\n"
        },
        {"aws.devicecert",
            "-----BEGIN CERTIFICATE-----\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF                                0123456789ABCDEF\n"
            "0123456789ABCDEF      certificate.pem.crt       0123456789ABCDEF\n"
            "0123456789ABCDEF                                0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789AB\n"
            "-----END CERTIFICATE-----        \n"
        },
        {"aws.devicekeys",
            "-----BEGIN RSA PRIVATE KEY-----\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF                                0123456789ABCDEF\n"
            "0123456789ABCDEF        private.pem.key         0123456789ABCDEF\n"
            "0123456789ABCDEF                                0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF01234567\n"
            "-----END RSA PRIVATE KEY-----\n"
        },
        */
        //array terminator
        {NULL,NULL}
};