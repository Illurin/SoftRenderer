#pragma once
#include "Camera.h"
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

struct Texture {
	Texture() {}
	Texture(const char* path, uint32_t BPP) {
		LoadImageWithSTB(path, BPP);
	}
	void Release() {
		stbi_image_free(source);
	}

	uint32_t width, height, BPP;
	uint64_t imageSize;
	stbi_uc* source;

	Vector4f borderColor = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);

	enum class AddressMode {
		REAPEAT = 0,
		MIRROR,
		CLAMP,
		BORDER
	};
	AddressMode addressMode = AddressMode::REAPEAT;
	
	void SetSampler(AddressMode addressMode, Vector4f borderColor) {
		this->addressMode = addressMode;
		this->borderColor = borderColor;
	}

	void LoadImageWithSTB(const char* path, uint32_t BPP) {
		this->BPP = BPP;
		int channelInFile;
		stbi_uc* source = stbi_load(path, reinterpret_cast<int*>(&width), reinterpret_cast<int*>(&height), &channelInFile, 4);

		uint64_t pixelRowPitch = ((uint64_t)width * (uint64_t)BPP + 7) / 8;
		imageSize = pixelRowPitch * (uint64_t)height;

		this->source = reinterpret_cast<uint8_t*>(source);
	}

	float TextureAddress(float coord, AddressMode addressMode) {
		switch (addressMode) {
		case AddressMode::REAPEAT:
			while (coord > 1.0f) {
				coord -= 1.0f;
			}
			while (coord < 0.0f) {
				coord += 1.0f;
			}
			break;
		case AddressMode::MIRROR:
		{
			bool mirror = false;
			while (coord > 1.0f) {
				coord -= 1.0f;
				mirror = !mirror;
			}
			while (coord < 0.0f) {
				coord += 1.0f;
				mirror = !mirror;
			}
			if (mirror)
				coord = 1.0f - coord;
			break;
		}
		case AddressMode::CLAMP:
			coord = coord > 1.0f ? 1.0f : coord;
			coord = coord < 0.0f ? 0.0f : coord;
			break;
		case AddressMode::BORDER:
			coord = coord > 1.0f || coord < 0.0f ? -1.0f : coord;
			break;
		}
		return coord;
	}

	Vector4f SampleNearest(float x, float y) {
		x = TextureAddress(x, addressMode);
		y = TextureAddress(y, addressMode);
		if (x == -1.0f || y == -1.0f)
			return borderColor;

		y = 1.0f - y;

		x = x * (float)this->width;
		y = y * (float)this->height;

		x -= 0.5f;
		y -= 0.5f;

		int width = x;
		int height = y;

		if (x - width > 0.5f)width++;
		if (x - height > 0.5f)height++;

		int byte = BPP / 32;

		Vector4f color;
		color.x = (float)source[height * this->width * byte * 4 + width * 4] / 255.0f;
		color.y = (float)source[height * this->width * byte * 4 + width * 4 + 1] / 255.0f;
		color.z = (float)source[height * this->width * byte * 4 + width * 4 + 2] / 255.0f;
		color.w = (float)source[height * this->width * byte * 4 + width * 4 + 3] / 255.0f;
		return color;
	}

	Vector4f SampleBilinear(float x, float y) {
		x = TextureAddress(x, addressMode);
		y = TextureAddress(y, addressMode);
		if (x == -1.0f || y == -1.0f)
			return borderColor;

		y = 1.0f - y;
		
		x = x * (float)this->width;
		y = y * (float)this->height;

		x -= 0.5f;
		y -= 0.5f;

		int width = x;
		int height = y;
		
		int byte = BPP / 32;

		Vector4f color[4];
		for (int i = 0; i < 4; i++) {
			if (i == 1 && x > 0.0f && x < 9.0f)
				width++;
			if (i == 2 && x > 0.0f && x < 9.0f)
				width--;
			if (i == 3 && x > 0.0f && x < 9.0f)
				width++;
			if (i == 2 && y > 0.0f && y < 9.0f)
				height++;

			color[i].x = (float)source[height * this->width * byte * 4 + width * 4] / 255.0f;
			color[i].y = (float)source[height * this->width * byte * 4 + width * 4 + 1] / 255.0f;
			color[i].z = (float)source[height * this->width * byte * 4 + width * 4 + 2] / 255.0f;
			color[i].w = (float)source[height * this->width * byte * 4 + width * 4 + 3] / 255.0f;
		}

		float lerpX = (x - width) / 1.0f;
		float lerpY = (y - height) / 1.0f;

		color[0] = Lerp(color[0], color[1], lerpX);
		color[2] = Lerp(color[2], color[3], lerpX);
		return Lerp(color[0], color[2], lerpY);
	}
};

struct Light {
	Vector3f ambientLight;
	Vector3f strength;
	float fallOffStart;					   //point/spot light only
	Vector3f direction;					   //directional/spot light only
	float fallOffEnd;					   //point/spot light only
	Vector3f position;					   //point/spot light only
	float spotPower;					   //spot light only
};

struct Material {
	Vector3f diffuseAlbedo;
	Vector3f fresnelR0;
	float shininess;
};

Vector3f CalcBarycentric(float x0, float y0, float x1, float y1, float x2, float y2, float px, float py) {
	float Ux = x1 - x0;
	float Uy = y1 - y0;
	float Vx = x2 - x0;
	float Vy = y2 - y0;
	float POx = x0 - px;
	float POy = y0 - py;

	Vector3f kAB1 = Cross(Vector3f(Ux, Vx, POx), Vector3f(Uy, Vy, POy));
	
	if (abs(kAB1.z) < 1.0f) {
		return Vector3f(-1.0f, 1.0f, 1.0f);
	}

	float A = kAB1.x / kAB1.z;
	float B = kAB1.y / kAB1.z;

	return Vector3f(1.0f - A - B, A, B);
}

class Renderer {
public:
	Renderer(SDL_Renderer* renderer, int width, int height, int sampleCount) :renderer(renderer), width(width), height(height), sampleCount(sampleCount) {
		zBuffer = new float[width * height * sampleCount];
		frameBuffer = new Vector4f[width * height * sampleCount];
		for (int i = 0; i < width * height * sampleCount; i++) {
			zBuffer[i] = FLT_MAX;
			frameBuffer[i] = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
		}
	}
	~Renderer(){
		delete[] zBuffer;
		delete[] frameBuffer;
	}
	void SetWorldMatrix(Matrix4x4f matrix) { this->worldMatrix = matrix; }
	void SetViewMatrix(Matrix4x4f matrix) { this->viewMatrix = matrix; }
	void SetProjectionMatrix(Matrix4x4f matrix) { this->projMatrix = matrix; }
	void SetVertexBuffer(Vertex* vertexBuffer, int num) {
		this->vertices.resize(num);
		this->vertexBuffer = vertexBuffer;
	}
	void SetLight(Vector3f ambientLight, Light& light) {
		this->ambientLight = ambientLight;
		this->light = light;
	}
	void SetEyePosition(Vector3f eyePos) {
		this->eyePos = eyePos;
	}
	void SetMaterial(Material& material) {
		this->material = material;
	}

	void Clear(Vector4f clearValue) {
		for (int i = 0; i < width * height * sampleCount; i++) {
			frameBuffer[i] = clearValue;
		}
	}
	
	void Present() {
		if (sampleCount == 4) {
			for (int i = 0; i < width * height; i++) {
				Vector4f color;
				color.x = frameBuffer[i].x * 0.25f
					+ frameBuffer[width * height + i].x * 0.25f
					+ frameBuffer[width * height * 2 + i].x * 0.25f
					+ frameBuffer[width * height * 3 + i].x * 0.25f;
				color.y = frameBuffer[i].y * 0.25f
					+ frameBuffer[width * height + i].y * 0.25f
					+ frameBuffer[width * height * 2 + i].y * 0.25f
					+ frameBuffer[width * height * 3 + i].y * 0.25f;
				color.z = frameBuffer[i].z * 0.25f
					+ frameBuffer[width * height + i].z * 0.25f
					+ frameBuffer[width * height * 2 + i].z * 0.25f
					+ frameBuffer[width * height * 3 + i].z * 0.25f;
				SDL_SetRenderDrawColor(renderer, (Uint8)(color.x * 255), (Uint8)(color.y * 255), (Uint8)(color.z * 255), SDL_ALPHA_OPAQUE);
				SDL_Point point;
				point.x = i % width;
				point.y = i / width;
				SDL_RenderDrawPoints(renderer, &point, 1);
			}
		}
		else {
			for (int i = 0; i < width * height; i++) {
				Vector4f color = frameBuffer[i];
				SDL_SetRenderDrawColor(renderer, (Uint8)(color.x * 255), (Uint8)(color.y * 255), (Uint8)(color.z * 255), SDL_ALPHA_OPAQUE);
				SDL_Point point;
				point.x = i % width;
				point.y = i / width;
				SDL_RenderDrawPoints(renderer, &point, 1);
			}
		}
	}

	Vector4f FragmentShading(Vector4f diffuse, Vector3f normal) {
		Vector3f lightVec = light.position - eyePos;

		float lambertFactor = max(Dot(normal, light.direction), 0.0f);
		Vector3f lightStrength = light.strength * lambertFactor;

		/*float distance = sqrt(lightVec.x * lightVec.x + lightVec.y * lightVec.y + lightVec.z * lightVec.z);
		Vector3f lightStrength = light.strength / (distance * distance);

		lightStrength = lambertFactor * light.strength;

		float spotFactor = Dot(-lightVec, light.direction);
		spotFactor = spotFactor > 0.0f ? spotFactor : 0.0f;
		spotFactor = pow(spotFactor, light.spotPower);
		lightStrength = spotFactor * lightStrength;*/
		
		//计算微平面
		const float shininess = material.shininess * 256.0f;
		Vector3f halfVec = (normal + light.direction).Normalize();
		float roughnessFactor = (shininess + 8.0f) * pow(max(Dot(halfVec, normal), 0.0f), shininess) / 8.0f;

		//计算菲涅耳方程
		float cosIncidentAngle = saturate(Dot(normal, lightVec));
		float f0 = 1.0f - cosIncidentAngle;
		Vector3f reflectPercent = material.fresnelR0 + pow(f0, 5) * (Vector3f(1.0f) - material.fresnelR0);

		//计算镜面反射值
		Vector3f specularAlbedo = roughnessFactor * reflectPercent;

		//(漫反射 + 镜面反射) * 光强
		Vector3f lightingResult = (material.diffuseAlbedo + specularAlbedo) * lightStrength;

		//计算最终颜色
		Vector4f litColor = Vector4f((ambientLight + lightingResult), 1.0f) * diffuse;

		litColor.x = saturate(litColor.x);
		litColor.y = saturate(litColor.y);
		litColor.z = saturate(litColor.z);

		return litColor;
	}

	void DrawPixel(SDL_Renderer* renderer, int x, int y, Vector4f color) {
		color.x = color.x * color.w + frameBuffer[y * width + x].x * (1.0f - color.w);
		color.y = color.y * color.w + frameBuffer[y * width + x].y * (1.0f - color.w);
		color.z = color.z * color.w + frameBuffer[y * width + x].z * (1.0f - color.w);
		color.w = 1.0f;
		frameBuffer[y * width + x] = color;
	}
	void DrawPixel(SDL_Renderer* renderer, int x, int y, int samplePoint, Vector4f color) {
		int index = samplePoint * width * height + y * width + x;
		color.x = color.x * color.w + frameBuffer[index].x * (1.0f - color.w);
		color.y = color.y * color.w + frameBuffer[index].y * (1.0f - color.w);
		color.z = color.z * color.w + frameBuffer[index].z * (1.0f - color.w);
		color.w = 1.0f;
		frameBuffer[index] = color;
	}

	void DrawPoint(int index) {
		DrawVertex(index);
		vertices[index].position.x /= vertices[index].position.w;
		vertices[index].position.y /= vertices[index].position.w;
		vertices[index].position.z /= vertices[index].position.w;
		vertices[index].position.w = 1.0f;

		Vector4f point = Multiply(vertices[index].position, ScreenSpaceMatrix());
		int x = point.x + 0.5f, y = point.y + 0.5f;
		float z = point.z;
		if (x >= width || x < 0 || y >= height || y < 0)return;

		for (int i = 0; i < sampleCount; i++) {
			DrawPixel(renderer, x, y, i, vertices[index].color);
		}
	}

	void DrawLine(int* indices) {
		for (int i = 0; i < 2; i++) {
			DrawVertex(indices[i]);
			vertices[indices[i]].position.x /= vertices[indices[i]].position.w;
			vertices[indices[i]].position.y /= vertices[indices[i]].position.w;
			vertices[indices[i]].position.z /= vertices[indices[i]].position.w;
			vertices[indices[i]].position.w = 1.0f;
		}

		Vector4f point0 = Multiply(vertices[indices[0]].position, ScreenSpaceMatrix());
		Vector4f point1 = Multiply(vertices[indices[1]].position, ScreenSpaceMatrix());

		int x0 = point0.x + 0.5f, y0 = point0.y + 0.5f;
		int x1 = point1.x + 0.5f, y1 = point1.y + 0.5f;
		float z0 = point0.z, z1 = point1.z;

		Vector4f color0 = vertices[indices[0]].color;
		Vector4f color1 = vertices[indices[1]].color;

		bool steep = false;

		if (abs(x1 - x0) < abs(y1 - y0))
			steep = true;

		if (((x0 > x1) && (!steep)) || ((y0 > y1) && (steep))) {
			int temp = x1;
			x1 = x0;
			x0 = temp;
			temp = y1;
			y1 = y0;
			y0 = temp;

			float tempZ = z1;
			z1 = z0;
			z0 = tempZ;

			Vector4f tempVec = color0;
			color0 = color1;
			color1 = tempVec;
		}

		int dx = x1 - x0;
		int dy = y1 - y0;

		int x = x0;
		int y = y0;

		if (!steep) {
			int d = 1;
			if (dy < 0) {
				d = -1;
				dy = -dy;
			}

			int sub = 2 * dy - dx;

			while (x < x1) {
				x++;
				if (sub > 0) {
					sub += 2 * dy - 2 * dx;
					y += d;
				}
				else {
					sub += 2 * dy;
				}
				if (x >= width || x < 0 || y >= height || y < 0)
					continue;
				float depth = Lerp(z0, z1, (float)(x - x0) / (float)(x1 - x0));
				if (!DepthTest(x, y, depth))
					continue;
				DrawPixel(renderer, x, y, Lerp(color0, color1, (float)(x - x0) / (float)(x1 - x0)));
			}
		}
		else {
			int d = 1;
			if (dx < 0) {
				d = -1;
				dx = -dx;
			}

			int sub = 2 * dx - dy;

			while (y < y1) {
				y++;
				if (sub > 0) {
					sub += 2 * dx - 2 * dy;
					x += d;
				}
				else {
					sub += 2 * dx;
				}
				if (x >= width || x < 0 || y >= height || y < 0)
					continue;
				float depth = Lerp(z0, z1, (float)(y - y0) / (float)(y1 - y0));
				if (!DepthTest(x, y, depth))
					continue;
				DrawPixel(renderer, x, y, Lerp(color0, color1, (float)(y - y0) / (float)(y1 - y0)));
			}
		}
	}

	void DrawLine(int* indices, Texture* texture) {
		float z[2];
		for (int i = 0; i < 2; i++) {
			DrawVertex(indices[i]);
			z[i] = vertices[indices[i]].position.z;
			vertices[indices[i]].position.x /= vertices[indices[i]].position.w;
			vertices[indices[i]].position.y /= vertices[indices[i]].position.w;
			vertices[indices[i]].position.z /= vertices[indices[i]].position.w;
			vertices[indices[i]].position.w = 1.0f;
		}

		Vector4f point0 = Multiply(vertices[indices[0]].position, ScreenSpaceMatrix());
		Vector4f point1 = Multiply(vertices[indices[1]].position, ScreenSpaceMatrix());

		int x0 = point0.x + 0.5f, y0 = point0.y + 0.5f;
		int x1 = point1.x + 0.5f, y1 = point1.y + 0.5f;
		float z0 = point0.z, z1 = point1.z;

		Vector2f texCoord0 = vertices[indices[0]].texCoord;
		Vector2f texCoord1 = vertices[indices[1]].texCoord;

		bool steep = false;

		if (abs(x1 - x0) < abs(y1 - y0))
			steep = true;

		if (((x0 > x1) && (!steep)) || ((y0 > y1) && (steep))) {
			int temp = x1;
			x1 = x0;
			x0 = temp;
			temp = y1;
			y1 = y0;
			y0 = temp;

			float tempZ = z1;
			z1 = z0;
			z0 = tempZ;

			Vector2f tempVec = texCoord0;
			texCoord0 = texCoord1;
			texCoord1 = tempVec;
		}

		int dx = x1 - x0;
		int dy = y1 - y0;

		int x = x0;
		int y = y0;

		if (!steep) {
			int d = 1;
			if (dy < 0) {
				d = -1;
				dy = -dy;
			}

			int sub = 2 * dy - dx;

			while (x < x1) {
				x++;
				if (sub > 0) {
					sub += 2 * dy - 2 * dx;
					y += d;
				}
				else {
					sub += 2 * dy;
				}
				if (x >= width || x < 0 || y >= height || y < 0)
					continue;
				float depth = Lerp(z0, z1, (float)(x - x0) / (float)(x1 - x0));
				if (!DepthTest(x, y, depth))
					continue;
				Vector2f texCoord = PerspectiveCorrectInterpolate(z[0], z[1], texCoord0, texCoord1, (float)(x - x0) / (float)(x1 - x0));
				DrawPixel(renderer, x, y, texture->SampleBilinear(texCoord.x, texCoord.y));
			}
		}
		else {
			int d = 1;
			if (dx < 0) {
				d = -1;
				dx = -dx;
			}

			int sub = 2 * dx - dy;

			while (y < y1) {
				y++;
				if (sub > 0) {
					sub += 2 * dx - 2 * dy;
					x += d;
				}
				else {
					sub += 2 * dx;
				}
				if (x >= width || x < 0 || y >= height || y < 0)
					continue;
				float depth = Lerp(z0, z1, (float)(y - y0) / (float)(y1 - y0));
				if (!DepthTest(x, y, depth))
					continue;
				Vector2f texCoord = PerspectiveCorrectInterpolate(z[0], z[1], texCoord0, texCoord1, (float)(y - y0) / (float)(y1 - y0));
				DrawPixel(renderer, x, y, texture->SampleBilinear(texCoord.x, texCoord.y));
			}
		}
	}

	void DrawTriangle(SDL_Renderer* renderer, int x0, int y0, int x1, int y1, int x2, int y2, Vector4f color) {
		if (y0 == y1 && y1 == y2) {
			return;
		}

		int temp;
		if (y0 > y1)temp = y0, y0 = y1, y1 = temp, temp = x0, x0 = x1, x1 = temp;
		if (y0 > y2)temp = y0, y0 = y2, y2 = temp, temp = x0, x0 = x2, x2 = temp;
		if (y1 > y2)temp = y1, y1 = y2, y2 = temp, temp = x1, x1 = x2, x2 = temp;

		int height = y2 - y0;

		for (int i = 0; i < height; i++) {
			bool split = i > y1 - y0 || y1 == y0;
			int segmentHeight = split ? y2 - y1 : y1 - y0;

			float alpha = (float)i / height;
			float beta = (float)(i - (split ? y1 - y0 : 0)) / segmentHeight;

			int p0 = x0 + (x2 - x0) * alpha;
			int p1 = split ? x1 + (x2 - x1) * beta : x0 + (x1 - x0) * beta;

			if (p1 < p0)temp = p0, p0 = p1, p1 = temp;

			for (int p = p0; p <= p1; p++) {
				DrawPixel(renderer, p, y0 + i, color);
			}
		}
	}

	void DrawTriangle(int* indices) {
		float z[3];
		for (int i = 0; i < 3; i++) {
			DrawVertex(indices[i]);
			z[i] = vertices[indices[i]].position.z;
			vertices[indices[i]].position.x /= vertices[indices[i]].position.w;
			vertices[indices[i]].position.y /= vertices[indices[i]].position.w;
			vertices[indices[i]].position.z /= vertices[indices[i]].position.w;
			vertices[indices[i]].position.w = 1.0f;
		}

		Vector4f point0 = Multiply(vertices[indices[0]].position, ScreenSpaceMatrix());
		Vector4f point1 = Multiply(vertices[indices[1]].position, ScreenSpaceMatrix());
		Vector4f point2 = Multiply(vertices[indices[2]].position, ScreenSpaceMatrix());
	
		int x0 = point0.x + 0.5f, y0 = point0.y + 0.5f;
		int x1 = point1.x + 0.5f, y1 = point1.y + 0.5f;
		int x2 = point2.x + 0.5f, y2 = point2.y + 0.5f;

		Vector4f color0 = vertices[indices[0]].color;
		Vector4f color1 = vertices[indices[1]].color;
		Vector4f color2 = vertices[indices[2]].color;

		int minX = x0, minY = y0, maxX = x0, maxY = y0;

		if (x1 < minX)minX = x1;
		if (y1 < minY)minY = y1;
		if (x1 > maxX)maxX = x1;
		if (y1 > maxY)maxY = y1;

		if (x2 < minX)minX = x2;
		if (y2 < minY)minY = y2;
		if (x2 > maxX)maxX = x2;
		if (y2 > maxY)maxY = y2;

		for (int y = minY; y <= maxY; y++) {
			for (int x = minX; x <= maxX; x++) {
				if (x >= width || x < 0 || y >= height || y < 0)continue;
				if (sampleCount == 4) {
					float offset[2] = { 1.0f / 16.0f * 6.0f, 1.0f / 16.0f * 2.0f };
					Vector2f position[4];
					position[0] = Vector2f(x - offset[1], y + offset[0]);
					position[1] = Vector2f(x + offset[0], y + offset[1]);
					position[2] = Vector2f(x - offset[0], y - offset[1]);
					position[3] = Vector2f(x + offset[1], y - offset[0]);

					for (int i = 0; i < 4; i++) {
						Vector3f mass = CalcBarycentric(x0, y0, x1, y1, x2, y2, position[i].x, position[i].y);
						if (mass.x >= -1e-5f && mass.y >= -1e-5f && mass.z >= -1e-5f) {
							float depth = mass.x * vertices[indices[0]].position.z + mass.y * vertices[indices[1]].position.z + mass.z * vertices[indices[2]].position.z;
							if (!DepthTest(x, y, i, depth))
								continue;

							Vector4f color = PerspectiveCorrectInterpolate(z[0], z[1], z[2], color0, color1, color2, mass.y, mass.z);
							DrawPixel(renderer, x, y, i, color);
						}
					}
				}
				else {
					Vector3f mass = CalcBarycentric(x0, y0, x1, y1, x2, y2, x, y);
					if (mass.x >= -1e-5f && mass.y >= -1e-5f && mass.z >= -1e-5f) {
						float depth = mass.x * vertices[indices[0]].position.z + mass.y * vertices[indices[1]].position.z + mass.z * vertices[indices[2]].position.z;
						if (!DepthTest(x, y, depth))
							continue;

						Vector4f color = PerspectiveCorrectInterpolate(z[0], z[1], z[2], color0, color1, color2, mass.y, mass.z);
						DrawPixel(renderer, x, y, color);
					}
				}
			}
		}
	}

	void DrawTriangle(int* indices, Texture* texture) {
		float z[3];
		for (int i = 0; i < 3; i++) {
			DrawVertex(indices[i]);
			z[i] = vertices[indices[i]].position.z;
			vertices[indices[i]].position.x /= vertices[indices[i]].position.w;
			vertices[indices[i]].position.y /= vertices[indices[i]].position.w;
			vertices[indices[i]].position.z /= vertices[indices[i]].position.w;
			vertices[indices[i]].position.w = 1.0f;
		}

		Vector4f point0 = Multiply(vertices[indices[0]].position, ScreenSpaceMatrix());
		Vector4f point1 = Multiply(vertices[indices[1]].position, ScreenSpaceMatrix());
		Vector4f point2 = Multiply(vertices[indices[2]].position, ScreenSpaceMatrix());

		int x0 = point0.x + 0.5f, y0 = point0.y + 0.5f;
		int x1 = point1.x + 0.5f, y1 = point1.y + 0.5f;
		int x2 = point2.x + 0.5f, y2 = point2.y + 0.5f;

		if (CullFace(Vector2i(x0, y0), Vector2i(x1, y1), Vector2i(x2, y2)))return;

		Vector2f texCoord0 = vertices[indices[0]].texCoord;
		Vector2f texCoord1 = vertices[indices[1]].texCoord;
		Vector2f texCoord2 = vertices[indices[2]].texCoord;

		int minX = x0, minY = y0, maxX = x0, maxY = y0;

		if (x1 < minX)minX = x1;
		if (y1 < minY)minY = y1;
		if (x1 > maxX)maxX = x1;
		if (y1 > maxY)maxY = y1;

		if (x2 < minX)minX = x2;
		if (y2 < minY)minY = y2;
		if (x2 > maxX)maxX = x2;
		if (y2 > maxY)maxY = y2;
		
		for (int y = minY; y <= maxY; y++) {
			for (int x = minX; x <= maxX; x++) {
				if (x >= width || x < 0 || y >= height || y < 0)continue;
				if (sampleCount == 4) {
					float offset[2] = { 1.0f / 16.0f * 6.0f, 1.0f / 16.0f * 2.0f };
					Vector2f position[4];
					position[0] = Vector2f(x - offset[1], y + offset[0]);
					position[1] = Vector2f(x + offset[0], y + offset[1]);
					position[2] = Vector2f(x - offset[0], y - offset[1]);
					position[3] = Vector2f(x + offset[1], y - offset[0]);
					
					Vector4f finalColor;
					bool detected = false;

					for (int i = 0; i < 4; i++) {
						Vector3f mass = CalcBarycentric(x0, y0, x1, y1, x2, y2, position[i].x, position[i].y);
						if (mass.x >= -1e-5f && mass.y >= -1e-5f && mass.z >= -1e-5f) {
							float depth = mass.x * vertices[indices[0]].position.z + mass.y * vertices[indices[1]].position.z + mass.z * vertices[indices[2]].position.z;
							if (!DepthTest(x, y, i, depth))
								continue;

							if (!detected) {
								Vector2f texCoord = PerspectiveCorrectInterpolate(z[0], z[1], z[2], texCoord0, texCoord1, texCoord2, mass.y, mass.z);
								Vector3f normal = PerspectiveCorrectInterpolate(z[0], z[1], z[2], vertices[indices[0]].normal, vertices[indices[1]].normal, vertices[indices[2]].normal, mass.y, mass.z);
								finalColor = FragmentShading(texture->SampleBilinear(texCoord.x, texCoord.y), normal);
								detected = true;
							}
							DrawPixel(renderer, x, y, i, finalColor);
						}
					}
				}
				else {
					Vector3f mass = CalcBarycentric(x0, y0, x1, y1, x2, y2, x, y);
					if (mass.x >= -1e-5f && mass.y >= -1e-5f && mass.z >= -1e-5f) {
						float depth = mass.x * vertices[indices[0]].position.z + mass.y * vertices[indices[1]].position.z + mass.z * vertices[indices[2]].position.z;
						if (!DepthTest(x, y, depth))
							continue;

						Vector2f texCoord = PerspectiveCorrectInterpolate(z[0], z[1], z[2], texCoord0, texCoord1, texCoord2, mass.y, mass.z);
						Vector3f normal = PerspectiveCorrectInterpolate(z[0], z[1], z[2], vertices[indices[0]].normal, vertices[indices[1]].normal, vertices[indices[2]].normal, mass.y, mass.z);

						Vector4f color = FragmentShading(texture->SampleBilinear(texCoord.x, texCoord.y), normal);
						DrawPixel(renderer, x, y, color);
					}
				}
			}
		}
	}
	
	void DrawTriangle(int* indices, Texture* texture, int maxMipLevel) {
		float z[3];
		for (int i = 0; i < 3; i++) {
			DrawVertex(indices[i]);
			z[i] = vertices[indices[i]].position.z;
			vertices[indices[i]].position.x /= vertices[indices[i]].position.w;
			vertices[indices[i]].position.y /= vertices[indices[i]].position.w;
			vertices[indices[i]].position.z /= vertices[indices[i]].position.w;
			vertices[indices[i]].position.w = 1.0f;
		}

		Vector4f point0 = Multiply(vertices[indices[0]].position, ScreenSpaceMatrix());
		Vector4f point1 = Multiply(vertices[indices[1]].position, ScreenSpaceMatrix());
		Vector4f point2 = Multiply(vertices[indices[2]].position, ScreenSpaceMatrix());

		int x0 = point0.x, y0 = point0.y;
		int x1 = point1.x, y1 = point1.y;
		int x2 = point2.x, y2 = point2.y;

		Vector2f texCoord0 = vertices[indices[0]].texCoord;
		Vector2f texCoord1 = vertices[indices[1]].texCoord;
		Vector2f texCoord2 = vertices[indices[2]].texCoord;

		int minX = x0, minY = y0, maxX = x0, maxY = y0;

		if (x1 < minX)minX = x1;
		if (y1 < minY)minY = y1;
		if (x1 > maxX)maxX = x1;
		if (y1 > maxY)maxY = y1;

		if (x2 < minX)minX = x2;
		if (y2 < minY)minY = y2;
		if (x2 > maxX)maxX = x2;
		if (y2 > maxY)maxY = y2;

		float dx = 1.0f / width, dy = 1.0f / height;

		for (int y = minY; y <= maxY; y += 2) {
			for (int x = minX; x <= maxX; x += 2) {
				Vector3f mass[4];
				Vector2f texCoord[4];
				mass[0] = CalcBarycentric(x0, y0, x1, y1, x2, y2, x, y);
				mass[1] = CalcBarycentric(x0, y0, x1, y1, x2, y2, x + 1, y);
				mass[2] = CalcBarycentric(x0, y0, x1, y1, x2, y2, x, y + 1);
				mass[3] = CalcBarycentric(x0, y0, x1, y1, x2, y2, x + 1, y + 1);
				texCoord[0] = PerspectiveCorrectInterpolate(z[0], z[1], z[2], texCoord0, texCoord1, texCoord2, mass[0].y, mass[0].z);
				texCoord[1] = PerspectiveCorrectInterpolate(z[0], z[1], z[2], texCoord0, texCoord1, texCoord2, mass[1].y, mass[1].z);
				texCoord[2] = PerspectiveCorrectInterpolate(z[0], z[1], z[2], texCoord0, texCoord1, texCoord2, mass[2].y, mass[2].z);
				texCoord[3] = PerspectiveCorrectInterpolate(z[0], z[1], z[2], texCoord0, texCoord1, texCoord2, mass[3].y, mass[3].z);
			    
				float Lx0 = powf(texCoord[0].x / dx - texCoord[1].x / dx, 2.0f) + powf(texCoord[0].y / dy - texCoord[1].y / dy, 2.0f);
				float Lx1 = powf(texCoord[2].x / dx - texCoord[3].x / dx, 2.0f) + powf(texCoord[2].y / dy - texCoord[3].y / dy, 2.0f);
				float Ly0 = powf(texCoord[0].x / dx - texCoord[2].x / dx, 2.0f) + powf(texCoord[0].y / dy - texCoord[2].y / dy, 2.0f);
				float Ly1 = powf(texCoord[1].x / dx - texCoord[3].x / dx, 2.0f) + powf(texCoord[1].y / dy - texCoord[3].y / dy, 2.0f);

				for (int i = 0; i < 4; i++) {
					if (!(mass[i].x >= -1e-5f && mass[i].y >= -1e-5f && mass[i].z >= -1e-5f))continue;
					if (x >= width || x < 0 || y >= height || y < 0)continue;
					float depth = mass[i].x * vertices[indices[0]].position.z + mass[i].y * vertices[indices[1]].position.z + mass[i].z * vertices[indices[2]].position.z;
					if (!DepthTest(i % 2 == 0 ? x : x + 1, i < 2 ? y : y + 1, depth))continue;

					float L;
					switch (i) {
					case 0:
						L = sqrtf(Lx0 > Ly0 ? Lx0 : Ly0);
						break;
					case 1:
						L = sqrtf(Lx0 > Ly1 ? Lx0 : Ly1);
						break;
					case 2:
						L = sqrtf(Lx1 > Ly0 ? Lx1 : Ly0);
						break;
					case 3:
						L = sqrtf(Lx1 > Ly1 ? Lx1 : Ly1);
					}
					
					float mipLevel = logf(L);
					int mipLevel0 = mipLevel;

					Vector4f color;
					if (mipLevel - mipLevel0 != 0.0f) {
						float percent = mipLevel - mipLevel0;
						Vector4f color0 = texture[mipLevel0].SampleBilinear(texCoord[i].x, texCoord[i].y);
						Vector4f color1 = texture[mipLevel0 + 1].SampleBilinear(texCoord[i].x, texCoord[i].y);
						color = Lerp(color0, color1, percent);
					}
					else
						color = texture[mipLevel0].SampleBilinear(texCoord[i].x, texCoord[i].y);

					mipLevel = mipLevel > maxMipLevel ? maxMipLevel : mipLevel;
					DrawPixel(renderer, i % 2 == 0 ? x : x + 1, i < 2 ? y : y + 1, color);
				}
			}
		}
	}

private:
	void DrawVertex(int index) {
		Matrix4x4f transformation = Multiply(worldMatrix, Multiply(viewMatrix, projMatrix));
		vertices[index].position = Multiply(vertexBuffer[index].position, transformation);
		vertices[index].color = vertexBuffer[index].color;
		vertices[index].texCoord = vertexBuffer[index].texCoord;
		vertices[index].normal = Multiply(Vector4f(vertexBuffer[index].normal, 0.0f), worldMatrix).GetVector3f();
	}
	
	bool DepthTest(int x, int y, float z) {
		if (z <= zBuffer[y * width + x]) {
			zBuffer[y * width + x] = z;
			return true;
		}
		return false;
	}
	bool DepthTest(int x, int y, int samplePoint, float z) {
		if (z <= zBuffer[samplePoint * width * height + y * width + x]) {
			zBuffer[samplePoint * width * height + y * width + x] = z;
			return true;
		}
		return false;
	}

	bool CullFace(Vector2i v0, Vector2i v1, Vector2i v2) {
		Vector3f v01 = Vector3f(v1.x - v0.x, v1.y - v0.y, 0.0f);
		Vector3f v02 = Vector3f(v2.x - v0.x, v2.y - v0.y, 0.0f);
		Vector3f cross = Cross(v01, v02);
		if (cross.z > 0.0f)
			return false;                     //逆时针绕序，剔除
		return true;                          //顺时针绕序，不剔除
	}

	Matrix4x4f ScreenSpaceMatrix() {
		return Matrix4x4f(
			Vector4f(width / 2.0f, 0.0f, 0.0f, 0.0f),
			Vector4f(0.0f, -height / 2.0f, 0.0f, 0.0f),
			Vector4f(0.0f, 0.0f, 1.0f, 0.0f),
			Vector4f(width / 2.0f, height / 2.0f, 0.0f, 1.0f)
		);
	}

	Matrix4x4f worldMatrix;
	Matrix4x4f viewMatrix;
	Matrix4x4f projMatrix;

	std::vector<Vertex> vertices;
	Vertex* vertexBuffer;
	float* zBuffer;
	Vector4f* frameBuffer;

	SDL_Renderer* renderer;
	int width, height;
	int bufferWidth, bufferHeight;
	int sampleCount = 1;

	Vector3f eyePos;
	Vector3f ambientLight;
	Light light;
	Material material;
};