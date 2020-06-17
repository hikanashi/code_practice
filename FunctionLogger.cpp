#include "FunctionLogger.h"
#include <cstdarg>

FunctionLogger FunctionLogger::logger_;


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



FunctionLogger::FunctionLogger()
	:  functions_()
	, mutex_()
{
}

FunctionLogger::~FunctionLogger()
{
}

void FunctionLogger::logout(
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

	functionlog->logout(function, line, log);
}

FunctionLogEvalPtr FunctionLogger::addPattern(
						const char* function,
						const char* pattern)
{
	FunctionLogEvalPtr eval;
	
	if(function == NULL)
	{
		return eval;
	}
	
	eval = FunctionLogEvalPtr(new FunctionLogEval(function));
	eval->setPattern(pattern);
	
	FunctionLogger::GetLogger().addEval(eval);
	
	return eval;
}

void FunctionLogger::addEval(
		FunctionLogEvalPtr eval)
{
	if(!eval)
	{
		return;
	}

	
	std::string func = eval->getFunction();
	
	std::lock_guard<std::mutex> lock(mutex_);	
	FunctionLogListPtr& functionlog = functions_[func];

	if (!functionlog)
	{
		functionlog = FunctionLogListPtr(new FunctionLogList(func.c_str()));
	}
	
	functionlog->addFunctionEval(eval);
}


void FunctionLogger::delEval(
		FunctionLogEvalPtr eval)
{
	if(!eval)
	{
		return;
	}
	
	std::string func = eval->getFunction();
	
	std::lock_guard<std::mutex> lock(mutex_);	

	FunctionLogListPtr& functionlog = functions_[func];

	if (!functionlog)
	{
		return;
	}

	functionlog->delFunctionEval(eval);
}

void FunctionLogger::reset()
{
	return functions_.clear();
}
