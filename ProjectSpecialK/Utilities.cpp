//#include <regex>
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
		return glm::vec4(r, g, b, a);
	}
	throw std::runtime_error(fmt::format("GetJSONColor: {} is not a valid color.", val->Stringify()));
}

static glm::vec2 checkDate(glm::vec2 date)
{
	date[0] = (float)glm::clamp((int)date[0], 1, 31);
	date[1] = (float)glm::clamp((int)date[0], 1, 12);
	return date;
}
glm::vec2 GetJSONDate(JSONValue* val)
{
	if (val->IsArray())
		return checkDate(GetJSONVec2(val));
	if (val->IsString())
	{
		auto str = val->AsString();
		auto split = str.find_last_of(' ');
		if (split == str.npos)
			split = str.find_last_of('/');
		if (split == str.npos)
			throw std::runtime_error(fmt::format("GetJSONDate: value {} can't split on space or slash.", val->Stringify()));
		auto day = std::stoi(str.substr(split + 1));
		auto mon = str.substr(0, 3);
		StringToLower(mon);
		static const std::string names[] = { "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec" };
		for (int i = 0; i < 12; i++)
		{
			if (names[i] == mon)
				return checkDate(glm::vec2(day, i + 1));
		}
	}
	throw std::runtime_error(fmt::format("GetJSONDate: value {} is not a month/day pair.", val->Stringify()));
}

void GetAtlas(std::vector<glm::vec4> &ret, const std::string& jsonFile)
{
	auto rjs = VFS::ReadJSON(jsonFile);
	if (!rjs)
		return;
	auto doc = rjs->AsObject();
	ret.clear();
	if (doc["type"] == nullptr)
	{
		delete rjs;
		return;
	}

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

std::string LoadCamera(JSONValue* json)
{
	std::string result = "";
	try
	{
		if (json == nullptr)
			result = "no data.";
		else if (!json->IsObject())
			result = "not an object.";
		else
		{
			JSONObject& obj = (JSONObject&)json->AsObject();
			if (obj["target"] == nullptr || obj["angles"] == nullptr || obj["distance"] == nullptr)
				result = "not all required camera properties accounted for.";
			else
			{
				MainCamera.Target(GetJSONVec3(obj["target"]));
				MainCamera.Angles(GetJSONVec3(obj["angles"]));
				if (!obj["distance"]->IsNumber())
					result = "distance is not a number.";
				else
					MainCamera.Distance(obj["distance"]->AsNumber());
				if (obj["drum"] != nullptr && obj["drum"]->IsBool())
					MainCamera.Drum = obj["drum"]->AsBool();
				if (obj["drumAmount"] != nullptr && obj["drumAmount"]->IsNumber())
					MainCamera.DrumAmount = obj["drumAmount"]->AsNumber();
				if (obj["drumPower"] != nullptr && obj["drumPower"]->IsNumber())
					MainCamera.DrumPower = obj["drumPower"]->AsNumber();
				//if (obj["locked"] != nullptr && obj["locked"]->IsBool())
				//	MainCamera.Locked = obj["locked"]->AsBool();
			}
		}
	}
	catch (std::runtime_error& x)
	{
		result = x.what();
	}
	if (!result.empty())
		conprint(1, "Could not load camera setup: {}", result);
	return result;
}

std::string LoadCamera(const std::string& path)
{
	std::string result = "";
	try
	{
		auto json = VFS::ReadJSON(path);
		if (json == nullptr)
			result = "no data.";
		else
			LoadCamera(json);
	}
	catch (std::runtime_error& x)
	{
		result = x.what();
	}
	return result;
}

std::string LoadLights(JSONValue* json)
{
	std::string result = "";
	try
	{
		if (json == nullptr)
			result = "no data.";
		else if (!json->IsArray())
			result = "not an array.";
		else
		{
			auto i = 0;
			for (auto lobj : json->AsArray())
			{
				if (!lobj->IsObject())
					result = "not an object.";
				else
				{
					JSONObject& l = (JSONObject&)lobj->AsObject();
					if (l["pos"] == nullptr || l["col"] == nullptr)
						result = "not all light properties accounted for.";
					else
					{
						lightPos[i] = GetJSONVec4(l["pos"]);
						lightCol[i] = GetJSONColor(l["col"]);
					}
					i++;
					if (i == MaxLights)
						break;
				}
			}
		}
	}
	catch (std::runtime_error& x)
	{
		result = x.what();
	}
	if (!result.empty())
		conprint(1, "Could not load lighting setup: {}", result);
	return result;
}

std::string LoadLights(const std::string& path)
{
	std::string result = "";
	try{
		auto json = VFS::ReadJSON(path);
		if (json == nullptr)
			result = "no data.";
		else
			LoadLights(json);
	}
	catch (std::runtime_error& x)
	{
		result = x.what();
	}
	return result;
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
			auto here = cel.length();
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
			line += fmt::format(fmt::format(u8"│ {{:{}}} ", width[col]), cel);
		}
		line += u8"│";
		conprint(7, line);
		if (row == 0)
			conprint(7, fmt::format(u8"├{}┤", middle));
	}

	conprint(7, u8"└{}┘", bottom);
}

static tm _tm;
tm* GetNthWeekdayOfMonth(int month, int dayOfWeek, int howManyth)
{
	//C++17 doesn't have chrono calendar stuff yet, that's C++20.
	//So instead we're figuring this out the hard way.
	tm rightNow;
	time_t yesNow = std::time(nullptr);
	gmtime_s(&rightNow, &yesNow);
	int thisYear = rightNow.tm_year;

	tm novemberFirst = { 0 };
	novemberFirst.tm_year = thisYear;
	novemberFirst.tm_mon = month;
	novemberFirst.tm_mday = 1;
	novemberFirst.tm_hour = 12;
	std::mktime(&novemberFirst);
	time_t t = std::mktime(&novemberFirst);
	while (true)
	{
		tm here;
		gmtime_s(&here, &t);
		if (here.tm_wday == dayOfWeek)
		{
			t += ((60 * 60 * 24) * 7) * (howManyth - 1); //add a whole week for the amount of thursdays we want
			if (novemberFirst.tm_wday == dayOfWeek)
				t += (60 * 60 * 24) * 7; //add one more week if we *started* on a thursday
			break;
		}
		t += (60 * 60 * 24); //add one day
	}
	gmtime_s(&_tm, &t);
	return &_tm;
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
	while (true)
	{
		auto incPos = code.find("#include \"");
		if (incPos == std::string::npos)
			break;
		auto incStr = incPos + 10;
		auto incEnd = code.find('\"', incStr);
		auto file = code.substr(incStr, incEnd - incStr);
		auto includedFile = VFS::ReadString(path + file);
		code = code.replace(incPos, incEnd - incPos + 1, includedFile);
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

namespace NookCode
{
	static const char alphabet[]{ "ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#&+-" };

	static inline unsigned char reverseBits(unsigned char b)
	{
		return unsigned char((b * 0x0202020202ULL & 0x010884422010ULL) % 1023);
	}

	static inline unsigned char rotateLeft(unsigned char b, int i)
	{
		while (i > 0)
		{
			unsigned char m = (b & 0x80) ? 1 : 0;
			b <<= 1;
			b |= m;
			i--;
		}
		return b;
	}

	static inline unsigned char rotateRight(unsigned char b, int i)
	{
		while (i > 0)
		{
			unsigned char m = (b & 0x01) ? 1 : 0;
			b >>= 1;
			b |= m << 7;
			i--;
		}
		return b;
	}

	static inline void swap(unsigned char* a, unsigned char* b)
	{
		auto c = *a;
		*a = *b;
		*b = c;
	}

	std::string Encode(std::array<unsigned char, 8>& d)
	{
		for (int i = 0; i < 5; i++)
			d[5] += d[i];

		for (int i = 0; i < 6; i++)
			d[i] = reverseBits(d[i]);

		swap(&d[0], &d[1]); //-V525 yes yes I know
		swap(&d[2], &d[3]);
		swap(&d[2], &d[4]);
		swap(&d[0], &d[3]);

		for (int i = 0; i < 6; i++)
			d[i] = rotateLeft(d[i], i + 1);

		auto v = *(unsigned long long*)&d;
		auto pv = v;
		pv ^= pv >> 1;
		pv ^= pv >> 2;
		pv = (pv & 0x1111111111111111UL) * 0x1111111111111111UL;
		unsigned char parity = (pv >> 60) & 1;

		unsigned char c[10]{ 0 };
		for (int i = 0; i < 10; i++)
		{
			c[i] = v & 31;
			v >>= 5;
		}
		c[9] |= parity << 4;

		std::string ret{ ".........." };
		for (int i = 0; i < 10; i++)
			ret[i] = alphabet[c[i]];

		return ret;
	}

	std::string Encode(hash itemHash, int variant, int pattern)
	{
		auto d = std::array<unsigned char, 8>();
		d[0] = (itemHash >> 0) & 0xFF;
		d[1] = (itemHash >> 8) & 0xFF;
		d[2] = (itemHash >> 16) & 0xFF;
		d[3] = (itemHash >> 24) & 0xFF;
		d[4] = (unsigned char)(variant << 4) | (pattern & 0xF);
		return Encode(d);
	}

	std::array<unsigned char, 8> Decode(const std::string& code)
	{
		std::array<unsigned char, 8> d;

		unsigned char c[10]{ 0 };
		for (int i = 0; i < 10; i++)
		{
			auto pos = strchr(alphabet, code[i]);
			if (pos == nullptr)
				throw std::runtime_error("Invalid character in NookCode.");
			c[i] = (unsigned char)(pos - alphabet);
		}

		//unsigned char parity = c[9] & 16;
		c[9] &= 7;

		auto v = 0ULL;
		for (int i = 0; i < 10; i++)
			v |= ((unsigned long long)c[i]) << (5 * i);

		for (int i = 0; i < 8; i++)
		{
			d[i] = v & 0xFF;
			v >>= 8;
		}

		for (int i = 0; i < 8; i++)
			d[i] = rotateRight(d[i], i + 1);

		swap(&d[0], &d[3]); //-V525
		swap(&d[2], &d[4]);
		swap(&d[2], &d[3]);
		swap(&d[0], &d[1]);

		for (int i = 0; i < 6; i++)
			d[i] = reverseBits(d[i]);

		unsigned char check = 0;
		for (int i = 0; i < 5; i++)
			check += d[i];
		if (check != d[5])
			throw std::runtime_error("NookCode checksum failed.");

		return d;
	}

	void Decode(const std::string& code, hash& itemHash, int& variant, int& pattern)
	{
		auto d = Decode(code);
		itemHash = d[0];
		itemHash |= d[1] << 8;
		itemHash |= d[2] << 16;
		itemHash |= d[3] << 24;
		variant = (d[4] >> 4) & 0xF;
		pattern = d[4] & 0xF;
	}
}

extern unsigned int crcLut[256];

hash GetCRC(const std::string& text)
{
	unsigned int crc = 0xFFFFFFFFL;

	for (auto c : text)
		crc = (crc >> 8) ^ crcLut[c ^ crc & 0xFF];

	return crc ^ 0xFFFFFFFFL;
}

hash GetCRC(unsigned char *buffer, int len)
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
