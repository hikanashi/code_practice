#ifndef FUNCTIONLOGGER_H_
#define FUNCTIONLOGGER_H_

#include <memory>
#include <string>
#include <map>
#include <mutex>
#include "FunctionLogList.h"
#include "FunctionLogEval.h"

class FunctionLogger
{
public:
	static FunctionLogger& GetLogger() { return logger_; }

	void logout(
		const char* function,
		uint64_t line,
		const char* log);

	static
	FunctionLogEvalPtr	addAPIIN(
			const char* function,
			const char* append_pattern = nullptr);

	static
	FunctionLogEvalPtr	addAPIOUT(
			const char* function,
			const char* append_pattern = nullptr);

	static
	FunctionLogEvalPtr addPattern(
			const char* function,
			const char* pattern);

	void addEval(
		FunctionLogEvalPtr eval);

	void delEval(
		FunctionLogEvalPtr eval);

	void reset();

	void setDefaultWaitLogTimeoutCallback(
		FunctionTimeoutCallback func);

	void setDefaultWaitLogTimeout(
		int second);

protected:
	FunctionLogger();
	virtual ~FunctionLogger();

private:
	void operator =(const FunctionLogger& src) {}
	FunctionLogger(const FunctionLogger& src) {}

private:
	std::map <std::string, FunctionLogListPtr> functions_;
	std::recursive_mutex	functions_mutex_;

	int					    default_waitlog_timeout_sec_;
	FunctionTimeoutCallback default_waitlog_timeout_callback_;
	std::recursive_mutex	waitlog_timeout_mutex_;

	static FunctionLogger logger_;
};




#endif
