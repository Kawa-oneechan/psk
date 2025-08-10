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

//Go through all root Tickables, calling their Tick methods in reverse order.
//Disabled Tickables are skipped. If any of them returns false, inputs are flushed.
extern bool RevAllTickables(const std::vector<TickableP>& tickables, float dt);
//Go through all root Tickables, calling their Draw methods.
//Invisible Tickables are skipped.
extern void DrawAllTickables(const std::vector<TickableP>& tickables, float dt);

//Returns the CRC32 hash for the given text.
extern hash GetCRC(const std::string& text);
//Returns the CRC32 hash for the given data.
extern hash GetCRC(unsigned char *buffer, int len);

#define conprint(C, F, ...) console->Print(C, fmt::format(F, __VA_ARGS__))
#ifdef DEBUG
#define debprint(C, F, ...) console->Print(C, fmt::format(F, __VA_ARGS__))
#else
#define debprint(C, F, ...)
#endif

__declspec(noreturn)
extern void FatalError(const std::string& message);

extern int width, height;
extern float scale;
