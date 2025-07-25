#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <JSON/JSON.h>
#include "engine/Types.h"

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

//Helper function to project a point in world space to screen space, according to the current camera.
//TODO: does it return true if it's in front of the camera plane or behind it?
extern bool Project(const glm::vec3& in, glm::vec2& out);

//Invokes Scale2x, 3x, or 4x on an image. Returned pixel data is the caller's responsibility to delete.
extern unsigned char* ScaleImage(unsigned char* original, int origWidth, int origHeight, int channels, int targetScale);
