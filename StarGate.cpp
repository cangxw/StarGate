// StarGate.cpp: 定义应用程序的入口点。
//

#include "StarGate.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <iostream>

using namespace std;

int main()
{
	//  GLFW的错误输出到控制台
	glfwSetErrorCallback([](int error, const char* description) {
		cerr << "GLFW Error (" << error << "): " << description << endl;
		});

	// 初始化GLFW
	if (!glfwInit())
	{
		cerr << "Failed to initialize GLFW" << endl;
		return -1;
	}

	// 创建一个窗口
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	GLFWwindow* window = glfwCreateWindow(
		1280, 720, "StarGate", nullptr, nullptr);

	if (!window)
	{
		cerr << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}

	// 主循环
	while (!glfwWindowShouldClose(window))
	{
		glfwWaitEventsTimeout(0.016); // 等待事件，避免CPU占用过高

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
	}

	// 清理和退出
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
