#pragma once
#include "Core/Math/Math.h"
#include "Function/Texture/Texture.h"

struct Material {
	Vector4f diffuseAlbedo;
	Vector3f fresnelR0;
	float roughness;
};

enum class LightType {
	Directional = 0,
	Point,
	Spot
};

struct Light {
	LightType type;
	Vector3f ambientLight;
	Vector3f strength;
	Vector3f direction;					   //directional/spot light only
	Vector3f position;					   //point/spot light only
	float spotPower;					   //spot light only
};

class LightingUtil {
protected:
	struct Material {
		Vector4f diffuseAlbedo;
		Vector3f fresnelR0;
		float shininess;
	};

	Vector3f SchlickFresnel(Vector3f R0, Vector3f normal, Vector3f lightVec)const;
	Vector3f BlinnPhong(Vector3f lightStrength, Vector3f lightVec, Vector3f normal, Vector3f toEye, Material mat)const;
	Vector3f ComputeDirectionalLight(Light light, Material mat, Vector3f normal, Vector3f toEye)const;
	Vector3f ComputePointLight(Light light, Material mat, Vector3f pos, Vector3f normal, Vector3f toEye)const;
	Vector3f ComputeSpotLight(Light light, Material mat, Vector3f pos, Vector3f normal, Vector3f toEye)const;
};

class GouraudShader : LightingUtil {
public:
	struct {
		Vector3f eyePos;
		Vector3f ambientLight;
		Matrix4x4f viewMatrix;
		Matrix4x4f projMatrix;
		Light light;
	}passConstant;

	struct {
		Matrix4x4f worldMatrix;
		Matrix4x4f normalMatrix;
	}objectConstant;

	struct {
		Vector4f diffuseAlbedo;
		Vector3f fresnelR0;
		float roughness;
	}materialConstant;

	struct VertexInput {
		Vector4f position;
		Vector2f texCoord;
		Vector3f normal;
	};

	struct FragmentInput {
		Vector3f worldPos;
		Vector2f texCoord;
		Vector3f normal;
	};

	Sampler sampler;
	Texture* texture{ nullptr };
	
	VertexInput InputAssembler(Vertex* vertex)const;
	Vector4f VertexShader(VertexInput input, FragmentInput& output)const;
	Vector4f FragmentShader(FragmentInput input)const;
};