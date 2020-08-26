#include "FunctionLogList.h"
#include "FunctionLog.h"
#include <algorithm>

FunctionLogList::FunctionLogList(const char*  function)
	: function_(function)
	, evals_()
{
}

FunctionLogList::~FunctionLogList()
{
}

void FunctionLogList::logout(
	const char* function,
	uint64_t line,
	const char* log)
{

	FunctionLog functionlog(function, line, log);

	std::vector<FunctionLogEvalPtr> proclist;

	{
		std::lock_guard<std::recursive_mutex> lock(mutex_);
		proclist = evals_;
	}


	for (FunctionLogEvalPtr& eval : proclist)
	{
		if(eval)
		{
			if( eval->IsProcess(functionlog) != false )
			{
				eval->Process(functionlog);
			}
		}
	}

}

void FunctionLogList::addFunctionEval(
		FunctionLogEvalPtr eval)
{
	std::lock_guard<std::recursive_mutex> lock(mutex_);
	evals_.push_back(eval);

}

void FunctionLogList::delFunctionEval(
		FunctionLogEvalPtr eval)
{
	std::lock_guard<std::recursive_mutex> lock(mutex_);

	auto itr = evals_.begin();
	while (itr != evals_.end())
	{
		if((*itr) == eval)
		{
			itr = evals_.erase(itr);
			break;
		}
		else
		{
			itr++;
		}
	}
}

const std::string&  FunctionLogList::getFunction()
{ 
	return function_;
}

