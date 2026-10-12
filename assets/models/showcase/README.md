# Flight Helmet 展示场景

当前默认场景为飞行头盔、圆形展台和地面。启动 StarGate 或点击 File → Load Scene 即可读取 assets/scenes/default.json；showcase.json 是这一展示场景的独立副本。原场景保存在同目录的 before_showcase_*.json。

## 模型与外部贴图

主模型：models/showcase/flight_helmet/FlightHelmet.gltf。
来源：https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/FlightHelmet
官方描述：USAAF A-11 Flying Helmet on a wooden stand。
许可：CC0-1.0；官方来源说明保存在 SOURCE_README.md，完整许可保存在 LICENSE-CC0.txt。

使用外部 .gltf + .bin + PNG 文件，没有采用内嵌纹理。Assimp 从 glTF 材质读取 BASE_COLOR 图片路径，ModelLoader 相对于模型目录解析路径，Texture2D 使用 stb_image 解码，再上传到 GPU。无需在 C++ 中写死这些图片文件名。

模型有 6 个子网格、55,392 个导入顶点、94,722 个三角形。当前使用 5 张独立基础颜色纹理：RubberWood、GlassPlastic、MetalParts、LeatherParts 为 2048×2048，Lenses 为 1024×1024。
原模型的法线贴图和 Occlusion/Roughness/Metallic 图片也保留在目录中，供后续学习使用；当前渲染代码尚未采样它们，也没有实现玻璃透射。

展台和地面为本项目原创 OBJ/MTL 文件，使用纯色材质，不触发缺图备用纹理。整个展示资源目录约 48.5 MB，主要是模型原始 PNG。

## 取景与变换

沿用当前相机初始位置 (0, 0, 3)，朝 -Z，垂直 FOV 45°。
头盔 position = (0, -0.69994858, -0.1)，rotation = (0, -20, 0)，scale = (2.4, 2.4, 2.4)。
展台 position = (0, -0.85, -0.1)，直径 1.56，高 0.15。
地面 position = (0, -0.853, 0)，覆盖 8×8 世界单位。

## 建议灯光参数

当前灯光和相机参数不在场景 JSON 中持久化。添加场景不会自动改变 StarGate.cpp 的灯光参数。

在 StarGate.cpp 初始化光照的位置，将对应变量改为：

```cpp
glm::vec3 toLightDirection(-0.6f, 0.8f, 1.0f);
glm::vec3 lightColor(1.0f, 0.96f, 0.90f);
float ambientStrength = 0.22f;
```

每帧设置高光 uniform 的位置改为：

```cpp
shader.SetFloat("uSpecularStrength", 0.18f);
shader.SetFloat("uShininess", 64.0f);
```

初始化清屏颜色的位置改为：

```cpp
glClearColor(0.035f, 0.045f, 0.065f, 1.0f);
```

uToLightDirection 表示从表面指向光源的世界空间方向，着色器中会归一化；这是一盏无限远的方向光，没有位置或距离衰减。该方向将光源放在世界的左上前方，实际传播方向与其相反。
光源为稍暖的白色；ImGui 默认 0～255 显示时，对应约 (255, 245, 230)。环境光为常量白光近似 0.22。漫反射系数仍是 lit.frag 中的 0.8。高光强度 0.18、指数 64，避免皮革和木头都出现过亮的塑料式高光。
这些参数是现有 Blinn–Phong 演示的无量纲系数，不是物理灯光单位。当前没有阴影、PBR、法线贴图或 Gamma/sRGB 流程。

## 验证

已通过本项目 ModelLoader、Model、Texture2D、Shader 和 SceneSerializer 的实际代码验证：
- 3 个场景模型成功加载；5 张外部基础颜色贴图完整解码，无备用贴图。
- 场景名称、每个物体的模型路径、位置、旋转和缩放在保存/恢复后保持一致。
- 在 NVIDIA RTX 4060 Laptop GPU 的隐藏 OpenGL 3.3 窗口中完成真实渲染，没有 GL 错误。
- 预览使用当前项目的 GLSL 和模型绘制实现；没有实现模型原本的完整 PBR 效果。

StarGate 源代码及灯光默认值未修改；建议参数需要按上面代码应用。
