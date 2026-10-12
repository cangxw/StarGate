#pragma once

#include <glad/gl.h>

class Framebuffer
{
public:
	Framebuffer(int width, int height);
	~Framebuffer();

	bool Resize(int width, int height);
	bool IsValid() const;

	void Bind() const;		//让后面的绘制写入这个FrameBuffer
	static void Unbind();	//恢复到GLFW的窗口

	GLuint GetColorTexture() const;	// 取得场景图像的纹理编号

	int GetWidth() const;
	int GetHeight() const;

	Framebuffer(const Framebuffer&) = delete;
	Framebuffer& operator=(const Framebuffer&) = delete;

private:
	GLuint m_FramebufferID = 0;
	GLuint m_ColorTextureID = 0;
	GLuint m_DepthStencilID = 0;

	int m_Width = 0;
	int m_Height = 0;

	bool m_IsValid = false;
};