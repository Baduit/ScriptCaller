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
			_outputNamedPipe->getline();
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
			return json::parse(_outputNamedPipe->getline()).get<returnType>();
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