#pragma once

#include <utility>
#include <fstream>
#include <vector>
#include <string>

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
		RubyParser() = default;

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

		void	extractInformationFromChunks(const auto& chunks)
		{
			int		blockDepth = 0;
			bool	isInString = false;
			bool	isNextClass = false;
			bool	isNextFunction = false;
			for (const auto& [str, c]: chunks)
			{
				// everything in a string is useless
				if (isInString)
				{
					// check if this is the last chunk of the string
					if (c == '\'' || c == 34)
						isInString = false;
					continue;
				}

				// register the usefull informations
				if (isNextFunction && blockDepth == 1 && !isInString)
					addFunction(str); 
				else if (isNextClass && blockDepth == 1 && !isInString)
					addClass(str);

				// here we can reset class and function boolean because they have been added if usefull
				isNextClass = false;
				isNextFunction = false;

				// detect if the next chunk will be a class or a function
				if (str == "def")
					isNextFunction = true;
				else if (str == "class")
					isNextClass = true;

				// calculating the depth
				
				if (str == "end")
					--blockDepth;
				if (str == "def" || str == "class" || str == "if" ||
					str == "begin" || str == "case" || str == "do" ||
					str == "while" || str == "until" || str == "for" ||
					str == "unless")
					++blockDepth;

				// at the end, check is the next chunk will be in a string
				if (c == '\'' || c == 34)
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
};

}