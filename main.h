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

  float angle;
  
  float w, h, d;
} Entity;

typedef struct{
  int frames;
} AnimTimer;

typedef enum{
  top,
  bot,
  right,
  left,
  sideCounter,
} Side;

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

typedef struct{
  int id;

  vec3 pos;
  
  ObjectType type;
  void* objInfo;

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

  vec3 intersection;
  vec2 gridIntesec;
  
  Tile* selectedTile;

  WallType brush;
  
  float w, h; // of cursor
} Mouse;

#define vec3dToVec3(vec3d) (vec3){vec3d.x,vec3d.y,vec3d.z}

void addObjToTile(Tile* tile, Object* obj);

void renderWall(vec3 pos, GLenum mode, float blockW, float blockD, WallType wall, float wallH, float r, float g, float b);

void renderCube(vec3 pos, float w, float h, float d, float r, float g, float b);

void renderTile(vec3 pos, GLenum mode, float w, float d, float r, float g, float b);

vec3 normalize(const vec3 vec);

bool rayIntersectsTriangle(const vec3 start, const vec3 end, const vec3 lb, const vec3 rt, vec3* posOfIntersection);

void addObjToStore(Object* obj);

#define FPS 60

#define greenColor 0.0f, 1.0f, 0.0f

#define redColor 1.0f, 0.0f, 0.0f

#define blueColor 0.0f, 0.0f, 1.0f

#define darkPurple 0.1f, 0.0f, 0.1f

#define gridH 8
#define gridW 8

#define windowW 800
#define windowH 600

#define game "Doomer game"
