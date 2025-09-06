#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <thread>
#include <future>
#include "Console.h"
#include "Cursor.h"
#include "SpriteRenderer.h"

extern GLFWwindow* window;
extern int width, height;
extern Texture* whiteRect;

void ThreadedLoader(std::function<void(float*)> loader)
{
#ifdef DEBUG
	conprint(0, "Starting threaded loader task.");
	auto startingTime = std::chrono::high_resolution_clock::now();
#endif

	glDisable(GL_DEPTH_TEST);
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
	const int barTop = (int)glm::floor(height / 2) - (barHeight / 1);

	while (true)
	{
		auto time = (float)glfwGetTime();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		Sprite::DrawSprite(loadIcon, loadPos, glm::vec2(128), glm::vec4(0), sinf(time) * glm::radians(1000.0f));

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
