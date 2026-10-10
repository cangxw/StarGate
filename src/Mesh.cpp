#include "Mesh.h"

Mesh::Mesh(
	const float* vertices,
	GLsizeiptr vertexDataSize,
	const unsigned int* indices,
	GLsizei indexCount
) : m_IndexCount(indexCount)
{
	//创建并绑定VAO
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	//创建顶点缓冲对象VBO，并上传顶点数据
	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	// 创建数据存储，并复制verticess中的数据，把数据从CPU内存搬到GPU缓存区
	// GL_ARRAY_BUFFER 操作当前绑定的顶点缓冲对象
	// 这里使用GL_STATIC_DRAW表示数据不会频繁修改，适合静态数据
	glBufferData(
		GL_ARRAY_BUFFER,
		vertexDataSize,
		vertices,
		GL_STATIC_DRAW
	);

	//创建EBO，上传索引数据
	glGenBuffers(1, &m_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		//GLsizeiptr 本身是整数类型。 名字里的 ptr 表示它的大小与平台指针宽度相匹配
		//在 Windows 64 位环境下 最终对应signed long long int
		static_cast<GLsizeiptr>(indexCount) * sizeof(unsigned int),  // 计算索引一共占多少字节
		indices, 
		GL_STATIC_DRAW
	);

	// 1个顶点：3个位置，uv 2个，法线3个。一共8个数据
	const GLsizei stride = 8 * sizeof(float);		//步进

	// 属性0：位置
	glVertexAttribPointer(
		0,					// 属性编号，之后对应着色器的位置输入
		3,					// 每个顶点属性的分量数量，这里是3个（x, y, z）
		GL_FLOAT,	// 数据类型
		GL_FALSE,	// 是否归一化
		stride,			// 步长（每个顶点的字节数）
		nullptr			// 从缓冲区的第0个字节开始读取数据
	);
	// 启用编号为0的顶点属性数组
	glEnableVertexAttribArray(0);

	// 属性1 ：UV
	glVertexAttribPointer(
		1, 2, GL_FLOAT, GL_FALSE, stride,
		//reinterpret_cast 可以理解为：把一个值重新解释成另一种类型.
		// 它常用于指针类型之间，或整数与指针之间的转换。
		reinterpret_cast<const void*>(3 * sizeof(float))
	);
	glEnableVertexAttribArray(1);

	// 属性2：法线
	glVertexAttribPointer(
		2, 3, GL_FLOAT, GL_FALSE, stride, 
		reinterpret_cast<const void*>(5 * sizeof(float))
	);
	glEnableVertexAttribArray(2);

	// 设置完成，取消当前VAO的绑定，避免后续操作意外修改它
	glBindVertexArray(0);
}

Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_EBO);
}

void Mesh::Draw() const
{
	glBindVertexArray(m_VAO);
	// 用索引绘制，这一步开始执行顶点着色器
	glDrawElements(
		GL_TRIANGLES,
		m_IndexCount,
		GL_UNSIGNED_INT,
		nullptr
	);

	glBindVertexArray(0);
}