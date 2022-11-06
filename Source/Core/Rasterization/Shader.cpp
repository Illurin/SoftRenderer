#include "Shader.h"

Vector3f LightingUtil::SchlickFresnel(Vector3f R0, Vector3f normal, Vector3f lightVec)const {
	float cosIncidentAngle = saturate(Dot(normal, lightVec));
	float f0 = 1.0f - cosIncidentAngle;
	Vector3f reflectPercent = R0 + powf(f0, 5) * (Vector3f(1.0f) - R0);
	return reflectPercent;
}

Vector3f LightingUtil::BlinnPhong(Vector3f lightStrength, Vector3f lightVec, Vector3f normal, Vector3f toEye, Material mat)const {
	//计算微平面
	const float shininess = mat.shininess * 256.0f;
	Vector3f halfVec = (toEye + lightVec).Normalize();
	float roughnessFactor = (shininess + 8.0f) * powf(max(Dot(halfVec, normal), 0.0f), shininess) / 8.0f;

	//计算菲涅耳方程
	Vector3f reflectPercent = SchlickFresnel(mat.fresnelR0, normal, lightVec);

	//计算镜面反射值
	Vector3f specularAlbedo = roughnessFactor * reflectPercent;

	//(漫反射 + 镜面反射) * 光强
	return (mat.diffuseAlbedo.GetVector3f() + specularAlbedo) * lightStrength;
}

Vector3f LightingUtil::ComputeDirectionalLight(Light light, Material mat, Vector3f normal, Vector3f toEye)const {
	Vector3f lightVec = -light.direction;
	float lambertFactor = max(Dot(normal, lightVec), 0.0f);
	Vector3f lightStrength = light.strength * lambertFactor;
	return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

Vector3f LightingUtil::ComputePointLight(Light light, Material mat, Vector3f pos, Vector3f normal, Vector3f toEye)const {
	Vector3f lightVec = light.position - pos;

	float lambertFactor = max(Dot(normal, light.direction), 0.0f);
	Vector3f lightStrength = light.strength * lambertFactor;

	float distance = lightVec.Length();
	lightStrength = lightStrength / (distance * distance);

	return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

Vector3f LightingUtil::ComputeSpotLight(Light light, Material mat, Vector3f pos, Vector3f normal, Vector3f toEye)const {
	Vector3f lightVec = light.position - pos;

	float lambertFactor = max(Dot(normal, light.direction), 0.0f);
	Vector3f lightStrength = light.strength * lambertFactor;

	float distance = lightVec.Length();
	lightStrength = lightStrength / (distance * distance);

	float spotFactor = powf(max(Dot(-lightVec, light.direction), 0.0f), light.spotPower);
	lightStrength = spotFactor * lightStrength;

	return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

GouraudShader::VertexInput GouraudShader::InputAssembler(Vertex* vertex)const {
	VertexInput input;
	input.position = vertex->position;
	input.texCoord = vertex->texCoord;
	input.normal = vertex->normal;
	return input;
}

Vector4f GouraudShader::VertexShader(VertexInput input, FragmentInput& output)const {
	Matrix4x4f viewProjMatrix = Multiply(passConstant.viewMatrix, passConstant.projMatrix);
	output.worldPos = Multiply(input.position, objectConstant.worldMatrix).GetVector3f();
	output.normal = Multiply(Vector4f(input.normal, 0.0f), objectConstant.normalMatrix).GetVector3f();
	output.texCoord = input.texCoord;
	return Multiply(Vector4f(output.worldPos, 1.0f), viewProjMatrix);
}

Vector4f GouraudShader::FragmentShader(FragmentInput input)const {
	Vector4f diffuse = materialConstant.diffuseAlbedo * texture->Sample(sampler, input.texCoord);

	input.normal = input.normal.Normalize();
	Vector3f toEye = (passConstant.eyePos - input.worldPos).Normalize();

	float shininess = 1.0f - materialConstant.roughness;
	Material mat = { materialConstant.diffuseAlbedo, materialConstant.fresnelR0, shininess };

	Vector3f lightingResult(0.0f);
	switch (passConstant.light.type) {
	case LightType::Directional:
		lightingResult = ComputeDirectionalLight(passConstant.light, mat, input.normal, toEye);
		break;
	case LightType::Point:
		lightingResult = ComputePointLight(passConstant.light, mat, input.worldPos, input.normal, toEye);
		break;
	case LightType::Spot:
		lightingResult = ComputeSpotLight(passConstant.light, mat, input.worldPos, input.normal, toEye);
		break;
	}

	Vector4f litColor = Vector4f((passConstant.ambientLight + lightingResult), 1.0f) * diffuse;
	litColor.w = materialConstant.diffuseAlbedo.w;
	return saturate(litColor);
}