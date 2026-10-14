#include "Texture2D.h"

#include <stb_image.h>
#include <iostream>

Texture2D::Texture2D(const char* path)
{
    // 图片加载失败时，使用备用棋盘格
    const unsigned char fallbackPixels[] = {
        0, 0, 0, 255,       255, 255, 255, 255,
        255, 255, 255, 255, 0, 0, 0, 255
    };

    int width = 0;
    int height = 0;
    int originalChannels = 0;

    // 图片通常是从顶部开始存储，翻转后对应当前的UV方向
    stbi_set_flip_vertically_on_load(true);

    // 解码图片，输出RGBA四个通道
    unsigned char* imageData = stbi_load(
        path,
        &width,
        &height,
        &originalChannels,
        STBI_rgb_alpha
    );

    const unsigned char* pixels = imageData;
    if (imageData == nullptr)
    {
        std::cerr << "Failed to load texture: " << path
            << "\nReason : " << stbi_failure_reason()
            << std::endl;

        width = 2;
        height = 2;
        pixels = fallbackPixels;
    }

    glGenTextures(1, &m_TextureID);	//向 OpenGL 申请纹理编号，并把编号写进你提供的变量

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TextureID); //首次绑定时建立二维纹理对象，并选中它供后续操作

    // 当UV超出0-1的时候，重复纹理
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // 使用最近邻接采样，让棋盘格边界清晰
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // 缩小时使用mipmap  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //放大时最近邻过滤

    // 创建纹理存储，并上传像素数据
    glTexImage2D(
        GL_TEXTURE_2D,				// 操作当前纹理单元上绑定的二维纹理
        0,										// 定义的是mipmap第0层，即原始图像
        GL_RGBA8,						// 纹理内部保存RGBA四个通道，每个通道8位
        width, height,                   // 宽度和高度
        0,										// 历史遗留数据，必须选0
        GL_RGBA,							// 输入数据中数据的通道顺序
        GL_UNSIGNED_BYTE,		//输入数据中每个通道的数据类型
        pixels
    );

    glGenerateMipmap(GL_TEXTURE_2D);

    // 图片已经上传，释放stb_image分配的内存
    if (imageData != nullptr)
    {
        stbi_image_free(imageData);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture2D::~Texture2D()
{
    if (m_TextureID != 0)
    {
        glDeleteTextures(1, &m_TextureID);
    }
}

void Texture2D::Bind(unsigned int slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_TextureID);
}