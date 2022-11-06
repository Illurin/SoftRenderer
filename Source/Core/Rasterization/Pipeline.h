#pragma once
#include "Core/Rasterization/Shader.h"

enum class TopologyType {
	PointList = 0,
	LineList,
	LineStrip,
	TriangleList,
	TriangleStrip
};

enum class SampleCount {
	Count1 = 0,
	Count2,
	Count4,
	Count8,
	Count16
};

class Pipeline {
public:
	Pipeline(uint32_t width, uint32_t height, SampleCount sampleCount);
	~Pipeline() { delete[] zBuffer; delete[] framebuffer; }

	void SetVertexBuffer(Vertex* vertexBuffer) { this->vertexBuffer = vertexBuffer; }
	void SetIndexBuffer(uint32_t* indexBuffer) { this->indexBuffer = indexBuffer; }
	void SetTopologyType(TopologyType topologyType) { this->topologyType = topologyType; }

	void Clear(Vector4f colorValue, float depthValue);
	Vector3f ReadFramebuffer(int x, int y);

protected:
	void WriteFramebuffer(int x, int y, int samplePoint, Vector4f color);
	bool DepthTest(int x, int y, int samplePoint, float z);
	bool CullFace(Vector2i v0, Vector2i v1, Vector2i v2)const;

	Matrix4x4f ScreenSpaceMatrix()const;

	uint32_t width{ 0 }, height{ 0 }, multipleBuffer{ 1 };

	float* zBuffer;
	Vector4f* framebuffer;
	Vertex* vertexBuffer{ nullptr };
	uint32_t* indexBuffer{ nullptr };

	TopologyType topologyType{ TopologyType::TriangleList };
	SampleCount sampleCount{ SampleCount::Count1 };
};

class GouraudShaderPipeline : public Pipeline {
public:
	using Pipeline::Pipeline;

	void SetShader(GouraudShader shader) { this->shader = shader; }
	void Draw(size_t baseVertexOffset, size_t count);
	void DrawIndexed(size_t indexOffset, size_t baseVertexOffset, size_t count);
	
private:
	void DrawData(Vertex** vertices, size_t count);
	GouraudShader shader;
};