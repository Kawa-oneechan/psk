#include <filesystem>
#include <chrono>
#include "SpecialK.h"

#include "support/glad/glad.h"
#include <GLFW/glfw3.h>

#include "Console.h"
#include "InputsMap.h"
#include "Cursor.h"
#include "Background.h"
#include "DoomMenu.h"
#include "DialogueBox.h"
#include "PanelLayout.h"
#include "DateTimePanel.h"
#include "ItemHotbar.h"
#include "Town.h"
#include "MusicManager.h"
#include "Framebuffer.h"

#include <fstream>

constexpr auto WindowTitle = "Project Special K"
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

GLFWwindow* window;

Shader* spriteShader = nullptr;
Shader* modelShader = nullptr;
Texture* whiteRect = nullptr;
DialogueBox* dlgBox = nullptr;
CursorP cursor = nullptr;
Console* console = nullptr;
Audio* bgm = nullptr;

glm::vec4 lightPos[MaxLights] = { { 0, 0, 0, 0 } };
glm::vec4 lightCol[MaxLights] = { { 0, 0, 0, 0 } };

sol::state Sol;

int width = ScreenWidth, height = ScreenHeight;
float scale = height / 1080.0f;

float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;

bool wireframe = false;
bool postFx = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float timeScale = 0.25f;

int articlePlease;

bool cheatsEnabled;

#ifdef DEBUG
float uiTime = 0;
float glTime = 0;
#endif

CommonUniforms commonUniforms;

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

	static void Load()
	{
		auto doc = VFS::ReadJSON("ui/ui.json");
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
		DS("language", (int)Language::USen);
		DS("continue", (int)LoadSpawnChoice::FrontDoor);
		DS("speech", (int)DialogueBox::Sound::Bebebese);
		DS("pingRate", 3);
		DS("balloonChance", 15);
		DS("cursorScale", 100);
		DS("24hour", true);
		DS("contentFilters", JSONObject());
		DS("musicVolume", 0.7f);
		DS("ambientVolume", 0.5f);
		DS("soundVolume", 1.0f);
		DS("speechVolume", 1.0f);
		DS("keyBinds", JSONArray());
		DS("gamepadBinds", JSONArray());
#undef DS

		width = settings["screenWidth"]->AsInteger();
		height = settings["screenHeight"]->AsInteger();

		constexpr Language opt2lan[] = { Language::USen, Language::JPja, Language::EUde, Language::EUes, Language::EUfr, Language::EUit, Language::EUhu, Language::EUnl, Language::EUen };
		gameLang = opt2lan[settings["language"]->AsInteger()];

		Audio::MusicVolume = settings["musicVolume"]->AsNumber();
		Audio::AmbientVolume = settings["ambientVolume"]->AsNumber();
		Audio::SoundVolume = settings["soundVolume"]->AsNumber();
		Audio::SpeechVolume = settings["speechVolume"]->AsNumber();

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

		settings["musicVolume"] = new JSONValue(Audio::MusicVolume);
		settings["ambientVolume"] = new JSONValue(Audio::AmbientVolume);
		settings["soundVolume"] = new JSONValue(Audio::SoundVolume);
		settings["speechVolume"] = new JSONValue(Audio::SpeechVolume);

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

std::vector<Tickable*> tickables;

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	window;
	::width = width;
	::height = height;
	scale = ::height / 1080.0f;
	glViewport(0, 0, width, height);
	commonUniforms.screenRes = glm::uvec2(width, height);
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

	if (key == GLFW_KEY_F2 && action == GLFW_PRESS)
		postFx = !postFx;
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

	if (Inputs.MouseHoldMiddle && !MainCamera.Locked)
	{
		auto angles = MainCamera.GetAngles();
		angles.z -= xoffset;
		angles.y -= yoffset;
		MainCamera.Angles(angles);
	}
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
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
		glfwWindowHint(GLFW_DECORATED, 0);
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

class TemporaryTownDrawer : public Tickable
{
public:
	void Draw(float dt)
	{
		Sprite::FlushBatch();

		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
		modelShader->Use();

		for (int i = 0; i < MaxLights; i++)
		{
			modelShader->Set(fmt::format("lights[{}].color", i), lightCol[i]);
			modelShader->Set(fmt::format("lights[{}].pos", i), lightPos[i]);
		}

		for (const auto& v : town.Villagers)
			v->Draw(dt * timeScale);

		glDisable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
};


float degreesLeft(float startDeg, float endDeg)
{
	return glm::mod(endDeg - startDeg, 360.0f);
}

float degreesRight(float startDeg, float endDeg)
{
	return glm::mod(startDeg - endDeg, 360.0f);
}

int main(int, char**)
{
	setlocale(LC_ALL, "en_US.UTF-8");
	std::srand((unsigned int)std::time(nullptr));

	console = new Console();
	try
	{
		VFS::Initialize();
	}
	catch (std::runtime_error& x)
	{
		FatalError(x.what());
	}

	Audio::Initialize();
	SolBinds::Setup();

	UI::Load();

	glfwInit();

	//test
	/*
	{
		town.StartNewDay();
	}
	*/

	if (auto r = InitOpenGL())
		return r;

	spriteShader = new Shader("shaders/sprite.fs");
	modelShader = new Shader("shaders/model.vs", "shaders/model.fs");
	auto skyShader = new Shader("shaders/sky.fs");
	whiteRect = new Texture("white.png", GL_CLAMP_TO_EDGE);
	UI::controls = std::make_shared<Texture>("ui/controls.png");

	GLuint commonBind = 1, commonBuffer = 0;
	glGenBuffers(1, &commonBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, commonBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(commonUniforms), &commonUniforms, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, commonBind, commonBuffer);

#ifdef DEBUG
	SetupImGui();
#endif

	cursor = std::make_shared<Cursor>();

	Inputs.HaveGamePad = (glfwJoystickPresent(GLFW_JOYSTICK_1) && glfwJoystickIsGamepad(GLFW_JOYSTICK_1));

	ThreadedLoader(Database::LoadGlobalStuff);

	thePlayer.Name = "Kawa";
	thePlayer.Gender = Gender::BEnby;

	//Now that we've loaded the key names we can fill in some blanks.
	for (int i = 0; i < NumKeyBinds; i++)
		Inputs.Keys[i].Name = GetKeyName(Inputs.Keys[i].ScanCode);

	tickables.push_back(&musicManager);
	//auto background = Background("discobg2.png");
	//tickables.push_back(new Background("discobg2.png"));
	//tickables.push_back(new TemporaryTownDrawer());
	auto townDrawer = TemporaryTownDrawer();
	tickables.push_back(new DateTimePanel());
	tickables.push_back(new ItemHotbar());
	//hotbar->Tween(&hotbar->Position.y, -100.0f, 0, 0.002f, glm::bounceEaseOut<float>);
	//hotbar->Tween(&hotbar->Alpha, 0, 0.75f, 0.006f);
	//auto logoJson = VFS::ReadJSON("cinematics/logo/logo.json")->AsObject();
	//auto logoAnim = new PanelLayout(logoJson["logoPanels"]);
	//tickables.push_back(logoAnim);
	//tickables.push_back(new DoomMenu());
	dlgBox = new DialogueBox();
	tickables.push_back(dlgBox);

	//tickables.push_back(new TextField());

#ifdef DEBUG
	RunTests();
#endif

	modelShader->Use();
	modelShader->Set("viewPos", MainCamera.Position());
	modelShader->Set("albedoTexture", 0);
	modelShader->Set("normalTexture", 1);
	modelShader->Set("mixTexture", 2);
	modelShader->Set("opacityTexture", 3);

	skyShader->Use();
	skyShader->Set("cloudImage", 1);
	skyShader->Set("starsImage", 2);
	auto cloudImage = Texture("altostratus.png");
	auto starsImage = Texture("starfield.png");

	auto rainLayer = Background("rain.png", glm::vec2(1.0, 2.0));

	/*
	auto bob = Database::Find<Villager>("ac:cat00", villagers);
	bob->defaultClothingID = "acnh:djkklogotee/neonpink"; //-V519 this is on purpose daijoubu
	bob->Manifest();
	town.Villagers.push_back(bob);
	*/
	auto cat01 = Database::Find<Villager>("ac:cat01", villagers);
	cat01->Manifest();
	town.Villagers.push_back(cat01);

	if (!LoadLights("lights/initial.json").empty())
	{
		lightPos[0] = { 0, 15, 20, 0 };
		lightCol[0] = { 1, 1, 1, 0.25 };
	}
	if (!LoadCamera("cameras/field.json").empty())
	{
		MainCamera.Set(glm::vec3(0, 0, -6), glm::vec3(0, 110, 0), 60);
	}

	{
		//commonUniforms.Projection = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);
		//glBindBuffer(GL_UNIFORM_BUFFER, commonBuffer);
		auto p = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(CommonUniforms, Projection), sizeof(glm::mat4), &p);
	}

	auto frameBuffer = Framebuffer("shaders/framebuffer.fs", width, height);

#ifdef DEBUG
	auto startingTime = std::chrono::high_resolution_clock::now();
#endif

	int oldTime = 0;
	commonUniforms.totalTime = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		Audio::Update();

#ifdef DEBUG
		auto endingTime = std::chrono::high_resolution_clock::now();
		glTime = std::chrono::duration_cast<std::chrono::microseconds>(endingTime - startingTime).count() * 0.001f;
		startingTime = endingTime;
#endif

		Inputs.UpdateGamepad();

		int newTime = std::clock();
		int deltaTime = newTime - oldTime;
		oldTime = newTime;
		float dt = (float)deltaTime;
		commonUniforms.totalTime += dt;

		glClear(GL_DEPTH_BUFFER_BIT);

		//important: disable depth testing to allow multiple sprites to overlap.
		glDisable(GL_DEPTH_TEST);

		if (console->visible)
			console->Tick(dt);
		else
		{
			//a bit ugly but technically still better vis-a-vis iterators
			for (auto t = tickables.crbegin(); t != tickables.crend(); ++t)
				(*t)->Tick(dt);
		}

		{
			glBufferSubData(GL_UNIFORM_BUFFER, offsetof(CommonUniforms, totalTime), sizeof(float), &commonUniforms.totalTime);
			glBufferSubData(GL_UNIFORM_BUFFER, offsetof(CommonUniforms, deltaTime), sizeof(float), &dt);
			glBufferSubData(GL_UNIFORM_BUFFER, offsetof(CommonUniforms, screenRes), sizeof(float), &commonUniforms.screenRes);
			//commonUniforms.View = MainCamera.ViewMat();
			auto v = MainCamera.ViewMat();
			glBufferSubData(GL_UNIFORM_BUFFER, offsetof(CommonUniforms, View), sizeof(glm::mat4), &v);
		}
		
#ifdef DEBUG
		endingTime = std::chrono::high_resolution_clock::now();
		uiTime = std::chrono::duration_cast<std::chrono::milliseconds>(endingTime - startingTime).count() * 0.001f;
		startingTime = endingTime;
#endif

		auto pitch = MainCamera.Angles().y;
		if (pitch > 180) pitch -= 360;
		skyShader->Use();
		skyShader->Set("pitch", pitch);
		cloudImage.Use(1);
		starsImage.Use(2);

		//TEST TEST TEST
		if (town.Villagers.size() != 0)
		{
			auto mitz = town.Villagers[0];
			auto facing = mitz->Facing;
			auto anythingPressed = false;
			
			if (Inputs.Keys[(int)Binds::WalkS].State == 1)
			{
				facing = 0.0;
				if (Inputs.Keys[(int)Binds::WalkE].State == 1)
					facing = 45.0;
				else if (Inputs.Keys[(int)Binds::WalkW].State == 1)
					facing = 315.0; //-45
				anythingPressed = true;
			}
			else if (Inputs.Keys[(int)Binds::WalkN].State == 1)
			{
				facing = 180.0;
				if (Inputs.Keys[(int)Binds::WalkE].State == 1)
					facing = 135.0;
				else if (Inputs.Keys[(int)Binds::WalkW].State == 1)
					facing = 225.0; //-135
				anythingPressed = true;
			}
			else if (Inputs.Keys[(int)Binds::WalkE].State == 1)
			{
				facing = 90.0;
				anythingPressed = true;
			}
			else if (Inputs.Keys[(int)Binds::WalkW].State == 1)
			{
				facing = 270.0f; //-90
				anythingPressed = true;
			}

			if (anythingPressed)
			{
				facing += MainCamera.Angles().z;

				auto m = mitz->Facing;
				if (m < 0) m += 360.0f;

				auto cw = facing - m;
				if (cw < 0.0) cw += 360.0f;
				auto ccw = m - facing;
				if (ccw < 0.0) ccw += 360.0f;
				
				auto t = (ccw < cw) ? -glm::min(10.0f, ccw) : glm::min(10.0f, cw);
				
				auto f = m + t;
				if (f < 0) f += 360.0f;

				mitz->Facing = glm::mod(f, 360.0f);
			}
		}

		if (postFx)
		{
			frameBuffer.Use();
			glClearColor(0.2f, 0.3f, 0.3f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			Sprite::DrawSprite(skyShader, *whiteRect, glm::vec2(0), glm::vec2(width, height));
			townDrawer.Draw(dt * timeScale);
			rainLayer.Draw(dt * timeScale);
			frameBuffer.Drop();
			//background.Draw(dt * timeScale);
			frameBuffer.Draw();
		}
		else
		{
			//background.Draw(dt * timeScale);
			Sprite::DrawSprite(skyShader, *whiteRect, glm::vec2(0), glm::vec2(width, height));
			townDrawer.Draw(dt * timeScale);
			rainLayer.Draw(dt * timeScale);
		}

		for (const auto& t : tickables)
			t->Draw(dt * timeScale);

		console->Draw(dt);
		Sprite::FlushBatch();

		tickables.erase(std::remove_if(tickables.begin(), tickables.end(), [](Tickable* i) {
			return i->dead;
		}), tickables.end());

#ifdef DEBUG
		DoImGui();
#endif

		//turn depth testing back on for 3D shit
		glEnable(GL_DEPTH_TEST);

		cursor->Draw();
		Sprite::FlushBatch();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	UI::Save();

	glfwTerminate();
	return 0;
}

extern "C" int __stdcall WinMain(struct HINSTANCE__*, struct HINSTANCE__*, char*, int) { return main(__argc, __argv); }
