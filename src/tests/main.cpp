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

    //std::string scriptFile = "/mnt/c/Users/Simon/Documents/FileCpp/src/ruby/functions.rb";
    
    /* std::string testRubyFile = scriptFile;

    ScriptCaller::FunctionList fList = { "hello", "add", "concat", "makeArray" };
    ScriptCaller::ClassList cList = { "Fred" };

    ScriptCaller::RubyFile rb(testRubyFile, fList, cList);
    rb.createFile();
    rb.launchScript();
    {
        auto answer = rb.createObject<json>("Fred", "fred", "cat", 99);
        std::cout << answer << std::endl;
    }
    {
        auto answer = rb.callMethod<std::string>("fred", "hi");
        std::cout << answer << std::endl;
    } */
    /* for (auto& i: answer)
        std::cout << i << std::endl; */
}

// à faire: changer les pipes nommés en sockets