#include "Math.h"

float min(float a, float b) {
	return a < b ? a : b;
}

float max(float a, float b) {
	return a > b ? a : b;
}

float saturate(float a) {
	a = a > 1.0f ? 1.0f : a;
	a = a < 0.0f ? 0.0f : a;
	return a;
}

Vector3f saturate(const Vector3f& v) {
	return Vector3f(saturate(v.x), saturate(v.y), saturate(v.z));
}

Vector4f saturate(const Vector4f& v) {
	return Vector4f(saturate(v.x), saturate(v.y), saturate(v.z), saturate(v.w));
}

long Factorial(size_t n) {
	long result = 1;
	for (size_t i = 1; i <= n; i++) {
		result *= i;
	}
	return result;
}

float Vector2f::Length()const { return sqrtf(x * x + y * y); }
float Vector3f::Length()const { return sqrtf(x * x + y * y + z * z); }
float Vector4f::Length()const { return sqrtf(x * x + y * y + z * z); }

Vector3f Vector3f::Normalize()const {
	float length = Length();
	return Vector3f(x / length, y / length, z / length);
}

Vector4f Vector4f::Normalize()const {
	float length = Length();
	return Vector4f(x / length, y / length, z / length, w);
}

Vector2i operator+(const Vector2i& v0, const Vector2i& v1) {
	return Vector2i(v0.x + v1.x, v0.y + v1.y);
}

Vector2i operator-(const Vector2i& v0, const Vector2i& v1) {
	return Vector2i(v0.x - v1.x, v0.y - v1.y);
}

Vector2f operator*(const float& a, const Vector2f& v) {
	return Vector2f(a * v.x, a * v.y);
}

Vector2f operator+(const Vector2f& v0, const Vector2f& v1) {
	return Vector2f(v0.x + v1.x, v0.y + v1.y);
}

Vector3f operator-(const Vector3f& v) {
	return Vector3f(-v.x, -v.y, -v.z);
}

Vector3f operator+(const Vector3f& v0, const Vector3f& v1) {
	return Vector3f(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z);
}

Vector3f operator-(const Vector3f& v0, const Vector3f& v1) {
	return Vector3f(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);
}

Vector3f operator*(const float& a, const Vector3f& v) {
	return Vector3f(a * v.x, a * v.y, a * v.z);
}

Vector3f operator*(const Vector3f& v0, const Vector3f& v1) {
	return Vector3f(v0.x * v1.x, v0.y * v1.y, v0.z * v1.z);
}

Vector3f operator/(const Vector3f& v, const float& a) {
	return Vector3f(v.x / a, v.y / a, v.z / a);
}

Vector4f operator+(const Vector4f& v0, const Vector4f& v1) {
	return Vector4f(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z, v0.w + v1.w);
}

Vector4f operator-(const Vector4f& v0, const Vector4f& v1) {
	return Vector4f(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z, v0.w - v1.w);
}

Vector4f operator*(const float& a, const Vector4f& v) {
	return Vector4f(a * v.x, a * v.y, a * v.z, a * v.w);
}

Vector4f operator*(const Vector4f& v0, const Vector4f& v1) {
	return Vector4f(v0.x * v1.x, v0.y * v1.y, v0.z * v1.z, v0.w * v1.w);
}

Matrix3x3f operator+(const Matrix3x3f m0, const Matrix3x3f m1) {
	return Matrix3x3f(
		m0.GetRowVector1() + m1.GetRowVector1(),
		m0.GetRowVector2() + m1.GetRowVector2(),
		m0.GetRowVector3() + m1.GetRowVector3()
	);
}

Matrix3x3f operator*(float a, Matrix3x3f m) {
	return Matrix3x3f(
		a * m.GetRowVector1(),
		a * m.GetRowVector2(),
		a * m.GetRowVector3()
	);
}

Matrix4x4f operator+(const Matrix4x4f m0, const Matrix4x4f m1) {
	return Matrix4x4f(
		m0.GetRowVector1() + m1.GetRowVector1(),
		m0.GetRowVector2() + m1.GetRowVector2(),
		m0.GetRowVector3() + m1.GetRowVector3(),
		m0.GetRowVector4() + m1.GetRowVector4()
	);
}

Matrix4x4f operator*(float a, Matrix4x4f m) {
	return Matrix4x4f(
		a * m.GetRowVector1(),
		a * m.GetRowVector2(),
		a * m.GetRowVector3(),
		a * m.GetRowVector4()
	);
}

float Dot(Vector3f v0, Vector3f v1) {
	return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

float Dot(Vector4f v0, Vector4f v1) {
	return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z + v0.w * v1.w;
}

Vector3i Cross(Vector3i v0, Vector3i v1) {
	Vector3i v;
	v.x = v0.y * v1.z - v0.z * v1.y;
	v.y = v0.z * v1.x - v0.x * v1.z;
	v.z = v0.x * v1.y - v0.y * v1.x;
	return v;
}

Vector3f Cross(Vector3f v0, Vector3f v1) {
	Vector3f v;
	v.x = v0.y * v1.z - v0.z * v1.y;
	v.y = v0.z * v1.x - v0.x * v1.z;
	v.z = v0.x * v1.y - v0.y * v1.x;
	return v;
}

float Lerp(float v0, float v1, float percent) {
	return v0 * (1.0f - percent) + v1 * percent;
}

Vector2f Lerp(Vector2f v0, Vector2f v1, float percent) {
	return Vector2f(
		Lerp(v0.x, v1.x, percent),
		Lerp(v0.y, v1.y, percent)
	);
}

Vector3f Lerp(Vector3f v0, Vector3f v1, float percent) {
	return Vector3f(
		Lerp(v0.x, v1.x, percent),
		Lerp(v0.y, v1.y, percent),
		Lerp(v0.z, v1.z, percent)
	);
}

Vector4f Lerp(Vector4f v0, Vector4f v1, float percent) {
	return Vector4f(
		Lerp(v0.x, v1.x, percent),
		Lerp(v0.y, v1.y, percent),
		Lerp(v0.z, v1.z, percent),
		Lerp(v0.w, v1.w, percent)
	);
}

Vector3f Reflect(Vector3f v, Vector3f n) {
	return v + 2 * Dot(-v, n) * n;
}

Vector3f Refract(Vector3f v, Vector3f n, float refIndex, float& cosFraction) {
	float cosIncident = Dot(-v, n);
	cosFraction = 1.0f - refIndex * refIndex * (1.0f - cosIncident * cosIncident);
	Vector3f parallel = (v + cosIncident * n).Normalize();
	return sqrtf(1.0f - cosFraction) * parallel + sqrtf(cosFraction) * (-n);
}

float Schlick(float cosine, float refIndex) {
	float R0 = (1.0f - refIndex) / (1.0f + refIndex);
	R0 *= R0;
	return R0 + (1.0f - R0) * powf((1.0f - cosf(cosine)), 5.0f);
}

Vertex VertexInterplote(const Vertex& v0, const Vertex& v1, float percent) {
	Vertex v;
	v.position = Lerp(v0.position, v1.position, percent);
	v.normal = Lerp(v0.normal, v1.normal, percent);
	v.color = Lerp(v0.color, v1.color, percent);
	v.texCoord = Lerp(v0.texCoord, v1.texCoord, percent);
	return v;
}

float PerspectiveCorrectInterpolate(float z1, float z2, float z3, float n1, float n2, float n3, float u, float v) {
	float zt = 1.0f / ((1.0f - u - v) / z1 + u / z2 + v / z3);
	float nt = ((1.0f - u - v) / z1 * n1 + u / z2 * n2 + v / z3 * n3) * zt;
	return nt;
}

Vector2f PerspectiveCorrectInterpolate(float z1, float z2, Vector2f n1, Vector2f n2, float percent) {
	float zt = 1.0f / ((1.0f - percent) / z1 + percent / z2);
	Vector2f nt;
	nt.x = ((1.0f - percent) / z1 * n1.x + percent / z2 * n2.x) * zt;
	nt.y = ((1.0f - percent) / z1 * n1.y + percent / z2 * n2.y) * zt;
	return nt;
}

Vector2f PerspectiveCorrectInterpolate(float z1, float z2, float z3, Vector2f n1, Vector2f n2, Vector2f n3, float u, float v) {
	float zt = 1.0f / ((1.0f - u - v) / z1 + u / z2 + v / z3);
	Vector2f nt;
	nt.x = ((1.0f - u - v) / z1 * n1.x + u / z2 * n2.x + v / z3 * n3.x) * zt;
	nt.y = ((1.0f - u - v) / z1 * n1.y + u / z2 * n2.y + v / z3 * n3.y) * zt;
	return nt;
}

Vector3f PerspectiveCorrectInterpolate(float z1, float z2, float z3, Vector3f n1, Vector3f n2, Vector3f n3, float u, float v) {
	float zt = 1.0f / ((1.0f - u - v) / z1 + u / z2 + v / z3);
	Vector3f nt;
	nt.x = ((1.0f - u - v) / z1 * n1.x + u / z2 * n2.x + v / z3 * n3.x) * zt;
	nt.y = ((1.0f - u - v) / z1 * n1.y + u / z2 * n2.y + v / z3 * n3.y) * zt;
	nt.z = ((1.0f - u - v) / z1 * n1.z + u / z2 * n2.z + v / z3 * n3.z) * zt;
	return nt;
}

Vector4f PerspectiveCorrectInterpolate(float z1, float z2, float z3, Vector4f n1, Vector4f n2, Vector4f n3, float u, float v) {
	float zt = 1.0f / ((1.0f - u - v) / z1 + u / z2 + v / z3);
	Vector4f nt;
	nt.x = ((1.0f - u - v) / z1 * n1.x + u / z2 * n2.x + v / z3 * n3.x) * zt;
	nt.y = ((1.0f - u - v) / z1 * n1.y + u / z2 * n2.y + v / z3 * n3.y) * zt;
	nt.z = ((1.0f - u - v) / z1 * n1.z + u / z2 * n2.z + v / z3 * n3.z) * zt;
	nt.w = ((1.0f - u - v) / z1 * n1.w + u / z2 * n2.w + v / z3 * n3.w) * zt;
	return nt;
}

Matrix3x3f Matrix4x4f::GetMatrix3x3f()const {
	return Matrix3x3f(
		GetRowVector1().GetVector3f(),
		GetRowVector2().GetVector3f(),
		GetRowVector3().GetVector3f()
	);
}

Vector3f Multiply(Vector3f v, Matrix3x3f m) {
	return Vector3f(
		Dot(v, m.GetColumnVector1()),
		Dot(v, m.GetColumnVector2()),
		Dot(v, m.GetColumnVector3())
	);
}

Matrix4x4f Multiply(Matrix4x4f m0, Matrix4x4f m1) {
	Matrix4x4f matrix;
	matrix.a1 = Dot(m0.GetRowVector1(), m1.GetColumnVector1());
	matrix.a2 = Dot(m0.GetRowVector1(), m1.GetColumnVector2());
	matrix.a3 = Dot(m0.GetRowVector1(), m1.GetColumnVector3());
	matrix.a4 = Dot(m0.GetRowVector1(), m1.GetColumnVector4());
	matrix.b1 = Dot(m0.GetRowVector2(), m1.GetColumnVector1());
	matrix.b2 = Dot(m0.GetRowVector2(), m1.GetColumnVector2());
	matrix.b3 = Dot(m0.GetRowVector2(), m1.GetColumnVector3());
	matrix.b4 = Dot(m0.GetRowVector2(), m1.GetColumnVector4());
	matrix.c1 = Dot(m0.GetRowVector3(), m1.GetColumnVector1());
	matrix.c2 = Dot(m0.GetRowVector3(), m1.GetColumnVector2());
	matrix.c3 = Dot(m0.GetRowVector3(), m1.GetColumnVector3());
	matrix.c4 = Dot(m0.GetRowVector3(), m1.GetColumnVector4());
	matrix.d1 = Dot(m0.GetRowVector4(), m1.GetColumnVector1());
	matrix.d2 = Dot(m0.GetRowVector4(), m1.GetColumnVector2());
	matrix.d3 = Dot(m0.GetRowVector4(), m1.GetColumnVector3());
	matrix.d4 = Dot(m0.GetRowVector4(), m1.GetColumnVector4());
	return matrix;
}

Vector4f Multiply(Vector4f v, Matrix4x4f m) {
	return Vector4f(
		Dot(v, m.GetColumnVector1()),
		Dot(v, m.GetColumnVector2()),
		Dot(v, m.GetColumnVector3()),
		Dot(v, m.GetColumnVector4())
	);
}

float Matrix3x3f::Determinant()const {
	return a1 * (b2 * c3 - b3 * c2) - a2 * (b1 * c3 - b3 * c1) + a3 * (b1 * c2 - b2 * c1);
}

float Matrix4x4f::Determinant()const {
	return a1 * Minor(0, 0).Determinant() - a2 * Minor(0, 1).Determinant() + a3 * Minor(0, 2).Determinant() - a4 * Minor(0, 3).Determinant();
}

Matrix3x3f Matrix3x3f::Transpose()const {
	return Matrix3x3f(
		GetColumnVector1(),
		GetColumnVector2(),
		GetColumnVector3()
	);
}

Matrix4x4f Matrix4x4f::Transpose()const {
	return Matrix4x4f(
		GetColumnVector1(),
		GetColumnVector2(),
		GetColumnVector3(),
		GetColumnVector4()
	);
}

Matrix3x3f Matrix4x4f::Minor(size_t i, size_t j)const {
	std::vector<std::vector<float>> matrix = {
		{ a1, a2, a3, a4 },
		{ b1, b2, b3, b4 },
		{ c1, c2, c3, c4 },
		{ d1, d2, d3, d4 }
	};
	matrix.erase(matrix.begin() + i);
	for (auto& row : matrix) {
		row.erase(row.begin() + j);
	}
	return Matrix3x3f(
		Vector3f(matrix[0][0], matrix[0][1], matrix[0][2]),
		Vector3f(matrix[1][0], matrix[1][1], matrix[1][2]),
		Vector3f(matrix[2][0], matrix[2][1], matrix[2][2])
	);
}

Matrix3x3f Matrix3x3f::Adjoint()const {
	return Matrix3x3f(
		Vector3f(b2 * c3 - b3 * c2, -b1 * c3 + b3 * c1, b1 * c2 - b2 * c1),
		Vector3f(-a2 * c3 + a3 * c2, a1 * c3 - a3 * c1, -a1 * c2 + a2 * c1),
		Vector3f(a2 * b3 - a3 * b2, -a1 * b3 + a3 * b1, a1 * b2 - a2 * b1)
	);
}

Matrix4x4f Matrix4x4f::Adjoint()const {
	return Matrix4x4f(
		Vector4f(Minor(0, 0).Determinant(), -Minor(0, 1).Determinant(), Minor(0, 2).Determinant(), -Minor(0, 3).Determinant()),
		Vector4f(Minor(1, 0).Determinant(), -Minor(1, 1).Determinant(), Minor(1, 2).Determinant(), -Minor(1, 3).Determinant()),
		Vector4f(Minor(2, 0).Determinant(), -Minor(2, 1).Determinant(), Minor(2, 2).Determinant(), -Minor(2, 3).Determinant()),
		Vector4f(Minor(3, 0).Determinant(), -Minor(3, 1).Determinant(), Minor(3, 2).Determinant(), -Minor(3, 3).Determinant())
	);
}

Matrix3x3f Matrix3x3f::Inverse()const {
	return 1.0f / Determinant() * Adjoint();
}

Matrix4x4f Matrix4x4f::Inverse()const {
	return 1.0f / Determinant() * Adjoint();
}

Matrix4x4f Scale(float x, float y, float z) {
	return Matrix4x4f(
		Vector4f(x, 0.0f, 0.0f, 0.0f),
		Vector4f(0.0f, y, 0.0f, 0.0f),
		Vector4f(0.0f, 0.0f, z, 0.0f),
		Vector4f(0.0f, 0.0f, 0.0f, 1.0f)
	);
}

Matrix4x4f RotateX(float angle) {
	return Matrix4x4f(
		Vector4f(1.0f, 0.0f, 0.0f, 0.0f),
		Vector4f(0.0f, cos(angle), sin(angle), 0.0f),
		Vector4f(0.0f, -sin(angle), cos(angle), 0.0f),
		Vector4f(0.0f, 0.0f, 0.0f, 1.0f)
	);
}

Matrix4x4f RotateY(float angle) {
	return Matrix4x4f(
		Vector4f(cos(angle), 0.0f, -sin(angle), 0.0f),
		Vector4f(0.0f, 1.0f, 0.0f, 0.0f),
		Vector4f(sin(angle), 0.0f, cos(angle), 0.0f),
		Vector4f(0.0f, 0.0f, 0.0f, 1.0f)
	);
}

Matrix4x4f RotateZ(float angle) {
	return Matrix4x4f(
		Vector4f(cos(angle), sin(angle), 0.0f, 0.0f),
		Vector4f(-sin(angle), cos(angle), 0.0f, 0.0f),
		Vector4f(0.0f, 0.0f, 1.0f, 0.0f),
		Vector4f(0.0f, 0.0f, 0.0f, 1.0f)
	);
}

Matrix4x4f RotateAxis(Vector3f axis, float angle) {
	float c = cos(angle); float s = sin(angle);
	float x = axis.x; float y = axis.y; float z = axis.z;

	return Matrix4x4f(
		Vector4f(c + (1.0f - c) * x * x, (1.0f - c) * x * y + s * z, (1.0f - c) * x * z - s * y, 0.0f),
		Vector4f((1.0f - c) * x * y - s * z, c + (1.0f - c) * y * y, (1.0f - c) * y * z + s * x, 0.0f),
		Vector4f((1.0f - c) * x * z + s * y, (1.0f - c) * y * z - s * x, c + (1.0f - c) * z * z, 0.0f),
		Vector4f(0.0f, 0.0f, 0.0f, 1.0f)
	);
}

Matrix4x4f Translate(float x, float y, float z) {
	return Matrix4x4f(
		Vector4f(1.0f, 0.0f, 0.0f, 0.0f),
		Vector4f(0.0f, 1.0f, 0.0f, 0.0f),
		Vector4f(0.0f, 0.0f, 1.0f, 0.0f),
		Vector4f(x, y, z, 1.0f)
	);
}

Matrix4x4f OrthograpicProjection(float l, float r, float t, float b, float n, float f) {
	Matrix4x4f translate(
		Vector4f(1.0f, 0.0f, 0.0f, 0.0f),
		Vector4f(0.0f, 1.0f, 0.0f, 0.0f),
		Vector4f(0.0f, 0.0f, 1.0f, 0.0f),
		Vector4f(-(l + r) / 2.0f, -(t + b) / 2.0f, -(n + f) / 2.0f, 1.0f)
	);
	Matrix4x4f scale(
		Vector4f(2.0f / (r - l), 0.0f, 0.0f, 0.0f),
		Vector4f(0.0f, 2.0f / (t - b), 0.0f, 0.0f),
		Vector4f(0.0f, 0.0f, 2.0f / (n - f), 0.0f),
		Vector4f(0.0f, 0.0f, 0.0f, 1.0f)
	);
	return Multiply(translate, scale);
}

Matrix4x4f PerspectiveProjection(float l, float r, float t, float b, float n, float f) {
	Matrix4x4f perspToOrtho(
		Vector4f(n, 0.0f, 0.0f, 0.0f),
		Vector4f(0.0f, n, 0.0f, 0.0f),
		Vector4f(0.0f, 0.0f, n + f, 1.0f),
		Vector4f(0.0f, 0.0f, -n * f, 0.0f)
	);
	return Multiply(perspToOrtho, OrthograpicProjection(l, r, t, b, n, f));
}

Vector3f CalcBarycentric(Vector2f v0, Vector2f v1, Vector2f v2, Vector2f p) {
	float Ux = v1.x - v0.x;
	float Uy = v1.y - v0.y;
	float Vx = v2.x - v0.x;
	float Vy = v2.y - v0.y;
	float POx = v0.x - p.x;
	float POy = v0.y - p.y;

	Vector3f kAB1 = Cross(Vector3f(Ux, Vx, POx), Vector3f(Uy, Vy, POy));

	if (abs(kAB1.z) < 1.0f) {
		return Vector3f(-1.0f, 1.0f, 1.0f);
	}

	float A = kAB1.x / kAB1.z;
	float B = kAB1.y / kAB1.z;

	return Vector3f(1.0f - A - B, A, B);
}