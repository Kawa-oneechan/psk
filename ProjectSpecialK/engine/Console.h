#pragma once
#include <functional>
#include <fstream>
#include <format.h>
#include <glm/glm.hpp>
#include "Tickable.h"
#include "JsonUtils.h"

class TextField;

struct CVar
{
	//Should not contain any spaces so as to not confuse the parser.
	std::string name;
	enum class Type
	{
		Bool, Int, Float, String, Vec2, Vec3, Vec4, Color
	} type;
	union
	{
		void* asVoid;
		bool* asBool;
		int* asInt;
		float* asFloat;
		std::string* asString;
		glm::vec2* asVec2;
		glm::vec3* asVec3;
		glm::vec4* asVec4;
	};
	//If enabled, the player is not allowed to change the variable's value
	//without first setting cheatsEnabled in some way.
	bool cheat;
	//For int and float type variables, specifies the minimum and maximum
	//values allowed.
	int min, max;
	std::string description;

	//Attempts to set the variable to the given value, which is parsed as if
	//it is JSON. So for a vec2 type console variable, the value should be in
	//the form [ x, y ]. For a color, it can be an array of three or four
	//float values, or a string hex-code (""#rrggbb" or "#aarrggbb").
	//Note that the game calling Set does not bother with the cheat flag.
	bool Set(const std::string& value);
	std::string ToString();
};

struct CCmd
{
	std::string name;
	std::function<void(const jsonArray& args)> act;
	std::string description;
};

//Offers a Quake-like command console. It is Tickable, but should not be
//part of the tree.
class Console : public Tickable
{
private:
	std::vector<std::pair<int, std::string>> buffer;
	std::vector<std::string> history;
	int historyCursor;
	int scrollCursor;
	TextField* inputLine;
	float timer;
	int appearState;

	std::ofstream hardcopy;
	std::string prediction;
	void predict();

public:
	std::vector<CVar> cvars;
	std::vector<CCmd> ccmds;
	bool visible;

	Console();
	//Attempts to execute user input. If the input is just a console
	//variable's name, its value will be displayed in response. If it's
	//a variable followed by a value of the appropriate type, it'll be
	//set. If it's a console command's name, that command will be
	//executed. If it's none of these, it's assumed to be a Lua command,
	//passed along to the Sol library.
	bool Execute(const std::string& str);
	//Appends the given string to the console log. For the in-game console,
	//the text is marked to be in the specific color. Internal line breaks
	//are allowed.
	void Print(int color, const std::string& str);
	//Appends the given string to the console log, in white.
	void Print(const std::string& str);
	//Flushes pending writes to the log file.
	void Flush();
	//Internal use. Handles character input. Passes the buck to the
	//internal TextField, then runs tab complete prediction.
	bool Character(unsigned int codepoint) override;
	//Internal use. Handles non-character input. Handles browsing the
	//command history, execution, and tab completion.
	bool Scancode(unsigned int scancode) override;
	//Opens the console with a little animation.
	void Open();
	//Closes the console with a little animation.
	void Close();
	bool Tick(float dt) override;
	void Draw(float dt) override;
	//Registers a console variable, mapping it by name to an arbitrary variable in the game.
	void RegisterCVar(const std::string& name, CVar::Type type, void* target, bool cheat = false, int min = -1, int max = -1);
	//Registers a console command, mapping it by name to a void(jsonArray&) function.
	void RegisterCCmd(const std::string& name, std::function<void(const jsonArray& args)> act);

	static bool CheckSplat(const std::string& pattern, const std::string& text);
};

extern Console* console;

#define conprint(C, F, ...) console->Print(C, fmt::format(F, __VA_ARGS__))
#ifdef DEBUG
#define debprint(C, F, ...) console->Print(C, fmt::format(F, __VA_ARGS__))
#else
#define debprint(C, F, ...)
#endif
