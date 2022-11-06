# SoftRenderer
A pipeline-like software rendering frame

仿图形渲染管线设计的CPU软渲染器。

已实现功能：
* 基于SDL2的窗口显示和绘制
* 基础光栅化算法，三维空间内绘制点，线，三角形
* Camera摄像机自由移动功能
* 深度缓冲和基于绕序的背面剔除
* 纹理映射（最近邻过滤和线性过滤）和各种纹理寻址模式
* 基本的基于Alpha分量的颜色混合
* 基本的Blinn-Phong和BRDF光照算法
* 基本的MSAA反走样算法
* 曲面细分，曲面简化，生成曲线/曲面

待完善功能：
* 建立完备的数学库
* 纹理Mipmap
* 完整的几何细分和LOD支持

相关文章：

[软光栅化渲染器学习笔记（一）：简单几何图元](https://illurin.github.io/2022/10/15/renderer01/)

[软光栅化渲染器学习笔记（二）：深入光栅化细节](https://illurin.github.io/2022/10/15/renderer02/)

[软光栅化渲染器学习笔记（三）：细分几何](https://illurin.github.io/2022/10/16/renderer03/)

# Core 核心部分

## 光栅化 Rasterization

### 着色器模块 Shader Module

**基础光照模块 LightingUtil**

基础光照模块内置了基于Blinn-Phong模型计算BRDF光照的一些基本方法，在进行高洛德着色时会使用到这些方法：

```C++
//计算菲涅耳方程的石里克近似
Vector3f SchlickFresnel(Vector3f R0, Vector3f normal, Vector3f lightVec)const;
//计算Blinn-Phong光照结果
Vector3f BlinnPhong(Vector3f lightStrength, Vector3f lightVec, Vector3f normal, Vector3f toEye, Material mat)const;
//计算方向光
Vector3f ComputeDirectionalLight(Light light, Material mat, Vector3f normal, Vector3f toEye)const;
//计算点光源
Vector3f ComputePointLight(Light light, Material mat, Vector3f pos, Vector3f normal, Vector3f toEye)const;
//计算聚光灯
Vector3f ComputeSpotLight(Light light, Material mat, Vector3f pos, Vector3f normal, Vector3f toEye)const;
```

参与光照计算的基本数据：

```C++
//灯光信息
struct Light {
	LightType type;
	Vector3f ambientLight;
	Vector3f strength;
	Vector3f direction;         //仅用于方向光/聚光灯
	Vector3f position;	    //仅用于点光源/聚光灯
	float spotPower;            //仅用于聚光灯
};

//材质信息
struct Material {
	Vector4f diffuseAlbedo;
	Vector3f fresnelR0;
	float roughness;
};
```

**高洛德着色 GouraudShader**

高洛德着色器是基于顶点数据插值计算光照颜色的基本着色器，在使用之前需要填充参与着色器计算的常量数据和纹理数据：

```C++
//场景信息常量
struct {
	Vector3f eyePos;
	Vector3f ambientLight;
	Matrix4x4f viewMatrix;
	Matrix4x4f projMatrix;
	Light light;
}passConstant;

//物体信息常量
struct {
	Matrix4x4f worldMatrix;
	Matrix4x4f normalMatrix;
}objectConstant;

//材质信息常量
struct {
	Vector4f diffuseAlbedo;
	Vector3f fresnelR0;
	float roughness;
}materialConstant;

//采样器
Sampler sampler;
//纹理
Texture* texture{ nullptr };
```

可以通过修改GouraudShader的顶点着色器和片元着色器来改变渲染的效果：

```C++
Vector4f VertexShader(VertexInput input, FragmentInput& output)const;
Vector4f FragmentShader(FragmentInput input)const;
```

### 渲染管线 Pipeline

创建GouraudShaderPipeline对象的方法：

```C++
GouraudShaderPipeline pipeline(width, height, SampleCount);
```

支持的图元拓扑类型：

```C++
enum class TopologyType {
	PointList = 0,
	LineList,
	TriangleList,
};
```

支持的MSAA反走样倍数：

```C++
enum class SampleCount {
	Count1 = 0,
	Count2,
	Count4,
	Count8,
	Count16
};
```

GouraudShaderPipeline的渲染设置：

```C++
//设置顶点缓冲
void SetVertexBuffer(Vertex* vertexBuffer);
//设置索引缓冲（非必须）
void SetIndexBuffer(uint32_t* indexBuffer);
//设置图元拓扑
void SetTopologyType(TopologyType topologyType);
//设置着色器数据（必须为GouraudShader）
void SetShader(GouraudShader shader);
```

默认深度测试采用的比较方程是小于等于，默认背面剔除采用的是顺时针绕序剔除：

```C++
bool DepthTest(int x, int y, int samplePoint, float z);
bool CullFace(Vector2i v0, Vector2i v1, Vector2i v2)const;
```

清空framebuffer和z-buffer数据方法：

```C++
pipeline.Clear(Vector4f(0.0f, 0.0f, 0.0f, 0.0f), FLT_MAX);
```

读出framebuffer存储的像素颜色数据方法：

```C++
Vector3f ReadFramebuffer(int x, int y);
```

DrawCall：

```C++
//只使用顶点缓冲
void Draw(size_t baseVertexOffset, size_t count);
//使用索引缓冲
void DrawIndexed(size_t indexOffset, size_t baseVertexOffset, size_t count);
```

### 渲染器 Renderer

渲染器接入了SDL库，可以将framebuffer存储的颜色数据最终显示在屏幕上。

创建Renderer对象的方法：

```C++
Renderer(SDL_Window* window);
```

将framebuffer呈现在屏幕上的方法：

```C++
void Present(Pipeline* pipeline)const;
```

## 数学库 Math

数学库采用行主序存储和右乘结合，内置了基本的向量和矩阵运算功能，支持的类型：

* Vector2i, Vector2f, Vector3i, Vector3f, Vector4f
* Matrix3x3f, Matrix4x4f

支持的基本运算:

* 向量的点乘（Dot），叉乘（Cross），归一化（Normalize）
* 矩阵的转置（Transpose），矩阵的行列式（Determinant），基于伴随矩阵的矩阵求逆（Inverse）
* 向量和矩阵，矩阵和矩阵的右乘（Multiply）

常用矩阵变换：

```C++
//缩放矩阵
Matrix4x4f Scale(float x, float y, float z);

//旋转矩阵
Matrix4x4f RotateX(float angle);
Matrix4x4f RotateY(float angle);
Matrix4x4f RotateZ(float angle);
Matrix4x4f RotateAxis(Vector3f axis, float angle);

//平移矩阵
Matrix4x4f Translate(float x, float y, float z);

//正交投影矩阵
Matrix4x4f OrthograpicProjection(float l, float r, float t, float b, float n, float f);
//透视投影矩阵
Matrix4x4f PerspectiveProjection(float l, float r, float t, float b, float n, float f);
```

# Function 功能部分

## 摄像机对象 Camera

创建摄像机对象的方法：

```C++
Camera();
Camera(float aspect); //在创建时设定宽高比
```

获取基本参数和矩阵：

```C++
Vector3f GetPosition3f()const;
Matrix4x4f GetViewMatrix4x4f()const;
Matrix4x4f GetProjMatrix4x4f()const;
```

设定相机的视锥体参数：

```C++
void SetLens(float fovY, float aspect, float nearZ, float farZ);
```

相机的基本观察，移动，旋转操作：

```C++
void LookAt(Vector3f pos, Vector3f target, Vector3f worldUp);
void Walk(float distance);
void Strafe(float distance);
void Pitch(float angle);
void RotateY(float angle);

//在每次操作完相机后更新观察矩阵
void UpdataViewMatrix();
```

## 纹理

### 采样器对象 Sampler

设置采样器对象的过滤器，UV方向上的寻址模式，边框寻址模式下的边框颜色：

```C++
Sampler(Filter filter, AddressMode addressModeU, AddressMode addressModeV);
void SetBorderColor(Vector4f color);
```

可供选择的过滤器：

```C++
enum class Filter {
	Nearest = 0,
    Linear
};
```

可供选择的寻址模式：

```C++
enum class AddressMode {
	Repeat = 0,
	Mirror,
	Clamp,
	Border
};
```

### 纹理对象 Texture

利用STB图像库载入图像数据：

```C++
//子资源数量为1（不使用Mipmap）
//直接使用构造函数
Texture(const char* path, uint32_t BPP);
//使用LoadImageWithSTB函数
void LoadImageWithSTB(const char* path, uint32_t BPP);
//多个子资源（使用Mipmap）
Texture(size_t subresourceCount);
void LoadImageWithSTB(const char* path, uint32_t BPP, size_t subresource);
```

释放Texture中存储的数据：

```C++
void Release();
```

## 模型对象 Model

有关使用Assimp库加载模型的介绍见：[Assimp与模型渲染的故事：模型加载，骨骼蒙皮动画](https://illurin.github.io/2022/10/15/assimp/)

使用Model类加载模型：

```C++
Model(std::string path);
```

Model对象下存储着模型的渲染数据：

```C++
std::vector<MaterialInfo> materials;
std::vector<Mesh::RenderInfo> renderInfo;
std::vector<std::string> texturePath;
```

## 细分 Tessellation

To be continued...