#include "TitleScreen.h"
#include "MusicManager.h"
#include "InputsMap.h"
#include "PanelLayout.h"
#include "DoomMenu.h"
#include "Town.h"
#include "InGame.h"
#include "Iris.h"

extern std::vector<Tickable*> tickables;
extern std::vector<Tickable*> newTickables;

static std::string psText;
static glm::vec2 psSize;
static PanelLayout* logoAnim;
static DoomMenu* optionsMenu;
static Iris* iris;

TitleScreen::TitleScreen()
{
	tickables.clear();
	auto logoJson = VFS::ReadJSON("cinematics/logo/logo.json")->AsObject();
	logoAnim = new PanelLayout(logoJson["cinematic"]);

	{
		auto logoJoke = logoAnim->GetPanel("logoJoke");
		int options = 0;
		for (int i = 0; i < 100; i++)
		{
			auto result = Text::Get(fmt::format("logojoke:{}", i));
			if (result.length() >= 3 && result.substr(0, 3) == "???")
			{
				options = i;
				break;
			}
		}
		if (options == 0)
		{
			conprint(2, "TitleScreen: could not find a logo joke.");
			logoJoke->Text = "???";
		}
		else
		{
			int choice = std::rand() % options;
			logoJoke->Text = Text::Get(fmt::format("logojoke:{}", choice));
		}
		Text::Forget("logojoke:");
	}
	
	LoadCamera("cameras/title.json");

	auto key = Inputs.Keys[(int)Binds::Accept];
	psText = fmt::format(Text::Get("title:pressstart"), key.Name, GamepadPUAMap[key.GamepadButton]);
	psSize = Sprite::MeasureText(1, psText, 100);

	optionsMenu = new DoomMenu();
	optionsMenu->Enabled = false;
	optionsMenu->Visible = false;

	iris = new Iris();

	tickables.push_back(logoAnim);
	tickables.push_back(optionsMenu);
	tickables.push_back(iris);
}

bool TitleScreen::Tick(float dt)
{
	RevAllTickables(tickables, dt);

	if (state == State::Init)
	{
		musicManager.Play("title", true);
		state = State::FadeIn;
		//MainCamera.Target(&(town.Villagers[0]->Position));
		iris->In();
	}
	else if (state == State::FadeIn)
	{
		if (iris->Done())
		{
			state = State::Wait;
			logoAnim->Play("open");
		}
	}
	else if (state == State::Wait)
	{
		if (!optionsMenu->Visible)
		{
			if (Inputs.KeyDown(Binds::Accept))
			{
				state = State::FadeOut;
				musicManager.FadeOut();
				iris->Out();
				return false;
			}
			else if (Inputs.KeyDown(Binds::Back))
			{
				optionsMenu->Visible = true;
				optionsMenu->Enabled = true;
				return false;
			}
		}
		else
		{
			if (optionsMenu->dead)
			{
				optionsMenu->dead = false;
				optionsMenu->Visible = false;
				optionsMenu->Enabled = false;
				return false;
			}
		}
	}
	else if (state == State::FadeOut)
	{
		if (iris->Done())
		{
			dead = true;
			::tickables.push_back(new InGame());
		}
	}

	return true;
}

TitleScreen::~TitleScreen()
{
	delete logoAnim;
	delete optionsMenu;
	delete iris;
}

void TitleScreen::Draw(float dt)
{
	DrawAllTickables(tickables, dt);

	if (!optionsMenu->Visible)
	{
		Sprite::DrawText(1, psText, (glm::vec2(width, height) - psSize) * glm::vec2(0.5f, 0.86f), glm::vec4(1, 1, 1, glm::abs(glm::sin((float)glfwGetTime())) * 1.0f), 150.0f * scale);
	}

#ifdef DEBUG
	Sprite::DrawText(0, "Debug build " __DATE__, glm::vec2(8, height - 24), glm::vec4(1, 1, 1, 0.5));
#endif
	iris->Draw(dt);
}
