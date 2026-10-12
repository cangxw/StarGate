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
#include <assimp/version.h>
#include <limits>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/gtc/type_ptr.hpp>	//这个头文件提供 glm::value_ptr()，用来取得 GLM 向量或矩阵内部数据的指针

#include <json11.hpp>
#include <iterator> // std::size
#include <string>
#include <filesystem>
#include <system_error>
#include <memory>	//管理动态创建的对象 比如make_unique
#include <utility>	// 提供移动等通用工具 std::move等

//#include <GL/gl.h> // OpenGL 的基础函数声明、类型和常量
#include "Shader.h"
#include "Camera.h"
#include "SceneObject.h"
#include "Model.h"
#include "Framebuffer.h"
#include "SceneData.h"
#include "SceneSerializer.h"


struct InputState
{
	Camera* camera = nullptr;

	double lastMouseX = 0.0;
	double lastMouseY = 0.0;
	bool firstMouse = true;
	bool cameraControlActive = false;
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

	// 输出assimp版本
	std::cout << "Assimp "
		<< aiGetVersionMajor() << "."
		<< aiGetVersionMinor() << "."
		<< aiGetVersionRevision()
		<< std::endl;

	// 创建一个窗口
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);

	// 使用 OpenGL 3.3 核心模式
	// 利点は、一つの関数で数多くのパラメータを設定できることです。
	// 欠点は、決められた順序を厳しく守って設定する必要があることです。
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

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

	//GLint maxComponents = 0;
	//glGetIntegerv(GL_MAX_VERTEX_OUTPUT_COMPONENTS, &maxComponents);

	//std::cout << "Max vertex output components: "  --> 128个
	//	<< maxComponents << std::endl;

	//[&]() -> int
	//	{
	//		// 函数体
	//		return 0;
	//	}
	// [&]：需要使用的外部局部变量，默认按引用捕获。

	const int runResult = [&]()-> int
	{
		Shader shader(
			STARGATE_ASSET_DIR "/shaders/lit.vert",
			STARGATE_ASSET_DIR "/shaders/lit.frag"
		);

		if (!shader.IsValid())
		{
			std::cerr << "Failed to create shader program" << std::endl;
			return -1;
		}

		SceneData scene;
		scene.modelPath = "models/material_test/cube.obj";

		// 默认场景：文件不存在或读取失败时使用
		scene.objects.resize(3);
		// 左边
		scene.objects[0].name = "Left Cube";
		scene.objects[0].position = glm::vec3(-1.5f, 0.0f, -1.0f);
		scene.objects[0].rotation = glm::vec3(20.0f, 30.0f, 0.0f);
		scene.objects[0].scale = glm::vec3(0.7f);

		// 中间
		scene.objects[1].name = "Middle Cube";
		scene.objects[1].rotation = glm::vec3(20.0f, 0.0f, 0.0f);

		// 右边
		scene.objects[2].name = "Right Cube";
		scene.objects[2].position = glm::vec3(1.5f, 0.0f, -1.0f);
		scene.objects[2].rotation = glm::vec3(0.0f, -25.0f, 15.0f);
		scene.objects[2].scale = glm::vec3(0.8f);

		const char* sceneFilePath = STARGATE_ASSET_DIR "/scenes/default.json";

		// 检查是否已经有保存的场景
		std::error_code sceneFileError;
		const bool sceneFileExists = std::filesystem::exists(sceneFilePath, sceneFileError);

		if (sceneFileError)
		{
			std::cerr << "Failed to check scene file: " << sceneFileError.message() << '\n';
		}
		else if (sceneFileExists)
		{
			if (LoadScene(sceneFilePath, scene))
			{
				std::cout << "Scene loaded.\n";
			}
			else
			{
				std::cerr << "Using default scene.\n";
			}
		}

		// 根据场景里的模型路径加载模型
		const std::string modelFullPath = std::string(STARGATE_ASSET_DIR) + "/" + scene.modelPath;

		// 使用unique 为了在运行中加载场景时，能够替换当前模型，并自动释放旧模型
		// 赋值转移的是指针的所有权，不需要复制或移动 Model 对象本身
		//model 释放旧模型 → 接管新模型
		// loadedModel     → 变为空指针
		auto model = std::make_unique<Model>(modelFullPath.c_str());

		if (!model->IsValid())
		{
			std::cerr << "Failed to create model"<< std::endl;
			return -1;
		}

		shader.Bind();
		shader.SetInt("uTexture", 0);

		// ------------------------光照信息-----------------------------------------
		glm::vec3 toLightDirection(0.0f, 0.0f, 1.0f);
		glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
		float ambientStrength = 0.2f;
		// ------------------------光照信息 END-----------------------------------------

		const bool timerResolutionEnabled = timeBeginPeriod(1) == TIMERR_NOERROR; // 设置系统定时器分辨率为1ms
		lastTime = glfwGetTime(); // 重新记录进入主循环前的时间

		const float moveSpeed = 0.8f; // 偏移量变化速度
		const float rotationSpeed = 90.0f; // 旋转速度，单位为度每秒

		Camera camera;
		InputState inputState;
		inputState.camera = &camera;

		glfwSetWindowUserPointer(window, &inputState); // 将缩放因子指针存储在窗口的用户指针中

		//glfwSetScrollCallback(
		//	window,
		//	[](GLFWwindow* window, double, double yoffset)
		//	{
		//		InputState* input = static_cast<InputState*>(
		//			glfwGetWindowUserPointer(window)
		//			);

		//		input->scaleFactor += static_cast<float>(yoffset) * 0.1f;

		//		if (input->scaleFactor < 0.1f)
		//		{
		//			input->scaleFactor = 0.1f;
		//		}
		//	}
		//);

		// 隐藏光标
		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPosCallback(window,
			[](GLFWwindow* window, double xpos, double ypos)
			{
				// 取回缩放变量的地址
				InputState* input = static_cast<InputState*>(
					glfwGetWindowUserPointer(window)
					);

				if (!input->cameraControlActive)
				{
					//没有控制相机的时候，重置
					input->firstMouse = true;
					return;
				}

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

		std::size_t selectedObjectIndex = 0;

		// 显示鼠标
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		// 创建Framebuffer
		Framebuffer sceneFramebuffer(800, 600);
		if(!sceneFramebuffer.IsValid())
		{ 
			std::cerr << "Failed to create scene framebuffer\n";
			return -1;
		}

		// 创建Imgui上下文
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		// 开启面板停靠
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		//使用深色的界面
		ImGui::StyleColorsDark();

		//DPI的适配
		// 获取窗体所在屏幕的显示缩放比例
		const float uiScale = ImGui_ImplGlfw_GetContentScaleForWindow(window);

		ImGuiStyle& style = ImGui::GetStyle();
		style.ScaleAllSizes(uiScale);	// 空间间距，边距等尺寸
		style.FontScaleDpi = uiScale;	// 字体大小

		//连接GLFW和OPENGL后端
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 330 core");

		std::string sceneSaveMessage;
		bool loadSceneRequested = false;

		// 主循环
		while (!glfwWindowShouldClose(window))
		{
			// 记录本次循环的开始时间
			const double frameStartTime = glfwGetTime();

			if (loadSceneRequested)
			{
				loadSceneRequested = false;

				SceneData loadedScene;

				if (!LoadScene(sceneFilePath, loadedScene))
				{
					sceneSaveMessage = "Load failed. Check console.";
				}
				else
				{
					const std::string loadedModelPath =
						std::string(STARGATE_ASSET_DIR) + "/" +
						loadedScene.modelPath;

					auto loadedModel =
						std::make_unique<Model>(loadedModelPath.c_str());

					if (!loadedModel->IsValid())
					{
						sceneSaveMessage = "Model load failed. Check console.";
					}
					else
					{
						// 场景数据和模型都准备好后，再替换当前状态
						scene = std::move(loadedScene);
						//会完成三件事：
						//	1. 删除 model 原来拥有的旧 Model 对象，调用它的析构函数。
						//	2. model 接管 loadedModel 拥有的新对象。
						//	3. loadedModel 变为空指针。
						model = std::move(loadedModel);

						selectedObjectIndex = 0;

						sceneSaveMessage = "Scene loaded.";
					}
				}

				// 加载可能耗时，避免后面的移动使用过大的时间间隔
				lastTime = glfwGetTime();
			}

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

			if (!ImGui::GetIO().WantCaptureKeyboard)
			{
				SceneObject& ketboardObject = scene.objects[selectedObjectIndex];

				if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
				{
					ketboardObject.position.x -= moveDistance;
				}

				if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
				{
					ketboardObject.position.x += moveDistance;
				}

				const float rotationStep = rotationSpeed * static_cast<float>(deltaTime); // 计算本帧的旋转角度增量
				if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
				{
					ketboardObject.rotation.y += rotationStep; // 逆时针旋转
				}
				if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
				{
					ketboardObject.rotation.y -= rotationStep; // 顺时针旋转
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
			}

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

			// 开始新的一帧
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			// 在主窗口的工作区域创建停靠空间
			ImGui::DockSpaceOverViewport();	//提供可以停靠的区域

			const ImGuiIO& io = ImGui::GetIO();
			ImGui::SetNextWindowPos(ImVec2(20.0f * uiScale, 20.0f * uiScale), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSize(ImVec2(220.0f * uiScale, 260.0f * uiScale), ImGuiCond_FirstUseEver);
			if (ImGui::Begin("Hierarchy"))
			{
				for (std::size_t i = 0; i < scene.objects.size(); ++i)
				{
					ImGui::PushID(static_cast<int>(i)); //后面的控件生成 ID 时包含,它需要成对使用，否则这个索引会继续影响后面的控件。

					const bool selected = (i == selectedObjectIndex);

					if (ImGui::Selectable(scene.objects[i].name.c_str(), selected))
					{
						selectedObjectIndex = i;
					}

					ImGui::PopID();
				}
			}
			ImGui::End();

			// 在列表处理之后取得选中物体
			SceneObject& selectedObject = scene.objects[selectedObjectIndex];
			//此时只是生成界面，实际绘制放在后面。
			//ImGui::ShowDemoWindow();
			ImGui::SetNextWindowPos(ImVec2(260.0f * uiScale, 20.0f * uiScale), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSize(ImVec2(360.0f * uiScale, 420.0f * uiScale), ImGuiCond_FirstUseEver);

			if (ImGui::Begin("Inspector"))
			{
				ImGui::Text(
					"Selected: %s",
					scene.objects[selectedObjectIndex].name.c_str()
				);

				ImGui::DragFloat3("Position", glm::value_ptr(selectedObject.position), 0.01f);
				ImGui::DragFloat3("Rotation", glm::value_ptr(selectedObject.rotation), 0.5f);
				ImGui::DragFloat3("Scale", glm::value_ptr(selectedObject.scale), 0.01f, 0.1f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

				if (ImGui::Button("Reset Transform"))
				{
					selectedObject.position = glm::vec3(0.0f);
					selectedObject.rotation = glm::vec3(0.0f);
					selectedObject.scale = glm::vec3(1.0f);
				}

				ImGui::Separator();

				// 保存场景
				if (ImGui::Button("Save Scene"))
				{
					const bool saved = SaveScene(
						STARGATE_ASSET_DIR "/scenes/default.json",
						scene.modelPath.c_str(),
						scene.objects.data(),
						scene.objects.size()
					);

					sceneSaveMessage = saved
						? "Scene saved."
						: "Save failed. Check console.";
				}

				ImGui::SameLine();

				if (ImGui::Button("Load Scene"))
				{
					loadSceneRequested = true;
					sceneSaveMessage = "Loading scene...";
				}

				if (!sceneSaveMessage.empty())
				{
					ImGui::TextWrapped("%s", sceneSaveMessage.c_str());
				}

				ImGui::Separator();
				ImGui::Text("Lighting");

				ImGui::ColorEdit3("Light Color", glm::value_ptr(lightColor));
				ImGui::SliderFloat("Ambient", &ambientStrength, 0.0f, 1.0f);

				ImGui::Separator();
				ImGui::Text("FPS:%.1f", ImGui::GetIO().Framerate);
			}
			ImGui::End();

			// IMGUI的Scene视图
			bool sceneHovered = false;
			bool sceneImageVisible = false;

			ImGui::SetNextWindowSize(ImVec2(800.0f, 600.0f ), ImGuiCond_FirstUseEver);
			if (ImGui::Begin("Scene", nullptr,
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoScrollWithMouse))
			{
				// 面板中的剩余可用空间
				const ImVec2 sceneSize = ImGui::GetContentRegionAvail();

				// 将界面尺寸转换成实际图像的像素尺寸
				const int sceneWidth = static_cast<int>(sceneSize.x * io.DisplayFramebufferScale.x);
				const int sceneHeight = static_cast<int>(sceneSize.y * io.DisplayFramebufferScale.y);

				if (sceneWidth > 0 && sceneHeight > 0)
				{
					sceneFramebuffer.Resize(sceneWidth, sceneHeight);

					const ImTextureRef sceneTexture(
						static_cast<ImTextureID>(sceneFramebuffer.GetColorTexture())
					);

					ImGui::Image(sceneTexture, sceneSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

					sceneHovered = ImGui::IsItemHovered();
					sceneImageVisible = true;	//如果 Scene 面板折叠了，或者内部没有足够的显示空间，这段代码不会执行，变量就保持 false。
				}
			}
			ImGui::End();

			// 鼠标的处理
			const bool windowFocused = glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_TRUE;
			const bool rightMouseDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

			//在Scene场景里按下才算开始控制相机
			const bool startCameraControl = sceneHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right);

			// 已经开始控制后，保持到松开右键
			const bool enableCamera = 
				windowFocused &&
				sceneImageVisible &&
				rightMouseDown &&
				(inputState.cameraControlActive || startCameraControl);

			if (enableCamera != inputState.cameraControlActive)
			{
				inputState.cameraControlActive = enableCamera;
				inputState.firstMouse = true;

				glfwSetInputMode(
					window,
					GLFW_CURSOR,
					enableCamera ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL
				);
			}

			// 鼠标位于场景画面上时，滚轮缩放选中的物体
			if (windowFocused && sceneHovered)
			{
				SceneObject& scrollObject = scene.objects[selectedObjectIndex];

				scrollObject.scale += glm::vec3(io.MouseWheel * 0.1f);
				scrollObject.scale = glm::max(scrollObject.scale, 0.1f);
			}

			const float aspectRatio = static_cast<float>(sceneFramebuffer.GetWidth()) / static_cast<float>(sceneFramebuffer.GetHeight());
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
			sceneFramebuffer.Bind();

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

			for (const SceneObject& object : scene.objects)
			{
				glm::mat4 transform(1.0f);

				transform = glm::translate(transform, object.position);

				transform = glm::rotate(transform, glm::radians(object.rotation.y),glm::vec3(0.0f, 1.0f, 0.0f));
				//glm::radians(rotationAngle) 将角度转换为弧度，因为glm::rotate函数需要弧度值
				// glm::vec3(0, 0, 1) 表示绕Z轴旋转，这里假设三角形在XY平面上
				transform = glm::rotate(transform, glm::radians(object.rotation.x), glm::vec3(1, 0, 0));
				transform = glm::rotate(transform,glm::radians(object.rotation.z),glm::vec3(0.0f, 0.0f, 1.0f));
				transform = glm::scale(transform, object.scale);

				shader.SetMat4("uTransform", transform); // 将偏移量传递给着色器
				model->Draw(shader);
			}

			Framebuffer::Unbind();
			//恢复窗口的视图大小
			glViewport(0, 0, framebufferWidth, framebufferHeight);
			// 清除窗口背景
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// IMGUI界面绘制顺序
			ImGui::Render();

			//绘制在3维场景上方
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		glfwSetCursorPosCallback(window, nullptr);
		glfwSetScrollCallback(window, nullptr); // 取消滚轮回调，避免悬空指针
		glfwSetWindowUserPointer(window, nullptr); // 清除窗口的用户指针，避免悬空指针

		return 0;
	}();
	// Shader已经释放，现在销毁OpenGL Context
	glfwDestroyWindow(window);
	glfwTerminate();
	return runResult;
}
