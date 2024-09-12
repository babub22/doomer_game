#include "deps.h"
#include "linearAlg.h"

static const double PI = 3.14159265358979323846;

Matrix multiplymat4(Matrix m1, Matrix m2) {
  Matrix out = IDENTITY_MATRIX;
  //  unsigned int row, column, row_offset;

  for (int row = 0; row < 4; ++row) {
    for (int column = 0; column < 4; ++column) {
      out.m[row * 4 + column] =
	(m1.m[row * 4 + 0] * m2.m[column + 0]) +
	(m1.m[row * 4 + 1] * m2.m[column + 4]) +
	(m1.m[row * 4 + 2] * m2.m[column + 8]) +
	(m1.m[row * 4 + 3] * m2.m[column + 12]);
    }
  }

	/*	for (row = 0, row_offset = row * 4; row < 4; ++row, row_offset = row * 4)
		for (column = 0; column < 4; ++column)
			out.m[row_offset + column] =
				(m1.m[row_offset + 0] * m2.m[column + 0]) +
				(m1.m[row_offset + 1] * m2.m[column + 4]) +
				(m1.m[row_offset + 2] * m2.m[column + 8]) +
				(m1.m[row_offset + 3] * m2.m[column + 12]);*/

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

Matrix perspective(float fovy, float aspect_ratio, float n, float f) {
  Matrix mat;
  float const a = 1.f / tanf(fovy / 2.f);

  mat.m[0] = a / aspect_ratio;
  // mat.m[0] = a / aspect_ratio;
  mat.m[1] = 0.f;
  mat.m[2] = 0.f;
  mat.m[3] = 0.f;

  mat.m[4] = 0.f;
  mat.m[5] = a;
  mat.m[6] = 0.f;
  mat.m[7] = 0.f;

  mat.m[8] = 0.f;
  mat.m[9] = 0.f;
  mat.m[10] = -((f + n) / (f - n));
  //mat.m[11] = -((2.f * f * n) / (f - n));
  mat.m[11] = -1.f;

  mat.m[12] = 0.f;
  mat.m[13] = 0.f;
  mat.m[14] = -1.f;
  //mat.m[14] = -((2.f * f * n) / (f - n));
  mat.m[15] = 0.f;

  return mat;
}

Matrix orthogonal(float l, float r, float b, float t, float n, float f)
{
  Matrix M = IDENTITY_MATRIX;

    M.m[0] = 2.0f / (r - l);
    M.m[5] = 2.0f / (t - b);
    M.m[10] = -2.0f / (f - n);
    M.m[15] = 1.0f;

    M.m[12] = -(r + l) / (r - l);
    M.m[13] = -(t + b) / (t - b);
    M.m[14] = -(f + n) / (f - n);
    return M;
}

vec3 cross3(const vec3 v1, const vec3 v2) {
  vec3 crossproduct = { 0, 0, 0 };

  crossproduct.x = v1.y * v2.z - v1.z * v2.y;
  crossproduct.y = v1.z * v2.x - v1.x * v2.z;
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

Matrix lookAt(vec3 eye, vec3 target, vec3 up) {
  Matrix mat = IDENTITY_MATRIX;
  
  vec3 Z = normalize3(subtract(eye, target)); // Z
  //  vec3 s = normalize3(cross3(up, f));
  vec3 X = normalize3(cross3(up, Z)); // x
  vec3 Y = cross3(Z, X); // y

  mat.m[0] = X.x;
  mat.m[1] = Y.x;
  mat.m[2] = Z.x;
  mat.m[3] = 0.f;

  mat.m[4] = X.y;
  mat.m[5] = Y.y;
  mat.m[6] = Z.y;
  mat.m[7] = 0.f;

  mat.m[8] = X.z;
  mat.m[9] = Y.z;
  mat.m[10] = Z.z;
  mat.m[11] = 0.f;

  //  mat.m[12] = -dotf3(X, eye);
  //  mat.m[13] = -dotf3(Y, eye);
  //  mat.m[14] = -dotf3(Z, eye);

  mat.m[12] = -dotf3(X, eye);
  mat.m[13] = -dotf3(Y, eye);
  mat.m[14] = -dotf3(Z, eye);
  mat.m[15] =  1.f;

  //  mat4x4_translate_in_place(m, -eye[0], -eye[1], -eye[2]);

  return mat;
}

Matrix fpsView(vec3 eye, float pitch, float yaw){
  float cosPitch = cosf(rad(pitch));
  float sinPitch = sinf(rad(pitch));
  float cosYaw = cosf(rad(yaw));
  float sinYaw = sinf(rad(yaw));

  vec3 xaxis = { cosYaw, 0, -sinYaw };
  vec3 yaxis = { sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };
  vec3 zaxis = { sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw };

  Matrix mat;

  mat.m[0] = xaxis.x;
  mat.m[1] = yaxis.x;
  mat.m[2] = zaxis.x;
  mat.m[3] = 0.f;

  mat.m[4] = xaxis.y;
  mat.m[5] = yaxis.y;
  mat.m[6] = zaxis.y;
  mat.m[7] = 0.f;

  mat.m[8] = xaxis.z;
  mat.m[9] = yaxis.z;
  mat.m[10] = zaxis.z;
  mat.m[11] = 0.f;
  
  mat.m[12] = -dotf3(xaxis, eye);
  mat.m[13] = -dotf3(yaxis, eye);
  mat.m[14] = -dotf3(zaxis, eye);
  mat.m[15] =  1.f;

  return mat;
}

float sign(vec2 p1, vec2 p2, vec2 p3){
    return (p1.x - p3.x) * (p2.z - p3.z) - (p2.x - p3.x) * (p1.z - p3.z);
}

vec3 interpolate2dTo3d(vec3 a, vec3 b, vec3 c, vec2 p){
    float x1 = a.x;
    float x2 = b.x;
    float x3 = c.x;

    float z1 = a.z;
    float z2 = b.z;
    float z3 = c.z;

    float w1=((z2-z3)*(p.x-x3)+(x3-x2)*(p.z-z3))
	/((z2-z3)*(x1-x3)+(x3-x2)*(z1-z3));

    float w2=((z3-z1)*(p.x-x3)+(x1-x3)*(p.z-z3))
	/((z2-z3)*(x1-x3)+(x3-x2)*(z1-z3));

    float w3=1-w1-w2;

    float yP = w1 * a.y + w2 * b.y + w3 * c.y;

    return (vec3){p.x,yP,p.z};
}

float triArea2D(vec2 a, vec2 b, vec2 c){
    return fabs((a.x*(b.z-c.z)
		 + b.x*(c.z-a.z) + c.x*(a.z-b.z)));// * .5f);
};

float triArea2Di(vec2 a, vec2 b, vec2 c){
    return fabs((a.x*(b.z-c.z)
		 + b.x*(c.z-a.z) + c.x*(a.z-b.z)));// * .5f);
};

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

void rotate(Matrix *m, float rad, float x, float y, float z) {
  //  float radians = rad(angle);
  float c = cosf(rad);
  float s = sinf(rad);
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

Matrix mat4_from_quat(vec4 q) {
    Matrix M = IDENTITY_MATRIX;
    /*
    float w = q.w;
    float x = q.x;
    float y = q.y;
    float z = q.z;
//    float a2 = a*a;
    float x2 = x*x;
    float y2 = y*y;
    float z2 = z*z;

    M.m[0] = 1.0f - 2.0f * (y2 + z2);
    M.m[1] = 2.0f * (x*y - z*w);
    M.m[2] = 2.0f * (x*z + y*w);
    M.m[3] = 0;

    M.m[4] = 2.0f * (x*y + z*w);
    M.m[5] = 1.0f - 2.0f * (x2 + z2);
    M.m[6] = 2.0f * (y*z - x*w);
    M.m[7] = 0;

    M.m[8] = 2.0f * (x*z - y*w);
    M.m[9] = 2.0f * (y*z + x*w);
    M.m[10] = 1.0f - 2.0f * (x2 - y2);
    M.m[11] = 0;

    M.m[12] = 0;
    M.m[13] = 0;
    M.m[14] = 0;
    M.m[15] = 1;*/

    float w = q.w;
    float x = q.x;
    float y = q.y;
    float z = q.z;

    float xy = x * y;
    float xz = x * z;
    float xw = x * w;
    float yz = y * z;
    float yw = y * w;
    float zw = z * w;
    float xSquared = x * x;
    float ySquared = y * y;
    float zSquared = z * z;
    M.m[0] = 1 - 2 * (ySquared + zSquared);
    M.m[1] = 2 * (xy - zw);
    M.m[2] = 2 * (xz + yw);
    M.m[3] = 0;
    M.m[4] = 2 * (xy + zw);
    M.m[5] = 1 - 2 * (xSquared + zSquared);
    M.m[6] = 2 * (yz - xw);
    M.m[7] = 0;
    M.m[8] = 2 * (xz - yw);
    M.m[9] = 2 * (yz + xw);
    M.m[10] = 1 - 2 * (xSquared + ySquared);
    M.m[11] = 0;
    M.m[12] = 0;
    M.m[13] = 0;
    M.m[14] = 0;
    M.m[15] = 1;

    return M;
}

//Matrix gltfTRS(vec3 T, vec4 R, vec3 S){
Matrix gltfTRS(float* t){
    float x = t[6];
    float y = t[7];
    float z = t[8];
    float w = t[9];
    
    float x2 = x + x;
    float y2 = y + y;
    float z2 = z + z;
    float xx = x * x2;
    float xy = x * y2;
    float xz = x * z2;
    float yy = y * y2;
    float yz = y * z2;
    float zz = z * z2;
    float wx = w * x2;
    float wy = w * y2;
    float wz = w * z2;
    float sx = t[3];
    float sy = t[4];
    float sz = t[5];

    Matrix out;
    
    out.m[0] = (1 - (yy + zz)) * sx;
    out.m[1] = (xy + wz) * sx;
    out.m[2] = (xz - wy) * sx;
    out.m[3] = 0;
    out.m[4] = (xy - wz) * sy;
    out.m[5] = (1 - (xx + zz)) * sy;
    out.m[6] = (yz + wx) * sy;
    out.m[7] = 0;
    out.m[8] = (xz + wy) * sz;
    out.m[9] = (yz - wx) * sz;
    out.m[10] = (1 - (xx + yy)) * sz;
    out.m[11] = 0;
    out.m[12] = t[0];
    out.m[13] = t[1];
    out.m[14] = t[2];
    out.m[15] = 1;
    
    return out;
}

vec3 rotateVectorByQuaternion(vec3 v, vec4 q) {
    vec3 u = (vec3){ argVec3(q)};
    float s = q.w;

    vec3 v1 = mulVec3Num(u, 2.0 * dotf3(u, v));
    vec3 v2 = mulVec3Num(v, (s * s - dotf3(u, u)));
    vec3 v3 = mulVec3Num(cross3(u, v), 2.0 * s);

    return addvec3(addvec3(v1, v2), v3);
}

Matrix quatToMat(vec4 quat){
    float x = quat.x;
    float y = quat.y;
    float z = quat.z;
    float w = quat.w;
    
    float x2 = x + x;
    float y2 = y + y;
    float z2 = z + z;
    float xx = x * x2;
    float xy = x * y2;
    float xz = x * z2;
    float yy = y * y2;
    float yz = y * z2;
    float zz = z * z2;
    float wx = w * x2;
    float wy = w * y2;
    float wz = w * z2;
    float sx = 1.0f;
    float sy = 1.0f;
    float sz = 1.0f;

    Matrix out;
    
    out.m[0] = (1 - (yy + zz)) * sx;
    out.m[1] = (xy + wz) * sx;
    out.m[2] = (xz - wy) * sx;
    out.m[3] = 0;
    out.m[4] = (xy - wz) * sy;
    out.m[5] = (1 - (xx + zz)) * sy;
    out.m[6] = (yz + wx) * sy;
    out.m[7] = 0;
    out.m[8] = (xz + wy) * sz;
    out.m[9] = (yz - wx) * sz;
    out.m[10] = (1 - (xx + yy)) * sz;
    out.m[11] = 0;
    out.m[12] = 0.0f;
    out.m[13] = 0.0f;
    out.m[14] = 0.0f;
    out.m[15] = 1;
    
    return out;
}

Matrix fromRotationTranslationScale(vec4 q, vec3 v, vec3 s) {
    Matrix out;
    // Quaternion math
    float x = q.x, y = q.y, z = q.z, w = q.w;
    float x2 = x + x;
    float y2 = y + y;
    float z2 = z + z;
    float xx = x * x2;
    float xy = x * y2;
    float xz = x * z2;
    float yy = y * y2;
    float yz = y * z2;
    float zz = z * z2;
    float wx = w * x2;
    float wy = w * y2;
    float wz = w * z2;

    float sx = s.x;
    float sy = s.y;
    float sz = s.z;

    out.m[0] = (1 - (yy + zz)) * sx;
    out.m[1] = (xy + wz) * sx;
    out.m[2] = (xz - wy) * sx;
    out.m[3] = 0;

    out.m[4] = (xy - wz) * sy;
    out.m[5] = (1 - (xx + zz)) * sy;
    out.m[6] = (yz + wx) * sy;
    out.m[7] = 0;

    out.m[8] = (xz + wy) * sz;
    out.m[9] = (yz - wx) * sz;
    out.m[10] = (1 - (xx + yy)) * sz;
    out.m[11] = 0;

    out.m[12] = v.x;
    out.m[13] = v.y;
    out.m[14] = v.z;
    out.m[15] = 1;

    return out;
}

Matrix multMat4(Matrix a, Matrix b){
    Matrix out;
    
    float     a00 = a.m[0];
    float    a01 = a.m[1];
    float    a02 = a.m[2];
    float	a03 = a.m[3];
    float	 a10 = a.m[4];
    float	a11 = a.m[5];
    float	a12 = a.m[6];
				 float	a13 = a.m[7];
    float	 a20 = a.m[8];
    float	a21 = a.m[9];
    float	a22 = a.m[10];
    float	a23 = a.m[11];
    float	 a30 = a.m[12];
    float	a31 = a.m[13];
    float	a32 = a.m[14];
    float	a33 = a.m[15];

    float b0,b1,b2,b3;

    b0 = b.m[0],
	b1 = b.m[1],
	b2 = b.m[2],
	b3 = b.m[3];
    out.m[0] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
    out.m[1] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
    out.m[2] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
    out.m[3] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;
    b0 = b.m[4];
    b1 = b.m[5];
    b2 = b.m[6];
    b3 = b.m[7];
    out.m[4] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
    out.m[5] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
    out.m[6] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
    out.m[7] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;
    b0 = b.m[8];
    b1 = b.m[9];
    b2 = b.m[10];
    b3 = b.m[11];
    out.m[8] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
    out.m[9] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
    out.m[10] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
    out.m[11] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;
    b0 = b.m[12];
    b1 = b.m[13];
    b2 = b.m[14];
    b3 = b.m[15];
    out.m[12] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
    out.m[13] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
    out.m[14] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
    out.m[15] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;
    return out;

}

// it should be normalized
// v1 - dir v2 = v2 - P
// where P - center
float angle2Vec(vec2 v1, vec2 v2){    
    float dot = v1.x * v2.x + v1.z * v2.z;
    float mag_v1 = sqrtf(v1.x * v1.x + v1.z * v1.z);
    float mag_v2 = sqrtf(v2.x * v2.x + v2.z * v2.z);
    float angle = acosf(dot / (mag_v1 * mag_v2));

    float cross = v1.x * v2.z - v1.z * v2.x;

    if (mag_v1 == 0 || mag_v2 == 0) {
	angle = 0.0f;  
    }else {
	float cosTheta = dot / (mag_v1 * mag_v2);
	if (cosTheta > 1.0f) cosTheta = 1.0f;
	if (cosTheta < -1.0f) cosTheta = -1.0f;
	angle = acosf(cosTheta);
    }

    if (cross < 0) {
	angle *= -1;
    }

    return angle;
}

Matrix lootAt2(vec3 eye, vec3 center, vec3 up){
  float x0, x1, x2, y0, y1, y2, z0, z1, z2, len;
  float eyex = eye.x;
  float eyey = eye.y;
  float eyez = eye.z;
  float upx = up.x;
  float upy = up.y;
  float upz = up.z;
  float centerx = center.x;
  float centery = center.y;
  float centerz = center.z;

  ///if (absf(eyex - centerx) < ENPSI && absf(eyey - centery) < EPSILON && absf(eyez - centerz) < EPSILON) {
  //  return IDENTITY_MATRIX;
  //}

  z0 = eyex - centerx;
  z1 = eyey - centery;
  z2 = eyez - centerz;
  len = 1 / hypotf(z0, z1, z2);
  z0 *= len;
  z1 *= len;
  z2 *= len;
  x0 = upy * z2 - upz * z1;
  x1 = upz * z0 - upx * z2;
  x2 = upx * z1 - upy * z0;
  len = hypotf(x0, x1, x2);

  if (!len) {
    x0 = 0;
    x1 = 0;
    x2 = 0;
  } else {
    len = 1 / len;
    x0 *= len;
    x1 *= len;
    x2 *= len;
  }

  y0 = z1 * x2 - z2 * x1;
  y1 = z2 * x0 - z0 * x2;
  y2 = z0 * x1 - z1 * x0;
  len = hypotf(y0, y1, y2);

  if (!len) {
    y0 = 0;
    y1 = 0;
    y2 = 0;
  } else {
    len = 1 / len;
    y0 *= len;
    y1 *= len;
    y2 *= len;
  }

  Matrix out;

  out.m[0] = x0;
  out.m[1] = y0;
  out.m[2] = z0;
  out.m[3] = 0;
  out.m[4] = x1;
  out.m[5] = y1;
  out.m[6] = z1;
  out.m[7] = 0;
  out.m[8] = x2;
  out.m[9] = y2;
  out.m[10] = z2;
  out.m[11] = 0;
  out.m[12] = -(x0 * eyex + x1 * eyey + x2 * eyez);
  out.m[13] = -(y0 * eyex + y1 * eyey + y2 * eyez);
  out.m[14] = -(z0 * eyex + z1 * eyey + z2 * eyez);
  out.m[15] = 1;
  
  return out;
}

Matrix addMats(Matrix a, Matrix b){
    Matrix out = {.m={0}};

    for(int i=0;i<16;i++){
	out.m[i] = a.m[i] + b.m[i];
    }

    return out;
}

Matrix mulMatNum(Matrix a, float n){
    Matrix out = {.m={0}};

    for(int i=0;i<16;i++){
	out.m[i] = a.m[i] * n;
    }

    return out;
}

vec3 mulVec3Num(vec3 a, float n){
    vec3 res = {
	a.x*n, a.y*n, a.z*n
    };
    
    return res;
}


float magnitude4(vec4 v){
    float dot = dotf4(v, v);
    return sqrtf(dot);
}

float magnitude3(vec3 v){
  float dot = dotf3(v, v);
  return sqrtf(dot);
}

vec4 addvec4(vec4 v1, vec4 v2){
    return (vec4){v1.x+v2.x,v1.y+v2.y,v1.z+v2.z,v1.w+v2.w};
}

vec3 addvec3(vec3 v1, vec3 v2){
    return (vec3){v1.x+v2.x,v1.y+v2.y,v1.z+v2.z };
}

vec4 multvec4(vec4 v, float n){
    return (vec4){v.x*n,v.y*n,v.z*n,v.w*n};
}

vec4 normalize4(vec4 v) {
    float length = sqrtf(dotf4(v, v));
    return multvec4(v, 1.0f / length);
}

/*vec4 slerp(vec4 p0, vec4 p1, float t) {
    float norm_p0 = magnitude4(p0);
    float norm_p1 = magnitude4(p1);
    float dot_product = dotf4(p0, p1) / (norm_p0 * norm_p1);
    
    // Clamp the dot product to avoid numerical issues
    if (dot_product > 1.0f) dot_product = 1.0f;
    if (dot_product < -1.0f) dot_product = -1.0f;

    float omega = acosf(dot_product);
    float so = sinf(omega);
    
    if (so == 0.0f) {
        // If sin(omega) is zero, return one of the endpoints
        return p0;
    }
    
    float factor_0 = sinf((1.0f - t) * omega) / so;
    float factor_1 = sinf(t * omega) / so;
    
    vec4 term1 = multvec4(p0, factor_0);
    vec4 term2 = multvec4(p1, factor_1);
    
    return addvec4(term1, term2);
    }*/

vec4 negate4(vec4 v) {
    return (vec4) {-v.x,-v.y,-v.z,-v.w};
}

vec4 slerp(vec4 p0, vec4 p1, float t) {
    // Normalize p0 and p1 to ensure they are unit quaternions
    p0 = normalize4(p0);
    p1 = normalize4(p1);

    float dot_product = dotf4(p0, p1);

    // Clamp the dot product to avoid numerical issues
    if (dot_product > 1.0f) dot_product = 1.0f;
    if (dot_product < -1.0f) dot_product = -1.0f;

    // If the dot product is negative, SLERP won't take the shorter path
    // so we invert one quaternion to ensure the shorter path is taken
    if (dot_product < 0.0f) {
        p1 = negate4(p1);
        dot_product = -dot_product;
    }

    float omega = acosf(dot_product);
    float so = sinf(omega);

    // If omega is very small, fall back to linear interpolation to avoid division by a small number
    if (so < 1e-6) {
        return addvec4(multvec4(p0, 1.0f - t), multvec4(p1, t));
    }

    float factor_0 = sinf((1.0f - t) * omega) / so;
    float factor_1 = sinf(t * omega) / so;

    vec4 term1 = multvec4(p0, factor_0);
    vec4 term2 = multvec4(p1, factor_1);

    return addvec4(term1, term2);
}

int inCircle(float x, float y, float circleX, float circleY, float r ){  
  float dx = fabs(x-circleX);
  float dy = fabs(y-circleY);
  return ( dx*dx + dy*dy <= r*r );
}

vec3 subVec3(vec3 v1, vec3 v2){
  return (vec3){v1.x-v2.x, v1.y-v2.y, v1.z-v2.z};
}

float distBetween3dLines(vec3 a1, vec3 a2, vec3 b1, vec3 b2){
  vec3 d = subVec3(a2, a1);
  vec3 n = cross3(b1, b2);
  float n_mag = magnitude3(n);
  float d_dot_n = fabs(dotf3(d, n));
  return d_dot_n / n_mag;
}

float isInsideCylinder(vec3 vertex, vec3 P1, vec3 P2) {
  vec3 axis = subtract(P2, P1);
  vec3 toVertex = subtract(vertex, P1);
  vec3 projection = { dotf3(toVertex, axis) / dotf3(axis, axis) * axis.x,
    dotf3(toVertex, axis) / dotf3(axis, axis) * axis.y,
    dotf3(toVertex, axis) / dotf3(axis, axis) * axis.z };
  vec3 closestPoint = addvec3(P1, projection);
  vec3 d = subtract(vertex, closestPoint);
  return sqrtf(dotf3(d, d));
}

float intersectCylinderLineSegment(vec3 P1, vec3 P2, vec3 A, vec3 B) {
  vec3 AB = subtract(B, A);
  vec3 P1P2 = subtract(P2, P1);
  float dotP1P2_AB = dotf3(P1P2, AB);
  float dotP1P2_P1P2 = dotf3(P1P2, P1P2);
    
  if (dotP1P2_P1P2 == 0) {
    return 0; // The cylinder axis is a point
  }

  float t = dotP1P2_AB / dotP1P2_P1P2;
  vec3 C1 = {P1.x + t * P1P2.x, P1.y + t * P1P2.y, P1.z + t * P1P2.z};
  vec3 C2 = {A.x + t * AB.x, A.y + t * AB.y, A.z + t * AB.z};
    
  vec3 d = subtract(C2, C1);
  return sqrtf(dotf3(d, d));
}
