#ifndef FUNCTIONLOG_H_
#define FUNCTIONLOG_H_

#include <memory>
#include <string>
#include <stdint.h>
#include <vector>


class FunctionLog
{
public:
	FunctionLog(const char* function, uint64_t line, const char* log);

	virtual ~FunctionLog();

	const std::string& getFunction();
	const uint64_t getLine();
	const std::string& getLog();

	bool parseLog(
		const char* pattern,
		std::vector<std::string>& result);

private:
	std::string function_;
	uint64_t	line_;
	std::string log_;
};

typedef std::shared_ptr<FunctionLog> FunctionLogPtr;


#endif