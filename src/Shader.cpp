#include "Shader.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp> // 用于将glm::mat4转换为float数组

#include <fstream>
#include <sstream>
#include <string>

static bool ReadShaderFile(const char* path, std::string& source)
{
	std::ifstream file(path);

	if (!file.is_open())
	{
		std::cerr << "Failed to open shader file: " << path << std::endl;
		return false;
	}

	std::ostringstream buffer;
	buffer << file.rdbuf();

	if (file.bad())
	{
		std::cerr << "Failed to read shader file: " << path << std::endl;
		return false;
	}

	source = buffer.str();
	if (source.empty())
	{
		std::cerr << "Shader file is empty: " << path << std::endl;
		return false;
	}

	return true;
}

static GLuint CompileShader(GLenum type, const char* source)
{	
	// 创建一个新的着色器对象，返回一个唯一的标识符（GLuint类型），用于后续的着色器操作
	const GLuint shader = glCreateShader(type);

	if (shader == 0)
	{
		std::cerr << "Failed to create shader" << std::endl;
		return 0;
	}

	// 将着色器源代码附加到着色器对象上
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	GLint success = GL_FALSE;
	// 检查着色器编译是否成功
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (success != GL_TRUE)
	{
		// 如果编译失败，获取错误日志并输出到控制台
		char log[1024] = {};
		glGetShaderInfoLog(shader, sizeof(log), nullptr, log);

		std::cerr << "Failed to compile shader" << std::endl;
		std::cerr << log << std::endl;

		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

// Shader类的构造函数，接受顶点着色器和片段着色器的源代码
Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	std::string vertexSource;
	std::string fragmentSource;

	if (!ReadShaderFile(vertexPath, vertexSource))
		return;

	if (!ReadShaderFile(fragmentPath, fragmentSource))
		return;


	const GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource.c_str());
	if (vertexShader == 0)
	{
		return;
	}

	const GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource.c_str());
	if (fragmentShader == 0)
	{
		glDeleteShader(vertexShader);
		return;
	}

	// 创建一个新的着色器程序对象
	m_ProgramID = glCreateProgram();
	if (m_ProgramID == 0)
	{
		std::cerr << "Failed to create shader program" << std::endl;
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return;
	}

	// 将编译好的顶点着色器和片段着色器附加到程序对象上
	glAttachShader(m_ProgramID, vertexShader);
	glAttachShader(m_ProgramID, fragmentShader);
	glLinkProgram(m_ProgramID);

	GLint success = GL_FALSE;
	glGetProgramiv(m_ProgramID, GL_LINK_STATUS, &success);

	// 连接结束，移除并释放着色器对象
	glDetachShader(m_ProgramID, vertexShader);
	glDetachShader(m_ProgramID, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	if (success != GL_TRUE)
	{
		char log[1024] = {};
		glGetProgramInfoLog(m_ProgramID, sizeof(log), nullptr, log);

		std::cerr << "Failed to link shader program" << std::endl;
		std::cerr << log << std::endl;

		glDeleteProgram(m_ProgramID);
		m_ProgramID = 0;
	}
}

Shader::~Shader()
{
	if (m_ProgramID != 0)
	{
		glDeleteProgram(m_ProgramID);
	}
}

void Shader::Bind() const
{
	if (m_ProgramID != 0)
	{
		glUseProgram(m_ProgramID);
	}
}

bool Shader::IsValid() const
{
	return m_ProgramID != 0;
}

void Shader::SetInt(const char* name, int value) const
{
	if (m_ProgramID != 0)
	{
		const GLint location = glGetUniformLocation(m_ProgramID, name);
		if (location != -1)
		{
			glUniform1i(location, value);
		}
		else
		{
			std::cerr << "Warning: Uniform '" << name << "' not found in shader program." << std::endl;
		}
	}
}

void Shader::SetFloat(const char* name, float value) const
{
	if (m_ProgramID != 0)
	{
		const GLint location = glGetUniformLocation(m_ProgramID, name);
		if (location != -1)
		{
			glUniform1f(location, value);
		}
		else
		{
			std::cerr << "Warning: Uniform '" << name << "' not found in shader program." << std::endl;
		}
	}
}

void Shader::SetVec3(const char* name, const glm::vec3& value) const
{
	if (m_ProgramID != 0)
	{
		const GLint location = glGetUniformLocation(m_ProgramID, name);
		if (location != -1)
		{
			glUniform3fv(location, 1, glm::value_ptr(value));
		}
		else
		{
			std::cerr << "Warning: Uniform '" << name << "' not found in shader program." << std::endl;
		}
	}
}

void Shader::SetMat4(const char* name, const glm::mat4& value) const
{
	if (m_ProgramID != 0)
	{
		const GLint location = glGetUniformLocation(m_ProgramID, name);
		if (location != -1)
		{
			// location 是uniform变量在着色器程序中的位置，1表示只设置一个矩阵，
			// GL_FALSE表示不需要转置矩阵，glm::value_ptr(value)获取矩阵的指针，以便OpenGL可以读取数据
			glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
		}
		else
		{
			std::cerr << "Warning: Uniform '" << name << "' not found in shader program." << std::endl;
		}
	}
}