#pragma once
#include <string>

enum PipeState: int
{
	WRITE = 1,
	READ = 2,
	UNDEFINED = 0
};

class INamedPipe
{
	public:
		~INamedPipe() = default;

		virtual void				setName(const std::string& name) = 0;
		virtual bool				create() = 0;
		virtual bool				open(PipeState pipeState) = 0;
		virtual void				close() = 0;
		virtual void				remove() = 0;
		virtual int					write(const std::string& str) = 0;
		virtual std::string			read() = 0;
		virtual PipeState			getPipeState() const  = 0;
		virtual const std::string&	getName() const = 0;
};
#include <string>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>


constexpr int MAX_BUF = 1024;

class NamedPipe: public INamedPipe
{
	public:
		NamedPipe() = default;
		NamedPipe(const std::string& name): _name(name) {}
		~NamedPipe()
		{
			if (getPipeState() != UNDEFINED)
				close();
		}

		void				setName(const std::string& name) { _name = name; }

		bool				create()
		{
			::unlink(_name.c_str());
			if (::mkfifo(_name.c_str(), 0666) == -1)
			{
				return false;
			}
			return true;
		}

		bool				open(PipeState pipeState)
		{
			if (_pipeState != UNDEFINED)
				close();
			
			if (pipeState == WRITE)
			{	
				if ((_fd = ::open(_name.c_str(), O_WRONLY)) == -1)
					return false;		
			}
			else if (pipeState == READ)
			{
				if ((_fd = ::open(_name.c_str(), O_RDONLY)) == -1)
					return false;
			}
			else
			{
				return false;
			}

			_pipeState = pipeState;
			return true;
		}

		void				close()
		{
			::close(_fd);
			_pipeState = UNDEFINED;
		}

		void				remove()
		{
			::unlink(_name.c_str());
		}

		int					write(const std::string& str)
		{
			if (getPipeState() != WRITE)
				return -1;
			return ::write(_fd, str.c_str(), str.length());
		}

		std::string			read()
		{
			if (getPipeState() != READ)
				return "";
			char buf[MAX_BUF];
			int nb_lu = ::read(_fd, buf, MAX_BUF);
			buf[nb_lu] = 0;
			return std::string(buf);
		}

		// overload stream operator << and >> to do maybe

		PipeState			getPipeState() const { return _pipeState; }
		int					getFd() const { return _fd; }
		const std::string&	getName() const { return _name; }

	private:
		int			_fd;
		PipeState	_pipeState = UNDEFINED;
		std::string	_name;
};


#include <string>
#include <vector>
#include <random>
#include <fstream>
#include <future>
#include <thread>
#include <cstdlib>
#include <memory>

#include <json.hpp>

// for convenience
using json = nlohmann::json;
using Strings = std::vector<std::string>;


namespace ScriptCaller
{


using ClassDescription = std::string;
using FunctionDescription = std::string;
using FunctionList = std::vector<FunctionDescription>;
using ClassList = std::vector<ClassDescription>;

namespace ActionType
{
constexpr char callFunction[] = "callFunction";
constexpr char callObjectMethod[] = "callObjectMethod";
constexpr char callStaticMethod[] = "callStaticMethod";
constexpr char getObject[] = "getObject";
constexpr char createObject[] = "createObject";
constexpr char getValue[] = "getValue";
constexpr char storeValue[] = "storeValue";
}

class Communicator
{
	public:
		Communicator() = default;
		
		~Communicator() { close(); }

		void	launchScript(const std::string& outputFileName, const std::string& namedPipeInputName, const std::string& namedPipeOutputName)
		{
			_inputNamedPipe = std::make_unique<NamedPipe>(namedPipeInputName);
			_inputNamedPipe->create();
			_outputNamedPipe = std::make_unique<NamedPipe>(namedPipeOutputName);
			_outputNamedPipe->create();

			_thread = std::thread([&]() {
				::system(std::string("ruby " + outputFileName).c_str());
			});
			_thread.detach();

			_inputNamedPipe->open(WRITE);
			_outputNamedPipe->open(READ);
			_pipesAreOpen = true;
		}

		template<typename returnType, typename ... Args>
		returnType	callFunction(const std::string& functionName, Args&& ... args)
		{
			json	j;
			j["functionName"] = functionName;
			j["actionType"] = ActionType::callFunction;
			addArgsToJson(j, std::forward<Args>(args) ...);
			return callScript<returnType>(j);
		}

		template<typename returnType, typename ... Args>
		returnType	callStaticMethod(const std::string& className, const std::string& methodName, Args&& ... args)
		{
			json	j;
			j["className"] = className;
			j["methodName"] = methodName;
			j["actionType"] = ActionType::callStaticMethod;
			addArgsToJson(j, std::forward<Args>(args) ...);
			return callScript<returnType>(j);
		}

		template<typename returnType, typename ... Args>
		returnType	callMethod(const std::string& objectName, const std::string& methodName, Args&& ... args)
		{
			json	j;
			j["objectName"] = objectName;
			j["methodName"] = methodName;
			j["actionType"] = ActionType::callObjectMethod;
			addArgsToJson(j, std::forward<Args>(args) ...);
			return callScript<returnType>(j);
		}

		template<typename ... Args>
		void		createObject(const std::string& className, const std::string& objectName, Args&& ... args)
		{
			json	j;
			j["className"] = className;
			j["objectName"] = objectName;
			j["actionType"] = ActionType::createObject;
			addArgsToJson(j, std::forward<Args>(args) ...);
			callScript(j);
		}

		template<typename returnType, typename ... Args>
		returnType	createObject(const std::string& className, const std::string& objectName, Args&& ... args)
		{
			json	j;
			j["className"] = className;
			j["objectName"] = objectName;
			j["actionType"] = ActionType::createObject;
			addArgsToJson(j, std::forward<Args>(args) ...);
			return callScript<returnType>(j);
		}

		template<typename inputType>
		void		storeValue(const std::string& valueName, const inputType& value)
		{
			json	j;
			j["valueName"] = valueName;
			j["value"] = value;
			j["actionType"] = ActionType::storeValue;
			callScript(j);
		}

		template<typename returnType>
		returnType	getValue(const std::string& valueName)
		{
			json	j;
			j["variableName"] = valueName;
			j["actionType"] = ActionType::getValue;
			return callScript<returnType>(j);
		}

		template<typename returnType>
		returnType	getObject(const std::string& valueName)
		{
			json	j;
			j["variableName"] = valueName;
			j["actionType"] = ActionType::getObject;
			return callScript<returnType>(j);
		}

		void	close()
		{
			if (_pipesAreOpen)
				_inputNamedPipe->write("__STOP_SCRIPT__\n");
			_pipesAreOpen = false;
		}

	private:
		void	callScript(const json& j)
		{
			sendInputToScript(j.dump());
			_outputNamedPipe->read();
		}

		template<typename returnType>
		returnType	callScript(const json& j)
		{
			sendInputToScript(j.dump());
			return getOutputFromScript<returnType>();
		}

		template<typename returnType>
		returnType	getOutputFromScript()
		{
			return json::parse(_outputNamedPipe->read()).get<returnType>();
		}

		void		sendInputToScript(const std::string& input)
		{
			_inputNamedPipe->write(input + "\n");
		}

		template<typename ... Args>
		void		addArgsToJson(json& j, Args&& ... args)
		{
			j["args"] = {};
			(j["args"].push_back(args), ...);
		}

	private:
		bool							_pipesAreOpen = false;

		std::unique_ptr<INamedPipe>		_inputNamedPipe; // cpp to script, script read here
		std::unique_ptr<INamedPipe>		_outputNamedPipe; // script to cpp, script write here
		
		std::thread						_thread;
};

}
#include <random>
#include <memory>

class RandGen
{
	public:
		RandGen(int min = 0, int max = 1000000)
		{
			std::random_device rd;
			_rng = std::make_unique<std::mt19937>(rd());
			_uni = std::make_unique<std::uniform_int_distribution<int>>(1, 1000000);
		}

		int rand()
		{
			return ((*_uni)(*_rng));
		}

		int	operator()()
		{
			return rand();
		}
	private:
		std::unique_ptr<std::mt19937>						_rng;
		std::unique_ptr<std::uniform_int_distribution<int>>	_uni;
};
#include <utility>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

namespace ScriptCaller
{

using Strings = std::vector<std::string>;

using ClassDescription = std::string;
using FunctionDescription = std::string;
using FunctionList = std::vector<FunctionDescription>;
using ClassList = std::vector<ClassDescription>;

using FileInformation = std::pair<FunctionList, ClassList>;

class RubyParser
{
	public:
		RubyParser() { _endBlockDelim = {"def", "class", "if", "begin", "case", "do", "while", "until", "for", "unless"}; }

	private:
		void	openFile(const std::string& fileName) { _file.open(fileName); }
		void	addFunction(FunctionDescription f) { _functions.push_back(f); }
		void	addClass(ClassDescription c) { _classes.push_back(c); }
		
		auto	splitFileToChunks()
		{
			std::vector<std::pair<std::string, char>> chunks;
			std::string currentChunk;
			char c;
			while ((_file.read(&c, 1)))
				if (isCharaterDelimiter(c))
				{
					chunks.push_back(make_pair(currentChunk, c));
					currentChunk = "";
				}
				else
					currentChunk += c;
			return chunks;
		}

		bool	isCharStringDelim(char c) { return (c == '\'' || c == 34) ? true : false; }
		bool	isStrEndBlock(const std::string& str) { return (str == "end") ? true : false; }
		bool	isStrBeginBlock(const std::string& str) { return (std::find(_endBlockDelim.begin(), _endBlockDelim.end(), str) != std::end(_endBlockDelim)) ? true : false; }
		bool	isStrBeginFunc(const std::string& str) { return (str == "def") ? true : false; }
		bool	isStrBeginClass(const std::string& str) { return (str == "class") ? true : false; }


		void	extractInformationFromChunks(const auto& chunks)
		{
			int		blockDepth = 0;
			bool	isInString = false;
			bool	isNextClass = false;
			bool	isNextFunction = false;

			for (const auto& [str, c]: chunks)
			{
				if (isInString)
				{
					if (isCharStringDelim(c))
						isInString = false;
					continue;
				}

				if (isNextFunction && blockDepth == 1)
					addFunction(str); 
				else if (isNextClass && blockDepth == 1)
					addClass(str);

				isNextFunction = isStrBeginFunc(str) ? true : false;
				isNextClass = isStrBeginClass(str) ? true : false;
	
				if (isStrEndBlock(str))
					--blockDepth;
				if (isStrBeginBlock(str))
					++blockDepth;

				if (isCharStringDelim(c))
					isInString = true;
			}
		}

		bool	isCharaterDelimiter(char c) const
		{
			std::string	delimiters = "\"'()\n \t\r";
			for (auto d: delimiters)
				if (!c || c == d)
					return true;
			return false;
		}

	public:
		FileInformation	operator()(const std::string& fileName)
		{
			openFile(fileName);
			auto chunks = splitFileToChunks();
			extractInformationFromChunks(chunks);
			return std::make_pair(_functions, _classes);
		}

	private:
		std::ifstream	_file;
		FunctionList	_functions;
		ClassList		_classes;

		Strings			_endBlockDelim;
};

}
#include <string>
#include <vector>
#include <random>
#include <fstream>
#include <future>
#include <thread>
#include <cstdlib>
#include <memory>


namespace ScriptCaller
{


using ClassDescription = std::string;
using FunctionDescription = std::string;
using FunctionList = std::vector<FunctionDescription>;
using ClassList = std::vector<ClassDescription>;

class RubyFile
{
	public:
		RubyFile() = default;
		RubyFile(std::string fileName, const FunctionList& fList, const ClassList& cList) :
			requiredFileName(fileName), functions(fList), classes(cList)
			{}
		
		~RubyFile() {}

		void	init(std::string fileName, const FunctionList& fList, const ClassList& cList)
		{
			requiredFileName = fileName;
			functions = fList;
			classes = cList;
		}

		void	createFile()
		{
			// create the file with a header
			_outputFileName = "/tmp/fifo_" + std::to_string(_randGen()) + ".rb";
			std::ofstream outputFile(_outputFileName);
			outputFile << "# File generated by ScriptCaller" << std::endl;
			outputFile << "# https://github.com/Baduit/ScriptCaller" << std::endl;

			// add the require statement
			outputFile << "require '" << requiredFileName << "'" << std::endl << std::endl;
			outputFile << "require 'json'" << std::endl << std::endl;

			// function object to hash
			outputFile << "def objectToHash(obj)" << std::endl;
			outputFile << "\t" << "hash = {}" << std::endl;
			outputFile << "\t" << "obj.instance_variables.each {|var| hash[var.to_s.delete(\"@\")] = obj.instance_variable_get(var.to_s) }" << std::endl;
			outputFile << "\t" << "return hash" << std::endl;
			outputFile << "end" << std::endl;



			// create both named pipe
			_namedPipeInputName = "/tmp/fifo_input" + std::to_string(_randGen());
			_namedPipeOutputName = "/tmp/fifo_output" + std::to_string(_randGen());

			outputFile << "input = open('" << _namedPipeInputName << "', \"r\")" << std::endl << std::endl;
			outputFile << "output = open('" << _namedPipeOutputName << "', \"w\")" << std::endl << std::endl;

			// create the function which call the good function
			outputFile << "def bigSwitch(order)" << std::endl;
			for (const auto& f: functions)
			{
				outputFile << "\t" << "if order.functionName == \"" << f << "\""<< std::endl;
				outputFile << "\t\t"<< "return " << f << " *(order.args)" << std::endl;
				outputFile << "\t" << "end" << std::endl;
			}
			outputFile << "\t" << "return 'Error: function unknown'" << std::endl;
			outputFile << "end" << std::endl << std::endl;

			// function to call a static method
			outputFile << "def callStaticMethod(order)" << std::endl;
			for (const auto& c: classes)
			{
				outputFile << "\t" << "if order.className == \"" << c << "\""<< std::endl;
				outputFile
					<< "\t\t"<< "return "
					<< c << ".method(order.methodName).call *(order.args)" << std::endl;
				outputFile << "\t" << "end" << std::endl;
			}
			outputFile << "\t" << "return 'Error: classes unknown'" << std::endl;
			outputFile << "end" << std::endl << std::endl;

			// function to store a value
			outputFile << "def storeValue(order, createdVariables)" << std::endl;
			outputFile << "\t" << "createdVariables[order.valueName] = order.value" << std::endl;
			outputFile << "\t" << "return createdVariables[order.valueName]" << std::endl;
			outputFile << "end" << std::endl << std::endl;
			
			// function to create an object
			outputFile << "def createObject(order, createdVariables)" << std::endl;
			for (const auto& c: classes)
			{
				outputFile << "\t" << "if order.className == \"" << c << "\""<< std::endl;
				outputFile
					<< "\t\t"<< "createdVariables[order.objectName] = "
					<< c << ".new(*(order.args))" << std::endl;
				outputFile << "\t" << "return objectToHash(createdVariables[order.objectName])" << std::endl;
				outputFile << "\t" << "end" << std::endl;
			}
			outputFile << "\t" << "return 'Error: classes unknown'" << std::endl;
			outputFile << "end" << std::endl << std::endl;

			// function to get an object
			outputFile << "def getObject(order, createdVariables)" << std::endl;
			outputFile << "\t" << "return objectToHash(createdVariables[order.variableName])" << std::endl;
			outputFile << "end" << std::endl << std::endl;

			// function to get an object
			outputFile << "def getValue(order, createdVariables)" << std::endl;
			outputFile << "\t" << "return createdVariables[order.variableName]" << std::endl;
			outputFile << "end" << std::endl << std::endl;

			// function to call a method
			outputFile << "def callMethod(order, createdVariables)" << std::endl;
			outputFile << "\t" << "return createdVariables[order.objectName].method(order.methodName).call *(order.args)" << std::endl;
			outputFile << "end" << std::endl << std::endl;

			// create the part using the named pipe created earlier
			outputFile << "taskOver = false" << std::endl;
			outputFile << "createdVariables = {}" << std::endl;
			outputFile << "while true do" << std::endl;
			outputFile << "\t" << "orderString = input.gets" << std::endl;
			outputFile << "\t" << "orderString = orderString.chop" << std::endl;
			outputFile << "\t" << "if orderString == '__STOP_SCRIPT__'" << std::endl;
			outputFile << "\t\t" << "exit(0)" << std::endl;
			outputFile << "\t" << "end" << std::endl;
			outputFile << "\t" << "order = JSON.parse(orderString, object_class: OpenStruct)" << std::endl;
			outputFile << "\t" << "answer = 'actionType incorrect'.to_json" << std::endl;
			outputFile << "\t" << "if order.actionType == '"<< ActionType::callFunction << "'" << std::endl;
			outputFile << "\t\t" << "answer = bigSwitch(order)" << std::endl;
			outputFile << "\t" << "elsif order.actionType == '"<< ActionType::callStaticMethod << "'" << std::endl;
			outputFile << "\t\t" << "answer = callStaticMethod(order)" << std::endl;
			outputFile << "\t" << "elsif order.actionType == '"<< ActionType::callObjectMethod << "'" << std::endl;
			outputFile << "\t\t" << "answer = callMethod(order, createdVariables)" << std::endl;
			outputFile << "\t" << "elsif order.actionType == '"<< ActionType::createObject << "'" << std::endl;
			outputFile << "\t\t" << "answer = createObject(order, createdVariables)" << std::endl;
			outputFile << "\t" << "elsif order.actionType == '"<< ActionType::storeValue << "'" << std::endl;
			outputFile << "\t\t" << "answer = storeValue(order, createdVariables)" << std::endl;
			outputFile << "\t" << "elsif order.actionType == '"<< ActionType::getObject << "'" << std::endl;
			outputFile << "\t\t" << "answer = getObject(order, createdVariables)" << std::endl;
			outputFile << "\t" << "elsif order.actionType == '"<< ActionType::getValue << "'" << std::endl;
			outputFile << "\t\t" << "answer = getValue(order, createdVariables)" << std::endl;
			outputFile << "\t" << "end" << std::endl;
			outputFile << "\t" << "output.puts(answer.to_json) " << std::endl;
			outputFile << "\t" << "output.flush " << std::endl;
			outputFile << "end" << std::endl;
		}


		const std::string& getOutputFileName() const { return _outputFileName; }
		const std::string& getNamedPipeInputName() const { return _namedPipeInputName; }
		const std::string& getNamedPipeOutputName() const { return _namedPipeOutputName; }

	public:
		std::string		requiredFileName;
		FunctionList	functions;
		ClassList		classes;

	private:
		std::string						_outputFileName;
		std::string						_namedPipeInputName; // cpp to script, script read here
		std::string						_namedPipeOutputName; // script to cpp, script write here

		RandGen							_randGen;
};

}

namespace ScriptCaller
{

class RubyScriptCaller
{
	public:
		RubyScriptCaller() = default;
		RubyScriptCaller(const std::string& fileName) { useScript(fileName); }

		void useScript(const std::string& fileName)
		{
			auto [fList, cList] = RubyParser()(fileName);

			_rubyFile.init(fileName, fList, cList);
			_rubyFile.createFile();
			_communicator.launchScript(_rubyFile.getOutputFileName(), _rubyFile.getNamedPipeInputName(), _rubyFile.getNamedPipeOutputName());
		}

		template<typename returnType, typename ... Args>
		returnType	callFunction(const std::string& functionName, Args&& ... args)
		{
			return _communicator.callFunction<returnType>(functionName, std::forward<Args>(args) ...);
		}

		template<typename returnType, typename ... Args>
		returnType	callStaticMethod(const std::string& className, const std::string& methodName, Args&& ... args)
		{
			return _communicator.callStaticMethod<returnType>(className, methodName, std::forward<Args>(args) ...);
		}

		template<typename returnType, typename ... Args>
		returnType	callMethod(const std::string& objectName, const std::string& methodName, Args&& ... args)
		{
			return _communicator.callMethod<returnType>(objectName, methodName, std::forward<Args>(args) ...);
		}

		template<typename ... Args>
		void	createObject(const std::string& className, const std::string& objectName, Args&& ... args)
		{
			_communicator.createObject(className, objectName, std::forward<Args>(args) ...);
		}

		template<typename returnType, typename ... Args>
		returnType	createObject(const std::string& className, const std::string& objectName, Args&& ... args)
		{
			return _communicator.createObject<returnType>(className, objectName, std::forward<Args>(args) ...);
		}

		template<typename inputType>
		void	storeValue(const std::string& valueName, const inputType& value)
		{
			_communicator.storeValue(valueName, value);
		}

		template<typename returnType>
		returnType	getValue(const std::string& valueName)
		{
			return _communicator.getValue<returnType>(valueName);
		}

		template<typename returnType>
		returnType	getObject(const std::string& valueName)
		{
			return _communicator.getObject<returnType>(valueName);
		}

		void	close()
		{
			_communicator.close();
		}

	private:
		RubyFile		_rubyFile;
		Communicator	_communicator;
};

}