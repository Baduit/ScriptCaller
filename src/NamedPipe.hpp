#pragma once

#include <string>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

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

