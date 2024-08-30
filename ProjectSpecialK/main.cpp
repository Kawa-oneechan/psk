#include <filesystem>
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
#include "Town.h"
#include "MusicManager.h"

#include <thread>
#include <future>
#include <fstream>

constexpr auto WindowTitle = "Project Special K"
#ifdef DEBUG
" (debug build " __DATE__ ")";
#endif

constexpr int ScreenWidth = 1920;
constexpr int ScreenHeight = 1080;
constexpr int WindowWidth = 1920; //1280
constexpr int WindowHeight = 1080; //720

extern "C"
{
	//why bother including windows headers lol
	int __stdcall MessageBoxW(_In_opt_ void* hWnd, _In_opt_ const wchar_t* lpText, _In_opt_ const wchar_t* lpCaption, _In_ unsigned int uType);
	int __stdcall MultiByteToWideChar(_In_ unsigned int CodePage, _In_ unsigned long dwFlags, const char* lpMultiByteStr, _In_ int cbMultiByte, wchar_t* lpWideCharStr, _In_ int cchWideChar);
}

GLFWwindow* window;

Shader* spriteShader = nullptr;
Shader* modelShader = nullptr;
Texture* whiteRect = nullptr;
//Camera camera(glm::vec3(0.0f, 5.0f, 50.0f));
DialogueBox* dlgBox = nullptr;
CursorP cursor = nullptr;
Console* console = nullptr;
Audio* bgm = nullptr;

glm::vec3 lightPos(0.0f, 15.0f, 20.0f);

sol::state Sol;

float lastX = ScreenWidth / 2.0f;
float lastY = ScreenHeight / 2.0f;
bool firstMouse = true;

bool wireframe = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float width = ScreenWidth, height = ScreenHeight;
float scale = (float)WindowWidth / (float)WindowHeight;

float timeScale = 0.25f;

int articlePlease;

__declspec(noreturn)
void FatalError(const std::string& message)
{
	conprint(1, "Fatal error: {}", message);

	wchar_t w[1024] = { 0 };
	MultiByteToWideChar(65001, 0, message.c_str(), -1, w, 1024);
	MessageBoxW(nullptr, w, L"Project Special K", 0x30);

	conprint(1, "Exiting...");
	exit(1);
}

namespace SolBinds
{
	extern void Setup();
}

extern void RunTests();

namespace UI
{
	std::map<std::string, glm::vec4> themeColors;
	std::vector<glm::vec4> textColors;

	std::shared_ptr<Texture> controls{ nullptr };

	JSONObject json = JSONObject();
	JSONObject settings = JSONObject();

	static std::unique_ptr<char*> LoadFile(const std::string &filename, size_t *size)
	{
		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		if (!file.good())
			throw std::exception("Couldn't open file.");
		std::streamsize fs = file.tellg();
		file.seekg(0, std::ios::beg);
		if (size != nullptr)
			*size = fs;
		auto ret = new char[fs + 2]{ 0 };
		file.read(ret, fs);
		return std::make_unique<char*>(ret);
	}

	static void SaveFile(const std::string &filename, const std::string& content)
	{
		std::ofstream file(filename, std::ios::trunc | std::ios::binary);
		if (!file.good())
			throw std::exception("Couldn't open file.");
		file << content; //eeugh
		file.close();
	}

	static void Load()
	{
		json = VFS::ReadJSON("ui/ui.json")->AsObject();
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
			auto data = LoadFile("options.json", nullptr);
			settings = JSON::Parse(*data.get())->AsObject();
		}
		catch (std::exception&)
		{
			settings = JSON::Parse("{}")->AsObject();
		}

#define DS(K, V) if (!settings[K]) settings[K] = new JSONValue(V)
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

		constexpr Language opt2lan[] = { Language::USen, Language::JPja, Language::EUde, Language::EUes, Language::EUfr, Language::EUit, Language::EUhu, Language::EUnl, Language::EUen };
		gameLang = opt2lan[(int)settings["language"]->AsNumber()];

		Audio::MusicVolume = (float)settings["musicVolume"]->AsNumber();
		Audio::AmbientVolume = (float)settings["ambientVolume"]->AsNumber();
		Audio::SoundVolume = (float)settings["soundVolume"]->AsNumber();
		Audio::SpeechVolume = (float)settings["speechVolume"]->AsNumber();

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
			Inputs.Keys[i].ScanCode = (int)keyBinds[i]->AsNumber();
			Inputs.Keys[i].GamepadButton = (int)padBinds[i]->AsNumber();
		}

		auto sounds = VFS::ReadJSON("sound/sounds.json")->AsObject();
		for (auto category : sounds)
		{
			for (auto sound : category.second->AsObject())
				generalSounds[category.first][sound.first] = std::make_shared<Audio>(sound.second->AsString());
		}
	}

	static void Save()
	{
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
			SaveFile("options.json", JSON::Stringify(new JSONValue(settings)));
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
	window;
	::width = (float)width;
	::height = (float)height;
	scale = ::height / ScreenHeight;
	glViewport(0, 0, width, height);
}

void char_callback(GLFWwindow* window, unsigned int codepoint)
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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	window, mods;
	//TODO: make this remappable
	if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS)
	{
		if (console->visible)
			console->Close();
		else
			console->Open();
		//console->visible = !console->visible;
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

	//if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	//	glfwSetWindowShouldClose(window, 1);
	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	{
		wireframe = !wireframe;
		glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
	}

	if (key == GLFW_KEY_T)
		lightPos.y += 0.5f;
	else if (key == GLFW_KEY_G)
		lightPos.y -= 0.5f;
	else if (key == GLFW_KEY_H)
		lightPos.x += 0.5f;
	else if (key == GLFW_KEY_F)
		lightPos.x -= 0.5f;
	else if (key == GLFW_KEY_R)
		lightPos.z -= 0.5f;
	else if (key == GLFW_KEY_Y)
		lightPos.z += 0.5f;

	if (mods == 0)
	{
		if (key == GLFW_KEY_W)
			MainCamera.Position.z -= 0.5f;
		else if (key == GLFW_KEY_S)
			MainCamera.Position.z += 0.5f;
		else if (key == GLFW_KEY_A)
			MainCamera.Position.x -= 0.5f;
		else if (key == GLFW_KEY_D)
			MainCamera.Position.x += 0.5f;
		else if (key == GLFW_KEY_Q)
			MainCamera.Position.y -= 0.5f;
		else if (key == GLFW_KEY_E)
			MainCamera.Position.y += 0.5f;
	}
	else if (mods == 1)
	{
		if (key == GLFW_KEY_W)
			MainCamera.Target.z -= 0.5f;
		else if (key == GLFW_KEY_S)
			MainCamera.Target.z += 0.5f;
		else if (key == GLFW_KEY_A)
			MainCamera.Target.x -= 0.5f;
		else if (key == GLFW_KEY_D)
			MainCamera.Target.x += 0.5f;
		else if (key == GLFW_KEY_Q)
			MainCamera.Target.y -= 0.5f;
		else if (key == GLFW_KEY_E)
			MainCamera.Target.y += 0.5f;
	}
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
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
	float yoffset = lastY - ypos; //reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	if (Inputs.MouseHoldMiddle)
		MainCamera.ProcessMouseMovement(xoffset, yoffset);
}

void mousebutton_callback(GLFWwindow* window, int button, int action, int mods)
{
	window; mods;
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

void joystick_callback(int jid, int event)
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

void ThreadedLoader(std::function<void(float*)> loader)
{
	glDisable(GL_DEPTH_TEST);
	cursor->Select(1);
	auto loadIcon = Texture("loading.png");
	auto loadPos = glm::vec2(width - 256, height - 256);
	int oldTime = 0;

	auto loadProgress = 0.0f;
	auto loadPointer = &loadProgress;

	std::promise<bool> p;
	auto future = p.get_future();

	std::thread t([&p, loader, loadPointer]
	{
		loader(loadPointer);
		p.set_value(true);
	});

	const int barWidth = (int)glm::floor(width * 0.80f);
	const int barHeight = 16;
	const int barLeft = (int)glm::floor(width / 2) - (barWidth / 2);
	const int barTop = (int)glm::floor(height / 2) - (barHeight / 1);
	
	while (true)
	{
		int newTime = std::clock();
		oldTime = newTime;

		auto time = (float)glfwGetTime();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		Sprend.DrawSprite(loadIcon, loadPos, glm::vec2(128), glm::vec4(0), sinf(time) * glm::radians(1000.0f));

		Sprend.DrawSprite(*whiteRect, glm::vec2(barLeft - 1, barTop - 1), glm::vec2(barWidth + 2, barHeight + 2), glm::vec4(0), 0.0f, glm::vec4(1, 1, 1, 1));
		Sprend.DrawSprite(*whiteRect, glm::vec2(barLeft, barTop), glm::vec2(barWidth, barHeight), glm::vec4(0), 0.0f, glm::vec4(0.25, 0.25, 0.25, 1));
		Sprend.DrawSprite(*whiteRect, glm::vec2(barLeft, barTop), glm::vec2(barWidth * loadProgress, barHeight), glm::vec4(0), 0.0f, glm::vec4(1, 1, 1, 1));

		cursor->Draw();
		Sprend.Flush();

		glfwSwapBuffers(window);
		glfwPollEvents();

		auto status = future.wait_for(std::chrono::milliseconds(1));
		if (status == std::future_status::ready)
			break;

	}
	t.join();
	cursor->Select(0);
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

	glfwInit();

	Audio::Initialize();
	SolBinds::Setup();

	UI::Load();

	//test
	{
		town.StartNewDay();
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 2);

	if (WindowWidth == ScreenWidth)
	{
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		if (mode->width == ScreenWidth && mode->height == ScreenHeight)
			glfwWindowHint(GLFW_DECORATED, 0);
		glfwWindowHint(GLFW_RESIZABLE, 0);
	}

	window = glfwCreateWindow(WindowWidth, WindowHeight, WindowTitle, NULL, NULL);
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

	framebuffer_size_callback(window, WindowWidth, WindowHeight);

	spriteShader = new Shader("shaders/sprite.fs");
	modelShader = new Shader("shaders/model.vs", "shaders/model.fs");
	whiteRect = new Texture("white.png", GL_CLAMP_TO_EDGE);
	UI::controls = std::make_shared<Texture>("ui/controls.png");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glFrontFace(GL_CW);

	//glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	//Required for sprites
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	cursor = std::make_shared<Cursor>();

	Inputs.HaveGamePad = (glfwJoystickPresent(GLFW_JOYSTICK_1) && glfwJoystickIsGamepad(GLFW_JOYSTICK_1));

	ThreadedLoader(Database::LoadGlobalStuff);


	thePlayer.Name = "Kawa";
	thePlayer.Gender = Gender::BEnby;

	TextAdd(*VFS::ReadJSON("text/datetime.json"));
	TextAdd(*VFS::ReadJSON("text/fixedform.json"));
	TextAdd(*VFS::ReadJSON("text/keynames.json"));
	TextAdd(*VFS::ReadJSON("text/optionsmenu.json"));
	TextAdd(*VFS::ReadJSON("text/tests.json"));

	//Now that we've loaded the key names we can fill in some blanks.
	for (int i = 0; i < NumKeyBinds; i++)
		Inputs.Keys[i].Name = GetKeyName(Inputs.Keys[i].ScanCode);

	tickables.push_back(&musicManager);
	tickables.push_back(new Background());
	tickables.push_back(new DoomMenu());
	auto hotbar = new PanelLayout(UI::json["hotbar"]);
	tickables.push_back(hotbar);
	hotbar->Tween(&hotbar->Position.y, -100.0f, 0, 0.002f, glm::bounceEaseOut<float>);
	hotbar->Tween(&hotbar->Alpha, 0, 0.75f, 0.006f);
	tickables.push_back(new DateTimePanel());
	//auto logoJson = ReadJSON("cinematics/logo/logo.json")->AsObject();
	//auto logoAnim = new PanelLayout(logoJson["logoPanels"]);
	//tickables.push_back(logoAnim);
	dlgBox = new DialogueBox();
	tickables.push_back(dlgBox);

	//tickables.push_back(new TextField());

	RunTests();

	modelShader->Use();
	modelShader->SetVec3("lights[0].color", 1.0f, 1.0f, 1.0f);
	modelShader->SetVec3("lights[0].pos", lightPos);
	modelShader->SetFloat("lights[0].strength", 0.25f);
	modelShader->SetVec3("viewPos", MainCamera.Position);
	modelShader->SetInt("albedoTexture", 0);
	modelShader->SetInt("normalTexture", 1);
	modelShader->SetInt("mixTexture", 2);
	modelShader->SetInt("opacityTexture", 3);

	auto bob = Database::Find<Villager>("psk:cat00", villagers);
	bob->defaultOutfitID = "psk:oppai/white";
	bob->defaultOutfitID = "acnh:djkklogotee/neonpink";
	bob->Manifest();

	MainCamera.Setup(glm::vec3(0.0f, 5.0f, 50.0f));
	MainCamera.Free = false;
	MainCamera.Target = glm::vec3(0);

	int oldTime = 0;
	while (!glfwWindowShouldClose(window))
	{
		Audio::Update();
		Inputs.UpdateGamepad();

		int newTime = std::clock();
		int deltaTime = newTime - oldTime;
		oldTime = newTime;
		float dt = (float)deltaTime;

		if (wireframe)
		{
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
		else
		{
			glClear(GL_DEPTH_BUFFER_BIT);
		}

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

		for (const auto& t : tickables)
			t->Draw(dt * timeScale);

		Sprend.Flush();
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		modelShader->Use();
		modelShader->SetVec3("lights[0].pos", lightPos);
		//testModel.Draw();
		//testModel2.Draw();
		bob->Draw(dt * timeScale);
		glDisable(GL_DEPTH_TEST);

		Sprend.DrawText(0, fmt::format("CAMERA\n------\nPos: {} {} {}\nPit/Yaw: {} {}\nTarget: {} {} {}\n\nLIGHT\n-----\nPos: {} {} {}",
			MainCamera.Position.x, MainCamera.Position.y, MainCamera.Position.z,
			MainCamera.Pitch, MainCamera.Yaw,
			MainCamera.Target.x, MainCamera.Target.y, MainCamera.Target.z,
			lightPos.x, lightPos.y, lightPos.z
		), glm::vec2(8), glm::vec4(1, 1, 0, 1));

		console->Draw(dt);
		cursor->Draw();

		Sprend.Flush();

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
