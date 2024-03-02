#include "linearAlg.h"

static const double PI = 3.14159265358979323846;

Matrix multiplymat4(const Matrix* m1, const Matrix* m2) {
	Matrix out = IDENTITY_MATRIX;
	unsigned int row, column, row_offset;

	for (row = 0, row_offset = row * 4; row < 4; ++row, row_offset = row * 4)
		for (column = 0; column < 4; ++column)
			out.m[row_offset + column] =
				(m1->m[row_offset + 0] * m2->m[column + 0]) +
				(m1->m[row_offset + 1] * m2->m[column + 4]) +
				(m1->m[row_offset + 2] * m2->m[column + 8]) +
				(m1->m[row_offset + 3] * m2->m[column + 12]);

	return out;
}

vec4 mulmatvec4(const Matrix* m, const vec4* v) {
	vec4 out;
	for(int i = 0; i < 4; ++i) {
		out.m[i] =
			(v->m[0] * m->m[i + 0]) +
			(v->m[1] * m->m[i + 4]) +
			(v->m[2] * m->m[i + 8]) +
			(v->m[3] * m->m[i + 12]);
	}

	return out;
}

void normalize4(vec4* v) {
	float sqr = v->m[0] * v->m[0] + v->m[1] * v->m[1] + v->m[2] * v->m[2];
	if (sqr == 1 || sqr == 0)
		return;
	float invrt = 1.f / sqrt(sqr);
	v->m[0] *= invrt;
	v->m[1] *= invrt;
	v->m[2] *= invrt;
}

vec4 cross4(vec4 v1, vec4 v2) {
	vec4 out = { {0} };
	out.m[0] = v1.m[1] * v2.m[2] - v1.m[2] * v2.m[1];
	out.m[1] = v1.m[2] * v2.m[0] - v1.m[0] * v2.m[2];
	out.m[2] = v1.m[0] * v2.m[1] - v1.m[1] * v2.m[0];
	return out;
}
void rotateX(const Matrix* m, float angle) {
	Matrix rotation = IDENTITY_MATRIX;
	float sine = (float)sin(angle);
	float cosine = (float)cos(angle);

	rotation.m[5] = cosine;
	rotation.m[6] = -sine;
	rotation.m[9] = sine;
	rotation.m[10] = cosine;

	memcpy(m->m, multiplymat4(m, &rotation).m, sizeof(m->m));
}
void rotateY(const Matrix* m, float angle) {
	Matrix rotation = IDENTITY_MATRIX;
	float sine = (float)sin(angle);
	float cosine = (float)cos(angle);

	rotation.m[0] = cosine;
	rotation.m[8] = sine;
	rotation.m[2] = -sine;
	rotation.m[10] = cosine;

	memcpy(m->m, multiplymat4(m, &rotation).m, sizeof(m->m));
}
void rotateZ(const Matrix* m, float angle) {
	Matrix rotation = IDENTITY_MATRIX;
	float sine = (float)sin(angle);
	float cosine = (float)cos(angle);

	rotation.m[0] = cosine;
	rotation.m[1] = -sine;
	rotation.m[4] = sine;
	rotation.m[5] = cosine;

	memcpy(m->m, multiplymat4(m, &rotation).m, sizeof(m->m));
}
void scale(const Matrix* m, float x, float y, float z) {
	Matrix scale = IDENTITY_MATRIX;

	scale.m[0] = x;
	scale.m[5] = y;
	scale.m[10] = z;

	memcpy(m->m, multiplymat4(m, &scale).m, sizeof(m->m));
}
void translate(const Matrix* m, float x, float y, float z) {
	Matrix translation = IDENTITY_MATRIX;

	translation.m[12] = x;
	translation.m[13] = y;
	translation.m[14] = z;

	memcpy(m->m, multiplymat4(m, &translation).m, sizeof(m->m));
}

Matrix perspective(float fovy, float aspect_ratio, float near_plane, float far_plane) {
	Matrix mat;
    float f = 1.0f / tanf(fovy * 0.5f);
    float rangeInv = 1.0f / (near_plane - far_plane);

    mat.m[0] = f / aspect_ratio;
    mat.m[1] = 0.0f;
    mat.m[2] = 0.0f;
    mat.m[3] = 0.0f;

    mat.m[4] = 0.0f;
    mat.m[5] = f;
    mat.m[6] = 0.0f;
    mat.m[7] = 0.0f;

    mat.m[8] = 0.0f;
    mat.m[9] = 0.0f;
    mat.m[10] = (far_plane + near_plane) * rangeInv;
    mat.m[11] = -1.0f;

    mat.m[12] = 0.0f;
    mat.m[13] = 0.0f;
    mat.m[14] = 2.0f * far_plane * near_plane * rangeInv;
    mat.m[15] = 0.0f;
    return mat;
}

Matrix orthogonal(float left, float right, float bottom, float top) {
	Matrix out = IDENTITY_MATRIX;
	out.m[0] = 2 / (right - left);
	out.m[5] = 2 / (top - bottom);
	out.m[10] = -1;
	out.m[12] = -(right + left) / (right - left);
	out.m[13] = -(top + bottom) / (top - bottom);

	return out;
}

vec3 cross3(const vec3 v1, const vec3 v2) {
	vec3 crossproduct = { 0, 0, 0 };

	crossproduct.x = v1.y * v2.z - v1.z * v2.y;
	crossproduct.y = v1.x * v2.z - v1.z * v2.x;
	crossproduct.z = v1.x * v2.y - v1.y * v2.x;

	return crossproduct;
}

vec3 normalize3(const vec3 vec) {
	float vecLen = sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
	vec3 norm = { 0 };

	if (vecLen != 0.0f) {
		norm.x = vec.x / vecLen;
		norm.y = vec.y / vecLen;
		norm.z = vec.z / vecLen;
	}

	return norm;
}

vec3 subtract(vec3 a, vec3 b) {
    vec3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

Matrix lookAt(vec3 eye, vec3 center, vec3 up) {
  /*  	vec4 f = dir;
	normalize4(&f);
	vec4 u = { {0, 1, 0, 0} };
	vec4 s = cross4(f, u);
	normalize4(&s);
	u = cross4(s, f);

	Matrix out = IDENTITY_MATRIX;
	out.m[0] = s.x;
	out.m[4] = s.y;
	out.m[8] = s.z;

	out.m[1] = u.x;
	out.m[5] = u.y;
	out.m[9] = u.z;

	out.m[2] = -f.x;
	out.m[6] = -f.y;
	out.m[10] = -f.z;

	out.m[12] = -(dotf4(s, pos));
	out.m[13] = -(dotf4(u, pos));
	out.m[14] =  dotf4(f, pos);
	return out;*/

  vec3 f = normalize3(subtract(center, eye));
  vec3 s = normalize3(cross3(f, up));
  vec3 u = cross3(s, f);

  Matrix mat;
  mat.m[0] = s.x;
  mat.m[1] = u.x;
  mat.m[2] = -f.x;
  mat.m[3] = 0.0f;

  mat.m[4] = s.y;
  mat.m[5] = u.y;
  mat.m[6] = -f.y;
  mat.m[7] = 0.0f;

  mat.m[8] = s.z;
  mat.m[9] = u.z;
  mat.m[10] = -f.z;
  mat.m[11] = 0.0f;

  mat.m[12] = -dotf3(s, eye);
  mat.m[13] = -dotf3(u, eye);
  mat.m[14] = dotf3(f, eye);
  mat.m[15] = 1.0f;
  
  return mat;  
}

bool gluInvertMatrix(const float m[16], float invOut[16]){
  float inv[16], det;
  int i;

  inv[0] = m[5]  * m[10] * m[15] - 
    m[5]  * m[11] * m[14] - 
    m[9]  * m[6]  * m[15] + 
    m[9]  * m[7]  * m[14] +
    m[13] * m[6]  * m[11] - 
    m[13] * m[7]  * m[10];

  inv[4] = -m[4]  * m[10] * m[15] + 
    m[4]  * m[11] * m[14] + 
    m[8]  * m[6]  * m[15] - 
    m[8]  * m[7]  * m[14] - 
    m[12] * m[6]  * m[11] + 
    m[12] * m[7]  * m[10];

  inv[8] = m[4]  * m[9] * m[15] - 
    m[4]  * m[11] * m[13] - 
    m[8]  * m[5] * m[15] + 
    m[8]  * m[7] * m[13] + 
    m[12] * m[5] * m[11] - 
    m[12] * m[7] * m[9];

  inv[12] = -m[4]  * m[9] * m[14] + 
    m[4]  * m[10] * m[13] +
    m[8]  * m[5] * m[14] - 
    m[8]  * m[6] * m[13] - 
    m[12] * m[5] * m[10] + 
    m[12] * m[6] * m[9];

  inv[1] = -m[1]  * m[10] * m[15] + 
    m[1]  * m[11] * m[14] + 
    m[9]  * m[2] * m[15] - 
    m[9]  * m[3] * m[14] - 
    m[13] * m[2] * m[11] + 
    m[13] * m[3] * m[10];

  inv[5] = m[0]  * m[10] * m[15] - 
    m[0]  * m[11] * m[14] - 
    m[8]  * m[2] * m[15] + 
    m[8]  * m[3] * m[14] + 
    m[12] * m[2] * m[11] - 
    m[12] * m[3] * m[10];

  inv[9] = -m[0]  * m[9] * m[15] + 
    m[0]  * m[11] * m[13] + 
    m[8]  * m[1] * m[15] - 
    m[8]  * m[3] * m[13] - 
    m[12] * m[1] * m[11] + 
    m[12] * m[3] * m[9];

  inv[13] = m[0]  * m[9] * m[14] - 
    m[0]  * m[10] * m[13] - 
    m[8]  * m[1] * m[14] + 
    m[8]  * m[2] * m[13] + 
    m[12] * m[1] * m[10] - 
    m[12] * m[2] * m[9];

  inv[2] = m[1]  * m[6] * m[15] - 
    m[1]  * m[7] * m[14] - 
    m[5]  * m[2] * m[15] + 
    m[5]  * m[3] * m[14] + 
    m[13] * m[2] * m[7] - 
    m[13] * m[3] * m[6];

  inv[6] = -m[0]  * m[6] * m[15] + 
    m[0]  * m[7] * m[14] + 
    m[4]  * m[2] * m[15] - 
    m[4]  * m[3] * m[14] - 
    m[12] * m[2] * m[7] + 
    m[12] * m[3] * m[6];

  inv[10] = m[0]  * m[5] * m[15] - 
    m[0]  * m[7] * m[13] - 
    m[4]  * m[1] * m[15] + 
    m[4]  * m[3] * m[13] + 
    m[12] * m[1] * m[7] - 
    m[12] * m[3] * m[5];

  inv[14] = -m[0]  * m[5] * m[14] + 
    m[0]  * m[6] * m[13] + 
    m[4]  * m[1] * m[14] - 
    m[4]  * m[2] * m[13] - 
    m[12] * m[1] * m[6] + 
    m[12] * m[2] * m[5];

  inv[3] = -m[1] * m[6] * m[11] + 
    m[1] * m[7] * m[10] + 
    m[5] * m[2] * m[11] - 
    m[5] * m[3] * m[10] - 
    m[9] * m[2] * m[7] + 
    m[9] * m[3] * m[6];

  inv[7] = m[0] * m[6] * m[11] - 
    m[0] * m[7] * m[10] - 
    m[4] * m[2] * m[11] + 
    m[4] * m[3] * m[10] + 
    m[8] * m[2] * m[7] - 
    m[8] * m[3] * m[6];

  inv[11] = -m[0] * m[5] * m[11] + 
    m[0] * m[7] * m[9] + 
    m[4] * m[1] * m[11] - 
    m[4] * m[3] * m[9] - 
    m[8] * m[1] * m[7] + 
    m[8] * m[3] * m[5];

  inv[15] = m[0] * m[5] * m[10] - 
    m[0] * m[6] * m[9] - 
    m[4] * m[1] * m[10] + 
    m[4] * m[2] * m[9] + 
    m[8] * m[1] * m[6] - 
    m[8] * m[2] * m[5];

  det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

  if (det == 0)
    return false;

  det = 1.0 / det;

  for (i = 0; i < 16; i++)
    invOut[i] = inv[i] * det;

  return true;
}
