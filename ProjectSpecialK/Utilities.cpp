﻿#include <regex>
#include "SpecialK.h"

glm::vec2 GetJSONVec2(JSONValue* val)
{
	if (!val->IsArray())
		throw std::runtime_error(fmt::format("GetJSONVec2: given value {} is not an array.", val->Stringify()));
	auto arr = val->AsArray();
	if (arr.size() != 2)
		throw std::runtime_error(fmt::format("GetJSONVec2: given array {} has {} entries, not 2.", val->Stringify(), arr.size()));
	if (std::any_of(arr.cbegin(), arr.cend(), [](JSONValue* x) { return !x->IsNumber(); }))
		throw std::runtime_error(fmt::format("GetJSONVec2: given array {} does not contain only numbers.", val->Stringify()));
	return glm::vec2(arr[0]->AsNumber(), arr[1]->AsNumber());
}

glm::vec3 GetJSONVec3(JSONValue* val)
{
	if (!val->IsArray())
		throw std::runtime_error("GetJSONVec3: given value is not an array.");
	auto arr = val->AsArray();
	if (arr.size() != 3)
		throw std::runtime_error(fmt::format("GetJSONVec3: given array has {} entries, not 3.", arr.size()));
	if (std::any_of(arr.cbegin(), arr.cend(), [](JSONValue* x) { return !x->IsNumber(); }))
		throw std::runtime_error("GetJSONVec3: given array does not contain only numbers.");
	return glm::vec3(arr[0]->AsNumber(), arr[1]->AsNumber(), arr[2]->AsNumber());
}

glm::vec4 GetJSONVec4(JSONValue* val)
{
	if (!val->IsArray())
		throw std::runtime_error("GetJSONVec4: given value is not an array.");
	auto arr = val->AsArray();
	if (arr.size() != 4)
		throw std::runtime_error(fmt::format("GetJSONVec4: given array has {} entries, not 4.", arr.size()));
	if (std::any_of(arr.cbegin(), arr.cend(), [](JSONValue* x) { return !x->IsNumber(); }))
		throw std::runtime_error("GetJSONVec4: given array does not contain only numbers.");
	return glm::vec4(arr[0]->AsNumber(), arr[1]->AsNumber(), arr[2]->AsNumber(), arr[3]->AsNumber());
}

glm::vec4 GetJSONColor(JSONValue* val)
{
	if (val->IsString())
	{
		auto hex = val->AsString();
		int r = 0, g = 0, b = 0, a = 0;
		if (hex.empty() || hex[0] != '#')
			return glm::vec4(0, 0, 0, -1);
		//TODO: consider checking if the value is in UI::themeColors.
		//That way, colors in panel definitions can be hexcodes or float arrays too.
		if (hex.length() == 7)
		{
			a = 0xFF;
			r = std::stoi(hex.substr(1, 2), nullptr, 16);
			g = std::stoi(hex.substr(3, 2), nullptr, 16);
			b = std::stoi(hex.substr(5, 2), nullptr, 16);
		}
		else if (hex.length() == 9)
		{
			a = std::stoi(hex.substr(1, 2), nullptr, 16);
			r = std::stoi(hex.substr(3, 2), nullptr, 16);
			g = std::stoi(hex.substr(5, 2), nullptr, 16);
			b = std::stoi(hex.substr(7, 2), nullptr, 16);
		}
		else
			throw std::runtime_error(fmt::format("GetJSONColor: {} is not a valid color.", val->Stringify()));
		return glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
	}
	if (val->IsArray())
	{
		auto arr = val->AsArray();
		for (auto x : arr)
			if (!x->IsNumber())
				throw std::runtime_error(fmt::format("GetJSONColor: {} is not a valid color.", val->Stringify()));
		float r, g, b, a;
		if (arr.size() == 3)
		{
			r = arr[0]->AsNumber();
			g = arr[1]->AsNumber();
			b = arr[2]->AsNumber();
			a = 1.0f;
		}
		else if (arr.size() == 4)
		{
			r = arr[0]->AsNumber();
			g = arr[1]->AsNumber();
			b = arr[2]->AsNumber();
			a = arr[3]->AsNumber();
		}
		else
			throw std::runtime_error(fmt::format("GetJSONColor: {} is not a valid color.", val->Stringify()));
		return glm::vec4(arr[0]->AsNumber(), arr[1]->AsNumber(), arr[2]->AsNumber(), arr[3]->AsNumber());
	}
	throw std::runtime_error(fmt::format("GetJSONColor: {} is not a valid color.", val->Stringify()));
}

void GetAtlas(std::vector<glm::vec4> &ret, const std::string& jsonFile)
{
	auto rjs = VFS::ReadJSON(jsonFile);
	if (!rjs)
		return;
	auto doc = rjs->AsObject();
	ret.clear();
	if (doc["type"]->AsString() == "simple")
	{
		auto size = GetJSONVec2(doc["size"]);
		auto dims = GetJSONVec2(doc["dims"]);
		for (int y = 0; y < (int)dims[1]; y++)
		{
			for (int x = 0; x < (int)dims[0]; x++)
			{
				ret.push_back(glm::vec4(x * size[0], y * size[1], size[0], size[1]));
			}
		}
	}
	else if (doc["type"]->AsString() == "atlas")
	{
		auto rects = doc["rects"]->AsArray();
		for (const auto& rect : rects)
		{
			ret.push_back(GetJSONVec4(rect));
		}
	}
	else
		throw std::runtime_error(fmt::format("GetAtlas: file {} has an unknown type \"{}\".", jsonFile, doc["type"]->AsString()));
	delete rjs;
}

bool PointInPoly(const glm::vec2 point, const std::vector<glm::vec2>& polygon)
{
	int crossings = 0;
	const auto numPts = polygon.size() - 1;

	for (auto i = 0; i < numPts; i++)
	{
		if (((polygon[i].y <= point.y) && (polygon[i + 1].y > point.y))
			|| ((polygon[i].y > point.y) && (polygon[i + 1].y <= point.y)))
		{
			auto vt = (point.y - polygon[i].y) / (polygon[i + 1].y - polygon[i].y);
			if (point.x < polygon[i].x + vt * (polygon[i + 1].x - polygon[i].x))
			{
				++crossings;
			}
		}
	}
	return (crossings & 1) == 1;
}

bool PointInRect(const glm::vec2 point, const glm::vec4 rect)
{
	return
		(point.x >= rect.x) &&
		(point.x < rect.x + rect.z) &&
		(point.y >= rect.y) &&
		(point.y < rect.y + rect.w);
}

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

void Table(std::vector<std::string> data, size_t stride)
{
	size_t width[64] = { 0 };
	auto rows = data.size() / stride;
	for (auto col = 0; col < stride; col++)
	{
		for (auto row = 0; row < rows; row++)
		{
			const auto& cel = data[row * stride + col];
			if (cel.length() > width[col])
				width[col] = cel.length();
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
			line += fmt::format(fmt::format(u8"│ {{:{}}} ", width[col]), cel);
		}
		line += u8"│";
		conprint(7, line);
		if (row == 0)
			conprint(7, fmt::format(u8"├{}┤", middle));
	}

	conprint(7, u8"└{}┘", bottom);
}

std::string GetDirFromFile(const std::string& path)
{
	return path.substr(0, path.rfind('/') + 1);
}

extern "C" { const char* glfwGetKeyName(int key, int scancode); }
std::string GetKeyName(int scancode)
{
	if (scancode == 1 || scancode == 14 || scancode == 15 || scancode == 28 || scancode == 57 ||
		(scancode >= 71 && scancode <=83) || scancode == 284 || scancode == 309)
		return Text::Get(fmt::format("keys:scan:{}", scancode));

	auto glfw = glfwGetKeyName(-1, scancode);
	if (glfw[0] == '\0')
		return Text::Get(fmt::format("keys:scan:{}", scancode));
	else
		return std::string(glfw);
}

bool IsID(const std::string& id)
{
	//valid IDs may only contain alphanumerics, :, and _.
	for (auto& c : id)
	{
		if (!(std::isalnum(c) || c == ':' || c == '_'))
			return false;
	}
	return true;
}

bool IDIsQualified(const std::string& id)
{
	//must have a : but not as the first character.
	return id.find(':') != std::string::npos && id[0] != ':';
}

std::string Qualify(const std::string& id, const std::string& ns)
{
	//if (id.substr(0, ns.length()) == ns)
	//	throw std::runtime_error(fmt::format("Qualify: cannot double-qualify \"{}\", already starts with \"{}\".", id, ns));
	return ns + ':' + id;
}

std::string UnQualify(const std::string& id)
{
	if (IDIsQualified(id))
		return id.substr(id.find(':') + 1);
	return id;
}

extern unsigned short caseFolding[2378];

void StringToLower(std::string& data)
{
	//std::transform(data.begin(), data.end(), data.begin(), [](unsigned char c) { return tolower(c); });
	std::string ret;
	ret.reserve(data.length());
	for (size_t i = 0; i < data.length();)
	{
		rune ch;
		size_t size;
		std::tie(ch, size) = GetChar(data, i);
		for (int c = 0; c < 2378; c += 2)
		{
			if (caseFolding[c] == ch)
			{
				ch = caseFolding[c + 1];
				break;
			}
		}
		AppendChar(ret, ch);
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
		for (int c = 1; c < 2378; c += 2)
		{
			if (caseFolding[c] == ch)
			{
				ch = caseFolding[c - 1];
				break;
			}
		}
		AppendChar(ret, ch);
		i += size;
	}
	data = ret;
}

void StripSpaces(std::string& data)
{
	while (data.find(' ') != -1)
		data.erase(std::find(data.begin(), data.end(), ' '));
}

std::string StripMSBT(const std::string& data)
{
	std::string ret = data;
	size_t msbtStart;
	while ((msbtStart = ret.find_first_of('<', 0)) != std::string::npos)
	{
		auto msbtEnd = ret.find_first_of('>', msbtStart);
		ret.replace(msbtStart, msbtEnd - msbtStart + 1, "");
	}
	return ret;
}

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
	static const std::regex incReg("#include\\s+?\"(.*?)\"");
	std::smatch incMatch;
	while (std::regex_search(code, incMatch, incReg))
	{
		if (incMatch.size() == 2)
		{
			auto includedFile = VFS::ReadString(path + incMatch[1].str());
			code = std::regex_replace(code, incReg, includedFile);
		}
	}
}

#include "support/scale2x/scalebit.h"
unsigned char* ScaleImage(unsigned char* original, int origWidth, int origHeight, int channels, int targetScale)
{
	if (targetScale < 2 || targetScale > 4)
		throw std::invalid_argument(fmt::format("ScaleImage: targetScale must be 2, 3, or 4, but was {}.", targetScale));

	int newWidth = origWidth * targetScale;
	int newHeight = origHeight * targetScale;

	auto target = new unsigned char[(newWidth * newHeight) * channels];
	scalebit(targetScale, target, newWidth  * channels, original, origWidth * channels, channels, origWidth, origHeight, 0);

	return target;
}

extern unsigned int crcLut[256];

unsigned int GetCRC(const std::string& text)
{
	unsigned int crc = 0xFFFFFFFFL;

	for (auto c : text)
		crc = (crc >> 8) ^ crcLut[c ^ crc & 0xFF];

	return crc ^ 0xFFFFFFFFL;
}

unsigned int GetCRC(unsigned char *buffer, int len)
{
	unsigned int crc = 0xFFFFFFFFL;

	for (auto i = 0; i < len; i++)
		crc = (crc >> 8) ^ crcLut[buffer[i] ^ crc & 0xFF];

	return crc ^ 0xFFFFFFFFL;
}

extern "C"
{
	unsigned long mz_crc32(unsigned long start, const unsigned char *ptr, size_t buf_len)
	{
		unsigned int crc = start ^ 0xFFFFFFFFL;

		for (auto i = 0; i < buf_len; i++)
			crc = (crc >> 8) ^ crcLut[ptr[i] ^ crc & 0xFF];

		return crc ^ 0xFFFFFFFFL;
	}
}
