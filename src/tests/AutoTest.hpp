#pragma once

#include <memory>
#include <vector>
#include <iostream>

#define EXPR_TO_STR(x) #x

#define TEST_BEGIN	Test::TestSuiteResult __result__;
#define TEST_END	return __result__;

#define ADD_LOG(expr, result) __result__.addLog(EXPR_TO_STR(expr), result, __func__, __FILE__, __LINE__);

#define INCR_SUCCESS(expr) ++(__result__.nbSuccess); ADD_LOG(expr, TestResult::SUCCESS)
#define INCR_WARNING(expr) ++(__result__.nbWarning); ADD_LOG(expr, TestResult::WARNING)
#define INCR_ERROR(expr) ++(__result__.nbError); ADD_LOG(expr, TestResult::ERROR)
#define INCR_CRITICAL_ERROR(expr) ++(__result__.nbCriticalError); ADD_LOG(expr, TestResult::CRITICAL_ERROR) return __result__;

#define TEST_WARNING(expr) if (expr) { INCR_SUCCESS(expr)  } else  { INCR_WARNING(expr) }
#define TEST_ERROR(expr) if (expr) { INCR_SUCCESS(expr) } else { INCR_ERROR(expr) }
#define TEST_CRITICAL_ERROR(expr) if (expr) { INCR_SUCCESS(expr) } else { INCR_CRITICAL_ERROR(expr) }

namespace Test
{

enum	TestResult
{
	SUCCESS,
	WARNING,
	ERROR,
	CRITICAL_ERROR
};

inline std::ostream&	operator<<(std::ostream& os, TestResult r)
{
	switch (r)
	{
	case TestResult::SUCCESS:
		os << "SUCCESS";
		break;
	case TestResult::WARNING:
		os << "WARNING";
		break;
	case TestResult::ERROR:
		os << "ERROR";
		break;
	case TestResult::CRITICAL_ERROR:
		os << "CRITICAL_ERROR";
		break;	
	}
	return os;
}

struct	TestLog
{
	std::string	expression;
	TestResult	resultCode;
	std::string	functionName;
	std::string	fileName;
	int			line;
};

struct TestSuiteResult
{
	TestSuiteResult& operator+=(const TestSuiteResult& tsr)
	{
		nbSuccess += tsr.nbSuccess;
		nbWarning += tsr.nbWarning;
		nbError += tsr.nbError;
		nbCriticalError += tsr.nbCriticalError;
		logs.insert(logs.end(), tsr.logs.begin(), tsr.logs.end());
		return (*this);
	}

	void	addLog(std::string s, TestResult r, const std::string& f, const std::string& fn, int l)
	{
		logs.push_back(TestLog({s, r, f, fn, l}));
	}

	int	nbSuccess = 0;
	int	nbWarning = 0;
	int	nbError = 0;
	int	nbCriticalError = 0;

	std::vector<TestLog>	logs;
};

inline std::ostream&	operator<<(std::ostream& os, const TestSuiteResult& tsr)
{
	os << "nbSuccess: " << tsr.nbSuccess << std::endl;
	os << "nbWarning: " << tsr.nbWarning << std::endl;
	os << "nbError: " << tsr.nbError << std::endl;
	os << "nbCriticalError: " << tsr.nbCriticalError << std::endl;
	return os;
}

class AutoTest
{
	private:
		struct Callable
		{
			struct ITypeErasure
			{
				virtual ~ITypeErasure(){}

				virtual TestSuiteResult	operator()() = 0;
			};

			template<typename T>
			struct TypeErasure: public ITypeErasure
			{
				TypeErasure(const T& t): value(t) {}

				TestSuiteResult	operator()()
				{
					return value();
				}

				T	value;
			};

			template<typename T>
			Callable(const T& t)
			{
				ptr = std::make_shared<TypeErasure<T>>(t);
			}

			TestSuiteResult operator()()
			{
				return (*ptr)();
			}

			std::shared_ptr<ITypeErasure>	ptr;
		};

	public:
		template<typename ... Args>
		AutoTest(Args&& ... args) { addTests(std::forward<Args>(args)...); }

		TestSuiteResult		operator()()
		{
			TestSuiteResult	testSuiteResult;
			for (auto& i: _testsFunctions)
				testSuiteResult += i();
			return testSuiteResult;
		}

		template<typename ... Args>
		auto&	addTests(Args&& ... args)
		{
			(_testsFunctions.push_back(args), ...);
			return *this;
		}

		auto&	reset()
		{
			_testsFunctions.clear();
			return *this;
		}

	private:
		std::vector<Callable>	_testsFunctions;

};

}