#ifndef FUNCTIONLOGEVAL_H_
#define FUNCTIONLOGEVAL_H_

class FunctionLog;
#include <string>
#include <sstream>
#include <iomanip>

#include <limits>

#include <memory>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <functional>

#include <cassert>

using FunctionLogEvalCallback = std::function<void()>;

class FunctionLogEval
{
public:
	FunctionLogEval(const char* function);
	virtual ~FunctionLogEval();

	void setPattern(const char* pattern);
	void wait();

	std::string getFunction();
	size_t getCount();

	template<typename T> bool getResult(size_t idx, T& value)
	{
		typedef std::numeric_limits<T> numlimit;

		assert(idx < result_.size() );

		std::istringstream ss(result_[idx]);
		ss >> value;

		if (ss.fail())
		{
			if (numlimit::is_signed)
			{
				value = (numlimit::min)();
			}
			else
			{
				value = (numlimit::max)();
			}

			return false;
		}
		else
		{
			return true;
		}
	}

	template<class T> bool getResult(size_t idx, T*& value)
	{
		assert(idx < result_.size());

		long ret = 0;
		std::stringstream ss;
		ss << std::setfill('0') << std::hex << result_[idx];
		ss >> ret;
		value = reinterpret_cast<T*>(ret);

		if (ss.fail())
		{
			ret = (std::numeric_limits<long>::max)();
			value = reinterpret_cast<T*>(ret);
			return false;
		}
		else
		{
			return true;
		}
	}

	const std::vector<std::string>& getResultList();

	virtual bool IsProcess( FunctionLog& log );
	virtual void Process( FunctionLog& log );

	void setCallback(FunctionLogEvalCallback func);

protected:
	void  notify(bool& need_wait);
	void  run_callback();
	void  wait_callback();

private:
	void operator =(const FunctionLogEval& src) {}
	FunctionLogEval(const FunctionLogEval& src) {}
	
private:
	std::string function_;
	std::string pattern_;
	std::vector<std::string> result_;
	size_t   count_;
	size_t   notify_;
	std::recursive_mutex mtx_;
 	std::condition_variable_any cond_;

	std::recursive_mutex wait_mtx_;
	std::condition_variable_any wait_cond_;
	size_t   wait_notify_;
	FunctionLogEvalCallback check_function_;
};

template<>
bool FunctionLogEval::getResult(size_t idx, std::string& value);

typedef std::shared_ptr<FunctionLogEval> FunctionLogEvalPtr;


#endif