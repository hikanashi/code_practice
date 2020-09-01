#include "FunctionLogger.h"


FunctionLogger FunctionLogger::logger_;

FunctionLogger::FunctionLogger()
	:  functions_()
	, functions_mutex_()
	, default_waitlog_timeout_sec_(0)
	, default_waitlog_timeout_callback_()
	, waitlog_timeout_mutex_()
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
	size_t count_func = 0;

	{
		std::lock_guard<std::recursive_mutex > lock(functions_mutex_);
		count_func = functions_.size();
	}

	if (0 == count_func)
	{
		return;
	}

	FunctionLogListPtr functionlog;

	{
		std::lock_guard<std::recursive_mutex > lock(functions_mutex_);
		std::string func(function);

		FunctionLogListPtr& fnclog = functions_[func];

		if (!fnclog)
		{
			fnclog = FunctionLogListPtr(new FunctionLogList(function));
		}

		functionlog = fnclog;
	}

	functionlog->logout(function, line, log);

}

FunctionLogEvalPtr FunctionLogger::addAPIIN(
	const char* function,
	const char* append_pattern)
{
	std::string pattern = "\\[API IN\\]";
	if (append_pattern != nullptr)
	{
		pattern += append_pattern;
	}

	return FunctionLogger::addPattern(function, pattern.c_str());
}

FunctionLogEvalPtr FunctionLogger::addAPIOUT(
	const char* function,
	const char* append_pattern)
{
	std::string pattern = "\\[API OUT\\]";
	if (append_pattern != nullptr)
	{
		pattern += append_pattern;
	}

	return FunctionLogger::addPattern(function, pattern.c_str());
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

	{
		std::lock_guard<std::recursive_mutex> lock(waitlog_timeout_mutex_);
		eval->setWaitLogTimeoutCallback(default_waitlog_timeout_callback_);
		eval->setWaitLogTimeout(default_waitlog_timeout_sec_);
	}
	
	std::string func = eval->getFunction();
	
	std::lock_guard<std::recursive_mutex> lock(functions_mutex_);
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
	
	std::lock_guard<std::recursive_mutex> lock(functions_mutex_);

	FunctionLogListPtr& functionlog = functions_[func];

	if (!functionlog)
	{
		return;
	}

	functionlog->delFunctionEval(eval);
}

void FunctionLogger::reset()
{
	std::lock_guard<std::recursive_mutex> lock(functions_mutex_);
	return functions_.clear();
}

void FunctionLogger::setDefaultWaitLogTimeoutCallback(
	FunctionTimeoutCallback func)
{
	std::lock_guard<std::recursive_mutex> lock(waitlog_timeout_mutex_);
	default_waitlog_timeout_callback_ = func;
}

void FunctionLogger::setDefaultWaitLogTimeout(
	int second)
{
	std::lock_guard<std::recursive_mutex> lock(waitlog_timeout_mutex_);
	default_waitlog_timeout_sec_ = second;
}
