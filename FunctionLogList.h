#ifndef FUNCTIONLOGLIST_H_
#define FUNCTIONLOGLIST_H_

#include <memory>
#include <vector>
#include "FunctionLogEval.h"

// class FunctionLogList;
// typedef std::shared_ptr<FunctionLogList> FunctionLogListPtr;

class FunctionLogList
{
public:
	FunctionLogList(const char*  function);

	virtual ~FunctionLogList();

	void logout(
		const char* function,
		uint64_t line,
		const char* log);
		
	void addFunctionEval(
		FunctionLogEvalPtr eval);

	void delFunctionEval(
		FunctionLogEvalPtr eval);

	const std::string& getFunction();

private:
	std::string function_;
	std::vector<FunctionLogEvalPtr> evals_;
};

typedef std::shared_ptr<FunctionLogList> FunctionLogListPtr;

#endif