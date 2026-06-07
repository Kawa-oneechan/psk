#pragma once
#include <vector>

//Returns the CRC32 hash for the given text.
extern hash GetCRC(const std::string& text);
//Returns the CRC32 hash for the given data.
extern hash GetCRC(unsigned char *buffer, int len);

constexpr extern hash operator ""_crc(const char* text, size_t size);
