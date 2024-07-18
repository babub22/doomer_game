#pragma once

typedef enum{
  farRightCorT, nearRightCorT, farLeftCorT, nearLeftCorT, doorFrameT
} CornerType;

typedef struct{
  CornerType type;
  vec3 pos;
  bool used;
} NavCornerPoint;

typedef struct{
    int id;
    char* name;

    int parentId;

    int* childIds;
    int childSize;
    
    vec3 trans;
    vec4 rot;
    vec3 scale;
    
    Matrix inversedMat;
    Matrix defMat;
    
    Matrix matrix;
} BoneInfo;

int bonesSize;
BoneInfo* bones;

typedef struct{
  GLuint tx;
  
  int categoryIndex;

  int h;
  int w;

  int index1D;
  int index2D; // where [from context][index2D]
} Texture;

typedef struct{
  int index;
  char* name;

  int txWithThisCategorySize;
} Category;

#define generalContextText "[O] objects [B] blocks [T] textures [P] planes [1] wall"

#define modelContextText "[F] focus [B] dialogs [Del] delete [Scroll Up/Down] step +/-\n[LCtrl] + [[R + X/Y/Z] rotate; [T] XZ move; [Z] Y move; [G] scale]"

#define blockContextText "[LCtrl + R] rotate [Up/Down] + [Ctrl] change block geometry"

#define planeContextText "[F] focus [B] dialogs [Del] delete [Scroll Up/Down] step +/-\n[LCtrl] + [[R + X/Y/Z] rotate; [T] XZ move; [Z] Y move; [G + Up/Down/Left/Right] width/height]"

#define tileContextText "[Up] move tile up [Down] move tile down"

#define selectedWallContextText "[Right/Left]+[Ctrl] - +/- wall width [Up] move wall from [Down/Up] move wall from/to camera %s"

//, itHasBlock ? "[LCtrl + H] aling to block" : ""


typedef enum{
  left,
  top,
  bot,
  right,
  basicSideCounter,
  center,
  topLeft,
  botLeft,
  botRight,
  rightTop,
  sideCounter
} Side;

typedef struct{
  vec3 lb;
  vec3 rt;
  
  int txIndex;
} Plane;

typedef struct{
  int frames;
} AnimTimer;

typedef enum{
  normWallT,
  LRWallT,
  LWallT,
  RWallT,
  
  doorT,
  hiddenDoorT,
  //  openedDoorT,
  
  windowT,
  
  halfWallT,
  
  hiddenWallT,
  hiddenLRWallT,
  hiddenLWallT,
  hiddenRWallT,

  wallTypeCounter,
} WallType;

const char* wallTypeStr[];

typedef enum{
  roofBlockT,
  stepsBlockT,
  angledRoofT,
  
  tileBlocksCounter
} TileBlocksTypes;

typedef enum{
//    playerStartMarkerT = 0,
    locationExitMarkerT,
  
    markersCounter
} MarkersTypes;

/*
typedef struct{
    vec3 pos;
    MarkersTypes type;
} Marker;*/

typedef enum{
  blocksMenuT,
  objectsMenuT,
  dialogEditorT,
  dialogViewerT,
  planeCreatorT,
  texturesMenuT,
  lightMenuT,
  menuTypesCounter
} MenuTypes;

typedef enum{
  dirLightShadowT,
  dirLightT,
  pointLightT,
  lightsTypeCounter
} LightType;

const char* tileBlocksStr[];
const char* markersStr[];
/*= { [roofBlockT] = "Roof",[stepsBlockT] = "Steps", [angledRoofT] = "Angle Roof" };*/

const char* lightTypesStr[];// = { [dirLightShadowT] = "Dir light", [pointLightT] = "Point light" };

typedef struct{
  bool opened;
} DoorInfo;

typedef enum{
  doorObj,
} ObjectType;

typedef struct{
  GLuint VBO;
  GLuint VAO;

  // TODO: unhardcode VBOsize in VBO generator
  int VBOsize;
} MeshBuffer;


typedef enum{
  whTopPlane, whFrontPlane, whBackPlane,
  // whFrontHiddenPlane, whBackHiddenPlane,
  
  whPlaneCounter
} HiddenWallPlanes;

typedef enum{
  wTopPlane, wFrontPlane, wBackPlane, wClosePlane,
  // wLeftPlane, wRightPlane,
  //  wLeftExPlane, wRightExPlane,
  wPlaneCounter,
} WallPlanes;

const char* wallPlanesStr[];/* = {
  [wTopPlane]= "Top plane",
  [wFrontPlane]= "Front plane",
  [wBackPlane]= "Back plane",
  };*/

typedef enum{
  winFrontCapPlane, winFrontBotPlane,
  winBackCapPlane,
  winBackBotPlane,
  winCenterFrontPlane, winCenterBackPlane,
  
  winInnerTopPlane,
  winInnerBotPlane,
  winInnerLeftPlane,
  winInnerRightPlane,

  winTopPlane, winFrontPodokonik,
  
  winPlaneCounter
} WindowPlanes;

const char* windowPlanesStr[];/* = {
  [winTopPlane]= "Top plane",
  [winFrontCapPlane]= "Front-cap plane",
  [winFrontBotPlane]= "Front-bot plane",
  [winBackCapPlane]= "Back-cap plane",
  [winBackBotPlane]= "Back-bot plane",
  [winCenterBackPlane]= "Center-back plane" ,
  [winCenterFrontPlane] = "Center-front plane" ,
  [winInnerBotPlane]= "Inner-bot plane",
  [winInnerTopPlane]= "Inner-top plane",
  
  [winFrontPodokonik]= "Front-padokonik",
  [winBackPodokonik]= "Back-padokonik",
  };*/

typedef enum{
  doorCenterPlane,
  
  doorTopPlane,
  doorFrontPlane, doorBackPlane,
  doorInnerTopPlane,
  doorPlaneCounter
} DoorPlanes;

const int planesInfo[wallTypeCounter];
const char* doorPlanesStr[];

typedef struct{
  Plane* planes;
  
  WallType type;
  WallType prevType;

  uint8_t sideForMat;
  Side side;
  
  Matrix mat;

  int id;
  int tileId;
} Wall;

void calculateAABB(Matrix mat, float* vertexes, int vertexesSize, int attrSize, vec3* lb, vec3* rt);

typedef enum{
  editorCameraT, gameCameraT  
} CameraType;

typedef struct{
  vec3 pos;

  vec3 front;
  vec3 up;
  vec3 right;

  float yaw;
  float pitch;
} Camera;

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
  float x,y,w,h;
} UIRect;

typedef enum{
  charNameInput,
  replicaInput,
  // prevAnswerInput, 
  answerInput1,
  answerInput2,
  answerInput3,
  answerInput4,
  answerInput5,
  answerInput6,  dialogEditorInputsCounter
} DialogEditorInputs;

typedef enum{
  testButton,
  saveButton,
  addButton1,
  addButton2,
  addButton3,
  addButton4,
  addButton5,
  nextButton1,
  nextButton2,
  nextButton3,
  nextButton4,
  nextButton5,
  nextButton6,
  minusButton1,
  minusButton2,
  minusButton3,
  minusButton4,
  minusButton5,
  minusButton6,
  prevDialogButton,
  dialogEditorButtonsCounter
} DialogEditorButtons;

typedef struct{
  bool active;

  int charsLimit;
  
  UIRect rect;
  char** buf;
} TextInput;

typedef struct{
  GLuint VBO;
  GLuint VAO;
  
  int vertexNum;
  float* vBuf; // with size vertexNum * 5
  
  int attrSize; // 8 - for normals 5 - just vert + tx
} VPair;

typedef struct {
  VPair* pairs;
  int planesNum;
} BlockInfo;

typedef struct{
  GLuint VBO;
  GLuint VAO;

  UIRect rect; // of menu background
  
  bool open;
  MenuTypes type;
  
  // iter with DialogEditorInputs
  VPair* vpairs;
  VPair* buttonsPairs;

  // first 2 charNameInput and replicaInput
  TextInput* textInputs;
  
  UIRect* buttons;
} Menu;

typedef enum{
  answerBut1,
  answerBut2,
  answerBut3,
  answerBut4,
  answerBut5,
  answerBut6,
  closeBut,
  dialogViewerButtonsCounter
} DialogViewerButtons;

typedef struct{
  int id;

  vec3 pos;
  
  ObjectType type;
  void* objInfo;

  vec3 min;
  vec3 max;

  AnimTimer anim;
} Object;

typedef struct Tile Tile;

typedef struct {
  // copy from brush
  //  VPair vpair;
  TileBlocksTypes type; 
  int rotateAngle;
  int txIndex;

  // assign
  //  Tile* tile;
  Matrix mat;
  
  vec3 lb;
  vec3 rt;

  int id;
  int tileId;
} TileBlock;

typedef struct{
  float* buf;
  int bufSize;
  
  bool alligned;
  bool txHidden;
} WallVertexBuffer;

#define spreadMat4(mat) "%f %f %f %f\n %f %f %f %f\n %f %f %f %f\n %f %f %f %f\n ", mat[0], mat[1], mat[2], mat[3], mat[4], mat[5], mat[6], mat[7], mat[8], mat[9], mat[10], mat[11], mat[12], mat[13], mat[14], mat[15]

struct Tile{
  TileBlock* block;
  Wall* wall[2];

  // 1 byte - empty/net/textured
  // 2 byte - under texture id
  // 3 byte - over texture id
  // 4 byte - empty // maybe store H here
  int8_t tx;
    
  int8_t marker;
    
  vec3 pos;

  int id;
};

typedef struct{
  int side;
  
  int txIndex;
  int tileId;

  Wall* wall;
  
  int plane;
} WallMouseData;

typedef struct{
  int tileId;
  int tx;
  vec3 pos;
  
  vec3 intersection;
} TileMouseData;

bool isAlreadyNavPoint(vec3 point);

vec2* netTileAABB;
int netTileSize;

typedef enum{
  objectModelType,
  playerModelT,
  modelTypeCounter
} ModelType;

typedef struct{
  char* str;
  int counter;
} ModelsTypesInfo;

ModelsTypesInfo modelsTypesInfo[];/* = {
  [objectModelType] = {"Obj",0},
  [playerModelT] = {"Char", 0}
  };*/

typedef struct Dialog Dialog;

// Definition of the struct
struct Dialog {
  char* replicaText;
  // answer text for 1 page it will NULL
  char* text;
  Dialog* answers;
  int answersSize;
};

typedef struct{
  int id;
  char* name;

  Dialog dialogs;
  int dialogsLen;

  Dialog* curDialog;
  
  int* dialogHistory;
  int historySize;
  
  int curDialogIndex;
  
  int modelId;
  int modelName;
} Character;

typedef struct{
  int id;
  
  int name;
  Matrix mat;

  // -1 if not char type
  int characterId;

  vec3 lb;
  vec3 rt;

  //  vec3 centroid;
} Model;

typedef struct{
  int id;
  
  int txIndex;
  Matrix mat;
  
  // -1 if not char type
  int characterId;

  vec3 lb;
  vec3 rt;
} Picture;

typedef struct{
  char* name;

  GLuint VBO;
  GLuint VAO;
  GLuint tx;

  int size;
  
  vec3* vertices;
  float* entireVert;
  
  ModelType type;

  int index1D;
  int index2D;

  vec3 modelSizes; // def width, height, depth without scale
} ModelInfo;

void batchModels();

typedef enum {
  mouseModelT = 1,
  mouseWallT,
  mouseBlockT,
  mousePlaneT,
  mouseTileT,
  mouseLightT,
  mouseMarkerT,
  mouseEntityT,
  
  mouseSelectionTypesCounter
} MouseSelectionType;

typedef enum {
  mouseModelBrushT = 1,
  mouseWallBrushT,
  mouseTextureBrushT,
  mouseTileBrushT,
  mouseBlockBrushT,
  mouseMarkerBrushT,
  mouseEntityBrushT,
} MouseBrushType;

const char* mouseBrushTypeStr[];/* = {
  [mouseModelBrushT] = "Model",
  [mouseWallBrushT] = "Wall",
  [mouseTextureBrushT] = "Texture",
  [mouseTileBrushT] = "Tile",
  [mouseBlockBrushT] = "Block",
  };*/

typedef struct{
  // ray of cursor
  vec2 screenPos;

  bool clickR;
  bool clickL; // in this frame

  bool leftDown;
  bool rightDown;

  Side tileSide;
  
  int wheel;

  vec3 rayDir;
  vec3 rayView;
  //  vec3 worldRayPos;

  float interDist;
  
  //  float w, h; // of cursor

  MouseBrushType brushType;
  void* brushThing;

  MouseSelectionType focusedType;
  void* focusedThing;

  MouseSelectionType selectedType;
  void* selectedThing;

  vec2 cursor; // pos
  vec2 lastCursor;
  
  vec3 gizmoPosOfInter;
} Mouse;


	 
#define cursorH 0.025f
#define cursorW 0.01f

VPair cursor;

typedef struct{
  float w, h, d;
} Sizes;

float* snowMeshVertixes;

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
  ROTATE_X = 1,
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

typedef struct{
  int id;
  //  vec3 pos;
  //  vec3 color;

  uint8_t r;
  uint8_t g;
  uint8_t b;
  
  //  vec3 dir;
  LightType type;

  Matrix mat;

  // TODO: Remove constant, linear, quad replace it with curLight
  int curLightPresetIndex;

  float rad;
  float cutOff;

  bool off; // on/off light

  vec3 lb;
  vec3 rt;

  //  int depthTxIndex;
} Light;

bool navPointsDraw;

void rerenderShadowForLight(int lightId);
void batchModels();
void rerenderShadowsForAllLights();

void renderCube(vec3 pos, int lightId);

bool rayIntersectsTriangle(vec3 origin, vec3 dir, vec3 lb, vec3 rt, vec3* posOfIntersection, float* dist);

void addObjToStore(Object* obj);

bool oppositeTileTo(vec2i XZ, Side side, vec2i* opTile, Side* opSid);

vec3 matrixMultPoint(const float matrix[16], vec3 point);

Object* doorConstructor(vec3 pos, bool opened);

vec3* wallPosBySide(Side side, float wallH, float wallD, float tileD, float tileW);

void wallsLoadVAOandVBO();

void renderText(char* text, float x, float y, float scale);

// Macro-Functions
// ~~~~~
#define valueIn(num, index) (num >> (index*8)) & 0xFF

#define setIn(num, index, newValue) num &=~(0xFF << (index * 8)); num |= (newValue << index * 8);

#define deleteIn(num, index) num &= ~(0xFF << (index * 8));

#define xyz_coordsToIndexes(x,y,z) {x / bBlockW, y / bBlockH, z / bBlockD}

#define xyz_indexesToCoords(x,y,z) {(float)x * bBlockW, (float)y * floorH, (float)z * bBlockD}

#define vec3_coordsToIndexes(vecCoords) {vecCoords.x / bBlockW, vecCoords.y / bBlockH, vecCoords.z / bBlockD}

#define vec3_indexesToCoords(vecIndexes) {(float)vecIndexes.x * bBlockW, (float)vecIndexes.y * bBlockH, (float)vecIndexes.z * bBlockD}

#define setSolidColorTx(color, a) do {					\
    float rgbaColor[] = {color, a};					\
    uint8_t colorBytes[4] = {(uint8_t)(rgbaColor[0] * 255), (uint8_t)(rgbaColor[1] * 255), (uint8_t)(rgbaColor[2] * 255), (uint8_t)(rgbaColor[3] * 255)};\
    glBindTexture(GL_TEXTURE_2D, solidColorTx); \
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, colorBytes);\
} while(false)
// ~~~~~~

// it also assigns lb, rt to model
void calculateModelAABB(Model* model);

void checkMouseVSEntities();

void renderScene(GLuint curShader);

bool loadSave(char* saveName);

bool saveMap(char *saveName);

void initSnowParticles();

int lastCharPos(char* str, char ch);

bool createMap(int x, int y, int z);

bool radarCheck(vec3 point);

int strcut(char *str, int begin, int len);

void cleanString(char *str);

int strtrim(char *str);

ModelInfo* loadOBJ(char* path, char* texturePath);

int noSpacesAndNewLinesStrLen(char* str);

#define increaseVAOnVBO() boundVBO++; boundVAO++

GLuint loadShader(GLenum shaderType, const char* filename);

SDL_Surface* IMG_Load_And_Flip_Vertical(char* path);

void destroyCharacter(int id);
void destroyDialogsFrom(Dialog* root);
int deserializeDialogTree(Dialog* root, Dialog* parent, FILE *fp);
void serializeDialogTree(Dialog* root, FILE *fp);

float* uiRectPercentage(float x, float y, float w, float h);
float* uiRectPoints(float x, float y, float w, float h);

TileBlock* constructNewBlock(int type, int angle);

void assembleWindowBlockVBO();
void assembleWallBlockVBO();
void assembleDoorBlockVBO();

#define resetMouse() mouse.selectedType = 0; mouse.selectedThing = NULL; mouse.focusedType = 0; mouse.focusedThing = NULL; mouse.brushType = 0; mouse.brushThing = NULL;

#define FPS 60

#define CONSOLE_BUF_CAP 64

#define greenColor 0.0f, 1.0f, 0.0f

#define blackColor 0.0f, 0.0f, 0.0f

#define redColor 1.0f, 0.0f, 0.0f

#define blueColor 0.0f, 0.0f, 1.0f

#define whiteColor 1.0f, 1.0f, 1.0f
#define greyColor 0.5f, 0.5f, 0.5f

#define darkPurple 0.1f, 0.0f, 0.1f
#define yellowColor 1.0f, 1.0f, 0.0f

#define cyan 0.0f, 1.0f, 1.0f

#define game "Doomer engine"
#define texturesFolder "./assets/textures/"
#define objsFolder "./assets/objs/"

#define speed 100.0f

#define bBlockW (1.0f)
#define bBlockD (1.0f)
#define bBlockH (2.0f)

#define floorH bBlockH / 2

#define selBorderD 0.01f
#define selBorderT 0.01f

#define dialogEditorReplicaInputLimit 283

#define dialogEditorNameInputLimit 32
#define dialogEditorAnswerInputLimit 63

#define selTileBorderH 0.001f

float* wallBySide(int* bufSize, Side side, float thick);

#define dialogEditorCharNameTitle "Char name: "


typedef enum{
  editorInstance,
  gameInstance,

  editorMapInstance,
  gameMapInstance,
  
  instancesCounter,
} EngineInstance;

typedef enum{
  render2DFunc,
  render3DFunc,
  preFrameFunc,
  preLoopFunc,
  matsSetup,
  eventFunc,
  onSetFunc,
  mouseVSFunc,
  renderCursorFunc,
  funcsCounter,
} EngineInstanceFunc;

Tile** markersStorage;
int markersStorageSize;
int markersCounterByType[markersCounter];

typedef enum{
    lightSourceShader, hudShader, mainShader, borderShader, screenShader, dirShadowShader, UIShader, UITransfShader, UITxShader, UITransfTx, UITransfColor, animShader, snowShader,shadersCounter
} Shaders;

const char* shadersFileNames[];// = {"lightSource", "hud", "fog", "borderShader","screenShader"};
const char* instancesStr[];

GLuint shadersId[shadersCounter];











const char sdlScancodesToACII[];/* = {
  [4] = 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', [55]='.'
};*/

const char* manipulationModeStr[];/* = { "None","Rotate_X", "Rotate_Y", "Rotate_Z", "Transform_XY", "Transform_Z", "Scale" };*/


// from '!' to 'z' in ASCII
const vec2i englLettersMap[];/* = {
  { 14, 13}, // '!'
  { 13, 13}, // '"'
  { 12, 13}, // '#'
  { 11, 13}, // '$'
  { 10, 13}, // '%'
  { 9, 13}, // '&'
  { 8, 13}, // '''
  { 7, 13}, // '('
  { 6, 13}, // ')'
  { 5, 13}, // '*'
  { 4, 13}, // '+'
  { 3, 13}, // ','
  { 2, 13}, // '-'
  { 1, 13}, // '.'
  { 0, 13}, // '/'

  { 15, 12}, // '0'
  { 14, 12}, // '1'
  { 13, 12}, // '2'
  { 12, 12}, // '3'
  { 11, 12}, // '4'
  { 10, 12}, // '5'
  { 9, 12}, // '6'
  { 8, 12}, // '7'
  { 7, 12}, // '8'
  { 6, 12}, // '9'
  { 5, 12}, // ':'
  { 4, 12}, // ';'
  { 3, 12}, // '<'
  { 2, 12}, // '='
  { 1, 12}, // '>'
  { 0, 12}, // '?'

  { 15, 11}, // '@'
  { 14, 11}, // 'A'
  { 13, 11}, // 'B'
  { 12, 11}, // 'C'
  { 11, 11}, // 'D'
  { 10, 11}, // 'E'
  { 9, 11}, // 'F'
  { 8, 11}, // 'G'
  { 7, 11}, // 'H'
  { 6, 11}, // 'I'
  { 5, 11}, // 'J'
  { 4, 11}, // 'K'
  { 3, 11}, // 'L'
  { 2, 11}, // 'M'
  { 1, 11}, // 'N'
  { 0, 11}, // 'O'

  { 15, 10}, // 'P'
  { 14, 10}, // 'Q'
  { 13, 10}, // 'R'
  { 12, 10}, // 'S'
  { 11, 10}, // 'T'
  { 10, 10}, // 'U'
  { 9, 10}, // 'V'
  { 8, 10}, // 'W'
  { 7, 10}, // 'X'
  { 6, 10}, // 'Y'
  { 5, 10}, // 'Z'
  { 4, 10}, // '['
  { 3, 10}, // '\'
  { 2, 10}, // ']'
  { 1, 10}, // '^'
  { 0, 10}, // '_'

  { 15, 9}, // '`'
  { 14, 9}, // 'a'
  { 13, 9}, // 'b'
  { 12, 9}, // 'c'
  { 11, 9}, // 'd'
  { 10, 9}, // 'e'
  { 9, 9}, // 'f'
  { 8, 9}, // 'g'
  { 7, 9}, // 'h'
  { 6, 9}, // 'i'
  { 5, 9}, // 'j'
  { 4, 9}, // 'k'
  { 3, 9}, // 'l'
  { 2, 9}, // 'm'
  { 1, 9}, // 'n'
  { 0, 9}, // 'o'

  { 15, 8}, // 'p'
  { 14, 8}, // 'q'
  { 13, 8}, // 'r'
  { 12, 8}, // 's'
  { 11, 8}, // 't'
  { 10, 8}, // 'u'
  { 9, 8}, // 'v'
  { 8, 8}, // 'w'
  { 7, 8}, // 'x'
  { 6, 8}, // 'y'
  { 5, 8}, // 'z'
  { 4, 8}, // '{'
  { 3, 8}, // '|'
  { 2, 8}, // '}'
  { 1, 8}, // '~'

};*/

const char* sidesToStr[];// = { [top]= "Top", [bot]="Bot", [right]="Right", [left]"Left"};

void batchGeometry();
void defaultGrid(int gX, int gY, int gZ);

vec3 calculateNormal(vec3 a, vec3 b, vec3 c);

typedef struct{
  float* verts;
  size_t tris;
  size_t size;
  VPair pairs;
} Geometry;

/*
Geometry* geometry;
*/

void attachNormalsToBuf(VPair* VPairBuf, int plane, int bufSize, float* buf);
void createTexture(int* tx,int w,int h, void*px);

#define rgbToGl(r,g,b) r/255.0f, g/255.0f, b/255.0f

void uniformVec3(int shader, char* var, vec3 value);
void uniformFloat(int shader, char* var, float value);
void uniformInt(int shader, char* var, int value);

void assembleBlocks();

char curSaveName[CONSOLE_BUF_CAP];

Menu* curMenu;
Menu dialogViewer;//= { .type = dialogViewerT };
Menu dialogEditor;// = { .type = dialogEditorT };

GLuint fontAtlas;

int curFloor;

//Light* pointlightStorage;
Light* lightStorage[lightsTypeCounter];
int lightStorageSize[lightsTypeCounter];
int lightStorageSizeByType[lightsTypeCounter];

//Light* offLightsStorage[lightsTypeCounter];

Light lightDef[lightsTypeCounter];/* = { .color = rgbToGl(253.0f, 244.0f, 220.0f), .constant = 1.0f, .linear = .09f, .quadratic = .032f, .dir = {0,-1, 0} };*/

Model* curModels;
size_t curModelsSize;

Character* characters;
size_t charactersSize;

Camera camera1;// = { .target={ 0.0f, 0.0f, 0.0f }, .yaw = -90.0f };
//Camera camera2 = { .target={ 0.0f, 0.0f, 0.0f }, .pitch = -14.0f, .yaw = -130.0f, .type=gameCameraT };

BlockInfo wallsVPairs[wallTypeCounter];
BlockInfo blocksVPairs[tileBlocksCounter];

char tempTextInputStorage[512];
int tempTextInputStorageCursor;

Camera* curCamera;// = &camera1;
bool fullScreen;
SDL_Window* window;
Mouse mouse;
float drawDistance;

float windowW;// = 1280.0f;
float windowH;// = 720.0f;

//EnviromentalConfig enviromental = { true, true };

int consoleBufferCursor;
char consoleBuffer[CONSOLE_BUF_CAP];
bool consoleHasResponse;
char consoleResponse[CONSOLE_BUF_CAP * 5];
Menu console;
#define consoleH (1.0f - (1.0f * .05f))

float dofPercent;

Tile**** grid;
int gridX;// = 120;
int gridY;// = 15;
int gridZ;// = 120;

Picture* planeOnCreation;
Uint8* currentKeyStates;// = SDL_GetKeyboardState(NULL);

Texture* loadedTextures1D;
Texture** loadedTextures2D;
char** loadedTexturesNames; // iter same as tex1D
int loadedTexturesCategoryCounter;
Category* loadedTexturesCategories; 
int loadedTexturesCounter;
int longestTextureNameLen;
int longestTextureCategoryLen;

// avaible/loaded models
ModelInfo* loadedModels1D;
ModelInfo** loadedModels2D;
Geometry* modelsBatch; // by tx
ModelInfo** loadedModels2D;

size_t loadedModelsSize;

int* loadedModelsTx;
size_t loadedModelsTxSize; // num of tx loaded for models

GLuint solidColorTx;
GLuint errorTx;
GLuint emptyTx;

VPair hudRect;

float fov;

#define letterW (.04f / 1.9f)
#define letterH (.07f)

#define letterCellW .04f
#define letterCellH .07f

#define objectsMenuWidth (-1.0f + 1.0f / 4.0f)

void uniformMat4(int shader, char* var, float* mat);
void uniformVec2(int shader, char* var, vec2 value);

int renderCapYLayer;

EngineInstance curInstance;

typedef struct {
    vec3i indx;

    uint8_t wallsSize;
    uint8_t* wallsIndexes;

    uint8_t jointsSize;
    uint8_t* jointsIndexes;
} BatchedTile;

BatchedTile* batchedGeometryIndexes;
int batchedGeometryIndexesSize;

Model* playerModel;

void assembleHideWallBlockVBO();

Wall** wallsStorage;
int wallsStorageSize;

//int jointsStorageSize;

TileBlock** blocksStorage;
int blocksStorageSize;

Tile* tilesStorage;
int tilesStorageSize;

Picture* picturesStorage;
int picturesStorageSize;

size_t* geomentyByTxCounter;

void allocateGrid(int gX, int gY, int gZ);


typedef struct{
  float* buf;
  size_t size;
} Geom;

typedef struct{
    float* buf;
    size_t size;
    
    int VAO;
    int VBO;
    int tris;
} GeomFin;

GeomFin* finalGeom;

void batchAllGeometry();
void batchAllGeometryNoHidden();
VPair planePairs;

bool showHiddenWalls;

void assembleNavigation();
void assembleHalfWallBlockVBO();

float* createNormalBuffer(float* buf, int size, int* finalSize);

Matrix hardWallMatrices[4];

typedef struct TextInput2 TextInput2;

typedef struct{
  vec2 pos[6];
  uint8_t c[4];

  char* text; // assotiative text
  vec2 textPos;

  void (*onClick)(void);
  char* onclickResText;

  MeshBuffer* highlight;
  TextInput2* input;

  //  int charLimit;
  //  char* input;

  vec2 lb;
  vec2 rt;
} UIRect2;

struct TextInput2 {
    //  vec4 rect;
    int limit;
    char* buf;

    UIRect2* relatedUIRect;
};

typedef enum{
    saveWindowT, loadWindowT, attachSaveWindowT, markersListWindowT, entityWindowT, UIStructsCounter
} UIStruct;

typedef struct{
  GLuint VBO;
  GLuint VAO;

  //  UIStruct UIName;

  int VBOsize;
  
  int rectsSize;
  UIRect2* rects;
} UIBuf;


UIBuf* UIStructBufs[UIStructsCounter];

void uniformVec4(int shader, char* var, vec4 value);

void doorFrameHighlighting();
void modelHighlighting();
void noHighlighting();

const void(*stancilHighlight[mouseSelectionTypesCounter])();
unsigned int depthMaps;

void bindUIQuad(vec2 pos[6], uint8_t c[4], MeshBuffer* buf);
void bindUIQuadTx(vec4 pos[6], MeshBuffer* buf);
void bindUITri(vec2 pos[3], uint8_t c[4], MeshBuffer* buf);


//void clearCurrentUI();
void saveMapUI();
void loadMapUI();

UIBuf curUIBuf;

TextInput* selectedTextInput;
TextInput2* selectedTextInput2;

MeshBuffer textInputCursorBuf;
vec2 textInputCursorMat;
int inputCursorPos;

UIBuf* batchUI(UIRect2* rects, int rectsSize);
void clearCurrentUI();

VPair markersBufs[markersCounter];

typedef struct{
    vec3 center;
} CollisionSquare;

typedef enum{
    acceptedLayerT, deniedLayerT, layersCounter
} CollisionLayers;

CollisionSquare* acceptedLayers;
CollisionSquare* deniedLayers;
int collisionLayersSize[layersCounter];

MeshBuffer navigationTilesMesh[layersCounter];

uint8_t*** collisionGrid;
void allocateCollisionGrid(int gX, int gY, int gZ);

void generateNavTiles();

typedef struct{
    vec3 rt;
    vec3 lb;
} AABB;

AABB* acceptedCollisionTilesAABB;

int selectedCollisionTileIndex;
MeshBuffer selectedCollisionTileBuf;

typedef struct{
    vec2 rt;
    vec2 lb;

    int h;
    
    bool f;
} CollisionTile;


Matrix markersMats[markersCounter];

typedef enum{
    playerEntityT, entityTypesCounter
} EntityType;

Matrix entitiesMats[entityTypesCounter];

typedef struct{
    Matrix mat;
    EntityType type;

    vec3 dir;

    vec3* path;
    int pathSize;
    int curPath;
    int frame;
} Entity;

Entity* entityStorage[entityTypesCounter];
int entityStorageSize[entityTypesCounter];

const char* entityTypeStr[];

void batchEntitiesBoxes();
MeshBuffer entitiesBatch[entityTypesCounter];

typedef struct{
    float g; float h; float f; int parZ; int parX;
} AstarCell;

typedef struct{
    float f; int z; int x;
} AstarOpenCell;

MeshBuffer lastFindedPath;


void loadFBXModel(char* name);

typedef struct{
    MeshBuffer mesh;
    char* name;
    float* buf;

} ModelInfo2;

BoneInfo* bones;
int bonesSize;

ModelInfo2* modelInfo2;

typedef struct{
    vec3 pos;
    vec2 uv;
    vec3 norm;
    vec4i bonesId;
    vec4 weights;
} ModelAttr;

typedef struct{
    uint8_t x,y,z,w;
} vec4i_8u;

typedef struct{
    int index;
    
    int boneId;
    float time;
    
    int action;
    int interp;
    
    vec4 value;

    //  vec3 trans;
//    vec4 rot;
    
//    Matrix inversedMat;
    
    //  Matrix matrix;
} BoneAction;

BoneAction* boneActions;
int timesCounter;
BoneAction*** boneAnimIndexed;
int* boneAnimIndexedSize;

//int* timesIndexesCounter[]; - for dif anims


int sortBonesByTime(BoneAction* a, BoneAction* b);

void loadGLTFModel(char* name);

void traverseBones(int jointIndex);
void updateChildBonesMats(int jointIndex);

Matrix* inversedMats;

MeshBuffer snowTilesMesh[layersCounter];
MeshBuffer snowMesh;
vec3* snowParticles;
int snowParticlesSize;

bool** snowGrid;
bool snowAreas;

void generateShowAreas();

int txOfGround;
