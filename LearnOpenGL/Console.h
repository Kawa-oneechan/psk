#pragma once

#include "SpecialK.h"
#include "TextField.h"

class Console : public Tickable
{
private:
	std::vector<std::string> buffer;
	TextField inputLine;

public:
	bool visible;

	Console();
	bool Execute(const std::string& str);
	void Print(const std::string& str);
	bool Character(unsigned int codepoint);
	void Tick(double dt);
	void Draw(double dt);
};
