#include <string>
#include <cctype>
#include <format.h>
#include "engine/Types.h"
#include "engine/Text.h"
#include "engine/TextUtils.h"
#include "engine/VFS.h"
#include "Town.h"

int GetLetterScore(const std::string& text, bool noCapitals)
{
	//Based on https://www.youtube.com/watch?v=8VbwWVvw-zI

	auto trigrams = VFS::ReadString(fmt::format("mailcheck/trigrams_{}.txt", Text::GetLangCode()));
	int score = 0;

	rune ch;
	size_t size;

	auto find = [&](std::string haystack, rune pct, size_t pos)
	{
		rune ch2;
		size_t size2;
		while (pos < haystack.length())
		{
			std::tie(ch2, size2) = GetChar(haystack, pos);
			if (ch2 == pct)
				return pos;
			pos += size2;
		}
		return std::string::npos;
	};

	//Check A: punctuation and capital letters.
	{
		auto lastPos = text.length() - 1;
		while ((text[lastPos] & 0xC0) == 0x80)
			lastPos--;

		std::tie(ch, size) = GetChar(text, lastPos);

		if (ch == '.' || ch == '!' || ch == '?' || ch == 0x3002 || ch == 0xFF01 || ch == 0xFF0E || ch == 0xFF1F)
		{
			score += 20;
		}

		//Check this for correctness.

		auto checkFor = [&score, text, &find](rune pct)
		{
			size_t pos = find(text, pct, 0);
			while (pos != std::string::npos)
			{
				size_t size;
				if (pos != 0)
				{
					rune ch;
					std::tie(ch, size) = GetChar(text, pos);
					pos += size;
				}
				if (pos + 3 >= text.length())
					break;
				bool yea = false;
				size_t j = 0;
				for (int i = 0; i < 3; i++)
				{
					rune ch2;
					std::tie(ch2, size) = GetChar(text, pos + j);
					j += size;
					if (std::isupper(ch2))
					{
						score += 10;
						yea = true;
						break;
					}
				}
				if (!yea)
					score -= 10;
				pos = find(text, pct, pos);
				if (pos == 0)
					pos++;
			}
		};

		checkFor('.');
		checkFor('!');
		checkFor('?');
		checkFor(0x3002);
		checkFor(0xFF01);
		checkFor(0xFF0E);
		checkFor(0xFF1F);
	}

	//Check B: trigrams
	{
		size_t pos = 0;
		int trisFound = 0;

		std::tie(ch, size) = GetChar(text, 0);
		while (std::isblank(ch))
		{
			pos += size;
			std::tie(ch, size) = GetChar(text, pos);
		}
		size = 0;

		while (pos != std::string::npos)
		{
			if (pos != 0)
				pos++;
			if (pos + 3 >= text.length())
				break;

			//auto tri = text.substr(pos, 3);
			std::string tri{ "" };
			size_t poop;

			std::tie(ch, poop) = GetChar(text, pos);
			AppendChar(tri, ch);
			pos += poop;
			std::tie(ch, poop) = GetChar(text, pos);
			AppendChar(tri, ch);
			pos += poop;
			std::tie(ch, poop) = GetChar(text, pos);
			AppendChar(tri, ch);
			pos += poop;

			if (poop == 1)
				StringToLower(tri);

			auto triPos = trigrams.find(tri);
			if (triPos != std::string::npos)
				trisFound++;

			if (poop == 1)
			{
				pos = find(text, ' ', pos);
				if (pos != std::string::npos)
					pos++;
			}
		}
		score += trisFound * 3;
	}

	//Check C: first letter is a capital
	if (!noCapitals)
	{
		for (size_t i = 0; i < text.length(); i += size)
		{
			//auto c = text[i];
			std::tie(ch, size) = GetChar(text, i);
			if (std::isblank(ch))
			{
				continue;
			}
			if (std::isupper(ch))
				score += 20;
			else if (std::islower(ch))
				score -= 10;
			break;
		}
	}

	//Check D: repeated characters
	{
		for (size_t i = 0; i < text.length(); i += size)
		{
			//auto c = text[i];
			std::tie(ch, size) = GetChar(text, i);
			auto a = 0;
			rune ch2;
			size_t size2;
			for (size_t j = i + 1; j < text.length() && j < i + 3; j += size2, i += size2)
			{
				std::tie(ch2, size2) = GetChar(text, j);
				if (ch2 == ch)
					a++;
				else
					break;
			}
			if (a == 2)
			{
				score -= 50;
				break;
			}
		}
	}

	//Check E: space/non-space ratio
	{
		int spaces = 0;
		int nonspaces = 0;
		for (size_t i = 0; i < text.length(); i += size)
		{
			//auto c = text[i];
			std::tie(ch, size) = GetChar(text, i);
			if (std::isblank(ch))
				spaces++;
			else
				nonspaces++;
		}
		if (nonspaces == 0 || ((spaces * 100) / nonspaces < 20))
			score -= 20;
		else
			score += 20;
	}

	//Check F: no punctuation within 75 characters
	{
		if (text.length() >= 75)
		{
			auto first = text.substr(0, 75);
			//if (first.find('.') == std::string::npos && first.find('!') == std::string::npos && first.find('?') == std::string::npos)
			if (find(first, '.', 0) == std::string::npos &&
				find(first, '!', 0) == std::string::npos &&
				find(first, '?', 0) == std::string::npos &&
				find(first, 0x3002, 0) == std::string::npos &&
				find(first, 0xFF01, 0) == std::string::npos &&
				find(first, 0xFF0E, 0) == std::string::npos &&
				find(first, 0xFF1F, 0) == std::string::npos
				//y'know what? commas.
				&& find(first, ',', 0) == std::string::npos
				)
				score -= 150;
		}
	}

	//Check G: at least one space per 32 character cluster
	{
		size_t i = 0;
		while (i < text.length())
		{
			//size_t l = 32;
			//if (i + l > text.length())
			//	l = text.length() - i;
			//auto chunk = text.substr(i, l);
			std::string chunk{ "" };
			size_t pos = i;
			for (size_t j = 0; j < 32 && pos < text.length(); j++, pos += size)
			{
				std::tie(ch, size) = GetChar(text, pos);
				AppendChar(chunk, ch);
			}

			if (find(chunk, ' ', 0) == std::string::npos &&
				find(chunk, 0x3000, 0) == std::string::npos
				)
				score -= 20;

			i = pos;
		}
	}

	return score;
}

/*
int GetTownScore(const Town& town)
{
	//Read https://nookipedia.com/wiki/Environment_rating
}
*/

/*
int GetHHAScore(const House& house)
{
	//Should probably rewatch https://www.youtube.com/watch?v=A50i6xnGu7U
	//Read https://nookipedia.com/wiki/Happy_Home_Academy
	//Also read https://nookipedia.com/wiki/Feng_shui
}
*/
