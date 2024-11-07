#pragma once

#include "support/glad/glad.h"

class Texture;
class Shader;

class Framebuffer
{
private:
	unsigned int fbo;
	unsigned int rbo;
	Texture* texture;
	Shader* shader;
	int width, height;
	bool isSetup{ false };
	bool shaderOwned;

	void setup();
	
public:
	Framebuffer(const std::string& fragmentShader, int width, int height);
	Framebuffer(Shader* fragmentShader, int width, int height);
	~Framebuffer();
	void Use();
	void Drop();
	void Draw(glm::vec2 pos = glm::vec2(0));
	void Draw(glm::vec2 pos, glm::vec2 size);
	void ChangeShader(const std::string& newFragmentShader);
	void ChangeShader(Shader* newShader);
	Texture& GetTexture();
};
