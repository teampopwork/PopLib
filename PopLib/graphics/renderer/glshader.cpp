#include "glshader.hpp"
#include <fstream>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>

using namespace PopLib;

std::string ReadShaderFile(const std::filesystem::path &path)
{
	if (!std::filesystem::exists(path))
		throw std::runtime_error("Missing shader: " + path.string());

	std::ifstream file(path);
	return std::string(std::istreambuf_iterator<char>(file), {});
}

GLShader::~GLShader()
{
	if (program_ID)
		glDeleteProgram(program_ID);
}

bool GLShader::LoadFromSource(const std::string &vertexSrc, const std::string &fragmentSrc)
{
	GLuint vertex_shader = CompileShader(GL_VERTEX_SHADER, vertexSrc);
	GLuint fragment_shader = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);

	if (!vertex_shader || !fragment_shader)
		return false;

	program_ID = glCreateProgram();
	glAttachShader(program_ID, vertex_shader);
	glAttachShader(program_ID, fragment_shader);
	glLinkProgram(program_ID);

	GLint success;
	glGetProgramiv(program_ID, GL_LINK_STATUS, &success);
	// TODO: ADD SUCCESS CHECK

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	return true;
}

bool GLShader::LoadFromFiles(const std::string &vertexPath, const std::string &fragmentPath)
{
	return LoadFromSource(ReadShaderFile(vertexPath), ReadShaderFile(fragmentPath));
}

void GLShader::Use() const
{
	glUseProgram(program_ID);
}

GLuint GLShader::CompileShader(GLenum type, const std::string &source)
{
	GLuint shader = glCreateShader(type);
	const char *src = source.c_str();
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	return shader;
}

GLuint GLShader::GetUniformLocation(const std::string &name) const
{
	GLuint pos = glGetUniformLocation(program_ID, name.c_str());
	return pos;
}

void GLShader::SetUniform(const std::string &name, int value) const
{
	glUniform1i(GetUniformLocation(name), value);
}
void GLShader::SetUniform(const std::string &name, float value) const
{
	glUniform1f(GetUniformLocation(name), value);
}
void GLShader::SetUniform(const std::string &name, const glm::vec2 &value) const
{
	glUniform2f(GetUniformLocation(name), value.x, value.y);
}
void GLShader::SetUniform(const std::string &name, const glm::vec4 &value) const
{
	glUniform4f(GetUniformLocation(name), value.r, value.g, value.b, value.a);
}
void GLShader::SetUniform(const std::string &name, const glm::mat4 &value) const
{
	glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}