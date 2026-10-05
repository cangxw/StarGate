// StarGate.cpp: 定义应用程序的入口点。
//

#define GLFW_INCLUDE_NONE
#define NOMINMAX // 防止Windows.h中定义的min和max宏与std::min和std::max冲突
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include<iomanip> // 用于格式化输出，控制显示小数的位数
#include <sstream> // 拼接窗口标题字符串
#include <chrono> // 用于表示休眠时长
#include <thread> // 用于线程休眠，控制帧率
#include <windows.h> // Windows 定时器接口
#include <mmsystem.h> // Windows 定时器接口

//#include <GL/gl.h> // OpenGL 的基础函数声明、类型和常量
#include "Shader.h"

int main()
{
	//  GLFW的错误输出到控制台
	glfwSetErrorCallback([](int error, const char* description) {
		std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
		});

	// 初始化GLFW
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	// 创建一个窗口
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);

	// 使用 OpenGL 3.3 核心模式
	// 利点は、一つの関数で数多くのパラメータを設定できることです。
	// 欠点は、決められた順序を厳しく守って設定する必要があることです。
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(
		1280, 720, "StarGate", nullptr, nullptr);

	if (!window)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// 将这个窗口的openGL上下文绑定到当前线程
	glfwMakeContextCurrent(window);

	//使用GLFW提供的函数地址查询接口，加载OpenGL
	const int version = gladLoadGL(glfwGetProcAddress);

	if (version == 0 || !GLAD_GL_VERSION_3_3)
	{
		std::cerr << "Failed to initialize OpenGL context" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	std::cout << "Loaded OpenGL "
		<< GLAD_VERSION_MAJOR(version) << "."
		<< GLAD_VERSION_MINOR(version)
		<< std::endl;

	// 暂时沿用现有的手动限帧
	glfwSwapInterval(0); // 不请求垂直同步，后面通过休眠手动限帧

	glClearColor(0.1f, 0.2f, 0.4f, 1.0f); // 设置清屏颜色

	double lastTime = glfwGetTime(); // 进入主循环前的时间记录
	double accumulatedTime = 0.0; // 累计时间，用于计算FPS
	int frameCount = 0; // 帧数计数器

	const double targetFrameTime = 1.0 / 60.0; // 目标帧时间，60 FPS

	// 绘制三角形
	const float  vertices[] = {
		-0.5f, -0.5f, 0.0f, // 左下角
		0.5f, -0.5f, 0.0f,  // 右下角
		0.0f,  0.5f, 0.0f   // 顶部
	};

	// 创建并绑定顶点数组对象（VAO）
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// 创建顶点缓冲对象（VBO
	GLuint vbo = 0; // vob保存的是对象编号
	glGenBuffers(1, &vbo);  // 生成一个缓冲对象编号，并写入vbo
	// 将这个缓冲对象绑定为当前的顶点缓冲区
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// 创建数据存储，并复制verticess中的数据
	// GL_ARRAY_BUFFER 操作当前绑定的顶点缓冲对象
	// 这里使用GL_STATIC_DRAW表示数据不会频繁修改，适合静态数据
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//描述位置属性的数据读取方式
	glVertexAttribPointer(
		0,							// 属性编号，之后对应着色器的位置输入
		3,							// 每个顶点属性的分量数量，这里是3个（x, y, z）
		GL_FLOAT,			// 数据类型
		GL_FALSE,			// 是否归一化
		3 * sizeof(float),	// 步长（每个顶点的字节数）
		nullptr					// 从缓冲区的第0个字节开始读取数据
	);

	// 启用编号为0的顶点属性数组
	glEnableVertexAttribArray(0); 

	// 设置完成，取消当前VAO的绑定，避免后续操作意外修改它
	glBindVertexArray(0);

	// Shader source code
	const char* vertexShaderSource = R"(
	#version 330 core

	layout(location = 0) in vec3 aPosition; // 顶点位置输入
	void main()
	{
		gl_Position = vec4(aPosition, 1.0); // 将顶点位置传递给裁剪空间
	}
	)";

	const char* fragmentShaderSource = R"(
	#version 330 core

	out vec4 fragmentColor; // 输出颜色

	void main()
	{
		fragmentColor = vec4(1.0, 0.6, 0.2, 1.0); // 设置输出颜色为橙色
	}
	)";

	{
		Shader shader(vertexShaderSource, fragmentShaderSource);
		if (!shader.IsValid())
		{
			std::cerr << "Failed to create shader program" << std::endl;

			glDeleteVertexArrays(1, &vao);
			glDeleteBuffers(1, &vbo);
			glfwDestroyWindow(window);
			glfwTerminate();
			return -1;
		}


		const bool timerResolutionEnabled = timeBeginPeriod(1) == TIMERR_NOERROR; // 设置系统定时器分辨率为1ms
		lastTime = glfwGetTime(); // 重新记录进入主循环前的时间

		// 主循环
		while (!glfwWindowShouldClose(window))
		{
			// 记录本次循环的开始时间
			const double frameStartTime = glfwGetTime();
			glfwPollEvents(); // 立即处理所有挂起的事件

			// 计算本次循环与上次循环的时间差
			const double currentTime = glfwGetTime();
			const double deltaTime = currentTime - lastTime;
			lastTime = currentTime;

			// 输入处理
			if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			{
				glfwSetWindowShouldClose(window, GLFW_TRUE);
			}

			// 渲染开始
			// 获取实际绘图区域的宽度和高度
			int framebufferWidth = 0;
			int framebufferHeight = 0;
			glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

			// 设置绘图区域
			glViewport(0, 0, framebufferWidth, framebufferHeight);

			// 用前面的颜色清除屏幕
			glClear(GL_COLOR_BUFFER_BIT);

			// 选择着色器程序和顶点读取配置
			shader.Bind();
			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, 0, 3); // 从第0个顶点开始，使用3个顶点绘制一个三角形
			glBindVertexArray(0); // 绘制完成后，解绑VAO，避免后续操作意外修改它

			// 把这一帧的渲染结果显示到屏幕上
			glfwSwapBuffers(window);
			// 渲染结束

			// 累计统计数据
			accumulatedTime += deltaTime;
			++frameCount;

			// 每0.5秒更新一次窗口标题，显示FPS
			if (accumulatedTime >= 0.5)
			{
				const double fps = frameCount / accumulatedTime;
				const double averageFrameMs = accumulatedTime / frameCount * 1000.0;

				std::ostringstream title;
				title << "StarGate | "
					<< std::fixed << std::setprecision(1)
					<< fps << " FPS | "
					<< std::setprecision(2)
					<< averageFrameMs << " ms";

				glfwSetWindowTitle(window, title.str().c_str());

				// 重置计数器
				accumulatedTime = 0.0;
				frameCount = 0;
			}

			// 计算本帧的工作时间
			const double frameWorkTime = glfwGetTime() - frameStartTime;
			const double remainingTime = targetFrameTime - frameWorkTime;

			//工作完成较快的时候，等待剩余时间，尽量接近目标帧率
			if (remainingTime > 0.0)
			{
				//chrono::duration 将double转换成表示时间的类型
				std::this_thread::sleep_for(std::chrono::duration<double>(remainingTime));
			}
		}

		if (timerResolutionEnabled)
		{
			timeEndPeriod(1); // 恢复系统定时器分辨率
		}


		// 释放VBO
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
	}
	// Shader已经释放，现在销毁OpenGL Context
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
