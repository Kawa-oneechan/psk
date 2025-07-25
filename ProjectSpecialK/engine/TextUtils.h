#pragma once
#include <vector>
#include <string>
#include "Types.h"

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

//Removes BJTS tags from a string.
extern std::string StripBJTS(const std::string& data);
//Applies all non-dynamic BJTS tags to a string.
extern std::string PreprocessBJTS(const std::string& data);

//Given a full path to a file ("data/foo/bar.txt"), returns the path part including the final separator ("data/foo/").
extern std::string GetDirFromFile(const std::string& path);

template<typename T>
auto StringToEnum(const std::string& s, std::initializer_list<const std::string> opts)
{
	auto it = std::find(opts.begin(), opts.end(), s);
	if (it == opts.end())
		throw std::range_error(fmt::format("StringToEnum: can't find \"{}\" in list \"{}\".", s, join(opts.begin(), opts.end())));
	return (T)std::distance(opts.begin(), it);
}

template<typename InputIt>
std::string join(InputIt begin, InputIt end, const std::string& separator = ", ", const std::string& concluder = "")
{
	std::ostringstream ss;

	if (begin != end)
		ss << *begin++;

	while (begin != end)
	{
		ss << separator;
		ss << *begin++;
	}

	ss << concluder;
	return ss.str();
}
