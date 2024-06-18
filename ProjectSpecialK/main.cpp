﻿#include <filesystem>
#include "SpecialK.h"

#include "Console.h"
#include "InputsMap.h"
#include "Cursor.h"
#include "Background.h"
#include "DoomMenu.h"
#include "DialogueBox.h"
#include "PanelLayout.h"
#include "DateTimePanel.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "support/glm/gtx/easing.hpp"

#include <thread>
#include <future>
#include <fstream>

#ifdef DEBUG
#define WINDOWTITLE "Project Special K (debug build " __DATE__ ")"
#else
#define WINDOWTITLE "Project Special K"
#endif

#define SCR_WIDTH 1920
#define SCR_HEIGHT 1080
#define WIN_WIDTH 1920 //1280
#define WIN_HEIGHT 1080 //720

extern "C"
{
	//why bother including windows headers lol
	_declspec(dllimport) int __stdcall MessageBoxA(_In_opt_ void* hWnd, _In_opt_ const char* lpText, _In_opt_ const char* lpCaption, _In_ unsigned int uType);
	_declspec(dllimport) int __stdcall MessageBoxW(_In_opt_ void* hWnd, _In_opt_ const wchar_t* lpText, _In_opt_ const wchar_t* lpCaption, _In_ unsigned int uType);
	_declspec(dllimport) int __stdcall MultiByteToWideChar(_In_ unsigned int CodePage, _In_ unsigned long dwFlags, const char* lpMultiByteStr, _In_ int cbMultiByte, wchar_t* lpWideCharStr, _In_ int cchWideChar);
#define MessageBox MessageBoxW
}

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

__declspec(noreturn)
void FatalError(const std::string& message)
{
	conprint(1, "Fatal error: {}", message);

	wchar_t w[1024] = { 0 };
	MultiByteToWideChar(65001, 0, message.c_str(), -1, w, 1024);
	MessageBox(nullptr, w, L"Project Special K", 0x30);

	conprint(1, "Exiting...");
	exit(1);
}

namespace SolBinds
{
	extern void Setup();
}

namespace UI
{
	std::map<std::string, glm::vec4> themeColors;
	std::vector<glm::vec4> textColors;

	Texture* controls;
	TextureAtlas controlsAtlas;
	//TODO: turn UI into a static class or whatever so it can initialize AFTER the VFS system, thus allowing removal of controlsAtlas.

	JSONObject& json = JSONObject();
	JSONObject& settings = JSONObject();

	static char* LoadFile(const std::string &filename, size_t *size)
	{
		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		if (!file.good())
			throw std::exception("Couldn't open file.");
		std::streamsize fs = file.tellg();
		file.seekg(0, std::ios::beg);
		if (size != nullptr)
			*size = fs;
		char* ret = (char*)malloc(fs + 2);
		if (ret == nullptr)
			throw std::exception("Could not allocate space for file.");
		memset(ret, 0, fs + 2);
		file.read(ret, fs);
		file.close();
		return ret;
	}

	static void SaveFile(const std::string &filename, const std::string& content)
	{
		std::ofstream file(filename, std::ios::trunc | std::ios::binary);
		if (!file.good())
			throw std::exception("Couldn't open file.");
		file.write(content.c_str(), content.size());
		file.close();
	}

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

		//TODO: load from and save to file.
		try
		{
			char *data = LoadFile("options.json", nullptr);
			settings = JSON::Parse(data)->AsObject();
			free(data);
		}
		catch (std::exception&)
		{
			settings["language"] = new JSONValue(0); //English
			settings["continue"] = new JSONValue(0); //Front door
			settings["speech"] = new JSONValue(1); //Bebebese
			settings["pingRate"] = new JSONValue(3);
			settings["balloonChance"] = new JSONValue(15);
			settings["cursorScale"] = new JSONValue(100);
			settings["24hour"] = new JSONValue(true);
			settings["contentFilters"] = new JSONValue(JSONObject());
			settings["musicVolume"] = new JSONValue(0.7f);
			settings["ambientVolume"] = new JSONValue(0.5f);
			settings["soundVolume"] = new JSONValue(1);
			settings["speechVolume"] = new JSONValue(1);
		}

		static const Language opt2lan[] = { Language::USen, Language::JPja, Language::EUde, Language::EUes, Language::EUfr, Language::EUit, Language::EUhu, Language::EUnl };
		gameLang = opt2lan[(int)settings["language"]->AsNumber()];

		Audio::MusicVolume = (float)settings["musicVolume"]->AsNumber();
		Audio::AmbientVolume = (float)settings["ambientVolume"]->AsNumber();
		Audio::SoundVolume = (float)settings["soundVolume"]->AsNumber();
		Audio::SpeechVolume = (float)settings["speechVolume"]->AsNumber();
	}

	static void Save()
	{
		settings["musicVolume"] = new JSONValue(Audio::MusicVolume);
		settings["ambientVolume"] = new JSONValue(Audio::AmbientVolume);
		settings["soundVolume"] = new JSONValue(Audio::SoundVolume);
		settings["speechVolume"] = new JSONValue(Audio::SpeechVolume);
		try
		{
			SaveFile("options.json", JSON::Stringify(&JSONValue(settings)));
		}
		catch (std::exception&)
		{
			conprint(2, "Couldn't save settings.");
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

void ThreadedLoader(std::function<void(void)> loader)
{
	glDisable(GL_DEPTH_TEST);
	cursor->Select(1);
	auto loadIcon = new Texture("loading.png");
	auto loadPos = glm::vec2(width - 256, height - 256);
	int oldTime = 0;

	std::promise<bool> p;
	auto future = p.get_future();

	std::thread t([&p, loader]
	{
		loader();
		p.set_value(true);
	});

	while (true)
	{
		int newTime = std::clock();
		int deltaTime = newTime - oldTime;
		oldTime = newTime;
		double dt = deltaTime;

		auto time = (float)glfwGetTime();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		sprender->DrawSprite(loadIcon, loadPos, glm::vec2(128), glm::vec4(0), sinf(time) * glm::radians(1000.0f));
		/*
		sprender->DrawSprite(loadIcon, loadPos + glm::vec2(0, (sinf(time * 2) * 10)), glm::vec2(128), glm::vec4(0), sinf(time) * glm::radians(1000.0f));
		for (int i = 0; i < 10; i++)
		{
		sprender->DrawSprite(whiteRect, loadPos + glm::vec2(0, 80 + (i * 2)), glm::vec2(128), glm::vec4(0), 0.0f, glm::vec4(0, 0, 0, i * 0.1f));
		}
		*/

		cursor->Draw();
		sprender->Flush();

		glfwSwapBuffers(window);
		glfwPollEvents();

		auto status = future.wait_for(std::chrono::milliseconds(1));
		if (status == std::future_status::ready)
			break;

	}
	t.join();
	cursor->Select(0);
}

int main(int argc, char** argv)
{
	std::srand((unsigned int)std::time(nullptr));

	//prepForUTF8andSuch(); //setlocale(LC_ALL, ".UTF8");
	console = new Console();
	try
	{
		InitVFS();
	}
	catch (std::runtime_error& x)
	{
		FatalError(x.what());
	}

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
		glfwTerminate();
		FatalError("Failed to create GLFW window.");
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
		FatalError("Failed to initialize GLAD.");
		return -1;
	}

	framebuffer_size_callback(window, WIN_WIDTH, WIN_HEIGHT);

	Shader ourShader("shaders/model.vs", "shaders/model.fs");
	spriteShader = new Shader("shaders/sprite.fs");
	whiteRect = new Texture("white.png", GL_CLAMP_TO_EDGE);
	UI::controls = new Texture("ui/controls.png");
	GetAtlas(UI::controlsAtlas, "ui/controls.json");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glFrontFace(GL_CW);

	//glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	//Required for sprites
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	sprender = new SpriteRenderer();
	cursor = new Cursor();



	ThreadedLoader(Database::LoadGlobalStuff);


	thePlayer.Name = "Kawa";
	thePlayer.Gender = Gender::BEnby;

	TextAdd(*ReadJSON("datetime.json"));
	TextAdd(*ReadJSON("fixedform.json"));
	TextAdd(*ReadJSON("optionsmenu.json"));
	TextAdd(*ReadJSON("tests.json"));

	tickables.push_back(new Background());
	dlgBox = new DialogueBox();
	//tickables.push_back(dlgBox);
	tickables.push_back(new DoomMenu());
	auto hotbar = new PanelLayout(UI::json["hotbar"]);
	tickables.push_back(hotbar);
	hotbar->Tween(&hotbar->Position.y, tweeny::from(-100.0f).to(0).during(100));
	hotbar->Tween(&hotbar->Alpha, tweeny::from(0.0f).to(0.75f).during(200));
	tickables.push_back(new DateTimePanel());

	//tickables.push_back(new TextField());

	int oldTime = 0;
	auto pos = 0.0f;
	while (!glfwWindowShouldClose(window))
	{
		Audio::Update();

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

		/*
		Tween crap using nothing but GLM, work it into Utilities. Might replace Tweeny with it.
		auto origin = glm::vec2(width, height) * 0.5f;
		auto color1 = glm::vec4(1, 0, 0, 1);
		auto color2 = glm::vec4(0, 0, 1, 1);
		pos += 0.001f; if (pos >= 1.0f) pos = 0;
		auto lerp = glm::linearInterpolation(pos);
		auto color = glm::vec4((color2.r - color1.r) * lerp + color1.r,
			(color2.g - color1.g) * lerp + color1.g,
			(color2.b - color1.b) * lerp + color1.b,
			(color2.a - color1.a) * lerp + color1.a);
		sprender->DrawSprite(whiteRect, origin - glm::vec2(16, 16) + glm::vec2(0, lerp * 64), glm::vec2(32, 32), glm::vec4(0), 0.0, color);
		lerp = glm::bounceEaseOut(pos);
		color = glm::vec4((color2.r - color1.r) * lerp + color1.r,
			(color2.g - color1.g) * lerp + color1.g,
			(color2.b - color1.b) * lerp + color1.b,
			(color2.a - color1.a) * lerp + color1.a);
		sprender->DrawSprite(whiteRect, origin - glm::vec2(16, 16) + glm::vec2(32, lerp * 64), glm::vec2(32, 32), glm::vec4(0), 0.0, color);
		*/

		sprender->Flush();

		tickables.erase(std::remove_if(tickables.begin(), tickables.end(), [](Tickable* i) {
			return i->dead;
		}), tickables.end());

		//turn depth testing back on for 3D shit
		glEnable(GL_DEPTH_TEST);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	UI::Save();

	glfwTerminate();
	return 0;
}

extern "C" int __stdcall WinMain(struct HINSTANCE__*, struct HINSTANCE__*, char*, int) { return main(__argc, __argv); }
