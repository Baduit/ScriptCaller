#include "TestRubyScriptCaller.hpp"

using namespace ScriptCaller;
using namespace Test;

TestSuiteResult	basicTestRubyScriptCaller()
{
	TEST_BEGIN

	std::string testRubyFile = "../src/ruby/functions.rb";

	ScriptCaller::RubyScriptCaller script(testRubyFile);

	try
	{
		// callFunction
		{
			TEST_ERROR((script.callFunction<std::string>("hello") == "Hello world"));
			TEST_ERROR((script.callFunction<int>("add", 5, 9) == 14));
			TEST_ERROR((script.callFunction<std::string>("concat", "Hello", " world") == "Hello world"));

			auto intArray = script.callFunction<std::vector<int>>("makeArray", 4, 5, 9);
			TEST_ERROR((intArray.size() == 3));
			TEST_ERROR((intArray[0] == 4));
			TEST_ERROR((intArray[1] == 5));
			TEST_ERROR((intArray[2] == 9));
		}
		// callStaticMethod
		{
			TEST_ERROR((script.callStaticMethod<std::string>("Fred", "staticHi") == "static hi"));
		}
		// createObject & getObjet & callMethod
		{
			auto createdObject = script.createObject<json>("Fred", "freddy", "catty", 99);
			TEST_ERROR((createdObject["a"] == "catty"));
			TEST_ERROR((createdObject["b"] == 99));

			script.createObject("Fred", "fred", "cat", 99);
	
			auto objectFromScript = script.getObject<json>("fred");
			TEST_ERROR((objectFromScript["a"] == "cat"));
			TEST_ERROR((objectFromScript["b"] == 99));

			auto methodResult = script.callMethod<std::string>("fred", "hi");
			TEST_ERROR((methodResult == "Hi"));
		}
		// storeValue & getValue
		{
			script.storeValue("myInt", 55);
			TEST_ERROR((script.getValue<int>("myInt") == 55));
		}
	}
	catch (...)
	{
		TEST_CRITICAL_ERROR((false))
	}

	TEST_END
}