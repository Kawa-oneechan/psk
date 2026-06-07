#include <ctime>
#include <numeric>
#include <glad/glad.h>
#include <stb_image_write.h>
#include "Utilities.h"
#include "Console.h"
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
		(point.x < rect.z) &&
		(point.y >= rect.y) &&
		(point.y < rect.w);
}
