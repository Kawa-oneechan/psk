#pragma once

class Tickable
{
public:
	bool* mutex = nullptr;
	void Tick(double dt) {};
};
