#pragma once

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