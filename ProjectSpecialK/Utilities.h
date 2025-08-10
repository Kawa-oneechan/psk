#pragma once
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include "engine/Types.h"
#include "engine/JsonUtils.h"

extern "C" {
	const char* glfwGetKeyName(int key, int scancode);
	int glfwGetKeyScancode(int key);
}

//Loads camera settings from a JSONValue.
extern std::string LoadCamera(jsonValue& json);
//Loads camera settings from a JSON file.
extern std::string LoadCamera(const std::string& path);

//Loads lighting settings from a JSONValue.
extern std::string LoadLights(jsonValue& json);
//Loads lighting settings from a JSON file.
extern std::string LoadLights(const std::string& path);

//Returns a calendar date for things like "the fourth Friday in November".
extern tm* GetNthWeekdayOfMonth(int month, int dayOfWeek, int howManyth);

//Helper function to project a point in world space to screen space, according to the current camera.
//TODO: does it return true if it's in front of the camera plane or behind it?
extern bool Project(const glm::vec3& in, glm::vec2& out);

//Invokes Scale2x, 3x, or 4x on an image. Returned pixel data is the caller's responsibility to delete.
extern unsigned char* ScaleImage(unsigned char* original, int origWidth, int origHeight, int channels, int targetScale);

//Returns the name of a key for the given scancode, using glfwGetKeyName for printables and Text::Get for specials.
extern std::string GetKeyName(int scancode);

//Checks if a string contains only characters valid for an ID (alhpanumerics, colons, underscores).
extern bool IsID(const std::string& id);
//Checks if a string contains a colon, which would mark it as a valid ID.
extern bool IDIsQualified(const std::string& id);
//Prepends the given namespace to an ID.
extern std::string Qualify(const std::string& id, const std::string& ns);
//Removes the frontmost namespace from an ID.
extern std::string UnQualify(const std::string& id);

//Shows a loading screen while running another thread.
extern void ThreadedLoader(std::function<void(float*)> loader);
