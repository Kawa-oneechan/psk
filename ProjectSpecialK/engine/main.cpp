#include <filesystem>
#include <fstream>
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <sol.hpp>
#include "Platform.h"
#include "Tickable.h"
#include "Audio.h"
#include "Console.h"
#include "Cursor.h"
#include "InputsMap.h"
#include "TextUtils.h"
#include "Texture.h"
#include "Utilities.h"
#include "Shader.h"
#include "SpriteRenderer.h"
#include "Text.h"
#include "VFS.h"
#include "Game.h"
#include "../Game.h"

constexpr auto WindowTitle = BECKETT_GAMENAME " - " BECKETT_VERSIONJOKE
#ifdef DEBUG
	" (debug build " __DATE__ ")";

extern bool IsImGuiHovered();
extern void SetupImGui();
extern void DoImGui();
#endif
;

constexpr int ScreenWidth = BECKETT_SCREENWIDTH;
constexpr int ScreenHeight = BECKETT_SCREENHEIGHT;

glm::mat4 perspectiveProjection, orthographicProjection;
bool useOrthographic = false;

GLFWwindow* window;
sol::state Sol;

Texture* whiteRect = nullptr;
CursorP cursor = nullptr;
Console* console = nullptr;

int width = ScreenWidth, height = ScreenHeight;
float scale = height / (float)ScreenHeight;

float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;

bool wireframe = false;

double DeltaTime = 0.0;
float timeScale = 1.0f;
bool cheatsEnabled;

#ifdef DEBUG
float uiTime = 0;
float glTime = 0;
#endif

CommonUniforms commonUniforms;
unsigned int commonBuffer = 0;

__declspec(noreturn)
	void FatalError(const std::string& message)
{
	conprint(1, "Fatal error: {}", message);
	Platform::MessageBox(message);
	conprint(1, "Exiting...");
	exit(1);
}

//Currently active Tickables.
std::vector<TickableP> rootTickables;
//Tickables to add next cycle.
std::vector<TickableP> newTickables;


namespace UI
{
	std::map<std::string, glm::vec4> themeColors;
	std::vector<glm::vec4> textColors;

	jsonValue json;
	jsonValue settings;

	std::string initFile = "init.json";

	void Load()
	{
		json = VFS::ReadJSON("ui/ui.json");
		if (!json)
			FatalError("Could not read ui/ui.json. Something is very wrong.");
		auto js = json.as_object();

		auto thC = js["colors"].as_object()["theme"].as_object();
		std::for_each(thC.cbegin(), thC.cend(), [&](auto ink)
		{
			themeColors[ink.first] = GetJSONColor(ink.second);
		});
		auto txC = js["colors"].as_object()["text"].as_array();
		std::for_each(txC.cbegin(), txC.cend(), [&](auto ink)
		{
			textColors.push_back(GetJSONColor(ink));
		});

		try
		{
			settings = VFS::ReadSaveJSON("options.json");
		}
		catch (std::runtime_error&)
		{
			settings = json5pp::parse5("{}");
		}

		auto sets = settings.as_object();

#define DS(K, V) if (!sets[K]) sets[K] = jsonValue(V)
#define DA(K, V) if (!sets[K]) sets[K] = json5pp::array(V)
		DS("screenWidth", ScreenWidth);
		DS("screenHeight", ScreenHeight);
		DA("keyBinds", {});
		DA("gamepadBinds", {});
		DS("gamepadRunThreshold", Inputs.RunThreshold);
		DS("language", "USen");
		DS("musicVolume", 70);
		DS("soundVolume", 100);
#ifdef BECKETT_MOREVOLUME
		DS("ambientVolume", 50);
		DS("speechVolume", 100);
#endif
#undef DA
#undef DS

		width = sets["screenWidth"].as_integer();
		height = sets["screenHeight"].as_integer();

		Inputs.RunThreshold = sets["gamepadRunThreshold"].as_number();

		gameLang = Text::GetLangCode(sets["language"].as_string());

		//Convert from saved integer values to float.
		Audio::MusicVolume = sets["musicVolume"].as_integer() / 100.0f;
		Audio::SoundVolume = sets["soundVolume"].as_integer() / 100.0f;
#ifdef BECKETT_MOREVOLUME
		Audio::AmbientVolume = sets["ambientVolume"].as_integer() / 100.0f;
		Audio::SpeechVolume = sets["speechVolume"].as_integer() / 100.0f;
#endif

		auto keyBinds = sets["keyBinds"].as_array();
		if (keyBinds.size() != NumKeyBinds)
		{
			keyBinds.reserve(NumKeyBinds);
			for (auto &k : DefaultInputBindings)
				keyBinds.emplace_back(jsonValue(glfwGetKeyScancode(k)));
		}

		auto padBinds = sets["gamepadBinds"].as_array();
		if (padBinds.size() != NumKeyBinds)
		{
			padBinds.reserve(NumKeyBinds);
			for (auto &k : DefaultInputGamepadBindings)
				padBinds.emplace_back(jsonValue(k));
		}

		for (int i = 0; i < NumKeyBinds; i++)
		{
			Inputs.Keys[i].ScanCode = keyBinds[i].as_integer();
			Inputs.Keys[i].GamepadButton = padBinds[i].as_integer();
		}

		Game::LoadSettings(sets);
	}

	void Save()
	{
		auto set = settings.as_object();
		set["screenWidth"] = width;
		set["screenHeight"] = height;

		auto binds = json5pp::array({});
		for (auto& k : Inputs.Keys)
			binds.as_array().push_back(k.ScanCode);
		set["keyBinds"] = std::move(binds);

		auto binds2 = json5pp::array({});
		for (auto& k : Inputs.Keys)
			binds2.as_array().push_back(k.GamepadButton);
		set["gamepadBinds"] = std::move(binds2);
		set["gamepadRunThreshold"] = Inputs.RunThreshold;

		//Convert from float values to easier-to-read integers.
		set["musicVolume"] = (int)(Audio::MusicVolume * 100.0f);
		set["soundVolume"] = (int)(Audio::SoundVolume * 100.0f);
#ifdef BECKETT_MOREVOLUME
		set["ambientVolume"] = (int)(Audio::AmbientVolume * 100.0f);
		set["speechVolume"] = (int)(Audio::SpeechVolume * 100.0f);
#endif

		Game::SaveSettings(set);

		try
		{
			VFS::WriteSaveJSON("options.json", settings);
		}
		catch (std::exception&)
		{
			conprint(2, "Couldn't save settings.");
		}
	}
};

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	window;
	::width = width;
	::height = height;
	scale = ::height / (float)ScreenHeight;
	glViewport(0, 0, width, height);
	commonUniforms.ScreenRes = glm::uvec2(width, height);

	perspectiveProjection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 500.0f);
	constexpr auto orthoScale = 0.025f;
	orthographicProjection = glm::ortho(-((float)width * orthoScale), ((float)width * orthoScale), -((float)height * orthoScale), ((float)height * orthoScale), -1.0f, 300.0f);
	commonUniforms.Projection = useOrthographic ? orthographicProjection : perspectiveProjection;

	Game::OnResize();
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
	for (unsigned int i = (unsigned int)rootTickables.size(); i-- > 0; )
	{
		auto t = rootTickables[i];
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

	Inputs.Shift = (mods & GLFW_MOD_SHIFT) != 0;
	Inputs.Control = (mods & GLFW_MOD_CONTROL) != 0;
	Inputs.Alt = (mods & GLFW_MOD_ALT) != 0;

	Inputs.Process(scancode, action);

	for (unsigned int i = (unsigned int)rootTickables.size(); i-- > 0; )
	{
		auto t = rootTickables[i];
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

	Game::OnMouse(xposIn, yposIn, xoffset, yoffset);
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

static void confirmGamepad(int jid)
{
	conprint(0, "Gamepad connected: {}", glfwGetJoystickName(jid));
	int axes = 0, buttons = 0, hats = 0;
	glfwGetJoystickAxes(jid, &axes);
	glfwGetJoystickButtons(jid, &buttons);
	glfwGetJoystickHats(jid, &hats);
	if (axes < 2 || buttons < 6 || hats < 1)
	{
		conprint(2, "Rejecting gamepad: not enough inputs ({}/2 axes, {}/6 buttons, {}/1 hats)", axes, buttons, hats);
		Inputs.HaveGamePad = false;
	}
}

static void joystick_callback(int jid, int event)
{
	if (event == GLFW_CONNECTED)
	{
		Inputs.HaveGamePad = (jid == GLFW_JOYSTICK_1 && glfwJoystickIsGamepad(jid));
		if (Inputs.HaveGamePad)
			confirmGamepad(jid);
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
#ifdef BECKETT_MSAA
	glfwWindowHint(GLFW_SAMPLES, 4);
#else
	glfwWindowHint(GLFW_SAMPLES, 0); //Disable
#endif

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
#ifdef BECKETT_RESIZABLE
	glfwWindowHint(GLFW_RESIZABLE, 1);
#else
	glfwWindowHint(GLFW_RESIZABLE, 0);
#endif

	window = glfwCreateWindow(width, height, WindowTitle, NULL, NULL);
	if (window == NULL)
	{
		glfwTerminate();
		FatalError("Failed to create GLFW window.");
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

bool skipTitle = false;

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "en_US.UTF-8");

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

	UI::Load();

	if (auto r = InitOpenGL())
		return r;

#ifdef DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
#endif

	whiteRect = new Texture("white.png", GL_CLAMP_TO_EDGE);

	Shader::LoadAll();

	GLuint commonBind = 1;
	glGenBuffers(1, &commonBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, commonBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(commonUniforms), &commonUniforms, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, commonBind, commonBuffer);

#ifdef DEBUG
	SetupImGui();
#endif

	cursor = std::make_shared<Cursor>();
	Audio::Initialize();

	Game::Initialize();

	Inputs.HaveGamePad = (glfwJoystickPresent(GLFW_JOYSTICK_1) && glfwJoystickIsGamepad(GLFW_JOYSTICK_1));
	if (Inputs.HaveGamePad)
		confirmGamepad(GLFW_JOYSTICK_1);

#ifdef DEBUG
	auto startingTime = std::chrono::high_resolution_clock::now();
#endif

	auto oldTime = glfwGetTime();
	commonUniforms.TotalTime = 0.0f;

	Game::Start(rootTickables);

	while (!glfwWindowShouldClose(window))
	{
		Audio::Update();

		Game::LoopStart();

#ifdef DEBUG
		auto endingTime = std::chrono::high_resolution_clock::now();
		glTime = std::chrono::duration_cast<std::chrono::microseconds>(endingTime - startingTime).count() * 0.001f;
		startingTime = endingTime;
#endif

		Inputs.UpdateGamepad();

		auto newTime = glfwGetTime();
		DeltaTime = newTime - oldTime;
		oldTime = newTime;
		float dt = (float)DeltaTime;
		commonUniforms.TotalTime += dt;
		commonUniforms.DeltaTime = dt;

		commonUniforms.Projection = useOrthographic ? orthographicProjection : perspectiveProjection;

		//important: disable depth testing to allow multiple sprites to overlap.
		glDisable(GL_DEPTH_TEST);

		if (console->visible)
			console->Tick(dt);
		else
			RevAllTickables(rootTickables, dt);

		glBindBuffer(GL_UNIFORM_BUFFER, commonBuffer);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CommonUniforms), &commonUniforms);

#ifdef DEBUG
		endingTime = std::chrono::high_resolution_clock::now();
		uiTime = std::chrono::duration_cast<std::chrono::milliseconds>(endingTime - startingTime).count() * 0.001f;
		startingTime = endingTime;
#endif

		Game::PreDraw(dt * timeScale);
		DrawAllTickables(rootTickables, dt * timeScale);
		Game::PostDraw(dt * timeScale);

		console->Draw(dt);
		Sprite::FlushBatch();

		rootTickables.erase(std::remove_if(rootTickables.begin(), rootTickables.end(), [](TickableP i)
		{
			return i->Dead;
		}), rootTickables.end());
		if (newTickables.size() > 0)
		{
			for (const auto& t : newTickables)
				rootTickables.push_back(t);
			newTickables.clear();
		}

#ifdef DEBUG
		DoImGui();
#endif

		cursor->Draw();
		Sprite::FlushBatch();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	Game::OnQuit();
	UI::Save();

	glfwTerminate();
	return 0;
}
