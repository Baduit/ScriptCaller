#include "TestRubyFile.hpp"

using namespace ScriptCaller;
using namespace Test;

TestSuiteResult	basicTestRubyFile()
{
	TEST_BEGIN

	    std::string testRubyFile = "../src/ruby/functions.rb";

		ScriptCaller::FunctionList fList = { "hello", "add", "concat", "makeArray" };
		ScriptCaller::ClassList cList = { "Fred" };

		ScriptCaller::RubyFile rb(testRubyFile, fList, cList);
		rb.createFile();
		rb.launchScript();

		try
		{
			// callFunction
			{
				TEST_ERROR((rb.callFunction<std::string>("hello") == "Hello world"));
				TEST_ERROR((rb.callFunction<int>("add", 5, 9) == 14));
				TEST_ERROR((rb.callFunction<std::string>("concat", "Hello", " world") == "Hello world"));

				auto intArray = rb.callFunction<std::vector<int>>("makeArray", 4, 5, 9);
				TEST_ERROR((intArray.size() == 3));
				TEST_ERROR((intArray[0] == 4));
				TEST_ERROR((intArray[1] == 5));
				TEST_ERROR((intArray[2] == 9));
			}
			// callStaticMethod
			{
				TEST_ERROR((rb.callStaticMethod<std::string>("Fred", "staticHi") == "static hi"));
			}
			// createObject & getObjet & callMethod
			{
				auto createdObject = rb.createObject<json>("Fred", "fred", "cat", 99);
				TEST_ERROR((createdObject["a"] == "cat"));
				TEST_ERROR((createdObject["b"] == 99));

				auto objectFromScript = rb.getObject<json>("fred");
				TEST_ERROR((objectFromScript["a"] == "cat"));
				TEST_ERROR((objectFromScript["b"] == 99));

				auto methodResult = rb.callMethod<std::string>("fred", "hi");
				TEST_ERROR((methodResult == "Hi"));
			}
			// storeValue & getValue
			{
				TEST_ERROR((rb.storeValue<int>("myInt", 55) == 55));
				TEST_ERROR((rb.getValue<int>("myInt") == 55));
			}
		}
		catch (...)
		{
			TEST_CRITICAL_ERROR((false))
		}

	TEST_END
}