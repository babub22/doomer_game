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

typedef enum{
  top = 0,
  bot = 8,
  right = 16,
  left = 24,
  sideCounter = 4,
} WallSide;

typedef enum{
  wallT=1,
  doorT,
  windowT,
  halfWallT,
} WallType;

typedef struct{
  bool opened;
  WallType dir;
} DoorInfo;

typedef enum{
  doorObj,
} ObjectType;

typedef struct{
  int id;
  vec3 pos;
  ObjectType type;
  void* objInfo;
} Object;

typedef struct{
  // maybe make walls contains types
  // topDoor/topWall/topWindow
  // it assumes that we can have
  // only wall or door or window
  int walls;  // top, bot, left, right

  Object** objs; // on this tile
  size_t objsSize;
  
  int center;    
} Tile;

typedef struct{
  vec3 start;
  vec3 end;

  vec2 screenPos;

  bool click; // in this frame

  float w, h; // of cursor
} Mouse;

#define vec3dToVec3(vec3d) (vec3){vec3d.x,vec3d.y,vec3d.z}

void addObjToTile(Tile* tile, Object* obj);

void renderWall(vec3 pos, GLenum mode, float blockW, float blockD, WallType wall, float wallH, float r, float g, float b);

void renderCube(vec3 pos, float w, float h, float d, float r, float g, float b);

void renderTile(vec3 pos, float w, float d, float r, float g, float b);

vec3 normalize(const vec3 vec);

bool rayIntersectsTriangle(const vec3 start, const vec3 end, const vec3 lb, const vec3 rt);

void addObjToStore(Object* obj);

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
