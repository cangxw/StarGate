// StarGate.cpp: 定义应用程序的入口点。
//

#define GLFW_INCLUDE_NONE
#define NOMINMAX // 防止Windows.h中定义的min和max宏与std::min和std::max冲突
#include <GLFW/glfw3.h>
#include <iostream>
#include<iomanip> // 用于格式化输出，控制显示小数的位数
#include <sstream> // 拼接窗口标题字符串
#include <chrono> // 用于表示休眠时长
#include <thread> // 用于线程休眠，控制帧率
#include <windows.h> // Windows 定时器接口
#include <mmsystem.h> // Windows 定时器接口
#include <GL/gl.h> // OpenGL 的基础函数声明、类型和常量

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

	// 暂时沿用现有的手动限帧
	glfwSwapInterval(0); // 禁用垂直同步，手动控制帧率

	glClearColor(0.1f, 0.2f, 0.4f, 1.0f); // 设置清屏颜色

	// 进入主循环前的时间记录
	double lastTime = glfwGetTime();

	double accumulatedTime = 0.0; // 累计时间，用于计算FPS
	int frameCount = 0; // 帧数计数器

	const double targetFrameTime = 1.0 / 60.0; // 目标帧时间，60 FPS

	const bool timerResolutionEnabled = timeBeginPeriod(1) == TIMERR_NOERROR; // 设置系统定时器分辨率为1ms

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

	// 清理和退出
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
