#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <thread>
#include <future>
#include "Console.h"
#include "Cursor.h"
#include "SpriteRenderer.h"
#include "VFS.h"
#include "PanelLayout.h"

extern GLFWwindow* window;
extern int width, height;
extern Texture* whiteRect;

static PanelLayout* cinematic{ nullptr };

void ThreadedLoader(std::function<void(float*)> loader, const std::string& cinematicFile)
{
#ifdef DEBUG
	conprint(0, "Starting threaded loader task.");
	auto startingTime = std::chrono::high_resolution_clock::now();
#endif

	auto oldTime = glfwGetTime();

	glDisable(GL_DEPTH_TEST);

	if (!cinematicFile.empty())
		cinematic = new PanelLayout(VFS::ReadJSON(cinematicFile));

	cursor->Select(cursor->WaitIndex);
	auto loadIcon = Texture("loading.png");
	auto loadPos = glm::vec2(width - 256, height - 256);
	//int oldTime = 0;

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
	const int barTop = cinematic ?
		(int)(height - glm::floor(height / 10) - (barHeight / 1)) :
		(int)glm::floor(height / 2) - (barHeight / 1);

	while (true)
	{
		auto newTime = glfwGetTime();
		float dt = (float)(newTime - oldTime);
		oldTime = newTime;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		if (cinematic)
		{
			cinematic->Tick(dt);
			cinematic->Draw(dt);
		}
		else
			Sprite::DrawSprite(loadIcon, loadPos, glm::vec2(128), glm::vec4(0), sinf((float)newTime) * glm::radians(1000.0f));

		Sprite::DrawSprite(*whiteRect, glm::vec2(barLeft - 1, barTop - 1), glm::vec2(barWidth + 2, barHeight + 2), glm::vec4(0), 0.0f, glm::vec4(1, 1, 1, 1));
		Sprite::DrawSprite(*whiteRect, glm::vec2(barLeft, barTop), glm::vec2(barWidth, barHeight), glm::vec4(0), 0.0f, glm::vec4(0.25, 0.25, 0.25, 1));
		Sprite::DrawSprite(*whiteRect, glm::vec2(barLeft, barTop), glm::vec2(barWidth * loadProgress, barHeight), glm::vec4(0), 0.0f, glm::vec4(1, 1, 1, 1));

		cursor->Draw();
		Sprite::FlushBatch();

		glfwSwapBuffers(window);
		glfwPollEvents();

		auto status = future.wait_for(std::chrono::milliseconds(1));
		if (status == std::future_status::ready)
			break;

	}
	t.join();
	cursor->Select(0);

#ifdef DEBUG
	auto endingTime = std::chrono::high_resolution_clock::now();
	auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(endingTime - startingTime);
	conprint(0, "Threaded loader: task took {} milliseconds.", ms_int.count());
#endif
}
