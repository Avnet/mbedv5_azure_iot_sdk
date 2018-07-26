// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/lock.h"
#include "azure_c_shared_utility/xlogging.h"
#include "mbed.h"

// the Lock functionallity is the same as a Mutes in the Mbed OS so these
// API's provide Azure with a common api for manipulating them
//

LOCK_HANDLE Lock_Init(void)
{
    Mutex* result = new Mutex();
    return (LOCK_HANDLE)result;
}

LOCK_RESULT Lock(LOCK_HANDLE handle)
{
    Mutex* lock_mtx = (Mutex*)handle;
    if (lock_mtx == NULL)
        return LOCK_ERROR;

    return lock_mtx->trylock()? LOCK_OK : LOCK_ERROR;
}

LOCK_RESULT Unlock(LOCK_HANDLE handle)
{
    Mutex* lock_mtx = (Mutex*)handle;
    if (lock_mtx == NULL)
        return LOCK_ERROR;

    return (lock_mtx->unlock() == osOK)? LOCK_OK : LOCK_ERROR;
}

LOCK_RESULT Lock_Deinit(LOCK_HANDLE handle)
{
    Mutex* lock_mtx = (Mutex*)handle;
    if (lock_mtx == NULL)
        return LOCK_ERROR;
    delete lock_mtx;
    return LOCK_OK;
}
