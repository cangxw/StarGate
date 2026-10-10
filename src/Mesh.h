#pragma once

#include <glad/gl.h>

class Mesh
{
public:
	Mesh(
		const float* vertices, 
		GLsizeiptr vertexDataSize, 
		const unsigned int* indices, 
		GLsizei indexCount
	);

	~Mesh();
	
	void Draw() const;

	//禁止复制
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;
private:
	GLuint m_VAO = 0;
	GLuint m_VBO = 0;
	GLuint m_EBO = 0;

	GLsizei m_IndexCount = 0;  // 表示数量，例如索引个数
};