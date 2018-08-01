#pragma once

#include "RubyFile.hpp"
#include "RubyParser.hpp"
#include "../communications/Communicator.hpp"

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
		RubyFile		_rubyFile; // remplacer ça par un communicator et un rubyScritCreator
		Communicator	_communicator;
};

}