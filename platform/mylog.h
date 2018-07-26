
#ifndef _MYLOG_H
#define _MYLOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#if 0
/**
 * @brief Debug level trace logging macro.
 *
 * Macro to print message function entry and exit
 */
#define FUNC_ENTRY    \
	{\
	printf("FUNC_ENTRY: %s L#%d (%s)\n", __func__, __LINE__, __FILE__);  \
	}

#define FUNC_EXIT    \
	{\
	printf("FUNC_EXIT:  %s L#%d (%s)\n", __func__, __LINE__, __FILE__);  \
	}

#define FUNC_TR    \
	{\
	printf("FUNC_TR:  %s L#%d (%s)\r", __func__, __LINE__, __FILE__); \
	}

#define FUNC_EXIT_RC(x)    \
	{\
	printf("FUNC_EXIT:  %s L#%d Return Code : %d (%s)\n", __func__, __LINE__, x, __FILE__);  \
	return x; \
	}

#define DBG_INFO(...)    \
	{\
	printf("INFO:  %s L#%d ", __func__, __LINE__);  \
	printf(__VA_ARGS__); \
	printf("\n"); \
	}
#else
#define FUNC_ENTRY    {}
#define FUNC_EXIT    {}
#define FUNC_TR    {}
#define DBG_INFO(...)    {}

#endif

#ifdef __cplusplus
}
#endif

#endif // _MYLOG_H


