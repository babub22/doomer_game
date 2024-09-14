#include "deps.h"
#include "linearAlg.h"
#include "main.h"
#include "editor.h"
#include "game.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

int* tempModelIdxs;
int tempAABBSize;

vec3* tempModelsPos;
int tempModelsPosSize;

uint32_t skyboxVAO, skyboxVBO, cubemapTexture;

bool collisionToDraw = false;
bool hideScene = false;

MeshBuffer AABBBoxes;
MeshBuffer cylinderMesh;

MeshBuffer linesVisualiser;
float* linesVisualiserBuf;
int linesVisualiserBufSize;

MeshBuffer triInterVisualiser;
float* triInterVisualiserBuf;
int triInterVisualiserBufSize;

MeshBuffer triVisualiser;
float* triVisualiserBuf;
int triVisualiserBufSize;

MeshBuffer lines2Visualiser;
float* lines2VisualiserBuf;
int lines2VisualiserBufSize;

MeshBuffer d2GridTri;
MeshBuffer d2GridPlayerPoint;
Matrix gridTriMat;

uint32_t lowResFBO;
uint32_t lowResTx;

int fakeWinW = 1920;//854;//1920;//854;
int fakeWinH = 1080;//480;//1080;// 480;

float radius = 40.0f;

bool showHiddenWalls = true;

const char* shaderVarSufixStr[] = {
  [cgltf_light_type_point] = "point",
  [cgltf_light_type_directional] = "dir",
  //	    [dirLightShadowT] = "dirShadow"
};

TextInput* selectedTextInput;
TextInput2* selectedTextInput2;

MeshBuffer textInputCursorBuf;
vec2 textInputCursorMat;
int inputCursorPos;

UIBuf curUIBuf;

Matrix hardWallMatrices[4];

int renderCapYLayer;
EngineInstance curInstance = editorInstance;

bool navPointsDraw = false;

Wall** wallsStorage;
int wallsStorageSize;

TileBlock** blocksStorage;
int blocksStorageSize;

Tile* tilesStorage;
int tilesStorageSize;

float elapsedMs = 0;

MeshBuffer doorDoorPlane;

const int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;  
unsigned int depthMapFBO;
unsigned int depthMaps;

void glErrorCheck(){
  GLenum er = glGetError();

  if(er != GL_NO_ERROR){
    printf("\nEr: %d %d\n\n", __LINE__, er);
  }
}

float near_plane;
float far_plane;

AABB curSceneAABB;

const void(*instances[instancesCounter][funcsCounter])() = {
  [editorInstance] = {
    [render2DFunc] = editor2dRender,
    [render3DFunc] = editor3dRender,
    [preLoopFunc] = editorPreLoop,
    [preFrameFunc] = editorPreFrame,
    [matsSetup] = editorMatsSetup,
    [eventFunc] = editorEvents,
    [onSetFunc] = editorOnSetInstance,
    [mouseVSFunc] = editorMouseVS,
    [renderCursorFunc] = editorRenderCursor
  },
  [gameInstance] = {
    [render2DFunc] = game2dRender,
    [render3DFunc] = game3dRender,
    [preLoopFunc] = gamePreLoop,
    [preFrameFunc] = gamePreFrame,
    [matsSetup] = gameMatsSetup,
    [eventFunc] = gameEvents,
    [onSetFunc] = gameOnSetInstance,
    [mouseVSFunc] = gameMouseVS,
    [renderCursorFunc] = gameRenderCursor
  },
};

// ~~~~~~~~~~~~~~~~~
const char* instancesStr[] = { [editorInstance]="Editor", [gameInstance]="Game" };

const char* markersStr[] = { [locationExitMarkerT]="Exit marker" };

const char* lightTypesStr[] = { [pointLightT] = "PointLight", [dirLightShadowT] = "DirLight(shadow)", [dirLightT] = "DirLight" };

const char* mouseBrushTypeStr[] = {
  [mouseModelBrushT] = "Model",
  [mouseWallBrushT] = "Wall",
  [mouseTextureBrushT] = "Texture",
  [mouseTileBrushT] = "Tile",
  [mouseBlockBrushT] = "Block",
  [mouseMarkerBrushT] = "Marker",
  [mouseEntityBrushT] = "Entity",
};

const int GLTFmapSize[] = { [cgltf_type_vec3] = 3,[cgltf_type_vec4] = 4,[cgltf_type_vec2] = 2 };

const int GLTFtypeSize[] = {
  [cgltf_component_type_r_32f] = sizeof(float),
  [cgltf_component_type_r_16u] = sizeof(uint16_t),
  [cgltf_component_type_r_8u] = sizeof(uint8_t) };

const int GLTFattrPad[] = {
  [cgltf_attribute_type_position] = 0, [cgltf_attribute_type_normal] = 5,
  [cgltf_attribute_type_texcoord] = 3, [cgltf_attribute_type_joints] = 8,
  [cgltf_attribute_type_weights] = 12
};

const char* manipulationModeStr[] = { "None","Rotate_X", "Rotate_Y", "Rotate_Z", "Transform_XY", "Transform_Z", "Scale" };

const char* entityTypeStr[] = { [playerEntityT] = "Player entity" };

ModelsTypesInfo modelsTypesInfo[] = {
  [objectModelType] = {"Obj",0},
  [playerModelT] = {"Player", 0}
};

const char* shadersFileNames[] = { "lightSource", "hud", "fog", "borderShader", "screenShader", [dirShadowShader] = "dirShadowDepth", [UIShader] = "UI", [UITransfShader] = "UITransf", [UITransfTx] = "UITransfTx", [animShader] = "animModels", [snowShader] = "snowShader", [skyboxShader] = "skybox",[waterShader] = "waterShader", [windowShader] = "windowShader" };

const char sdlScancodesToACII[] = {
  [4] = 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',[55] = '.'
};

const vec2i englLettersMap[] = {
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

};

Camera* curCamera = &camera1;

bool fullScreen = 0;

MeshBuffer dirPointerLine;

Light lightDef[lightsTypeCounter] = {
  [pointLightT] = {.r = 253, .g=244, .b=220, .curLightPresetIndex = 11},
  [dirLightShadowT] = {.r = 253, .g=244, .b=220, .curLightPresetIndex = 11},
  [dirLightT] = {.r = 253, .g=244, .b=220, .curLightPresetIndex = 11}
};

Menu dialogViewer = { .type = dialogViewerT };
Menu dialogEditor = { .type = dialogEditorT };


float windowW = 1920.0f;
float windowH = 1080.0f;

float dofPercent = 1.0f;

// ~~~~~~~~~~~~~~~~~

GLuint textVBO;
GLuint textVAO;

unsigned int screenTexture;
unsigned int textureColorBufferMultiSampled;

VPair quad;

VPair tileOver;

GLuint fbo;
GLuint intermediateFBO;

GLuint selectionRectVBO;
GLuint selectionRectVAO;

VPair hudRect;

VPair planePairs;

TextInput dialogEditorNameInput;

Particle* snowParticle;
int snowAmount;
float snowSpeed;

VPair cube;

GLuint objectsMenuTypeRectVBO;
GLuint objectsMenuTypeRectVAO;

int main(int argc, char* argv[]) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

  SDL_DisplayMode DM;
  SDL_GetCurrentDisplayMode(0, &DM);

  SDL_Rect r1;

  SDL_GetDisplayUsableBounds(0, &r1);

  windowW = DM.w;
  windowH = DM.h;

  fakeWinW = DM.w;
  fakeWinH = DM.h;

  char windowTitle[100] = game;
  window = SDL_CreateWindow(windowTitle,
			    SDL_WINDOWPOS_CENTERED,
			    SDL_WINDOWPOS_CENTERED,
			    windowW, windowH,
			    SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
			    );

  SDL_WarpMouseInWindow(window, windowW / 2.0f, windowH / 2.0f);
  SDL_SetRelativeMouseMode(SDL_TRUE);

  //  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);


  SDL_GLContext context = SDL_GL_CreateContext(window);

  SDL_ShowCursor(SDL_DISABLE);

  glewInit();

  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);


  // cursor buffers
  {
    glGenBuffers(1, &cursor.VBO);
    glGenVertexArrays(1, &cursor.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  {
    glGenBuffers(1, &lines2Visualiser.VBO);
    glGenVertexArrays(1, &lines2Visualiser.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  {
    glGenBuffers(1, &triVisualiser.VBO);
    glGenVertexArrays(1, &triVisualiser.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  {
    glGenBuffers(1, &linesVisualiser.VBO);
    glGenVertexArrays(1, &linesVisualiser.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  {
    glGenBuffers(1, &triInterVisualiser.VBO);
    glGenVertexArrays(1, &triInterVisualiser.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  // 2d free rect
  {
    glGenBuffers(1, &dirPointerLine.VBO);
    glGenVertexArrays(1, &dirPointerLine.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  // plane 3d
  {
    glGenBuffers(1, &planePairs.VBO);
    glGenVertexArrays(1, &planePairs.VAO);
    
    planePairs.attrSize = 8;

    float plane[] = {
      bBlockW, bBlockD, 0.0f , 0.0f, 1.0f, -0.000000, 1.000000, -0.000000,
      0.0f, bBlockD, 0.0f , 1.0f, 1.0f, -0.000000, 1.000000, -0.000000, 
      bBlockW, 0.0f, 0.0f , 0.0f, 0.0f, -0.000000, 1.000000, -0.000000,
      
      0.0f, bBlockD, 0.0f , 1.0f, 1.0f, 0.000000, 1.000000, 0.000000,
      bBlockW, 0.0f, 0.0f , 0.0f, 0.0f, 0.000000, 1.000000, 0.000000,
      0.0f, 0.0f, 0.0f , 1.0f, 0.0f,  0.000000, 1.000000, 0.000000,
    };

    planePairs.vBuf = malloc(sizeof(plane));
    memcpy(planePairs.vBuf,plane,sizeof(plane));
    
    planePairs.vertexNum = 6;

    glBindVertexArray(planePairs.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, planePairs.VBO);

    // printf("alloced size - %d  used size - %d \n", preGeom[i].size, txLastIndex[i] * sizeof(float));
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane), planePairs.vBuf, GL_STATIC_DRAW);
  
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  // cursor buffers
  {
    glGenBuffers(1, &selectionRectVBO);
    glGenVertexArrays(1, &selectionRectVAO);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  // init VBO and VAO for text
  {
    glGenBuffers(1, &textVBO);
    glGenVertexArrays(1, &textVAO);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  {
    float quadVertices[] = {
      -1.0f,  1.0f,  0.0f, 1.0f,
      -1.0f, -1.0f,  0.0f, 0.0f,
      1.0f, -1.0f,  1.0f, 0.0f,

      -1.0f,  1.0f,  0.0f, 1.0f,
      1.0f, -1.0f,  1.0f, 0.0f,
      1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quad.VAO);
    glGenBuffers(1, &quad.VBO);

    glBindVertexArray(quad.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, quad.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
  }

    
  {
    mirrorProj = perspective(rad(45.0f), 1.0f, 0.01f, 1000.0f);
    //mirrorProj = orthogonal(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
  }

  loadShaders();

  uniformFloat(mainShader, "screenW", fakeWinW);
  uniformFloat(mainShader, "screenH", fakeWinH);
    
  vec3 fogColor = { 0.5f, 0.5f, 0.5f };
  glClearColor(argVec3(fogColor), 1.0f);

  // init opengl
  {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glDepthFunc(GL_LEQUAL);
    //	glDepthFunc(GL_EQUAL);
    //	glDepthFunc(GL_GREATER);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_CLAMP);
    //	glEnable(GL_CULL_FACE);
    //	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //	glDepthRange(-1.0f, 1.0f);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

    
  // fbo rbo multisample things
  {
    // intermediateFBO
    {
      glGenFramebuffers(1, &intermediateFBO);
      glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);

      glGenTextures(1, &screenTexture);
      glBindTexture(GL_TEXTURE_2D, screenTexture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fakeWinW, fakeWinH, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
      //	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      //	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	    
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	    
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      printf("intermediateFBO creation failed! With %d \n", glCheckFramebufferStatus(GL_FRAMEBUFFER));
      exit(0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  loadGLTFScene("./assets/base.gltf");

  loadGLTFModel("./assets/objs/Doomer.gltf");
    
  glGenBuffers(1, &AABBBoxes.VBO);
  glGenVertexArrays(1, &AABBBoxes.VAO);
  
  loadCubemap();
  
  // init cylinders buffers
  {
    glGenBuffers(1, &cylinderMesh.VBO);
    glGenVertexArrays(1, &cylinderMesh.VAO);

  }

  if(false){
    int segments = 7;
    float* cylinder = malloc((segments+1) * 6 * sizeof(float));
    
    float angleStep = 2.0f * M_PI / segments;

    int index = 0;

    float height = 1.5f;

    float radius = 0.3f;
    
    // Top circle vertices
    for (int i = 0; i < segments; ++i) {
      float angle = i * angleStep;
      float x = radius * cos(angle);
      float z = radius * sin(angle);
        
      // Top circle vertex
      cylinder[index] = x;
      cylinder[index+1] = height / 2.0f;
      cylinder[index+2] = z;
        
      // Bottom circle vertex
      cylinder[index+3] = x;
      cylinder[index+4] = -height / 2.0f;
      cylinder[index+5] = z;
      
      index+=6;
    }

    float x = radius * cos(0);
    float z = radius * sin(0);

    cylinder[index] = x;
    cylinder[index+1] = height / 2.0f;
    cylinder[index+2] = z;
    
    cylinder[index+3] = x;
    cylinder[index+4] = -height / 2.0f;
    cylinder[index+5] = z;

    glBindVertexArray(cylinderMesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, cylinderMesh.VBO);

    cylinderMesh.VBOsize = (segments + 1) * 2;

    glBufferData(GL_ARRAY_BUFFER, (segments+1) * 6 * sizeof(float), cylinder, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    free(cylinder);
  }

  // place player
  {
    Entity* entity = calloc(1,sizeof(Entity));
    entity->type = playerEntityT;

    // redo with specs
    entity->model = calloc(1,sizeof(ModelData));
    entity->model->data = &modelsData[0];

    entity->model->nodes = malloc(sizeof(GLTFNode)* entity->model->data->nodesSize);
    entity->model->tempTransforms = malloc(sizeof(float)* 10 * entity->model->data->nodesSize);
    
    memcpy(entity->model->nodes,
	   entity->model->data->nodes,
	   sizeof(GLTFNode)* entity->model->data->nodesSize);

    entity->model->jointsMats = malloc(sizeof(Matrix)*entity->model->data->jointsIdxsSize);
    
    entity->mat = IDENTITY_MATRIX;
	
    entity->mat.m[12] = .0f;
    entity->mat.m[13] = 5.0f;
    entity->mat.m[14] = .0f;

    vec3 pos;
    float entityR = max(entity->model->data->size[0], entity->model->data->size[2]) / 2.0f;
    float entityH = entity->model->data->size[1];
    
    entityVsMeshes((vec3) { entity->mat.m[12], entity->mat.m[13], entity->mat.m[14] }, 50.0f, entityR, entityH, &pos);

    entity->mat.m[12] = pos.x;
    entity->mat.m[13] = pos.y;
    entity->mat.m[14] = pos.z;
    
    entity->dir = (vec3){ .0f, .0f, 1.0f};

    entityStorageSize[entity->type]++;

    if(!entityStorage[entity->type]){
      entityStorage[entity->type] = malloc(sizeof(Entity));
    }

    memcpy(&entityStorage[entity->type][entityStorageSize[entity->type]-1], entity, sizeof(Entity));
  }

  bindObjectsAABBBoxes();
  bindCylindersAroundEntities();

  {
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMaps);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthMaps);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32, SHADOW_WIDTH, SHADOW_HEIGHT, 8, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMaps, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      printf("main fbo creation failed! With %d \n", glCheckFramebufferStatus(GL_FRAMEBUFFER));
      exit(0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  {
    glGenTextures(1, &fontAtlas);

    // -1 because of solidColorTx
    SDL_Surface* texture = IMG_Load_And_Flip_Vertical("./iosevka-bold.png");

    if (!texture) {
      printf("Loading of texture font.png\" failed");
      exit(0);
    }

    glBindTexture(GL_TEXTURE_2D, fontAtlas);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->w,
		 texture->h, 0, GL_RGBA,
		 GL_UNSIGNED_BYTE, texture->pixels);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
      printf("OpenGL error: %d\n", err);
      // Handle OpenGL error




      // Optionally return or perform other error handling
    }

    SDL_FreeSurface(texture);

    glBindTexture(GL_TEXTURE_2D, 0);
  }

  // tang of fov calculations
  fov = editorFOV;
  // tangFOV = tanf(rad(fov) * 0.5);

  mouse = (Mouse){ .interDist = 1000.0f };

  float dist = sqrt(1 / 3.0);
  bool cameraMode = true;

  float testFOV = editorFOV;

  // load or init grid
  //  if (!loadSave(".")) {
  
  //    allocateGrid(120, 8, 120);
  //    allocateCollisionGrid(120, 8, 120);
    
  //    defaultGrid(gridX, gridY, gridZ);
  //  }

  //    lightPos = (vec3){gridX /2.0f,2.0f,gridZ/2.0f};

  //  renderCapYLayer = gridY;
  //  batchGeometry();
    
  //   batchModels();

  // set up camera
  //    GLint cameraPos = glGetUniformLocation(shadersId[mainShader], "cameraPos");
  {
    camera1.yaw = 0.0f;
    camera1.pitch = 0.0f;
    camera1.pos = (vec3){ .0f,3.0f,.0f };//(vec3)xyz_indexesToCoords(gridX / 2, 10, gridZ / 2);
    //    camera1.pos = (vec3)xyz_indexesToCoords(29, 14, 56);
    //    camera1.pos = (vec3){35, 16, 63};
    //camera2.pos = (vec3)xyz_indexesToCoords(gridX/2, 2, gridZ/2);
    camera1.up = (vec3){ 0.0f, 1.0f, 0.0f };
    //  camera2.up = (vec3){ 0.0f, 1.0f, 0.0f };
  }

  // set draw distance to gridX/2
  /*    GLint radius = glGetUniformLocation(shadersId[mainShader], "radius");
	drawDistance = 10;
	glUniform1f(radius, drawDistance);*/

  ManipulationMode manipulationMode = 0;
  float manipulationStep = 0.01f;
  float manipulationScaleStep = 5 * 0.01f + 1;

  // show show

  //    initSnowParticles();

  const float entityH = 0.17f;
  const float entityW = 0.1f / 2.0f;
  const float entityD = entityH / 6;

  vec3 initPos = { 0.3f + 0.1f / 2, 0.0f, 0.3f + 0.1f / 2 }; //+ 0.1f/2 - (entityD * 0.75f)/2 };

  bool quit = false;

  //  clock_t lastFrame = clock();
  //    float deltaTime;

  float cameraSpeed = speed;
  SDL_Event event;

  //  for(int i=0;i<instancesCounter;i++){
  ((void (*)(void))instances[curInstance][preLoopFunc])();
  //  }

  near_plane = 0.01f;
  far_plane  = 120.0f;
  
  uniformFloat(dirShadowShader, "far_plane", far_plane);

  uniformInt(snowShader, "colorMap", 0); 
    
  uniformInt(snowShader, "colorMap", 0); 
  uniformInt(snowShader, "shadowMap", 1);
  uniformFloat(snowShader, "far_plane", far_plane);
  
  uniformInt(mainShader, "colorMap", 0); 
  uniformInt(mainShader, "shadowMap", 1);
  uniformFloat(mainShader, "far_plane", far_plane);

  uniformInt(animShader, "colorMap", 0); 
  uniformInt(animShader, "shadowMap", 1);
  uniformFloat(animShader, "far_plane", far_plane);

  uniformInt(waterShader, "colorMap", 0); 
  uniformInt(waterShader, "shadowMap", 1);
  uniformFloat(waterShader, "far_plane", far_plane);

  uniformInt(skyboxShader, "skybox", 0); 


  while (!quit) {
    uint64_t start_time = SDL_GetPerformanceCounter();

    glErrorCheck();

    currentKeyStates = SDL_GetKeyboardState(NULL);

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
	quit = true;
      }

      if (event.type == SDL_KEYDOWN) {
	if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
	  exit(1);
	}

	if(selectedTextInput2){
	  if(!selectedTextInput2->buf){
	    selectedTextInput2->buf = calloc(1, sizeof(char) * (selectedTextInput2->limit + 1));
	  }
      
	  if(selectedTextInput2->relatedUIRect && selectedTextInput2->relatedUIRect->onclickResText){
	    free(selectedTextInput2->relatedUIRect->onclickResText);
	    selectedTextInput2->relatedUIRect->onclickResText =NULL;
	  }
      
	  int strLen = selectedTextInput2->buf ? strlen(selectedTextInput2->buf) : 0; 

	  if(event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE && inputCursorPos > 0) {
	    selectedTextInput2->buf[inputCursorPos-1] = 0;
	  
	    textInputCursorMat.x -= letterW;
	    inputCursorPos--;
	  }else if(strLen < selectedTextInput2->limit){
	    if(event.key.keysym.scancode >= 4 && event.key.keysym.scancode <= 39){
	      char newChar = sdlScancodesToACII[event.key.keysym.scancode];
	      selectedTextInput2->buf[inputCursorPos] = newChar;

	      textInputCursorMat.x += letterW;
	      inputCursorPos++;
	    }
	  }
	}

	if(event.key.keysym.scancode == SDL_SCANCODE_F1){
	  loadGLTFScene("./assets/base.gltf");
	  bindObjectsAABBBoxes();
	  printf("Map reloaded\n");
	}

	if(event.key.keysym.scancode == SDL_SCANCODE_F4){
	  collisionToDraw = !collisionToDraw;
	}

	if(event.key.keysym.scancode == SDL_SCANCODE_F10){
	  hideScene = !hideScene;
	}

	if(event.key.keysym.scancode == SDL_SCANCODE_P){
	  radius+=0.1f;
	  uniformFloat(mainShader, "radius", radius);
	  uniformFloat(waterShader, "radius", radius);
	  uniformFloat(skyboxShader, "radius", radius);
	}else if(event.key.keysym.scancode == SDL_SCANCODE_O){
	  radius-=0.1f;
	  uniformFloat(mainShader, "radius", radius);
	  uniformFloat(waterShader, "radius", radius);
	  uniformFloat(skyboxShader, "radius", radius);
	}

	if(event.key.keysym.scancode == SDL_SCANCODE_F2){
	  reloadShaders();

	  uniformFloat(mainShader, "screenW", fakeWinW);
	  uniformFloat(mainShader, "screenH", fakeWinH);

	  uniformFloat(waterShader, "screenW", fakeWinW);
	  uniformFloat(waterShader, "screenH", fakeWinH);

	  uniformFloat(mainShader, "radius", radius);
	  uniformFloat(waterShader, "radius", radius);
	  uniformFloat(skyboxShader, "radius", radius);
		    
	  printf("Shaders reloaded\n");
	}

	if (false && event.key.keysym.scancode == SDL_SCANCODE_F3) {
	  if(navPointsDraw){
	    navPointsDraw = !navPointsDraw;
	  }else{
	    //			generateNavTiles();
		
	    navPointsDraw = true;
	  }
	}

	if (event.key.keysym.scancode == SDL_SCANCODE_SPACE && !selectedTextInput2) {
		    
	  
	  if (curInstance >= gameInstance) {
	    curInstance = 0;
	  }
	  else {
	    curInstance++;
	  }
                     
	  ((void (*)(void))instances[curInstance][onSetFunc])();
	}
      }

      
      ((void (*)(SDL_Event))instances[curInstance][eventFunc])(event);
    }

    ((void (*)(int))instances[curInstance][mouseVSFunc])(mainShader);    
    ((void (*)(float))instances[curInstance][preFrameFunc])(elapsedMs);
    
    //  if (lightStorage)
    {
      //	    glViewport(0, 0, 640, 360);
      glBindFramebuffer(GL_FRAMEBUFFER, fbo);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
      glEnable(GL_DEPTH_TEST);

	    
      static uint8_t padTable[] = {
	[cgltf_animation_path_type_rotation] = 6,
	[cgltf_animation_path_type_translation]=0,
	[cgltf_animation_path_type_scale]= 3
      };

      static uint8_t sizeTable[] = {
	[cgltf_animation_path_type_rotation] = 4,
	[cgltf_animation_path_type_translation]=3,
	[cgltf_animation_path_type_scale]= 3
      };

      // test anim
      if(entityStorageSize[playerEntityT] != 0){
	bool changeStage = false;
		
	int curAnim = entityStorage[playerEntityT][0].model->curAnim;
	//int curStage = entityStorage[playerEntityT][0].model->curStage;

	/*
	  if(entityStorage[playerEntityT][0].model->mirrored){
	  int lastStage = entityStorage[playerEntityT]->model->data->stage[curAnim] - 1;
		    
	  curStage = lastStage - curStage;
	  curTime = entityStorage[playerEntityT]->model->data->stageTime[curAnim][lastStage] - curTime;
	  }*/

	if(curAnim != entityStorage[playerEntityT][0].model->nextAnim){
	  if(entityStorage[playerEntityT]->model->blendFactor == 0){
	    // or apply do data->nodes cur samplers and blend from them to safe memory
	    for(int i=0;i<entityStorage[playerEntityT][0].model->data->nodesSize;i++){
	      memcpy(entityStorage[playerEntityT][0].model->tempTransforms + (i * 10),
		     entityStorage[playerEntityT][0].model->nodes[i].t,
		     sizeof(float) * 10);
	    }
	  }
		    
	  // then blend it
	  int nextAnim = entityStorage[playerEntityT][0].model->nextAnim;
	  float blendFactor = entityStorage[playerEntityT]->model->blendFactor/10.0f;
	  int sampler = 0;

	  for(int i=0;i<entityStorage[playerEntityT]->model->data->channelsSize[nextAnim];i++){
	    int16_t path = entityStorage[playerEntityT]->model->data->channels[nextAnim][i].path;
	    int16_t nodeId = entityStorage[playerEntityT]->model->data->channels[curAnim][i].nodeId;

	    if(path == cgltf_animation_path_type_rotation){
	      vec4 rotCur;
	      vec4 rotNext;
				
	      memcpy(&rotNext, entityStorage[playerEntityT]->model->data->channels[nextAnim][i].samples[sampler].data, sizeof(vec4));
	      memcpy(&rotCur, &entityStorage[playerEntityT]->model->tempTransforms[(nodeId * 10) + padTable[path]], sizeof(vec4));

	      vec4 inter = slerp(rotCur, rotNext, blendFactor);

	      memcpy(entityStorage[playerEntityT]->model->nodes[nodeId].t+padTable[path], &inter, sizeof(vec4));
	    }else{
	      for(int i2=0;i2<sizeTable[path];i2++){
		float valCur = entityStorage[playerEntityT]->model->tempTransforms[(nodeId * 10) + padTable[path]+i2];
		float valNext = entityStorage[playerEntityT]->model->data->channels[nextAnim][i].samples[sampler].data[i2];
			    
		entityStorage[playerEntityT]->model->nodes[nodeId].t[padTable[path]+i2] =
		  (1.0f - blendFactor) * valCur + valNext * blendFactor;
	      }
	    }
	  }

	  if(blendFactor == 1.0f){
	    entityStorage[playerEntityT][0].model->blendFactor = 0;
	    entityStorage[playerEntityT][0].model->curAnim = entityStorage[playerEntityT][0].model->nextAnim;
	    //entityStorage[playerEntityT][0].model->curStage = 0;
	  }else{
	    entityStorage[playerEntityT][0].model->blendFactor++;
	  }

	  //		    entityStorage[playerEntityT][0].model->curAnim = entityStorage[playerEntityT][0].model->nextAnim;
	}else {
	  float t = entityStorage[playerEntityT][0].model->time;
		
	  if(entityStorage[playerEntityT][0].model->mirrored){
	    float finalTime = entityStorage[playerEntityT]->model->data->channels[curAnim][0].samples
	      [entityStorage[playerEntityT]->model->data->channels[curAnim][0].samplesSize-1].time;
	    t = finalTime - t;
	  }
		    
	  for(int i=0;i<entityStorage[playerEntityT]->model->data->channelsSize[curAnim];i++){
	    int sampler = -1;
	    float curT;
	    float nextT;

	    /*			if(entityStorage[playerEntityT][0].model->mirrored){
				for(int i2=1;i2<entityStorage[playerEntityT]->model->data->channels[curAnim][i].samplesSize; i2++) {
				float sT = entityStorage[playerEntityT]->model->data->channels[curAnim][i].samples[i2].time;
				float prevST = entityStorage[playerEntityT]->model->data->channels[curAnim][i].samples[i2-1].time;
			    
				if(t >= prevST && t <= sT){
				sampler = i2;
				break;
				}
				}2
			    
				curT = entityStorage[playerEntityT]->model->data->channels[curAnim][i].samples[sampler].time;
				nextT = entityStorage[playerEntityT]->model->data->channels[curAnim][i].samples[sampler-1].time;
				}else{*/
	    for(int i2=0;i2<entityStorage[playerEntityT]->model->data->channels[curAnim][i].samplesSize - 1; i2++) {
	      float sT = entityStorage[playerEntityT]->model->data->channels[curAnim][i].samples[i2].time;
	      float nestST = entityStorage[playerEntityT]->model->data->channels[curAnim][i].samples[i2+1].time;
			    
	      if(t >= sT && t <= nestST){
		sampler = i2;
		break;
	      }
	    }
			    
	    curT = entityStorage[playerEntityT]->model->data->channels[curAnim][i].samples[sampler].time;
	    nextT = entityStorage[playerEntityT]->model->data->channels[curAnim][i].samples[sampler+1].time;
	    //			}
			
	    int16_t interpolation = entityStorage[playerEntityT]->model->data->channels[curAnim][i].interpolation;
	    int16_t path = entityStorage[playerEntityT]->model->data->channels[curAnim][i].path;
	    int16_t nodeId = entityStorage[playerEntityT]->model->data->channels[curAnim][i].nodeId;
				
	    float tFactor = (t-curT)/(nextT-curT);

	    if(interpolation == cgltf_interpolation_type_cubic_spline) {
	      printf("Cubic usupported\n");
	      exit(-1);
	    }
			
	    if(interpolation == cgltf_interpolation_type_step){
	      memcpy(entityStorage[playerEntityT]->model->nodes[nodeId].t + padTable[path],
		     entityStorage[playerEntityT]->model->data->channels[curAnim][i].samples[sampler].data,
		     sizeof(float) * sizeTable[path]);
	    }else{
	      if(path == cgltf_animation_path_type_rotation){
		if(interpolation == cgltf_interpolation_type_linear){
		  vec4 rotCur;
		  vec4 rotNext;
				
		  memcpy(&rotNext, entityStorage[playerEntityT]->model->data->channels[curAnim][i].samples[sampler+1].data, sizeof(vec4));
		  memcpy(&rotCur, entityStorage[playerEntityT]->model->nodes[nodeId].t + padTable[path], sizeof(vec4));

		  vec4 inter = slerp(rotCur, rotNext, tFactor);

		  memcpy(entityStorage[playerEntityT]->model->nodes[nodeId].t+padTable[path],&inter, sizeof(vec4));
		}
	      }else if(interpolation == cgltf_interpolation_type_linear){
				
		for(int i2=0;i2<sizeTable[path];i2++){
		  float valCur = entityStorage[playerEntityT]->model->nodes[nodeId].t[padTable[path]+i2];
		  float valNext = entityStorage[playerEntityT]->model->data->channels[curAnim][i].samples[sampler+1].data[i2];
			    
		  entityStorage[playerEntityT]->model->nodes[nodeId].t[padTable[path]+i2] =
		    (1.0f - tFactor) * valCur + valNext * tFactor;
		}
	      }
	    }
	  }

	  //		    entityStorage[playerEntityT][0].model->time += elapsedMs;
	  uint32_t time = SDL_GetTicks();
	  float dT = (time - entityStorage[playerEntityT][0].model->lastUpdate) / 1000.0f;
	  entityStorage[playerEntityT][0].model->time += dT;
	  entityStorage[playerEntityT][0].model->lastUpdate = time;
	}
		    
	updateNodes(entityStorage[playerEntityT]->model->data->rootNode, -1,
		    &entityStorage[playerEntityT]->model->nodes);

	//		glUseProgram(shadersId[animShader]);
		    
	char buf[64];
		    
	for(int i=0;i< entityStorage[playerEntityT]->model->data->jointsIdxsSize;i++){
	  int index = entityStorage[playerEntityT]->model->data->jointsIdxs[i];

	  if(false && index == entityStorage[playerEntityT][0].model->data->neckNode){
	    vec3 dir2 = {entityStorage[playerEntityT][0].dir.x , 0.0f ,entityStorage[playerEntityT][0].dir.z};
	    vec3 right = cross3((vec3) { .0f, 1.0f, .0f }, dir2);

	    float tempX = entityStorage[playerEntityT]->model->nodes[index].globalMat.m[12];
	    float tempY = entityStorage[playerEntityT]->model->nodes[index].globalMat.m[13];
	    float tempZ = entityStorage[playerEntityT]->model->nodes[index].globalMat.m[14];

	    entityStorage[playerEntityT]->model->nodes[index].globalMat.m[12] = 0;
	    entityStorage[playerEntityT]->model->nodes[index].globalMat.m[13] = 0;
	    entityStorage[playerEntityT]->model->nodes[index].globalMat.m[14] = 0;
			    
	    rotate(&entityStorage[playerEntityT]->model->nodes[index].globalMat, rad(60.0f), argVec3(right));
			    
	    entityStorage[playerEntityT]->model->nodes[index].globalMat.m[12] = tempX;
	    entityStorage[playerEntityT]->model->nodes[index].globalMat.m[13] = tempY;
	    entityStorage[playerEntityT]->model->nodes[index].globalMat.m[14] = tempZ;
	  }
	    
	  entityStorage[playerEntityT]->model->jointsMats[i] = multMat4(entityStorage[playerEntityT]->model->nodes[index].globalMat, entityStorage[playerEntityT]->model->data->invBindMats[i]);
	  entityStorage[playerEntityT]->model->jointsMats[i] = multMat4(entityStorage[playerEntityT]->model->nodes[entityStorage[playerEntityT]->model->data->parentNode].invGlobalMat, entityStorage[playerEntityT]->model->jointsMats[i]);
	    	    
	  sprintf(buf, "finalBonesMatrices[%d]", i);
	  uniformMat4(animShader, buf, entityStorage[playerEntityT]->model->jointsMats[i].m);
	}

	// chage stage to next
	//		if(entityStorage[playerEntityT][0].model->curAnim == entityStorage[playerEntityT][0].model->nextAnim){
	/*int finalStage;

	  if(entityStorage[playerEntityT][0].model->mirrored){
	  curStage =entityStorage[playerEntityT]->model->data->stage[curAnim] - curStage - 1;
	  finalStage = -1;
	  }else{
	  curStage = entityStorage[playerEntityT][0].model->curStage;
	  finalStage = entityStorage[playerEntityT]->model->data->stage[curAnim];
	  }*/
	float t = entityStorage[playerEntityT][0].model->time;
	float finalTime = entityStorage[playerEntityT]->model->data->channels[curAnim][0].samples
	  [entityStorage[playerEntityT]->model->data->channels[curAnim][0].samplesSize-1].time;

	if(entityStorage[playerEntityT][0].model->mirrored){
	  t = finalTime - t;
	  finalTime = 0;

	  if(t < finalTime){
	    entityStorage[playerEntityT][0].model->time = 0;
	    entityStorage[playerEntityT][0].model->mirrored = false;
	  }
	}else{
	  if(t > finalTime){
	    entityStorage[playerEntityT][0].model->time = 0;
	    //	entityStorage[playerEntityT][0].model->mirrored = false;
	  }
	}

	/*
	  if(t > finalTime){
	  ///*if(entityStorage[playerEntityT][0].model->action == playAnimAndPauseT){
	  entityStorage[playerEntityT][0].model->curStage--;
	  }else if(entityStorage[playerEntityT][0].model->action == playAnimInLoopT){
	  entityStorage[playerEntityT][0].model->curStage = 0;
	  }else if(entityStorage[playerEntityT][0].model->action == playAnimOnceT){
	  entityStorage[playerEntityT][0].model->curStage--;
	  entityStorage[playerEntityT][0].model->nextAnim = idleAnim;
	  entityStorage[playerEntityT][0].model->action = playAnimInLoopT;
	  //}//*/

	//entityStorage[playerEntityT][0].model->time = 0;
	//entityStorage[playerEntityT][0].model->mirrored = false;
	//}
	//*/
	//		}

	//		    entityStorage[playerEntityT][0].frame = 0;
	//		}
		    
	entityStorage[playerEntityT][0].frame++;
      }
	
      ((void (*)(int))instances[curInstance][matsSetup])(mainShader);

      //	    glUseProgram(shadersId[snowShader]); 
      //	    uniformVec3(snowShader, "cameraPos", curCamera->pos);
	    
      //	    glUseProgram(shadersId[mainShader]); 
      //	    glUniform3f(cameraPos, argVec3(curCamera->pos));

      ((void (*)(void))instances[curInstance][render3DFunc])();

      if(true)
	{
	  glDepthMask(GL_FALSE);
	  glUseProgram(shadersId[skyboxShader]);
	  glActiveTexture(GL_TEXTURE0);
	  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	  glBindVertexArray(skyboxVAO);
	  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	  glDrawArrays(GL_TRIANGLES, 0, 36);
	  glDepthMask(GL_TRUE);
	}

      // blender scene
      if(!hideScene)
      {
	glEnable(GL_BLEND);
	glUseProgram(shadersId[mainShader]);
		
	for(int i=0;i<objectsSize;++i){
	  uniformMat4(mainShader, "model", objects[i].mat.m);

	  for(int i2=0;i2< objectsInfo[objects[i].infoId].meshesSize;++i2){
	    glBindTexture(GL_TEXTURE_2D, objectsInfo[objects[i].infoId].meshes[i2].tx);
			
	    glBindBuffer(GL_ARRAY_BUFFER, objectsInfo[objects[i].infoId].meshes[i2].VBO);
	    glBindVertexArray(objectsInfo[objects[i].infoId].meshes[i2].VAO);
		    
	    glDrawArrays(GL_TRIANGLES, 0, objectsInfo[objects[i].infoId].meshes[i2].VBOSize);
	  }
	}

	glUseProgram(shadersId[waterShader]);
		
	static float timeAcc = 0.0f;
	static uint32_t lastTime = 0.0f;
		
	uint32_t time = SDL_GetTicks();
	timeAcc += (time - lastTime) / 1000.0f;
	lastTime = time;
		
	uniformFloat(waterShader, "dTime", timeAcc);
		
	for(int i=0;i<waterSurfacesSize;++i){
	  uniformMat4(waterShader, "model", waterSurfaces[i].mat.m);

	  for(int i2=0;i2< objectsInfo[waterSurfaces[i].infoId].meshesSize;++i2){
	    glBindTexture(GL_TEXTURE_2D, objectsInfo[waterSurfaces[i].infoId].meshes[i2].tx);
			
	    glBindBuffer(GL_ARRAY_BUFFER, objectsInfo[waterSurfaces[i].infoId].meshes[i2].VBO);
	    glBindVertexArray(objectsInfo[waterSurfaces[i].infoId].meshes[i2].VAO);
		    
	    glDrawArrays(GL_TRIANGLES, 0, objectsInfo[waterSurfaces[i].infoId].meshes[i2].VBOSize);
	  }
	}

	glDisable(GL_BLEND);
      }

      // object boxes
      //	    /*

      if(collisionToDraw)
	{
	  glUseProgram(shadersId[lightSourceShader]);

	  {
	    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	    glBindVertexArray(triVisualiser.VAO);
	    glBindBuffer(GL_ARRAY_BUFFER, triVisualiser.VBO);

	    Matrix out = IDENTITY_MATRIX;
	    uniformMat4(lightSourceShader, "model", out.m);
	    uniformVec3(lightSourceShader, "color", (vec3) { cyan });
		    
	    glDrawArrays(GL_LINES, 0, triVisualiser.VBOsize);
	  }
	  
	  {
	    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
	    glBindVertexArray(triInterVisualiser.VAO);
	    glBindBuffer(GL_ARRAY_BUFFER, triInterVisualiser.VBO);

	    Matrix out = IDENTITY_MATRIX;
	    uniformMat4(lightSourceShader, "model", out.m);
	    uniformVec3(lightSourceShader, "color", (vec3) { redColor });
		    
	    glDrawArrays(GL_LINES, 0, triInterVisualiser.VBOsize);
	  }


	  if(false)
	  {
	    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
	    glBindVertexArray(linesVisualiser.VAO);
	    glBindBuffer(GL_ARRAY_BUFFER, linesVisualiser.VBO);

	    Matrix out = IDENTITY_MATRIX;
	    uniformMat4(lightSourceShader, "model", out.m);
	    uniformVec3(lightSourceShader, "color", (vec3) { redColor });
		    
	    glDrawArrays(GL_LINES, 0, linesVisualiser.VBOsize);
	  }

	  if(false)
	  {
	    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
	    glBindVertexArray(lines2Visualiser.VAO);
	    glBindBuffer(GL_ARRAY_BUFFER, lines2Visualiser.VBO);

	    Matrix out = IDENTITY_MATRIX;
	    uniformMat4(lightSourceShader, "model", out.m);
	    uniformVec3(lightSourceShader, "color", (vec3) { cyan });
		    
	    glDrawArrays(GL_LINES, 0, lines2Visualiser.VBOsize);
	  }

	  if(false)
	  {
	    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
	    glBindVertexArray(AABBBoxes.VAO);
	    glBindBuffer(GL_ARRAY_BUFFER, AABBBoxes.VBO);

	    Matrix out = IDENTITY_MATRIX;
	    uniformMat4(lightSourceShader, "model", out.m);
	    uniformVec3(lightSourceShader, "color", (vec3) { greenColor });
		    
	    glDrawArrays(GL_TRIANGLES, 0, AABBBoxes.VBOsize);
	  }

	  // cylinder draw
	  //  if(false)
	  {
	    glBindVertexArray(cylinderMesh.VAO);
	    glBindBuffer(GL_ARRAY_BUFFER, cylinderMesh.VBO);

        Matrix out = IDENTITY_MATRIX;
	    uniformMat4(lightSourceShader, "model", out.m);
	    uniformVec3(lightSourceShader, "color", (vec3) { greenColor });
		    
	    //	    glDrawArrays(GL_TRIANGLE_STRIP, 0, cylinderMesh.VBOsize);
	    glDrawArrays(GL_TRIANGLES, 0, cylinderMesh.VBOsize);

	    glBindVertexArray(0);
	    glBindBuffer(GL_ARRAY_BUFFER, 0);
	  }
	    
	  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}//*/

      // draw snow
      {
	glUseProgram(shadersId[snowShader]);

	{
	  glEnable(GL_BLEND);
	  glBindBuffer(GL_ARRAY_BUFFER, snowMesh.VBO);
	  glBindVertexArray(snowMesh.VAO);

	  Matrix out = IDENTITY_MATRIX;
	  uniformMat4(snowShader, "model", out.m);
		    
	  glDrawArrays(GL_LINES, 0, snowMesh.VBOsize);

	  glBindBuffer(GL_ARRAY_BUFFER, 0);
	  glBindVertexArray(0);
	  glDisable(GL_BLEND);
	}
      }
      
      static int oppositeSnowDir[snowDirCounter] = {
	[X_MINUS_SNOW] = X_PLUS_SNOW,
	[X_PLUS_SNOW] = X_MINUS_SNOW,
	[Z_MINUS_SNOW] = Z_PLUS_SNOW,
	[Z_PLUS_SNOW] = Z_MINUS_SNOW,
      };

      // process snow
	    
      if(navPointsDraw){
	if(selectedCollisionTileIndex != -1){
	  glBindBuffer(GL_ARRAY_BUFFER, selectedCollisionTileBuf.VBO);
	  glBindVertexArray(selectedCollisionTileBuf.VAO);

	  Matrix out = IDENTITY_MATRIX;
	  uniformMat4(lightSourceShader, "model", out.m);
	  uniformVec3(lightSourceShader, "color", (vec3) { yellowColor });
		    
	  glDrawArrays(GL_TRIANGLES, 0, selectedCollisionTileBuf.VBOsize);
	}

	for(int i=0;i< layersCounter;i++){
	  glBindBuffer(GL_ARRAY_BUFFER, navigationTilesMesh[i].VBO);
	  glBindVertexArray(navigationTilesMesh[i].VAO);

	  Matrix out = IDENTITY_MATRIX;
	  uniformMat4(lightSourceShader, "model", out.m);
	      
	  if(i==0){
	    uniformVec3(lightSourceShader, "color", (vec3) { greenColor });
	  }else if(i==1){
	    uniformVec3(lightSourceShader, "color", (vec3) { redColor });
	  }
		    
	  glDrawArrays(GL_TRIANGLES, 0, navigationTilesMesh[i].VBOsize);

	  glBindBuffer(GL_ARRAY_BUFFER, 0);
	  glBindVertexArray(0);
	}
      }

      if(snowAreas){
	glUseProgram(shadersId[lightSourceShader]);
		
	for(int i=0;i< layersCounter;i++){
	  glBindBuffer(GL_ARRAY_BUFFER, snowTilesMesh[i].VBO);
	  glBindVertexArray(snowTilesMesh[i].VAO);

	  Matrix out = IDENTITY_MATRIX;
	  uniformMat4(lightSourceShader, "model", out.m);
	      
	  if(i==1){
	    uniformVec3(lightSourceShader, "color", (vec3) { redColor });
	  }else if(i==0){
	    uniformVec3(lightSourceShader, "color", (vec3) { blueColor });
	  }
		    
	  glDrawArrays(GL_TRIANGLES, 0, snowTilesMesh[i].VBOsize);

	  glBindBuffer(GL_ARRAY_BUFFER, 0);
	  glBindVertexArray(0);
	}
      }


      for(int i=0;i<mirrorsStorageSize;i++){		
	glUseProgram(shadersId[mainShader]);
	glBindVertexArray(planePairs.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, planePairs.VBO);
	glBindTexture(GL_TEXTURE_2D, mirrorsStorage[i].tx);
	uniformMat4(mainShader, "model", mirrorsStorage[i].mat.m);
	glDrawArrays(GL_TRIANGLES, 0, 6);

		
	// draw dir
	{
	  glUseProgram(shadersId[lightSourceShader]);
	    
	  glBindBuffer(GL_ARRAY_BUFFER, dirPointerLine.VBO);
	  glBindVertexArray(dirPointerLine.VAO);

	  vec3 centeroid = {
	    (mirrorsStorage[i].rt.x+mirrorsStorage[i].lb.x)/2.0f,
	    (mirrorsStorage[i].rt.y+mirrorsStorage[i].lb.y)/2.0f,
	    (mirrorsStorage[i].rt.z+mirrorsStorage[i].lb.z)/2.0f,
	  };

	  Matrix out = IDENTITY_MATRIX;
	  uniformMat4(lightSourceShader, "model", out.m);
	  uniformVec3(lightSourceShader, "color", (vec3) { redColor });
		    		    
	  float line[] = {			
	    centeroid.x, centeroid.y, centeroid.z,
			
	    centeroid.x + mirrorsStorage[i].dir.x,
	    centeroid.y + mirrorsStorage[i].dir.y,
	    centeroid.z + mirrorsStorage[i].dir.z
	  };
	
	  glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);

	  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	  glEnableVertexAttribArray(0);
		    
	  glDrawArrays(GL_LINES, 0, 2);
	}		
      }	   
    
      for(int i=0;i<entityTypesCounter;i++){
	if(entityStorageSize[i] == 0){
	  continue;
	}

	glUseProgram(shadersId[animShader]);
		
	glBindTexture(GL_TEXTURE_2D, entityStorage[i][0].model->data->tx);
		
	uniformMat4(animShader, "model", entityStorage[i][0].mat.m);

	glBindBuffer(GL_ARRAY_BUFFER, entityStorage[i][0].model->data->mesh.VBO);
	glBindVertexArray(entityStorage[i][0].model->data->mesh.VAO);

	glDrawArrays(GL_TRIANGLES, 0, entityStorage[i][0].model->data->mesh.VBOsize);

	// draw dir
	/*
	  {
	  glUseProgram(shadersId[lightSourceShader]);
	    
	  glBindBuffer(GL_ARRAY_BUFFER, dirPointerLine.VBO);
	  glBindVertexArray(dirPointerLine.VAO);

	  Matrix out = IDENTITY_MATRIX;
	  out.m[12] = entityStorage[i][0].mat.m[12];
	  out.m[13] = entityStorage[i][0].mat.m[13];
	  out.m[14] = entityStorage[i][0].mat.m[14];
		    
	  uniformMat4(lightSourceShader, "model", out.m);
	  uniformVec3(lightSourceShader, "color", (vec3) { redColor });
		    
		    
	  float line[] = {			
	  .0f, 1.2f, .0f,
			
	  entityStorage[i][0].dir.x,
	  entityStorage[i][0].dir.y + 1.2f,
	  entityStorage[i][0].dir.z
	  };
	
	  glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);

	  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	  glEnableVertexAttribArray(0);
		    
	  glDrawArrays(GL_LINES, 0, 2);
	  }

	  {
	  glUseProgram(shadersId[lightSourceShader]);
	    
	  glBindBuffer(GL_ARRAY_BUFFER, dirPointerLine.VBO);
	  glBindVertexArray(dirPointerLine.VAO);

	  Matrix out = IDENTITY_MATRIX;
		    
	  out.m[12] = entityStorage[i][0].mat.m[12];
	  out.m[13] = entityStorage[i][0].mat.m[13];
	  out.m[14] = entityStorage[i][0].mat.m[14];
		    
	  uniformMat4(lightSourceShader, "model", out.m);
	  uniformVec3(lightSourceShader, "color", (vec3) { greenColor });

	  float dist = 0.1f;
		    
	  float line[] = {			
	  (dist)*entityStorage[i][0].dir.x,
	  entityStorage[playerEntityT][0].model->data->size[1]*0.95f,
	  (dist)*entityStorage[i][0].dir.z,
			
	  entityStorage[i][0].dir.x,
	  entityStorage[playerEntityT][0].model->data->size[1]*0.95f,
	  entityStorage[i][0].dir.z
	  };
	
	  glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);

	  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	  glEnableVertexAttribArray(0);
		    
	  glDrawArrays(GL_LINES, 0, 2);
	  }*/
		    

			
	glUseProgram(shadersId[animShader]);
		

	glBindTexture(GL_TEXTURE_2D, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
      }
	 
      glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);  
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);

      //	    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);  
      //	    glBindFramebuffer(GL_FRAMEBUFFER, lowResFBO);
	    
      glBlitFramebuffer(0, 0, fakeWinW, fakeWinH, 0, 0, fakeWinW, fakeWinH, 
			GL_COLOR_BUFFER_BIT, GL_NEAREST);
	    
      //	    glBlitFramebuffer(0, 0, windowW, windowH, 0, 0, 640, 360, GL_COLOR_BUFFER_BIT, GL_NEAREST);
      //	    glBlitFramebuffer(0, 0, 640, 360, 0, 0, windowW, windowH, GL_COLOR_BUFFER_BIT, GL_NEAREST);
      //	    glBlitFramebuffer(0, 0, 640, 360, 0, 0, windowW, windowH, GL_COLOR_BUFFER_BIT, GL_NEAREST);


      // render to fbo 
      glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default

      glViewport(0,0, windowW, windowH);
      // update mirrors tx's
      {
	//	glEnable(GL_DEPTH_TEST);
	for(int i=0;i<mirrorsStorageSize;i++){
		    
	  vec3 dir = mirrorsStorage[i].dir;
	  vec3 origin = {
	    mirrorsStorage[i].mat.m[12],
	    -mirrorsStorage[i].mat.m[13],
	    -mirrorsStorage[i].mat.m[14]
	  };

	  vec3 centeroid = {
	    (mirrorsStorage[i].rt.x+mirrorsStorage[i].lb.x)/2.0f,
	    (mirrorsStorage[i].rt.y+mirrorsStorage[i].lb.y)/2.0f,
	    (mirrorsStorage[i].rt.z+mirrorsStorage[i].lb.z)/2.0f,
	  };

	  centeroid.y *= -1.0f;
	  centeroid.z *= -1.0f;
		    
	  Matrix view = lookAt(centeroid,
			       (vec3){ centeroid.x+dir.x,
				 centeroid.y+dir.y,
				 centeroid.z+dir.z},
			       (vec3){.0f,1.0f,.0f});
		    
	  glBindFramebuffer(GL_FRAMEBUFFER, mirrorsStorage[i].writeFbo);
		    
	  vec3 fogColor = { 0.5f, 0.5f, 0.5f };
	  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		    
	  {
	    // player
	    if(entityStorageSize[i] == 1){
	      glUseProgram(shadersId[animShader]);
			    
	      glBindTexture(GL_TEXTURE_2D, entityStorage[i][0].model->data->tx);
		
	      uniformMat4(animShader, "model", entityStorage[i][0].mat.m);
	      uniformMat4(animShader, "view", view.m);
	      uniformMat4(animShader, "proj", mirrorProj.m);

	      glBindBuffer(GL_ARRAY_BUFFER, entityStorage[i][0].model->data->mesh.VBO);
	      glBindVertexArray(entityStorage[i][0].model->data->mesh.VAO);

	      //			    glEnable(GL_CULL_FACE);
	      //			    glCullFace(GL_BACK);
	      glDrawArrays(GL_TRIANGLES, 0, entityStorage[i][0].model->data->mesh.VBOsize);
	      //			    glDisable(GL_CULL_FACE);
	    }

	    // blender scene
	    {
	      glEnable(GL_BLEND);
	      glUseProgram(shadersId[mainShader]);
			    
	      uniformMat4(mainShader, "view", view.m);
	      uniformMat4(mainShader, "proj", mirrorProj.m);
		
	      for(int i=0;i<objectsSize;++i){
		uniformMat4(mainShader, "model", objects[i].mat.m);

		for(int i2=0;i2< objectsInfo[objects[i].infoId].meshesSize;++i2){
		  glBindTexture(GL_TEXTURE_2D, objectsInfo[objects[i].infoId].meshes[i2].tx);
			
		  glBindBuffer(GL_ARRAY_BUFFER, objectsInfo[objects[i].infoId].meshes[i2].VBO);
		  glBindVertexArray(objectsInfo[objects[i].infoId].meshes[i2].VAO);
		    
		  glDrawArrays(GL_TRIANGLES, 0, objectsInfo[objects[i].infoId].meshes[i2].VBOSize);
		}
	      }

	      glUseProgram(shadersId[waterShader]);
		
	      static float timeAcc = 0.0f;
	      timeAcc += elapsedMs;
	      //			    printf("elapsed: %f\n", elapsedMs);
		
	      uniformFloat(waterShader, "dTime", timeAcc);

	      uniformMat4(waterShader, "view", view.m);
	      uniformMat4(waterShader, "proj", mirrorProj.m);
		
	      for(int i=0;i<waterSurfacesSize;++i){
		uniformMat4(waterShader, "model", waterSurfaces[i].mat.m);

		for(int i2=0;i2< objectsInfo[waterSurfaces[i].infoId].meshesSize;++i2){
		  glBindTexture(GL_TEXTURE_2D, objectsInfo[waterSurfaces[i].infoId].meshes[i2].tx);
			
		  glBindBuffer(GL_ARRAY_BUFFER, objectsInfo[waterSurfaces[i].infoId].meshes[i2].VBO);
		  glBindVertexArray(objectsInfo[waterSurfaces[i].infoId].meshes[i2].VAO);
		    
		  glDrawArrays(GL_TRIANGLES, 0, objectsInfo[waterSurfaces[i].infoId].meshes[i2].VBOSize);
		}
	      }

	      glDisable(GL_BLEND);
	    }

	  }

	  glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
      }

      glDisable(GL_DEPTH_TEST);

      glUseProgram(shadersId[screenShader]);
	  
      uniformFloat(screenShader, "dof", -(dofPercent - 1.0f));

      float seed = (float)(rand() % 1000 + 1) / 1000.0f;
      uniformFloat(screenShader, "time", seed);

      glBindVertexArray(quad.VAO);
      glBindTexture(GL_TEXTURE_2D, screenTexture);
      glDrawArrays(GL_TRIANGLES, 0, 6);

      glBindTexture(GL_TEXTURE_2D, 0);
      glBindFramebuffer(GL_FRAMEBUFFER, 0); 
      glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }

    // 2d ui drawing
    glDisable(GL_DEPTH_TEST);
    glUseProgram(shadersId[hudShader]);   

    instances[curInstance][render2DFunc]();
    
    {
      glUseProgram(shadersId[UIShader]);
    
      glBindVertexArray(curUIBuf.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, curUIBuf.VBO);

      glDrawArrays(GL_TRIANGLES, 0, curUIBuf.VBOsize);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      glUseProgram(shadersId[hudShader]);

      for (int i = 0; i < curUIBuf.rectsSize; i++) {
	bool underSelection = false;
      
	if (mouse.cursor.x > curUIBuf.rects[i].lb.x && mouse.cursor.x < curUIBuf.rects[i].rt.x
	    && mouse.cursor.z > curUIBuf.rects[i].lb.z && mouse.cursor.z < curUIBuf.rects[i].rt.z) {
	  underSelection = true;
	}

	if(underSelection && curUIBuf.rects[i].highlight){
	  glUseProgram(shadersId[UIShader]);
    
	  glBindVertexArray(curUIBuf.rects[i].highlight->VAO);
	  glBindBuffer(GL_ARRAY_BUFFER, curUIBuf.rects[i].highlight->VBO);

	  glDrawArrays(GL_TRIANGLES, 0, curUIBuf.rects[i].highlight->VBOsize);

	  glBindBuffer(GL_ARRAY_BUFFER, 0);
	  glBindVertexArray(0);

	  glUseProgram(shadersId[hudShader]);
	}
      
	if (curUIBuf.rects[i].text) {
	  renderText(curUIBuf.rects[i].text, curUIBuf.rects[i].textPos.x, curUIBuf.rects[i].textPos.z, 1.0f);
	}

	if(curUIBuf.rects[i].input){
	  renderText(curUIBuf.rects[i].input->buf, curUIBuf.rects[i].pos[0].x - (letterW/2.0f), curUIBuf.rects[i].pos[0].z + letterH, 1.0f);

	  if (mouse.clickL) {
	    if (underSelection) {
	      selectedTextInput2 = curUIBuf.rects[i].input;
	      textInputCursorMat.x = curUIBuf.rects[i].pos[0].x;
	      textInputCursorMat.z = curUIBuf.rects[i].pos[0].z;
	    }else{
	      inputCursorPos = 0;
	      selectedTextInput2 = NULL;
	    }
	  }
	}

	if(curUIBuf.rects[i].onclickResText){
	  renderText(curUIBuf.rects[i].onclickResText, curUIBuf.rects[i].pos[0].x, curUIBuf.rects[i].pos[0].z, 1.0f);
	}

	if (curUIBuf.rects[i].onClick) {
	  if (underSelection) {
	    if (mouse.clickL) {
	      curUIBuf.rects[i].onClick();
	    }
	  }
	}
      }
    }

    if(selectedTextInput2){
      glUseProgram(shadersId[UITransfShader]);

      uniformVec2(UITransfShader, "model2D", textInputCursorMat);
    
      glBindVertexArray(textInputCursorBuf.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, textInputCursorBuf.VBO);

      glDrawArrays(GL_TRIANGLES, 0, textInputCursorBuf.VBOsize);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      glUseProgram(shadersId[hudShader]);
    }

    instances[curInstance][renderCursorFunc]();
    glViewport(0,0, fakeWinW, fakeWinH);

    /*
      char buf[32];
      static int lastFrameFps;

      if(deltatime != 0){
      sprintf(buf, "Baza: %d%%", 1000 / deltatime);
      lastFrameFps = 1000 / deltatime;
      }else{
      sprintf(buf, "Baza: %d%%", lastFrameFps);
      }
	  
      renderText(buf, 1.0f - ((strlen(buf)+1) * letterW), 1.0f, 1.0f);

      sprintf(buf, "Dof: %d%%", (int)(dofPercent * 100.0f));
      renderText(buf, 1.0f - ((strlen(buf)+1) * letterW), 1.0f - letterH, 1.0f);
    */
	
    mouse.clickL = false;
    mouse.clickR = false;

    SDL_GL_SwapWindow(window);
	
    float delta = SDL_GetPerformanceCounter()-start_time;
    delta /= (float)SDL_GetPerformanceFrequency();

    elapsedMs = delta;
    SDL_Delay(floor(16.666f - elapsedMs));

    //	printf("Frame ms: %f fps %f \n", elapsedMs, 1.0f/ elapsedMs);
  }

  SDL_GL_DeleteContext(context);  
  SDL_DestroyWindow(window);   
  SDL_Quit(); 

  return 0; 
}

void renderCube(vec3 pos, int lightId){
  //    glActiveTexture(solidColorTx);
  glBindTexture(GL_TEXTURE_2D, solidColorTx);
  setSolidColorTx(1.0f,1.0f,1.0f, 1.0f);

  glBindBuffer(GL_ARRAY_BUFFER, cube.VBO); 
  glBindVertexArray(cube.VAO);

  //  glUniformMatrix4fv(lightModelLoc, 1, GL_FALSE, lightStorage[lightId].mat.m);



     
  glDrawArrays(GL_TRIANGLES, 0, cube.vertexNum);

  glBindTexture(GL_TEXTURE_2D, 0); 
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

bool rayIntersectsTriangle(vec3 origin, vec3 dir, vec3 lb, vec3 rt, vec3* posOfIntersection, float* dist) {
  vec3 dirfrac = { 1.0f / dir.x, 1.0f / dir.y, 1.0f / dir.z}; 

  float t1 = (lb.x - origin.x)*dirfrac.x;
  float t2 = (rt.x - origin.x)*dirfrac.x;
  float t3 = (lb.y - origin.y)*dirfrac.y;
  float t4 = (rt.y - origin.y)*dirfrac.y;
  float t5 = (lb.z - origin.z)*dirfrac.z;
  float t6 = (rt.z - origin.z)*dirfrac.z;

  float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6)); 
  float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));
  
  bool res = false;
  float t;

  if (tmax < 0)
    {
      // if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
      t = tmax;
      res= false;//false;
    }
  else if (tmin > tmax)
    {
      // if tmin > tmax, ray doesn't intersect AABB
      t = tmax;
      res= false;
    }
  else{
    t = tmin;
    res= true;
  }

  if (dist) {
    *dist = t;
  }
  if(res){
    if(posOfIntersection){
      vec3 displacement = {dir.x * t, dir.y * t, dir.z * t};

      vec3 intersection = {origin.x + displacement.x, origin.y + displacement.y, origin.z + displacement.z};

      *posOfIntersection =  intersection;
    }
  }

  return res;
}

GLuint loadShader(GLenum shaderType, const char* filename) {
  FILE* file = fopen(filename, "rb");
  if (!file) {
    fprintf(stderr, "Failed to open file: %s\n", filename);
    return 0;
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char* source = (char*)malloc(length + 1);
  fread(source, 1, length, file); 
  fclose(file);
  source[length] = '\0';

  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, (const GLchar**)&source, NULL);
  glCompileShader(shader); 

  free(source);

  GLint compileStatus;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
  if (compileStatus != GL_TRUE) {
    GLint logLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    char* log = (char*)malloc(logLength);
    glGetShaderInfoLog(shader, logLength, NULL, log);
    fprintf(stderr, "Failed to compile \"%s\" shader: %s\n",filename, log);
    free(log);
    glDeleteShader(shader);
    return 0;
  }

  return shader;
}

// w and h of one letter cell
const float atlasStep =  0.0625;

void renderText(char* text, float x, float y, float scale){
  glEnable(GL_BLEND);
  
  //  if(true) return;
  if(!text) return;

  int iter = 0;
  int padCounter = 0;
  char ch = text[iter];

  x += letterCellW;

  int initialX = x;
  
  int len = noSpacesAndNewLinesStrLen(text);
  int symblosSize = len * sizeof(float) * 6 * 4;
  float* symbols = malloc(symblosSize);

  int i = 0;

  while(ch){
    if(ch == '\n'){
      y-=letterCellH * 0.95f;
      padCounter = 0;

      iter++;
      ch = text[iter];
      continue;
    }

    if (ch == ' ') {
      padCounter++;
      iter++;
      ch = text[iter];
      continue;
    }
    
    float lettersPad = padCounter * letterW;
    int index = ch - 33;
    vec2i pos = englLettersMap[index];
  
    float baseY = atlasStep * pos.z;
    float baseX = atlasStep * pos.x;

    // 1
    symbols[i] = x + lettersPad;
    symbols[i+1] = y; 
    symbols[i+2] = baseX;
    symbols[i+3] = baseY + atlasStep;

    // 2
    symbols[i+4] = x - letterCellW + lettersPad;
    symbols[i+5] = y;
    symbols[i+6] = baseX + atlasStep;
    symbols[i+7] = baseY + atlasStep;
    
    // 3
    symbols[i+8] = x + lettersPad;
    symbols[i+9] = y - letterCellH;
    symbols[i+10] = baseX;
    symbols[i+11] = baseY;

    // 4
    symbols[i+12] = x - letterCellW + lettersPad;
    symbols[i+13] = y;
    symbols[i+14] = baseX + atlasStep;
    symbols[i+15] = baseY + atlasStep;

    // 5
    symbols[i+16] = x + lettersPad;
    symbols[i+17] = y - letterCellH;
    symbols[i+18] = baseX;
    symbols[i+19] = baseY;

    // 6
    symbols[i+20] = x - letterCellW + lettersPad;
    symbols[i+21] = y - letterCellH;
    symbols[i+22] = baseX + atlasStep;
    symbols[i+23] = baseY;

    i+=24;
    iter++;
    padCounter++;
    ch = text[iter];
  }

  glBindVertexArray(textVAO);
  glBindBuffer(GL_ARRAY_BUFFER, textVBO);
  glBindTexture(GL_TEXTURE_2D, fontAtlas);

  glBufferData(GL_ARRAY_BUFFER, symblosSize, symbols, GL_STATIC_DRAW);
  free(symbols);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1 , 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (2 * sizeof(float)));
  glEnableVertexAttribArray(1);
      
  glDrawArrays(GL_TRIANGLES, 0, symblosSize / 16);
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  
  glBindTexture(GL_TEXTURE_2D, 0);

  glDisable(GL_BLEND);
}

int strcut(char *str, int begin, int len)
{
  int l = strlen(str);

  if (len < 0) len = l - begin;
  if (begin + len > l) len = l - begin;
  memmove(str + begin, str + begin + len, l - len + 1);

  return len;
}

int strtrim(char *str){
  char *end;

  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)
    return str;

  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  end[1] = '\0';

  return str;
}

int lastCharPos(char* str, char ch){
  int len = strlen(str);
  for (int i = len - 1; i >= 0; i--) {
    if (str[i] == ch) {
      return i;
    }
  }
  return -1;
}


int noSpacesAndNewLinesStrLen(char* str){
  int len = strlen(str);
  
  for (int i = len - 1; i >= 0; i--) {
    if (str[i] == '\n' || str[i] == ' ') {
      len--;
    }
  }
  
  return len;
}

void cleanString(char *str) {
  int i, j;
  int isPreviousSpace = 0;

  for (i = 0, j = 0; str[i] != '\0'; i++) {
    if (isalnum(str[i]) || (str[i] >= '!' && str[i] <= '/') || (str[i] >= ':' && str[i] <= '@')) {
      str[j++] = str[i];
      isPreviousSpace = 0;
    }
      
    else if (isspace(str[i])) {
      if (!isPreviousSpace) {
	str[j++] = ' ';
	isPreviousSpace = 1;
      }
    }
  }
  str[j] = '\0';
}

void destroyCharacter(int id){
  if(id == -1 || !characters) {
    return;
  }
  
  Character* deletedCharacter = &characters[id];
  Dialog* dialog = &deletedCharacter->dialogs;
  destroyDialogsFrom(dialog);

  if(deletedCharacter->name){
    free(deletedCharacter->name);
  }

  if(deletedCharacter->dialogHistory){
    free(deletedCharacter->dialogHistory);
  }

  int index = 0;
		
  for(int i=0;i<charactersSize;i++){
    if(i == deletedCharacter->id){
      continue;
    }
		  
    characters[index] = characters[i];
    index++;
  }

  charactersSize--;

  if(charactersSize == 0){
    characters = NULL;
  }
  
  characters = realloc(characters, charactersSize * sizeof(Character));
}


void destroyDialogsFrom(Dialog* root)
{
  // Base case
  if (root == NULL) return;
  
  for (int i = 0; i < root->answersSize; i++) {
    destroyDialogsFrom(&root->answers[i]);
  }

  if (root->text) {
    free(root->text);
  }

  if (root->replicaText) {
    free(root->replicaText);
  }

  if(root->answersSize != 0){
    free(root->answers);
  }
}


void serializeDialogTree(Dialog* root, FILE *fp)
{
  if (root == NULL) return;
 
  fprintf(fp,"R\"%s\"A\"%s\" ", root->replicaText ? root->replicaText : "NULL", root->text ? root->text : "NULL");
  
  for (int i = 0; i < root->answersSize; i++) 
    serializeDialogTree(&root->answers[i],  fp);

  fprintf(fp,"%c", ')');
}

int deserializeDialogTree(Dialog* root, Dialog* parent, FILE *fp)
{
  char val[dialogEditorAnswerInputLimit + 1];
  char replicaStr[dialogEditorReplicaInputLimit + 1];
  char ch = fgetc(fp);

  while (ch != 'R') {
    if (ch == ')')
      return 1;
    else if (ch == EOF) 
      return 0; 
    else 
      ch = fgetc(fp);
  }

  ungetc(ch, fp);

  if (!fscanf(fp, "R\"%[^\"]\"A\"%[^\"]\" ",&replicaStr, &val) )  
    return 1;

  if (parent) {  
    parent->answersSize++;

    if (parent->answers) {
      parent->answers = realloc(parent->answers, parent->answersSize * sizeof(Dialog));
      memset(&parent->answers[parent->answersSize - 1], 0, sizeof(Dialog));
    }
    else {
      parent->answers = calloc(1, sizeof(Dialog));
    }

    root = &parent->answers[parent->answersSize - 1];  
  }

  if(strcmp(val, "NULL") != 0){ 
    root->text = malloc(sizeof(char) * strlen(val) + 1);
    strcpy(root->text, val);  
  }

  if(strcmp(replicaStr, "NULL") != 0){
    root->replicaText = malloc(sizeof(char) * strlen(replicaStr) + 1);
    strcpy(root->replicaText, replicaStr);  
  }

  for (int i = 0; i < 7; i++)
    if (deserializeDialogTree(&root->answers[i], root, fp))
      break;
 
  return 0;
}


// accepts percents
float* uiRectPercentage(float x, float y, float w, float h){
  vec2 lt = { 2 * x - 1, 2 * y - 1 };
  vec2 rt = { 2 * w - 1, 2 * y - 1 };

  vec2 lb = { 2 * x - 1, 2 * h - 1 };
  vec2 rb = { 2 * w - 1, 2 * h - 1 };
  
  float stackRect[] = { 
    argVec2(lt), 1.0f, 0.0f,
    argVec2(rt), 1.0f, 1.0f,
    argVec2(lb), 0.0f, 0.0f,

    argVec2(rt), 1.0f, 1.0f,
    argVec2(lb), 0.0f, 0.0f,
    argVec2(rb), 0.0f, 1.0f };
  
  float* rect = malloc(sizeof(stackRect));	
  memcpy(rect, stackRect, sizeof(stackRect));
	
  return rect;
}

float* uiRectPoints(float x, float y, float w, float h){
  vec2 lt = { x, y };
  vec2 rt = { x + w, y };

  vec2 lb = { x, y - h };
  vec2 rb = { x + w, y - h };
  
  float stackRect[] = { 
    argVec2(lt), 1.0f, 0.0f,
    argVec2(rt), 1.0f, 1.0f,
    argVec2(lb), 0.0f, 0.0f,

    argVec2(rt), 1.0f, 1.0f,
    argVec2(lb), 0.0f, 0.0f,
    argVec2(rb), 0.0f, 1.0f }; 
   
  float* rect = malloc(sizeof(stackRect));	
  memcpy(rect, stackRect, sizeof(stackRect));
	
  return rect;
}

SDL_Surface* IMG_Load_And_Flip_Vertical(char* path){
  SDL_Surface* surface = IMG_Load(path);

  if(!surface){
    return NULL;
  }

  SDL_LockSurface(surface);
    
  int pitch = surface->pitch;
  char* temp = malloc(sizeof(char) * surface->pitch);
  char* pixels = (char*) surface->pixels;
    
  for(int i = 0; i < surface->h / 2; ++i) {
    // get pointers to the two rows to swap
    char* row1 = pixels + i * pitch;
    char* row2 = pixels + (surface->h - i - 1) * pitch;
        
    // swap rows
    memcpy(temp, row1, pitch);
    memcpy(row1, row2, pitch);
    memcpy(row2, temp, pitch);
  }
    
  free(temp);

  SDL_UnlockSurface(surface);


  return surface;
}

void rerenderShadowsForAllLights(){
  GLint curShader = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &curShader);

  glUseProgram(shadersId[dirShadowShader]);
  glEnable(GL_DEPTH_TEST);
    
  glViewport(0,0, SHADOW_WIDTH, SHADOW_HEIGHT);
  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

    
  
  for (int i = 0; i < lightStorageSizeByType[dirLightShadowT]; i++) {
    glUseProgram(shadersId[dirShadowShader]);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMaps, 0, i);
	    
    vec3 negPos = { -lightStorage[dirLightShadowT][i].mat.m[12], -lightStorage[dirLightShadowT][i].mat.m[13], -lightStorage[dirLightShadowT][i].mat.m[14] };

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Matrix proj = perspective(rad(90.0f), 1.0f, 0.01f, 1000.0f);

    vec3 lightPos = { lightStorage[dirLightShadowT][i].mat.m[12],
      -lightStorage[dirLightShadowT][i].mat.m[13],
      -lightStorage[dirLightShadowT][i].mat.m[14]
    };
    
    vec3 lightDir = { lightStorage[dirLightShadowT][i].mat.m[0] + 0.001f, lightStorage[dirLightShadowT][i].mat.m[1], lightStorage[dirLightShadowT][i].mat.m[2] };
    
    Matrix view = lookAt(lightPos,
			 (vec3){
			   lightPos.x + lightDir.x,
			   lightPos.y + lightDir.y,
			   lightPos.z + lightDir.z
			 },
			 (vec3){0.0f,1.0f,0.0f});
    
    Matrix lightSpaceMatrix = multiplymat4(view, proj);

    char buf[128];
    sprintf(buf, "lightSpaceMatrix");
    uniformMat4(dirShadowShader, buf, lightSpaceMatrix.m);

    glActiveTexture(GL_TEXTURE0);
    renderScene(dirShadowShader);
    
    sprintf(buf, "lightSpaceMatrix[%d]", i);
	
    glUseProgram(shadersId[snowShader]);
    uniformMat4(snowShader, buf, lightSpaceMatrix.m);
	
    glUseProgram(shadersId[mainShader]);
    uniformMat4(mainShader, buf, lightSpaceMatrix.m);

    glUseProgram(shadersId[animShader]);
    uniformMat4(animShader, buf, lightSpaceMatrix.m);
  }

    
  glUseProgram(curShader);
}

void rerenderShadowForLight(int lightId){
  GLint curShader = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &curShader);

  glUseProgram(shadersId[dirShadowShader]);
  glEnable(GL_DEPTH_TEST);
    
  glViewport(0,0, SHADOW_WIDTH, SHADOW_HEIGHT);
  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

  glUseProgram(shadersId[dirShadowShader]);
  glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMaps, 0, lightId);

    
	    
  vec3 negPos = { -lightStorage[dirLightShadowT][lightId].mat.m[12], -lightStorage[dirLightShadowT][lightId].mat.m[13], -lightStorage[dirLightShadowT][lightId].mat.m[14] };

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  Matrix proj = perspective(rad(90.0f), 1.0f, 0.01f, 1000.0f);

  vec3 lightPos = { lightStorage[dirLightShadowT][lightId].mat.m[12],
    -lightStorage[dirLightShadowT][lightId].mat.m[13],
    -lightStorage[dirLightShadowT][lightId].mat.m[14]
  };
    
  vec3 lightDir = { lightStorage[dirLightShadowT][lightId].mat.m[0] + 0.001f, lightStorage[dirLightShadowT][lightId].mat.m[1], lightStorage[dirLightShadowT][lightId].mat.m[2] };
    
  Matrix view = lookAt(lightPos,
		       (vec3){
			 lightPos.x + lightDir.x,
			 lightPos.y + lightDir.y,
			 lightPos.z + lightDir.z
		       },
		       (vec3){0.0f,1.0f,0.0f});
    
  Matrix lightSpaceMatrix = multiplymat4(view, proj);

  char buf[128];
  sprintf(buf, "lightSpaceMatrix");
  uniformMat4(dirShadowShader, buf, lightSpaceMatrix.m);

  glActiveTexture(GL_TEXTURE0);
  renderScene(dirShadowShader);
    
  sprintf(buf, "lightSpaceMatrix[%d]", lightId);
    
  glUseProgram(shadersId[snowShader]);
  uniformMat4(snowShader, buf, lightSpaceMatrix.m);
    
  glUseProgram(shadersId[mainShader]);
  uniformMat4(mainShader, buf, lightSpaceMatrix.m);

  glUseProgram(shadersId[animShader]);
  uniformMat4(animShader, buf, lightSpaceMatrix.m);

    
  glUseProgram(curShader);
}

vec3 calculateNormal(vec3 a, vec3 b, vec3 c){
  vec3 x = {b.x - a.x, b.y - a.y, b.z - a.z}; 
  vec3 y = {c.x - a.x, c.y - a.y, c.z - a.z};

  //vec3 normal = cross3(x, y);
  vec3 normNormal = normalize3(cross3(x, y));
  //  vec3 normNormal = normalize3(normal);
  
  //  vec3 res = (dotf3(normNormal,normal) < 0.0f) ? normNormal : (vec3){-normNormal.x, -normNormal.y, -normNormal.z};
  
  return normNormal;
};


void attachNormalsToBuf(VPair* VPairBuf, int plane, int size, float* buf){
  glGenBuffers(1, &VPairBuf[plane].VBO);
  glGenVertexArrays(1, &VPairBuf[plane].VAO);

  glBindVertexArray(VPairBuf[plane].VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VPairBuf[plane].VBO);

  int oldVertSize = size / sizeof(float) / 5;
  size_t newSize = size + (sizeof(vec3) * oldVertSize);

  VPairBuf[plane].vBuf = malloc(newSize);
  VPairBuf[plane].vertexNum = newSize/sizeof(float)/8;
  VPairBuf[plane].attrSize = 8;

  int oldBufI = 0;
  for(int i=0;i<newSize/sizeof(float);i+=8*3, oldBufI+=5*3){
    vec3 a = (vec3){ buf[oldBufI], buf[oldBufI+1], buf[oldBufI+2] };
    vec3 b = (vec3){ buf[oldBufI+5], buf[oldBufI+6], buf[oldBufI+7] };
    vec3 c = (vec3){ buf[oldBufI+10], buf[oldBufI+11], buf[oldBufI+12] };

    // uv
    VPairBuf[plane].vBuf[i+3] = buf[oldBufI+3]; 
    VPairBuf[plane].vBuf[i+4] = buf[oldBufI+4];

    VPairBuf[plane].vBuf[i+11] = buf[oldBufI+8]; 
    VPairBuf[plane].vBuf[i+12] = buf[oldBufI+9];
    
    VPairBuf[plane].vBuf[i+19] = buf[oldBufI+13]; 
    VPairBuf[plane].vBuf[i+20] = buf[oldBufI+14];

    vec3 norm = calculateNormal(a,b,c);

    // norm
    VPairBuf[plane].vBuf[i+5] = norm.x;
    VPairBuf[plane].vBuf[i+6] = norm.y;
    VPairBuf[plane].vBuf[i+7] = norm.z;

    VPairBuf[plane].vBuf[i+13] = norm.x;
    VPairBuf[plane].vBuf[i+14] = norm.y;
    VPairBuf[plane].vBuf[i+15] = norm.z;

    VPairBuf[plane].vBuf[i+21] = norm.x;
    VPairBuf[plane].vBuf[i+22] = norm.y;
    VPairBuf[plane].vBuf[i+23] = norm.z;

    // vert
    VPairBuf[plane].vBuf[i+0] = a.x;
    VPairBuf[plane].vBuf[i+1] = a.y;
    VPairBuf[plane].vBuf[i+2] = a.z;

    VPairBuf[plane].vBuf[i+8] = b.x;
    VPairBuf[plane].vBuf[i+9] = b.y;
    VPairBuf[plane].vBuf[i+10] = b.z;

    VPairBuf[plane].vBuf[i+16] = c.x;
    VPairBuf[plane].vBuf[i+17] = c.y;
    VPairBuf[plane].vBuf[i+18] = c.z;
  }



  //  VPairBuf[plane].vBuf = malloc(size);
  //  memcpy(VPairBuf[plane].vBuf, buf, size);

  glBufferData(GL_ARRAY_BUFFER, newSize, VPairBuf[plane].vBuf, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 3 * sizeof(float));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 5 * sizeof(float));
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

float* createNormalBuffer(float* buf, int size, int* finalSize){
  int vertNum = size / sizeof(float) / 5;
  size_t newSize = (sizeof(float) * vertNum * 8);
  float* bufWithNormals = malloc(newSize);

  *finalSize = newSize;

  int oldBufI = 0;
  for(int i=0;i<newSize/sizeof(float);i+=8*3, oldBufI+=5*3){
    vec3 a = (vec3){ buf[oldBufI], buf[oldBufI+1], buf[oldBufI+2] };
    vec3 b = (vec3){ buf[oldBufI+5], buf[oldBufI+6], buf[oldBufI+7] };
    vec3 c = (vec3){ buf[oldBufI+10], buf[oldBufI+11], buf[oldBufI+12] };

    // uv
    bufWithNormals[i+3] = buf[oldBufI+3]; 
    bufWithNormals[i+4] = buf[oldBufI+4];

    bufWithNormals[i+11] = buf[oldBufI+8]; 
    bufWithNormals[i+12] = buf[oldBufI+9];
    
    bufWithNormals[i+19] = buf[oldBufI+13]; 
    bufWithNormals[i+20] = buf[oldBufI+14];

    vec3 norm = calculateNormal(a,b,c);

    // norm
    bufWithNormals[i+5] = norm.x;
    bufWithNormals[i+6] = norm.y;
    bufWithNormals[i+7] = norm.z;

    bufWithNormals[i+13] = norm.x;
    bufWithNormals[i+14] = norm.y;
    bufWithNormals[i+15] = norm.z;

    bufWithNormals[i+21] = norm.x;
    bufWithNormals[i+22] = norm.y;
    bufWithNormals[i+23] = norm.z;

    // vert
    bufWithNormals[i+0] = a.x;
    bufWithNormals[i+1] = a.y;
    bufWithNormals[i+2] = a.z;

    bufWithNormals[i+8] = b.x;
    bufWithNormals[i+9] = b.y;
    bufWithNormals[i+10] = b.z;

    bufWithNormals[i+16] = c.x;
    bufWithNormals[i+17] = c.y;
    bufWithNormals[i+18] = c.z;
  }
  
  return bufWithNormals;
}

/*
  0.000000 0.000000 -0.125000
  0.000000 2.000000 -0.125000
  1.000000 2.000000 -0.125000

  0.000000 0.000000 -0.125000
  1.000000 2.000000 -0.125000
  1.000000 0.000000 -0.125000*/


void createTexture(int* tx,int w,int h, void*px){
  glGenTextures(1, tx);

  glBindTexture(GL_TEXTURE_2D, *tx);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w,
	       h, 0, GL_RGBA,
	       GL_UNSIGNED_BYTE, px);
  
  glGenerateMipmap(GL_TEXTURE_2D);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
 
  glBindTexture(GL_TEXTURE_2D, 0);
}; 

// TODO: Func->macro
void uniformVec4(int shader, char* var, vec4 value){
  GLint curShader = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &curShader);
  glUseProgram(shadersId[shader]);
    
  int uni = glGetUniformLocation(shadersId[shader], var);
  glUniform4f(uni, argVec4(value));
    
  glUseProgram(curShader);
};

void uniformVec3(int shader, char* var, vec3 value){
  GLint curShader = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &curShader);
  glUseProgram(shadersId[shader]);
    
  int uni = glGetUniformLocation(shadersId[shader], var);
  glUniform3f(uni, argVec3(value));

  glUseProgram(curShader);
};

void uniformVec2(int shader, char* var, vec2 value){
  GLint curShader = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &curShader);
  glUseProgram(shadersId[shader]);
    
  int uni = glGetUniformLocation(shadersId[shader], var);
  glUniform2f(uni, argVec2(value));

  glUseProgram(curShader);
};

void uniformMat4(int shader, char* var, float* mat){
  GLint curShader = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &curShader);
  glUseProgram(shadersId[shader]);
    
  int uni = glGetUniformLocation(shadersId[shader], var);
  glUniformMatrix4fv(uni, 1, GL_FALSE, mat);

  glUseProgram(curShader);
};

void uniformFloat(int shader, char* var, float value) {
  GLint curShader = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &curShader);
  glUseProgram(shadersId[shader]);
    
  int uni = glGetUniformLocation(shadersId[shader], var);
  glUniform1f(uni, value);

  glUseProgram(curShader);
};

void uniformInt(int shader, char* var, int value) {
  GLint curShader = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &curShader);
  glUseProgram(shadersId[shader]);
    
  int uni = glGetUniformLocation(shadersId[shader], var);
  glUniform1i(uni,value);

  glUseProgram(curShader);
};

void assembleBlocks(){
  float t = (float)1 / 8;
  float capRatio = 0.12f;

  float stepW = bBlockW / 4.0f;
  float stepH = floorH / 4.0f;

  float texturedTileVerts[] = {
    // higher step top part
    0.0f, floorH, 0.0f, 1.0f, 1.0f,
    bBlockW, floorH, 0.0f , 0.0f, 0.0f,
    0.0f, floorH, stepW , 1.0f, 0.0f,
	
    bBlockW, floorH, 0.0f , 0.0f, 0.0f,
    0.0f, floorH, stepW , 1.0f, 0.0f,
    bBlockW, floorH, stepW, 0.0f, 0.0f,

    // higher step side part
    0.0f, floorH, stepW, 1.0f, 1.0f,
    bBlockW, floorH, stepW , 0.0f, 0.0f,
    0.0f, floorH - stepH, stepW , 1.0f, 0.0f,
	
    bBlockW, floorH , stepW , 0.0f, 0.0f,
    0.0f, floorH - stepH, stepW , 1.0f, 0.0f,
    bBlockW, floorH - stepH, stepW, 0.0f, 0.0f,

    // 3th step top part
    0.0f, floorH - stepH, stepW, 1.0f, 1.0f,
    bBlockW, floorH - stepH, stepW , 0.0f, 0.0f,
    0.0f, floorH - stepH, stepW*2, 1.0f, 0.0f,
	
    bBlockW, floorH - stepH, stepW , 0.0f, 0.0f,
    0.0f, floorH - stepH, stepW *2, 1.0f, 0.0f,
    bBlockW, floorH - stepH, stepW * 2, 0.0f, 0.0f,

    // 3th step side part
    0.0f, floorH - stepH, stepW*2, 1.0f, 1.0f,
    bBlockW, floorH - stepH, stepW*2 , 0.0f, 0.0f,
    0.0f, floorH - stepH * 2, stepW*2 , 1.0f, 0.0f,
	
    bBlockW, floorH - stepH, stepW*2 , 0.0f, 0.0f,
    0.0f, floorH - stepH * 2, stepW*2 , 1.0f, 0.0f,
    bBlockW, floorH - stepH * 2, stepW*2, 0.0f, 0.0f,

    // 2th step top part
    0.0f, floorH - stepH * 2, stepW * 2, 1.0f, 1.0f,
    bBlockW, floorH - stepH * 2, stepW *2, 0.0f, 0.0f,
    0.0f, floorH - stepH * 2, stepW*3, 1.0f, 0.0f,
	
    bBlockW, floorH - stepH * 2, stepW *2 , 0.0f, 0.0f, // 4
    0.0f, floorH - stepH * 2, stepW *3, 1.0f, 0.0f,
    bBlockW, floorH - stepH * 2, stepW * 3, 0.0f, 0.0f,

    // 2th step side part
    0.0f, floorH - stepH * 2, stepW*3, 1.0f, 1.0f,
    bBlockW, floorH - stepH * 2, stepW*3 , 0.0f, 0.0f,
    0.0f, floorH - stepH * 3, stepW*3 , 1.0f, 0.0f,
	
    bBlockW, floorH - stepH * 2, stepW*3 , 0.0f, 0.0f,
    0.0f, floorH - stepH * 3, stepW*3 , 1.0f, 0.0f,
    bBlockW, floorH - stepH * 3, stepW*3, 0.0f, 0.0f,

    // 1th step top part
    0.0f, floorH - stepH * 3, stepW * 3, 1.0f, 1.0f,
    bBlockW, floorH - stepH * 3, stepW *3, 0.0f, 0.0f,
    0.0f, floorH - stepH * 3, stepW*4, 1.0f, 0.0f,
	
    bBlockW, floorH - stepH * 3, stepW *3 , 0.0f, 0.0f,
    0.0f, floorH - stepH * 3, stepW *4, 1.0f, 0.0f,
    bBlockW, floorH - stepH * 3, stepW * 4, 0.0f, 0.0f,

    // 1th step side part
    0.0f, floorH - stepH * 3, stepW*4, 1.0f, 1.0f,
    bBlockW, floorH - stepH * 3, stepW*4 , 0.0f, 0.0f,
    0.0f, floorH - stepH * 4, stepW*4 , 1.0f, 0.0f,
	
    bBlockW, floorH - stepH * 3, stepW*4 , 0.0f, 0.0f,
    0.0f, floorH - stepH * 4, stepW*4 , 1.0f, 0.0f,
    bBlockW, floorH - stepH * 4, stepW*4, 0.0f, 0.0f,
  };

  float oppositePad = t/12;
	
  float roofBlock[] = {
    // main part
    bBlockW, floorH, 0.0f ,      1.0f, 1.0f, // 2
    0.0f, 0.0f, bBlockD + t,     0.0f, 0.0f, // 1
    bBlockW, 0.0f, bBlockD + t,  1.0f, 0.0f, // 3

    bBlockW, floorH, 0.0f ,    1.0f, 1.0f,
    0.0f, floorH, 0.0f,        0.0f, 1.0f,
    0.0f, 0.0f, bBlockD + t,   0.0f, 0.0f,

    // oposite main cap
    0.0f, -oppositePad, bBlockD + t,       0.0f, 0.0f,
    bBlockW, floorH-oppositePad, 0.0f ,    1.0f, 1.0f,
    0.0f, floorH-oppositePad, 0.0f,        0.0f, 1.0f,

    0.0f, -oppositePad, bBlockD + t,         0.0f, 0.0f,
    bBlockW, -oppositePad, bBlockD + t,      1.0f, 0.0f,
    bBlockW, floorH-oppositePad, 0.0f ,      1.0f, 1.0f,

    // cap
    0.0f, 0.0f, bBlockD + t,         0.0f, 0.0f,
    bBlockW, 0.0f, bBlockD + t,      1.0f, 0.0f,
    0.0f, -t, bBlockD + t + t,       0.0f, capRatio,

    bBlockW, 0.0f, bBlockD + t,      1.0f, 0.0f,
    0.0f, -t, bBlockD + t + t,       0.0f, capRatio,
    bBlockW, -t,  bBlockD + t + t,   1.0f, capRatio,
  };

  float angledRoof[] = {
    bBlockW, floorH, bBlockD,    0.0f, 1.0f,
    0.0f - t, 0.0f, bBlockD,         0.0f, 0.0f,
    0.0f - t, 0.0f, 0.0f -t,           1.0f, 1.0f,
    
    0.0f - t, 0.0f, 0.0f-t,            1.0f, 0.0f,
    bBlockW, floorH, bBlockD,   0.0f, 1.0f,
    bBlockW, 0.0f, 0.0f -t,        0.0f, 0.0f,

    // cap front
    0.0f - t, 0.0f, 0.0f - t,        1.0f, capRatio,
    bBlockW, 0.0f, 0.0f - t,         0.0f, 0.0f,
    -t - t, -t, 0.0f - t - t,        1.0f, 0.0f,

    bBlockW, 0.0f, 0.0f - t,         0.0f, capRatio,
    -t - t, -t, 0.0f - t - t,        1.0f, 0.0f,
    bBlockW, -t, 0.0f - t - t,       0.0f, 0.0f,

    // side cap
    0.0f - t, 0.0f, 0.0f - t,            1.0f, 0.0f,
    0.0f - t, 0.0f, bBlockD,             0.0f, 0.0f,
    0.0f -t - t, -t, 0.0f - t - t,       1.0f, capRatio,

    0.0-t, 0.0f, bBlockD,                0.0f, 0.0f,
    0.0f -t - t, -t, 0.0f - t - t,       1.0f, capRatio,
    0.0f -t - t, -t, bBlockD,            0.0f, capRatio,
  };

  //float cubeBlock[] = {

  //};

  float* blocksBuffers[tileBlocksCounter] = {
    [roofBlockT]=roofBlock,
    [stepsBlockT]=texturedTileVerts,
    [angledRoofT]=angledRoof,
  };

  int blocksBuffersSize[tileBlocksCounter] = {
    [roofBlockT]=sizeof(roofBlock),
    [stepsBlockT]=sizeof(texturedTileVerts),
    [angledRoofT]=sizeof(angledRoof),
  };

  
  for(int i=0;i<tileBlocksCounter;i++){
    blocksVPairs[i].planesNum = 1;
    blocksVPairs[i].pairs = calloc(blocksVPairs[i].planesNum, sizeof(VPair));

    for(int i2=0;i2<blocksVPairs[i].planesNum;i2++){
      attachNormalsToBuf(blocksVPairs[i].pairs, i2, blocksBuffersSize[i], blocksBuffers[i]);
    }
  }
}

void updateNodes(int curIndex, int parentIndex, GLTFNode** nodes){
  (*nodes)[curIndex].globalMat = gltfTRS((*nodes)[curIndex].t);
    
  //	(*nodes)[curIndex].T,  
  //	(*nodes)[curIndex].R, 
  //(*nodes)[curIndex].S


  if(parentIndex != -1){
    (*nodes)[curIndex].globalMat =
      multMat4((*nodes)[parentIndex].globalMat,
	       (*nodes)[curIndex].globalMat);	
  }

  inverse((*nodes)[curIndex].globalMat.m,
	  (*nodes)[curIndex].invGlobalMat.m);    
    
  for(int i=0;i< (*nodes)[curIndex].childSize;i++){
    updateNodes((*nodes)[curIndex].child[i],
		curIndex,
		nodes);
  }    
}


int sortAnimStepsByTime(AnimStep* a, AnimStep* b){
  if(a->time > b->time)  
    {  
      return 1;  
    }else if(a->time < b->time){  
    return -1;  
  }
    
  return 0;  
}

void loadGLTFModel(char* name){
  cgltf_options options = {0};
  cgltf_data* data = NULL;
  cgltf_result result = cgltf_parse_file(&options, name, &data);

  if (result != cgltf_result_success){
    exit(-1);
  }

  char buf[128];    

  if(!modelsData){
    modelsData = malloc(sizeof(ModelData));
  }else{
    modelsData = realloc(modelsData,sizeof(ModelData)*(modelsDataSize+1));
  }

  modelsData[modelsDataSize].mesh.VBOsize = data->meshes->primitives->indices->count;

  FILE* fo = NULL;

  // get path of .bin file
  {
    strcpy(buf, name);

    for (int i = 1; i < strlen(buf); i++) {
      if (buf[i] == '.') {
	buf[i] = '\0';
	strcat(buf, ".bin");
	break;
      }
    }

    fo = fopen(buf, "rb");
  }
    

  float* mesh = malloc(sizeof(float)*data->meshes->primitives->indices->count*16);

  modelsData[modelsDataSize].rawBuf = malloc(sizeof(float)*data->meshes->primitives->indices->count*3);
  modelsData[modelsDataSize].idxNum = data->meshes->primitives->indices->count;
  
  int rawBufIndex = 0;

  for(int i=0;i<3;i++){
    modelsData[modelsDataSize].size[i] = fabsf(data->meshes->primitives->attributes->data->min[i]) +
      data->meshes->primitives->attributes->data->max[i];
    modelsData[modelsDataSize].center[i] = (data->meshes->primitives->attributes->data->min[i]) +
      data->meshes->primitives->attributes->data->max[i] / 2.0f;
  }
    

  int elementSize = GLTFtypeSize[data->meshes->primitives->indices->component_type];
  for (int i = 0; i < data->meshes->primitives->indices->count;i++) {
    uint16_t index;
    fseek(fo, data->meshes->primitives->indices->buffer_view->offset + i * elementSize, SEEK_SET);
    fread(&index, elementSize, 1, fo);
	
    for(int i2=0;i2<data->meshes->primitives->attributes_count;i2++){
      int compSize = GLTFtypeSize[data->meshes->primitives->attributes[i2].data->component_type];
      int vecLen = GLTFmapSize[data->meshes->primitives->attributes[i2].data->type];
	    
      for(int i3=0;i3<vecLen;i3++){
	fseek(fo, data->meshes->primitives->attributes[i2].data->buffer_view->offset
	      + compSize*(vecLen * index + i3), SEEK_SET);
		
	if(data->meshes->primitives->attributes[i2].data->component_type
	   !=cgltf_component_type_r_32f){
	  uint8_t temp;
	  fread(&temp, compSize, 1, fo);
		    
	  mesh[(i*16)+i3+GLTFattrPad[data->meshes->primitives->attributes[i2].type]] = (float)temp;
	}else{
	  fread(&mesh[(i*16)+i3+GLTFattrPad[data->meshes->primitives->attributes[i2].type]]
		,compSize, 1, fo);

	  if(data->meshes->primitives->attributes[i2].type == cgltf_attribute_type_position){
	    modelsData[modelsDataSize].rawBuf[rawBufIndex] = mesh[(i*16)+i3+GLTFattrPad[data->meshes->primitives->attributes[i2].type]];
	    rawBufIndex++;
	  }
	}
      }
    }

    //	mulmatvec4();
  }

  float maxZ = data->meshes->primitives->attributes->data->max[2] / 2.0f;
  float curY = data->meshes->primitives->attributes->data->max[1] / 2.0f;

  for(int i=0;i<data->meshes->primitives->indices->count;i++){
    int index = i*16;

    if(mesh[index+2]>maxZ && mesh[index+1]>curY){
      curY = mesh[index+1];
      maxZ = mesh[index+2];
	    
      //	    printf("Max Z:! %f - p:(%f %f %f) t:(%f %f) n:(%f %f %f) j:(%f %f %f %f) w:(%f %f %f %f)\n", maxZ,
      //		   mesh[index], mesh[index+1],mesh[index+2],mesh[index+3], mesh[index+4], mesh[index+5]
      //		   ,mesh[index+6], mesh[index+7], mesh[index+8], mesh[index+9], mesh[index+10],
      //		   mesh[index+11], mesh[index+12], mesh[index+13], mesh[index+14], mesh[index+15]);

      memcpy(maxZVertex, &mesh[index], sizeof(float)*16);
    }
  }
    

  glGenVertexArrays(1, &modelsData[modelsDataSize].mesh.VAO);
  glGenBuffers(1, &modelsData[modelsDataSize].mesh.VBO);

  glBindVertexArray(modelsData[modelsDataSize].mesh.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, modelsData[modelsDataSize].mesh.VBO);
    
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*modelsData[modelsDataSize].mesh.VBOsize*16, mesh, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*16, (void*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*16, (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float)*16, (void*)(5 * sizeof(float)));
  glEnableVertexAttribArray(2);
    
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(float)*16, (void*)(8 * sizeof(float)));
  glEnableVertexAttribArray(3);

  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(float)*16, (void*)(12 * sizeof(float)));
  glEnableVertexAttribArray(4);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  free(mesh);

  // bones
  {
    modelsData[modelsDataSize].invBindMats = malloc(sizeof(Matrix) * data->skins->joints_count);

    fseek(fo, data->skins->inverse_bind_matrices->buffer_view->offset, SEEK_SET);
    fread(modelsData[modelsDataSize].invBindMats, data->skins->inverse_bind_matrices->buffer_view->size, 1, fo);

    modelsData[modelsDataSize].nodes = malloc(sizeof(GLTFNode) * data->nodes_count);
    modelsData[modelsDataSize].nodesSize = data->nodes_count;

    for (int i = 0; i < data->nodes_count; i++) {
      if (!data->nodes[i].parent) {
	modelsData[modelsDataSize].rootNode = i;
      }

      if (data->nodes[i].skin && data->nodes[i].mesh) {
	modelsData[modelsDataSize].parentNode = i;
      }
	    
      if (strcmp(data->nodes[i].name, "Head")==0) {
	modelsData[modelsDataSize].headNode = i;
      }

      if (strcmp(data->nodes[i].name, "Neck")==0) {
	modelsData[modelsDataSize].neckNode = i;
      }

      modelsData[modelsDataSize].nodes[i].name = malloc(sizeof(char) * (strlen(data->nodes[i].name) + 1));
      strcpy(modelsData[modelsDataSize].nodes[i].name, data->nodes[i].name);

      memcpy(modelsData[modelsDataSize].nodes[i].t, data->nodes[i].translation, sizeof(float) * 3);
      memcpy(modelsData[modelsDataSize].nodes[i].t + 3, data->nodes[i].scale, sizeof(float) * 3);
      memcpy(modelsData[modelsDataSize].nodes[i].t + 6, data->nodes[i].rotation, sizeof(float) * 4);
      /*
	modelsData[modelsDataSize].nodes[i].T = (vec3){data->nodes[i].translation[0],data->nodes[i].translation[1], data->nodes[i].translation[2]};
	modelsData[modelsDataSize].nodes[i].S = (vec3){data->nodes[i].scale[0], data->nodes[i].scale[1], data->nodes[i].scale[2]};
	modelsData[modelsDataSize].nodes[i].R = (vec4){data->nodes[i].rotation[0], data->nodes[i].rotation[1], data->nodes[i].rotation[2], data->nodes[i].rotation[3]};*/
    }

    for (int i = 0; i < data->nodes_count; i++) {
      modelsData[modelsDataSize].nodes[i].childSize = data->nodes[i].children_count;
      modelsData[modelsDataSize].nodes[i].child = malloc(sizeof(int) * data->nodes[i].children_count);

      modelsData[modelsDataSize].nodes[i].parent = -1;


      for (int i3 = 0; i3 < data->nodes_count; i3++) {
	if (data->nodes[i].parent && modelsData[modelsDataSize].nodes[i].parent == -1 &&
	    strcmp(data->nodes[i3].name, data->nodes[i].parent->name) == 0) {
	  modelsData[modelsDataSize].nodes[i].parent = i3;
	}

	for (int i2 = 0; i2 < data->nodes[i].children_count; i2++) {
	  if (strcmp(data->nodes[i3].name, data->nodes[i].children[i2]->name) == 0) {
	    modelsData[modelsDataSize].nodes[i].child[i2] = i3;
	    break;
	  }
	}
      }
    }

    //	updateNodes(modelsData[modelsDataSize].rootNode, -1, modelsDataSize);
    updateNodes(modelsData[modelsDataSize].rootNode, -1, &modelsData[modelsDataSize].nodes);

    glUseProgram(shadersId[animShader]);

    modelsData[modelsDataSize].jointsIdxsSize = data->skins->joints_count;
    modelsData[modelsDataSize].jointsIdxs = malloc(sizeof(uint8_t) * data->skins->joints_count);

    for (int i = 0; i < data->skins->joints_count; i++) {
      for (int i2 = 0; i < data->nodes_count; i2++) {
	if (strcmp(modelsData[modelsDataSize].nodes[i2].name, data->skins->joints[i]->name) == 0) {
	  modelsData[modelsDataSize].jointsIdxs[i] = i2;
	  break;
	};
      }
    }
	
  }

  // anims
  {
    static char* strType[] = {[cgltf_interpolation_type_linear] = "Linear",
      [cgltf_interpolation_type_step] = "Step",
      [cgltf_interpolation_type_cubic_spline] = "Cubic", [cgltf_interpolation_type_cubic_spline+1] = "ERROR" };

    static char* actionTypeStr[] = { [cgltf_animation_path_type_invalid] = "INVALID" ,
      [cgltf_animation_path_type_translation] = "Translation",
      [cgltf_animation_path_type_rotation] = "Rotation",
      [cgltf_animation_path_type_scale] = "Scale",
      [cgltf_animation_path_type_weights] = "Weithg",
      [cgltf_animation_path_type_weights+1] = "ERROR" };


    {
      modelsData[modelsDataSize].animSize = data->animations_count;
      modelsData[modelsDataSize].channels = malloc(sizeof(AnimChannel*)
						   *data->animations_count);
      modelsData[modelsDataSize].channelsSize = malloc(sizeof(int)
						       *data->animations_count);
      modelsData[modelsDataSize].animNames= malloc(sizeof(char*)
						   * data->animations_count);

	
      for(int i=0;i<data->animations_count;i++){
	int animIndex = -1;

	if(strcmp(data->animations[i].name,"Idle")==0){
	  animIndex= idleAnim;
	}else if(strcmp(data->animations[i].name,"Pick")==0){
	  animIndex= pickAnim;
	}else if(strcmp(data->animations[i].name,"Walk")==0){
	  animIndex= walkAnim;
	}else if(strcmp(data->animations[i].name,"Strafe")==0){
	  animIndex= strafeAnim;
	}else if(strcmp(data->animations[i].name,"SitIdle")==0){
	  animIndex= sitAnim;
	}else if(strcmp(data->animations[i].name,"Turn")==0){
	  animIndex= turnAnim;
	}else if (strcmp(data->animations[i].name, "ToSit") == 0) {
	  animIndex = toSitAnim;
	}
		
	modelsData[modelsDataSize].animNames[animIndex] = malloc(sizeof(char)*(strlen(data->animations[i].name)+1));
	strcpy(modelsData[modelsDataSize].animNames[animIndex], data->animations[i].name);
	    
	modelsData[modelsDataSize].channelsSize[animIndex] = data->animations[i].channels_count;
	modelsData[modelsDataSize].channels[animIndex] = malloc(sizeof(AnimChannel) * modelsData[modelsDataSize].channelsSize[animIndex]);

	AnimChannel* channels = modelsData[modelsDataSize].channels[animIndex];
		
	for(int i2=0;i2<modelsData[modelsDataSize].channelsSize[animIndex];i2++){
	  int nodeId = -1;

	  for(int i3=0;i3<modelsData[modelsDataSize].jointsIdxsSize;i3++){
	    nodeId = modelsData[modelsDataSize].jointsIdxs[i3];
			
	    if(strcmp(modelsData[modelsDataSize].nodes[nodeId].name,
		      data->animations[i].channels[i2].target_node->name)==0){
	      break;
	    };
	  }
		    
	  channels[i2].interpolation = data->animations[i].channels[i2].sampler->interpolation;
	  channels[i2].path = data->animations[i].channels[i2].target_path;
	  channels[i2].nodeId = nodeId;
		    
	  channels[i2].samplesSize = data->animations[i].channels[i2].sampler->input->count;
	  channels[i2].samples = malloc(sizeof(AnimSample) * (channels[i2].samplesSize));
		    
	  int vecLen = GLTFmapSize[data->animations[i].channels[i2].sampler->output->type];
		    
	  for (int i3 = 0; i3 < channels[i2].samplesSize; i3++) {
	    elementSize = GLTFtypeSize[data->animations[i].channels[i2].sampler->output->component_type];
			
	    fseek(fo, data->animations[i].channels[i2].sampler->output->buffer_view->offset + i3 * (elementSize*vecLen), SEEK_SET);
	    fread(channels[i2].samples[i3].data, (elementSize*vecLen), 1, fo);

	    elementSize = GLTFtypeSize[data->animations[i].channels[i2].sampler->input->component_type];
	    fseek(fo, data->animations[i].channels[i2].sampler->input->buffer_view->offset + i3 * elementSize, SEEK_SET);
	    fread(&channels[i2].samples[i3].time, elementSize, 1, fo);
	  }
	}

	/*
	  for (int i2 = 0; i2<channelsSize; i2++) {
	  printf("Node: %d Path: %s Inter: %s -> \n", channels[i2].nodeId,
	  actionTypeStr[channels[i2].path], strType[channels[i2].interpolation]);

	  for(int i3=0;i3<channels[i2].samplesSize;i3++){
	  printf("   Time: %f Data:", channels[i2].samples[i3].time);

	  for(int i4=0;i4<4;i4++){
	  printf("%f ", channels[i2].samples[i3].data[i4]);
	  }

	  printf("\n");
	  }
	  }
	*/


      }
    }
  }

  // texture
  {
    sprintf(buf, "%s%s", objsFolder, data->textures->image->uri);
    SDL_Surface* texture = IMG_Load(buf);
    
    if (!texture) {
      printf("Loading of texture \"%s\" failed", buf);
      exit(0);
    }

    createTexture(&modelsData[modelsDataSize].tx, texture->w,texture->h, texture->pixels);
  
    SDL_FreeSurface(texture);
  }

  modelsDataSize++;
  cgltf_free(data);
}

ObjectInfo* objectsInfo;
int objectsInfoSize;

Object* objects;
int objectsSize;



void loadGLTFScene(char* name){
  //  glUseProgram(shadersId[mainShader]);
  //    radius+=0.1f;
  uniformFloat(mainShader, "radius", radius);
  uniformFloat(waterShader, "radius", radius);
  uniformFloat(skyboxShader, "radius", radius);
		    
  size_t sceneWeight = 0;
    
  // free beofre run
  if(objectsInfoSize != 0){
    for(int i=0;i<objectsInfoSize;i++){
      for(int i2=0;i2<objectsInfo[i].meshesSize;i2++){
	glDeleteVertexArrays(1, &objectsInfo[i].meshes[i2].VAO);
	glDeleteBuffers(1, &objectsInfo[i].meshes[i2].VBO);
	glDeleteTextures(1, &objectsInfo[i].meshes[i2].tx);
      }

      //free(objectsInfo[i].name);
      free(objectsInfo[i].meshes);
    }

    free(objectsInfo);
    free(objects);
    free(waterSurfaces);
    free(lightsStorage);
    free(aabbEntities);
    free(tempModelIdxs);

    objectsInfoSize = 0;
    waterSurfacesSize = 0;
    objectsSize = 0;
    lightsStorageSize = 0;
    aabbEntitiesSize = 0;

    waterSurfaces = NULL;
    aabbEntities = NULL;
    objectsInfo = NULL;
    objects = NULL;
    lightsStorage = NULL;
    tempModelIdxs = NULL;
  }

    
  cgltf_options options = {0};
  cgltf_data* data = NULL;
  cgltf_result result = cgltf_parse_file(&options, name, &data);

  if (result != cgltf_result_success){
    exit(-1);
  }

  FILE* fo = NULL;
  char buf[128];    

  // get path of .bin file
  {
    strcpy(buf, name);

    for (int i = 1; i < strlen(buf); i++) {
      if (buf[i] == '.') {
	buf[i] = '\0';
	strcat(buf, ".bin");
	break;
      }
    }

    fo = fopen(buf, "rb");
  }

  size_t maxBufSize = 0;

  for(int i=0;i<data->meshes_count;++i){
    //aabbEntitiesSize += data->meshes[i].primitives_count;
    //	assert(data->meshes[i].primitives_count == 1);
	
    for(int i2=0;i2<data->meshes[i].primitives_count;++i2){
	    
      size_t curPrimitiveSize = 0;
	    
      for(int i3=0;i3<data->meshes[i].primitives[i2].attributes_count;++i3){
	int vecLen = GLTFmapSize[data->meshes[i].primitives[i2].attributes[i3].data->type];
		
	curPrimitiveSize += (vecLen);
      }
	    
      curPrimitiveSize *= data->meshes[i].primitives[i2].indices->count;
      maxBufSize = max(maxBufSize, curPrimitiveSize);
    }
  }

   
    
  float* mesh = malloc(maxBufSize * sizeof(float));
    
  int* uniqueMeshesVertexCountMap = malloc(data->nodes_count * sizeof(int));
  //    int uniqueMeshesVertexCountMapSize = 0;

  int* meshesIndx = malloc(data->nodes_count * sizeof(int));
  int* corespondingObjectInfo = malloc(data->nodes_count * sizeof(int));
  int* objectsInfoNodeIds = malloc(data->nodes_count * sizeof(int));

  int objectsPreSize = 0;
  //    int meshesSize = 0;

  lightsStorage = malloc(sizeof(Light2) * data->lights_count);

  //    int objsInfo = 0;
    
  for(int i=0;i<data->nodes_count;++i){
    if (data->nodes[i].light) {
      float t[10] = {0};

      t[0] = data->nodes[i].translation[0];
      t[1] = data->nodes[i].translation[1];
      t[2] = data->nodes[i].translation[2];

      t[3] = data->nodes[i].scale[0];
      t[4] = data->nodes[i].scale[1];
      t[5] = data->nodes[i].scale[2];

      t[6] = data->nodes[i].rotation[0];
      t[7] = data->nodes[i].rotation[1];
      t[8] = data->nodes[i].rotation[2];
      t[9] = data->nodes[i].rotation[3];
	    
      Matrix transformMat = gltfTRS(t);
		
      vec3 color = { data->nodes[i].light->color[0], data->nodes[i].light->color[1], data->nodes[i].light->color[2] };
      createLight(color,
		  data->nodes[i].light->type,
		  data->nodes[i].light->intensity, transformMat.m);

      continue;
    }
	
    bool exist = false;
    int idxLen = data->nodes[i].mesh->primitives->indices->count;
    int corespondingId = -1;

    for (int i2 = 0; i2 < data->nodes[i].mesh->primitives_count; ++i2) {
      if(data->nodes[i].mesh->primitives[i2].attributes_count != 0){
	aabbEntitiesSize++;
      }
    }

    for(int i2=0;i2<objectsInfoSize;++i2){
      if(uniqueMeshesVertexCountMap[i2] == idxLen) {
	exist = true;
	corespondingId = i2;
	break;
      }
    }
	
    if(!exist){
      uniqueMeshesVertexCountMap[objectsInfoSize] = idxLen;
      objectsInfoNodeIds[objectsInfoSize] = i;
	    
      corespondingId=objectsInfoSize;
	    
      objectsInfoSize++;
    }

    corespondingObjectInfo[objectsPreSize] = corespondingId;
    meshesIndx[objectsPreSize] = i;
    objectsPreSize++;
  }

  aabbEntities = malloc(sizeof(AABBEntity) * aabbEntitiesSize);
  aabbEntitiesSize = 0;


  objectsInfo = malloc(sizeof(Object) * objectsInfoSize);

  int bufSize = 0;

  for(int i=0;i<objectsInfoSize;++i){
    int nodeIndex = objectsInfoNodeIds[i];

    {
      objectsInfo[i].meshesSize = data->nodes[nodeIndex].mesh->primitives_count;
      objectsInfo[i].meshes = malloc(sizeof(Mesh) * objectsInfo[i].meshesSize);

      for(int i2=0;i2<objectsInfo[i].meshesSize;++i2){
	size_t attrPad = 0;

	for(int i4=0;i4<data->nodes[nodeIndex].mesh->primitives[i2].attributes_count;++i4){			   
	  int vecLen = GLTFmapSize[data->nodes[nodeIndex].mesh->primitives[i2].attributes[i4].data->type];
	  attrPad += vecLen;
	}

	sceneWeight += attrPad * data->nodes[nodeIndex].mesh->primitives[i2].indices->count;

	printf("Attr size: %d \n", attrPad);

	int maxUsedSize = 0;

	// copy of pos buffer
	int copyPosIndex = 0;
	{
	  assert(data->nodes[nodeIndex].mesh->primitives[i2].attributes->type == cgltf_attribute_type_position);
	  int size = 3 * data->nodes[nodeIndex].mesh->primitives[i2].indices->count * sizeof(float);
	  objectsInfo[i].meshes[i2].posBuf = malloc(size);
	  objectsInfo[i].meshes[i2].posBufSize = size / sizeof(float);
	}

	int indexSize = GLTFtypeSize[data->nodes[nodeIndex].mesh->primitives[i2].indices->component_type];
	bufSize += data->nodes[nodeIndex].mesh->primitives[i2].attributes->data->buffer_view->size;
	for (int i3 = 0; i3 < data->nodes[nodeIndex].mesh->primitives[i2].indices->count;++i3) {
	  uint16_t index;
	  fseek(fo, data->nodes[nodeIndex].mesh->primitives[i2].indices->buffer_view->offset + i3 * indexSize, SEEK_SET);
	  fread(&index, indexSize, 1, fo);

	  for(int i4=0;i4<data->nodes[nodeIndex].mesh->primitives[i2].attributes_count;++i4){
	    int compSize = GLTFtypeSize[data->nodes[nodeIndex].mesh->primitives[i2].attributes[i4].data->component_type];
	    int vecLen = GLTFmapSize[data->nodes[nodeIndex].mesh->primitives[i2].attributes[i4].data->type];

	    /*
	      int index = (i3*attrPad)+GLTFattrPad[data->nodes[nodeIndex].mesh->primitives[i2].attributes[i4].type];
	      fseek(fo, data->nodes[nodeIndex].mesh->primitives[i2].attributes[i4].data->buffer_view->offset
	      + compSize*(vecLen * index), SEEK_SET);
	      fread(&mesh[index], compSize*vecLen, 1, fo);*/


	    for (int i5 = 0; i5 < vecLen; ++i5) {
	      fseek(fo, data->nodes[nodeIndex].mesh->primitives[i2].attributes[i4].data->buffer_view->offset
		    + compSize * (vecLen * index + i5), SEEK_SET);

	      int index = (i3 * attrPad) + i5 + GLTFattrPad[data->nodes[nodeIndex].mesh->primitives[i2].attributes[i4].type];

	      if (data->nodes[nodeIndex].mesh->primitives[i2].attributes[i4].data->component_type
		  != cgltf_component_type_r_32f) {
		uint8_t temp;
		fread(&temp, compSize, 1, fo);

		mesh[index] = (float)temp;
	      }
	      else {
		fread(&mesh[index], compSize, 1, fo);

		if(data->nodes[nodeIndex].mesh->primitives[i2].attributes[i4].type == cgltf_attribute_type_position){
		  objectsInfo[i].meshes[i2].posBuf[copyPosIndex] = mesh[index];
		  copyPosIndex++;
		}
	      }
	    }
	  }
	}
		    
	glGenVertexArrays(1, &objectsInfo[i].meshes[i2].VAO);
	glGenBuffers(1, &objectsInfo[i].meshes[i2].VBO);

	glBindVertexArray(objectsInfo[i].meshes[i2].VAO);
	glBindBuffer(GL_ARRAY_BUFFER, objectsInfo[i].meshes[i2].VBO);
    
	objectsInfo[i].meshes[i2].VBOSize = data->nodes[nodeIndex].mesh->primitives[i2].indices->count;
	glBufferData(GL_ARRAY_BUFFER, attrPad * data->nodes[nodeIndex].mesh->primitives[i2].indices->count * sizeof(float)
		     , mesh, GL_STATIC_DRAW);

	int itemPad[] = { 3, 2, 3, 4, 4};

	int pad = 0;
	for(int i3=0;i3<data->nodes[nodeIndex].mesh->primitives[i2].attributes_count;++i3){
	  glVertexAttribPointer(i3, itemPad[i3], GL_FLOAT, GL_FALSE, attrPad * sizeof(float), pad * sizeof(float));
	  glEnableVertexAttribArray(i3);

	  pad+= itemPad[i3];
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//		{

	//		    fseek(fo, data->nodes[nodeIndex].mesh->primitives[i2].attributes->data->buffer_view->offset, SEEK_SET);
	//		    fread(objectsInfo[i].meshes[i2].posBuf, data->nodes[nodeIndex].mesh->primitives[i2].attributes->data->buffer_view->size, 1, fo);
	//		}

	// texture
	objectsInfo[i].meshes[i2].VBOSize = data->nodes[nodeIndex].mesh->primitives[i2].indices->count;
				    
	{
	  sprintf(buf, "%s%s", assetsFolder,
		  data->nodes[nodeIndex].mesh->primitives[i2].material->pbr_metallic_roughness.base_color_texture.texture->image->uri);
	  SDL_Surface* texture = IMG_Load(buf);

    
	  if (!texture) {
	    printf("Loading of texture \"%s\" failed", buf);
	    exit(0);
	  }

	  sceneWeight += texture->w * texture->h * texture->format->BytesPerPixel;

	  createTexture(&objectsInfo[i].meshes[i2].tx, texture->w,texture->h, texture->pixels);
  
	  SDL_FreeSurface(texture);
	}
      }
    }
  }

  printf("bufSize: %d \n", bufSize);

    

  for(int i=0;i<objectsPreSize;++i){
    int index = meshesIndx[i];

    float t[10] = {0};

    t[0] = data->nodes[index].translation[0];
    t[1] = data->nodes[index].translation[1];
    t[2] = data->nodes[index].translation[2];

    t[3] = data->nodes[index].scale[0];
    t[4] = data->nodes[index].scale[1];
    t[5] = data->nodes[index].scale[2];

    t[6] = data->nodes[index].rotation[0];
    t[7] = data->nodes[index].rotation[1];
    t[8] = data->nodes[index].rotation[2];
    t[9] = data->nodes[index].rotation[3];

    Object** targetBuffer = NULL;
    int* targetBufferSize = NULL;

    bool isWater = false;
    if (data->nodes[index].name[0] == 'w') {
      char* token = strtok(data->nodes[index].name, ".");
      if (strcmp(token, "water") == 0) {
	isWater = true;
      }
    }

    if(isWater){
      targetBuffer = &waterSurfaces;
      targetBufferSize = &waterSurfacesSize;
    }
    else{
      targetBuffer = &objects;
      targetBufferSize = &objectsSize;
    }

    int targetBufSize = *targetBufferSize;

    if(!(*targetBuffer)){
      *targetBuffer = malloc(sizeof(Object));
    }else{
      *targetBuffer = realloc(*targetBuffer, sizeof(Object) * (targetBufSize+1));
    }
	    
    (*targetBuffer)[targetBufSize].mat = gltfTRS(t);
	
    // aabb here

    for(int i2=0;i2<data->nodes[index].mesh->primitives_count;++i2){
      if (data->nodes[index].mesh->primitives[i2].attributes_count == 0) continue;
      assert(data->nodes[index].mesh->primitives[i2].attributes->type == cgltf_attribute_type_position);
	
      int compSize = GLTFtypeSize[data->nodes[index].mesh->primitives[i2].attributes->data->component_type];
      int vecLen = GLTFmapSize[data->nodes[index].mesh->primitives[i2].attributes->data->type];

      int posLen =  data->nodes[index].mesh->primitives[i2].attributes->data->buffer_view->size / vecLen / compSize;

      float pos[3];
      int vecSize = vecLen * compSize;

      aabbEntities[aabbEntitiesSize].col.rt = (vec3){-FLT_MAX, -FLT_MAX, -FLT_MAX};
      aabbEntities[aabbEntitiesSize].col.lb = (vec3){FLT_MAX, FLT_MAX, FLT_MAX};

      //		aabbEntities[aabbEntitiesSize].col[1].rt = (vec3){ -FLT_MAX, -FLT_MAX, -FLT_MAX };
      //		aabbEntities[aabbEntitiesSize].col[1].lb = (vec3){ FLT_MAX, FLT_MAX, FLT_MAX };

      //		float height = (data->nodes[index].mesh->primitives[i2].attributes->data->min[1] + data->nodes[index].mesh->primitives[i2].attributes->data->max[1]) / 2.0f;
	    
      for (int i3 = 0; i3 < posLen; ++i3) {	
	fseek(fo, data->nodes[index].mesh->primitives[i2].attributes->data->buffer_view->offset + i3 * vecSize, SEEK_SET);
	fread(pos, vecSize, 1, fo);

	vec4 transfPos = mulmatvec4((*targetBuffer)[targetBufSize].mat, (vec4) { pos[0], pos[1], pos[2], 1.0f });
	int ix = 0;
		
	//		if(pos[1] >= height){
	//			ix = 1;
	//		}
		
	aabbEntities[aabbEntitiesSize].col.lb.x = min(aabbEntities[aabbEntitiesSize].col.lb.x, transfPos.x);
	aabbEntities[aabbEntitiesSize].col.lb.y = min(aabbEntities[aabbEntitiesSize].col.lb.y, transfPos.y);
	aabbEntities[aabbEntitiesSize].col.lb.z = min(aabbEntities[aabbEntitiesSize].col.lb.z, transfPos.z);

	aabbEntities[aabbEntitiesSize].col.rt.x = max(aabbEntities[aabbEntitiesSize].col.rt.x, transfPos.x);
	aabbEntities[aabbEntitiesSize].col.rt.y = max(aabbEntities[aabbEntitiesSize].col.rt.y, transfPos.y);
	aabbEntities[aabbEntitiesSize].col.rt.z = max(aabbEntities[aabbEntitiesSize].col.rt.z, transfPos.z);
      }

      /*
	aabbEntities[aabbEntitiesSize].rot = (vec4){  data->nodes[index].rotation[0],
	data->nodes[index].rotation[1],
	data->nodes[index].rotation[2],
	data->nodes[index].rotation[3]
	};
	    
	aabbEntities[aabbEntitiesSize].center = (vec3){
	(aabbEntities[aabbEntitiesSize].col.lb.x + aabbEntities[aabbEntitiesSize].col.rt.x) * 0.5f,
	(aabbEntities[aabbEntitiesSize].col.lb.y + aabbEntities[aabbEntitiesSize].col.rt.y) * 0.5f,
	(aabbEntities[aabbEntitiesSize].col.lb.z + aabbEntities[aabbEntitiesSize].col.rt.z) * 0.5f
	};

	aabbEntities[aabbEntitiesSize].extents = (vec3){
	(aabbEntities[aabbEntitiesSize].col.rt.x - aabbEntities[aabbEntitiesSize].col.lb.x) * 0.5f,
	(aabbEntities[aabbEntitiesSize].col.rt.y - aabbEntities[aabbEntitiesSize].col.lb.y) * 0.5f,
	(aabbEntities[aabbEntitiesSize].col.rt.z - aabbEntities[aabbEntitiesSize].col.lb.z) * 0.5f
	};*/

      aabbEntities[aabbEntitiesSize].buf = targetBuffer;
      aabbEntities[aabbEntitiesSize].bufId = targetBufSize;
      aabbEntitiesSize++;
    }

    //aabbEntities[aabbEntitiesSize].objectId = targetBufSize;
    //	aabbEntities[aabbEntitiesSize]. = targetBufSize;

    (*targetBuffer)[targetBufSize].infoId = corespondingObjectInfo[i];
    (*targetBufferSize)++;
  }

  printf("Pre pass objInfo %d objsSize %d\n", objectsInfoSize, objectsSize);

  free(mesh);
  fclose(fo);
  cgltf_free(data);
    
  tempModelIdxs = malloc(sizeof(int) * aabbEntitiesSize);
  tempModelsPos = malloc(sizeof(vec3) * aabbEntitiesSize);

  uniformLights();

  printf("Scene size: %f KB\n", ((float)(sceneWeight)/1000.0f));
  printf("ObjInfo %d Objs %d \n", objectsInfoSize, objectsSize);
}


void updateBones(Bone* cur, int i){
  if(cur->parent != -1){
    cur->mat = multiplymat4(bones[cur->parent].mat,cur->mat);

    printf("\n");
    printf("world %s: ", bonesNames[i]);
    for (int x = 0; x < 4; x++) {
      printf("\n");
      for (int y = 0; y < 4; y++) {
	printf("%f ", bones[i].mat.m[x*4+y]);
      }
    }

    printf("\n");
	
    printf("parent %s: ", bonesNames[cur->parent]);
    for (int x = 0; x < 4; x++) {
      printf("\n");
      for (int y = 0; y < 4; y++) {
	printf("%f ", bones[cur->parent].mat.m[x*4+y]);
      }
    }
		
    printf("\n");
  }

  for(int i=0;i<cur->childSize;i++){
    updateBones(&bones[cur->child[i]], i);
  }

}

// make render for shadow and for normal
void renderScene(GLuint curShader){
}

void checkMouseVSEntities(){  
  mouse.selectedThing = NULL;
  mouse.selectedType = 0;
}

void bindUIQuad(vec2 pos[6], uint8_t c[4], MeshBuffer* buf){
  float* finalBatch = malloc(sizeof(float) * 6 * 6);
    
  for(int i2=0;i2<6;i2++){
    finalBatch[i2*6+0] = pos[i2].x;
    finalBatch[i2*6+1] = pos[i2].z;
      
    finalBatch[i2*6+2] = c[0];
    finalBatch[i2*6+3] = c[1];
    finalBatch[i2*6+4] = c[2];
    finalBatch[i2*6+5] = c[3]; 
  }

  
  glGenBuffers(1, &buf->VBO);
  glGenVertexArrays(1, &buf->VAO);
  
  glBindVertexArray(buf->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, buf->VBO);

  buf->VBOsize = 6;
  
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 6, finalBatch, GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  free(finalBatch);
}


void bindUITri(vec2 pos[3], uint8_t c[4], MeshBuffer* buf){
  float* finalBatch = malloc(sizeof(float) * 6 * 3);
    
  for(int i2=0;i2<3;i2++){
    finalBatch[i2*6+0] = pos[i2].x;
    finalBatch[i2*6+1] = pos[i2].z;
      
    finalBatch[i2*6+2] = c[0];
    finalBatch[i2*6+3] = c[1];
    finalBatch[i2*6+4] = c[2];
    finalBatch[i2*6+5] = c[3]; 
  }

  
  glGenBuffers(1, &buf->VBO);
  glGenVertexArrays(1, &buf->VAO);
  
  glBindVertexArray(buf->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, buf->VBO);

  buf->VBOsize = 3;
  
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 3, finalBatch, GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  free(finalBatch);
}

void bindUIQuadTx(vec4 pos[6], MeshBuffer* buf){
  float* finalBatch = malloc(sizeof(float) * 6 * 4);
    
  for(int i2=0;i2<6;i2++){
    finalBatch[i2*4+0] = pos[i2].x;
    finalBatch[i2*4+1] = pos[i2].y;
      
    finalBatch[i2*4+2] = pos[i2].z;
    finalBatch[i2*4+3] = pos[i2].w;
  }

  
  glGenBuffers(1, &buf->VBO);
  glGenVertexArrays(1, &buf->VAO);
  
  glBindVertexArray(buf->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, buf->VBO);

  buf->VBOsize = 6;
  
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, finalBatch, GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  free(finalBatch);
}


void clearCurrentUI(){
  curUIBuf.VBOsize = 0;

  for(int i=0;i<curUIBuf.rectsSize;i++){ 
    if(curUIBuf.rects[i].input){
      inputCursorPos = 0;
      selectedTextInput2 = NULL; 
      
      if(curUIBuf.rects[i].input->buf){
	free(curUIBuf.rects[i].input->buf);
	curUIBuf.rects[i].input->buf = NULL;
      }
    }
  }
  
  curUIBuf.rectsSize = 0;
  curUIBuf.rects = NULL;
}


UIBuf* batchUI(UIRect2* rects, int rectsSize){
  UIBuf* uiBuf = malloc(sizeof(UIBuf));
  
  uiBuf->rectsSize = rectsSize;
  uiBuf->rects = rects;

  float* finalBatch = malloc(sizeof(float) * uiBuf->rectsSize * 6 * 6);
  
  for(int i=0;i<uiBuf->rectsSize;i++){
    int pad = i * 6 * 6;
    
    for(int i2=0;i2<6;i2++){
      finalBatch[pad + (i2)*6+0] = rects[i].pos[i2].x;
      finalBatch[pad + (i2)*6+1] = rects[i].pos[i2].z;
      
      finalBatch[pad + (i2)*6+2] = rects[i].c[0];
      finalBatch[pad + (i2)*6+3] = rects[i].c[1];
      finalBatch[pad + (i2)*6+4] = rects[i].c[2];
      finalBatch[pad + (i2)*6+5] = rects[i].c[3]; 
    }
  }

  glGenBuffers(1, &uiBuf->VBO);
  glGenVertexArrays(1, &uiBuf->VAO);
  
  glBindVertexArray(uiBuf->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, uiBuf->VBO);

  uiBuf->VBOsize = uiBuf->rectsSize * 6;
  
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * uiBuf->rectsSize * 6 * 6, finalBatch, GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  free(finalBatch);

  return uiBuf;
}

void allocateAstar(int gX, int gY, int gZ){
  closedList = malloc(sizeof(bool*) * gZ*3);

  for(int z=0;z<gZ*3;z++){
    closedList[z] = calloc(1,sizeof(bool) * gX*3);
  }
					    
  cellsDetails = malloc(sizeof(AstarCell*) * gZ*3);

  for(int z=0;z<gZ*3;z++){
    cellsDetails[z] = calloc(1,sizeof(AstarCell) * gX*3);
  }

  openCells = malloc(sizeof(AstarOpenCell) * (gZ*3) * (gX * 3));
}

void loadShaders(bool firstInit){    
  int longestName = 0;
    
  for (int i = 0; i < shadersCounter; i++) {
    int nameLen = strlen(shadersFileNames[i]) + 1;
    longestName = max(nameLen, longestName);
  }

  vertFileName = malloc(sizeof(char) * (longestName + strlen(".vert") + 1));
  fragFileName = malloc(sizeof(char) * (longestName + strlen(".frag") + 1));
  geomFileName = malloc(sizeof(char) * (longestName + strlen(".geom") + 1));

  bool noErrors = true;

  GLuint tempShadersId[shadersCounter];

  for (int i = 0; i < shadersCounter; i++) {
    int nameLen = strlen(shadersFileNames[i]) + 1;

    strcpy(vertFileName, shadersFileNames[i]);
    strcat(vertFileName, ".vert");

    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertFileName);

    strcpy(fragFileName, shadersFileNames[i]);
    strcat(fragFileName, ".frag");

    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragFileName);

    /*
      strcpy(geomFileName, shadersFileNames[i]); 
      strcat(geomFileName, ".geom");

      GLuint geometryShader = loadShader(GL_GEOMETRY_SHADER, geomFileName);
    */

    tempShadersId[i] = glCreateProgram();
    glAttachShader(tempShadersId[i], fragmentShader);
    glAttachShader(tempShadersId[i], vertexShader);

    /*
      if(geometryShader != 0){
      glAttachShader(tempShadersId[i], geometryShader);   
      }
    */

    glLinkProgram(tempShadersId[i]);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    /*
      if(geometryShader != 0){
      glDeleteShader(geometryShader);   
      }
    */


    // Check for linking errors
    GLint linkStatus;
    glGetProgramiv(tempShadersId[i], GL_LINK_STATUS, &linkStatus);

    if (linkStatus != GL_TRUE) {
      GLint logLength;
      glGetProgramiv(tempShadersId[i], GL_INFO_LOG_LENGTH, &logLength);
      char* log = (char*)malloc(logLength);
      glGetProgramInfoLog(tempShadersId[i], logLength, NULL, log);
      fprintf(stderr, "Failed to link \"%s\": %s\n", shadersFileNames[i], log);
      free(log);

      exit(0);
    }
  }

  memcpy(shadersId, tempShadersId, shadersCounter * sizeof(uint32_t));
}

void reloadShaders(){    
  bool noErrors = true;

  uint32_t tempShadersId[shadersCounter];

  for (int i = 0; i < shadersCounter; i++) {
    int nameLen = strlen(shadersFileNames[i]) + 1;

    strcpy(vertFileName, shadersFileNames[i]);
    strcat(vertFileName, ".vert");

    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertFileName);

    strcpy(fragFileName, shadersFileNames[i]);
    strcat(fragFileName, ".frag");

    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragFileName);

    tempShadersId[i] = glCreateProgram();
    glAttachShader(tempShadersId[i], fragmentShader);
    glAttachShader(tempShadersId[i], vertexShader);

    glLinkProgram(tempShadersId[i]);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Check for linking errors
    GLint linkStatus;
    glGetProgramiv(tempShadersId[i], GL_LINK_STATUS, &linkStatus);

    if (linkStatus != GL_TRUE) {
      GLint logLength;
      glGetProgramiv(tempShadersId[i], GL_INFO_LOG_LENGTH, &logLength);
      char* log = (char*)malloc(logLength);
      glGetProgramInfoLog(tempShadersId[i], logLength, NULL, log);
      fprintf(stderr, "Failed to link \"%s\": %s\n", shadersFileNames[i], log);
      free(log);

      noErrors = false;
    }
  }

  if(noErrors){
    glUseProgram(0);

    for (int i = 0; i < shadersCounter; i++) {
      glDeleteProgram(shadersId[i]);
    }
	
    memcpy(shadersId, tempShadersId, sizeof(tempShadersId));
    uniformLights();
  }
}

void createLight(vec3 color, int type, float power, float* mat){
  int newIndex = lightsStorageSize;
  lightsStorageSize++;

  /*
    if(!lightsStorage){
    lightsStorage = malloc(sizeof(Light2));
    }else{
    lightsStorage = realloc(lightsStorage, sizeof(Light2) * lightsStorageSize);
    }*/


  lightsStorage[newIndex].color = color;
  lightsStorage[newIndex].type = type;
  lightsStorage[newIndex].power = power  / 100.0f;
  memcpy(lightsStorage[newIndex].mat.m, mat, sizeof(Matrix));

  uniformLights();

  //  if(type == dirLightShadowT){
  //	rerenderShadowsForAllLights();
  //    }

  //  if (type == shadowLightT) {
  //rerenderShadowForLight(lightStorage[type][indexOfNew].id);
  //  }
}

void bindObjectsAABBBoxes(){
  // 36 * 3 * float
  int cubeSize = 432;
  int pad = cubeSize / sizeof(float);

  int meshSize = cubeSize * (aabbEntitiesSize); // + cubeSize for player AABB
   
  float* mesh = malloc(meshSize);
    
  for(int i=0;i<aabbEntitiesSize;++i){
    int index = pad * i;
	    
    float lowX = aabbEntities[i].col.lb.x;
    float lowY = aabbEntities[i].col.lb.y;
    float lowZ = aabbEntities[i].col.lb.z;

    float highX = aabbEntities[i].col.rt.x;
    float highY = aabbEntities[i].col.rt.y;
    float highZ = aabbEntities[i].col.rt.z;
	    
    float box[] = {
      // positions          
      lowX,  highY, lowZ,
      lowX, lowY, lowZ,
      highX, lowY, lowZ,
      highX, lowY, lowZ,
      highX,  highY, lowZ,
      lowX,  highY, lowZ,

      lowX, lowY,  highZ,
      lowX, lowY, lowZ,
      lowX,  highY, lowZ,
      lowX,  highY, lowZ,
      lowX,  highY,  highZ,
      lowX, lowY,  highZ,

      highX, lowY, lowZ,
      highX, lowY,  highZ,
      highX,  highY,  highZ,
      highX,  highY,  highZ,
      highX,  highY, lowZ,
      highX, lowY, lowZ,

      lowX, lowY,  highZ,
      lowX,  highY,  highZ,
      highX,  highY,  highZ,
      highX,  highY,  highZ,
      highX, lowY,  highZ,
      lowX, lowY,  highZ,

      lowX,  highY, lowZ,
      highX,  highY, lowZ,
      highX,  highY,  highZ,
      highX,  highY,  highZ,
      lowX,  highY,  highZ,
      lowX,  highY, lowZ,

      lowX, lowY, lowZ,
      lowX, lowY,  highZ,
      highX, lowY, lowZ,
      highX, lowY, lowZ,
      lowX, lowY,  highZ,
      highX, lowY,  highZ
    };
	
    memcpy(&mesh[index], box, sizeof(box));
  }

  glBindVertexArray(AABBBoxes.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, AABBBoxes.VBO);

  AABBBoxes.VBOsize = pad * (aabbEntitiesSize);

  glBufferData(GL_ARRAY_BUFFER, meshSize, mesh, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);

  free(mesh);
}

#define cubemapFolder "./assets/cubemap/"

void loadCubemap(){
  float step = 1000.0f;
  float skyboxVertices[] = {
    // positions          
    -step,  step, -step,
    -step, -step, -step,
    step, -step, -step,
    step, -step, -step,
    step,  step, -step,
    -step,  step, -step,

    -step, -step,  step,
    -step, -step, -step,
    -step,  step, -step,
    -step,  step, -step,
    -step,  step,  step,
    -step, -step,  step,

    step, -step, -step,
    step, -step,  step,
    step,  step,  step,
    step,  step,  step,
    step,  step, -step,
    step, -step, -step,

    -step, -step,  step,
    -step,  step,  step,
    step,  step,  step,
    step,  step,  step,
    step, -step,  step,
    -step, -step,  step,

    -step,  step, -step,
    step,  step, -step,
    step,  step,  step,
    step,  step,  step,
    -step,  step,  step,
    -step,  step, -step,

    -step, -step, -step,
    -step, -step,  step,
    step, -step, -step,
    step, -step, -step,
    -step, -step,  step,
    step, -step,  step
  };
      
  glGenVertexArrays(1, &skyboxVAO);
  glGenBuffers(1, &skyboxVBO);
  glBindVertexArray(skyboxVAO);
  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
    
  glGenTextures(1, &cubemapTexture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

  char* names[] = {
    "right.png",
    "left.png",
    "top.png",
    "bot.png",
    "front.png",
    "back.png",
  };

  int width, height, nrChannels;
  char buf[128];

    
  for (int i = 0; i < 6; ++i)
    {
      sprintf(buf, "%s%s", cubemapFolder, names[i]);
      SDL_Surface* texture = IMG_Load(buf);

      if (!texture) {
	printf("Loading of texture \"%s\" failed", buf);
	exit(0);
      }
	
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
		   0, GL_RGBA8, texture->w, texture->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->pixels);

      SDL_FreeSurface(texture);
    }
    
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

float entityVsMeshesAvg = 0;
int entityVsMeshesRuns = 0;

bool entityVsMeshes(vec3 entityPos, float legH, float entityR, float entityH, vec3* resPos){
  entityVsMeshesRuns++;
  uint64_t start_time = SDL_GetPerformanceCounter();
  
  bool valid = false;

  vec4 playerPoint = { argVec3(entityPos), 1.0f};
	
  for(int i=0;i<aabbEntitiesSize;++i){
    if(playerPoint.x > aabbEntities[i].col.lb.x && playerPoint.x < aabbEntities[i].col.rt.x &&
       playerPoint.y * 0.95f > aabbEntities[i].col.lb.y && playerPoint.y * 0.95f < aabbEntities[i].col.rt.y &&
       playerPoint.z > aabbEntities[i].col.lb.z && playerPoint.z < aabbEntities[i].col.rt.z)
      {
	tempModelIdxs[tempAABBSize] = i;
	tempAABBSize++;
      }
  }

  if(tempAABBSize != 0){
    float maxY = -FLT_MAX;
    vec3 finalPos;
	
    vec2 player2DPos = {playerPoint.x, playerPoint.z};// no height
	
    for(int i=0;i<tempAABBSize;++i){
      Object obj = (*(aabbEntities[tempModelIdxs[i]].buf))[aabbEntities[tempModelIdxs[i]].bufId];
      int infoId = obj.infoId;

      bool isInter = false;

      for(int i2=0;i2<objectsInfo[infoId].meshesSize;++i2){
	for(int i3=0;i3<objectsInfo[infoId].meshes[i2].posBufSize/9; i3++) {
	  int index = i3*3*3;
	  vec4 a = mulmatvec4(obj.mat, (vec4){objectsInfo[infoId].meshes[i2].posBuf[index+0], objectsInfo[infoId].meshes[i2].posBuf[index+1], objectsInfo[infoId].meshes[i2].posBuf[index+2], 1.0f});
	  vec4 b = mulmatvec4(obj.mat, (vec4){objectsInfo[infoId].meshes[i2].posBuf[index+3],objectsInfo[infoId].meshes[i2].posBuf[index+4]
					      ,objectsInfo[infoId].meshes[i2].posBuf[index+5], 1.0f});
	  vec4 c = mulmatvec4(obj.mat, (vec4){objectsInfo[infoId].meshes[i2].posBuf[index+6],objectsInfo[infoId].meshes[i2].posBuf[index+7]
					      ,objectsInfo[infoId].meshes[i2].posBuf[index+8], 1.0f});

	  float entityStepMax = entityPos.y + legH;
	  float entityStepMin = entityPos.y - legH;

	  bool invalidTri = min(a.y,min(b.y,c.y)) > entityStepMax || max(a.y,max(b.y,c.y)) < entityStepMin;

	  // if tri unreachable skip it
	  if(invalidTri){
	    continue;
	  }
		    
	  // can be faster
	  vec2 a1 = (vec2){ argVec2(a) };
	  vec2 b1 = (vec2){ argVec2(b) };
	  vec2 c1 = (vec2){ argVec2(c) };

	  vec2 tri2d[3] = {(vec2){ argVec2(a) }, (vec2){ argVec2(b) }, (vec2){ argVec2(c) }};

	  int A = triArea2D(a1, b1, c1);
	  int AA = A;
	  //	  float A = triArea2D(a1, b1, c1);
	  float sum = 0;
	  sum += triArea2D(player2DPos,a1,b1);
	  if((int)sum > A) continue;
	  
	  sum += triArea2D(player2DPos,b1,c1);
	  if((int)sum > A) continue;
	  
	  sum += triArea2D(player2DPos,c1,a1);
	  
	  if((int)sum == A){
	    vec3 a3 = {a.x, a.y, a.z};
	    vec3 b3 = {b.x, b.y, b.z};
	    vec3 c3 = {c.x, c.y, c.z};
				
	    printf("\nIntersected with: v1(%f %f %f) v2(%f %f %f) v3(%f %f %f)\n",
		   argVec3(a3),argVec3(b3),argVec3(c3));
				
	    printf("Player pos: v1(%f %f) \n", player2DPos.x, player2DPos.z);

	    vec3 interPos = interpolate2dTo3d(a3,b3,c3,player2DPos);


	    float minH = interPos.y - legH;
	    float maxH = interPos.y + legH;

	    float diff = max(entityPos.y, interPos.y) - min(entityPos.y, interPos.y);

	    if(maxY < interPos.y){
	      maxY=interPos.y;
	      if(diff <= legH && diff >= -legH){
		finalPos = interPos;

		float minH = entityPos.y + legH;
		float maxH = entityH;

		//		if(inCircle()){
		//valid = false;
		//		}

		valid = true;
		//		valid = true;
	      }
	      else {
		valid = false;
	      }
	    }
	    
	  }
	}
      }
    }

    if (valid) {
      if(triInterVisualiserBuf){
	free(triInterVisualiserBuf);
	triInterVisualiserBuf = NULL;
	triInterVisualiserBufSize=0;
      }

      if(triVisualiserBuf){
	free(triVisualiserBuf);
	triVisualiserBuf = NULL;
	triVisualiserBufSize=0;
      }

      if(linesVisualiserBuf){
	free(linesVisualiserBuf);
	linesVisualiserBuf = NULL;
	linesVisualiserBufSize=0;
      }

      if(lines2VisualiserBuf){
	free(lines2VisualiserBuf);
	lines2VisualiserBuf = NULL;
	lines2VisualiserBufSize=0;
      }
      
        float minH = finalPos.y + legH;
        float maxH = finalPos.y + entityH;

        vec3 cylinderP1 = { finalPos.x,minH,finalPos.z }; // low
        vec3 cylinderP2 = { finalPos.x,maxH,finalPos.z }; // high

       // float a = entityStorage[0][0].model->data->animSize;
        //vec3 rt = { finalPos.x + maxR,finalPos.y + entityStorage[0][0].model->data->size[1], finalPos.z + maxR };
        //vec3 lb = { finalPos.x - maxR,finalPos.y + (entityStorage[0][0].model->data->size[1] * 0.25f),finalPos.z - maxR };

        //vec3 lb = { finalPos.x - 1.0,finalPos.y + (entityStorage[0][0].model->data->size[1] * 0.25f),finalPos.z - 1.0 };
    
  
      
      AABB col = {0};
      col.lb.x = finalPos.x - entityR;
      col.lb.y = finalPos.y + legH;
      col.lb.z = finalPos.z - entityR;
    
      col.rt.x = finalPos.x + entityR;
      col.rt.y = finalPos.y + entityH;
      col.rt.z = finalPos.z + entityR;


      float minDist = FLT_MAX;
      float minDist2 = FLT_MAX;
      
      bool intersect = false;

      int intCounter = 0;

      for(int i=0;i<aabbEntitiesSize;++i){
	Object obj = (*(aabbEntities[i].buf))[aabbEntities[i].bufId];
	int infoId = obj.infoId;

	bool isInter = false;

	for(int i2=0;i2<objectsInfo[infoId].meshesSize;++i2){
	  for(int i3=0;i3<objectsInfo[infoId].meshes[i2].posBufSize/9; i3++) {
	    int index = i3*3*3;
	    vec4 a = mulmatvec4(obj.mat, (vec4){objectsInfo[infoId].meshes[i2].posBuf[index+0], objectsInfo[infoId].meshes[i2].posBuf[index+1], objectsInfo[infoId].meshes[i2].posBuf[index+2], 1.0f});
	    vec4 b = mulmatvec4(obj.mat, (vec4){objectsInfo[infoId].meshes[i2].posBuf[index+3],objectsInfo[infoId].meshes[i2].posBuf[index+4]
						,objectsInfo[infoId].meshes[i2].posBuf[index+5], 1.0f});
	    vec4 c = mulmatvec4(obj.mat, (vec4){objectsInfo[infoId].meshes[i2].posBuf[index+6],objectsInfo[infoId].meshes[i2].posBuf[index+7]
						,objectsInfo[infoId].meshes[i2].posBuf[index+8], 1.0f});

	    float halftH = (entityH - legH)/2.0f;
	    int triInter = AABBvsTri((vec3){argVec3(a)},(vec3){argVec3(b)},(vec3){argVec3(c)},
				     (vec3){finalPos.x, finalPos.y + legH + halftH, finalPos.z}, halftH, entityR);
	    
	    if(triInter){
	      triInterVisualiserBufSize++;

	      if(!triInterVisualiserBuf){
		triInterVisualiserBuf = malloc(sizeof(float) * 6 * 3);
	      }else{
		triInterVisualiserBuf = realloc(triInterVisualiserBuf, triInterVisualiserBufSize*sizeof(float) * 6 * 3);
	      }

	      int ix = (triInterVisualiserBufSize-1) * 6 * 3;
	      
	      triInterVisualiserBuf[ix]= a.x;
	      triInterVisualiserBuf[ix+1]= a.y;
	      triInterVisualiserBuf[ix+2]= a.z;

	      triInterVisualiserBuf[ix+3]= b.x;
	      triInterVisualiserBuf[ix+4]= b.y;
	      triInterVisualiserBuf[ix+5]= b.z;

	      triInterVisualiserBuf[ix+6]= b.x;
	      triInterVisualiserBuf[ix+7]= b.y;
	      triInterVisualiserBuf[ix+8]= b.z;

	      triInterVisualiserBuf[ix+9]= c.x;
	      triInterVisualiserBuf[ix+10]= c.y;
	      triInterVisualiserBuf[ix+11]= c.z;

	      triInterVisualiserBuf[ix+12]= c.x;
	      triInterVisualiserBuf[ix+13]=c.y;
	      triInterVisualiserBuf[ix+14]=c.z;

	      triInterVisualiserBuf[ix+15]= a.x;
	      triInterVisualiserBuf[ix+16]= a.y;
	      triInterVisualiserBuf[ix+17]= a.z;
	      printf("Intersected with tri \n");
	    }else{
	      triVisualiserBufSize++;

	      if(!triVisualiserBuf){
		triVisualiserBuf = malloc(sizeof(float) * 6 * 3);
	      }else{
		triVisualiserBuf = realloc(triVisualiserBuf, triVisualiserBufSize*sizeof(float) * 6 * 3);
	      }

	      int ix = (triVisualiserBufSize-1) * 6 * 3;
	      
	      triVisualiserBuf[ix]= a.x;
	      triVisualiserBuf[ix+1]= a.y;
	      triVisualiserBuf[ix+2]= a.z;

	      triVisualiserBuf[ix+3]= b.x;
	      triVisualiserBuf[ix+4]= b.y;
	      triVisualiserBuf[ix+5]= b.z;

	      triVisualiserBuf[ix+6]= b.x;
	      triVisualiserBuf[ix+7]= b.y;
	      triVisualiserBuf[ix+8]= b.z;

	      triVisualiserBuf[ix+9]= c.x;
	      triVisualiserBuf[ix+10]= c.y;
	      triVisualiserBuf[ix+11]= c.z;

	      triVisualiserBuf[ix+12]= c.x;
	      triVisualiserBuf[ix+13]=c.y;
	      triVisualiserBuf[ix+14]=c.z;

	      triVisualiserBuf[ix+15]= a.x;
	      triVisualiserBuf[ix+16]= a.y;
	      triVisualiserBuf[ix+17]= a.z;
	    }

	    /*
	    vec4 edges[3][2] = {{a,b},{b,c},{a,c}};

	    float maxY = max(a.y,max(b.y,c.y));
	    float minY = min(a.y,min(b.y,c.y));

	    if(minY<=maxH && maxY>=minH){
	      //     if(true){
	      for(int i4=0;i4<3;++i4){
		vec2 a1 = {edges[i4][0].x, edges[i4][0].z};
		vec2 b1 = {edges[i4][1].x, edges[i4][1].z};


		vec3 dir = { edges[i4][1].x - edges[i4][0].x, edges[i4][1].y - edges[i4][0].y, edges[i4][1].z - edges[i4][0].z };
        dir = normalize3(dir);

		float distPow = (edges[i4][1].x - edges[i4][0].x) * (edges[i4][1].x - edges[i4][0].x) +
		  (edges[i4][1].y - edges[i4][0].y) * (edges[i4][1].y - edges[i4][0].y) +
		  (edges[i4][1].z - edges[i4][0].z) * (edges[i4][1].z - edges[i4][0].z);

		float dist = 0;
		bool inter = false;
		if(rayIntersectsTriangle((vec3){argVec3(edges[i4][0])}, dir, col.lb, col.rt, NULL, &dist)){
		  if(dist * dist < distPow){
		  inter = true;
		  
		  printf("%f %f %f dir\n", argVec3(dir));
		    intCounter++;

		    linesVisualiserBufSize++;

		    if(!linesVisualiserBuf){
		      linesVisualiserBuf = malloc(sizeof(float) * 6);
		    }else{
		      linesVisualiserBuf = realloc(linesVisualiserBuf, linesVisualiserBufSize*sizeof(float) * 6);
		    }

		    int ix = (linesVisualiserBufSize-1) * 6;
		    linesVisualiserBuf[ix]= edges[i4][0].x;
		    linesVisualiserBuf[ix+1]= edges[i4][0].y;
		    linesVisualiserBuf[ix+2]= edges[i4][0].z;

		    linesVisualiserBuf[ix+3]= edges[i4][1].x;
		    linesVisualiserBuf[ix+4]= edges[i4][1].y;
		    linesVisualiserBuf[ix+5]= edges[i4][1].z;
		  }

		    
		    //      }
		}

		if(!inter){
		  {
		    lines2VisualiserBufSize++;

		    if(!lines2VisualiserBuf){
		      lines2VisualiserBuf = malloc(sizeof(float) * 6);
		    }else{
		      lines2VisualiserBuf = realloc(lines2VisualiserBuf, lines2VisualiserBufSize*sizeof(float) * 6);
		    }

		    int ix = (lines2VisualiserBufSize-1) * 6;
		    lines2VisualiserBuf[ix]= edges[i4][0].x;
		    lines2VisualiserBuf[ix+1]= edges[i4][0].y;
		    lines2VisualiserBuf[ix+2]= edges[i4][0].z;

		    lines2VisualiserBuf[ix+3]= edges[i4][1].x;
		    lines2VisualiserBuf[ix+4]= edges[i4][1].y;
		    lines2VisualiserBuf[ix+5]= edges[i4][1].z;
		  }

		}
		
	      }
	    }*/


	    
	  }
	}
      }

      printf("inters %d \n", intCounter);

      if(valid){
	entityPos = finalPos;
      }
    }
  }

  {
    glBindVertexArray(triVisualiser.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, triVisualiser.VBO);

    triVisualiser.VBOsize = triVisualiserBufSize * 6;

    glBufferData(GL_ARRAY_BUFFER, triVisualiserBufSize*sizeof(float) * 6 * 3, triVisualiserBuf, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);
  }

  {
    glBindVertexArray(triInterVisualiser.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, triInterVisualiser.VBO);

    triInterVisualiser.VBOsize = triInterVisualiserBufSize * 6;

    glBufferData(GL_ARRAY_BUFFER, triInterVisualiserBufSize*sizeof(float) * 6 * 3, triInterVisualiserBuf, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);
  }

  {
    glBindVertexArray(lines2Visualiser.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, lines2Visualiser.VBO);

    lines2Visualiser.VBOsize = lines2VisualiserBufSize * 2;

    glBufferData(GL_ARRAY_BUFFER, lines2VisualiserBufSize*sizeof(float) * 6, lines2VisualiserBuf, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);
  }

  {
    glBindVertexArray(lines2Visualiser.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, lines2Visualiser.VBO);

    lines2Visualiser.VBOsize = lines2VisualiserBufSize * 2;

    glBufferData(GL_ARRAY_BUFFER, lines2VisualiserBufSize*sizeof(float) * 6, lines2VisualiserBuf, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);
  }

  *resPos = entityPos;

  tempAABBSize = 0;
  
  float delta = SDL_GetPerformanceCounter()-start_time;
  delta /= (float)SDL_GetPerformanceFrequency();

  entityVsMeshesAvg += delta;
  printf("entityVsMeshes: %f Avg: %f \n", delta, entityVsMeshesAvg / entityVsMeshesRuns);
  
  return valid;// (vec3){argVec3(entityPos)};
}


void bindCylindersAroundEntities(){
  /*entityStorage[0][0].col.lb.x = FLT_MAX;
  entityStorage[0][0].col.lb.y = FLT_MAX;
  entityStorage[0][0].col.lb.z = FLT_MAX;
    
  entityStorage[0][0].col.rt.x = -FLT_MAX;
  entityStorage[0][0].col.rt.y = -FLT_MAX;
  entityStorage[0][0].col.rt.z = -FLT_MAX;
  */
  /*for(int i=0;i<entityStorage[0][0].model->data->idxNum/3;++i){
    int index = i * 3;

    float x = entityStorage[0][0].model->data->rawBuf[index];
    float y = entityStorage[0][0].model->data->rawBuf[index+1];
    float z = entityStorage[0][0].model->data->rawBuf[index+2];

    vec4 v = mulmatvec4(entityStorage[0][0].mat, (vec4){x,y,z,1.0f});

    entityStorage[0][0].col.lb.x = min(entityStorage[0][0].col.lb.x, v.x);
    entityStorage[0][0].col.lb.y = min(entityStorage[0][0].col.lb.y, v.y);
    entityStorage[0][0].col.lb.z = min(entityStorage[0][0].col.lb.z, v.z);

    entityStorage[0][0].col.rt.x = max(entityStorage[0][0].col.rt.x, v.x);
    entityStorage[0][0].col.rt.y = max(entityStorage[0][0].col.rt.y, v.y);
    entityStorage[0][0].col.rt.z = max(entityStorage[0][0].col.rt.z, v.z);
  }*/

  float maxR = max(entityStorage[0][0].model->data->size[0],entityStorage[0][0].model->data->size[2]) / 2.0f;
  
  entityStorage[0][0].col.lb.x = entityStorage[0][0].mat.m[12] - maxR;
  entityStorage[0][0].col.lb.y = entityStorage[0][0].mat.m[13] + (entityStorage[0][0].model->data->size[1] * 0.25f);
  entityStorage[0][0].col.lb.z = entityStorage[0][0].mat.m[14] - maxR;
    
  entityStorage[0][0].col.rt.x = entityStorage[0][0].mat.m[12] + maxR;
  entityStorage[0][0].col.rt.y = entityStorage[0][0].mat.m[13] + entityStorage[0][0].model->data->size[1];
  entityStorage[0][0].col.rt.z = entityStorage[0][0].mat.m[14] + maxR;

  float lowX = entityStorage[0][0].col.lb.x;
  float lowY = entityStorage[0][0].col.lb.y;
  float lowZ = entityStorage[0][0].col.lb.z;

  float highX = entityStorage[0][0].col.rt.x;
  float highY = entityStorage[0][0].col.rt.y;
  float highZ = entityStorage[0][0].col.rt.z;
	    
  float box[] = {
    // positions          
    lowX,  highY, lowZ,
    lowX, lowY, lowZ,
    highX, lowY, lowZ,
    highX, lowY, lowZ,
    highX,  highY, lowZ,
    lowX,  highY, lowZ,

    lowX, lowY,  highZ,
    lowX, lowY, lowZ,
    lowX,  highY, lowZ,
    lowX,  highY, lowZ,
    lowX,  highY,  highZ,
    lowX, lowY,  highZ,

    highX, lowY, lowZ,
    highX, lowY,  highZ,
    highX,  highY,  highZ,
    highX,  highY,  highZ,
    highX,  highY, lowZ,
    highX, lowY, lowZ,

    lowX, lowY,  highZ,
    lowX,  highY,  highZ,
    highX,  highY,  highZ,
    highX,  highY,  highZ,
    highX, lowY,  highZ,
    lowX, lowY,  highZ,

    lowX,  highY, lowZ,
    highX,  highY, lowZ,
    highX,  highY,  highZ,
    highX,  highY,  highZ,
    lowX,  highY,  highZ,
    lowX,  highY, lowZ,

    lowX, lowY, lowZ,
    lowX, lowY,  highZ,
    highX, lowY, lowZ,
    highX, lowY, lowZ,
    lowX, lowY,  highZ,
    highX, lowY,  highZ
  };
	
  //  memcpy(&mesh[index], box, sizeof(box));

  glBindVertexArray(cylinderMesh.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, cylinderMesh.VBO);

  cylinderMesh.VBOsize = sizeof(box) / 3 / sizeof(float);

  glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);

  /*
    int segments = 7;

    int cylinderSize = (segments+1) * 6 * sizeof(float);
  float* cylinder = malloc(cylinderSize);

  float legH = entityStorage[0][0].model->data->size[1] / 4.0f;
  float height = entityStorage[0][0].model->data->size[1] - legH;

  float radius = max(entityStorage[0][0].model->data->size[0], entityStorage[0][0].model->data->size[2])/2.0f;

  vec3 pos = {entityStorage[0][0].mat.m[12], entityStorage[0][0].mat.m[13], entityStorage[0][0].mat.m[14]};
  int index = 0;
  float angleStep = 2.0f * M_PI / segments;

  // Top circle vertices
  for (int i = 0; i < segments; ++i) {
    float angle = i * angleStep;
    float x = radius * cos(angle) + pos.x;
    float z = radius * sin(angle) + pos.z;
        
    // Top circle vertex
    cylinder[index] = x;
    cylinder[index + 1] = height + pos.y + legH;
    cylinder[index + 2] = z;

    // Bottom circle vertex
    cylinder[index + 3] = x;
    cylinder[index + 4] = 0.0f + pos.y + legH;
    cylinder[index + 5] = z;

    index += 6;
  }

  float x = radius * cos(0) + pos.x;
  float z = radius * sin(0) + pos.z;

  cylinder[index] = x;
  cylinder[index + 1] = height + pos.y + legH;
  cylinder[index + 2] = z;

  cylinder[index + 3] = x;
  cylinder[index + 4] = 0.0f + pos.y + legH;
  cylinder[index+5] = z;

  glBindVertexArray(cylinderMesh.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, cylinderMesh.VBO);

  cylinderMesh.VBOsize = (segments + 1) * 2;

  glBufferData(GL_ARRAY_BUFFER, (segments+1) * 6 * sizeof(float), cylinder, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);

  free(cylinder);
  */
}

