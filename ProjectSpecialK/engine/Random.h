#pragma once

namespace Random
{
	//Resets the randomizer seed to the given value.
	extern void Seed(unsigned int newSeed);
	//Resets the randomizer seed to the current tick count.
	extern void Seed();
	//Returns a value from min to max, exclusively.
	extern int GetInt(int min, int max);
	//Returns a value from zero to max, exclusively.
	extern int GetInt(int max);
	//Returns a value from zero to RAND_MAX.
	extern int GetInt();
	//Returns a value from min to max.
	extern float GetFloat(float min, float max);
	//Returns a value from zero to max.
	extern float GetFloat(float max);
	//Returns a value from 0.0 to 1.0.
	extern float GetFloat();
	//Returns a 50% chance of true or false.
	extern bool Flip();
}
