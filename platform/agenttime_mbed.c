// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/agenttime.h"
#include "mylog.h"

time_t get_time(time_t* currentTime)
{
    FUNC_TR;
    return time(currentTime);
}

double get_difftime(time_t stopTime, time_t startTime)
{
    FUNC_TR;
    return difftime(stopTime, startTime);
}

struct tm* get_gmtime(time_t* currentTime)
{
    struct tm *result;
    FUNC_TR;
    return gmtime_r(currentTime, result);
}

char* get_ctime(time_t* timeToGet)
{
    FUNC_TR;
    return ctime(timeToGet);
}

