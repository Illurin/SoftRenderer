#include "Core/Rasterization/Renderer.h"
#include "Function/Model/Model.h"
#include "Function/Camera/Camera.h"
#include "Function/Tessellation/Tessellation.h"

const int width = 600;
const int height = 400;

int main(int argc, char* argv[]) {
	SDL_Window* window;
	window = SDL_CreateWindow("Soft Renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);

	Renderer renderer(window);

	GouraudShaderPipeline pipeline(width, height, SampleCount::Count4);

	Model model("Assets\\Model.fbx");

	std::vector<Texture> textures(model.texturePath.size());
	for (int i = 0; i < textures.size(); i++) {
		textures[i].LoadImageWithSTB(model.texturePath[i].c_str(), 32);
	}

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	for (auto& r : model.renderInfo) {
		for (auto& i : r.indices)
			indices.push_back(i + vertices.size());
		vertices.insert(vertices.end(), r.vertices.begin(), r.vertices.end());
	}

	Camera camera((float)width / (float)height);
	camera.SetLens(0.25f * M_PI, (float)width / height, 0.1f, 1000.0f);
	camera.LookAt(Vector3f(0.0f, 10.0f, 30.0f), Vector3f(0.0f, 10.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f));
	camera.UpdataViewMatrix();

	pipeline.Clear(Vector4f(0.0f, 0.0f, 0.0f, 0.0f), FLT_MAX);

	Light light;
	light.type = LightType::Directional;
	light.strength = Vector3f(0.0f, 0.0f, 2.0f);
	light.direction = Vector3f(1.0f, -1.0f, 0.0f).Normalize();

	GouraudShader shader;
	shader.passConstant.ambientLight = Vector3f(0.2f, 0.2f, 0.2f);
	shader.passConstant.eyePos = camera.GetPosition3f();
	shader.passConstant.light = light;
	shader.passConstant.viewMatrix = camera.GetViewMatrix4x4f();
	shader.passConstant.projMatrix = camera.GetProjMatrix4x4f();

	shader.objectConstant.worldMatrix = RotateX(-M_PI * 0.5f);
	shader.objectConstant.normalMatrix = shader.objectConstant.worldMatrix.Inverse().Transpose();

	shader.materialConstant.diffuseAlbedo = Vector4f(1.0f);
	shader.materialConstant.fresnelR0 = Vector3f(0.5f, 0.5f, 0.5f);
	shader.materialConstant.roughness = 0.2f;

	shader.sampler = Sampler(Sampler::Filter::Linear, Sampler::AddressMode::Repeat, Sampler::AddressMode::Repeat);
	shader.sampler.SetBorderColor(Vector4f(0.0f, 0.0f, 0.0f, 1.0f));

	pipeline.SetVertexBuffer(vertices.data());
	pipeline.SetIndexBuffer(indices.data());
	pipeline.SetTopologyType(TopologyType::TriangleList);

	int baseOffset = 0;
	for (int i = 0; i < model.renderInfo.size(); i++) {
		shader.texture = &textures[i];
		pipeline.SetShader(shader);
		pipeline.DrawIndexed(baseOffset, 0, model.renderInfo[i].indices.size());
		baseOffset += model.renderInfo[i].indices.size();
	}

	renderer.Present(&pipeline);

	bool quit = false;
	while (!quit) {
		SDL_Event sdlEvent;
		while (SDL_PollEvent(&sdlEvent)) {
			if (sdlEvent.type == SDL_QUIT) {
				quit = true;
			}
		}

		
	}

	return 0;
}