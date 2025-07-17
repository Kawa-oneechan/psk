#include "TextUtils.h"
#include "InputsMap.h"
#include "DialogueBox.h"

#ifdef _WIN32
extern "C"
{
	//Normally, CharUpper/CharLower take either strings *or* characters, by checking the high bytes or sumth.
	//It's a hack, but we don't *want* to use it on entire strings anyway cos we're UTF8.
	int __stdcall CharUpperW(_In_ int lpsz);
	int __stdcall CharLowerW(_In_ int lpsz);
	wchar_t* __stdcall CharNextW(_In_ const wchar_t* lpText);
}
#else
extern unsigned short caseFolding[2378];
#endif

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
#ifdef _WIN32
		ch = CharLowerW(ch);
#else
		for (int c = 0; c < 2378; c += 2)
		{
			if (caseFolding[c] == ch)
			{
				ch = caseFolding[c + 1];
				break;
			}
		}
#endif
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
#ifdef _WIN32
		ch = CharUpperW(ch);
#else
		for (int c = 1; c < 2378; c += 2)
		{
			if (caseFolding[c] == ch)
			{
				ch = caseFolding[c - 1];
				break;
			}
		}
#endif
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

static void bjtsStr(std::string& data, BJTSParams)
{
	if (tags.size() < 2)
	{
		conprint(2, "Missing parameter in BJTS Str: {}", data.substr(start, len));
		return;
	}

	auto speaker = dlgBox->Speaker();

	if (tags[1] == "...")
		data.replace(start, len, Text::Get("str:fix:001"));
	else if (tags[1] == "player")
		data.replace(start, len, thePlayer.Name);
	else if (tags[1] == "kun")
		data.replace(start, len, Text::Get("str:kun"));
	else if (tags[1] == "vname")
	{
		if (!speaker)
		{
			conprint(2, "BJTS Str called for villager name, but no speaker is set.");
			data.replace(start, len, "Buggsy");
			return;
		}
		data.replace(start, len, speaker->Name());
	}
	else if (tags[1] == "vspec")
	{
		if (!speaker)
		{
			conprint(2, "BJTS Str called for villager species, but no speaker is set.");
			data.replace(start, len, "bug");
			return;
		}
		data.replace(start, len, speaker->SpeciesName());
	}
	else if (tags[1] == "catchphrase")
	{
		if (!speaker)
		{
			conprint(2, "BJTS Str called for villager catchphrase, but no speaker is set.");
			data.replace(start, len, "bugbug");
			return;
		}
		data.replace(start, len, speaker->Catchphrase());
	}
}

static void bjtsEllipses(std::string& data, BJTSParams)
{
	tags;
	auto fakeTags = std::vector<std::string>
	{
		"str", "..."
	};
	bjtsStr(data, fakeTags, start, len);
}

static void bjtsWordstruct(std::string& data, BJTSParams)
{
	if (tags.size() < 2)
	{
		conprint(2, "Missing parameter in BJTS Wordstruct: {}", data.substr(start, len));
		return;
	}

	//Grab the entire thing, not just individual tags.
	//This method is cheaper than re-joining the tags.
	auto key = data.substr(start + 1, len - 2);

	auto speaker = dlgBox->Speaker();

	auto ppos = key.find('?');
	if (ppos != key.npos)
	{
		//Use speaker's personality, unless there
		//is no speaker in which case we use "normal".
		//But if the speaker's personality isn't an option,
		//reset and use "normal" after all.
		if (!speaker)
			key.replace(ppos, 1, "normal");
		else
		{
			key.replace(ppos, 1, speaker->personality->ID);
			//check if this is available
			auto result = Text::Get(fmt::format("{}:0", key));
			if (result.length() >= 3 && result.substr(0, 3) == "???")
			{
				//guess not :shrug:
				key = data.substr(start + 1, len - 2);
				key.replace(ppos, 1, "normal");
			}
		}
	}

	//Count the number of options
	int options = 0;
	for (int i = 0; i < 32; i++)
	{
		auto result = Text::Get(fmt::format("{}:{}", key, i));
		if (result.length() >= 3 && result.substr(0, 3) == "???")
		{
			options = i;
			break;
		}
	}
	if (options == 0)
	{
		conprint(2, "Wordstructor: could not find anything for \"{}\".", key);
		data.replace(start, len, "???WS???");
		return;
	}

	int choice = rnd::getInt(options - 1);
	data.replace(start, len, Text::Get(fmt::format("{}:{}", key, choice)));
}

static void bjtsKeyControl(std::string& data, BJTSParams)
{
	if (tags.size() < 2)
	{
		conprint(2, "Missing parameter in BJTS Key: {}", data.substr(start, len));
		return;
	}
	if (tags[1] == "arrows")
	{
		data.replace(start, len, fmt::format("{}/{}/{}/{}",
			Inputs.Keys[(int)Binds::WalkN].Name,
			Inputs.Keys[(int)Binds::WalkS].Name,
			Inputs.Keys[(int)Binds::WalkE].Name,
			Inputs.Keys[(int)Binds::WalkW].Name
		));
		return;
	}
	else  if (tags[1] == "updown")
	{
		data.replace(start, len, fmt::format("{}/{}",
			Inputs.Keys[(int)Binds::Up].Name,
			Inputs.Keys[(int)Binds::Down].Name
		));
		return;
	}
	else  if (tags[1] == "page")
	{
		data.replace(start, len, fmt::format("{}/{}",
			Inputs.Keys[(int)Binds::PageUp].Name,
			Inputs.Keys[(int)Binds::PageDown].Name
		));
		return;
	}
	for (int i = 0; i < NumKeyBinds; i++)
	{
		if (tags[1] == bindingNames[i])
		{
			data.replace(start, len, Inputs.Keys[i].Name);
			return;
		}
	}
}

static void bjtsGamepad(std::string& data, BJTSParams)
{
	if (tags.size() < 2)
	{
		conprint(2, "Missing parameter in BJTS Pad: {}", data.substr(start, len));
		return;
	}
	if (tags[1] == "dpad")
	{
		data.replace(start, len, fmt::format("{}/{}/{}/{}",
			GamepadPUAMap[Inputs.Keys[(int)Binds::WalkN].GamepadButton],
			GamepadPUAMap[Inputs.Keys[(int)Binds::WalkS].GamepadButton],
			GamepadPUAMap[Inputs.Keys[(int)Binds::WalkE].GamepadButton],
			GamepadPUAMap[Inputs.Keys[(int)Binds::WalkW].GamepadButton]
		));
		return;
	}
	else if (tags[1] == "updown")
	{
		data.replace(start, len, fmt::format("{}/{}",
			GamepadPUAMap[Inputs.Keys[(int)Binds::Up].GamepadButton],
			GamepadPUAMap[Inputs.Keys[(int)Binds::Down].GamepadButton]
		));
		return;
	}
	for (int i = 0; i < NumKeyBinds; i++)
	{
		if (tags[1] == bindingNames[i])
		{
			data.replace(start, len, GamepadPUAMap[Inputs.Keys[i].GamepadButton]);
			return;
		}
	}
}

typedef void(*BJTSFunc)(std::string& data, BJTSParams);

//BJTS functions that actually change the string content.
static const std::map<std::string, BJTSFunc> bjtsPhase1 = {
	{ "str", &bjtsStr },
	{ "...", &bjtsEllipses },
	{ "ws", &bjtsWordstruct },
	{ "key", &bjtsKeyControl },
	{ "pad", &bjtsGamepad },
};
//BJTS functions loaded from Lua scripts.
std::map<std::string, std::string> bjtsPhase1X;

std::string PreprocessBJTS(const std::string& data)
{
	if (bjtsPhase1X.empty())
	{
		auto extensions = VFS::ReadJSON("bjts/content.json");
		if (extensions)
		{
			for (auto extension : extensions->AsObject())
			{
				if (!extension.second->IsString())
				{
					conprint(2, "BJTS extension {} is not a string.", extension.first);
					continue;
				}
				auto val = extension.second->AsString();
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

std::string GetDirFromFile(const std::string& path)
{
	return path.substr(0, path.rfind('/') + 1);
}

extern "C" { const char* glfwGetKeyName(int key, int scancode); }
std::string GetKeyName(int scancode)
{
	if (scancode == 1 || scancode == 14 || scancode == 15 || scancode == 28 || scancode == 57 ||
		(scancode >= 71 && scancode <= 83) || scancode == 284 || scancode == 309)
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
