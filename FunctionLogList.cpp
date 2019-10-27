#include "FunctionLogList.h"
#include <regex>

FunctionLogList::FunctionLogList(const char*  function)
	: function_(function)
	, list_()
{
}

FunctionLogList::~FunctionLogList()
{
}

void FunctionLogList::addFunctionLog(
	const char* function,
	uint64_t line,
	const char* log)
{

	FunctionLogPtr functionlog =
		FunctionLogPtr(
			new FunctionLog(function, line, log));

	list_.push_back(functionlog);

}

const std::string&  FunctionLogList::getFunction()
{ 
	return function_;
}

const FunctionLogPtr  FunctionLogList::getLog(size_t index) 
{ 
	FunctionLogPtr ret;

	if (index >= list_.size())
	{
		return ret;
	}

	return list_[index];
}
size_t FunctionLogList::size()
{ 
	return list_.size(); 
}

FunctionLogListPtr FunctionLogList::searchLog(
	const char* pattern)
{
	FunctionLogListPtr ret = 
				FunctionLogListPtr(new FunctionLogList(function_.c_str()));

	std::regex re(pattern);
	std::smatch match;

	for (FunctionLogPtr funclog : list_)
	{
		const std::string& log = funclog->getLog();
		if (std::regex_search(log, match, re) != false)
		{ 
			ret->list_.push_back(funclog);

		}
	}


	return ret;
}

