#pragma once
#include "Math.h"

class Camera {
public:
	Camera() {
		SetLens(0.25f * M_PI, 2.0f, 0.1f, 1000.0f);
	}
	Camera(float aspect) {
		SetLens(0.25f * M_PI, aspect, 0.1f, 1000.0f);
	}

	Vector3f GetPosition3f()const { return position; }
	Matrix4x4f GetViewMatrix4x4()const {
		return viewMatrix;
	}
	Matrix4x4f GetProjMatrix4x4()const {
		return projMatrix;
	}

	void SetLens(float fovY, float aspect, float nearZ, float farZ) {
		this->fovY = fovY;
		this->aspect = aspect;
		this->nearZ = nearZ;
		this->farZ = farZ;

		float h = tan(fovY / 2.0f) * nearZ;
		float l = h * aspect;

		projMatrix = PerspectiveProjection(-l, l, -h, h, nearZ, farZ);
	}

	void LookAt(Vector3f pos, Vector3f target, Vector3f worldUp) {
		position = pos;
		look = Vector3f(target.x - pos.x, target.y - pos.y, target.z - pos.z).Normalize();
		right = Cross(worldUp, look).Normalize();
		up = Cross(look, right);

		viewDirty = true;
	}
	void UpdataViewMatrix() {
		if (viewDirty) {
			look = look.Normalize();
			right = Cross(up, look).Normalize();
			up = Cross(look, right);

			viewMatrix = Matrix4x4f(
				Vector4f(right.x, up.x, -look.x, 0.0f),
				Vector4f(right.y, up.y, -look.y, 0.0f),
				Vector4f(right.z, up.z, -look.z, 0.0f),
				Vector4f(-Dot(position, right), -Dot(position, up), -Dot(position, Vector3f(-look.x, -look.y, -look.z)), 1.0f)
			);

			viewDirty = false;
		}
	}

	void Walk(float distance) {
		position.x += look.x * distance;
		position.y += look.y * distance;
		position.z += look.z * distance;
		viewDirty = true;
	}

	void Strafe(float distance) {
		position.x -= right.x * distance;
		position.y -= right.y * distance;
		position.z -= right.z * distance;
		viewDirty = true;
	}

	void Pitch(float angle) {
		Matrix4x4f m = RotateAxis(right, angle);
		up = Multiply(Vector4f(up, 1.0f), m).GetVector3f();
		look = Multiply(Vector4f(look, 1.0f), m).GetVector3f();

		viewDirty = true;
	}

	void RotateY(float angle) {
		Matrix4x4f m(
			Vector4f(cos(angle), 0.0f, -sin(angle), 0.0f),
			Vector4f(0.0f ,1.0f, 0.0f, 0.0f),
			Vector4f(sin(angle), 0.0f, cos(angle), 0.0f),
			Vector4f(0.0f, 0.0f, 0.0f, 1.0f)
		);
		up = Multiply(Vector4f(up, 1.0f), m).GetVector3f();
		right = Multiply(Vector4f(right, 1.0f), m).GetVector3f();
		look = Multiply(Vector4f(look, 1.0f), m).GetVector3f();

		viewDirty = true;
	}

private:
	Vector3f position = { 0.0f, 0.0f, 0.0f };
	Vector3f right = { 1.0f, 0.0f, 0.0f };
	Vector3f up = { 0.0f, 1.0f, 0.0f };
	Vector3f look = { 0.0f, 0.0f, -1.0f };

	Matrix4x4f viewMatrix;
	Matrix4x4f projMatrix;

	float nearZ = 0.0f;
	float farZ = 0.0f;
	float aspect = 0.0f;
	float fovY = 0.0f;

	bool viewDirty = true;
};