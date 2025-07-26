#include <random>

namespace rnd
{
	std::random_device device;
	std::mt19937 engine(device());

	int GetInt(int min, int max)
	{
		std::uniform_int_distribution<> dist(min, max);
		return dist(engine);
	}

	int GetInt(int max)
	{
		return GetInt(0, max);
	}

	int GetInt()
	{
		return GetInt(0, RAND_MAX);
	}

	float GetFloat(float min, float max)
	{
		std::uniform_real_distribution<> dist(min, max);
		return (float)dist(engine);
	}

	float GetFloat(float max)
	{
		return GetFloat(0.0f, max);
	}

	float GetFloat()
	{
		return GetFloat(0.0f, 1.0f);
	}

	bool Flip()
	{
		return GetFloat() > 0.5f;
	}
}
