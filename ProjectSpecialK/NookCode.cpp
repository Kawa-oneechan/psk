#include <string>
#include <array>
#include "Types.h"

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

		int swaps[]{ 0, 1, 2, 3, 2, 4, 0, 3 };
		for (int i = 0; i < 8; i += 2)
			swap(&d[swaps[i]], &d[swaps[i + 1]]);

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

		int swaps[]{ 0, 3, 2, 4, 2, 3, 0, 1 };
		for (int i = 0; i < 8; i += 2)
			swap(&d[swaps[i]], &d[swaps[i + 1]]);

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
