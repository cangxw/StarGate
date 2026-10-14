#pragma once

#include <glad/gl.h>

class Texture2D
{
public:
	// expilicit 必须显式调用构造函数
	explicit Texture2D(const char* path);

	~Texture2D();

	//绑定到纹理单元编号，默认为0
	void Bind(unsigned int slot = 0) const;

	// 禁止复制
	Texture2D(const Texture2D&) = delete;
	Texture2D& operator=(const Texture2D&) = delete;

private:
	GLuint m_TextureID = 0;		//纹理对象编号
};