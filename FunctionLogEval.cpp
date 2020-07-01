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
}


void FunctionLogEval::notify()
{
	std::lock_guard<std::mutex> lk(mtx_);
	notify_++;
	cond_.notify_all();
}


void FunctionLogEval::wait()
{
	std::unique_lock<std::mutex> lk(mtx_);
	cond_.wait(lk, [&]{ return ( notify_ > 0 ); });
	notify_--;
}
