#include "Framebuffer.h"
#include <iostream>

Framebuffer::Framebuffer(int width, int height)
{
	Resize(width, height);
}

Framebuffer::~Framebuffer()
{
	// 为什么传入的是地址，详见下面测试代码
	glDeleteFramebuffers(1, &m_FramebufferID);
	glDeleteTextures(1, &m_ColorTextureID);
	glDeleteRenderbuffers(1, &m_DepthStencilID);
}

// -------------- 这是一段测试代码
//int xx = 1;
//int ss[] = { 1, 2, 3 };  // C++ 的数组声明方式
//glTest(1, &xx);  // 正确
//glTest(1, ss);

// 一般来说必须传入Count, 否则只能读result[0]，或者用vector等
//void glTest(int count, const int* result)
//{
//	for (int i = 0; i < count; ++i)
//	{
//		std::cout << result[i] << '\n';
//	}
//}
// -------------- 这是一段测试代码 END

bool Framebuffer::IsValid() const
{
	return m_IsValid;
}

void Framebuffer::Bind() const
{
	// Framebuffer 设置为当前绘制目标
	glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);
	// 设置视口，也就是绘制目标中的一个矩形区域
	glViewport(0, 0, m_Width, m_Height);
}

void Framebuffer::Unbind() 
{
	// 0 表示窗口的默认 Framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint Framebuffer::GetColorTexture() const
{
	return m_ColorTextureID;
}

int Framebuffer::GetWidth() const
{
	return m_Width;
}

int Framebuffer::GetHeight() const
{
	return m_Height;
}

bool Framebuffer::Resize(int width, int height)
{
	if (width <= 0 || height <= 0)
		return false;

	if (m_IsValid && width == m_Width && height == m_Height)
		return true;

	// 先创建一套新对象，成功后再替换旧对象
	GLuint newFramebuffer = 0;
	GLuint newColorTexture = 0;
	GLuint newDepthStencil = 0;

	glGenFramebuffers(1, &newFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, newFramebuffer);

	//创建颜色纹理
	glGenTextures(1, &newColorTexture);
	glBindTexture(GL_TEXTURE_2D, newColorTexture);

	//创建纹理存储，绑定到framebuffer后，最后结果写回
	glTexImage2D(
		GL_TEXTURE_2D,						// 操作当前纹理单元上绑定的二维纹理
		0,												// 定义的是mipmap第0层，即原始图像
		GL_RGBA8,								// 纹理内部保存RGBA四个通道，每个通道8位
		width,										// 宽度和高度(纹素)
		height,
		0,												// 历史遗留数据，必须选0
		GL_RGBA,									// 输入数据中数据的通道顺序
		GL_UNSIGNED_BYTE,				// 输入数据中每个通道的数据类型
		nullptr										// 只分配颜色纹理的存储空间，没有上传图片
	);

	//GL_LINEAR	混合附近纹素，显示更平滑
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//GL_CLAMP_TO_EDGE 表示超出范围时，延伸最靠近的边缘纹素颜色。
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// 将颜色纹理连接到当前的framebuffer
	// 把 newColorTexture 的第 0 层，连接到当前 Framebuffer 的第 0 个颜色附件位置
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		newColorTexture,
		0
	);

	// 创建深度模版缓存
	glGenRenderbuffers(1, &newDepthStencil);
	glBindRenderbuffer(GL_RENDERBUFFER, newDepthStencil);

	glRenderbufferStorage(
		GL_RENDERBUFFER,
		GL_DEPTH24_STENCIL8,
		width,
		height
	);

	// 将深度模板缓存附件链接到当前framebuffer
	glFramebufferRenderbuffer(
		GL_FRAMEBUFFER,
		GL_DEPTH_STENCIL_ATTACHMENT,
		GL_RENDERBUFFER,
		newDepthStencil
	);

	//它检查的是 Framebuffer 当前的配置是否完整、能否作为绘制目标使用，不检查绘制结果。
	const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Framebuffer creation failed. Status: "
			<< status << '\n';

		glDeleteFramebuffers(1, &newFramebuffer);
		glDeleteTextures(1, &newColorTexture);
		glDeleteRenderbuffers(1, &newDepthStencil);

		return false;
	}

	// 新对象创建成功，释放旧对象
	glDeleteFramebuffers(1, &m_FramebufferID);
	glDeleteTextures(1, &m_ColorTextureID);
	glDeleteRenderbuffers(1, &m_DepthStencilID);

	m_FramebufferID = newFramebuffer;
	m_ColorTextureID = newColorTexture;
	m_DepthStencilID = newDepthStencil;

	m_Width = width;
	m_Height = height;
	m_IsValid = true;

	return true;
}