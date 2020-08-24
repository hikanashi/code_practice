#include "FunctionLogger.h"
#include <cstdarg>
#include "loggerwrapper.h"

void LogoutFunction(
	const char* function,
	uint64_t line,
	const char* format,
	...)
{
	char tmpbuf[10240] = { 0 };

	va_list args;

	va_start(args, format);
	vsnprintf(tmpbuf, sizeof(tmpbuf) - 1, format, args);
	va_end(args);

	FunctionLogger::GetLogger().logout(function, line, tmpbuf);

	return;
}

void ResetFunctionLog()
{
	FunctionLogger::GetLogger().reset();
	return;
}
