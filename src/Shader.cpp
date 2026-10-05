#include "Shader.h"
#include <iostream>

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
Shader::Shader(const char* vertexSource, const char* fragmentSource)
{
	const GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
	if (vertexShader == 0)
	{
		return;
	}

	const GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
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