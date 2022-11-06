#include "Pipeline.h"

Pipeline::Pipeline(uint32_t width, uint32_t height, SampleCount sampleCount)
	: width(width), height(height), sampleCount(sampleCount) {
	switch (sampleCount) {
	case SampleCount::Count1:
		multipleBuffer = 1;
		break;
	case SampleCount::Count2:
		multipleBuffer = 2;
		break;
	case SampleCount::Count4:
		multipleBuffer = 4;
		break;
	case SampleCount::Count8:
		multipleBuffer = 8;
		break;
	case SampleCount::Count16:
		multipleBuffer = 16;
		break;
	}
	zBuffer = new float[width * height * multipleBuffer];
	framebuffer = new Vector4f[width * height * multipleBuffer];
	for (int i = 0; i < width * height * multipleBuffer; i++) {
		zBuffer[i] = FLT_MAX;
		framebuffer[i] = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

void Pipeline::Clear(Vector4f colorValue, float depthValue) {
	for (int i = 0; i < width * height * multipleBuffer; i++) {
		framebuffer[i] = colorValue;
		zBuffer[i] = depthValue;
	}
}

Vector3f Pipeline::ReadFramebuffer(int x, int y) {
	Vector3f color;
	for (uint32_t i = 0; i < multipleBuffer; i++) {
		int pos = width * height * i + y * width + x;
		color.x = color.x + framebuffer[pos].x;
		color.y = color.y + framebuffer[pos].y;
		color.z = color.z + framebuffer[pos].z;
	}
	return 1.0f / multipleBuffer * color;
}

void Pipeline::WriteFramebuffer(int x, int y, int samplePoint, Vector4f color) {
	int pos = samplePoint * width * height + y * width + x;
	color.x = color.x * color.w + framebuffer[pos].x * (1.0f - color.w);
	color.y = color.y * color.w + framebuffer[pos].y * (1.0f - color.w);
	color.z = color.z * color.w + framebuffer[pos].z * (1.0f - color.w);
	color.w = color.w;
	framebuffer[pos] = color;
}

bool Pipeline::DepthTest(int x, int y, int samplePoint, float z) {
	if (z <= zBuffer[samplePoint * width * height + y * width + x]) {
		zBuffer[samplePoint * width * height + y * width + x] = z;
		return true;
	}
	return false;
}

bool Pipeline::CullFace(Vector2i v0, Vector2i v1, Vector2i v2)const {
	Vector3f v01 = Vector3f(v1.x - v0.x, v1.y - v0.y, 0.0f);
	Vector3f v02 = Vector3f(v2.x - v0.x, v2.y - v0.y, 0.0f);
	Vector3f cross = Cross(v01, v02);
	if (cross.z > 0.0f)
		return false;                     //ÄæÊ±ÕëÈÆÐò£¬²»ÌÞ³ý
	return true;                          //Ë³Ê±ÕëÈÆÐò£¬ÌÞ³ý
}

Matrix4x4f Pipeline::ScreenSpaceMatrix()const {
	return Matrix4x4f(
		Vector4f(width / 2.0f, 0.0f, 0.0f, 0.0f),
		Vector4f(0.0f, -(int)height / 2.0f, 0.0f, 0.0f),
		Vector4f(0.0f, 0.0f, 1.0f, 0.0f),
		Vector4f(width / 2.0f, height / 2.0f, 0.0f, 1.0f)
	);
}

void GouraudShaderPipeline::DrawData(Vertex** vertices, size_t count) {
	switch (topologyType) {
	case TopologyType::PointList: {
		for (size_t i = 0; i < count; i++) {
			GouraudShader::FragmentInput fragmentInput;
			Vector4f position = shader.VertexShader(shader.InputAssembler(vertices[i]), fragmentInput);

			position.x /= position.w;
			position.y /= position.w;
			position.z /= position.w;
			position.w = 1.0f;
			position = Multiply(position, ScreenSpaceMatrix());

			uint32_t x = position.x + 0.5f, y = position.y + 0.5f;
			if (x >= width || x < 0 || y >= height || y < 0)return;
			float depth = position.z;

			//DepthTest
			if (!DepthTest(x, y, 0, depth)) continue;

			WriteFramebuffer(x, y, 0, shader.FragmentShader(fragmentInput));
		}
		break;
	}

	case TopologyType::LineList:
	case TopologyType::LineStrip: {
		for (size_t i = 0; i < count / 2; i++) {
			GouraudShader::FragmentInput fragmentInput[2];

			Vector4f position[2];
			position[0] = shader.VertexShader(shader.InputAssembler(vertices[i * 2]), fragmentInput[0]);
			position[1] = shader.VertexShader(shader.InputAssembler(vertices[i * 2 + 1]), fragmentInput[1]);

			for (auto& p : position) {
				p.x /= p.w;
				p.y /= p.w;
				p.z /= p.w;
				p.w = 1.0f;
				p = Multiply(p, ScreenSpaceMatrix());
			}

			int x0 = position[0].x + 0.5f, y0 = position[0].y + 0.5f;
			int x1 = position[1].x + 0.5f, y1 = position[1].y + 0.5f;
			float z0 = position[0].z, z1 = position[1].z;

			bool steep = false;

			if (abs(x1 - x0) < abs(y1 - y0))
				steep = true;

			if (((x0 > x1) && (!steep)) || ((y0 > y1) && (steep))) {
				std::swap(x0, x1);
				std::swap(y0, y1);
				std::swap(z0, z1);
				std::swap(fragmentInput[0], fragmentInput[1]);
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
					else sub += 2 * dy;

					float lerpPercent = (float)(x - x0) / (float)(x1 - x0);

					if (x >= width || x < 0 || y >= height || y < 0) continue;
					if (!DepthTest(x, y, 0, Lerp(z0, z1, lerpPercent))) continue;

					GouraudShader::FragmentInput finalInput;
					finalInput.worldPos = Lerp(fragmentInput[0].worldPos, fragmentInput[1].worldPos, lerpPercent);
					finalInput.texCoord = Lerp(fragmentInput[0].texCoord, fragmentInput[1].texCoord, lerpPercent);
					finalInput.normal = Lerp(fragmentInput[0].normal, fragmentInput[1].normal, lerpPercent);

				    WriteFramebuffer(x, y, 0, shader.FragmentShader(finalInput));
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
					else sub += 2 * dx;

					float lerpPercent = (float)(y - y0) / (float)(y1 - y0);

					if (x >= width || x < 0 || y >= height || y < 0) continue;
					if (!DepthTest(x, y, 0, Lerp(z0, z1, lerpPercent))) continue;

					GouraudShader::FragmentInput finalInput;
					finalInput.worldPos = Lerp(fragmentInput[0].worldPos, fragmentInput[1].worldPos, lerpPercent);
					finalInput.texCoord = Lerp(fragmentInput[0].texCoord, fragmentInput[1].texCoord, lerpPercent);
					finalInput.normal = Lerp(fragmentInput[0].normal, fragmentInput[1].normal, lerpPercent);

					WriteFramebuffer(x, y, 0, shader.FragmentShader(finalInput));
				}
			}
		}
		break;
	}

	case TopologyType::TriangleList: {
		for (size_t i = 0; i < count / 3; i++) {
			GouraudShader::FragmentInput fragmentInput[3];

			Vector4f position[3];
			position[0] = shader.VertexShader(shader.InputAssembler(vertices[i * 3]), fragmentInput[0]);
			position[1] = shader.VertexShader(shader.InputAssembler(vertices[i * 3 + 1]), fragmentInput[1]);
			position[2] = shader.VertexShader(shader.InputAssembler(vertices[i * 3 + 2]), fragmentInput[2]);

			float worldZ[3];

			for (size_t i = 0; i < 3; i++) {
				worldZ[i] = position[i].z;
				position[i].x /= position[i].w;
				position[i].y /= position[i].w;
				position[i].z /= position[i].w;
				position[i].w = 1.0f;

				position[i] = Multiply(position[i], ScreenSpaceMatrix());
			}

			int x0 = position[0].x + 0.5f, y0 = position[0].y + 0.5f;
			int x1 = position[1].x + 0.5f, y1 = position[1].y + 0.5f;
			int x2 = position[2].x + 0.5f, y2 = position[2].y + 0.5f;

			int minX = x0, minY = y0, maxX = x0, maxY = y0;

			if (x1 < minX) minX = x1;
			if (y1 < minY) minY = y1;
			if (x1 > maxX) maxX = x1;
			if (y1 > maxY) maxY = y1;

			if (x2 < minX) minX = x2;
			if (y2 < minY) minY = y2;
			if (x2 > maxX) maxX = x2;
			if (y2 > maxY) maxY = y2;

			for (int y = minY; y <= maxY; y++) {
				for (int x = minX; x <= maxX; x++) {
					if (x >= width || x < 0 || y >= height || y < 0)continue;

					std::vector<Vector2f> coords;
					switch (sampleCount) {
					case SampleCount::Count1: {
						coords.push_back(Vector2f(x, y));
						break;
					}
					case SampleCount::Count2: {
						constexpr float offset = 0.25f;
						coords.push_back(Vector2f(x - offset, y - offset));
						coords.push_back(Vector2f(x + offset, y + offset));
						break;
					}
					case SampleCount::Count4: {
						constexpr float offset[] = { 6.0f / 16.0f, 2.0f / 16.0f };
						coords.push_back(Vector2f(x - offset[1], y - offset[0]));
						coords.push_back(Vector2f(x + offset[0], y - offset[1]));
						coords.push_back(Vector2f(x - offset[0], y + offset[1]));
						coords.push_back(Vector2f(x + offset[1], y + offset[0]));
						break;
					}
					case SampleCount::Count8: {
						constexpr float offset[] = { 1.0f / 16.0f, 3.0f / 16.0f, 5.0f / 16.0f, 7.0f / 16.0f };
						coords.push_back(Vector2f(x - offset[3], y - offset[0]));
						coords.push_back(Vector2f(x - offset[1], y - offset[2]));
						coords.push_back(Vector2f(x + offset[0], y - offset[1]));
						coords.push_back(Vector2f(x + offset[3], y - offset[3]));
						coords.push_back(Vector2f(x - offset[2], y + offset[2]));
						coords.push_back(Vector2f(x - offset[0], y + offset[1]));
						coords.push_back(Vector2f(x + offset[1], y + offset[3]));
						coords.push_back(Vector2f(x + offset[2], y + offset[0]));
						break;
					}
					case SampleCount::Count16: {
						constexpr float offset[] = { 1.0f / 16.0f, 2.0f / 16.0f, 3.0f / 16.0f, 4.0f / 16.0f, 5.0f / 16.0f, 6.0f / 16.0f, 7.0f / 16.0f, 0.5f };
						coords.push_back(Vector2f(x - offset[6], y - offset[7]));
						coords.push_back(Vector2f(x - offset[4], y - offset[1]));
						coords.push_back(Vector2f(x - offset[3], y - offset[5]));
						coords.push_back(Vector2f(x - offset[0], y - offset[2]));

						coords.push_back(Vector2f(x, y - offset[6]));

						coords.push_back(Vector2f(x + offset[2], y - offset[4]));
						coords.push_back(Vector2f(x + offset[3], y - offset[0]));
						coords.push_back(Vector2f(x + offset[6], y - offset[3]));

						coords.push_back(Vector2f(x - offset[7], y));

						coords.push_back(Vector2f(x - offset[5], y + offset[3]));
						coords.push_back(Vector2f(x - offset[2], y + offset[1]));
						coords.push_back(Vector2f(x - offset[1], y + offset[5]));

						coords.push_back(Vector2f(x + offset[0], y + offset[0]));
						coords.push_back(Vector2f(x + offset[1], y + offset[4]));
						coords.push_back(Vector2f(x + offset[4], y + offset[2]));
						coords.push_back(Vector2f(x + offset[5], y + offset[6]));
						break;
					}
					}

					Vector4f finalColor;
					bool detected = false;

					for (int i = 0; i < coords.size(); i++) {
						Vector3f mass = CalcBarycentric(Vector2f(x0, y0), Vector2f(x1, y1), Vector2f(x2, y2), coords[i]);
						if (mass.x >= -1e-5f && mass.y >= -1e-5f && mass.z >= -1e-5f) {
							float depth = mass.x * position[0].z + mass.y * position[1].z + mass.z * position[2].z;
							if (!DepthTest(x, y, i, depth)) continue;

							if (!detected) {
								GouraudShader::FragmentInput finalInput;
								finalInput.worldPos = PerspectiveCorrectInterpolate(worldZ[0], worldZ[1], worldZ[2], fragmentInput[0].worldPos, fragmentInput[1].worldPos, fragmentInput[2].worldPos, mass.y, mass.z);
								finalInput.texCoord = PerspectiveCorrectInterpolate(worldZ[0], worldZ[1], worldZ[2], fragmentInput[0].texCoord, fragmentInput[1].texCoord, fragmentInput[2].texCoord, mass.y, mass.z);
								finalInput.normal = PerspectiveCorrectInterpolate(worldZ[0], worldZ[1], worldZ[2], fragmentInput[0].normal, fragmentInput[1].normal, fragmentInput[2].normal, mass.y, mass.z);
								finalColor = shader.FragmentShader(finalInput);

								detected = true;
							}

							WriteFramebuffer(x, y, i, finalColor);
						}
					}
				}
			}
			/*float dx = 1.0f / width, dy = 1.0f / height;

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
			}*/
		}
		break;
	}
	}
}

void GouraudShaderPipeline::Draw(size_t baseVertexOffset, size_t count) {
	std::vector<Vertex*> vertices;
	for (size_t i = 0; i < count; i++) {
		vertices.push_back(&vertexBuffer[baseVertexOffset + i]);
	}
	DrawData(vertices.data(), vertices.size());
}

void GouraudShaderPipeline::DrawIndexed(size_t indexOffset, size_t baseVertexOffset, size_t count) {
	std::vector<Vertex*> vertices;
	for (size_t i = 0; i < count; i++) {
		vertices.push_back(&vertexBuffer[baseVertexOffset + indexBuffer[indexOffset + i]]);
	}
	DrawData(vertices.data(), vertices.size());
}