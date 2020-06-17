#ifndef FUNCTIONLOGGER_H_
#define FUNCTIONLOGGER_H_

#include <memory>
#include <string>
#include <map>
#include <mutex>
#include "FunctionLogList.h"

#define RECORD_FUNCTION(msg, ...) LogoutFunction(__func__, __LINE__, msg, ##__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

void LogoutFunction(
		const char* function,
		uint64_t line,
		const char* format,
		...);

void ResetFunctionLog();

#ifdef __cplusplus
}
#endif

class FunctionLogger
{
public:
	static FunctionLogger& GetLogger() { return logger_; }

	void logout(
		const char* function,
		uint64_t line,
		const char* log);
		
	static
	FunctionLogEvalPtr addPattern(
			const char* function,
			const char* pattern);

	void addEval(
		FunctionLogEvalPtr eval);

	void delEval(
		FunctionLogEvalPtr eval);

	void reset();


protected:
	FunctionLogger();
	virtual ~FunctionLogger();

private:
	void operator =(const FunctionLogger& src) {}
	FunctionLogger(const FunctionLogger& src) {}

private:
	std::map <std::string, FunctionLogListPtr> functions_;
	std::mutex	mutex_;

	static FunctionLogger logger_;
};




#endif
