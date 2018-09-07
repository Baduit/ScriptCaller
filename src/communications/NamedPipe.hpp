#pragma once

#include <string>
#include <fstream>

#include <sys/stat.h>
#include <unistd.h>

#include "INamedPipe.hpp"

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
				_fstream.open(_name, std::ios::out);
			}
			else if (pipeState == READ)
			{
				_fstream.open(_name, std::ios::in);
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
			_fstream.close();
			_pipeState = UNDEFINED;
		}

		void				remove()
		{
			::unlink(_name.c_str());
		}

		bool					write(const std::string& str)
		{
			if (getPipeState() != WRITE)
				return false;
			_fstream << str;
			_fstream.flush();
			return true;
		}

		std::string			getline()
		{
			if (getPipeState() != READ)
				return "";
			std::string str;
			std::getline(_fstream, str, '\n');
			return str;
		}

		// overload stream operator << and >> to do maybe

		PipeState			getPipeState() const { return _pipeState; }
		int					getFd() const { return _fd; }
		const std::string&	getName() const { return _name; }

	private:
		int				_fd;
		PipeState		_pipeState = UNDEFINED;
		std::string		_name;
		std::fstream	_fstream;
};

