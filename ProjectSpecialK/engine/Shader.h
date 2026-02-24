#pragma once

#include <glm/glm.hpp>
#include "VFS.h"

class Shader
{
public:
	unsigned int ID;

	//Creates a new Shader from a pair of vertex and fragment files.
	Shader(const std::string& vertexPath, const std::string& fragmentPath);
	//Creates a new Shader from a fragment file, assuming the vertex shader is "sprite.vs".
	explicit Shader(const std::string& fragmentPath);
	~Shader();
	//Binds the shader.
	void Use();
	//Sets the named boolean uniform.
	void Set(const std::string& name, bool value) const;
	//Sets the named integer uniform.
	void Set(const std::string& name, int value) const;
	//Sets the named float uniform.
	void Set(const std::string& name, float value) const;
	//Sets the named vec2 uniform.
	void Set(const std::string& name, const glm::vec2& value) const;
	//Sets the named vec2 uniform.
	void Set(const std::string& name, float x, float y) const;
	//Sets the named vec3 uniform.
	void Set(const std::string& name, const glm::vec3& value) const;
	//Sets the named vec3 uniform.
	void Set(const std::string& name, float x, float y, float z) const;
	//Sets the named vec4 uniform.
	void Set(const std::string& name, const glm::vec4& value) const;
	//Sets the named vec4 uniform.
	void Set(const std::string& name, float x, float y, float z, float w) const;
	//Sets the named mat2x2 uniform.
	void Set(const std::string& name, const glm::mat2& mat, size_t count = 1) const;
	//Sets the named mat3x3 uniform.
	void Set(const std::string& name, const glm::mat3& mat, size_t count = 1) const;
	//Sets the named mat4x4 uniform.
	void Set(const std::string& name, const glm::mat4& mat, size_t count = 1) const;
	//Recompiles the shader from the files originally specified.
	void Reload();
	//Preloads all the shaders in "shaders/shaders.json".
	static void LoadAll();
	//Reloads all the shaders loaded from "shaders/shaders.json".
	static void ReloadAll();

private:
	std::string vertexShaderPath, fragmentShaderPath;
	void load();
};

//A quick library of shaders loaded from "shaders/shaders.json".
extern std::map<std::string, Shader*> Shaders;
