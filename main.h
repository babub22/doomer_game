#pragma once

typedef struct {
  float x, y;
} vec2;

typedef struct {
  float x, y, z;
} vec3;

typedef struct {
  float x, y, z, i;
} vec4;

typedef struct {
  double x, y, z;
} vec3d;

typedef struct{
  float x,y,w,h;
} rect2;

typedef struct{
  float x,y,z,w,h,d;
} box;

typedef enum{
  top,
  bot,
  right,
  left,
  basicSideCounter,
  topLeft,
  botLeft,
  botRight,
  rightTop,
  sideCounter
} Side;

typedef struct{
  vec3 min;
  vec3 max;
} AABB;

typedef struct {
  // use pos as min for AABB
  vec3 pos; // normal XY

  vec3 min; // normal XY
  vec3 max;

  float angle;
  Side side;

  
  // vec3 colBox;
  float w, h, d;
} Entity;

typedef struct{
  int frames;
} AnimTimer;

typedef enum{
  wallT=1,
  doorT,
  windowT,
  halfWallT,
  wallTypeCounter
} WallType;

typedef struct{
  bool opened;
} DoorInfo;

typedef enum{
  doorObj,
} ObjectType;

typedef float mat4[16];
typedef float mat3[9];

typedef struct{
  vec3 pos;
  vec3 target;
  vec3 dir;

  vec3 front;
  vec3 up;
  vec3 right;

  float yaw;
  float pitch;
} Camera;

typedef struct{
  int id;

  vec3 pos;
  
  ObjectType type;
  void* objInfo;

  vec3 min;
  vec3 max;

  AnimTimer anim;
} Object;

typedef struct{
  int walls;
  Object** wallsData;
  
  Object** objs; // without walls objs
  size_t objsSize;
  
  int center;    
} Tile;

typedef struct{
  // ray of cursor
  vec3 start;
  vec3 end;

  vec2 screenPos;

  bool clickR;
  bool clickL; // in this frame

  Side tileSide;
  Side wallSide;

  int wheel;

  vec3 intersection;
  vec2 gridIntesec;
  
  Tile* selectedTile;

  WallType brush;
  
  float w, h; // of cursor
} Mouse;

#define rad(deg) deg * M_PI / 180

#define vec3dToVec3(vec3d) (vec3){vec3d.x,vec3d.y,vec3d.z}

void addObjToTile(Tile* tile, Object* obj);

void renderWall(vec3 pos, GLenum mode, float blockW, float blockD, WallType wall, float wallH, float r, float g, float b);

bool gluInvertMatrix(const double m[16], double invOut[16]);

void renderCube(vec3 pos, float w, float h, float d, float r, float g, float b);

void renderTile(vec3 pos, GLenum mode, float w, float d, float r, float g, float b);

vec3 normalize(const vec3 vec);

int dot(const vec3 v1, const vec3 v2);

vec3 cross(const vec3 v1, const vec3 v2);

bool rayIntersectsTriangle(const vec3 start, const vec3 end, const vec3 lb, const vec3 rt, vec3* posOfIntersection);

void addObjToStore(Object* obj);

vec3 matrixMultPoint(const float matrix[16], vec3 point);

#define FPS 60

#define greenColor 0.0f, 1.0f, 0.0f

#define redColor 1.0f, 0.0f, 0.0f

#define blueColor 0.0f, 0.0f, 1.0f

#define darkPurple 0.1f, 0.0f, 0.1f

#define cyan 0.0f, 1.0f, 1.0f
#define white 1.0f, 1.0f, 1.0f 

#define gridZ 30
#define gridX 30
#define gridY 12

#define game "Doomer game"

#define speed 0.001f/2

