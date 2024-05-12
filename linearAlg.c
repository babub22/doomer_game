#include "deps.h"
#include "linearAlg.h"

static const double PI = 3.14159265358979323846;

Matrix multiplymat4(Matrix m1, Matrix m2) {
	Matrix out = IDENTITY_MATRIX;
	unsigned int row, column, row_offset;

	for (row = 0, row_offset = row * 4; row < 4; ++row, row_offset = row * 4)
		for (column = 0; column < 4; ++column)
			out.m[row_offset + column] =
				(m1.m[row_offset + 0] * m2.m[column + 0]) +
				(m1.m[row_offset + 1] * m2.m[column + 4]) +
				(m1.m[row_offset + 2] * m2.m[column + 8]) +
				(m1.m[row_offset + 3] * m2.m[column + 12]);

	return out;
}

vec4 mulmatvec4(Matrix m, vec4 v) {
  vec4 out;

  int i=0;
  
  out.x =
    (v.x * m.m[i + 0]) +
    (v.y * m.m[i + 4]) +
    (v.z * m.m[i + 8]) +
    (v.w * m.m[i + 12]);

  i++;
  
  out.y =
    (v.x * m.m[i + 0]) +
    (v.y * m.m[i + 4]) +
    (v.z * m.m[i + 8]) +
    (v.w * m.m[i + 12]);

  i++;
  
  out.z =
    (v.x * m.m[i + 0]) +
    (v.y * m.m[i + 4]) +
    (v.z * m.m[i + 8]) +
    (v.w * m.m[i + 12]);

  i++;
  
  out.w =
    (v.x * m.m[i + 0]) +
    (v.y * m.m[i + 4]) +
    (v.z * m.m[i + 8]) +
    (v.w * m.m[i + 12]);


  return out;
  /*	
  for(int i = 0; i < 4; ++i) {
    out.m[i] =
      (v.x * m.m[i + 0]) +
			(v.y * m.m[i + 4]) +
			(v.z * m.m[i + 8]) +
			(v.w * m.m[i + 12]);
	}
  */
}

void rotateX(const Matrix* m, float angle) {
	Matrix rotation = IDENTITY_MATRIX;
	float sine = (float)sin(angle);
	float cosine = (float)cos(angle);

	rotation.m[5] = cosine;
	rotation.m[6] = -sine;
	rotation.m[9] = sine;
	rotation.m[10] = cosine;
     
	memcpy(m->m, multiplymat4(*m, rotation).m, sizeof(m->m));
}
void rotateY(const Matrix* m, float angle) {
	Matrix rotation = IDENTITY_MATRIX;
	float sine = sinf(angle);
	float cosine = cosf(angle);

	rotation.m[0] = cosine;
	rotation.m[8] = sine;
	rotation.m[2] = -sine;
	rotation.m[10] = cosine;

	memcpy(m->m, multiplymat4(*m, rotation).m, sizeof(m->m));
}
void rotateZ(const Matrix* m, float angle) {
	Matrix rotation = IDENTITY_MATRIX;
	float sine = sinf(angle);
	float cosine = cosf(angle);

	rotation.m[0] = cosine;
	rotation.m[1] = -sine;
	rotation.m[4] = sine;
	rotation.m[5] = cosine;

	memcpy(m->m, multiplymat4(*m, rotation).m, sizeof(m->m));
}
void scale(const Matrix* m, float x, float y, float z) {
	Matrix scale = IDENTITY_MATRIX;

	scale.m[0] = x;
	scale.m[5] = y;
	scale.m[10] = z;

	memcpy(m->m, multiplymat4(*m, scale).m, sizeof(m->m));
}
void translate(const Matrix* m, float x, float y, float z) {
	Matrix translation = IDENTITY_MATRIX;

	translation.m[12] = x;
	translation.m[13] = y;
	translation.m[14] = z;

	memcpy(m->m, multiplymat4(*m, translation).m, sizeof(m->m));
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

Matrix orthogonal(float l, float r, float b, float t, float n, float f)
{
  Matrix M = IDENTITY_MATRIX;
  
    M.m[0] = 2.f / (r - l);
    M.m[1] = 0.f;
    M.m[2] = 0.f;
    M.m[3] = 0.f;

    M.m[4] = 0.f;
    M.m[5] = 2.f / (t - b);
    M.m[6] = 0.f;
    M.m[7] = 0.f;

    M.m[8] = 0.f;
    M.m[9] = 0.f;
    M.m[10] = -2.f / (f - n);
    M.m[11] = 0.f;

    M.m[12] = -(r + l) / (r - l);
    M.m[13] = -(t + b) / (t - b);
    M.m[14] = -(f + n) / (f - n);
    M.m[15] = 1.f;

    return M;
}

vec3 cross3(const vec3 v1, const vec3 v2) {
	vec3 crossproduct = { 0, 0, 0 };

	crossproduct.x = v1.y * v2.z - v1.z * v2.y;
	crossproduct.y = v1.x * v2.z - v1.z * v2.x;
	crossproduct.z = v1.x * v2.y - v1.y * v2.x;

	return crossproduct;
}

vec3 normalize3(const vec3 vec) {
	float vecLen = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
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
  vec3 f = normalize3(subtract(center, eye));
  vec3 u = normalize3(up);
  vec3 s = normalize3(cross3(f, u));
  u = cross3(s,f);

  Matrix mat = IDENTITY_MATRIX;
  mat.m[0] = s.x;
  mat.m[1] = u.x;
  mat.m[2] = -f.x;
  //mat.m[3] = 0.0f;

  mat.m[4] = s.y;
  mat.m[5] = u.y;
  mat.m[6] = -f.y;
  //mat.m[7] = 0.0f;

  mat.m[8] = s.z;
  mat.m[9] = u.z;
  mat.m[10] = -f.z;
  //mat.m[11] = 0.0f;

  mat.m[12] = -dotf3(s, eye);
  mat.m[13] = -dotf3(u, eye);
  mat.m[14] = dotf3(f, eye);
  mat.m[15] = 1.0f;
  
  return mat;  
}

void inverse(float M[], float T[]) {
    float s[6];
    float c[6];
    
    s[0] = M[0]*M[5] - M[4]*M[1];
    s[1] = M[0]*M[6] - M[4]*M[2];
    s[2] = M[0]*M[7] - M[4]*M[3];
    s[3] = M[1]*M[6] - M[5]*M[2];
    s[4] = M[1]*M[7] - M[5]*M[3];
    s[5] = M[2]*M[7] - M[6]*M[3];

    c[0] = M[8]*M[13] - M[12]*M[9];
    c[1] = M[8]*M[14] - M[12]*M[10];
    c[2] = M[8]*M[15] - M[12]*M[11];
    c[3] = M[9]*M[14] - M[13]*M[10];
    c[4] = M[9]*M[15] - M[13]*M[11];
    c[5] = M[10]*M[15] - M[14]*M[11];

    /* Assumes it is invertible */
    float idet = 1.0f/( s[0]*c[5]-s[1]*c[4]+s[2]*c[3]+s[3]*c[2]-s[4]*c[1]+s[5]*c[0] );

    T[0] = ( M[5] * c[5] - M[6] * c[4] + M[7] * c[3]) * idet;
    T[1] = (-M[1] * c[5] + M[2] * c[4] - M[3] * c[3]) * idet;
    T[2] = ( M[13] * s[5] - M[14] * s[4] + M[15] * s[3]) * idet;
    T[3] = (-M[9] * s[5] + M[10] * s[4] - M[11] * s[3]) * idet;

    T[4] = (-M[4] * c[5] + M[6] * c[2] - M[7] * c[1]) * idet;
    T[5] = ( M[0] * c[5] - M[2] * c[2] + M[3] * c[1]) * idet;
    T[6] = (-M[12] * s[5] + M[14] * s[2] - M[15] * s[1]) * idet;
    T[7] = ( M[8] * s[5] - M[10] * s[2] + M[11] * s[1]) * idet;

    T[8] = ( M[4] * c[4] - M[5] * c[2] + M[7] * c[0]) * idet;
    T[9] = (-M[0] * c[4] + M[1] * c[2] - M[3] * c[0]) * idet;
    T[10] = ( M[12] * s[4] - M[13] * s[2] + M[15] * s[0]) * idet;
    T[11] = (-M[8] * s[4] + M[9] * s[2] - M[11] * s[0]) * idet;

    T[12] = (-M[4] * c[3] + M[5] * c[1] - M[6] * c[0]) * idet;
    T[13] = ( M[0] * c[3] - M[1] * c[1] + M[2] * c[0]) * idet;
    T[14] = (-M[12] * s[3] + M[13] * s[1] - M[14] * s[0]) * idet;
    T[15] = ( M[8] * s[3] - M[9] * s[1] + M[10] * s[0]) * idet;
}

void mat4transpose(float *M, const float *N)
{
  int i, j;
  for (j = 0; j < 4; ++j) {
    for (i = 0; i < 4; ++i) {
      M[i * 4 + j] = N[j * 4 + i];
    }
  }
}

void rotate(Matrix *m, float angle, float x, float y, float z) {
  float radians = rad(angle);
  float c = cosf(radians);
  float s = sinf(radians);
  float length = sqrtf(x * x + y * y + z * z);
  float u[3] = {x / length, y / length, z / length};

  Matrix rotation;
  rotation.m[0] = c + u[0] * u[0] * (1 - c);
  rotation.m[1] = u[0] * u[1] * (1 - c) - u[2] * s;
  rotation.m[2] = u[0] * u[2] * (1 - c) + u[1] * s;
  rotation.m[3] = 0.0f;

  rotation.m[4] = u[1] * u[0] * (1 - c) + u[2] * s;
  rotation.m[5] = c + u[1] * u[1] * (1 - c);
  rotation.m[6] = u[1] * u[2] * (1 - c) - u[0] * s;
  rotation.m[7] = 0.0f;

  rotation.m[8] = u[2] * u[0] * (1 - c) - u[1] * s;
  rotation.m[9] = u[2] * u[1] * (1 - c) + u[0] * s;
  rotation.m[10] = c + u[2] * u[2] * (1 - c);
  rotation.m[11] = 0.0f;

  rotation.m[12] = 0.0f;
  rotation.m[13] = 0.0f;
  rotation.m[14] = 0.0f;
  rotation.m[15] = 1.0f;

  Matrix out = multiplymat4(*m, rotation);
  memcpy(m,&out,sizeof(Matrix));
}
