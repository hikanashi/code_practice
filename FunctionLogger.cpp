#include "FunctionLogger.h"
#include <cstdarg>

FunctionLogger FunctionLogger::logger_;


void RecordFunctionLog(
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

	FunctionLogger::GetLogger().addFunction(function, line, tmpbuf);

	return;
}

void ResetFunctionLog()
{
	FunctionLogger::GetLogger().reset();
	return;
}

size_t CountFunctionLog(
	const char* function)
{
	return 	FunctionLogger::GetLogger().CountAPILog(function);
}


FunctionLogger::FunctionLogger()
	:  functions_()
	, record_(true)
	, mutex_()
{
}

FunctionLogger::~FunctionLogger()
{
}

void FunctionLogger::addFunction(
	const char* function,
	uint64_t line,
	const char* log)
{
	std::lock_guard<std::mutex> lock(mutex_);
	std::string func(function);

	FunctionLogListPtr& functionlog = functions_[func];

	if (!functionlog)
	{
		functionlog = FunctionLogListPtr(new FunctionLogList(function));
	}

	functionlog->addFunctionLog(function, line, log);
}

FunctionLogListPtr FunctionLogger::GetLogList(
	const char* function,
	const char* pattern)
{
	std::string func(function);
	FunctionLogListPtr list = functions_[func];

	if (!list)
	{
		list = FunctionLogListPtr(new FunctionLogList(function));
		return list;
	}

	if (pattern == NULL)
	{
		return list;
	}
	else
	{
		return list->searchLog(pattern);

	}
}

size_t FunctionLogger::CountAPILog(const char* function)
{
	std::string func(function);
	FunctionLogListPtr list = functions_[func];

	if (!list)
	{
		return 0;
	}

	FunctionLogListPtr apilog = list->searchLog("\\[API_IN\\]");

	return apilog->size();
}


void FunctionLogger::reset()
{
	return functions_.clear();
}
