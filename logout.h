#pragma once

#include "loggerwrapper.h"

#define LOGOUT(...)								\
	do {										\
		RECORD_FUNCTION(__VA_ARGS__);			\
		char logbuf[1024] = {0};				\
		snprintf(logbuf, sizeof(logbuf), __VA_ARGS__);	\
		logout(logbuf);							\
	} while (0);

#define FUNC_TO_STR2(x) #x
#define FUNC_TO_STR(x) FUNC_TO_STR2(x)

#define LOGOUT_APIIN(...)	LOGOUT("[curl_test][API IN]"  __VA_ARGS__)
#define LOGOUT_APIOUT(...)	LOGOUT("[curl_test][API OUT]" __VA_ARGS__)


#ifdef __cplusplus
extern "C" {
#endif

void logout(const char* logstr);

#ifdef __cplusplus
}
#endif
