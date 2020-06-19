#ifndef FUNCTIONLOGEVAL_H_
#define FUNCTIONLOGEVAL_H_

#include <memory>
#include <vector>
#include <condition_variable>
#include <mutex>

class FunctionLog;
#include <string>


class FunctionLogEval
{
public:
	FunctionLogEval(const char* function);
	virtual ~FunctionLogEval();

	void setPattern(const char* pattern);
	void wait();

	std::string getFunction();
	size_t getCount();
	const std::string getResult(size_t idx);
	const std::vector<std::string>& getResultList();

	virtual bool IsProcess( FunctionLog& log );
	virtual void Process( FunctionLog& log );

private:
	void operator =(const FunctionLogEval& src) {}
	FunctionLogEval(const FunctionLogEval& src) {}
	
private:
	std::string function_;
	std::string pattern_;
	std::vector<std::string> result_;
	size_t   count_;
	size_t   notify_;
	std::mutex mtx_;
 	std::condition_variable cond_;
};

typedef std::shared_ptr<FunctionLogEval> FunctionLogEvalPtr;


#endif