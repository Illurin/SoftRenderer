#pragma once
#include "Core/Math/Math.h"

class Camera {
public:
	Camera() { SetLens(0.25f * M_PI, 2.0f, 0.1f, 1000.0f); }
	Camera(float aspect) { SetLens(0.25f * M_PI, aspect, 0.1f, 1000.0f); }

	Vector3f GetPosition3f()const { return position; }
	Matrix4x4f GetViewMatrix4x4f()const { return viewMatrix; }
	Matrix4x4f GetProjMatrix4x4f()const { return projMatrix; }

	void SetLens(float fovY, float aspect, float nearZ, float farZ);

	void UpdataViewMatrix();

	void LookAt(Vector3f pos, Vector3f target, Vector3f worldUp);
	void Walk(float distance);
	void Strafe(float distance);
	void Pitch(float angle);
	void RotateY(float angle);

private:
	Vector3f position{ 0.0f, 0.0f, 0.0f };
	Vector3f right{ 1.0f, 0.0f, 0.0f };
	Vector3f up{ 0.0f, 1.0f, 0.0f };
	Vector3f look{ 0.0f, 0.0f, -1.0f };

	Matrix4x4f viewMatrix{ Matrix4x4f(1.0f) };
	Matrix4x4f projMatrix{ Matrix4x4f(1.0f) };

	float nearZ{ 0.0f };
	float farZ{ 0.0f };
	float aspect{ 0.0f };
	float fovY{ 0.0f };

	bool viewDirty{ false };
};