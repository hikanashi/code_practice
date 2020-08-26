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

	template<typename T>
	void getErrorValue(T& value)
	{
		typedef std::numeric_limits<T> numlimit;
		if (numlimit::is_signed)
		{
			value = (numlimit::min)();
		}
		else
		{
			value = (numlimit::max)();
		}
	}

	template<typename T>
	bool getResult(size_t idx, T& value)
	{
		typedef std::numeric_limits<T> numlimit;
		
		std::string result;
		bool exist_result = getResult(idx, result);
		if( exist_result == false )
		{
			getErrorValue(value);
			return false;
		}

		std::istringstream ss(result);
		ss >> value;

		if (ss.fail())
		{
			getErrorValue(value);
			return false;
		}
		else
		{
			return true;
		}
	}

	template<typename T> 
	bool getResult(size_t idx, T*& value)
	{
		unsigned long ret = 0;

		std::string result;
		bool exist_result = getResult(idx, result);
		if (exist_result == false)
		{
			getErrorValue(ret);
			value = reinterpret_cast<T*>(ret);
			return false;
		}

		std::stringstream ss;
		ss << std::setfill('0') << std::hex << result;
		ss >> ret;
		value = reinterpret_cast<T*>(ret);

		if (ss.fail())
		{
			getErrorValue(ret);
			value = reinterpret_cast<T*>(ret);
			return false;
		}
		else
		{
			return true;
		}
	}

	const std::vector<std::string> getResultList();

	virtual bool IsProcess( FunctionLog& log );
	virtual void Process( FunctionLog& log );

	void setCallback(FunctionLogEvalCallback func);

protected:
	void  notify();
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
	std::recursive_mutex count_mtx_;


	size_t   notify_;
	std::recursive_mutex notify_mtx_;
 	std::condition_variable_any notify_cond_;


	bool running_callback_;
	std::recursive_mutex running_mtx_;

	std::recursive_mutex callback_notify_mtx_;
	std::condition_variable_any callback_notify_cond_;
	size_t   callback_notify_;


	FunctionLogEvalCallback callback_function_;
	std::recursive_mutex callback_function_mtx_;
};

template<>
bool FunctionLogEval::getResult<std::string>(size_t idx, std::string& value);

typedef std::shared_ptr<FunctionLogEval> FunctionLogEvalPtr;


#endif