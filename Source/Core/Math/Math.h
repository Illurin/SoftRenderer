#pragma once
#include <cmath>
#include <corecrt_math_defines.h>
#include <cstdint>
#include <algorithm>
#include <vector>

float min(float a, float b);
float max(float a, float b);
float saturate(float a);

long Factorial(size_t n);

float Lerp(float v0, float v1, float percent);
float PerspectiveCorrectInterpolate(float z1, float z2, float z3, float n1, float n2, float n3, float u, float v);

struct Vector2i {
	Vector2i() {}
	Vector2i(int x, int y) : x(x), y(y) {}
	int x{ 0 }, y{ 0 };
};

Vector2i operator+(const Vector2i& v0, const Vector2i& v1);
Vector2i operator-(const Vector2i& v0, const Vector2i& v1);

struct Vector2f {
	Vector2f() {}
	Vector2f(float x, float y) : x(x), y(y) {}
	float x{ 0.0f }, y{ 0.0f };

	float Length()const;
};

Vector2f operator*(const float& a, const Vector2f& v);
Vector2f operator+(const Vector2f& v0, const Vector2f& v1);

Vector2f Lerp(Vector2f v0, Vector2f v1, float percent);
Vector2f PerspectiveCorrectInterpolate(float z1, float z2, Vector2f n1, Vector2f n2, float percent);
Vector2f PerspectiveCorrectInterpolate(float z1, float z2, float z3, Vector2f n1, Vector2f n2, Vector2f n3, float u, float v);

struct Vector3i {
	Vector3i() {}
	Vector3i(int x, int y, int z) : x(x), y(y), z(z) {}
	int x{ 0 }, y{ 0 }, z{ 0 };
};

Vector3i Cross(Vector3i v0, Vector3i v1);

struct Vector3f {
	Vector3f() {}
	Vector3f(float a) : x(a), y(a), z(a) {}
	Vector3f(float x, float y, float z) : x(x), y(y), z(z) {}
	float x{ 0.0f }, y{ 0.0f }, z{ 0.0f };

	float Length()const;
	Vector3f Normalize()const;
};

Vector3f operator-(const Vector3f& v);
Vector3f operator+(const Vector3f& v0, const Vector3f& v1);
Vector3f operator-(const Vector3f& v0, const Vector3f& v1);
Vector3f operator*(const float& a, const Vector3f& v);
Vector3f operator*(const Vector3f& v0, const Vector3f& v1);
Vector3f operator/(const Vector3f& v, const float& a);

Vector3f saturate(const Vector3f& v);
Vector3f Lerp(Vector3f v0, Vector3f v1, float percent);
Vector3f PerspectiveCorrectInterpolate(float z1, float z2, float z3, Vector3f n1, Vector3f n2, Vector3f n3, float u, float v);

float Dot(Vector3f v0, Vector3f v1);
Vector3f Cross(Vector3f v0, Vector3f v1);

Vector3f Reflect(Vector3f v, Vector3f n);
Vector3f Refract(Vector3f v, Vector3f n, float refIndex, float& cosFraction);
float Schlick(float cosine, float refIndex);

struct Vector4f {
	Vector4f() {}
	Vector4f(Vector3f v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}
	Vector4f(float a) : x(a), y(a), z(a), w(a) {}
	Vector4f(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	Vector3f GetVector3f()const { return Vector3f(x, y, z); }

	float x{ 0.0f }, y{ 0.0f }, z{ 0.0f }, w{ 0.0f };

	float Length()const;
	Vector4f Normalize()const;
};

Vector4f operator+(const Vector4f& v0, const Vector4f& v1);
Vector4f operator-(const Vector4f& v0, const Vector4f& v1);
Vector4f operator*(const float& a, const Vector4f& v);
Vector4f operator*(const Vector4f& v0, const Vector4f& v1);

Vector4f saturate(const Vector4f& v);
Vector4f Lerp(Vector4f v0, Vector4f v1, float percent);
Vector4f PerspectiveCorrectInterpolate(float z1, float z2, float z3, Vector4f n1, Vector4f n2, Vector4f n3, float u, float v);

float Dot(Vector4f v0, Vector4f v1);

struct Vertex {
	Vector4f position;
	Vector4f color;
	Vector2f texCoord;
	Vector3f normal;
};

Vertex VertexInterplote(const Vertex& v0, const Vertex& v1, float percent);

struct Matrix3x3f {
	Matrix3x3f() {}
	Matrix3x3f(float value) : a1(value), b2(value), c3(value) {}
	Matrix3x3f(Vector3f v1, Vector3f v2, Vector3f v3) {
		a1 = v1.x, a2 = v1.y, a3 = v1.z;
		b1 = v2.x, b2 = v2.y, b3 = v2.z;
		c1 = v3.x, c2 = v3.y, c3 = v3.z;
	}

	Matrix3x3f Transpose()const;
	float Determinant()const;
	Matrix3x3f Adjoint()const;
	Matrix3x3f Inverse()const;

	Vector3f GetRowVector1()const { return Vector3f(a1, a2, a3); }
	Vector3f GetRowVector2()const { return Vector3f(b1, b2, b3); }
	Vector3f GetRowVector3()const { return Vector3f(c1, c2, c3); }

	Vector3f GetColumnVector1()const { return Vector3f(a1, b1, c1); }
	Vector3f GetColumnVector2()const { return Vector3f(a2, b2, c2); }
	Vector3f GetColumnVector3()const { return Vector3f(a3, b3, c3); }

	float a1{ 0.0f }, a2{ 0.0f }, a3{ 0.0f };
	float b1{ 0.0f }, b2{ 0.0f }, b3{ 0.0f };
	float c1{ 0.0f }, c2{ 0.0f }, c3{ 0.0f };
};

Matrix3x3f operator+(const Matrix3x3f m0, const Matrix3x3f m1);
Matrix3x3f operator*(float a, Matrix3x3f m);

Vector3f Multiply(Vector3f v, Matrix3x3f m);

struct Matrix4x4f {
	Matrix4x4f() {}
	Matrix4x4f(float value) : a1(value), b2(value), c3(value), d4(value) {}
	Matrix4x4f(Vector4f v1, Vector4f v2, Vector4f v3, Vector4f v4) {
		a1 = v1.x, a2 = v1.y, a3 = v1.z, a4 = v1.w;
		b1 = v2.x, b2 = v2.y, b3 = v2.z, b4 = v2.w;
		c1 = v3.x, c2 = v3.y, c3 = v3.z, c4 = v3.w;
		d1 = v4.x, d2 = v4.y, d3 = v4.z, d4 = v4.w;
	}

	Matrix3x3f GetMatrix3x3f()const;
	Matrix4x4f Transpose()const;

	Matrix3x3f Minor(size_t i, size_t j)const;
	float Determinant()const;
	Matrix4x4f Adjoint()const;
	Matrix4x4f Inverse()const;

	Vector4f GetRowVector1()const { return Vector4f(a1, a2, a3, a4); }
	Vector4f GetRowVector2()const { return Vector4f(b1, b2, b3, b4); }
	Vector4f GetRowVector3()const { return Vector4f(c1, c2, c3, c4); }
	Vector4f GetRowVector4()const { return Vector4f(d1, d2, d3, d4); }

	Vector4f GetColumnVector1()const { return Vector4f(a1, b1, c1, d1); }
	Vector4f GetColumnVector2()const { return Vector4f(a2, b2, c2, d2); }
	Vector4f GetColumnVector3()const { return Vector4f(a3, b3, c3, d3); }
	Vector4f GetColumnVector4()const { return Vector4f(a4, b4, c4, d4); }

	float a1{ 0.0f }, a2{ 0.0f }, a3{ 0.0f }, a4{ 0.0f };
	float b1{ 0.0f }, b2{ 0.0f }, b3{ 0.0f }, b4{ 0.0f };
	float c1{ 0.0f }, c2{ 0.0f }, c3{ 0.0f }, c4{ 0.0f };
	float d1{ 0.0f }, d2{ 0.0f }, d3{ 0.0f }, d4{ 0.0f };
};

Matrix4x4f operator+(const Matrix4x4f m0, const Matrix4x4f m1);
Matrix4x4f operator*(float a, Matrix4x4f m);

Matrix4x4f Multiply(Matrix4x4f m0, Matrix4x4f m1);
Vector4f Multiply(Vector4f v, Matrix4x4f m);

Matrix4x4f Scale(float x, float y, float z);

Matrix4x4f RotateX(float angle);
Matrix4x4f RotateY(float angle);
Matrix4x4f RotateZ(float angle);
Matrix4x4f RotateAxis(Vector3f axis, float angle);

Matrix4x4f Translate(float x, float y, float z);

Matrix4x4f OrthograpicProjection(float l, float r, float t, float b, float n, float f);
Matrix4x4f PerspectiveProjection(float l, float r, float t, float b, float n, float f);

Vector3f CalcBarycentric(Vector2f v0, Vector2f v1, Vector2f v2, Vector2f p);