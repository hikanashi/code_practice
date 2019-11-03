#ifndef FUNCTIONLOGGER_H_
#define FUNCTIONLOGGER_H_

#include <memory>
#include <string>
#include <map>
#include <mutex>
#include "FunctionLogList.h"

#define RECORD_FUNCTION(...) RecordFunctionLog(__func__, __LINE__, __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

void RecordFunctionLog(
		const char* function,
		uint64_t line,
		const char* format,
		...);


void ResetFunctionLog();

size_t CountFunctionLog(
	const char* function);

#ifdef __cplusplus
}
#endif

class FunctionLogger
{
public:
	static FunctionLogger& GetLogger() { return logger_; }

	void addFunction(
		const char* function,
		uint64_t line,
		const char* log);

	void reset();

	FunctionLogListPtr  GetLogList(const char* function,
									const char* pattern = NULL);

	size_t              CountAPILog(const char* function);

protected:
	FunctionLogger();
	virtual ~FunctionLogger();

private:
	void operator =(const FunctionLogger& src) {}
	FunctionLogger(const FunctionLogger& src) {}

private:
	std::map <std::string, FunctionLogListPtr> functions_;
	bool record_;
	static FunctionLogger logger_;
	std::mutex	mutex_;
};




#endif
