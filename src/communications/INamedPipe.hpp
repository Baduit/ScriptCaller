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
		virtual bool				write(const std::string& str) = 0;
		virtual std::string			read() = 0;
		virtual PipeState			getPipeState() const  = 0;
		virtual const std::string&	getName() const = 0;
};