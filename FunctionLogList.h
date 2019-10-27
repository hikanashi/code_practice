#ifndef FUNCTIONLOGLIST_H_
#define FUNCTIONLOGLIST_H_

#include <memory>
#include <vector>
#include "FunctionLog.h"

class FunctionLogList;
typedef std::shared_ptr<FunctionLogList> FunctionLogListPtr;

class FunctionLogList
{
public:
	FunctionLogList(const char*  function);

	virtual ~FunctionLogList();

	void addFunctionLog(
		const char* function,
		uint64_t line,
		const char* log);

	const FunctionLogPtr getLog(
							size_t index);

	FunctionLogListPtr searchLog(
						const char* pattern);

	size_t size();

	const std::string& getFunction();

private:
	std::string function_;
	std::vector<FunctionLogPtr> list_;
};

typedef std::shared_ptr<FunctionLogList> FunctionLogListPtr;

#endif