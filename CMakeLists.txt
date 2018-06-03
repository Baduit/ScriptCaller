cmake_minimum_required(VERSION 3.0)

INCLUDE(CMakeForceCompiler) 
CMAKE_FORCE_C_COMPILER(gcc GNU)
CMAKE_FORCE_CXX_COMPILER(g++ GNU)

project(ScriptCaller)

set (CMAKE_CXX_FLAGS "-std=gnu++17 -pthread -g")

include_directories(test PRIVATE ./src)

set(SRCS
    src/tests/main.cpp
    src/tests/TestRubyFile.cpp
    src/tests/TestRubyParser.cpp
    src/tests/TestRubyScriptCaller.cpp
    )

add_executable(test.exe ${SRCS})