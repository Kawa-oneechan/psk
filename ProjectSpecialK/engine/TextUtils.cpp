#include <ctype.h>
#include <iostream>
#include <sstream>
#include <string>
#include <sol.hpp>
#include "VFS.h"
#include "Platform.h"
#include "TextUtils.h"
#include "InputsMap.h"
#include "Console.h"
#include "../Game.h"

extern sol::state Sol;

static const char* bindingNames[] = {
	"up", "down", "left", "right",
	"accept", "back", "pageup", "pagedown",
	"walkn", "walkw", "walks", "walke",
	"interact", "pickup",
	"cameracw", "cameraccw", "cameraup", "cameradown",
	"inventory", "map", "react",
	"hotbar1", "hotbar2", "hotbar3", "hotbar4", "hotbar5",
	"hotbar6", "hotbar7", "hotbar8", "hotbar9", "hotbar10",
	"console"
};

std::tuple<rune, size_t> GetChar(const std::string& what, size_t where)
{
	if (where >= what.size())
		return{ 0, 0 };
	rune ch = what[where] & 0xFF;
	size_t size = 1;
	if ((ch & 0xE0) == 0xC0)
		size = 2;
	else if ((ch & 0xF0) == 0xE0)
		size = 3;
	if (where + size > what.length())
		throw std::exception("Broken UTF-8 sequence.");
	if (size == 2)
	{
		ch = (ch & 0x1F) << 6;
		ch |= (what[where + 1] & 0x3F);
	}
	else if (size == 3)
	{
		ch = (ch & 0x1F) << 12;
		ch |= (what[where + 1] & 0x3F) << 6;
		ch |= (what[where + 2] & 0x3F);
	}
	return{ ch, size };
}

void AppendChar(std::string& where, rune what)
{
	if (what < 0x80)
	where += (char)what;
	else if (what < 0x0800)
	{
		where += (char)(((what >> 6) & 0x1F) | 0xC0);
		where += (char)(((what >> 0) & 0x3F) | 0x80);
	}
	else if (what < 0x10000)
	{
		where += (char)(((what >> 12) & 0x0F) | 0xE0);
		where += (char)(((what >> 6) & 0x3F) | 0x80);
		where += (char)(((what >> 0) & 0x3F) | 0x80);
	}
}

size_t Utf8CharLength(const std::string& what)
{
	rune ch;
	size_t size;
	size_t ret = 0;
	for (size_t i = 0; i < what.length();)
	{
		std::tie(ch, size) = GetChar(what, i);
		i += size;
		ret++;
	}
	return ret;
}

void Table(std::vector<std::string> data, size_t stride)
{
	size_t width[64] = { 0 };
	auto rows = data.size() / stride;
	for (auto col = 0; col < stride; col++)
	{
		for (auto row = 0; row < rows; row++)
		{
			const auto& cel = data[row * stride + col];
			auto here = Utf8CharLength(cel);
			if (here > width[col])
				width[col] = here;
		}
	}

	std::string top;
	std::string middle;
	std::string bottom;
	for (auto col = 0; col < stride; col++)
	{
		for (auto i = 0; i < width[col] + 2; i++)
		{
			top += u8"─";
			middle += u8"─";
			bottom += u8"─";
		}
		if (col < stride - 1)
		{
			top += u8"┬";
			middle += u8"┼";
			bottom += u8"┴";
		}
	}

	conprint(7, u8"┌{}┐", top);

	for (auto row = 0; row < rows; row++)
	{
		std::string line;
		for (auto col = 0; col < stride; col++)
		{
			const auto& cel = data[row * stride + col];
#if 0
			line += fmt::format(u8"│ {:{}} ", cel, width[col]);
#else
			//More expensive, but handles Ismène.
			auto celLen = Utf8CharLength(cel);
			auto padding = width[col] - celLen;
			line += fmt::format(u8"│ {}{:{}} ", cel, "", padding);
#endif
		}
		line += u8"│";
		conprint(7, line);
		if (row == 0)
			conprint(7, fmt::format(u8"├{}┤", middle));
	}

	conprint(7, u8"└{}┘", bottom);
}

void StringToLower(std::string& data)
{
	std::string ret;
	ret.reserve(data.length());
	for (size_t i = 0; i < data.length();)
	{
		rune ch;
		size_t size;
		std::tie(ch, size) = GetChar(data, i);
		AppendChar(ret, Platform::CharLower(ch));
		i += size;
	}
	data = ret;
}

void StringToUpper(std::string& data)
{
	std::string ret;
	ret.reserve(data.length());
	for (size_t i = 0; i < data.length();)
	{
		rune ch;
		size_t size;
		std::tie(ch, size) = GetChar(data, i);
		AppendChar(ret, Platform::CharUpper(ch));
		i += size;
	}
	data = ret;
}

void StripSpaces(std::string& data)
{
	while (data.find(' ') != -1)
		data.erase(std::find(data.begin(), data.end(), ' '));
}

void ReplaceAll(std::string& data, const std::string& find, const std::string& replace)
{
	size_t pos = 0;
	size_t fl = find.length();
	size_t rl = replace.length();
	while (true)
	{
		pos = data.find(find, pos);
		if (pos == std::string::npos)
			break;
		data.replace(pos, fl, replace);
		pos += rl;
	}
}

#ifndef BECKETT_NOBJTS
std::string StripBJTS(const std::string& data)
{
	std::string ret = data;
	size_t bjtsStart;
	while ((bjtsStart = ret.find_first_of('<', 0)) != std::string::npos)
	{
		auto bjtsEnd = ret.find_first_of('>', bjtsStart);
		ret.replace(bjtsStart, bjtsEnd - bjtsStart + 1, "");
	}
	return ret;
}

std::string PreprocessBJTS(const std::string& data)
{
	if (bjtsPhase1X.empty())
	{
		auto extensions = VFS::ReadJSON("bjts/content.json");
		if (extensions)
		{
			for (auto extension : extensions.as_object())
			{
				if (!extension.second.is_string())
				{
					conprint(2, "BJTS extension {} is not a string.", extension.first);
					continue;
				}
				auto val = extension.second.as_string();
				if (val.length() < 4 || val.substr(val.length() - 4) != ".lua")
				{
					//This is a raw string. Convert it to a Lua thing.
					//And for that, we need to escape quotes!
					ReplaceAll(val, "\"", "\\\"");
					//Possibly other things to but IDCRN.

					val = fmt::format("return \"{}\"\r\n", val);
				}
				else
				{
					val = VFS::ReadString(fmt::format("bjts/{}", val));
				}
				bjtsPhase1X[extension.first] = val;
			}
		}
	}

	auto ret = std::string(data);
	for (size_t i = 0; i < ret.length(); i++)
	{
		auto bjtsStart = ret.find_first_of('<', i);
		if (bjtsStart != std::string::npos)
		{
			bjtsStart++;
			auto bjtsEnd = ret.find_first_of('>', bjtsStart);
			i = bjtsEnd;

			auto bjtsWhole = ret.substr(bjtsStart, bjtsEnd - bjtsStart);
			auto bjts = Split(bjtsWhole, ':');
			auto func = bjtsPhase1.find(bjts[0]);
			auto start = (int)bjtsStart - 1;
			auto len = (int)(bjtsEnd - bjtsStart) + 2;
			if (func != bjtsPhase1.end())
			{
				std::invoke(func->second, ret, bjts, start, len);
				//std::invoke(func->second, bjts, (int)bjtsStart - 1, (int)(bjtsEnd - bjtsStart) + 2);
				i = (size_t)-1; //-1 because we may have subbed in a new tag.
			}
			else
			{
				//Is it an extension?
				auto func2 = bjtsPhase1X.find(bjts[0]);
				if (func2 != bjtsPhase1X.end())
				{
					Sol.set("bjts", bjts);
					ret.replace(start, len, Sol.script(func2->second).get<std::string>());
					i = bjtsStart;
				}
			}
		}
		else
			break;
	}
	return ret;
}
#endif

std::vector<std::string> Split(std::string& data, char delimiter)
{
	std::vector<std::string> ret;
	std::string part;
	std::istringstream stream(data);
	while (std::getline(stream, part, delimiter))
		ret.push_back(part);
	return ret;
}

void HandleIncludes(std::string& code, const std::string& path)
{
	while (true)
	{
		auto incPos = code.find("#include \"");
		if (incPos == std::string::npos)
			break;
		auto incStr = incPos + 10;
		auto incEnd = code.find('\"', incStr);
		auto file = code.substr(incStr, incEnd - incStr);
		auto includedFile = VFS::ReadString(fmt::format("{}/{}", path, file));
		code = code.replace(incPos, incEnd - incPos + 1, includedFile);
	}
}

std::string ResolvePath(const std::string& maybeRelative)
{
	if (maybeRelative.find("..") == std::string::npos)
		return maybeRelative;
	auto parts = Split((std::string&)maybeRelative, '/');
	for (int i = 0; i < parts.size(); i++)
	{
		if (parts[i] == "..")
		{
			if (i == 0)
				throw std::exception("Can't go up from the root.");

			//erase both the .. and the previous bit
			parts.erase(parts.begin() + i - 1);
			parts.erase(parts.begin() + i - 1);

			//start over
			i = 0;
		}
		else if (parts[i] == ".")
		{
			//erase just this part
			parts.erase(parts.begin() + i);
			i = 0;
		}
	}
	return StringJoin(parts.begin(), parts.end(), "/");
}
