#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

class Shader
{
public:
	// 创建着色器程序
	Shader(const char* vertexSource, const char* fragmentSource);
	// 对象销毁时释放着色器程序
	~Shader();

	// 使用 const 修饰符表示这个成员函数不会修改对象中的成员数据，但它仍然可以调用 OpenGL 来切换当前程序。
	// 使用着色器程序
	void Bind() const;
	// 检查创建是否成功
	bool IsValid() const;
	// 设置int变量的值
	void SetInt(const char* name, int value) const;
	// 设置float变量的值
	void SetFloat(const char* name, float value) const;
	// 设置4x4矩阵变量的值
	void SetMat4(const char* name, const glm::mat4& value) const;

	// 禁止复制，避免两个对象重复释放同一个着色器程序
	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

private:
	// 保存OpenGL着色器程序编号
	GLuint m_ProgramID = 0; // 着色器程序的唯一标识符
};