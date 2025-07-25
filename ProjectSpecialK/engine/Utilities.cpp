#include "Utilities.h"
#include "InputsMap.h"
#include "Tickable.h"
#include "JsonUtils.h"

extern int width, height;

bool PointInPoly(const glm::vec2 point, const polygon& polygon)
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

bool RevAllTickables(const std::vector<TickableP>& tickables, float dt)
{
	//for (auto t = tickables.crbegin(); t != tickables.crend(); ++t)
	for (unsigned int i = (unsigned int)tickables.size(); i-- > 0; )
	{
		auto t = tickables[i];
		if (!t->Enabled)
			continue;
		if (!t->Tick(dt))
			Inputs.Clear();
			//return false;
		//t->Tick(dt);
		//(*t)->Tick(dt);
	}
	return true;
}

void DrawAllTickables(const std::vector<TickableP>& tickables, float dt)
{
	for (const auto& t : tickables)
	{
		if (!t->Visible)
			continue;
		t->Draw(dt);
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
