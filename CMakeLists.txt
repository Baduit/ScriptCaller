cmake_minimum_required(VERSION 3.0)

project(ScriptCaller)

set (CMAKE_CXX_FLAGS "-std=c++17 -pthread -g")

include_directories(test PRIVATE ./src ./src/ruby ./tests ./tests/ruby ./ext_lib)

set(SRCS
    tests/main.cpp
    tests/ruby/TestRubyParser.cpp
    tests/ruby/TestRubyScriptCaller.cpp
    )

add_executable(test.exe ${SRCS})