#include "engine/Utilities.h"
#include "engine/InputsMap.h"
#include "engine/Tickable.h"
#include "engine/JsonUtils.h"
#include "engine/Text.h"
#include "Game.h"
#include "Camera.h"

extern int width, height;

std::string LoadCamera(JSONValue* json)
{
	std::string result = "";
	try
	{
		if (json == nullptr)
			result = "no data.";
		else if (!json->IsObject())
			result = "not an object.";
		else
		{
			JSONObject& obj = (JSONObject&)json->AsObject();
			if (obj["target"] == nullptr || obj["angles"] == nullptr || obj["distance"] == nullptr)
				result = "not all required camera properties accounted for.";
			else
			{
				MainCamera->Target(GetJSONVec3(obj["target"]));
				MainCamera->Angles(GetJSONVec3(obj["angles"]));
				if (!obj["distance"]->IsNumber())
					result = "distance is not a number.";
				else
					MainCamera->Distance(obj["distance"]->AsNumber());
				if (obj["drum"] != nullptr && obj["drum"]->IsBool())
					commonUniforms.CurveEnabled = obj["drum"]->AsBool();
				if (obj["drumAmount"] != nullptr && obj["drumAmount"]->IsNumber())
					commonUniforms.CurveAmount = obj["drumAmount"]->AsNumber();
				if (obj["drumPower"] != nullptr && obj["drumPower"]->IsNumber())
					commonUniforms.CurvePower = obj["drumPower"]->AsNumber();
				//if (obj["locked"] != nullptr && obj["locked"]->IsBool())
				//	MainCamera.Locked = obj["locked"]->AsBool();
			}
		}
	}
	catch (std::runtime_error& x)
	{
		result = x.what();
	}
	if (!result.empty())
		conprint(1, "Could not load camera setup: {}", result);
	return result;
}

std::string LoadCamera(const std::string& path)
{
	std::string result = "";
	try
	{
		auto json = VFS::ReadJSON(path);
		if (json == nullptr)
			result = "no data.";
		else
			LoadCamera(json);
	}
	catch (std::runtime_error& x)
	{
		result = x.what();
	}
	return result;
}

std::string LoadLights(JSONValue* json)
{
	std::string result = "";
	try
	{
		if (json == nullptr)
			result = "no data.";
		else if (!json->IsArray())
			result = "not an array.";
		else
		{
			auto i = 0;
			for (auto lobj : json->AsArray())
			{
				if (!lobj->IsObject())
					result = "not an object.";
				else
				{
					JSONObject& l = (JSONObject&)lobj->AsObject();
					if (l["pos"] == nullptr || l["col"] == nullptr)
						result = "not all light properties accounted for.";
					else
					{
						commonUniforms.Lights[i].pos = GetJSONVec4(l["pos"]);
						commonUniforms.Lights[i].color = GetJSONColor(l["col"]);
					}
					i++;
					if (i == MaxLights)
						break;
				}
			}
		}
	}
	catch (std::runtime_error& x)
	{
		result = x.what();
	}
	if (!result.empty())
		conprint(1, "Could not load lighting setup: {}", result);
	return result;
}

std::string LoadLights(const std::string& path)
{
	std::string result = "";
	try
	{
		auto json = VFS::ReadJSON(path);
		if (json == nullptr)
			result = "no data.";
		else
			LoadLights(json);
	}
	catch (std::runtime_error& x)
	{
		result = x.what();
	}
	return result;
}

static tm _tm;
tm* GetNthWeekdayOfMonth(int month, int dayOfWeek, int howManyth)
{
	//C++17 doesn't have chrono calendar stuff yet, that's C++20.
	//So instead we're figuring this out the hard way.
	tm rightNow;
	time_t yesNow = std::time(nullptr);
	gmtime_s(&rightNow, &yesNow);
	int thisYear = rightNow.tm_year;

	tm novemberFirst = { 0 };
	novemberFirst.tm_year = thisYear;
	novemberFirst.tm_mon = month;
	novemberFirst.tm_mday = 1;
	novemberFirst.tm_hour = 12;
	std::mktime(&novemberFirst);
	time_t t = std::mktime(&novemberFirst);
	while (true)
	{
		tm here;
		gmtime_s(&here, &t);
		if (here.tm_wday == dayOfWeek)
		{
			t += ((60 * 60 * 24) * 7) * (howManyth - 1); //add a whole week for the amount of thursdays we want
			if (novemberFirst.tm_wday == dayOfWeek)
				t += (60 * 60 * 24) * 7; //add one more week if we *started* on a thursday
			break;
		}
		t += (60 * 60 * 24); //add one day
	}
	gmtime_s(&_tm, &t);
	return &_tm;
}

bool Project(const glm::vec3& in, glm::vec2& out)
{
	//TODO: account for the curve.
	auto res = glm::project(in, commonUniforms.View, commonUniforms.Projection, glm::vec4(0.0f, 0, width, height));
	out.x = res.x;
	out.y = height - res.y;
	return res.z >= 0.0f;
}

#include <scale2x/scalebit.h>
unsigned char* ScaleImage(unsigned char* original, int origWidth, int origHeight, int channels, int targetScale)
{
	if (targetScale < 2 || targetScale > 4)
		throw std::invalid_argument(fmt::format("ScaleImage: targetScale must be 2, 3, or 4, but was {}.", targetScale));

	int newWidth = origWidth * targetScale;
	int newHeight = origHeight * targetScale;

	auto target = new unsigned char[(newWidth * newHeight) * channels];
	scalebit(targetScale, target, newWidth  * channels, original, origWidth * channels, channels, origWidth, origHeight, 0);

	return target;
}

extern "C" { const char* glfwGetKeyName(int key, int scancode); }
std::string GetKeyName(int scancode)
{
	if (scancode == 1 || scancode == 14 || scancode == 15 || scancode == 28 || scancode == 57 ||
		(scancode >= 71 && scancode <= 83) || scancode == 284 || scancode == 309)
		return Text::Get(fmt::format("keys:scan:{}", scancode));

	auto glfw = glfwGetKeyName(-1, scancode);
	if (glfw[0] == '\0')
		return Text::Get(fmt::format("keys:scan:{}", scancode));
	else
		return std::string(glfw);
}

bool IsID(const std::string& id)
{
	//valid IDs may only contain alphanumerics, :, and _.
	for (auto& c : id)
	{
		if (!(std::isalnum(c) || c == ':' || c == '_'))
			return false;
	}
	return true;
}

bool IDIsQualified(const std::string& id)
{
	//must have a : but not as the first character.
	return id.find(':') != std::string::npos && id[0] != ':';
}

std::string Qualify(const std::string& id, const std::string& ns)
{
	//if (id.substr(0, ns.length()) == ns)
	//	throw std::runtime_error(fmt::format("Qualify: cannot double-qualify \"{}\", already starts with \"{}\".", id, ns));
	return ns + ':' + id;
}

std::string UnQualify(const std::string& id)
{
	if (IDIsQualified(id))
		return id.substr(id.find(':') + 1);
	return id;
}
