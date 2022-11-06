#include "Camera.h"

void Camera::SetLens(float fovY, float aspect, float nearZ, float farZ) {
	this->fovY = fovY;
	this->aspect = aspect;
	this->nearZ = nearZ;
	this->farZ = farZ;

	float h = tan(fovY / 2.0f) * nearZ;
	float l = h * aspect;

	projMatrix = PerspectiveProjection(-l, l, -h, h, nearZ, farZ);
}

void Camera::UpdataViewMatrix() {
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

void Camera::LookAt(Vector3f pos, Vector3f target, Vector3f worldUp) {
	position = pos;
	look = Vector3f(target.x - pos.x, target.y - pos.y, target.z - pos.z).Normalize();
	right = Cross(worldUp, look).Normalize();
	up = Cross(look, right);

	viewDirty = true;
}

void Camera::Walk(float distance) {
	position.x += look.x * distance;
	position.y += look.y * distance;
	position.z += look.z * distance;
	viewDirty = true;
}

void Camera::Strafe(float distance) {
	position.x -= right.x * distance;
	position.y -= right.y * distance;
	position.z -= right.z * distance;
	viewDirty = true;
}

void Camera::Pitch(float angle) {
	Matrix4x4f m = RotateAxis(right, angle);
	up = Multiply(Vector4f(up, 1.0f), m).GetVector3f();
	look = Multiply(Vector4f(look, 1.0f), m).GetVector3f();

	viewDirty = true;
}

void Camera::RotateY(float angle) {
	Matrix4x4f m(
		Vector4f(cos(angle), 0.0f, -sin(angle), 0.0f),
		Vector4f(0.0f, 1.0f, 0.0f, 0.0f),
		Vector4f(sin(angle), 0.0f, cos(angle), 0.0f),
		Vector4f(0.0f, 0.0f, 0.0f, 1.0f)
	);
	up = Multiply(Vector4f(up, 1.0f), m).GetVector3f();
	right = Multiply(Vector4f(right, 1.0f), m).GetVector3f();
	look = Multiply(Vector4f(look, 1.0f), m).GetVector3f();

	viewDirty = true;
}