#pragma once
#include <vector>
#include <string>
#include "Types.h"
#include "../Game.h"

//Decodes a UTF-8 byte sequence to a codepoint, returns it and the size of the sequence.
extern std::tuple<rune, size_t> GetChar(const std::string& what, size_t where);
//Encodes a codepoint into a UTF-8 byte sequence and appends it to the given string.
extern void AppendChar(std::string& where, rune what);

//Like string::length() but counts UTF-8 characters, not bytes.
extern size_t Utf8CharLength(const std::string& what);

//Renders a set of tabular data to the console in a nice lined table.
extern void Table(std::vector<std::string> data, size_t stride);

//Given a piece of code (shader?) that may contain "#include" statements and a search path, inserts the included files, 
extern void HandleIncludes(std::string& code, const std::string& path);

//Splits a string into a vector of strings by the specified delimiter.
std::vector<std::string> Split(std::string& data, char delimiter);

//Changes a string's characters to lowercase, in place.
extern void StringToLower(std::string& data);
//Changes a string's characters to uppercase, in place.
extern void StringToUpper(std::string& data);

//Removes spaces from a string, in place.
extern void StripSpaces(std::string& data);

//Finds and replaces all instances of a thing in a string, in place.
extern void ReplaceAll(std::string& data, const std::string& find, const std::string& replace);

#ifndef BECKETT_NOBJTS
//Removes BJTS tags from a string.
extern std::string StripBJTS(const std::string& data);
//Applies all non-dynamic BJTS tags to a string.
extern std::string PreprocessBJTS(const std::string& data);
#endif

//Given a path that may contain ".." or "." parts, returns an absolute path ("foo/bar/../baz" becomes "foo/baz").
extern std::string ResolvePath(const std::string& maybeRelative);

template<typename T>
auto StringToEnum(const std::string& s, std::initializer_list<const std::string> opts)
{
	auto it = std::find(opts.begin(), opts.end(), s);
	if (it == opts.end())
		throw std::range_error(fmt::format("StringToEnum: can't find \"{}\" in list \"{}\".", s, StringJoin(opts.begin(), opts.end())));
	return (T)std::distance(opts.begin(), it);
}

template<typename InputIt>
std::string StringJoin(InputIt begin, InputIt end, const std::string& separator = ", ", const std::string& concluder = "")
{
	std::string ret;
	ret.reserve(256);
	if (begin != end)
		ret += *begin++;
	while (begin != end)
	{
		ret += separator;
		ret += *begin++;
	}
	ret += concluder;
	return ret;
}
