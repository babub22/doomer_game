#pragma once

typedef struct {
	float x, y;
} vec2;

typedef struct{
  float x,y,w,h;
} rect2;

typedef struct {
	vec2 pos; // normal XY
	float w, h;
} Entity;


//   0 <- Top
// 0 + 0 <- Right
//   0 <- Bot

typedef struct{
  bool exist;

  vec2 leftBot;
  vec2 rightTop;  
} Wall;

typedef enum{
  topWall,
  botWall,
  rightWall,
  leftWall,
  wallsCounter
} WallsTypes;

typedef struct{
  /*Wall* walls;
  
  Wall topWall;
  Wall botWall;
  Wall rightWall;
  Wall leftWall;
  */
  int center;    
} Tile;

vec2 toIsoVec2(vec2 point);

#define greenColor 0.0f, 1.0f, 0.0f

#define redColor 1.0f, 0.0f, 0.0f

#define blueColor 0.0f, 0.0f, 1.0f

#define darkPurple 0.1f, 0.0f, 0.1f
