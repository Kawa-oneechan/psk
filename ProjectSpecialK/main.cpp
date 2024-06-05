﻿#include "SpecialK.h"

#include "Console.h"
#include "InputsMap.h"
#include "Cursor.h"
#include "Background.h"
#include "DoomMenu.h"
#include "DialogueBox.h"
#include "PanelLayout.h"

#include <thread>

#ifdef DEBUG
#define WINDOWTITLE "Project Special K (debug build " __DATE__ ")"
#else
#define WINDOWTITLE "Project Special K"
#endif

#define SCR_WIDTH 1920
#define SCR_HEIGHT 1080
#define WIN_WIDTH 1920 //1280
#define WIN_HEIGHT 1080 //720

GLFWwindow* window;

Shader* spriteShader = nullptr;
Texture* whiteRect = nullptr;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
SpriteRenderer* sprender = nullptr;
DialogueBox* dlgBox = nullptr;
Cursor* cursor = nullptr;
Console* console = nullptr;
Audio* bgm = nullptr;

sol::state Sol;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

bool wireframe = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float width = SCR_WIDTH, height = SCR_HEIGHT;
float scale = (float)WIN_WIDTH / (float)SCR_HEIGHT;

int articlePlease;

glm::vec2 GetJSONVec2(JSONValue* val)
{
	if (!val->IsArray())
		throw std::runtime_error("GetJSONVec2: given value is not an array.");
	auto arr = val->AsArray();
	if (arr.size() != 2)
		throw std::runtime_error(fmt::format("GetJSONVec2: given array has {} entries, not 2.", arr.size()));
	if (!arr[0]->IsNumber() || !arr[1]->IsNumber())
		throw std::runtime_error("GetJSONVec2: given array does not contain only numbers.");
	return glm::vec2(arr[0]->AsNumber(), arr[1]->AsNumber());
}

glm::vec4 GetJSONVec4(JSONValue* val)
{
	if (!val->IsArray())
		throw std::runtime_error("GetJSONVec4: given value is not an array.");
	auto arr = val->AsArray();
	if (arr.size() != 4)
		throw std::runtime_error(fmt::format("GetJSONVec4: given array has {} entries, not 4.", arr.size()));
	if (!arr[0]->IsNumber() || !arr[1]->IsNumber() || !arr[2]->IsNumber() || !arr[3]->IsNumber())
		throw std::runtime_error("GetJSONVec4: given array does not contain only numbers.");
	return glm::vec4(arr[0]->AsNumber(), arr[1]->AsNumber(), arr[2]->AsNumber(), arr[3]->AsNumber());
}

void GetAtlas(TextureAtlas &ret, const std::string& jsonFile)
{
	auto rjs = ReadJSON(jsonFile);
	if (rjs == nullptr)
		return;
	auto doc = rjs->AsObject();
	ret.clear();
	if (doc["type"]->AsString() == "simple")
	{
		auto size = GetJSONVec2(doc["size"]);
		auto dims = GetJSONVec2(doc["dims"]);
		for (int y = 0; y < (int)dims[0]; y++)
		{
			for (int x = 0; x < (int)dims[1]; x++)
			{
				ret.push_back(glm::vec4(x * size[0], y * size[1], size[0], size[1]));
			}
		}
		return;
	}
	else if (doc["type"]->AsString() == "atlas")
	{
		auto rects = doc["rects"]->AsArray();
		for (const auto& rect : rects)
		{
			ret.push_back(GetJSONVec4(rect));
		}
		return;
	}

	throw std::runtime_error(fmt::format("GetAtlas: file {} has an unknown type \"{}\".", jsonFile, doc["type"]->AsString()));
}

bool PointInPoly(const glm::vec2 point, const std::vector<glm::vec2>& polygon)
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

namespace SolBinds
{
	extern void Setup();
}

namespace UI
{
	std::map<std::string, glm::vec4> themeColors;
	std::vector<glm::vec4> textColors;

	JSONObject& json = JSONObject();

	static void Load(const JSONValue* source)
	{
		json = source->AsObject();
		auto colors = json["colors"]->AsObject();
		for (auto& ink : colors["theme"]->AsObject())
		{
			themeColors[ink.first] = GetJSONVec4(ink.second);
		}
		for (auto& ink : colors["text"]->AsArray())
		{
			textColors.push_back(GetJSONVec4(ink));
		}
	}
};

std::vector<Tickable*> tickables;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	::width = (float)width;
	::height = (float)height;
	scale = ::height / SCR_HEIGHT;
	glViewport(0, 0, width, height);
}

void char_callback(GLFWwindow* window, unsigned int codepoint)
{
	if (console->visible)
	{
		if (codepoint == '`') return;
		console->Character(codepoint);
		return;
	}
	for (unsigned int i = (unsigned int)tickables.size(); i-- > 0; )
	{
		auto t = tickables[i];
		if (t->Character(codepoint))
			break;
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS)
	{
		console->visible = !console->visible;
		return;
	}

	Inputs.Process(key, action);

	//Passthroughs
	if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS)
		char_callback(window, '\b');
	else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		char_callback(window, 0xFFF0);
	else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		char_callback(window, 0xFFF1);
	else if (key == GLFW_KEY_HOME && action == GLFW_PRESS)
		char_callback(window, 0xFFF2);
	else if (key == GLFW_KEY_END && action == GLFW_PRESS)
		char_callback(window, 0xFFF3);

	//if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	//	glfwSetWindowShouldClose(window, 1);
	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	{
		wireframe = !wireframe;
		glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
	}

	if (key == GLFW_KEY_W)
		camera.ProcessKeyboard(CameraMovement::Forward, 0.025f);
	else if (key == GLFW_KEY_A)
		camera.ProcessKeyboard(CameraMovement::Left, 0.025f);
	else if (key == GLFW_KEY_S)
		camera.ProcessKeyboard(CameraMovement::Backward, 0.025f);
	else if (key == GLFW_KEY_D)
		camera.ProcessKeyboard(CameraMovement::Right, 0.025f);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	Inputs.MouseMove(xpos, ypos);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	//camera.ProcessMouseMovement(xoffset, yoffset);
}

void mousebutton_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		Inputs.MouseHoldLeft = action == GLFW_PRESS;
		if (!Inputs.MouseLeft && action == GLFW_RELEASE) //-V1051 no I'm pretty sure I meant this
			Inputs.MouseLeft = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
	{
		if (!Inputs.MouseMiddle && action == GLFW_RELEASE)
			Inputs.MouseMiddle = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (!Inputs.MouseRight && action == GLFW_RELEASE)
			Inputs.MouseRight = true;
	}
}

extern "C"
{
	//why bother including windows headers lol
	_declspec(dllimport) int __stdcall MessageBoxA(_In_opt_ void* hWnd, _In_opt_ const char* lpText, _In_opt_ const char* lpCaption, _In_ unsigned int uType);
	_declspec(dllimport) int __stdcall MessageBoxW(_In_opt_ void* hWnd, _In_opt_ const wchar_t* lpText, _In_opt_ const wchar_t* lpCaption, _In_ unsigned int uType);
	_declspec(dllimport) int __stdcall MultiByteToWideChar(_In_ unsigned int CodePage, _In_ unsigned long dwFlags, const char* lpMultiByteStr, _In_ int cbMultiByte, wchar_t* lpWideCharStr, _In_ int cchWideChar);
#define MessageBox MessageBoxW
}

__declspec(noreturn)
void FatalError(const std::string& message)
{
	wchar_t w[1024] = { 0 };
	MultiByteToWideChar(65001, 0, message.c_str(), -1, w, 1024);
	MessageBox(nullptr, w, L"Project Special K", 0x30);
	exit(1);
}

int main(int argc, char** argv)
{
	std::srand((unsigned int)std::time(nullptr));

	//prepForUTF8andSuch(); //setlocale(LC_ALL, ".UTF8");
	console = new Console();
	InitVFS();

	Audio::Initialize();
	SolBinds::Setup();

	UI::Load(ReadJSON("ui/ui.json"));




	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 2);

	if (WIN_WIDTH == SCR_WIDTH)
	{
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		if (mode->width == SCR_WIDTH && mode->height == SCR_HEIGHT)
			glfwWindowHint(GLFW_DECORATED, 0);
		glfwWindowHint(GLFW_RESIZABLE, 0);
	}

	window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, WINDOWTITLE, NULL, NULL);
	if (window == NULL)
	{
		printf("Failed to create GLFW window\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCharCallback(window, char_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mousebutton_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("Failed to initialize GLAD\n");
		return -1;
	}

	framebuffer_size_callback(window, WIN_WIDTH, WIN_HEIGHT);

	Shader ourShader("shaders/model.vs", "shaders/model.fs");
	spriteShader = new Shader("shaders/sprite.fs");
	whiteRect = new Texture("white.png", true, GL_CLAMP_TO_EDGE);

	int oldTime = 0;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glFrontFace(GL_CW);

	//glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	//Required for sprites
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	sprender = new SpriteRenderer();
	cursor = new Cursor();

	//Loading screen. Might make for a good general-use thing, pass it a function pointer 
	{
		std::thread loader(Database::LoadGlobalStuff);
		loader.detach();
		glDisable(GL_DEPTH_TEST);
		cursor->Select(1);
		auto loadIcon = new Texture("loading.png");
		auto loadPos = glm::vec2(width - 256, height - 256);
		while (!glfwWindowShouldClose(window) && !Database::DoneLoading)
		{
			int newTime = std::clock();
			int deltaTime = newTime - oldTime;
			oldTime = newTime;
			double dt = deltaTime;

			auto time = (float)glfwGetTime();

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			sprender->DrawSprite(loadIcon, loadPos, glm::vec2(128), glm::vec4(0), sinf(time) * glm::radians(1000.0f));

			cursor->Draw();
			sprender->Flush();

			glfwSwapBuffers(window);
			glfwPollEvents();
		}
		cursor->Select(0);
	}




	thePlayer.Name = "Kawa";
	thePlayer.Gender = Gender::BEnby;

	TextAdd(*ReadJSON("datetime.json"));
	TextAdd(*ReadJSON("fixedform.json"));





	tickables.push_back(new Background());
	dlgBox = new DialogueBox();
	tickables.push_back(dlgBox);
	tickables.push_back(new DoomMenu());
	auto hotbar = new PanelLayout(UI::json["hotbar"]);
	tickables.push_back(hotbar);
	hotbar->Tween(&hotbar->Position.y, tweeny::from(-100.0f).to(0).during(100));
	hotbar->Tween(&hotbar->Alpha, tweeny::from(0.0f).to(0.75f).during(200));
	//tickables.push_back(new TextField());

	while (!glfwWindowShouldClose(window))
	{
		int newTime = std::clock();
		int deltaTime = newTime - oldTime;
		oldTime = newTime;
		double dt = deltaTime;

		auto time = (float)glfwGetTime();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
		//important: disable depth testing to allow multiple sprites to overlap.
		glDisable(GL_DEPTH_TEST);

		if (console->visible)
			console->Tick(dt);
		else
		{
			for (unsigned int i = (unsigned int)tickables.size(); i-- > 0; )
				tickables[i]->Tick(dt);
		}

		for (unsigned int i = 0; i < (unsigned int)tickables.size(); i++)
			tickables[i]->Draw(dt * 0.25);
		console->Draw(dt);

		cursor->Draw();
		sprender->Flush();

		//turn depth testing back on for 3D shit
		glEnable(GL_DEPTH_TEST);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

extern "C" int __stdcall WinMain(struct HINSTANCE__*, struct HINSTANCE__*, char*, int) { return main(__argc, __argv); }
