#include "FunctionLog.h"
#include <regex>

FunctionLog::FunctionLog(const char* function, uint64_t line, const char* log)
	: function_(function)
	, line_(line)
	, log_(log)
{

}

FunctionLog::~FunctionLog()
{
}

const std::string& FunctionLog::getFunction()
{ 
	return function_;
}

const uint64_t FunctionLog::getLine() 
{
	return line_;
}

const std::string& FunctionLog::getLog()
{
	return log_; 
}

bool FunctionLog::parseLog(
	const char* pattern,
	std::vector<std::string>& result)
{
	if (pattern == NULL)
	{
		return false;
	}

	std::regex re(pattern);
	std::smatch match;

	if (std::regex_search(log_, match, re) == false)
	{
		return false;
	}

	result.clear();
	// skip all submatch string. so start index=1
	for (size_t idx = 1; idx != match.size(); idx++) 
	{
		result.push_back(match[idx].str());
	}

	return true;
}