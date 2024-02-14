#pragma once

typedef struct {
	float x, y;
} vec2;

typedef struct {
  float x, y, z;
} vec3;

typedef struct {
	double x, y, z;
} vec3d;

typedef struct{
  float x,y,w,h;
} rect2;

typedef enum{
  Room,
  Balkon
} LocationType;

typedef struct {
  vec3 pos; // normal XY
  float w, h, d;

  LocationType loc;
} Entity;

typedef struct{
  int walls;  // top, bot, left, right

  // we can place doors and windows
  // into wall but not
  // simmonteniously
  int doors;
  int windows;
  
  int center;    
} Tile;

typedef enum{
  top,
  bot,
  right,
  left,
} WallType;

typedef enum{
  door,
} ObjectType;

typedef struct{
  int id;
  vec3 pos;
  ObjectType type;
} Object;

typedef struct{
  vec3 start;
  vec3 end;

  vec2 screenPos;

  float w, h; // of cursor
} Mouse;

#define vec3dToVec3(vec3d) (vec3){vec3d.x,vec3d.y,vec3d.z}

vec2 toIsoVec2(vec2 point);

void renderWall(vec3 pos, GLenum mode, float blockW, float blockD, WallType wall, float wallH, float r, float g, float b);

void renderCube(vec3 pos, float w, float h, float d, float r, float g, float b);

void renderTile(vec3 pos, float w, float d, float r, float g, float b);

vec3 normalize(const vec3 vec);

bool rayIntersectsTriangle(const vec3 start, const vec3 end, const vec3 lb, const vec3 rt);

#define FPS 60

#define greenColor 0.0f, 1.0f, 0.0f

#define redColor 1.0f, 0.0f, 0.0f

#define blueColor 0.0f, 0.0f, 1.0f

#define darkPurple 0.1f, 0.0f, 0.1f

#define gridH 5
#define gridW 6

#define balkonH 1

#define windowW 800
#define windowH 600
