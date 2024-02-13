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

typedef struct {
  vec3 pos; // normal XY
  float w, h, d;
} Entity;

typedef struct{
  int walls;  // top, bot, left, right
  
  int center;    
} Tile;

typedef enum{
  top,
  bot,
  right,
  left,
} WallType;

typedef struct{
  vec3d worldPos;

  vec3d start;
  vec3d end;

  vec2 screenPos;

  float w, h; // of cursor
} Mouse;

vec2 toIsoVec2(vec2 point);

void renderWall(vec3 pos, float blockW, float blockD, WallType wall, float wallH, float r, float g, float b);

void renderCube(vec3 pos, float w, float h, float d, float r, float g, float b);

void renderTile(vec3 pos, float w, float d, float r, float g, float b);

vec3d normalize(vec3d vec);

bool rayIntersectsTriangle(const vec3d start, const vec3d end, const vec3d point);

#define FPS 60

#define greenColor 0.0f, 1.0f, 0.0f

#define redColor 1.0f, 0.0f, 0.0f

#define blueColor 0.0f, 0.0f, 1.0f

#define darkPurple 0.1f, 0.0f, 0.1f
