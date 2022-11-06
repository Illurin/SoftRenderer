#pragma once
#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/mesh.h"
#include "assimp/texture.h"
#include "Core/Math/Math.h"

struct MaterialInfo {
	int diffuseMaps;
};

bool CompareMaterial(MaterialInfo dest, MaterialInfo source);

class Mesh {
public:
	struct RenderInfo {
		std::vector<Vertex> vertices;
		std::vector<int> indices;
	};

	std::vector<Vertex> vertices;
	std::vector<int> indices;
	int materialIndex;
	
	Mesh(std::vector<Vertex> vertices, std::vector<int> indices, int materialIndex) {
		this->vertices = vertices;
		this->indices = indices;
		this->materialIndex = materialIndex;
	}
};

class Model {
public:
	Model(std::string path);
	
	std::vector<MaterialInfo> materials;
	std::vector<Mesh::RenderInfo> renderInfo;
	std::vector<std::string> texturePath;

private:
	void ProcessNode(const aiScene* scene, aiNode* node);
	Mesh ProcessMesh(const aiScene* scene, aiMesh* mesh);
	int SetupMaterial(std::vector<int> diffuseMaps);
	void SetupRenderInfo();
	std::vector<int> LoadMaterialTextures(aiMaterial* mat, aiTextureType type);

	std::string directory;
	std::vector<Mesh> meshes;
};