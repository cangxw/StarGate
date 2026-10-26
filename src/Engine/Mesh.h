#pragma once

#include <glad/gl.h>
#include <cstddef>
#include <vector>

struct SkinnedVertex;

class Mesh
{
public:
	Mesh(
		const float* vertices, 
		GLsizeiptr vertexDataSize, 
		const unsigned int* indices, 
		GLsizei indexCount
	);

	Mesh(
		const std::vector<SkinnedVertex>& vertices,
		const std::vector<unsigned int>& indices
	);

	~Mesh();
	
	void Draw() const;

	void Draw(std::size_t firstIndex, GLsizei indexCount) const;

	bool IsValid() const;

	//禁止复制
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;
private:
	GLuint m_VAO = 0;
	GLuint m_VBO = 0;
	GLuint m_EBO = 0;

	GLsizei m_IndexCount = 0;  // 表示数量，例如索引个数
};