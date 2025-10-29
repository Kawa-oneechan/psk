#include "engine/Utilities.h"
#include "engine/TextUtils.h"
#include "engine/Text.h"
#include "engine/Random.h"
#include "engine/Console.h"
#include "engine/SpriteRenderer.h"
#include "TitleScreen.h"
#include "MusicManager.h"
#include "Town.h"
#include "InGame.h"
#include "Utilities.h"
#include "Player.h"

extern "C" { double glfwGetTime(void); }

extern std::vector<TickableP> newTickables;

TitleScreen::TitleScreen()
{
	RemoveAll();
	auto logoJson = VFS::ReadJSON("cinematics/logo/logo.json").as_object();
	logoAnim = std::make_shared<PanelLayout>(logoJson["cinematic"]);

	{
		auto logoJoke = logoAnim->GetPanel("logoJoke");
		auto options = Text::Count("logojoke:");
		if (options == 0)
		{
			conprint(2, "TitleScreen: could not find a logo joke.");
			logoJoke->Text = "404 logo joke not found";
		}
		else
		{
			int choice = Random::GetInt((int)options);
			logoJoke->Text = Text::Get(fmt::format("logojoke:{}", choice));
		}
		Text::Forget("logojoke:");
	}
	
	LoadCamera("cameras/title.json");

	//psText = PreprocessBJTS(Text::Get("title:pressstart"));
	//psSize = Sprite::MeasureText(1, psText, 100);
	auto playerText = fmt::format("{}\n{}", thePlayer.Name, town->Name);
	auto playerPanelWidth = (int)(Sprite::MeasureText(1, playerText, 50.0f, true).x + 128);
	playerPanel = std::make_shared<NineSlicer>("ui/roundrect.png", width - playerPanelWidth - 30, height, playerPanelWidth, 140);
	playerPanel->Scale = 1.0f;
	playerPanel->Color = UI::themeColors["dialogue"];
	playerPanel->Visible = false;

	auto label = std::make_shared<TextLabel>(playerText, glm::vec2(32, 24));
	label->Color = UI::textColors[7];
	//label->Size = 100.0f;
	playerPanel->AddChild(label);
	AddChild(playerPanel);

	optionsMenu = std::make_shared<OptionsMenu>();
	optionsMenu->Enabled = false;
	optionsMenu->Visible = false;

	iris = std::make_shared<Iris>();

	AddChild(logoAnim);
	AddChild(optionsMenu);
	AddChild(iris);
}

bool TitleScreen::Tick(float dt)
{
	Tickable::Tick(dt);

	if (state == State::Init)
	{
		musicManager->Play("title", true);
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
			pressStart = new DropLabel(PreprocessBJTS(Text::Get("title:pressstart")), 1, 150, UI::themeColors["white"], DropLabel::Style::Drop);
			playerPanel->Visible = true;
		}
	}
	else if (state == State::Wait)
	{
		//panelPop.step();
		if (panelPop < 1.0f)
		{
			panelPop += dt * 0.5f;
			playerPanel->Position.y = glm::mix((float)height, (float)height - 170, glm::bounceEaseOut(glm::clamp(panelPop, 0.0f, 1.0f)));
		}

		if (!optionsMenu->Visible)
		{
			if (Inputs.KeyDown(Binds::Accept))
			{
				state = State::FadeOut;
				musicManager->FadeOut();
				iris->Out();
				return false;
			}
			else if (Inputs.KeyDown(Binds::Back))
			{
				optionsMenu->Visible = true;
				optionsMenu->Enabled = true;
				logoAnim->Visible = false;
				playerPanel->Visible = false;
				return false;
			}
		}
		else
		{
			if (optionsMenu->Dead)
			{
				optionsMenu->Dead = false;
				optionsMenu->Visible = false;
				optionsMenu->Enabled = false;
				logoAnim->Visible = true;
				logoAnim->Play("idle");
				playerPanel->Visible = true;
				return false;
			}
		}
	}
	else if (state == State::FadeOut)
	{
		if (iris->Done())
		{
			Dead = true;
			RemoveAll();
			::newTickables.push_back(std::make_shared<InGame>());
		}
	}

	return true;
}

TitleScreen::~TitleScreen()
{
	delete pressStart;
}

void TitleScreen::Draw(float dt)
{
	DrawAllTickables(ChildTickables, dt);

	if (!optionsMenu->Visible && pressStart != nullptr)
	{
		//Sprite::DrawText(1, psText, (glm::vec2(width, height) - psSize) * glm::vec2(0.5f, 0.86f), glm::vec4(1, 1, 1, glm::abs(glm::sin((float)glfwGetTime())) * 1.0f), 150.0f * scale);
		Sprite::DrawSprite(pressStart->Texture(),
			(glm::vec2(width, height) - (pressStart->Size() * scale)) * glm::vec2(0.5f, 0.86f),
			pressStart->Size() * scale, glm::vec4(0), 0.0f,
			glm::vec4(1, 1, 1, glm::abs(glm::sin((float)glfwGetTime())) * 1.0f));
	}

#ifdef DEBUG
	Sprite::DrawText(0, "Debug build " __DATE__, glm::vec2(8, height - 24), glm::vec4(1, 1, 1, 0.5));
#endif
}
