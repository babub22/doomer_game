#pragma once

typedef struct {
	float x, y;
} vec2;

typedef struct {
  float x, y, z;
} vec3;

typedef struct{
  float x,y,w,h;
} rect2;

typedef struct {
	vec2 pos; // normal XY
	float w, h;
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

vec2 toIsoVec2(vec2 point);

#define greenColor 0.0f, 1.0f, 0.0f

#define redColor 1.0f, 0.0f, 0.0f

#define blueColor 0.0f, 0.0f, 1.0f

#define darkPurple 0.1f, 0.0f, 0.1f
