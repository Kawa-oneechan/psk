#include <ctime>
#include <numeric>
#include <glad/glad.h>
#include <stb_image_write.h>
#include "Console.h"

extern int width, height;

void Screenshot()
{
	auto pixels = new unsigned char[3 * width * height];
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	stbi_flip_vertically_on_write(1);
	char filename[128];
	auto now = time(NULL);
	tm gm {};
	localtime_s(&gm, &now);
	std::strftime(filename, 128, "%Y%m%d_%H%M%S.png", &gm);
	stbi_write_png(filename, width, height, 3, pixels, width * 3);
	delete[] pixels;
	conprint(0, "Screenshot taken: {}", filename);
}
