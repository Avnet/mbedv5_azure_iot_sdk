// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/platform.h"
#include "easy-connect.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xio.h"
#include "NTPClient.h"
#include "mylog.h"

#define TZ_OFFSET	-5   //GMT offset for this timezone

extern "C" const IO_INTERFACE_DESCRIPTION* tlsio_mbedtls_get_interface_description(void);

NetworkInterface* platform_network;

int platform_init(void)
{
    platform_network = easy_connect(true);
    if (!platform_network) 
        return __FAILURE__;

    NTPClient ntp(platform_network);
    time_t timestamp=ntp.get_timestamp();

    set_time(timestamp);
    printf("Time set to %s",ctime(&timestamp));
    return 0;
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
