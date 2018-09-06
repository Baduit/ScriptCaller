#pragma once

#include <string>
#include <ostream>

class Indent
{
	public:
		enum Type
		{
			TAB,
			SPACE
		};
	public:
		Indent() = default;
		Indent(Type t): _type(t) {}
		Indent(unsigned int s): _size(s) {}
		Indent(Type t, unsigned int s): _type(t), _size(s) {}
		Indent(Type t, unsigned int s, unsigned int ss): _type(t), _size(s), _spaceSize(ss) {}

		Indent          operator*(unsigned int n) const
		{
			return Indent(_type, _size * n, _spaceSize);
		}

		Indent&         operator*=(unsigned int n)
		{
			_size * n;
			return *this;
		}

		std::string     toString() const
		{
			std::string	str;
			if (_type == Type::TAB)
			{
				for (int i = 0; i < _size; ++i)
					str += "\t";
				return str;
			}
			else
			{
				std::string add;
				for (int i = 0; i < _spaceSize; ++i)
					add += " ";

				for (int i = 0; i < _size; ++i)
					str += add;
				return str;
			}
		}

		Type            getType() const { return _type; }
		unsigned int    getSize() const { return _size; }
		unsigned int    getSpaceSize() const { return _spaceSize; }
	private:
		const Type          _type = Type::TAB;
		const unsigned int  _size = 1;
		const unsigned int  _spaceSize = 4;
};

inline
std::ostream&    operator<<(std::ostream& os, const Indent& i)
{
	return (os << i.toString());
}

class IndentFactory
{
	public:
		IndentFactory(Indent::Type t = Indent::Type::TAB, unsigned int s = 1, unsigned int ss = 4): _type(t), _size(s), _spaceSize(ss) {}

		Indent	operator()(unsigned int n = 1)
		{
			return (Indent(_type, _size, _spaceSize) * n);
		}
	private:
		const Indent::Type  _type;
		const unsigned int  _size;
		const unsigned int  _spaceSize;
};