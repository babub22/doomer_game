//#include "deps.h"
#include <math.h>

typedef struct {
	// x z because i use vec2
	// only when i work with grid
	// without height
	float x, z;
} vec2;

typedef struct {
	float x, y;
} uv2;

typedef struct {
	int x, z;
} vec2i;

typedef struct {
	float x, y, z;
} vec3;

typedef struct {
	int x, y, z;
} vec3i; // mostly to srote indexes of grid cell

typedef struct {
	double x, y, z;
} vec3d;

typedef struct {
    int x, y, z, w;
} vec4i;

typedef struct Matrix {
	float m[16];
} Matrix;


/*
0 1 2 3
4 5 6 7
8 9 A B
C D E F
*/

typedef struct{
  float x, y, z, w;
} vec4;

#define f(div) (float) div

#define argVec4(vec) vec.x, vec.y, vec.z, vec.w 
#define argVec3(vec) vec.x, vec.y, vec.z
#define argVec2(vec) vec.x, vec.z

Matrix fpsView(vec3 eye, float pitch, float yaw);

#define rad(deg) deg * 3.14159265358979323846/180.0

#define dotf3(v1,v2) v1.x * v2.x + v1.y * v2.y + v1.z * v2.z
#define dotf4(v1,v2) v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w

static const vec4 X_AXIS = {1, 0, 0, 0};
static const vec4 Y_AXIS = {0, 1, 0, 0};
static const vec4 Z_AXIS = {0, 0, 1, 0};
static const vec4 INV_X_AXIS = {-1, 0, 0, 0};
static const vec4 INV_Y_AXIS = {0, -1, 0, 0};
static const vec4 INV_Z_AXIS = {0, 0, -1, 0};

static const Matrix IDENTITY_MATRIX = {{
	1,
	0,
	0,
       	0,
	
	0,
	1,
	0,
       	0,
	
	0,
	0,
	1,
	0,
	
	0,
	0,
	0,
	1
}};

Matrix multiplymat4(Matrix m1, Matrix m2);
vec4 mulmatvec4(Matrix m, vec4 v);
vec4 cross4(vec4 v1, vec4 v2);
vec3 normalize3(const vec3 vec);

float magnitude4(vec4 v);
vec4 addvec4(vec4 v1, vec4 v2);
vec4 multvec4(vec4 v, float n);
vec4 slerp(vec4 p0, vec4 p1, float t);

vec3 matrixMultPoint(const float matrix[16], vec3 point);
void mat4transpose(float *M, const float *N);

vec3 cross3(const vec3 v1, const vec3 v2);
void rotateX(const Matrix* m, float angle);
void rotateY(const Matrix* m, float angle);
void rotateZ(const Matrix* m, float angle);
void scale(const Matrix* m, float x, float y, float z);
void translate(const Matrix* m, float x, float y, float z);

void inverse(float M[], float T[]);

Matrix perspective(float fovy, float aspect_ratio, float near_plane, float far_plane);

Matrix orthogonal(float l, float r, float b, float t, float n, float f);

Matrix lookAt(vec3 eye, vec3 target, vec3 up);

#define vec3ToVec4(vec3, vec4) (vec4){ vec3.x, vec3.y, vec3.z, 1.0f} 

#define map(x, in_min, in_max, out_min, out_max)(x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min

void rotate(Matrix *m, float rad, float x, float y, float z);

Matrix mat4_from_quat(vec4 q);

//Matrix gltfTRS(vec3 T, vec4 R, vec3 S);
Matrix gltfTRS(float* t);

Matrix fromRotationTranslationScale(vec4 q, vec3 v, vec3 s);
Matrix multMat4(Matrix a, Matrix b);

float angle2Vec(vec2 v1, vec2 v2);

Matrix lootAt2(vec3 eye, vec3 center, vec3 up);

Matrix addMats(Matrix a, Matrix b);
Matrix mulMatNum(Matrix a, float n);

vec3 addvec3(vec3 v1, vec3 v2);

vec4 normalize4(vec4 v);

Matrix quatToMat(vec4 quat);
vec4 subQuats(vec4 a, vec4 b);

vec3 rotateVectorByQuaternion(vec3 v, vec4 q);
vec3 mulVec3Num(vec3 a, float n);

float triArea2D(vec2 a, vec2 b, vec2 c);

vec3 interpolate2dTo3d(vec3 a, vec3 b, vec3 c, vec2 p);

#define fcmp(a,b) fabs(a-b)<FLT_EPSILON
