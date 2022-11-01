#include <SDL/SDL.h>
#include "Render.h"
#include "Model.h"
#include "Tessellation.h"
#include <Windows.h>

const int width = 600;
const int height = 400;

int main(int argc, char* argv[]) {
	SDL_Window* window;
	SDL_Renderer* renderer;

	window = SDL_CreateWindow("Soft Renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);

	Model model("Assets\\Model.fbx");

	std::vector<Texture> textures(model.texturePath.size());
	for (int i = 0; i < textures.size(); i++) {
		textures[i] = Texture(model.texturePath[i].c_str(), 32);
		textures[i].SetSampler(Texture::AddressMode::BORDER, Vector4f(0.0f, 0.0f, 0.0f, 1.0f));
	}

	std::vector<Vertex> vertices;
	std::vector<int> indices;
	for (auto& r : model.renderInfo) {
		for (auto& i : r.indices)
			indices.push_back(i + vertices.size());
		vertices.insert(vertices.end(), r.vertices.begin(), r.vertices.end());
	}

	Camera camera((float)width / (float)height);
	camera.SetLens(0.25f * M_PI, (float)width / height, 0.1f, 1000.0f);
	camera.LookAt(Vector3f(0.0f, 0.0f, 10.0f), Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f));
	camera.Walk(-15.0f);
	camera.UpdataViewMatrix();

	Renderer pipeline(renderer, width, height, 1);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);

	pipeline.Clear(Vector4f(0.0f, 0.0f, 0.0f, 0.0f));

	Light light;
	light.strength = Vector3f(0.0f, 0.0f, 2.0f);
	light.direction = Vector3f(1.0f, 1.0f, 0.0f).Normalize();

	Material mat;
	mat.diffuseAlbedo = Vector3f(1.0f);
	mat.fresnelR0 = Vector3f(0.5f, 0.5f, 0.5f);
	mat.shininess = 0.9f;

	pipeline.SetEyePosition(camera.GetPosition3f());
	pipeline.SetMaterial(mat);
	pipeline.SetLight(Vector3f(0.3f), light);
	pipeline.SetVertexBuffer(&vertices[0], vertices.size());
	pipeline.SetWorldMatrix(Multiply(Multiply(Scale(5.0f, 5.0f, 5.0f), RotateY(-M_PI * 0.2f)), Translate(0.0f, 0.0f, 0.0f)));
	//pipeline.SetWorldMatrix(Multiply(Multiply(Scale(1.0f, 1.0f, 1.0f), RotateX(-M_PI * 0.5f)), Translate(0.0f, -10.0f, 0.0f)));
	pipeline.SetViewMatrix(camera.GetViewMatrix4x4());
	pipeline.SetProjectionMatrix(camera.GetProjMatrix4x4());
	/*int baseOffset = 0;
	for (int i = 0; i < model.renderInfo.size(); i++) {
		for (int j = 0; j <= model.renderInfo[i].indices.size() - 3; j += 3) {
			pipeline.DrawTriangle(&indices[j + baseOffset], &textures[i]);
		}
		baseOffset += model.renderInfo[i].indices.size();
	}*/

	Vertex originVert[8];
	originVert[0].position = Vector4f(-1.0f, 1.0f, 1.0f, 1.0f);
	originVert[1].position = Vector4f(-1.0f, -1.0f, 1.0f, 1.0f);
	originVert[2].position = Vector4f(1.0f, -1.0f, 1.0f, 1.0f);
	originVert[3].position = Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
	originVert[4].position = Vector4f(1.0f, -1.0f, -1.0f, 1.0f);
	originVert[5].position = Vector4f(1.0f, 1.0f, -1.0f, 1.0f);
	originVert[6].position = Vector4f(-1.0f, -1.0f, -1.0f, 1.0f);
	originVert[7].position = Vector4f(-1.0f, 1.0f, -1.0f, 1.0f);
	originVert[0].color = Vector4f(0.0f, 1.0f, 0.8f, 1.0f);
	originVert[1].color = Vector4f(0.0f, 1.0f, 0.8f, 1.0f);
	originVert[2].color = Vector4f(0.0f, 1.0f, 0.8f, 1.0f);
	originVert[3].color = Vector4f(0.0f, 1.0f, 0.8f, 1.0f);
	originVert[4].color = Vector4f(0.0f, 0.0f, 0.8f, 1.0f);
	originVert[5].color = Vector4f(0.0f, 0.0f, 0.8f, 1.0f);
	originVert[6].color = Vector4f(0.0f, 0.0f, 0.8f, 1.0f);
	originVert[7].color = Vector4f(0.0f, 0.0f, 0.8f, 1.0f);

	size_t inputFaces[6][4] = {
		{0, 1, 2, 3},
		{3, 2, 4, 5},
		{5, 4, 6, 7},
		{7, 0, 3, 5},
		{7, 6, 1, 0},
		{1, 6, 4, 2}
	};

	auto originMesh = std::make_unique<HalfEdgeStructure>();
	std::vector<HalfEdgeStructure::Vertex*> vertPtr;
	for (auto& v : originVert) {
		//v.color = Vector4f(0.0f, 0.8f, 0.8, 1.0f);
		vertPtr.push_back(originMesh->AddVertex(v));
	}

	for (size_t i = 0; i < 6; i++) {
		HalfEdgeStructure::Vertex* ptr[] = { vertPtr[inputFaces[i][0]],vertPtr[inputFaces[i][1]], vertPtr[inputFaces[i][2]], vertPtr[inputFaces[i][3]] };
		originMesh->AddFace(ptr, 4);
	}
	/*for (size_t i = 0; i < indices.size(); i += 3) {
		HalfEdgeStructure::Vertex* ptr[] = { vertPtr[indices[i]], vertPtr[indices[i + 1]], vertPtr[indices[i + 2]] };
		originMesh->AddFace(ptr, 3);
	}*/

	/*originMesh = CatmullClarkSubdivision(originMesh.get());
	vertPtr = originMesh->GetVertices();
	EdgeCollapsing(originMesh.get(), 1);
	for (auto& face : originMesh->GetFaces()) {
		auto outputVertices = originMesh->GetVerticesFromFace(face);
		std::vector<Vertex> drawData;
		for (auto& v : outputVertices)
			drawData.push_back(v->data);
		pipeline.SetVertexBuffer(drawData.data(), drawData.size());
		for (size_t i = 0; i < drawData.size(); i++) {
			int drawIndex[] = { i % drawData.size(), (i + 1) % drawData.size() };
			pipeline.DrawLine(drawIndex);
		}
	}*/

	pipeline.SetWorldMatrix(Matrix4x4f(1.0f));
	pipeline.SetViewMatrix(Matrix4x4f(1.0f));
	pipeline.SetProjectionMatrix(Matrix4x4f(1.0f));
	
	std::vector<Vertex> controlPoints(6);
	controlPoints[0].position = Vector4f(-0.5f, -0.5f, 0.0f, 1.0f);
	controlPoints[1].position = Vector4f(-0.8f, 0.0f, 0.0f, 1.0f);
	controlPoints[2].position = Vector4f(-0.3f, 0.5f, 0.0f, 1.0f);
	controlPoints[3].position = Vector4f(0.3f, 0.5f, 0.0f, 1.0f);
	controlPoints[4].position = Vector4f(0.8f, 0.0f, 0.0f, 1.0f);
	controlPoints[5].position = Vector4f(0.5f, -0.5f, 0.0f, 1.0f);

	controlPoints[0].color = Vector4f(1.0f, 0.0f, 0.0f, 1.0f);
	controlPoints[1].color = Vector4f(1.0f, 0.0f, 0.0f, 1.0f);
	controlPoints[2].color = Vector4f(1.0f, 0.0f, 0.0f, 1.0f);
	controlPoints[3].color = Vector4f(1.0f, 0.0f, 0.0f, 1.0f);
	controlPoints[4].color = Vector4f(1.0f, 0.0f, 0.0f, 1.0f);
	controlPoints[5].color = Vector4f(1.0f, 0.0f, 0.0f, 1.0f);

	std::vector<float> knotVector = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f / 3.0f, 2.0f / 3.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	BSpline spline(controlPoints, knotVector);

	for (float t = 0.0f; t <= 1.0f; t += 0.001f) {
		Vertex point = spline.GetCurvePoint(t);
		pipeline.SetVertexBuffer(&point, 1);
		pipeline.DrawPoint(0);
	}

	pipeline.Present();
	SDL_RenderPresent(renderer);

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