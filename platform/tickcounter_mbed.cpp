// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "mbed.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/tickcounter.h"
#include "mylog.h"

// the Mbed OS implements a timer class for timing

TICK_COUNTER_HANDLE tickcounter_create(void)
{
    Timer* t = new Timer;
    t->start();
    return t;
}

void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter)
{
    Timer* t = (Timer*)tick_counter;
    if (t != NULL)
        delete tick_counter;
}

int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t * current_ms)
{
    Timer* t = (Timer*)tick_counter;
    if (t == NULL || current_ms == NULL)
        return __FAILURE__;
    
    *current_ms = t->read_ms();
    return 0;
}
