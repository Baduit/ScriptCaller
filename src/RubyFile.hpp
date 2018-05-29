#pragma once

#include <string>
#include <vector>
#include <random>
#include <fstream>
#include <future>
#include <thread>
#include <cstdlib>
#include <memory>

#include "json.hpp"

// for convenience
using json = nlohmann::json;
using Strings = std::vector<std::string>;

#include "RandGen.hpp"
#include "INamedPipe.hpp"
#include "NamedPipe.hpp"

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

class RubyFile
{
	public:
		RubyFile() = default;
		RubyFile(std::string fileName, const FunctionList& fList, const ClassList& cList) :
			requiredFileName(fileName), functions(fList), classes(cList)
			{}
		
		~RubyFile() { close(); }

		void	createFile()
		{
			// create the file with a header
			_outputFileName = "/tmp/fifo_" + std::to_string(_randGen()) + ".rb";
			std::ofstream outputFile(_outputFileName);
			outputFile << "# File generated by ScriptCaller" << std::endl;
			outputFile << "# Ici il faudra que j'insère le lien github" << std::endl;

			//outputFile << "puts 'HI'" << std::endl << std::endl;

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

		void	launchScript()
		{
			_inputNamedPipe = std::make_unique<NamedPipe>(_namedPipeInputName);
			_inputNamedPipe->create();
			_outputNamedPipe = std::make_unique<NamedPipe>(_namedPipeOutputName);
			_outputNamedPipe->create();

			_thread = std::thread([&]() {
				::system(std::string("ruby " + _outputFileName).c_str());
			});
			_thread.detach();

			_inputNamedPipe->open(WRITE);
			_outputNamedPipe->open(READ);
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

		template<typename returnType, typename inputType>
		returnType	storeValue(const std::string& valueName, const inputType& value)
		{
			json	j;
			j["valueName"] = valueName;
			j["value"] = value;
			j["actionType"] = ActionType::storeValue;
			return callScript<returnType>(j);
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
			_inputNamedPipe->write("__STOP_SCRIPT__\n");
		}

	private:
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

	public:
		std::string		requiredFileName;
		FunctionList	functions;
		ClassList		classes;

	private:
		std::string						_outputFileName;
		std::string						_namedPipeInputName; // cpp to script, script read here
		std::string						_namedPipeOutputName; // script to cpp, script write here

		std::unique_ptr<INamedPipe>		_inputNamedPipe; // cpp to script, script read here
		std::unique_ptr<INamedPipe>		_outputNamedPipe; // script to cpp, script write here

		RandGen							_randGen;
		
		std::thread						_thread;
};

}