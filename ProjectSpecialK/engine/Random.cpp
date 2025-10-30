#include <random>
#include <ctime>

namespace Random
{
	//static std::random_device device;
	//static std::mt19937 engine(device());
	static std::mt19937 engine((unsigned int)std::clock());

	void Seed(unsigned int newSeed)
	{
		engine.seed(newSeed);
	}

	void Seed()
	{
		auto newSeed = (unsigned int)std::clock();
		engine.seed(newSeed);
	}

	int GetInt(int min, int max)
	{
		std::uniform_int_distribution<> dist(min, max - 1);
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
