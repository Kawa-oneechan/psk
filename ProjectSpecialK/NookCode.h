#pragma once
#include <string>
#include <array>

using hash = unsigned int;

namespace NookCode
{
	//Encode an arbitrary set of eight bytes to a NookCode.
	extern std::string Encode(std::array<unsigned char, 8>& d);
	//Encode an item hash, variant, and pattern to a NookCode.
	extern std::string Encode(hash itemHash, int variant, int pattern);
	//Decode a NookCode to an eight byte array.
	extern std::array<unsigned char, 8> Decode(const std::string& code);
	//Decode a NookCode to an item hash, variant, and pattern.\
	//Does not check if the hash refers to a valid item or character.
	extern void Decode(const std::string& code, hash& itemHash, int& variant, int& pattern);
}
