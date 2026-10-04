# StarGate

StarGate 是一个从零学习和实现的 C++ 游戏引擎项目，参考 [Piccolo](https://github.com/BoomingTech/Piccolo) 的设计逐步完善。

## 当前进度

- 使用 Visual Studio、CMake 和 C++20 构建。
- 已接入 GLFW，创建初始尺寸为 1280 × 720 的窗口。
- 创建并绑定 OpenGL 3.3 核心模式上下文，实现蓝色背景清屏和双缓冲显示。
- 每帧查询帧缓冲区的实际像素尺寸并设置视口，适应窗口大小变化。
- 计算帧间时间差，每 0.5 秒在窗口标题中更新平均 FPS 和平均帧耗时。
- 使用线程休眠进行手动限帧，目标为 60 FPS；在 Windows 上请求 1 毫秒定时器分辨率，并在退出时释放请求。
- 支持窗口事件处理、Esc 退出和基础错误输出。

当前完成的是窗口与基础清屏阶段，尚未绘制三角形或三维模型，也未接入物理系统。垂直同步暂时关闭，实际帧率会受到休眠精度和线程调度等因素影响，手动限帧不保证严格达到 60 FPS。

## 构建与运行

当前实现面向 Windows，需要显卡及驱动支持 OpenGL 3.3 核心模式。

1. 在 Visual Studio Installer 中安装“使用 C++ 的桌面开发”，并确认包含“用于 Windows 的 C++ CMake 工具”。
2. 克隆仓库后，在 Visual Studio 中通过“文件 → 打开 → 文件夹”打开包含 `CMakeLists.txt` 的根目录。
3. 等待 CMake 配置完成，选择 `x64 Debug` 配置和 `StarGate` 启动项。
4. 按 `Ctrl + F5` 运行，应看到蓝色背景，窗口标题显示 FPS 和平均帧耗时。
5. 按 `Esc` 或点击窗口关闭按钮退出。

GLFW 源码已包含在 `third_party/glfw` 中，无需额外下载。当前清屏使用 Windows 提供的基础 OpenGL 接口，CMake 已配置链接 `glfw`、`winmm` 和 `opengl32`。

## 目录结构

```text
StarGate/
├── CMakeLists.txt       # 构建目标和依赖
├── CMakePresets.json    # Visual Studio 构建配置
├── StarGate.cpp         # 程序入口、窗口循环、帧时间统计和 OpenGL 清屏
├── StarGate.h           # 预留项目头文件，当前未使用
└── third_party/glfw/    # GLFW 源码
```

## 后续计划

下一步接入 GLAD，加载现代 OpenGL 接口，并绘制第一个三角形。之后逐步实现输入管理、三维摄像机、纹理与模型加载、基础光照、场景和资源管理。

后续计划扩展 Vulkan 渲染后端，并支持在启动时选择 OpenGL 或 Vulkan；目前尚未实现后端切换。

## 第三方依赖

[GLFW](https://www.glfw.org/) 用于窗口、输入与 OpenGL 上下文管理，其许可说明保留在 [third_party/glfw/LICENSE.md](third_party/glfw/LICENSE.md) 中。
