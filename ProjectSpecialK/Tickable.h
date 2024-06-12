#pragma once

class Tickable
{
public:
	bool* mutex = nullptr;
	virtual ~Tickable() {}
	virtual void Tick(double dt) {};
	virtual void Draw(double dt) {};
	virtual bool Character(unsigned int codepoint) { return false; }
};
