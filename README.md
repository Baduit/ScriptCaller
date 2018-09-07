# ScriptCaller
This is a header only C++ library allowing us to call ruby scripts in only few line of codes. It does not need to "adjust" the script.

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
It reads a ruby file and extracts the name of the classes and functions, then it creates an intermediate ruby script in order to be able to have a communication between the C++ executable and the script ruby, then it starts in a thread the script. The C++ programme can call the script by telling him which functions/object/classes to use/create with the arguments and then the script sends the response.

## How to install it
This is a header only library, so you just need to include the file __ScriptCaller.hpp__ located in the folder __unique_header__ or all the headers located in the folder __src__.

## The tests
### Description
The current tests are very basic and does not show the error handling but it shows that a normal utilisation of the library works fine, i will probably add more tests in the future and if you want to, feel free to contribute. I only ran the test on my computer with the ubuntu bash of windows 10.
For the moment the code should not work on windows because i use named pipe and the system calls are different on windows.

### Run the tests
In order to run the test on linux:
* mkdir build
* cd build
* cmake ..
* make
* ./test.exe

Then you should see the number of successful tests, warnings, errors and critical errors and if there are errors you will see the file, the line, the expression of the test and the result (warning, error or critical erros).

### Why not use the Boost unit test library or an equivalent?
It is a personnal project and i wanted develop the little tool to manage the testing on my own for the fun.

## How to use it?
Here a little example of how you can use the lib.

```C++
int main()
{
 	ScriptCaller::RubyScriptCaller script("../src/ruby/functions.rb");

	// call a function which add 2 integers
	script.callFunction<int>("add", 5, 9);

	// call a static method named "staticHi" of a class named Fred which return a string
	script.callStaticMethod<std::string>("Fred", "staticHi");

	// create an instance of the class Fred named fred with the constructor parameter,
    	// then get the object and at the end call a method of this object
	script.createObject("Fred", "fred", "cat", 99);
	script.getObject<json>("fred");
    	script.callMethod<std::string>("fred", "hi");

	// store an integer and then get it back
	script.storeValue("myInt", 55);
	script.getValue<int>("myInt");
}
```

The template parameter you can see in front of __callFunction__, __callStaticMethod__, __getObject__ and __callMethod__ is the return type.
The type __json__ is juste a typedef for __nlohmann::json__ from the library i use to handle json. (https://github.com/nlohmann/json)

Here the ruby file used to run this example
```ruby
def hello()
	return "Hello world"
end

def add(a, b)
	return a + b
end

def concat(s1, s2)
	return s1 + s2
end

def makeArray(a, b, c)
	return [a, b, c]
end

class Fred
    def initialize(p1, p2)
      @a, @b = p1, p2
	end
	
	def hi()
		return "Hi"
	end

	def a()
		return @a
	end

	def self.staticHi()
		return "static hi"
	end
end
```
