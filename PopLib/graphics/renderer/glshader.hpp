#ifndef __GLSHADER_HPP__
#define __GLSHADER_HPP__

#pragma once

#include <string>
#include <glad/gl.h>
#include <glm/glm.hpp>

namespace PopLib
{
class GLShader
{
  private:
	GLuint program_ID = 0;

	GLuint CompileShader(GLenum type, const std::string &source);
	GLuint GetUniformLocation(const std::string &name) const;

  public:
	GLShader() = default;
	~GLShader();

	bool LoadFromSource(const std::string &vertexSrc, const std::string &fragmentSrc);
	bool LoadFromFiles(const std::string &vertexPath, const std::string &fragmentPath);

	void Use() const;
	GLuint GetID() const
	{
		return program_ID;
	}

	void SetUniform(const std::string &name, int value) const;
	void SetUniform(const std::string &name, float value) const;
	void SetUniform(const std::string &name, const glm::vec2 &value) const;
	void SetUniform(const std::string &name, const glm::vec4 &value) const;
	void SetUniform(const std::string &name, const glm::mat4 &value) const;
};

} // namespace PopLib

#endif // __GLSHADER_HPP__