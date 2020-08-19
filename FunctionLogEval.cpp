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
	
	notify();
	wait_callback();
}


void FunctionLogEval::notify()
{
	std::lock_guard<std::mutex> lk(mtx_);
	notify_++;
	cond_.notify_all();
}


void FunctionLogEval::wait(
	FunctionLogEvalCallback func)
{
	std::unique_lock<std::mutex> lk(mtx_);
	check_function_ = func;

	cond_.wait(lk, [&]{ return ( notify_ > 0 ); });
	notify_--;

	run_callback();

	check_function_ = nullptr;
}

void FunctionLogEval::wait_callback()
{
	if (check_function_ == nullptr)
	{
		return;
	}

	std::unique_lock<std::mutex> waitlk(wait_mtx_);
	wait_cond_.wait(waitlk, [&] { return (wait_notify_ > 0); });
	wait_notify_--;
}

void FunctionLogEval::run_callback()
{
	if (check_function_ == nullptr)
	{
		return;
	}

	std::lock_guard<std::mutex> waitlk(wait_mtx_);

	check_function_();
	wait_notify_++;
	wait_cond_.notify_all();

}
