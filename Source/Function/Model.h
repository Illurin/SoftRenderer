#pragma once
#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/mesh.h"
#include "assimp/texture.h"
#include "Render.h"
#include <vector>

struct MaterialInfo {
	int diffuseMaps;
};

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
	std::vector<Mesh> meshes;
	std::vector<MaterialInfo> materials;
	std::vector<Mesh::RenderInfo> renderInfo;
	std::vector<std::string> texturePath;

private:
	std::string directory;

	void ProcessNode(const aiScene* scene, aiNode* node);
	Mesh ProcessMesh(const aiScene* scene, aiMesh* mesh);
	int SetupMaterial(std::vector<int> diffuseMaps);
	void SetupRenderInfo();
	std::vector<int> LoadMaterialTextures(aiMaterial* mat, aiTextureType type);
};

bool CompareMaterial(MaterialInfo dest, MaterialInfo source);

Model::Model(std::string path) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_FlipWindingOrder);
	directory = path.substr(0, path.find_last_of('\\')) + '\\';
	ProcessNode(scene, scene->mRootNode);
	SetupRenderInfo();
}

void Model::ProcessNode(const aiScene* scene, aiNode* node) {
	for (size_t i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(ProcessMesh(scene, mesh));
	}

	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(scene, node->mChildren[i]);
	}
}

Mesh Model::ProcessMesh(const aiScene* scene, aiMesh* mesh) {
	std::vector<Vertex> vertices;
	std::vector<int> indices;

	for (size_t i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;
		vertex.position.w = 1.0f;
		
		vertex.color = Vector4f(1.0f, 1.0f, 1.0f, 1.0f);

		vertex.normal.x = mesh->mNormals[i].x;
		vertex.normal.y = mesh->mNormals[i].y;
		vertex.normal.z = mesh->mNormals[i].z;

		/*vertex.tangent.x = mesh->mTangents[i].x;
		vertex.tangent.y = mesh->mTangents[i].y;
		vertex.tangent.z = mesh->mTangents[i].z;*/

		if (mesh->mTextureCoords[0]) {
			vertex.texCoord.x = mesh->mTextureCoords[0][i].x;
			vertex.texCoord.y = mesh->mTextureCoords[0][i].y;
		}
		else {
			vertex.texCoord = { 0.0f, 0.0f };
		}

		vertices.push_back(vertex);
	}

	for (size_t i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	std::vector<int> diffuseMaps;
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE);
	}
	int materialIndex = SetupMaterial(diffuseMaps);

	return Mesh(vertices, indices, materialIndex);
}

std::vector<int> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type) {
	std::vector<int> textures;
	for (int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, 0, &str);
		bool skip = false;
		std::string texturePath = directory + str.C_Str();
		for (int j = 0; j < this->texturePath.size(); j++)
		{
			if (this->texturePath[j] == texturePath) {
				textures.push_back(j);
				skip = true;
				break;
			}
		}
		if (!skip) {
			textures.push_back(this->texturePath.size());
			this->texturePath.push_back(texturePath);
		}
	}
	return textures;
}

int Model::SetupMaterial(std::vector<int> diffuseMaps) {
	MaterialInfo material;
	if (diffuseMaps.size() > 0)
		material.diffuseMaps = diffuseMaps[0];
	else
		material.diffuseMaps = 0;

	int materialIndex;
	bool skip = false;
	for (int i = 0; i < materials.size(); i++) {
		if (CompareMaterial(materials[i], material)) {
			materialIndex = i;
			skip = true;
			break;
		}
	}
	if (!skip) {
		materialIndex = materials.size();
		materials.push_back(material);
	}
	return materialIndex;
}

void Model::SetupRenderInfo() {
	renderInfo.resize(materials.size());

	for (int i = 0; i < meshes.size(); i++)
	{
		int index = meshes[i].materialIndex;
		int indexOffset = renderInfo[index].vertices.size();

		for (int j = 0; j < meshes[i].indices.size(); j++)
			renderInfo[index].indices.push_back(meshes[i].indices[j] + indexOffset);

		renderInfo[index].vertices.insert(renderInfo[index].vertices.end(), meshes[i].vertices.begin(), meshes[i].vertices.end());
	}
}

bool CompareMaterial(MaterialInfo dest, MaterialInfo source) {
	bool isSame = false;
	if (dest.diffuseMaps == source.diffuseMaps)
		isSame = true;
	else
		isSame = false;
	return isSame;
}