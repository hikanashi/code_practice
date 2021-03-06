#include "FunctionLogEval.h"
#include "FunctionLog.h"
#include "FunctionLogger.h"

FunctionLogEval::FunctionLogEval(const char* function)
	: function_(function)
	, pattern_()
	, result_()
	, count_(0)
	, count_mtx_()
	, notify_(0)
	, notify_mtx_()
	, notify_cond_()
	, running_callback_(false)
	, running_mtx_()
	, callback_notify_mtx_()
	, callback_notify_cond_()
	, callback_notify_(0)
	, callback_function_()
	, callback_function_mtx_()
	, waitlog_timeout_sec_(0)
	, waitlog_timeout_callback_()
	, waitlog_timeout_mutex_()
{
}


FunctionLogEval::~FunctionLogEval()
{
}

std::string FunctionLogEval::getFunction()
{
	return function_;
}

std::string FunctionLogEval::getPattern()
{
	return pattern_;
}

size_t FunctionLogEval::getCount()
{
	std::lock_guard<std::recursive_mutex> lk(count_mtx_);
	return count_;
}

template<> 
bool FunctionLogEval::getResult<std::string>(size_t idx, std::string& value)
{
	std::lock_guard<std::recursive_mutex> lk(count_mtx_);

	if (idx >= result_.size())
	{
		value.clear();
		return false;
	}

	value = result_[idx];
	return true;
}

const std::vector<std::string> FunctionLogEval::getResultList()
{
	std::lock_guard<std::recursive_mutex> lk(count_mtx_);
	return result_;
}

void FunctionLogEval::setPattern(const char* pattern)
{
	if (pattern != nullptr)
	{
		pattern_ = pattern;
	}
}

bool FunctionLogEval::IsProcess( FunctionLog& log, std::vector<std::string>& result)
{ 
	{
		std::lock_guard<std::recursive_mutex> runlk(running_mtx_);
		if (running_callback_ != false)
		{
			return false;
		}
	}

	if (pattern_.length() == 0)
	{
		return true;
	}

	bool match =  log.parseLog(pattern_.c_str(), result);
	
	return match;
}

void FunctionLogEval::Process( FunctionLog& log, std::vector<std::string>& result)
{
	{
		std::lock_guard<std::recursive_mutex> lk(count_mtx_);
		count_++;
		result_ = result;
	}

	notify();
	
	wait_callback();

}


void FunctionLogEval::notify()
{
	{
		std::lock_guard<std::recursive_mutex> lk(notify_mtx_);
		notify_++;
		notify_cond_.notify_all();
	}
}


void FunctionLogEval::wait()
{
	bool isTimeout = false;

	{
		int timeout_sec = 0;
		FunctionTimeoutCallback timeoutfunc;

		{
			std::lock_guard<std::recursive_mutex> lock(waitlog_timeout_mutex_);
			timeout_sec = waitlog_timeout_sec_;
			timeoutfunc = waitlog_timeout_callback_;
		}

		std::unique_lock<std::recursive_mutex> lk(notify_mtx_);
		if (timeout_sec > 0)
		{
			const auto timeout = std::chrono::seconds(timeout_sec);
			notify_cond_.wait_for(lk, timeout, [&] { return (notify_ > 0); });
			// no notify is timeout
			if (notify_ <= 0)
			{
				isTimeout = true;
				if (timeoutfunc)
				{
					timeoutfunc(*this);
				}
			}
		}
		else
		{
			notify_cond_.wait(lk, [&] { return (notify_ > 0); });
		}

		if (isTimeout == false)
		{
			notify_--;
		}
	}

	if(isTimeout)
	{
		std::lock_guard<std::recursive_mutex> lk(callback_function_mtx_);
		callback_function_ = nullptr;
	}


	run_callback();
}

void FunctionLogEval::setCallback(FunctionLogEvalCallback func)
{
	std::lock_guard<std::recursive_mutex> lk(callback_function_mtx_);
	callback_function_ = func;
}

void FunctionLogEval::wait_callback()
{
	{
		std::lock_guard<std::recursive_mutex> lk(callback_function_mtx_);
		if (!callback_function_)
		{
			return;
		}
	}

	{
		std::unique_lock<std::recursive_mutex> waitlk(callback_notify_mtx_);
		callback_notify_cond_.wait(waitlk, [&] { return (callback_notify_ > 0); });
		callback_notify_--;
	}
}

void FunctionLogEval::run_callback()
{
	{
		std::lock_guard<std::recursive_mutex> runlk(running_mtx_);
		running_callback_ = true;
	}

	{
		FunctionLogEvalCallback func;
		{
			std::lock_guard<std::recursive_mutex> lk(callback_function_mtx_);
			func = callback_function_;
			callback_function_ = nullptr;
		}

		if (func)
		{
			func();
		}
	}

	{
		std::lock_guard<std::recursive_mutex> runlk(running_mtx_);
		running_callback_ = false;
	}

	{
		std::lock_guard<std::recursive_mutex> waitlk(callback_notify_mtx_);
		callback_notify_++;
		callback_notify_cond_.notify_all();
	}
}

void FunctionLogEval::setWaitLogTimeoutCallback(
	FunctionTimeoutCallback func)
{
	std::lock_guard<std::recursive_mutex> lock(waitlog_timeout_mutex_);
	waitlog_timeout_callback_ = func;
}

void FunctionLogEval::setWaitLogTimeout(
	int second)
{
	std::lock_guard<std::recursive_mutex> lock(waitlog_timeout_mutex_);
	waitlog_timeout_sec_ = second;
}
