#include "TestRubyParser.hpp"

using namespace ScriptCaller;
using namespace Test;

TestSuiteResult	basicTestRubyParser()
{
	TEST_BEGIN

	auto [functions, classes] = RubyParser()("../tests/ruby/functions.rb");

	TEST_ERROR((functions.size() == 4))
	TEST_ERROR((std::find(functions.begin(), functions.end(), "hello") != std::end(functions)))
	TEST_ERROR((std::find(functions.begin(), functions.end(), "add") != std::end(functions)))
	TEST_ERROR((std::find(functions.begin(), functions.end(), "concat") != std::end(functions)))
	TEST_ERROR((std::find(functions.begin(), functions.end(), "makeArray") != std::end(functions)))

	TEST_ERROR((classes.size() == 1))
	TEST_ERROR((std::find(classes.begin(), classes.end(), "Fred") != std::end(classes)))

	TEST_END
}