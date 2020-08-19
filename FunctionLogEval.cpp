#include "FunctionLogEval.h"
#include "FunctionLog.h"
#include "FunctionLogger.h"

FunctionLogEval::FunctionLogEval(const char* function)
	: function_(function)
	, pattern_()
	, result_()
	, count_(0)
	, notify_(0)
	, mtx_()
	, cond_()
	, wait_mtx_()
	, wait_cond_()
	, wait_notify_(0)
	, check_function_()
{
}


FunctionLogEval::~FunctionLogEval()
{
}

std::string FunctionLogEval::getFunction()
{
	return function_;
}

size_t FunctionLogEval::getCount()
{
	return count_;
}

template<> 
bool FunctionLogEval::getResult(size_t idx, std::string& value)
{
	assert(idx < result_.size());

	value = result_[idx];
	return true;
}

const std::vector<std::string>& FunctionLogEval::getResultList()
{
	return result_;
}

void FunctionLogEval::setPattern(const char* pattern)
{
	if (pattern != nullptr)
	{
		pattern_ = pattern;
	}
}

bool FunctionLogEval::IsProcess( FunctionLog& log )
{ 
	if (pattern_.length() == 0)
	{
		return true;
	}

	bool match =  log.parseLog(pattern_.c_str(), result_);
	
	return match;
}

void FunctionLogEval::Process( FunctionLog& log )
{
	count_++;
	
	bool need_wait = false;

	notify(need_wait);
	
	if( need_wait != false)
	{
		wait_callback();
	}
}


void FunctionLogEval::notify(bool& need_wait)
{
	std::lock_guard<std::recursive_mutex> lk(mtx_);


	if (check_function_ != nullptr)
	{
		need_wait = true;
	}

	notify_++;
	cond_.notify_all();
}


void FunctionLogEval::wait()
{
	std::unique_lock<std::recursive_mutex> lk(mtx_);
	cond_.wait(lk, [&]{ return ( notify_ > 0 ); });
	notify_--;

	run_callback();
}

void FunctionLogEval::setCallback(FunctionLogEvalCallback func)
{
	std::unique_lock<std::recursive_mutex> lk(mtx_);
	check_function_ = func;
}

void FunctionLogEval::wait_callback()
{
	std::unique_lock<std::recursive_mutex> waitlk(wait_mtx_);
	wait_cond_.wait(waitlk, [&] { return (wait_notify_ > 0); });
	wait_notify_--;
}

void FunctionLogEval::run_callback()
{
	std::unique_lock<std::recursive_mutex> lk(mtx_);
	if (check_function_ == nullptr)
	{
		return;
	}

	std::lock_guard<std::recursive_mutex> waitlk(wait_mtx_);
	
	FunctionLogEvalCallback func = check_function_;
	check_function_ = nullptr;
	func();

	wait_notify_++;
	wait_cond_.notify_all();

}
