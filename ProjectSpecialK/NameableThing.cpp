#include <format.h>
#include "NameableThing.h"
#include "engine/Text.h"
#include "engine/TextUtils.h"
#include "engine/Utilities.h"
#include "engine/Platform.h"
#include "engine/VFS.h"

//TODO: hack, clean this up
//Put it in TextUtils and make <info> a proper tag.
int articlePlease = 0;
int capitalizePlease = 0;

NameableThing::NameableThing(jsonObject& value, const std::string& filename) : ID(value["id"].as_string())
{
	Hash = GetCRC(ID);

	auto ref = fmt::format("name:{}", ID);
	StringToLower(ref);
	StripSpaces(ref);
	RefName = ref;

	//All NameableThings should be at least on folder deep.
	//Still, who knows.
	Path = VFS::GetPathPart(filename);
	File = VFS::GetFilePart(filename);

	auto val = value["name"];
	if (val.is_array())
	{
		//We may be trying to load a Species, which has masculine and feminine names.
		//Don't even bother setting EnName, as Species hides it with EnName[].
		return;
	}
	if (val.is_string() && val.as_string()[0] == '#')
		RefName = ref = val.as_string().substr(1);
	else
		Text::Add(ref, val);

	EnName = StripBJTS(Text::Get(ref));
}

const std::string NameableThing::Name() const
{
	auto text = Text::Get(RefName);
	if (text[0] == '<' && text.substr(0, 6) == "<info:")
	{
		auto bjtsEnd = text.find_first_of('>', 7);
		auto rest = text.substr(bjtsEnd + 1);
		if (articlePlease)
		{
			auto bjtsWhole = text.substr(7, bjtsEnd - 7);
			auto bjts = Split(bjtsWhole, ':');
			auto art = articlePlease - 1;
			articlePlease = 0;
			text = bjts[art] + rest;
		}
		else
			text = rest;
	}
	if (capitalizePlease)
	{
		rune ch;
		size_t size;
		std::tie(ch, size) = GetChar(text, 0);
		ch = Platform::CharUpper(ch);
		std::string newBit;
		AppendChar(newBit, ch);
		text.replace(0, 1, newBit);
		capitalizePlease = 0;
	}
	return text;
}
