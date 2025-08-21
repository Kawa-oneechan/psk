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

extern "C" { double glfwGetTime(void); }

extern std::vector<TickableP> newTickables;

TitleScreen::TitleScreen()
{
	tickables.clear();
	auto logoJson = VFS::ReadJSON("cinematics/logo/logo.json").as_object();
	logoAnim = std::make_shared<PanelLayout>(logoJson["cinematic"]);

	{
		auto logoJoke = logoAnim->GetPanel("logoJoke");
		auto options = Text::Count("logojoke:");
		if (options == 0)
		{
			conprint(2, "TitleScreen: could not find a logo joke.");
			logoJoke->Text = "???";
		}
		else
		{
			int choice = rnd::GetInt((int)options);
			logoJoke->Text = Text::Get(fmt::format("logojoke:{}", choice));
		}
		Text::Forget("logojoke:");
	}
	
	LoadCamera("cameras/title.json");

	//psText = PreprocessBJTS(Text::Get("title:pressstart"));
	//psSize = Sprite::MeasureText(1, psText, 100);

	optionsMenu = std::make_shared<OptionsMenu>();
	optionsMenu->Enabled = false;
	optionsMenu->Visible = false;

	iris = std::make_shared<Iris>();

	tickables.push_back(logoAnim);
	tickables.push_back(optionsMenu);
	tickables.push_back(iris);
}

bool TitleScreen::Tick(float dt)
{
	RevAllTickables(tickables, dt);

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
			pressStart = new DropLabel(PreprocessBJTS(Text::Get("title:pressstart")), 1, 150, DropLabel::Style::Drop);
		}
	}
	else if (state == State::Wait)
	{
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
				logoAnim->Visible = true;
				logoAnim->Play("idle");
				return false;
			}
		}
	}
	else if (state == State::FadeOut)
	{
		if (iris->Done())
		{
			dead = true;
			tickables.clear();
			delete pressStart;
			::newTickables.push_back(std::make_shared<InGame>());
		}
	}

	return true;
}

void TitleScreen::Draw(float dt)
{
	DrawAllTickables(tickables, dt);

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
