#include <glad/glad.h>
#include <map>
#include <format.h>
#include "Shader.h"
#include "TextUtils.h"
#include "VFS.h"
#include "Console.h"

#define HEADER "#version 430 core\n#define PSK\n"

__declspec(noreturn)
	extern void FatalError(const std::string& message);

static unsigned int currentShader;
std::map<std::string, Shader*> Shaders;

void Shader::load()
{
	auto vShaderCode = HEADER + VFS::ReadString(vertexShaderPath);
	auto fShaderCode = HEADER + VFS::ReadString(fragmentShaderPath);

	HandleIncludes(vShaderCode, VFS::GetPathPart(vertexShaderPath));
	HandleIncludes(fShaderCode, VFS::GetPathPart(fragmentShaderPath));

	unsigned int vertex, fragment;
	const char* vs = vShaderCode.c_str();
	const char* fs = fShaderCode.c_str();

	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vs, NULL);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX");

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fs, NULL);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT");

	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	//Hook up Common Uniforms if any.
	auto commonBlockIndex = glGetUniformBlockIndex(ID, "CommonData");
	glUniformBlockBinding(ID, commonBlockIndex, 1);
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) : vertexShaderPath(vertexPath), fragmentShaderPath(fragmentPath)
{
	load();
}

Shader::Shader(const std::string& fragmentPath) : Shader("shaders/sprite.vs", fragmentPath)
{}

Shader::~Shader()
{
	glDeleteProgram(ID);
}

void Shader::Use()
{
	if (currentShader != ID)
	{
		glUseProgram(ID);
		currentShader = ID;
	}
}

void Shader::Set(const std::string& name, bool value) const
{
	glProgramUniform1i(ID, glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::Set(const std::string& name, int value) const
{
	glProgramUniform1i(ID, glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::Set(const std::string& name, float value) const
{
	glProgramUniform1f(ID, glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::Set(const std::string& name, const glm::vec2& value) const
{
	glProgramUniform2fv(ID, glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::Set(const std::string& name, float x, float y) const
{
	glProgramUniform2f(ID, glGetUniformLocation(ID, name.c_str()), x, y);
}

void Shader::Set(const std::string& name, const glm::vec3& value) const
{
	glProgramUniform3fv(ID, glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::Set(const std::string& name, float x, float y, float z) const
{
	glProgramUniform3f(ID, glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Shader::Set(const std::string& name, const glm::vec4& value) const
{
	glProgramUniform4fv(ID, glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::Set(const std::string& name, float x, float y, float z, float w) const
{
	glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

void Shader::Set(const std::string& name, const glm::mat2& mat, size_t count) const
{
	glProgramUniformMatrix2fv(ID, glGetUniformLocation(ID, name.c_str()), (GLsizei)count, GL_FALSE, &mat[0][0]);
}

void Shader::Set(const std::string& name, const glm::mat3& mat, size_t count) const
{
	glProgramUniformMatrix3fv(ID, glGetUniformLocation(ID, name.c_str()), (GLsizei)count, GL_FALSE, &mat[0][0]);
}

void Shader::Set(const std::string& name, const glm::mat4& mat, size_t count) const
{
	glProgramUniformMatrix4fv(ID, glGetUniformLocation(ID, name.c_str()), (GLsizei)count, GL_FALSE, &mat[0][0]);
}

void Shader::Reload()
{
	glDeleteProgram(ID);
	load();
}

void Shader::LoadAll()
{
	auto doc = VFS::ReadJSON("shaders/shaders.json");
	if (!doc)
		FatalError("Could not read shaders/shaders.json. Something is very wrong.");
	for (auto& vs : doc.as_object())
	{
		auto vsFile = fmt::format("shaders/{}", vs.first);
		for (auto& fs : vs.second.as_object())
		{
			const auto& key = fs.first;
			auto fsFile = fmt::format("shaders/{}", fs.second.as_string());
			Shaders[key] = new Shader(vsFile, fsFile);
		}
	}
}

void Shader::ReloadAll()
{
	for (auto& s : Shaders)
	{
		s.second->Reload();
	}
}

void Shader::checkCompileErrors(unsigned int shader, const std::string& type)
{
	int success;
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			conprint(1, "Shader compilation error, type {}:\n{}\n", type, infoLog);
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			conprint(1, "Shader linking error, type {}:\n{}\n", type, infoLog);
		}
	}
}
