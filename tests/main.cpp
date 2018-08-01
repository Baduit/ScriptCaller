#include <iostream>
#include <string>

#include "AutoTest.hpp"
#include "TestRubyParser.hpp"
#include "TestRubyScriptCaller.hpp"

int main(int argc, char **argv)
{
    auto result = Test::AutoTest(&basicTestRubyParser, &basicTestRubyScriptCaller)();

    std::cout << result;

    for (const auto& i: result.logs)
        if (i.resultCode != Test::TestResult::SUCCESS)
            std::cout
                << i.fileName << "\t"
                << i.line << "\t"
                << i.functionName << "\t"
                << i.expression << "\t"
                << i.resultCode << std::endl;
}
