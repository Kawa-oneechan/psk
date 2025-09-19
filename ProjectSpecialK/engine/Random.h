#pragma once

namespace Random
{
	extern void Seed(unsigned int newSeed);
	extern void Seed();
	extern int GetInt(int min, int max);
	extern int GetInt(int max);
	extern int GetInt();
	extern float GetFloat(float min, float max);
	extern float GetFloat(float max);
	extern float GetFloat();
	extern bool Flip();
}
