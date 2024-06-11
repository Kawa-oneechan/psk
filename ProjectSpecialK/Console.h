#pragma once

#include "SpecialK.h"
#include "TextField.h"

#include <fstream>

class Console : public Tickable
{
private:
	std::vector<std::pair<int, std::string>> buffer;
	std::vector<std::string> history;
	int historyCursor;
	int scrollCursor;
	void* inputLine;

	std::ofstream hardcopy;

public:
	bool visible;

	Console();
	bool Execute(const std::string& str);
	void Print(int color, const std::string& str);
	void Print(const std::string& str);
	bool Character(unsigned int codepoint);
	void Tick(double dt);
	void Draw(double dt);
};
extern Console* console;
