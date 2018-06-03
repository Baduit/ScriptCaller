#pragma once

#include "RubyFile.hpp"
#include "RubyParser.hpp"

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
			_rubyFile.launchScript();
		}

		template<typename returnType, typename ... Args>
		returnType	callFunction(const std::string& functionName, Args&& ... args)
		{
			return _rubyFile.callFunction<returnType>(functionName, std::forward<Args>(args) ...);
		}

		template<typename returnType, typename ... Args>
		returnType	callStaticMethod(const std::string& className, const std::string& methodName, Args&& ... args)
		{
			return _rubyFile.callStaticMethod<returnType>(className, methodName, std::forward<Args>(args) ...);
		}

		template<typename returnType, typename ... Args>
		returnType	callMethod(const std::string& objectName, const std::string& methodName, Args&& ... args)
		{
			return _rubyFile.callMethod<returnType>(objectName, methodName, std::forward<Args>(args) ...);
		}

		template<typename ... Args>
		void	createObject(const std::string& className, const std::string& objectName, Args&& ... args)
		{
			_rubyFile.createObject(className, objectName, std::forward<Args>(args) ...);
		}

		template<typename returnType, typename ... Args>
		returnType	createObject(const std::string& className, const std::string& objectName, Args&& ... args)
		{
			return _rubyFile.createObject<returnType>(className, objectName, std::forward<Args>(args) ...);
		}

		template<typename inputType>
		void	storeValue(const std::string& valueName, const inputType& value)
		{
			_rubyFile.storeValue(valueName, value);
		}

		template<typename returnType>
		returnType	getValue(const std::string& valueName)
		{
			return _rubyFile.getValue<returnType>(valueName);
		}

		template<typename returnType>
		returnType	getObject(const std::string& valueName)
		{
			return _rubyFile.getObject<returnType>(valueName);
		}

		void	close()
		{
			_rubyFile.close();
		}

	private:
		RubyFile	_rubyFile;
};

}