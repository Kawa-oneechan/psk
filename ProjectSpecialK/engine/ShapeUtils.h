#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <json5pp.hpp>
#include "Tickable.h"
#include "Types.h"

//Returns true if point is inside of polygon.
extern bool PointInPoly(const glm::vec2 point, const polygon& polygon);
//Returns true if point is in rect.
extern bool PointInRect(const glm::vec2 point, const glm::vec4 rect);
