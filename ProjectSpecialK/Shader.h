#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "VFS.h"

class Shader
{
public:
	unsigned int ID;

	Shader(const std::string& vertexPath, const std::string& fragmentPath);
	Shader(const std::string& fragmentPath);
	~Shader();
	void Use();
	void Set(const std::string& name, bool value) const;
	void Set(const std::string& name, int value) const;
	void Set(const std::string& name, float value) const;
	void Set(const std::string& name, const glm::vec2& value) const;
	void Set(const std::string& name, float x, float y) const;
	void Set(const std::string& name, const glm::vec3& value) const;
	void Set(const std::string& name, float x, float y, float z) const;
	void Set(const std::string& name, const glm::vec4& value) const;
	void Set(const std::string& name, float x, float y, float z, float w) const;
	void Set(const std::string& name, const glm::mat2& mat, size_t count = 1) const;
	void Set(const std::string& name, const glm::mat3& mat, size_t count = 1) const;
	void Set(const std::string& name, const glm::mat4& mat, size_t count = 1) const;

	void Reload();

	static void LoadAll();
	static void ReloadAll();

private:
	std::string vertexShaderPath, fragmentShaderPath;
	void checkCompileErrors(unsigned int shader, const std::string& type);
	void load();
};

extern std::map<std::string, Shader*> Shaders;
