// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/platform.h"
#include "easy-connect.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/tlsio_mbedtls.h"

extern "C" const IO_INTERFACE_DESCRIPTION* tlsio_mbedtls_get_interface_description(void);

int platform_init(void)
{
    NetworkInterface *network = easy_connect(true);
    if (!network) 
        return __FAILURE__;
        
    return 0;
}

const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
    return tlsio_mbedtls_get_interface_description();
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

