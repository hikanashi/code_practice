#ifndef LOGOUTWRAPPER_H_
#define LOGOUTWRAPPER_H_

#include <stdint.h>

#define RECORD_FUNCTION(msg, ...) LogoutFunction(__func__, __LINE__, msg, ##__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

void LogoutFunction(
		const char* function,
		uint64_t line,
		const char* format,
		...);

void ResetFunctionLog();

#ifdef __cplusplus
}
#endif

#endif
