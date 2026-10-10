# StarGate

StarGate 是一个从零学习和实现的 C++ 游戏引擎项目，参考 [Piccolo](https://github.com/BoomingTech/Piccolo) 的设计逐步完善。当前使用 OpenGL，通过可交互的三维演示学习渲染与引擎开发。

目前可以自由移动相机，观察带有木箱纹理的立方体，并调整物体变换、切换光源颜色。

## 当前进度

- 使用 Visual Studio、CMake 和 C++20 构建，当前面向 Windows。
- 使用 GLFW 创建初始尺寸为 1280 × 720 的窗口，处理窗口事件、键盘、鼠标和滚轮输入。
- 创建 OpenGL 3.3 核心模式上下文，通过 GLAD 加载 OpenGL 函数。
- 使用 VAO、VBO、EBO 和 `glDrawElements` 绘制立方体：24 个顶点、36 个索引，共 12 个三角形。
- 每个顶点包含位置、UV 和法线，共 8 个浮点数。同一几何角点在不同面上具有不同的 UV 或法线，因此按面保存顶点；每个面的两个三角形共享该面的顶点。
- 使用 `Shader` 类管理着色器编译、链接、错误输出、绑定与释放，提供 `SetInt`、`SetFloat`、`SetVec3` 和 `SetMat4` 设置 uniform。
- 使用 GLM 组合模型矩阵，完成平移、旋转和缩放；顶点着色器通过 `P × V × M` 计算齐次裁剪坐标。
- 使用 `Camera` 类管理相机位置、偏航角、俯仰角和视图矩阵。支持 WASD 移动和鼠标转向，归一化组合移动方向，避免斜向移动速度增加；俯仰角限制为 ±89°。
- 相机初始位置为 `(0, 0, 3)`，朝世界 −Z 方向观察，世界上方向为 +Y。使用右手坐标系和垂直视野角 45° 的透视投影，近、远裁剪面距离分别为 0.1 和 100。
- 启用深度测试，根据帧缓冲区实际尺寸更新视口和投影宽高比；窗口最小化时暂时跳过绘制。
- 使用 stb_image 读取 PNG 图片，翻转像素行以匹配当前 UV 方向，并解码为 RGBA 数据后上传到 OpenGL。读取失败时输出错误信息，使用 2×2 黑白棋盘作为备用纹理。
- 使用 `glGenerateMipmap` 生成 mipmap；缩小时采用 `GL_LINEAR_MIPMAP_LINEAR`，放大时采用 `GL_NEAREST`，UV 超出 0～1 时重复纹理。
- 使用模型矩阵的逆转置，将法线转换到世界空间；片段着色器归一化法线，通过 Lambert 余弦项计算方向光的漫反射，并叠加常量环境光近似。
- 从 C++ 通过 uniform 设置指向光源的方向、光源颜色和环境光强度，支持按键切换白光、暖色光和蓝色光。
- 按帧时间计算相机和物体的移动、旋转；支持滚轮缩放物体，最小缩放系数为 0.1。
- 每 0.5 秒在窗口标题中更新平均 FPS 和平均帧耗时。关闭垂直同步，通过线程休眠手动限帧，目标为 60 FPS；Windows 定时器分辨率请求在退出时成对释放。
- 纹理上传后释放图片的 CPU 内存，并在销毁 OpenGL 上下文前释放纹理、VAO、VBO、EBO 和着色器程序。

当前网格仍由代码定义，尚未接入模型文件加载。光照目前是基础方向光与常量环境光模型，尚未加入高光、阴影、sRGB／Gamma 处理和完整的物理渲染流程。

手动限帧不保证严格达到 60 FPS。窗口标题中的平均帧耗时包含限帧等待时间。

## 构建与运行

需要 Windows、Visual Studio 的“使用 C++ 的桌面开发”工作负载和 CMake 工具，以及支持 OpenGL 3.3 核心模式的显卡和驱动。

1. 在 Visual Studio 中通过“文件 → 打开 → 文件夹”打开包含 `CMakeLists.txt` 的项目根目录。
2. 等待 CMake 配置完成，选择 `x64 Debug` 配置和 `StarGate` 启动项。
3. 确认 `assets/textures/crate.png` 存在。
4. 按 `Ctrl + F5` 运行，应看到蓝色背景上的木箱立方体及表面明暗变化，窗口标题显示 FPS 和平均帧耗时。

也可以在 Visual Studio 的 x64 开发者 PowerShell 中，从项目根目录使用现有预设构建：

~~~powershell
cmake --preset x64-debug
cmake --build out/build/x64-debug
.\out\build\x64-debug\StarGate.exe
~~~

现有预设依赖 Visual Studio 的 MSVC、Ninja 和安装目录环境。

## 操作说明

| 操作 | 效果 |
| --- | --- |
| W / S | 沿相机当前朝向前进 / 后退 |
| A / D | 向左 / 向右移动相机 |
| 移动鼠标 | 调整相机偏航角和俯仰角 |
| 左 / 右方向键 | 向左 / 向右平移立方体 |
| Q / E | 增加 / 减小立方体的 Y 轴旋转角度 |
| 鼠标滚轮向上 / 向下 | 整体放大 / 缩小立方体 |
| 1 | 白光 |
| 2 | 暖色光 |
| 3 | 蓝色光 |
| Esc 或关闭窗口 | 退出程序 |

运行时捕获并隐藏鼠标光标。相机的前后移动跟随当前朝向，包括俯仰后的竖直分量。

立方体初始绕 Y 轴旋转 30°，另有固定的 20° X 轴倾斜。缩放系数最小为 0.1，当前未设置上限。

默认指向光源的世界空间方向为 `(0.5, 1.0, 0.3)`，环境光强度为 0.2。切换灯光颜色只影响直接漫反射项，环境光项保持白色。

## 图片资源

示例纹理位于 `assets/textures/crate.png`，是使用图像生成工具制作的木箱单面 PNG，用于学习演示。

CMake 通过 `STARGATE_ASSET_DIR` 将源码目录下的 `assets` 路径传入程序，读取图片不依赖启动时的工作目录。目前运行时读取源码目录中的资源；移动项目后应重新配置并构建。

当前纹理流程为：

~~~text
PNG 文件
  → stb_image 解码为 RGBA 像素
  → glTexImage2D 上传
  → 生成 mipmap
  → 纹理单元 0 绑定纹理对象
  → 片段着色器根据插值后的 UV 采样
  → 结合环境光和漫反射输出颜色
~~~

## 目录结构

~~~text
StarGate/
├── CMakeLists.txt
├── CMakePresets.json
├── README.md
├── assets/
│   └── textures/
│       └── crate.png    # 木箱示例纹理
├── src/
│   ├── StarGate.cpp     # 程序入口、输入、纹理、光照与立方体绘制
│   ├── StarGate.h       # 预留项目头文件，当前未使用
│   ├── Shader.h
│   ├── Shader.cpp       # 着色器与 uniform 接口
│   ├── Camera.h
│   ├── Camera.cpp       # 相机移动、转向与视图矩阵
│   └── StbImage.cpp     # stb_image 实现的编译入口
└── third_party/
    ├── glfw/            # 窗口与输入库源码
    ├── glad/            # OpenGL 函数加载代码
    ├── glm/             # 数学库头文件与许可说明
    └── stb/
        └── stb_image.h  # 图片解码库
~~~

## 后续目标

- **接下来：**整理网格数据与绘制资源，接入简单模型文件加载，构建包含多个物体的演示场景。
- **第一个月目标：**完成可交互的三维演示，包括相机、纹理、基础光照和简单模型加载，并整理构建、运行与实现说明。
- **第二个月目标：**完善场景与资源管理，加入基础编辑界面，支持选择物体、修改属性，以及保存和重新加载场景。

这些是学习目标，具体范围会根据实际进度调整。完整复刻 Piccolo 是长期目标；物理系统、编辑器、Vulkan 后端、AI 集成和 VR 研究功能尚未实现。

## 第三方依赖

依赖以源码或头文件形式保存在 `third_party` 中，构建时无需额外下载。

- [GLFW](https://www.glfw.org/)：窗口、输入与 OpenGL 上下文管理。许可说明位于 `third_party/glfw/LICENSE.md`。
- [GLAD](https://github.com/Dav1dde/glad)：加载 OpenGL 函数，当前生成配置为 OpenGL 3.3 Core。
- [GLM](https://github.com/g-truc/glm)：向量、矩阵和变换计算，以头文件形式接入。许可说明位于 `third_party/glm/copying.txt`。
- [stb_image](https://github.com/nothings/stb)：图片文件解码。通过 `src/StbImage.cpp` 编译实现；MIT／公有领域双许可说明保留在 `third_party/stb/stb_image.h` 文件末尾。
