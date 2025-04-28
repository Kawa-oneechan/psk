/*
 * File JSON.cpp part of the SimpleJSON Library - http://mjpa.in/json
 *
 * Copyright (C) 2010 Mike Anchor
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "JSON.h"

/**
 * Blocks off the public constructor
 *
 * @access private
 *
 */
JSON::JSON()
{
}

/**
 * Parses a complete JSON encoded string (UNICODE input version)
 *
 * @access public
 *
 * @param char* data The JSON text
 *
 * @return JSONValue* Returns a JSON Value representing the root, or NULL on error
 */
JSONValue *JSON::Parse(const char *data)
{
	// Skip any preceding whitespace, end of data = no JSON = fail
	if (!SkipWhitespace(&data))
		return NULL;

	// We need the start of a value here now...
	JSONValue *value = JSONValue::Parse(&data);
	if (value == NULL)
		throw std::runtime_error("JSON value expected."); //return NULL;

	// Can be white space now and should be at the end of the string then...
	if (SkipWhitespace(&data))
	{
		delete value;
		return NULL;
	}

	// We're now at the end of the string
	return value;
}

#if WITHSTRINGIFY
/**
 * Turns the passed in JSONValue into a JSON encode string
 *
 * @access public
 *
 * @param JSONValue* value The root value
 *
 * @return std::string Returns a JSON encoded string representation of the given value
 */
std::string JSON::Stringify(const JSONValue *value)
{
	if (value != NULL)
		return value->Stringify(true);
	else
		return "";
}
#endif

/**
 * Skips over any whitespace characters (space, tab, \r or \n) defined by the JSON spec
 *
 * @access protected
 *
 * @param char** data Pointer to a char* that contains the JSON text
 *
 * @return bool Returns true if there is more data, or false if the end of the text was reached
 */
bool JSON::SkipWhitespace(const char **data)
{
	while (**data != 0 && (**data == L' ' || **data == L'\t' || **data == L'\r' || **data == L'\n'))
		(*data)++;

	//KAWA: add comment support
	if (**data == L'/')
	{
		(*data)++;
		if (**data == L'/')
		{
			(*data)++;
			//single line, skip ahead until the next line
			while (**data != 0 && (**data != L'\r' && **data != L'\n'))
				(*data)++;
			SkipWhitespace(data);
		}
		else if (**data == L'*')
		{
			(*data)++;
			while (**data != 0)
			{
				(*data)++;
				if (**data == L'*')
				{
					(*data)++;
					if (**data == L'/')
					{
						(*data)++;
						SkipWhitespace(data);
						break;
					}
				}
				else  if (**data == 0)
					throw std::runtime_error("JSON: Expected the end of a comment at some point.");
			}
		}
	}

	return **data != 0;
}

/**
 * Extracts a JSON String as defined by the spec - "<some chars>"
 * Any escaped characters are swapped out for their unescaped values
 *
 * @access protected
 *
 * @param char** data Pointer to a char* that contains the JSON text
 * @param std::string& str Reference to a std::string to receive the extracted string
 *
 * @return bool Returns true on success, false on failure
 */
bool JSON::ExtractString(const char **data, std::string &str)
{
	//str = "";
	str.clear();

	while (**data != 0)
	{
		// Save the char so we can change it if need be
		char next_char = **data;

		// Escaping something?
		if (next_char == '\\')
		{
			// Move over the escape char
			(*data)++;

			// Deal with the escaped char
			switch (**data)
			{
				case '"': next_char = '"'; break;
				case '\\': next_char = '\\'; break;
				case '/': next_char = '/'; break;
				case 'b': next_char = '\b'; break;
				case 'f': next_char = '\f'; break;
				case 'n': next_char = '\n'; break;
				case 'r': next_char = '\r'; break;
				case 't': next_char = '\t'; break;
				case 'u':
				{
					// We need 5 chars (4 hex + the 'u') or its not valid
					if (!simplejson_wcsnlen(*data, 5))
						return false;

					// Deal with the chars
					//KAWA: I want UTF-8 PLEASE
					long rune = 0;
					for (int i = 0; i < 4; i++)
					{
						// Do it first to move off the 'u' and leave us on the
						// final hex digit as we move on by one later on
						(*data)++;

						rune <<= 4;

						// Parse the hex digit
						if (**data >= '0' && **data <= '9')
							rune |= (**data - '0');
						else if (**data >= 'A' && **data <= 'F')
							rune |= (10 + (**data - 'A'));
						else if (**data >= 'a' && **data <= 'f')
							rune |= (10 + (**data - 'a'));
						else
						{
							// Invalid hex digit = invalid JSON
							return false;
						}
					}
					if (rune < 0x80)
						str += (char)rune;
					else if (rune < 0x0800)
					{
						str += (char)(((rune >> 6) & 0x1F) | 0xC0);
						str += (char)(((rune >> 0) & 0x3F) | 0x80);
					}
					else if (rune < 0x10000)
					{
						str += (char)(((rune >> 12) & 0x0F) | 0xE0);
						str += (char)(((rune >> 6) & 0x3F) | 0x80);
						str += (char)(((rune >> 0) & 0x3F) | 0x80);
					}
					next_char = 0;
					break;
				}

				// By the spec, only the above cases are allowed
				default:
					return false;
			}
		}

		// End of the string?
		else if (next_char == '"')
		{
			(*data)++;
			str.reserve(); // Remove unused capacity
			return true;
		}

		// Disallowed char?
		/*
		else if (next_char < ' ' && next_char != '\t')
		{
			// SPEC Violation: Allow tabs due to real world cases
			return false;
		}
		*/

		// Add the next char
		if (next_char != 0)
		str += next_char;

		// Move on
		(*data)++;
	}

	// If we're here, the string ended incorrectly
	return false;
}

/**
 * Parses some text as though it is an integer
 *
 * @access protected
 *
 * @param char** data Pointer to a char* that contains the JSON text
 *
 * @return double Returns the double value of the number found
 */
double JSON::ParseInt(const char **data)
{
	//KAWA: JSON5 detect and parse hex
	//VERY UGLY
	if (**data != 0 && **data == '0')
	{
		if (*(*data + 1) == 'x')
		{
			(*data) += 2;
			int integer = 0;
			while (**data != 0 && ((**data >= '0' && **data <= '9') || (**data >= 'A' && **data <= 'F') || (**data >= 'a' && **data <= 'f')))
			{
				if (**data >= '0' && **data <= '9')
					integer = integer * 16 + (*(*data)++ - '0');
				else if (**data >= 'A' && **data <= 'F')
					integer = integer * 16 + ((*(*data)++ - 'A') + 10);
				else if (**data >= 'a' && **data <= 'f')
					integer = integer * 16 + ((*(*data)++ - 'a') + 10);
			}
			return (double)integer;
		}
	}

	double integer = 0;
	while (**data != 0 && **data >= '0' && **data <= '9')
		integer = integer * 10 + (*(*data)++ - '0');

	return integer;
}

/**
 * Parses some text as though it is a decimal
 *
 * @access protected
 *
 * @param char** data Pointer to a char* that contains the JSON text
 *
 * @return double Returns the double value of the decimal found
 */
double JSON::ParseDecimal(const char **data)
{
	double decimal = 0.0;
  double factor = 0.1;
	while (**data != 0 && **data >= '0' && **data <= '9')
  {
    int digit = (*(*data)++ - '0');
		decimal = decimal + digit * factor;
    factor *= 0.1;
  }
	return decimal;
}
