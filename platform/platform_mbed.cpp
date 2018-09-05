// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/platform.h"
#include "easy-connect.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xio.h"
#include "NTPClient.h"

#if MBED_CONF_APP_NETWORK_INTERFACE == CELLULAR_BG96
#include "BG96Interface.h"
#endif

#define TZ_OFFSET	-5   //GMT offset for this timezone

extern "C" const IO_INTERFACE_DESCRIPTION* tlsio_mbedtls_get_interface_description(void);

NetworkInterface* platform_network;

int platform_init(void)
{
    platform_network = easy_connect(true);
    if (!platform_network) 
        return __FAILURE__;

    #if MBED_CONF_APP_NETWORK_INTERFACE == CELLULAR_BG96
    printf("[ Platform  ] BG96 SW Revision: %s\n", FIRMWARE_REV(platform_network));
    #endif
    NTPClient ntp(platform_network);
    time_t timestamp=ntp.get_timestamp();

    set_time(timestamp);
    printf("[ Platform  ] Time set to %s\n",ctime(&timestamp));
    return 0;
}

int platform_RSSI(void)
{
    int rssi=0;
    #if MBED_CONF_APP_NETWORK_INTERFACE == CELLULAR_BG96
    rssi=BG96_RSSI(platform_network);
    #endif
    return rssi;
}

STRING_HANDLE platform_get_platform_info(void)
{
    // Expected format: "(<runtime name>; <operating system name>; <platform>)"
    return STRING_construct("(native; mbed v5.x; undefined)");
}

void platform_deinit(void)
{
    return;
}
