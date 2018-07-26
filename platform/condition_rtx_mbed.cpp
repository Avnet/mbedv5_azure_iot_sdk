// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "mbed.h"
#include "azure_c_shared_utility/lock.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/condition.h"
#include "mylog.h"

// The Mbed OS doesn't use these functions so simply return the appropriate 
// condition.

COND_HANDLE Condition_Init(void)
{
    return NULL;
}

COND_RESULT Condition_Post(COND_HANDLE handle)
{
    return (handle == NULL)? COND_INVALID_ARG:COND_ERROR;
}

COND_RESULT Condition_Wait(COND_HANDLE  handle, LOCK_HANDLE lock, int timeout_milliseconds)
{
    return (handle == NULL)? COND_INVALID_ARG:COND_ERROR;
}

void Condition_Deinit(COND_HANDLE handle)
{
    return;
}
