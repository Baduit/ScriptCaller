# ScriptCaller
This is a header only library C++ allowing us to call ruby scripts in only few line of codes. It does not need to "adjust" the script.

## Why should i use it?
If you are using C++ and you need/want to use or a ruby script, then this is the library for you.

## What can i do with it?
You can:
* call a function with any number of argument and get the return value
* create an object
* get a description of a created object (it gives all the attributes)
* call a method and get the return value
* call a static method and get the return value
* store a value like an integer or a string
* get one of the stored value

## How does it work?
It read a ruby file and extract the name of the classes and functions, then it create an intermediate ruby script in order to be able to have a communication between the C++ executable and the script ruby, then it start in a thread the script. The C++ programme can call the script by telling him which functions/object/classes to use/create with the arguments and then the script send the response.

## How to install it
This is a header only library, so you just need to include the header in the folder __src__ (but not need to include the files from the folder test)

## The tests
### Description
The current tests are very basic and does not show the error handling but it show that a normal utilisation of the library works fine, i will probably add more tests in the future and if you want to, feel free to contribute. I only ran the test on my computer with the ubuntu bash of windows 10.
For the moment the code should not work on windows because i use named pipe and the system calls are différent on windows.

### Run the tests
In order to run the test on linux:
* mkdir build
* cmake ..
* make
* ./test.exe

Then you should see the number of successful tests, warnings, errors and critical errors and if there is errors you will see the file, the line, the expression of the test and the result (warning, error or critical erros).

### Why not use the Boost unit test library or an equivalent?
It is a personnal project and i wanted develop one on my own for the fun.
