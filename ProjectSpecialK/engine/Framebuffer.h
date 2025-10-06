#pragma once
#include <string>
#include <glm/glm.hpp>

class Texture;
class Shader;

class Framebuffer
{
private:
	unsigned int fbo{ 0 };
	unsigned int rbo{ 0 };
	Texture* texture{ nullptr };
	Texture* lut{ nullptr };
	Shader* shader{ nullptr };
	int width, height;
	bool isSetup{ false };
	bool shaderOwned;

	void setup();

public:
	Framebuffer(const std::string& fragmentShader, int width, int height);
	Framebuffer(Shader* fragmentShader, int width, int height);
	~Framebuffer();
	//Binds the FrameBuffer for use.
	void Use();
	//Releases the FrameBuffer, switching back to none.
	void Drop();
	//Draws the FrameBuffer's contents at full size using the currently attached shader.
	void Draw(const glm::vec2& pos = glm::vec2(0));
	//Draws the FrameBuffer's contents using the currently attached shader.
	void Draw(const glm::vec2& pos, const glm::vec2& size);
	//Replaces the FrameBuffer's attached shader.
	void ChangeShader(const std::string& newFragmentShader);
	//Replaces the FrameBuffer's attached shader, optionally taking ownership responsibilities.
	void ChangeShader(Shader* newShader, bool own = false);
	//Returns a pointer to the FrameBuffer's attached shader.
	Shader* GetShader() const { return shader; }
	//Reloads the attached shader.
	void ReloadShader();
	//Returns a reference to the FrameBuffer's underlying texture.
	Texture& GetTexture();
	//Replaces the color lookup table or whatever the attached shader needs in slot 1.
	void SetLut(Texture* newLut);
	//Returns a pointer to the current color lookup table.
	Texture* GetLut();

	Framebuffer(const Framebuffer &x) = delete;
	Framebuffer &operator=(const Framebuffer &x) = delete;
};
