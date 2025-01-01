#include "TitleScreen.h"
#include "MusicManager.h"
#include "Background.h"
#include "InputsMap.h"
#include "PanelLayout.h"
#include "DoomMenu.h"
#include "DateTimePanel.h"
#include "ItemHotbar.h"
#include "DialogueBox.h"
#include "Town.h"
#include "Iris.h"

extern std::vector<Tickable*> tickables;
extern std::vector<Tickable*> newTickables;

static std::string psText;
static glm::vec2 psSize;
static PanelLayout* logoAnim;
static DoomMenu* optionsMenu;

TitleScreen::TitleScreen()
{
	tickables.clear();
	tickables.push_back(&musicManager);
	tickables.push_back(&MainCamera);
	//tickables.push_back(new Background("discobg2.png"));
	tickables.push_back(&town);
	//tickables.push_back(iris);
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
}

void TitleScreen::Tick(float dt)
{
	logoAnim->Tick(dt);
	iris->Tick(dt);

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
		if (!optionsMenu)
		{
			if (Inputs.KeyDown(Binds::Accept))
			{
				Inputs.Clear();
				state = State::FadeOut;
				musicManager.FadeOut();
				iris->Out();
			}
			else if (Inputs.KeyDown(Binds::Back))
			{
				Inputs.Clear();
				optionsMenu = new DoomMenu();
			}
		}
		else
		{
			optionsMenu->Tick(dt);
			if (optionsMenu->dead)
			{
				delete optionsMenu;
				optionsMenu = nullptr;
			}
		}
	}
	else if (state == State::FadeOut)
	{
		if (iris->Done())
		{
			dead = true;
			dateTimePanel = new DateTimePanel();
			newTickables.push_back(dateTimePanel);
			newTickables.push_back(itemHotbar);
			newTickables.push_back(dlgBox);
			musicManager.Play(town.Music);
			LoadCamera("cameras/field.json");
			MainCamera.Target(&(thePlayer.Position));
			//dlgBox->Text(fmt::format("<color:1>Your NookCode is <color:2>{}<color:1>.", NookCode::Encode(0x543CA469, 0, 0)), 3);
			iris->In();
		}
	}
}

TitleScreen::~TitleScreen()
{
	delete logoAnim;
	delete optionsMenu;
}

void TitleScreen::Draw(float dt)
{
	if (!optionsMenu)
	{
		logoAnim->Draw(dt);
		//if (logoAnim->Playing())
		Sprite::DrawText(1, psText, (glm::vec2(width, height) - psSize) * glm::vec2(0.5f, 0.86f), glm::vec4(1, 1, 1, glm::abs(glm::sin((float)glfwGetTime())) * 1.0f), 150.0f * scale);
	}
	else
	{
		optionsMenu->Draw(dt);
	}

#ifdef DEBUG
	Sprite::DrawText(0, "Debug build " __DATE__, glm::vec2(8, height - 24), glm::vec4(1, 1, 1, 0.5));
#endif
	iris->Draw(dt);
}

