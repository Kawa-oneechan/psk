#pragma once

#include <glad/glad.h>

class Texture;
class Shader;

class Framebuffer
{
private:
	unsigned int fbo{ 0 };
	unsigned int rbo{ 0 };
	Texture* texture{ nullptr };
	Shader* shader{ nullptr };
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
	void Draw(const glm::vec2& pos = glm::vec2(0));
	void Draw(const glm::vec2& pos, const glm::vec2& size);
	void ChangeShader(const std::string& newFragmentShader);
	void ChangeShader(Shader* newShader, bool own = false);
	void ReloadShader();
	Texture& GetTexture();
};
