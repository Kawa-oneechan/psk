﻿#include <filesystem>
#include "SpecialK.h"

#include <glad/glad.h>
#include <glfw/glfw3.h>

#include "Console.h"
#include "InputsMap.h"
#include "Cursor.h"
#include "Background.h"
#include "Town.h"
#include "Messager.h"
#include "MusicManager.h"
#include "Framebuffer.h"
#include "TitleScreen.h"
#include "InGame.h"
#include "TextUtils.h"
#include "Utilities.h"

#include <fstream>

constexpr auto WindowTitle = "Project Special K - " VERSIONJOKE
#ifdef DEBUG
" (debug build " __DATE__ ")";

extern bool IsImGuiHovered();
extern void SetupImGui();
extern void DoImGui();
extern void RunTests();
#endif
;

constexpr int ScreenWidth = 1920;
constexpr int ScreenHeight = 1080;

#ifdef _WIN32
extern "C"
{
	//why bother including windows headers lol
	int __stdcall MessageBoxW(_In_opt_ void* hWnd, _In_opt_ const wchar_t* lpText, _In_opt_ const wchar_t* lpCaption, _In_ unsigned int uType);
	int __stdcall MultiByteToWideChar(_In_ unsigned int CodePage, _In_ unsigned long dwFlags, const char* lpMultiByteStr, _In_ int cbMultiByte, wchar_t* lpWideCharStr, _In_ int cchWideChar);
}
#endif

glm::mat4 perspectiveProjection, orthographicProjection;
bool useOrthographic = false;

GLFWwindow* window;

Texture* whiteRect = nullptr;
std::shared_ptr<DialogueBox> dlgBox = nullptr;
CursorP cursor = nullptr;
Console* console = nullptr;
Audio* bgm = nullptr;

sol::state Sol;

int width = ScreenWidth, height = ScreenHeight;
float scale = height / 1080.0f;

float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;

bool wireframe = false;

float DeltaTime = 0.0f;
float lastFrame = 0.0f;

float timeScale = 1.0f;

int articlePlease;

bool cheatsEnabled;
extern bool botherColliding;

#ifdef DEBUG
float uiTime = 0;
float glTime = 0;
#endif

CommonUniforms commonUniforms;
unsigned int commonBuffer = 0;

std::shared_ptr<Camera> MainCamera;
std::shared_ptr<Messager> messager;
std::shared_ptr<MusicManager> musicManager;
std::shared_ptr<Town> town;

namespace rnd
{
	std::random_device device;
	std::mt19937 engine(device());

	int getInt(int min, int max)
	{
		std::uniform_int_distribution<> dist(min, max);
		return dist(engine);
	}

	int getInt(int max)
	{
		return getInt(0, max);
	}

	int getInt()
	{
		return getInt(0, RAND_MAX);
	}

	float getFloat(float min, float max)
	{
		std::uniform_real_distribution<> dist(min, max);
		return (float)dist(engine);
	}

	float getFloat(float max)
	{
		return getFloat(0.0f, max);
	}

	float getFloat()
	{
		return getFloat(0.0f, 1.0f);
	}

	bool flip()
	{
		return getFloat() > 0.5f;
	}
}

__declspec(noreturn)
void FatalError(const std::string& message)
{
	conprint(1, "Fatal error: {}", message);

#ifdef _WIN32
	wchar_t w[1024] = { 0 };
	MultiByteToWideChar(65001, 0, message.c_str(), -1, w, 1024);
	MessageBoxW(nullptr, w, L"Project Special K", 0x30);
#else
	//TODO: report fatal errors some other way on non-Windows systems.
	//Internet says: write the error to a file, then xdg-open that file.
#endif

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

	std::shared_ptr<Texture> controls{ nullptr };

	JSONObject json = JSONObject();
	JSONObject settings = JSONObject();

	std::string initFile = "init.json";

	static void Load()
	{
		auto doc = VFS::ReadJSON("ui/ui.json");
		if (!doc)
			FatalError("Could not read ui/ui.json. Something is very wrong.");
		json = doc->AsObject();
		auto colors = json["colors"]->AsObject();
		for (auto& ink : colors["theme"]->AsObject())
		{
			themeColors[ink.first] = GetJSONColor(ink.second);
		}
		for (auto& ink : colors["text"]->AsArray())
		{
			textColors.push_back(GetJSONColor(ink));
		}

		try
		{
			settings = VFS::ReadSaveJSON("options.json")->AsObject();
		}
		catch (std::runtime_error&)
		{
			settings = JSON::Parse("{}")->AsObject();
		}

#define DS(K, V) if (!settings[K]) settings[K] = new JSONValue(V)
		DS("screenWidth", ScreenWidth);
		DS("screenHeight", ScreenHeight);
		DS("language", "USen"); //(int)Language::USen);
		DS("continue", (int)LoadSpawnChoice::FrontDoor);
		DS("speech", (int)DialogueBox::Sound::Bebebese);
		DS("pingRate", 3);
		DS("balloonChance", 15);
		DS("cursorScale", 100);
		DS("botherColliding", true);
		DS("24hour", true);
		DS("contentFilters", JSONObject());
		DS("musicVolume", 70);
		DS("ambientVolume", 50);
		DS("soundVolume", 100);
		DS("speechVolume", 100);
		DS("keyBinds", JSONArray());
		DS("gamepadBinds", JSONArray());
#undef DS

		width = settings["screenWidth"]->AsInteger();
		height = settings["screenHeight"]->AsInteger();

		//constexpr Language opt2lan[] = { Language::USen, Language::JPja, Language::EUde, Language::EUes, Language::EUfr, Language::EUit, Language::EUhu, Language::EUnl, Language::EUen };
		//gameLang = opt2lan[settings["language"]->AsInteger()];
		gameLang = Text::GetLangCode(settings["language"]->AsString());

		Audio::MusicVolume = settings["musicVolume"]->AsInteger() / 100.0f;
		Audio::AmbientVolume = settings["ambientVolume"]->AsInteger() / 100.0f;
		Audio::SoundVolume = settings["soundVolume"]->AsInteger() / 100.0f;
		Audio::SpeechVolume = settings["speechVolume"]->AsInteger() / 100.0f;

		botherColliding = settings["botherColliding"]->AsBool();

		auto keyBinds = settings["keyBinds"]->AsArray();
		if (keyBinds.size() != NumKeyBinds)
		{
			keyBinds.reserve(NumKeyBinds);
			for (auto &k : DefaultInputBindings)
				keyBinds.push_back(new JSONValue(glfwGetKeyScancode(k)));
		}

		auto padBinds = settings["gamepadBinds"]->AsArray();
		if (padBinds.size() != NumKeyBinds)
		{
			padBinds.reserve(NumKeyBinds);
			for (auto &k : DefaultInputGamepadBindings)
				padBinds.push_back(new JSONValue(k));
		}

		for (int i = 0; i < NumKeyBinds; i++)
		{
			Inputs.Keys[i].ScanCode = keyBinds[i]->AsInteger();
			Inputs.Keys[i].GamepadButton = padBinds[i]->AsInteger();
		}

		doc = VFS::ReadJSON("sound/sounds.json");
		auto sounds = doc->AsObject();
		for (auto category : sounds)
		{
			for (auto sound : category.second->AsObject())
				generalSounds[category.first][sound.first] = std::make_shared<Audio>(sound.second->AsString());
		}
		delete doc;
	}

	static void Save()
	{
		settings["screenWidth"] = new JSONValue(width);
		settings["screenHeight"] = new JSONValue(height);

		settings["musicVolume"] = new JSONValue((int)(Audio::MusicVolume * 100.0f));
		settings["ambientVolume"] = new JSONValue((int)(Audio::AmbientVolume * 100.0f));
		settings["soundVolume"] = new JSONValue((int)(Audio::SoundVolume * 100.0f));
		settings["speechVolume"] = new JSONValue((int)(Audio::SpeechVolume * 100.0f));

		settings["botherColliding"] = new JSONValue(botherColliding);

		auto binds = JSONArray();
		for (auto& k : Inputs.Keys)
			binds.push_back(new JSONValue(k.ScanCode));
		settings["keyBinds"] = new JSONValue(binds);

		binds.clear();
		for (auto& k : Inputs.Keys)
			binds.push_back(new JSONValue(k.GamepadButton));
		settings["gamepadBinds"] = new JSONValue(binds);

		try
		{
			VFS::WriteSaveJSON("options.json", new JSONValue(settings));
		}
		catch (std::exception&)
		{
			conprint(2, "Couldn't save settings.");
		}
	}
};

//Currently active Tickables.
std::vector<TickableP> tickables;
//Tickables to add next cycle.
std::vector<TickableP> newTickables;

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	window;
	::width = width;
	::height = height;
	scale = ::height / 1080.0f;
	glViewport(0, 0, width, height);
	commonUniforms.ScreenRes = glm::uvec2(width, height);
}

static void char_callback(GLFWwindow* window, unsigned int codepoint)
{
	window;
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

#ifdef DEBUG
extern bool debuggerEnabled;
#endif

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	window, mods;
	if (scancode == Inputs.Keys[(int)Binds::Console].ScanCode && action == GLFW_PRESS)
	{
		if (console->visible)
			console->Close();
		else
			console->Open();
		return;
	}

	if (console->visible && action == GLFW_PRESS)
	{
		console->Scancode(scancode);
		return;
	}

	Inputs.Process(scancode, action);

	for (unsigned int i = (unsigned int)tickables.size(); i-- > 0; )
	{
		auto t = tickables[i];
		if (t->Scancode(scancode))
			break;
	}

	if (console->visible)
		return;

	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
		wireframe = !wireframe;

#ifdef DEBUG
	if (key == GLFW_KEY_D && mods == GLFW_MOD_CONTROL && action == GLFW_PRESS)
		debuggerEnabled = !debuggerEnabled;
#endif
}

static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	window;
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
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	if (Inputs.MouseHoldMiddle && !MainCamera->Locked)
	{
		auto angles = MainCamera->GetAngles();
		angles.z -= xoffset;
		angles.y -= yoffset;
		MainCamera->Angles(angles);
	}

#if 0
	//Mouse picking test
	{
		auto projection = commonUniforms.Projection;
		auto view = commonUniforms.View;
		auto screen = glm::ivec4(0, 0, width, height);
		auto nearSource = glm::vec3(xpos, ypos, 0);
		auto farSource = glm::vec3(xpos, ypos, 1);
		auto nearPoint = glm::unProject(nearSource, view, projection, screen);
		auto farPoint = glm::unProject(farSource, view, projection, screen);
		auto direction = farPoint - nearPoint;
		direction = glm::normalize(direction);

		auto newTitle = fmt::format("Mouse picking: near pt {:.3} {:.3} {:.3}, far pt {:.3} {:.3} {:.3}, direction {:.3} {:.3} {:.3}", nearPoint.x, nearPoint.y, nearPoint.z, farPoint.x, farPoint.y, farPoint.z, direction.x, direction.y, direction.z);
		glfwSetWindowTitle(window, newTitle.c_str());

		/*
		Okay so what I had before in "Hidden Power" on XNA was a Ray class.
		That offered intersection methods with bounding boxes and such:
		
		var cursorRay = new Ray(nearPoint, direction);
		foreach (var block in Map.Blocks)
		{
			if (string.IsNullOrEmpty(block.Caption)) continue;
			if (cursorRay.Intersects(block.Box) != null)
			{
				PickedObject = block;
				picked = block.Caption;
				break;
			}
		}
		*/
	}
#endif
}

static void mousebutton_callback(GLFWwindow* window, int button, int action, int mods)
{
	window; mods;
	
#ifdef DEBUG
	if (IsImGuiHovered())
	{
		Inputs.MouseLeft = false;
		return;
	}
#endif

	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		Inputs.MouseHoldLeft = action == GLFW_PRESS;
		if (!Inputs.MouseLeft && action == GLFW_RELEASE) //-V1051 no I'm pretty sure I meant this
			Inputs.MouseLeft = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
	{
		Inputs.MouseHoldMiddle = action == GLFW_PRESS;
		if (!Inputs.MouseMiddle && action == GLFW_RELEASE) //-V1051
			Inputs.MouseMiddle = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		Inputs.MouseHoldRight = action == GLFW_PRESS;
		if (!Inputs.MouseRight && action == GLFW_RELEASE) //-V1051
			Inputs.MouseRight = true;
	}
}

static void joystick_callback(int jid, int event)
{
	if (event == GLFW_CONNECTED)
	{
		Inputs.HaveGamePad = (jid == GLFW_JOYSTICK_1 && glfwJoystickIsGamepad(jid));
	}
	else if (event == GLFW_DISCONNECTED)
	{
		// The joystick was disconnected
	}
}

static int InitOpenGL()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 2);

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	if (mode->width < width || mode->height < height)
	{
		width = mode->width;
		height = mode->height;
	}
	if (mode->width == width && mode->height == height)
	{
		glfwWindowHint(GLFW_DECORATED, 0);
		glfwWindowHint(GLFW_CENTER_CURSOR, 1);
	}
	else
	{
		glfwWindowHint(GLFW_POSITION_X, (mode->width / 2) - (width / 2));
		glfwWindowHint(GLFW_POSITION_Y, (mode->height / 2) - (height / 2));
	}
	glfwWindowHint(GLFW_RESIZABLE, 0);

	window = glfwCreateWindow(width, height, WindowTitle, NULL, NULL);
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
	glfwSetJoystickCallback(joystick_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		FatalError("Failed to initialize GLAD.");
		return -1;
	}

	framebuffer_size_callback(window, width, height);

	glEnable(GL_CULL_FACE);

	//Required for sprites
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return 0;
}

#ifdef DEBUG
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	length;
	userParam;

	static std::map<int, std::string> sources = {
		{ GL_DEBUG_SOURCE_API, "API" },
		{ GL_DEBUG_SOURCE_WINDOW_SYSTEM, "window system" },
		{ GL_DEBUG_SOURCE_SHADER_COMPILER, "shader compiler" },
		{ GL_DEBUG_SOURCE_THIRD_PARTY, "third party" },
		{ GL_DEBUG_SOURCE_APPLICATION, "application" },
	};
	static std::map<int, std::string> types = {
		{ GL_DEBUG_TYPE_ERROR, "error" },
		{ GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "deprecated behavior" },
		{ GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, "undefined behavior" },
		{ GL_DEBUG_TYPE_PORTABILITY, "portability issue" },
		{ GL_DEBUG_TYPE_PERFORMANCE, "performance issue" },
		{ GL_DEBUG_TYPE_MARKER, "marker" },
		{ GL_DEBUG_TYPE_PUSH_GROUP, "group push" },
		{ GL_DEBUG_TYPE_POP_GROUP, "group pop" },
		{ GL_DEBUG_TYPE_OTHER, "other" },
	};
	static std::map<int, std::string> severities = {
		{ GL_DEBUG_SEVERITY_HIGH, "high" },
		{ GL_DEBUG_SEVERITY_MEDIUM, "medium" },
		{ GL_DEBUG_SEVERITY_LOW, "low" },
		{ GL_DEBUG_SEVERITY_NOTIFICATION, "notification" },
	};
	conprint(5, "Message from OpenGL: ID {:X}, source {}, type {}, severity {}:  {}", id, sources[source], types[type], severities[severity], message);
}
#endif

Framebuffer* postFxBuffer;
//Background* rainLayer;

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "en_US.UTF-8");
	//std::srand((unsigned int)std::time(nullptr));

	bool skipTitle = false;

	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];
		if (arg == "-config" && i + 1 < argc)
		{
			i++;
			UI::initFile = argv[i];
		}
		if (arg == "-skiptitle")
		{
			skipTitle = true;
		}
	}

	console = new Console();
	try
	{
		VFS::Initialize();
	}
	catch (std::runtime_error& x)
	{
		FatalError(x.what());
	}

	glfwInit();

	Audio::Initialize();
	SolBinds::Setup();

	UI::Load();

	/*
	{
		town.StartNewDay();
	}
	*/

	if (auto r = InitOpenGL())
		return r;

#ifdef DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
#endif

	whiteRect = new Texture("white.png", GL_CLAMP_TO_EDGE);
	UI::controls = std::make_shared<Texture>("ui/controls.png");

	Shader::LoadAll();
	commonUniforms.GrassColor = 0.5f;

	GLuint commonBind = 1;
	glGenBuffers(1, &commonBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, commonBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(commonUniforms), &commonUniforms, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, commonBind, commonBuffer);

#ifdef DEBUG
	SetupImGui();
#endif

	cursor = std::make_shared<Cursor>();
	MainCamera = std::make_shared<Camera>();
	musicManager = std::make_shared<MusicManager>();

	Inputs.HaveGamePad = (glfwJoystickPresent(GLFW_JOYSTICK_1) && glfwJoystickIsGamepad(GLFW_JOYSTICK_1));

	ThreadedLoader(Database::LoadGlobalStuff);

	auto panels = new Texture("ui/panels.png");
	panels->Use();

	//thePlayer.Name = "Kawa";
	//thePlayer.Gender = Gender::BEnby;

	//Now that we've loaded the key names we can fill in some blanks.
	for (int i = 0; i < NumKeyBinds; i++)
		Inputs.Keys[i].Name = GetKeyName(Inputs.Keys[i].ScanCode);

	/*
	tickables.push_back(&musicManager);
	tickables.push_back(&MainCamera);
	//auto background = Background("discobg2.png");
	//tickables.push_back(new Background("discobg2.png"));
	//tickables.push_back(new TemporaryTownDrawer());
	auto townDrawer = TemporaryTownDrawer();
	tickables.push_back(new DateTimePanel());
	tickables.push_back(new ItemHotbar());
	//hotbar->Tween(&hotbar->Position.y, -100.0f, 0, 0.002f, glm::bounceEaseOut<float>);
	//hotbar->Tween(&hotbar->Alpha, 0, 0.75f, 0.006f);
	//auto logoJson = VFS::ReadJSON("cinematics/logo/logo.json")->AsObject();
	//auto logoAnim = new PanelLayout(logoJson["cinematic"]);
	//tickables.push_back(logoAnim);
	//tickables.push_back(new DoomMenu());
	dlgBox = new DialogueBox();
	tickables.push_back(dlgBox);
	auto messager = new Messager();
	tickables.push_back(messager);
	//tickables.push_back(new TextField());
	*/

	town = std::make_shared<Town>();
	dlgBox = std::make_shared<DialogueBox>();
	messager = std::make_shared<Messager>();

#ifdef DEBUG
	RunTests();
#endif

	//Load the player *here* so we don't get inventory test results mixed in.
	thePlayer.Load();

	auto cloudImage = Texture("altostratus.png");
	auto starsImage = Texture("starfield.png");
	auto skyImage = Texture("skycolors.png");

	//rainLayer = new Background("rain.png", glm::vec2(1.0, 2.0));

	if (!LoadLights("lights/initial.json").empty())
	{
		commonUniforms.Lights[0].pos = { 0, 15, 20, 0 };
		commonUniforms.Lights[0].color = { 1, 1, 1, 0.25 };
	}
	if (!LoadCamera("cameras/field.json").empty())
	{
		MainCamera->Set(glm::vec3(0, 0, -6), glm::vec3(0, 110, 0), 60);
	}

	if (town->Villagers.size() == 0)
	{
		town->Villagers.push_back(Database::Find<Villager>("ac:cat01", villagers));
	}
	{
		int i = 0;
		for (auto& vgr : town->Villagers)
		{
			vgr->Manifest();
			vgr->Position = glm::vec3(30, 0, 30 + (i++ * 10));
		}
	}
	thePlayer.Position = glm::vec3(40, 0, 30);
	//thePlayer.Tops = std::make_shared<InventoryItem>("psk:oppai");
	//thePlayer.Bottoms = std::make_shared<InventoryItem>("acnh:denimcutoffs/lightblue");

	perspectiveProjection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 500.0f);
	//Orthographic projection for a laugh. For best results, use a 45 degree pitch and yaw, no bending.
	constexpr auto orthoScale = 0.025f;
	orthographicProjection = glm::ortho(-(ScreenWidth * orthoScale), (ScreenWidth * orthoScale), -(ScreenHeight * orthoScale), (ScreenHeight * orthoScale), -1.0f, 300.0f);
	commonUniforms.Projection = useOrthographic ? orthographicProjection : perspectiveProjection;

	postFxBuffer = new Framebuffer(Shaders["postfx"], width, height);
	postFxBuffer->SetLut(new Texture("colormap.png"));

#ifdef DEBUG
	auto startingTime = std::chrono::high_resolution_clock::now();
#endif

	auto oldTime = glfwGetTime();
	commonUniforms.TotalTime = 0.0f;

	tickables.push_back(musicManager);
	tickables.push_back(MainCamera);
	tickables.push_back(town);
	if (skipTitle)
		tickables.push_back(std::make_shared<InGame>());
	else
		tickables.push_back(std::make_shared<TitleScreen>());

	//auto layoutOverlay = new Texture("layoutoverlay.png");

	while (!glfwWindowShouldClose(window))
	{
		Audio::Update();

#ifdef DEBUG
		auto endingTime = std::chrono::high_resolution_clock::now();
		glTime = std::chrono::duration_cast<std::chrono::microseconds>(endingTime - startingTime).count() * 0.001f;
		startingTime = endingTime;
#endif

		Inputs.UpdateGamepad();

		auto newTime = glfwGetTime();
		auto DeltaTime = newTime - oldTime;
		oldTime = newTime;
		float dt = (float)DeltaTime;
		commonUniforms.TotalTime += dt;
		commonUniforms.DeltaTime = dt;

		commonUniforms.Projection = useOrthographic ? orthographicProjection : perspectiveProjection;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		//glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		//important: disable depth testing to allow multiple sprites to overlap.
		glDisable(GL_DEPTH_TEST);

		if (console->visible)
			console->Tick(dt);
		else
		{
			//a bit ugly but technically still better vis-a-vis iterators
			//for (auto t = tickables.crbegin(); t != tickables.crend(); ++t)
			//	(*t)->Tick(dt);
			RevAllTickables(tickables, dt);
		}

		glBindBuffer(GL_UNIFORM_BUFFER, commonBuffer);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CommonUniforms), &commonUniforms);
		
#ifdef DEBUG
		endingTime = std::chrono::high_resolution_clock::now();
		uiTime = std::chrono::duration_cast<std::chrono::milliseconds>(endingTime - startingTime).count() * 0.001f;
		startingTime = endingTime;
#endif

		auto pitch = MainCamera->Angles().y;
		if (pitch > 180) pitch -= 360;
		Shaders["sky"]->Set("pitch", pitch);
		cloudImage.Use(1);
		starsImage.Use(2);
		skyImage.Use(3);

		DrawAllTickables(tickables, dt * timeScale);

		//Sprite::DrawSprite(*layoutOverlay, glm::vec2(0), glm::vec2(width, height), glm::vec4(0), 0.0f, glm::vec4(1, 1, 1, 0.5));

		console->Draw(dt);
		Sprite::FlushBatch();

		tickables.erase(std::remove_if(tickables.begin(), tickables.end(), [](TickableP i) {
			return i->dead;
		}), tickables.end());
		if (newTickables.size() > 0)
		{
			for (const auto& t : newTickables)
				tickables.push_back(t);
			newTickables.clear();
		}

		//TEST
		if (Inputs.KeyDown(Binds::Accept))
		{
			Inputs.Clear(Binds::Accept);
			static std::string lols[] = {
				"Whatever you say.",
				"You got crabs!",
				"And bingo!",
				"T minus 50 seconds 'til launch.",
				"Four score and seven apples ago...",
				"Look. If you had ONE shot...",
				"---Tom Nook calling---",
			};
			messager->Add(lols[rnd::getInt(7)]);
		}

#ifdef DEBUG
		DoImGui();
#endif

		//turn depth testing back on for 3D shit
		//glEnable(GL_DEPTH_TEST);

		cursor->Draw();
		Sprite::FlushBatch();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	thePlayer.Save();
	town->Save();
	UI::Save();

	glfwTerminate();
	return 0;
}

extern "C" int __stdcall WinMain(struct HINSTANCE__*, struct HINSTANCE__*, char*, int) { return main(__argc, __argv); }
