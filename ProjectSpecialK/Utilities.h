#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <JSON/JSON.h>
#include "Tickable.h"
#include "Types.h"

//Returns true if point is inside of polygon.
extern bool PointInPoly(const glm::vec2 point, const polygon& polygon);
//Returns true if point is in rect.
extern bool PointInRect(const glm::vec2 point, const glm::vec4 rect);

//Loads camera settings from a JSONValue.
extern std::string LoadCamera(JSONValue* json);
//Loads camera settings from a JSON file.
extern std::string LoadCamera(const std::string& path);

//Loads lighting settings from a JSONValue.
extern std::string LoadLights(JSONValue* json);
//Loads lighting settings from a JSON file.
extern std::string LoadLights(const std::string& path);

//Returns a calendar date for things like "the fourth Friday in November".
extern tm* GetNthWeekdayOfMonth(int month, int dayOfWeek, int howManyth);

//Go through all root Tickables, calling their Tick methods in reverse order.
//Disabled Tickables are skipped. If any of them returns false, inputs are flushed.
extern bool RevAllTickables(const std::vector<TickableP>& tickables, float dt);
//Go through all root Tickables, calling their Draw methods.
//Invisible Tickables are skipped.
extern void DrawAllTickables(const std::vector<TickableP>& tickables, float dt);

//Helper function to project a point in world space to screen space, according to the current camera.
//TODO: does it return true if it's in front of the camera plane or behind it?
extern bool Project(const glm::vec3& in, glm::vec2& out);

//Invokes Scale2x, 3x, or 4x on an image. Returned pixel data is the caller's responsibility to delete.
extern unsigned char* ScaleImage(unsigned char* original, int origWidth, int origHeight, int channels, int targetScale);

//Returns the CRC32 hash for the given text.
extern hash GetCRC(const std::string& text);
//Returns the CRC32 hash for the given data.
extern hash GetCRC(unsigned char *buffer, int len);
