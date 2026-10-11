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
#include <glm/glm.hpp> // 用于矩阵和向量操作
#include <glm/gtc/matrix_transform.hpp> // 用于矩阵变换

//#include <GL/gl.h> // OpenGL 的基础函数声明、类型和常量
#include "Shader.h"
#include "Camera.h"
#include "Mesh.h"
#include "Texture2D.h"

struct InputState
{
	Camera* camera = nullptr;

	float scaleFactor = 1.0f;

	double lastMouseX = 0.0;
	double lastMouseY = 0.0;
	bool firstMouse = true;
};

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
	glEnable(GL_DEPTH_TEST);

	double lastTime = glfwGetTime(); // 进入主循环前的时间记录
	double accumulatedTime = 0.0; // 累计时间，用于计算FPS
	int frameCount = 0; // 帧数计数器

	const double targetFrameTime = 1.0 / 60.0; // 目标帧时间，60 FPS

	// 绘制立方体
	const float vertices[] = {
		// X     Y     Z      U     V      Nx    Ny    Nz

		// 后面：0～3，朝 -Z
		 0.5f, -0.5f, -0.5f, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f, 1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f, 1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f, 0.0f, 1.0f,  0.0f,  0.0f, -1.0f,

		 // 前面：4～7，朝 +Z
		 -0.5f, -0.5f,  0.5f, 0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
		  0.5f, -0.5f,  0.5f, 1.0f, 0.0f,  0.0f,  0.0f,  1.0f,
		  0.5f,  0.5f,  0.5f, 1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
		 -0.5f,  0.5f,  0.5f, 0.0f, 1.0f,  0.0f,  0.0f,  1.0f,

		 // 左面：8～11，朝 -X
		 -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
		 -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
		 -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
		 -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,

		 // 右面：12～15，朝 +X
		  0.5f, -0.5f,  0.5f, 0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
		  0.5f, -0.5f, -0.5f, 1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
		  0.5f,  0.5f, -0.5f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
		  0.5f,  0.5f,  0.5f, 0.0f, 1.0f,  1.0f,  0.0f,  0.0f,

		  // 底面：16～19，朝 -Y
		  -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,  0.0f, -1.0f,  0.0f,
		   0.5f, -0.5f, -0.5f, 1.0f, 0.0f,  0.0f, -1.0f,  0.0f,
		   0.5f, -0.5f,  0.5f, 1.0f, 1.0f,  0.0f, -1.0f,  0.0f,
		  -0.5f, -0.5f,  0.5f, 0.0f, 1.0f,  0.0f, -1.0f,  0.0f,

		  // 顶面：20～23，朝 +Y
		  -0.5f,  0.5f,  0.5f, 0.0f, 0.0f,  0.0f,  1.0f,  0.0f,
		   0.5f,  0.5f,  0.5f, 1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
		   0.5f,  0.5f, -0.5f, 1.0f, 1.0f,  0.0f,  1.0f,  0.0f,
		  -0.5f,  0.5f, -0.5f, 0.0f, 1.0f,  0.0f,  1.0f,  0.0f
	};

	const unsigned int indices[] = {
		 0,  1,  2,   2,  3,  0,  
		 4,  5,  6,   6,  7,  4,
		 8,  9, 10,  10, 11,  8,
		12, 13, 14,  14, 15, 12,
		16, 17, 18,  18, 19, 16,
		20, 21, 22,  22, 23, 20
	};

	//GLint maxComponents = 0;
	//glGetIntegerv(GL_MAX_VERTEX_OUTPUT_COMPONENTS, &maxComponents);

	//std::cout << "Max vertex output components: "  --> 128个
	//	<< maxComponents << std::endl;

	{
		Shader shader(
			STARGATE_ASSET_DIR "/shaders/lit.vert",
			STARGATE_ASSET_DIR "/shaders/lit.frag"
		);
		if (!shader.IsValid())
		{
			std::cerr << "Failed to create shader program" << std::endl;

			glfwDestroyWindow(window);
			glfwTerminate();
			return -1;
		}

		// 创建立方体
		// 数组总字节数 / 一个元素的字节数 = 索引个数
		const GLsizei indexCount = static_cast<GLsizei>(sizeof(indices) / sizeof(indices[0]));

		Mesh cubeMesh(vertices, sizeof(vertices), indices, indexCount);

		// ------------------------创建纹理-----------------------------------------
		Texture2D texture(STARGATE_ASSET_DIR "/textures/crate.png");
		shader.Bind();
		shader.SetInt("uTexture", 0);

		// ------------------------光照信息-----------------------------------------
		glm::vec3 toLightDirection(0.0f, 0.0f, 1.0f);
		glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
		float ambientStrength = 0.2f;
		// ------------------------光照信息 END-----------------------------------------

		const bool timerResolutionEnabled = timeBeginPeriod(1) == TIMERR_NOERROR; // 设置系统定时器分辨率为1ms
		lastTime = glfwGetTime(); // 重新记录进入主循环前的时间

		float offsetX = 0.0f; // 偏移量初始值
		const float moveSpeed = 0.8f; // 偏移量变化速度

		float rotationAngle = 0.0f; // 旋转角度初始值
		const float rotationSpeed = 90.0f; // 旋转速度，单位为度每秒

		Camera camera;
		InputState inputState;
		inputState.camera = &camera;

		float& scaleFactor = inputState.scaleFactor;
		glfwSetWindowUserPointer(window, &inputState); // 将缩放因子指针存储在窗口的用户指针中

		glfwSetScrollCallback(
			window,
			[](GLFWwindow* window, double, double yoffset)
			{
				InputState* input = static_cast<InputState*>(
					glfwGetWindowUserPointer(window)
					);

				input->scaleFactor += static_cast<float>(yoffset) * 0.1f;

				if (input->scaleFactor < 0.1f)
				{
					input->scaleFactor = 0.1f;
				}
			}
		);

		// 隐藏光标
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPosCallback(window,
			[](GLFWwindow* window, double xpos, double ypos)
			{
				// 取回缩放变量的地址
				InputState* input = static_cast<InputState*>(
					glfwGetWindowUserPointer(window)
					);

				if (input->firstMouse)
				{
					input->lastMouseX = xpos;
					input->lastMouseY = ypos;
					input->firstMouse = false;
					return;
				}

				const float offsetX = static_cast<float>(xpos - input->lastMouseX);
				const float offsetY = static_cast<float>(input->lastMouseY - ypos);

				input->lastMouseX = xpos;
				input->lastMouseY = ypos;

				input->camera->Rotate(offsetX, offsetY);
			}
		);

		const glm::vec3 cubePositions[] = {
			glm::vec3(-1.5f, 0.0f, -1.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(1.5f, 0.0f, -1.0f)
		};

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

			// static_cast<float> 将 double 类型的 deltaTime 转换为 float 类型，以便与 moveSpeed 相乘
			const float moveDistance = moveSpeed * static_cast<float>(deltaTime);

			// 输入处理
			if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			{
				glfwSetWindowShouldClose(window, GLFW_TRUE);
			}

			if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			{
				offsetX -= moveDistance;
			}

			if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			{
				offsetX += moveDistance;
			}

			const float rotationStep = rotationSpeed * static_cast<float>(deltaTime); // 计算本帧的旋转角度增量
			if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			{
				rotationAngle += rotationStep; // 逆时针旋转
			}
			if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			{
				rotationAngle -= rotationStep; // 顺时针旋转
			}

			// 切换灯光颜色
			if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
			{
				lightColor = glm::vec3(1.0f, 1.0f, 1.0f); // 白光
			}
			else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
			{
				lightColor = glm::vec3(1.0f, 0.8f, 0.5f); // 暖色
			}
			else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
			{
				lightColor = glm::vec3(0.4f, 0.7f, 1.0f); // 冷色
			}


			float forward = 0.0f;
			float right = 0.0f;

			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			{
				forward += 1.0f;
			}

			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			{
				forward -= 1.0f;
			}

			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			{
				right -= 1.0f;
			}

			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			{
				right += 1.0f;
			}

			camera.Move(forward, right, static_cast<float>(deltaTime));

			// 渲染开始
			// 获取实际绘图区域的宽度和高度
			int framebufferWidth = 0;
			int framebufferHeight = 0;
			glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

			// 最小化等情况下，绘图区域可能为0，暂时跳过绘制
			if (framebufferWidth == 0 || framebufferHeight == 0)
			{
				// 休眠一小段时间，避免CPU空转
				std::this_thread::sleep_for(std::chrono::milliseconds(16));
				continue;
			}

			const float aspectRatio = static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight);
			// 使用正交投影矩阵，确保三角形在不同窗口尺寸下保持正确的比例
			//const glm::mat4 projection = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f);
			
			// OpenGL 右手系，默认朝向-z方向
			// Unity 左手系
			const glm::mat4 projection = glm::perspective(
				glm::radians(45.0f), // 视野角度，单位为弧度
				aspectRatio, 
				0.1f, 
				100.0f // 近平面和远平面距离
			); // 透视投影矩阵

			//const glm::mat4 view = glm::translate(
			//	glm::mat4(1.0f),  // 初始化为单位矩阵
			//	glm::vec3(0.0f, 0.0f, -3.0f) // 将相机向后移动3个单位，相当于003
			//); // 视图矩阵，向后移动相机

			const glm::mat4 view = camera.GetViewMatrix();

			// 设置绘图区域
			glViewport(0, 0, framebufferWidth, framebufferHeight);

			// 用前面的颜色清除屏幕 包括深度缓存
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			shader.Bind();
			shader.SetFloat("uAmbientStrength", ambientStrength);
			shader.SetFloat("uSpecularStrength", 1.0f);
			shader.SetFloat("uShininess", 128.0f);

			shader.SetVec3("uToLightDirection", toLightDirection);
			shader.SetVec3("uLightColor", lightColor);
			shader.SetVec3("uCameraPosition", camera.GetPosition());

			shader.SetMat4("uView", view);
			shader.SetMat4("uProjection", projection); // 将投影矩阵传递给着色器)
			
			texture.Bind();
			for (const glm::vec3& position : cubePositions)
			{
				glm::mat4 transform(1.0f);

				transform = glm::translate(transform, position + glm::vec3(offsetX, 0.0f, 0.0f));
				transform = glm::rotate(transform, glm::radians(rotationAngle),
					glm::vec3(0.0f, 1.0f, 0.0f));
				//glm::radians(rotationAngle) 将角度转换为弧度，因为glm::rotate函数需要弧度值
				// glm::vec3(0, 0, 1) 表示绕Z轴旋转，这里假设三角形在XY平面上
				transform = glm::rotate(transform, glm::radians(20.0f), glm::vec3(1, 0, 0));
				transform = glm::scale(transform, glm::vec3(scaleFactor));

				shader.SetMat4("uTransform", transform); // 将偏移量传递给着色器
				cubeMesh.Draw();
			}

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

		glfwSetCursorPosCallback(window, nullptr);
		glfwSetScrollCallback(window, nullptr); // 取消滚轮回调，避免悬空指针
		glfwSetWindowUserPointer(window, nullptr); // 清除窗口的用户指针，避免悬空指针
	}
	// Shader已经释放，现在销毁OpenGL Context
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
