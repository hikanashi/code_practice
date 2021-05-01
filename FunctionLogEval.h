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

#include <cstdint>

class FunctionLogEval;
using FunctionLogEvalCallback = std::function<void()>;
using FunctionTimeoutCallback = std::function<void(FunctionLogEval&)>;
using FunctionFormatCallback = std::function<void(std::stringstream&)>;

class FunctionLogEval
{
public:
	FunctionLogEval(const char* function);
	virtual ~FunctionLogEval();

	void setPattern(const char* pattern);
	void wait();

	std::string getFunction();
	std::string getPattern();
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
		return getResultByFormat(idx, value);
	}

	template<typename T>
	bool getResultHex(size_t idx, T& value)
	{
		FunctionFormatCallback settingformat = [](std::stringstream& sstream)
		{
			sstream << std::setfill('0') << std::hex;

		};

		return getResultByFormat(idx, value, settingformat);
	}

	template<typename T> 
	bool getResult(size_t idx, T*& value)
	{
		unsigned long retValue = 0;
		bool retResult = false;
		FunctionFormatCallback settingformat = [](std::stringstream& sstream)
		{
			sstream << std::setfill('0') << std::hex;

		};

		retResult = getResultByFormat(idx, retValue, settingformat);

		value = reinterpret_cast<T*>(retValue);
		return retResult;
	}

	template<typename T>
	bool getResultByFormat(size_t idx, T& value, FunctionFormatCallback settingstream = nullptr)
	{
		std::string result;
		bool exist_result = getResult(idx, result);
		if (exist_result == false)
		{
			getErrorValue(value);
			return false;
		}

		std::stringstream ss;
		if (settingstream)
		{
			settingstream(ss);
		}

		ss << result;
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

	const std::vector<std::string> getResultList();

	virtual bool IsProcess( 
					FunctionLog& log, 
					std::vector<std::string>& result);
	virtual void Process( 
					FunctionLog& log,
					std::vector<std::string>& result);

	void setCallback(FunctionLogEvalCallback func);


	void setWaitLogTimeoutCallback(
		FunctionTimeoutCallback func);

	void setWaitLogTimeout(
		int second);


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


	int64_t   notify_;
	std::recursive_mutex notify_mtx_;
 	std::condition_variable_any notify_cond_;


	bool running_callback_;
	std::recursive_mutex running_mtx_;

	std::recursive_mutex callback_notify_mtx_;
	std::condition_variable_any callback_notify_cond_;
	int64_t   callback_notify_;


	FunctionLogEvalCallback callback_function_;
	std::recursive_mutex callback_function_mtx_;

	int					    waitlog_timeout_sec_;
	FunctionTimeoutCallback waitlog_timeout_callback_;
	std::recursive_mutex	waitlog_timeout_mutex_;
};

template<>
bool FunctionLogEval::getResult<std::string>(size_t idx, std::string& value);

typedef std::shared_ptr<FunctionLogEval> FunctionLogEvalPtr;


#endif