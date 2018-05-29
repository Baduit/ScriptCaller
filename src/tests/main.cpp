#include <iostream>
#include <string>

#include "tests/AutoTest.hpp"
#include "tests/TestRubyParser.hpp"
#include "tests/TestRubyFile.hpp"

#define STRISATION(expr) "expr"

using json = nlohmann::json;

int main(int argc, char **argv)
{
    auto result = Test::AutoTest(&basicTestRubyParser, &basicTestRubyFile)();

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