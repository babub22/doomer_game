#pragma once

#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"

typedef enum{
  top,
  bot,
  right,
  left,
  basicSideCounter,
  center,
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
  //  doorT,
  halfWallT,
  doorFrameT,
  windowT,
  //windowLeftT,
  //windowRightT,
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
  vec3 normal;
  float distance; 
} Plane;

typedef struct{
  GLuint VBO;
  GLuint VAO;

  // TODO: unhardcode VBOsize in VBO generator
  int VBOsize;
} MeshBuffer;

typedef struct{
  vec3 pos;
  vec3 target;
  vec3 dir;

  vec3 front;
  vec3 up;
  vec3 right;

  float yaw;
  float pitch;
  
  vec3 X;
  vec3 Y;
  vec3 Z;
} Camera;

// to add new texture to game
// add to this enum and to
// ./texture/ .bmp image with
// same number
typedef enum{
  woodPlanks,
  metal,
  ground,
  redClo,
  frozenGround,
  pinTree,
  treeCore,
  solidColorTx,
  texturesCounter
} Texture;

typedef enum{
  snow,
  particlesCounter
} Particles;

typedef enum{
  particles,
  textures,
  assetsTypes,
} AssetType;


typedef struct{
  GLuint VBO;
  GLuint VAO;
  bool open;
} Menu;

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
  int wallsTx;
  
  Object** wallsData;

  // 1 byte - empty/net/textured
  // 2 byte - under texture id
  // 3 byte - over texture id
  // 4 byte - empty
  int ground;    
} Tile;

typedef enum{
  netTile = 1,
  texturedTile,
} GroundType;

typedef enum{
  yalinka,
  modelsCounter
} ModelName;

typedef struct{
  GLuint VBO;
  GLuint VAO;

  // for AABB recalculating
  vec3* vertices;

  ModelName name;
  Matrix mat;

  int size;

  vec3 lb;
  vec3 rt;
} Model;

typedef enum{
  // 1,2 to access .ground
  // with bitwise
  fromUnder = 1,
  fromOver = 2,
} GroundInter;

typedef struct{
  // ray of cursor
  vec2 screenPos;

  bool clickR;
  bool clickL; // in this frame

  Side tileSide;

  Side wallSide;
  vec3i wallTile;
  WallType wallType;
  Texture wallTx;
  // tile under selected wall

  GroundInter groundInter;
  
  int wheel;

  vec3 rayDir;

  float interDist;
  //  bool is 
  
  vec3 intersection;
  vec2i gridIntersect;
  
  Tile* selectedTile;

  WallType brush;
  
  float w, h; // of cursor

  // resets to NULL every start of frame
  Model* selectedModel;
  // player focused to manipulate
  // on key
  Model* focusedModel;
} Mouse;

typedef struct{
  float w, h, d;
} Sizes;

typedef struct{
  bool    active;

  float   life;
  float   fade;

  float   x;                
  float   y;                  
  float   z;                   
} Particle;

typedef struct{
  bool fog;
  bool snow;
} EnviromentalConfig;

typedef enum{
  WallEl,
  TileEl
} ElementType;

typedef enum{
  ROTATE_X,
  ROTATE_Y,
  ROTATE_Z,
  TRANSFORM_XY,
  TRANSFORM_Z,
  SCALE
} ManipulationMode;



#define snowGravity -0.8f
#define snowDefAmount 20000

#define editorFOV 50.0f

#define distToCamera(point) sqrtf(powf(point.x - camera.pos.x, 2) + powf(point.y - camera.pos.y, 2) + powf(point.z - camera.pos.z, 2))

void renderCube(vec3 pos, float w, float h, float d, float r, float g, float b);

bool rayIntersectsTriangle(vec3 origin, vec3 dir, vec3 lb, vec3 rt, vec3* posOfIntersection, float* dist);

void addObjToStore(Object* obj);

int textTexture(char *text, TTF_Font *font, float r, float g, float b, float a);

vec3 matrixMultPoint(const float matrix[16], vec3 point);

Object* doorConstructor(vec3 pos, bool opened);

bool oppositeTileTo(vec2i XZ, Side side, vec2i* opTile, Side* opSid);

vec3* wallPosBySide(Side side, float wallH, float wallD, float tileD, float tileW);

void wallsLoadVAOandVBO();

Model* loadOBJ(char* path, float scale);

// Macro-Functions
// ~~~~~
#define valueIn(num, index) (num >> (index*8)) & 0xFF

#define setIn(num, index, newValue) num &=~(0xFF << (index * 8)); num |= (newValue << index * 8);

#define deleteIn(num, index) num &= ~(0xFF << (index * 8));

#define xyz_coordsToIndexes(x,y,z) {x / bBlockW, y / bBlockH, z / bBlockD}

#define xyz_indexesToCoords(x,y,z) {(float)x * bBlockW, (float)y * bBlockH, (float)z * bBlockD}

#define vec3_coordsToIndexes(vecCoords) {vecCoords.x / bBlockW, vecCoords.y / bBlockH, vecCoords.z / bBlockD}

#define vec3_indexesToCoords(vecIndexes) {(float)vecIndexes.x * bBlockW, (float)vecIndexes.y * bBlockH, (float)vecIndexes.z * bBlockD}

#define setSolidColorTx(color, a) do {					\
    float rgbaColor[] = {color, a};					\
    uint8_t colorBytes[4] = {(uint8_t)(rgbaColor[0] * 255), (uint8_t)(rgbaColor[1] * 255), (uint8_t)(rgbaColor[2] * 255), (uint8_t)(rgbaColor[3] * 255)};\
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, colorBytes);\
} while(false)
// ~~~~~~

// it also assigns lb, rt to model
void calculateModelAABB(Model* model);

bool radarCheck(vec3 point);

Model* loadOBJ(char* path, ModelName name);

#define increaseVAOnVBO() boundVBO++; boundVAO++

GLuint loadShader(GLenum shaderType, const char* filename);

#define FPS 60

#define greenColor 0.0f, 1.0f, 0.0f

#define blackColor 0.0f, 0.0f, 0.0f

#define redColor 1.0f, 0.0f, 0.0f

#define blueColor 0.0f, 0.0f, 1.0f

#define whiteColor 1.0f, 1.0f, 1.0f, 1.0f

#define darkPurple 0.1f, 0.0f, 0.1f

#define cyan 0.0f, 1.0f, 1.0f

#define game "Doomer game"
#define texturesFolder "./assets/textures/"

#define speed 0.001f/2

#define bBlockW 0.1f
#define bBlockD 0.1f
#define bBlockH 0.2f

#define selBorderD 0.01f
#define selBorderT 0.01f

#define selTileBorderH 0.001f



  /*
    GL_QUADS order

  glTexCoord2f(0.0f, 1.0f); glVertex3d(pos[0].x, pos[0].y, pos[0].z);
  glTexCoord2f(1.0f, 1.0f); glVertex3d(pos[1].x, pos[1].y, pos[1].z);
  glTexCoord2f(1.0f, 0.0f); glVertex3d(pos[2].x, pos[2].y, pos[2].z);
  glTexCoord2f(0.0f, 0.0f); glVertex3d(pos[3].x, pos[3].y, pos[3].z);
   */
