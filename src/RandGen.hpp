#pragma once

#include <random>
#include <memory>

class RandGen
{
	public:
		RandGen(int min = 0, int max = 1000000)
		{
			std::random_device rd;
			_rng = std::make_unique<std::mt19937>(rd());
			_uni = std::make_unique<std::uniform_int_distribution<int>>(1, 1000000);
		}

		int rand()
		{
			return ((*_uni)(*_rng));
		}

		int	operator()()
		{
			return rand();
		}
	private:
		std::unique_ptr<std::mt19937>						_rng;
		std::unique_ptr<std::uniform_int_distribution<int>>	_uni;
};