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

typedef struct Matrix {
	float m[16];
} Matrix;

/*
0 1 2 3
4 5 6 7
8 9 A B
C D E F
*/

typedef union vec4 {
	 float m[4];
	 struct {
	 	float x, y, z, w;
	 };
} vec4;

#define dotf3(v1,v2) v1.x * v2.x + v1.y * v2.y + v1.z * v2.z
#define dotf4(v1,v2) v1.m[0] * v2.m[0] + v1.m[1] * v2.m[1] + v1.m[2] * v2.m[2] + v1.m[3] * v2.m[3]

static const vec4 X_AXIS = {{1, 0, 0, 0}};
static const vec4 Y_AXIS = {{0, 1, 0, 0}};
static const vec4 Z_AXIS = {{0, 0, 1, 0}};
static const vec4 INV_X_AXIS = {{-1, 0, 0, 0}};
static const vec4 INV_Y_AXIS = {{0, -1, 0, 0}};
static const vec4 INV_Z_AXIS = {{0, 0, -1, 0}};

static const Matrix IDENTITY_MATRIX = {{
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
}};

Matrix multiplymat4(const Matrix* m1, const Matrix* m2);
vec4 mulmatvec4(const Matrix* m, const vec4* v);
void normalize4(vec4* v);
vec4 cross4(vec4 v1, vec4 v2);
vec3 normalize3(const vec3 vec);
vec3 matrixMultPoint(const float matrix[16], vec3 point);
vec3 cross3(const vec3 v1, const vec3 v2);
void rotateX(const Matrix* m, float angle);
void rotateY(const Matrix* m, float angle);
void rotateZ(const Matrix* m, float angle);
void scale(const Matrix* m, float x, float y, float z);
void translate(const Matrix* m, float x, float y, float z);

void inverse(float M[], float T[]);

Matrix perspective(float fovy, float aspect_ratio, float near_plane, float far_plane);
Matrix orthogonal(float left, float right, float bottom, float top);

Matrix lookAt(vec3 eye, vec3 center, vec3 up);

#define vec3ToVec4(vec3, vec4) (vec4){ vec3.x, vec3.y, vec3.z, 1.0f} 

