#include "deps.h"
#include "linearAlg.h"
#include "main.h"
#include "editor.h"
#include "game.h"

#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"

bool showHiddenWalls = true;

GeomFin* finalGeom;

int renderCapYLayer;
EngineInstance curInstance = editorInstance;

int navPointsSize;
NavCornerPoint* navPoints;

VPair navPointsMesh;
VPair navPointsConnMesh;

bool navPointsDraw = false;

Wall** wallsStorage;
int wallsStorageSize;

TileBlock** blocksStorage;
int blocksStorageSize;

Tile** tilesStorage;
int tilesStorageSize;

MeshBuffer doorDoorPlane;

//const int SHADOW_WIDTH = 128, SHADOW_HEIGHT = 128;
const int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;  
unsigned int depthMapFBO;
unsigned int depthMaps;

void glErrorCheck(){
  GLenum er = glGetError();

  if(er != GL_NO_ERROR){
    printf("\nEr: %d %d\n\n", __LINE__, er);
  }
}

vec3 lightPos;// = {}

float near_plane;
float far_plane;

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
  }
};

// ~~~~~~~~~~~~~~~~~
const char* instancesStr[] = { [editorInstance]="Editor", [gameInstance]="Game" };

const char* wallTypeStr[] = {
  [normWallT] = "Wall",[RWallT] = "RWall", [LWallT] = "LWall",[LRWallT] = "LRWall",[windowT] = "Window",[doorT] = "Door"
};

const char* tileBlocksStr[] = { [roofBlockT] = "Roof",[stepsBlockT] = "Steps",[angledRoofT] = "Angle Roof" };

const char* lightTypesStr[] = { [pointLightT] = "Point light", [dirLightShadowT] = "Dir light(shadow)", [dirLightT] = "Dir light" };

const char* wallPlanesStr[] = {
  [wTopPlane] = "Top plane",
  [wFrontPlane] = "Front plane",
  [wBackPlane] = "Back plane",
  
  //[wLeftExPlane] = "Left ext plane",
 // [wRightExPlane] = "Right ext plane",
};

const char* windowPlanesStr[] = {
  [winTopPlane] = "Top plane",
  [winFrontCapPlane] = "Front-cap plane",
  [winFrontBotPlane] = "Front-bot plane",
  [winBackCapPlane] = "Back-cap plane",
  [winBackBotPlane] = "Back-bot plane",
  [winCenterBackPlane] = "Center-back plane" ,
  [winCenterFrontPlane] = "Center-front plane" ,

  [winInnerBotPlane] = "Inner-bot plane",
  [winInnerTopPlane] = "Inner-top plane",

  [winInnerLeftPlane] = "Inner-left plane",
  [winInnerRightPlane] = "Inner-right plane",

  [winFrontPodokonik] = "Front-padokonik",
};

const int planesInfo[wallTypeCounter] = {
  [normWallT] = wPlaneCounter,
    [LRWallT] = wPlaneCounter,
    [halfWallT] = wPlaneCounter,
    [RWallT] = wPlaneCounter,
    [LWallT] = wPlaneCounter,

  [hiddenWallT] = wPlaneCounter,
    [hiddenLRWallT] = wPlaneCounter,
  [hiddenLWallT] = wPlaneCounter,
  [hiddenRWallT] = wPlaneCounter,

  [windowT] = winPlaneCounter,
  [doorT] = doorPlaneCounter,
};
const char* doorPlanesStr[] = {
  [winTopPlane] = "Top plane",
  [doorFrontPlane] = "Front plane",
  [doorBackPlane] = "Back plane",
  [doorCenterPlane] = "Center plane" ,
};

const char* mouseBrushTypeStr[] = {
  [mouseModelBrushT] = "Model",
  [mouseWallBrushT] = "Wall",
  [mouseTextureBrushT] = "Texture",
  [mouseTileBrushT] = "Tile",
  [mouseBlockBrushT] = "Block",
};

const char* manipulationModeStr[] = { "None","Rotate_X", "Rotate_Y", "Rotate_Z", "Transform_XY", "Transform_Z", "Scale" };

ModelsTypesInfo modelsTypesInfo[] = {
  [objectModelType] = {"Obj",0},
  [playerModelT] = {"Player", 0}
};

const char* shadersFileNames[] = { "lightSource", "hud", "fog", "borderShader", "screenShader", [dirShadowShader] = "dirShadowDepth" };
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

const char* sidesToStr[] = { [top] = "Top",[bot] = "Bot",[right] = "Right",[left] = "Left" };

Camera* curCamera = &camera1;

bool fullScreen = 0;

Light lightDef[lightsTypeCounter] = {
  [pointLightT] = {.color = rgbToGl(253.0f, 244.0f, 220.0f), .dir = {0,-1, 0}, .curLightPresetIndex = 11},
  [dirLightShadowT] = {.color = rgbToGl(253.0f, 244.0f, 220.0f), .dir = {0,-1, 0}, .curLightPresetIndex = 11},
  [dirLightT] = {.color = rgbToGl(253.0f, 244.0f, 220.0f), .dir = {0,-1, 0}, .curLightPresetIndex = 11}
};

Menu dialogViewer = { .type = dialogViewerT };
Menu dialogEditor = { .type = dialogEditorT };


//const float windowW = 1280.0f;
//const float windowH = 720.0f;

float windowW = 1920.0f;
float windowH = 1080.0f;


float dofPercent = 1.0f;

int gridX = 120;
int gridY = 15;
int gridZ = 120;

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


BatchedTile* batchedGeometryIndexes;
int batchedGeometryIndexesSize;

//Picture* planeOnCreation;

int texture1DIndexByName(char* txName) {
  for (int i = 0; i < loadedTexturesCounter; i++) {
    if (strcmp(txName, loadedTexturesNames[i]) == 0) {
      return i;
    }
  }

  return -1;
};


int main(int argc, char* argv[]) {
    for (int i = 0; i < lightsTypeCounter; i++) {
  lightDef[i].rad = cosf(rad(12.5f));
  lightDef[i].cutOff = cosf(rad(17.f));
    }

  SDL_Init(SDL_INIT_VIDEO);

    
  SDL_DisplayMode DM;
  SDL_GetCurrentDisplayMode(0, &DM);

  SDL_Rect r1;

  SDL_GetDisplayUsableBounds(0, &r1);

  windowW = DM.w;
  windowH = DM.h;

  //  printf("def %d %d \n", windowH, windowW);
  //    printf(" %d %d \n",);
  //  printf("Usable %d %d \n", r1.h, r1.w);
    

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

  GLuint netTileVBO, netTileVAO;
  GLuint texturedTileVBO, texturedTileVAO;
  GLuint snowFlakesMeshVBO, snowFlakesMeshVAO;
  GLuint centerMouseSelVBO, centerMouseSelVAO;

  glewInit();

  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);

  // cursor buffers
  {
    glGenBuffers(1, &cursor.VBO);
    glGenVertexArrays(1, &cursor.VAO);
	
    /*      glBindVertexArray(cursor.VAO);
	    glBindBuffer(GL_ARRAY_BUFFER, cursor.VBO);
	
	    float cursorH = 0.06f;
	    float cursorW = 0.02f;
	
	    float cursorPoint[] = {
	    cursorW * 0.05f, cursorH,   0.0f, 0.0f,
	    cursorW, cursorH/2.0f,      0.0f, 0.0f,
	    0.0f, cursorH / 4.0f,       0.0f, 0.0f, 
	    };

	    glBufferData(GL_ARRAY_BUFFER, sizeof(cursorPoint), cursorPoint, GL_STATIC_DRAW);

	    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	    glEnableVertexAttribArray(0);

	    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	    glEnableVertexAttribArray(1);*/

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  // 2d free rect
  {
    glGenBuffers(1, &hudRect.VBO);
    glGenVertexArrays(1, &hudRect.VAO);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  // nav meshes
  {
    glGenBuffers(1, &navPointsMesh.VBO);
    glGenVertexArrays(1, &navPointsMesh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  // nav meshes
  {
    glGenBuffers(1, &navPointsConnMesh.VBO);
    glGenVertexArrays(1, &navPointsConnMesh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  assembleWallBlockVBO();
  assembleWindowBlockVBO();
  assembleDoorBlockVBO();
  assembleHideWallBlockVBO();
  assembleHalfWallBlockVBO();
  
  assembleBlocks();

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

  // init VBO and VAO for type
  {
    glGenBuffers(1, &objectsMenuTypeRectVBO);
    glGenVertexArrays(1, &objectsMenuTypeRectVAO);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  {
    // dialog editor
    {
      glGenVertexArrays(1, &dialogEditor.VAO);
      glBindVertexArray(dialogEditor.VAO);

      glGenBuffers(1, &dialogEditor.VBO);
      glBindBuffer(GL_ARRAY_BUFFER, dialogEditor.VBO);

      float* editorPoints = uiRectPercentage(f(1 / 8), f(7 / 8), f(7 / 8), f(1 / 8));

      dialogEditor.rect = (UIRect){ editorPoints[0], editorPoints[1], editorPoints[20] - editorPoints[0], editorPoints[21] - editorPoints[1] };

      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (6 * 4), editorPoints, GL_STATIC_DRAW);
      free(editorPoints);

      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }

    dialogEditor.vpairs = malloc(dialogEditorInputsCounter * sizeof(VPair));
    dialogEditor.textInputs = calloc(dialogEditorInputsCounter, sizeof(TextInput));

    dialogEditor.buttonsPairs = malloc(dialogEditorButtonsCounter * sizeof(VPair));
    dialogEditor.buttons = malloc(dialogEditorButtonsCounter * sizeof(UIRect));

    // dialog nameInput editor
    {
      glGenVertexArrays(1, &dialogEditor.vpairs[charNameInput].VAO);
      glBindVertexArray(dialogEditor.vpairs[charNameInput].VAO);

      glGenBuffers(1, &dialogEditor.vpairs[charNameInput].VBO);
      glBindBuffer(GL_ARRAY_BUFFER, dialogEditor.vpairs[charNameInput].VBO);

      float* nameInput = uiRectPoints(dialogEditor.rect.x + letterCellW * (strlen(dialogEditorCharNameTitle) + 1), dialogEditor.rect.y - letterH / 2, 33 * letterW, letterH);

      dialogEditor.textInputs[charNameInput].rect = (UIRect){ dialogEditor.rect.x + letterCellW * (strlen(dialogEditorCharNameTitle) + 1), dialogEditor.rect.y - letterH / 2, 33 * letterW, letterH };

      dialogEditor.textInputs[charNameInput].charsLimit = dialogEditorNameInputLimit;

      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (6 * 4), nameInput, GL_STATIC_DRAW);
      free(nameInput);

      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }

    // dialog replica input
    {
      glGenVertexArrays(1, &dialogEditor.vpairs[replicaInput].VAO);
      glBindVertexArray(dialogEditor.vpairs[replicaInput].VAO);

      glGenBuffers(1, &dialogEditor.vpairs[replicaInput].VBO);
      glBindBuffer(GL_ARRAY_BUFFER, dialogEditor.vpairs[replicaInput].VBO);

      float* replica = uiRectPoints(dialogEditor.rect.x + 0.01f, dialogEditor.rect.y - 0.25f, letterCellW * 37, letterCellH * 3 + 0.05f);

      dialogEditor.textInputs[replicaInput].rect = (UIRect){ dialogEditor.rect.x + 0.01f, dialogEditor.rect.y - 0.25f, letterCellW * 37, letterCellH * 3 + 0.05f };

      dialogEditor.textInputs[replicaInput].charsLimit = dialogEditorReplicaInputLimit;

      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (6 * 4), replica, GL_STATIC_DRAW);
      free(replica);

      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }


    // dialog prev phrase
    {
      glGenVertexArrays(1, &dialogEditor.buttonsPairs[prevDialogButton].VAO);
      glBindVertexArray(dialogEditor.buttonsPairs[prevDialogButton].VAO);

      glGenBuffers(1, &dialogEditor.buttonsPairs[prevDialogButton].VBO);
      glBindBuffer(GL_ARRAY_BUFFER, dialogEditor.buttonsPairs[prevDialogButton].VBO);

      float* prevBut = uiRectPoints(dialogEditor.textInputs[replicaInput].rect.x, dialogEditor.rect.y, letterCellW, letterCellH);

      dialogEditor.buttons[prevDialogButton] = (UIRect){ dialogEditor.textInputs[replicaInput].rect.x, dialogEditor.rect.y, letterCellW, letterCellH };

      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (6 * 4), prevBut, GL_STATIC_DRAW);
      free(prevBut);

      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }

    // dialog editor answer input
    {
      float baseY = dialogEditor.textInputs[replicaInput].rect.y - dialogEditor.textInputs[replicaInput].rect.h;
      float baseX = dialogEditor.textInputs[replicaInput].rect.x;

      float answerInputH = letterCellH;
      float answerInputW = letterW * 64;

      for (int i = answerInput1; i < dialogEditorInputsCounter; i++) {
	// next buttons
	{
	  int nextButIndex = (i - 2) + nextButton1;

	  glGenVertexArrays(1, &dialogEditor.buttonsPairs[nextButIndex].VAO);
	  glBindVertexArray(dialogEditor.buttonsPairs[nextButIndex].VAO);

	  glGenBuffers(1, &dialogEditor.buttonsPairs[nextButIndex].VBO);
	  glBindBuffer(GL_ARRAY_BUFFER, dialogEditor.buttonsPairs[nextButIndex].VBO);

	  float* buttonPoints = uiRectPoints(baseX + answerInputW + 0.02f + letterCellW, baseY - ((i - 1) * (answerInputH + 0.03f)), letterCellW, answerInputH);

	  dialogEditor.buttons[nextButIndex] = (UIRect){ baseX + answerInputW + 0.02f + letterCellW, baseY - ((i - 1) * (answerInputH + 0.03f)), letterCellW, answerInputH };

	  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (6 * 4), buttonPoints, GL_STATIC_DRAW);
	  free(buttonPoints);

	  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	  glEnableVertexAttribArray(0);

	  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
	  glEnableVertexAttribArray(1);

	  glBindBuffer(GL_ARRAY_BUFFER, 0);
	  glBindVertexArray(0);
	}

	// minus buttons
	{
	  int minusButIndex = (i - 2) + minusButton1;

	  glGenVertexArrays(1, &dialogEditor.buttonsPairs[minusButIndex].VAO);
	  glBindVertexArray(dialogEditor.buttonsPairs[minusButIndex].VAO);

	  glGenBuffers(1, &dialogEditor.buttonsPairs[minusButIndex].VBO);
	  glBindBuffer(GL_ARRAY_BUFFER, dialogEditor.buttonsPairs[minusButIndex].VBO);

	  float* buttonPoints = uiRectPoints(baseX + answerInputW + 0.01f, baseY - ((i - 1) * (answerInputH + 0.03f)), letterCellW, answerInputH);

	  dialogEditor.buttons[minusButIndex] = (UIRect){ baseX + answerInputW + 0.01f, baseY - ((i - 1) * (answerInputH + 0.03f)), letterCellW, answerInputH };

	  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (6 * 4), buttonPoints, GL_STATIC_DRAW);
	  free(buttonPoints);

	  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	  glEnableVertexAttribArray(0);

	  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
	  glEnableVertexAttribArray(1);

	  glBindBuffer(GL_ARRAY_BUFFER, 0);
	  glBindVertexArray(0);
	}

	int addButIndex = (i - 2) + addButton1;

	// add buttons
	if (addButIndex != addButton5 + 1)
	  {
	    glGenVertexArrays(1, &dialogEditor.buttonsPairs[addButIndex].VAO);
	    glBindVertexArray(dialogEditor.buttonsPairs[addButIndex].VAO);

	    glGenBuffers(1, &dialogEditor.buttonsPairs[addButIndex].VBO);
	    glBindBuffer(GL_ARRAY_BUFFER, dialogEditor.buttonsPairs[addButIndex].VBO);

	    float* buttonPoints = uiRectPoints(baseX + 0.02f, baseY - letterCellH - ((i - 1) * (answerInputH + 0.03f)) - 0.02f, letterCellW, answerInputH);

	    dialogEditor.buttons[addButIndex] = (UIRect){ baseX + 0.02f, baseY - letterCellH - ((i - 1) * (answerInputH + 0.03f)) - 0.02f, letterCellW, answerInputH };

	    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (6 * 4), buttonPoints, GL_STATIC_DRAW);
	    free(buttonPoints);

	    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	    glEnableVertexAttribArray(0);

	    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
	    glEnableVertexAttribArray(1);

	    glBindBuffer(GL_ARRAY_BUFFER, 0);
	    glBindVertexArray(0);
	  }

	// answers inputs
	{
	  glGenVertexArrays(1, &dialogEditor.vpairs[i].VAO);
	  glBindVertexArray(dialogEditor.vpairs[i].VAO);

	  glGenBuffers(1, &dialogEditor.vpairs[i].VBO);
	  glBindBuffer(GL_ARRAY_BUFFER, dialogEditor.vpairs[i].VBO);

	  float* answerInpt = uiRectPoints(baseX, baseY - ((i - 1) * (answerInputH + 0.03f)), answerInputW, answerInputH);

	  dialogEditor.textInputs[i].rect = (UIRect){ baseX, baseY - ((i - 1) * (answerInputH + 0.03f)), answerInputW, answerInputH };

	  dialogEditor.textInputs[i].charsLimit = dialogEditorAnswerInputLimit;

	  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (6 * 4), answerInpt, GL_STATIC_DRAW);
	  free(answerInpt);

	  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	  glEnableVertexAttribArray(0);

	  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
	  glEnableVertexAttribArray(1);

	  glBindBuffer(GL_ARRAY_BUFFER, 0);
	  glBindVertexArray(0);
	}
      }
    }

    dialogViewer.buttons = malloc(sizeof(UIRect) * dialogEditorButtonsCounter);
    // dialog viewer background

    {
      glGenVertexArrays(1, &dialogViewer.VAO);
      glBindVertexArray(dialogViewer.VAO);

      glGenBuffers(1, &dialogViewer.VBO);
      glBindBuffer(GL_ARRAY_BUFFER, dialogViewer.VBO);

      float xLeftPad = .025f;

      float* dialogViewerPoints = uiRectPoints(dialogEditor.rect.x - xLeftPad, -0.1f - 0.01f, dialogEditor.rect.w + .03f, (6 * letterCellH + 0.02f) + 0.05f + 5 * letterCellH);

      dialogViewer.rect = (UIRect){ dialogEditor.rect.x - xLeftPad, -0.1f - 0.01f , dialogEditor.rect.w + .03f, (6 * letterCellH + 0.02f) + 0.05f + 5 * letterCellH };

      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (6 * 4), dialogViewerPoints, GL_STATIC_DRAW);

      free(dialogViewerPoints);

      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }

    // dialog viewer answers background
    {
      float baseY = -(0.02f + 6 * letterCellH);

      for (int i = 0; i < answerBut6 + 1; i++) {
	//	glGenVertexArrays(1, &dialogViewer.buttonsPairs[i].VAO);
	//	glBindVertexArray(dialogViewer.buttonsPairs[i].VAO);

	//	glGenBuffers(1, &dialogViewer.buttonsPairs[i].VBO);
	//	glBindBuffer(GL_ARRAY_BUFFER, dialogViewer.buttonsPairs[i].VBO);

	float* answerButton = uiRectPoints(dialogViewer.rect.x + 0.03f, baseY - (i * (letterCellH + 0.01f)), dialogEditor.textInputs[answerInput1].rect.w, letterCellH);

	dialogViewer.buttons[i] = (UIRect){ dialogViewer.rect.x + 0.03f ,baseY - (i * (letterCellH + 0.01f)), dialogEditor.textInputs[answerInput1].rect.w, letterCellH };

	//	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (6 * 4), answerButton, GL_STATIC_DRAW);
	free(answerButton);

	/*glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	  glEnableVertexAttribArray(0);

	  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
	  glEnableVertexAttribArray(1);*/
      }

      //      glBindBuffer(GL_ARRAY_BUFFER, 0);
      //      glBindVertexArray(0);
    }}

  // console
  {
    glGenVertexArrays(1, &console.VAO);
    glBindVertexArray(console.VAO);

    glGenBuffers(1, &console.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, console.VBO);

    float consolePoints[] = {
      -1.0f, 1.0f, 1.0f, 0.0f,
      1.0f, 1.0f, 1.0f, 1.0f,
      -1.0f, consoleH, 0.0f, 0.0f,

      1.0f, 1.0f, 1.0f, 1.0f,
      -1.0f, consoleH, 0.0f, 0.0f,
      1.0f, consoleH, 0.0f, 1.0f };

    glBufferData(GL_ARRAY_BUFFER, sizeof(consolePoints), consolePoints, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
    glEnableVertexAttribArray(1);

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
    glGenBuffers(1, &cube.VBO);
    glGenVertexArrays(1, &cube.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, cube.VBO);
    glBindVertexArray(cube.VAO);

    float w = .1f;
    float h = .1f;
    float d = .1f;

    float hW = w / 2;
    float hH = h / 2;
    float hD = d / 2;

    float verts[] = {
      // bot
      -hW, -hH, -hD,
      hW, -hH, -hD,
      hW, -hH, hD,

      -hW, -hH, -hD,
      -hW, -hH, hD,
      hW, -hH, hD,

      // top
      -hW, hH, -hD,
      hW, hH, -hD,
      hW, hH, hD,

      -hW, hH, -hD,
      -hW, hH, hD,
      hW, hH, hD,

      // left
      -hW, 0.0f, 0.0f,
      -hW, -hH, hD,
      -hW, hH, -hD,

      -hW, hH, -hD,
      -hW, -hH, hD,
      -hW, hH, hD,

      // right
      hW, -hH, -hD,
      hW, -hH, hD,
      hW, hH, -hD,

      hW, hH, -hD,
      hW, -hH, hD,
      hW, hH, hD,

      // front
      -hW, -hH, -hD,
      hW, 0.0f, 0.0f,
      hW, hH, -hD,

      -hW, -hH, -hD,
      hW, hH, -hD,
      -hW, hH, -hD,

      // back
      -hW, -hH, hD,
      hW, -hH, hD,
      hW, hH, hD,

      -hW, -hH, hD,
      hW, hH, hD,
      -hW, hH, hD,
    };

    cube.vertexNum = sizeof(verts) / sizeof(float) / 3;
    cube.attrSize = 3;

    cube.vBuf = malloc(sizeof(verts));
    memcpy(cube.vBuf, verts, sizeof(verts));

    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }


  // snowflakes
  {
    glGenVertexArrays(1, &snowFlakesMeshVAO);
    glBindVertexArray(snowFlakesMeshVAO);

    glGenBuffers(1, &snowFlakesMeshVBO);
    glBindBuffer(GL_ARRAY_BUFFER, snowFlakesMeshVBO);

    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
  }

  // snowflakes
  {
    glGenVertexArrays(1, &tileOver.VAO);
    glBindVertexArray(tileOver.VAO);

    glGenBuffers(1, &tileOver.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, tileOver.VBO);

    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
  }

  for (int i = 0; i < shadersCounter; i++) {
    int nameLen = strlen(shadersFileNames[i]) + 1;

    char* vertFileName = malloc(sizeof(char) * (nameLen + strlen(".vert")));
    char* fragFileName = malloc(sizeof(char) * (nameLen + strlen(".frag")));
    char* geomFileName = malloc(sizeof(char) * (nameLen + strlen(".geom")));

    strcpy(vertFileName, shadersFileNames[i]);
    strcat(vertFileName, ".vert");

    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertFileName);

    strcpy(fragFileName, shadersFileNames[i]);
    strcat(fragFileName, ".frag");

    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragFileName);

    strcpy(geomFileName, shadersFileNames[i]); 
    strcat(geomFileName, ".geom");

    GLuint geometryShader = loadShader(GL_GEOMETRY_SHADER, geomFileName);
	
    free(vertFileName);
    free(fragFileName);
    free(geomFileName);

    shadersId[i] = glCreateProgram();
    glAttachShader(shadersId[i], fragmentShader);
    glAttachShader(shadersId[i], vertexShader);

    if(geometryShader != 0){
      glAttachShader(shadersId[i], geometryShader);   
    }

    // Link the shader shadersId[i]ram
    glLinkProgram(shadersId[i]); 

    // Check for linking errors
    GLint linkStatus;
    glGetProgramiv(shadersId[i], GL_LINK_STATUS, &linkStatus);

    if (linkStatus != GL_TRUE) {
      GLint logLength;
      glGetProgramiv(shadersId[i], GL_INFO_LOG_LENGTH, &logLength);
      char* log = (char*)malloc(logLength);
      glGetProgramInfoLog(shadersId[i], logLength, NULL, log);
      fprintf(stderr, "Failed to link \"%s\": %s\n", shadersFileNames[i], log);
      free(log);
      return 1;
    }
  }

  //    glUseProgram(shadersId[mainShader]);
  //    glUniform2f(viewportLoc, windowW, windowH);
  //    uniformVec2(mainShader,"", (vec2){windowW, windowH})

  vec3 fogColor = { 0.5f, 0.5f, 0.5f };
  glClearColor(argVec3(fogColor), 1.0f);

  // init opengl
  {
    //  glEnable(GL_MULTISAMPLE);  

    glEnable(GL_TEXTURE_2D);

    //          glEnable(GL_CULL_FACE);
       //glCullFace(GL_BACK);
	  //    glCullFace(GL_FRONT);

	
    //glEnable(GL_CULL_FACE);

    //	   glEnable(GL_TEXTURE_2D);
    //    glShadeModel(GL_SMOOTH);
    //glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    //    glDepthFunc(GL_LESS);
    
    //    glEnable(GL_STENCIL_TEST);
    //    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    //    glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
    //    glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    
    //GL_GEQUAL
    //    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
      //glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
    //	glEnable(GL_CULL_FACE);
	    

    //        glDepthFunc(GL_LEQUAL);

    //	glDepthFunc(GL_LESS);
    //  glDepthMask(GL_FALSE);

    //    glfwWindowHint(GLFW_SAMPLES, 4);
    glEnable(GL_MULTISAMPLE);

    //	glEnable(GL_CULL_FACE);
    //	glCullFace(GL_FRONT);
    //	glFrontFace(GL_CCW);

    //        glEnable(GL_STENCIL_TEST);
    //        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    //        glStencilMask(0x00);

    //    glStencilMask(0xFF);
    //    glStencilMask(0x00);
    //    glStencilFunc(GL_EQUAL, 1, 0xFF);


    //        glEnable(GL_BLEND);
    //    glAlphaFunc(GL_GREATER, 0.01);
    //glEnable(GL_ALPHA_TEST);
	// glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
       glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //    glEnable(GL_MULTISAMPLE);  
  }

  // fbo rbo multisample things
  {
    // shadow depth fbo
      
      
    // main fbo
    {
      glGenFramebuffers(1, &fbo);
      glBindFramebuffer(GL_FRAMEBUFFER, fbo);

      glGenTextures(1, &textureColorBufferMultiSampled);
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
      glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, windowW, windowH, GL_TRUE);
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);

      unsigned int rbo;
      glGenRenderbuffers(1, &rbo);
      glBindRenderbuffer(GL_RENDERBUFFER, rbo);
      glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, windowW, windowH);
      glBindRenderbuffer(GL_RENDERBUFFER, 0);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

      /*unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowW, windowH);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);*/
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      printf("main fbo creation failed! With %d \n", glCheckFramebufferStatus(GL_FRAMEBUFFER));
      exit(0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // intermediateFBO
    {
      glGenFramebuffers(1, &intermediateFBO);
      glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);

      glGenTextures(1, &screenTexture);
      glBindTexture(GL_TEXTURE_2D, screenTexture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowW, windowH, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      printf("intermediateFBO creation failed! With %d \n", glCheckFramebufferStatus(GL_FRAMEBUFFER));
      exit(0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  // load purple black texture to detect mistakes
  {
    GLubyte color[] = {
      160, 32, 240, 255, // purple
      50, 205, 50, 255, // lime
      160, 32, 240, 255,
      50, 205, 50, 255,
      160, 32, 240, 255,
      50, 205, 50, 255,

      160, 32, 240, 255, // purple
      50, 205, 50, 255, // lime
      160, 32, 240, 255,
      50, 205, 50, 255,
      160, 32, 240, 255,
      50, 205, 50, 255,
    };

    createTexture(&errorTx, 12, 1, color);
  }

  // make texture to hide
  {
    GLubyte color[] = { 0, 0, 0, 0 };
    createTexture(&emptyTx, 1, 1, color);
  }

  // load 1x1 texture to rende ricolors
  {
    GLubyte color[4] = { 255, 255, 255, 255 };
    createTexture(&solidColorTx, 1, 1, color);
  }

  // load textures
  {
    FILE* texSpecs = fopen(texturesFolder"texSpec.txt", "r");

    if (!texSpecs) {
      printf("Missing \"%s\"", texturesFolder"texSpec.txt");
      exit(-1);
    }

    char ch;
    while ((ch = fgetc(texSpecs)) != EOF)  if (ch == '\n') loadedTexturesCounter++;

    loadedTexturesCounter++;

    rewind(texSpecs);

    char category[300];
    char fileName[300];

    for (int i = 0; i < loadedTexturesCounter; i++) {
      fscanf(texSpecs, "%s %s\n", &fileName, &category);

      bool exists = false;
      for (int i2 = 0; i2 < loadedTexturesCategoryCounter; i2++) {
	if (strcmp(loadedTexturesCategories[i2].name, category) == 0) {
	  loadedTexturesCategories[i2].txWithThisCategorySize++;
	  exists = true;
	  break;
	}
      }

      if (!exists) {
	if (!loadedTexturesCategories) {
	  loadedTexturesCategories = malloc(sizeof(Category));
	}
	else {
	  loadedTexturesCategories = realloc(loadedTexturesCategories, sizeof(Category) * (loadedTexturesCategoryCounter + 1));
	};

	loadedTexturesCategories[loadedTexturesCategoryCounter].index = loadedTexturesCategoryCounter;

	int newCategoryLen = strlen(category);

	longestTextureCategoryLen = max(newCategoryLen, longestTextureCategoryLen);

	loadedTexturesCategories[loadedTexturesCategoryCounter].name = malloc(sizeof(char) * (newCategoryLen + 1));
	strcpy(loadedTexturesCategories[loadedTexturesCategoryCounter].name, category);

	loadedTexturesCategories[loadedTexturesCategoryCounter].txWithThisCategorySize = 1;

	loadedTexturesCategoryCounter++;
      }
    }

    rewind(texSpecs);

    loadedTextures1D = malloc(sizeof(Texture) * loadedTexturesCounter);
    loadedTextures2D = malloc(sizeof(Texture*) * loadedTexturesCategoryCounter);
    loadedTexturesNames = malloc(sizeof(char*) * loadedTexturesCounter);

    int* indexesTrackerFor2DTex = calloc(loadedTexturesCategoryCounter, sizeof(int));

    for (int i2 = 0; i2 < loadedTexturesCategoryCounter; i2++) {
      loadedTextures2D[i2] = malloc(sizeof(Texture) * loadedTexturesCategories[i2].txWithThisCategorySize);
    }

    for (int i = 0; i < loadedTexturesCounter; i++) {
      fscanf(texSpecs, "%s %s\n", &fileName, &category);

      loadedTexturesNames[i] = malloc(sizeof(char) * (strlen(fileName) + 1));
      strcpy(loadedTexturesNames[i], fileName);
      strcut(loadedTexturesNames[i], strlen(loadedTexturesNames[i]) - 4, strlen(loadedTexturesNames[i]));

      longestTextureNameLen = max(longestTextureNameLen, strlen(loadedTexturesNames[i]));

      char fullPath[300];

      strcpy(fullPath, texturesFolder);
      strcat(fullPath, fileName);

      SDL_Surface* texture = IMG_Load_And_Flip_Vertical(fullPath);

      if (!texture) {
	printf("Loading of texture \"%s\" failed", fullPath);
	exit(-1);
      }

      GLuint txId;


      createTexture(&txId, texture->w, texture->h, texture->pixels);  
      printf("%s -  %d \n", fullPath,txId);
      SDL_FreeSurface(texture);
      /*
	glGenTextures(1, &txId);

	glBindTexture(GL_TEXTURE_2D, txId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->w,
	texture->h, 0, GL_RGBA,
	GL_UNSIGNED_BYTE, texture->pixels);


	glBindTexture(GL_TEXTURE_2D, 0);
      */
      int categoryIndex = -1;

      for (int i2 = 0; i2 < loadedTexturesCategoryCounter; i2++) {
	if (strcmp(loadedTexturesCategories[i2].name, category) == 0) {
	  categoryIndex = i2;
	  break;
	}
      }

      int index2D = indexesTrackerFor2DTex[categoryIndex];

      loadedTextures2D[categoryIndex][index2D].tx = txId;
      indexesTrackerFor2DTex[categoryIndex]++;

      loadedTextures2D[categoryIndex][index2D].index2D = index2D;
      loadedTextures2D[categoryIndex][index2D].index1D = i;
      loadedTextures2D[categoryIndex][index2D].categoryIndex = categoryIndex;

      loadedTextures1D[i].tx = txId;
      loadedTextures1D[i].index2D = index2D;
      loadedTextures1D[i].index1D = i;
      loadedTextures1D[i].categoryIndex = categoryIndex;
    }

    free(indexesTrackerFor2DTex);
  }

    
  // load 3d models
  {
    FILE* objsSpecs = fopen("./assets/objs/ObjsSpecs.txt", "r");

    if (!objsSpecs) {
      printf("ObjsSpecs.txt was not found! \n");
    }
    else {
      char textureName[50];
      char objName[50];
      char typeStr[10];

      int charsCounter = 0;
      int objsCounter = 0;

      while (fscanf(objsSpecs, "%s %s %s\n", objName, textureName, typeStr) != EOF) {
	if (strcmp(typeStr, "Player") == 0) {
	  charsCounter++;
	}
	else if (strcmp(typeStr, "Obj") == 0) {
	  objsCounter++;
	}
	else {
	  printf("Model %s has wrong type %s \n", objName, typeStr);
	  exit(0);
	}
      };

      loadedModels1D = malloc(sizeof(ModelInfo) * (charsCounter + objsCounter));

      loadedModels2D = malloc(sizeof(ModelInfo*) * modelTypeCounter);
      loadedModels2D[objectModelType] = malloc(sizeof(ModelInfo) * objsCounter);
      loadedModels2D[playerModelT] = malloc(sizeof(ModelInfo) * objsCounter);

      rewind(objsSpecs);

      bool playerModelIsLoaded = false;

      while (fscanf(objsSpecs, "%s %s %s\n", objName, textureName, typeStr) != EOF) {
	char* fullObjPath = malloc(strlen(objName) + strlen(objsFolder) + 1);

	strcpy(fullObjPath, objsFolder);
	strcat(fullObjPath, objName);

	char* fullTxPath = malloc(strlen(textureName) + strlen(objsFolder) + 1);

	strcpy(fullTxPath, objsFolder);
	strcat(fullTxPath, textureName);

	ModelType type = -1;

	for (int i2 = 0; i2 < modelTypeCounter; i2++) {
	  if (strcmp(typeStr, modelsTypesInfo[i2].str) == 0) {
	    type = i2;
	    break;
	  }
	}

	ModelInfo* loadedModel = loadOBJ(fullObjPath, fullTxPath);

	if (strcmp(typeStr, modelsTypesInfo[playerModelT].str) == 0) {
	  if(playerModelIsLoaded){
	    printf("You're trying to load more than 1 model for player");
	    exit(0);
	  }
	  
	  loadedModel->type = playerModelT;
	  playerModelIsLoaded= true;
	}else{
	  loadedModel->type = objectModelType;
	}

	loadedModel->name = malloc(sizeof(char) * (strlen(objName) + 1));
	strcpy(loadedModel->name, objName);
	strcut(loadedModel->name, strlen(objName) - 4, strlen(objName));
	loadedModel->index1D = loadedModelsSize;
	loadedModel->index2D = modelsTypesInfo[type].counter;

	loadedModels1D[loadedModelsSize] = *loadedModel;
	loadedModels2D[type][modelsTypesInfo[type].counter] = *loadedModel;

	free(fullObjPath);
	free(fullTxPath);

	printf("Loaded %s\n", objName);

	modelsTypesInfo[type].counter++;
	loadedModelsSize++;
      }

      fclose(objsSpecs);
    }
  }

  geomentyByTxCounter = calloc(loadedTexturesCounter, sizeof(size_t));
  
  modelsBatch = calloc(loadedModelsTxSize, sizeof(Geometry));
  
  for (int i = 0; i < loadedModelsTxSize; i++) {
    glGenVertexArrays(1, &modelsBatch[i].pairs.VAO);
    glBindVertexArray(modelsBatch[i].pairs.VAO);

    glGenBuffers(1, &modelsBatch[i].pairs.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, modelsBatch[i].pairs.VBO);

    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
  }

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
    
    /* glBindTexture(GL_TEXTURE_2D_ARRAY, depthMaps);
	
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 6 * 6, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMaps, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

	
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      printf("main fbo creation failed! With %d \n", glCheckFramebufferStatus(GL_FRAMEBUFFER));
      exit(0);
    }
	
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);
    //	glActiveTexture(GL_TEXTURE0);
  }*/

  geometry = calloc(loadedTexturesCounter, sizeof(Geometry));

  for (int i = 0; i < loadedTexturesCounter; i++) {
    glGenVertexArrays(1, &geometry[i].pairs.VAO);
    glBindVertexArray(geometry[i].pairs.VAO);

    glGenBuffers(1, &geometry[i].pairs.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, geometry[i].pairs.VBO);

    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
  }

  finalGeom = malloc(sizeof(GeomFin) * loadedTexturesCounter);
    
  for (int i = 0; i < loadedTexturesCounter; i++) {
    glGenVertexArrays(1, &finalGeom[i].VAO);
    glBindVertexArray(finalGeom[i].VAO);

    glGenBuffers(1, &finalGeom[i].VBO);
    glBindBuffer(GL_ARRAY_BUFFER, finalGeom[i].VBO);

    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
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
  if (!loadSave("map")) {
    initGrid(gridX, gridY, gridZ);

    printf("Map not found!\n");
  }

  //    lightPos = (vec3){gridX /2.0f,2.0f,gridZ/2.0f};

  renderCapYLayer = gridY;
  //  batchGeometry();
  batchAllGeometry();
  batchModels();

  // set up camera
  GLint cameraPos = glGetUniformLocation(shadersId[mainShader], "cameraPos");
  {
        camera1.pos = (vec3)xyz_indexesToCoords(gridX / 2, 10, gridZ / 2);
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

  initSnowParticles();

  const float entityH = 0.17f;
  const float entityW = 0.1f / 2.0f;
  const float entityD = entityH / 6;

  vec3 initPos = { 0.3f + 0.1f / 2, 0.0f, 0.3f + 0.1f / 2 }; //+ 0.1f/2 - (entityD * 0.75f)/2 };

  Entity player = { initPos,initPos, (vec3) { initPos.x + entityW, initPos.y + entityH, initPos.z + entityD }, 180.0f, 0, entityW, entityH, entityD };

  bool quit = false;

  clock_t lastFrame = clock();
  float deltaTime;

  float cameraSpeed = speed;
  SDL_Event event;
    
  ((void (*)(void))instances[curInstance][preLoopFunc])();    

  near_plane = 0.01f;
  far_plane  = 120.0f;
  
  glUseProgram(shadersId[dirShadowShader]);
  uniformFloat(dirShadowShader, "far_plane", far_plane);
  
  glUseProgram(shadersId[mainShader]);
  uniformInt(mainShader, "colorMap", 0); 
  uniformInt(mainShader, "shadowMap", 1);
  uniformFloat(mainShader, "far_plane", far_plane);

  while (!quit) {
    glErrorCheck();
      
    uint32_t starttime = GetTickCount();

    clock_t currentFrame = clock();
    deltaTime = (double)(currentFrame - lastFrame) / CLOCKS_PER_SEC;
    lastFrame = currentFrame;

    // cameraSpeed = 10.0f * deltaTime;


    currentKeyStates = SDL_GetKeyboardState(NULL);

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
	quit = true;
      }

      if (event.type == SDL_KEYDOWN && !console.open) {
	if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
	  exit(1);
	}

	if(event.key.keysym.scancode == SDL_SCANCODE_F4){
	  showHiddenWalls = !showHiddenWalls;
	  
	  if(showHiddenWalls){
	    batchAllGeometry();
	  }else{
	    batchAllGeometryNoHidden();
	  }
	}

	if (event.key.keysym.scancode == SDL_SCANCODE_F3) {
	  navPointsDraw = !navPointsDraw;
	  
	  if(navPointsDraw){
	    assembleNavigation();
	  }
	}

	if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
	  if (curInstance >= instancesCounter-1) {
	    curInstance = 0;
	  }
	  else {
	    curInstance++;
	  }
                     
	  ((void (*)(void))instances[curInstance][onSetFunc])();
	}
      }

      if (event.type == SDL_KEYDOWN && console.open) {
	consoleHasResponse = false;

	if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
	  char copy_consoleBuffer[CONSOLE_BUF_CAP];
	  strcpy(copy_consoleBuffer, consoleBuffer);

	  char* str_tok = strtok(copy_consoleBuffer, " ");

	  if (str_tok) {
	    if (strcmp(str_tok, "help") == 0) {
	      strcpy(consoleResponse, "help - all available commands\nsave <file_name> - save current map in new file\nload <file_name> - load map from file\ncreate <x> <y> <z> <file_name> - to create new world with size x, y, z");
	    }
	    else if (strcmp(str_tok, "load") == 0) {
	      str_tok = strtok(NULL, " ");

	      if (str_tok) {
		if (loadSave(str_tok)) {
		  sprintf(consoleResponse, "Save \"%s\" was successfully loaded", str_tok);
		}
		else {
		  sprintf(consoleResponse, "Save \"%s\" doesnt exist", str_tok);
		}
	      }
	      else {
		strcpy(consoleResponse, "Provide name of save to load \"load <file_name>\"");
	      }
	    }
	    else if (strcmp(str_tok, "save") == 0) {
	      // get save arg
	      str_tok = strtok(NULL, " ");

	      if (str_tok) {
		if (saveMap(str_tok)) {
		  sprintf(consoleResponse, "Save \"%s\" was successfully saved", str_tok);
		}
		else {
		  // sprintf(consoleResponse, "Save \"%s\" was successfully saved", str_tok);
		}
	      }
	      else {
		strcpy(consoleResponse, "Provide name for save \"save <file_name>\"");
	      }

	    }
	    else if (strcmp(str_tok, "create") == 0) {
	      // get save arg
	      str_tok = strtok(NULL, " ");

	      int x, y, z;

	      bool generalMistake = false;

	      if (str_tok) {
		x = atoi(str_tok);

		if (x <= 0) {
		  sprintf(consoleResponse, "Incorrect x(%d) value, x must be > 0", x);
		}
		else {
		  str_tok = strtok(NULL, " ");

		  if (str_tok) {
		    y = atoi(str_tok);

		    if (y <= 9) {
		      sprintf(consoleResponse, "Incorrect y(%d) value, y must be >= 10", x);
		    }
		    else {
		      str_tok = strtok(NULL, " ");

		      if (str_tok) {
			z = atoi(str_tok);

			if (z <= 0) {
			  sprintf(consoleResponse, "Incorrect z(%d) value, z must be > 0", x);
			}
			else {
			  str_tok = strtok(NULL, " ");

			  if (str_tok) {
			    strcpy(curSaveName, str_tok);

			    createMap(x, y, z);
			    sprintf(consoleResponse, "Map was craeted with size x: %d y: %d z:%d", x, y, z);
			  }
			  else {
			    generalMistake = true;
			  }
			}
		      }
		      else {
			generalMistake = true;
		      }
		    }
		  }
		  else {
		    generalMistake = true;
		  }
		}
	      }
	      else {
		generalMistake = true;
	      }

	      if (generalMistake) {
		strcpy(consoleResponse, "From to create new map \"create <x> <y> <z> <file_name>\"");
	      }
	    }
	    else {
	      sprintf(consoleResponse, "Command \"%s\"  doesnt exist\nWrite \"help\" to get all available commands", str_tok);
	    }

	    consoleHasResponse = true;
	  }
	}
	else if (event.key.keysym.scancode == SDL_SCANCODE_F1) {
	  console.open = false;
	}
	else if (consoleBufferCursor > 0 && event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE) {
	  consoleBufferCursor--;
	  consoleBuffer[consoleBufferCursor] = 0;
	}
	else if (consoleBufferCursor < CONSOLE_BUF_CAP - 1) {
	  if (event.key.keysym.scancode >= 4 && event.key.keysym.scancode <= 39) {
	    consoleBuffer[consoleBufferCursor] = sdlScancodesToACII[event.key.keysym.scancode];
	    consoleBufferCursor++;
	  }
	  else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
	    bool prevCharIsntSpace = consoleBufferCursor > 0 && consoleBuffer[consoleBufferCursor - 1] != ' ';

	    if (prevCharIsntSpace) {
	      consoleBuffer[consoleBufferCursor] = ' ';
	      consoleBufferCursor++;

	    }
	  }
	}
      }
      
      ((void (*)(SDL_Event))instances[curInstance][eventFunc])(event);
    }


    mouse.tileSide = -1;
    //    checkMouseVSEntities();
    ((void (*)(int))instances[curInstance][mouseVSFunc])(mainShader);
    
    ((void (*)(float))instances[curInstance][preFrameFunc])(deltaTime);

    //   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    /*    glStencilMask(0xff);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glStencilMask(0x00);
	
    // render to depth cubemap
    glUseProgram(shadersId[dirShadowShader]);
    glEnable(GL_DEPTH_TEST);
    
    //    if(lightStorage[shadowLightT])
    glViewport(0,0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);*/

    /*    int index = 0;
    for(int i=0;i<lightStorageSizeByType[shadowLightT];i++){
      //      if(lightStorage[shadowLightT][i].off){
	//	continue;
	//      }      
      
	glStencilMask(0xff);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilMask(0x00);

	Matrix shadowProj = perspective(rad(90.0f), 1.0f, near_plane, far_plane); 

	vec3 negPos = { -lightStorage[shadowLightT][i].mat.m[12], -lightStorage[shadowLightT][i].mat.m[13], -lightStorage[shadowLightT][i].mat.m[14] };

	static const vec3 shadowRotation[6] = { {180.0f, 90.0f, 0.0f}, {180.0f, -90.0f, 0.0f}, {90.0f, 0.0f, 0.0f},
						{-90.0f, 0.0f, 0.0f}, {180.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, 180.0f}};

	
	
	for (int i2 = 6 * i; i2 < 6 * (i+1); ++i2){	      
	  Matrix viewMat = IDENTITY_MATRIX;

	  translate(&viewMat, argVec3(negPos));

	  rotateX(&viewMat, rad(shadowRotation[i2 - (6 * i)].x));
	  rotateY(&viewMat, rad(shadowRotation[i2 - (6 * i)].y));
	  rotateZ(&viewMat, rad(shadowRotation[i2 - (6 * i)].z));

	  Matrix shadowTransforms = multiplymat4(viewMat, shadowProj);

	  char buf[128];

	  sprintf(buf, "shadowMatrices[%d]", i2);

	  uniformMat4(dirShadowShader, buf, shadowTransforms.m);
	}
      }

    if(lightStorageSizeByType[shadowLightT] > 0){
	glActiveTexture(GL_TEXTURE0);
	renderScene(dirShadowShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);   
    }*/

    
    //  if (lightStorage)
      {
	glViewport(0, 0, windowW, windowH);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	//	glStencilMask(0xff);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//	glStencilMask(0x00);
	
	glEnable(GL_DEPTH_TEST);
	
	((void (*)(int))instances[curInstance][matsSetup])(mainShader);

	glUseProgram(shadersId[mainShader]); 
        glUniform3f(cameraPos, argVec3(curCamera->pos));
	
        uniformFloat(mainShader, "far_plane", far_plane);

	//	vec3 modelXLight = {lightStorage[0].mat.m[12], lightStorage[0].mat.m[13], lightStorage[0].mat.m[14]};
        //uniformVec3(mainShader, "lightPoss", modelXLight);

	// highlight selected model with stencil
        if (true)
        {
            if (mouse.selectedType == mouseModelT) {
                glEnable(GL_STENCIL_TEST);

                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                glStencilMask(0xFF);

                glDepthMask(GL_FALSE);
                glClear(GL_STENCIL_BUFFER_BIT);

                Model* model = (Model*)mouse.selectedThing;
                int name = model->name;

                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

                // 1 draw
                {
                    glBindTexture(GL_TEXTURE_2D, loadedModels1D[name].tx);

                    glBindVertexArray(loadedModels1D[name].VAO);
                    glBindBuffer(GL_ARRAY_BUFFER, loadedModels1D[name].VBO);

                    uniformMat4(mainShader, "model", model->mat.m);

                    glDrawArrays(GL_TRIANGLES, 0, loadedModels1D[name].size);
                }

                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

                glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
                glStencilMask(0x00);
                glDepthMask(GL_TRUE);
                glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

                // higlight
                {
                    glUseProgram(shadersId[borderShader]);

                    uniformMat4(borderShader, "model", model->mat.m);
                    uniformVec3(borderShader, "borderColor", (vec3) { redColor });
		    uniformFloat(borderShader, "thick", 0.01f);

                    glDrawArrays(GL_TRIANGLES, 0, loadedModels1D[name].size);

                    glUseProgram(shadersId[mainShader]);
                }

                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);

                glBindTexture(GL_TEXTURE_2D, 0);
		 glDisable(GL_STENCIL_TEST);
            }
            else if (mouse.selectedType == mouseWallT) {
                WallMouseData* wallData = (WallMouseData*)mouse.selectedThing;
                int type = wallData->wall->type;
                int plane = wallData->plane;

                if ((type == hiddenDoorT || type == doorT) && plane == doorCenterPlane) {
                    glEnable(GL_STENCIL_TEST);

                    glStencilFunc(GL_ALWAYS, 1, 0xFF);
		    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                    //glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
                    glStencilMask(0xFF);

		    glDepthMask(GL_FALSE);
                    glClear(GL_STENCIL_BUFFER_BIT);

                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

                    {
		      glBindTexture(GL_TEXTURE_2D, loadedTextures1D[wallData->wall->planes[plane].txIndex].tx);

		      glBindVertexArray(doorDoorPlane.VAO);
		      glBindBuffer(GL_ARRAY_BUFFER, doorDoorPlane.VBO);
			
		      uniformMat4(mainShader, "model", wallData->wall->mat.m);

		      glDrawArrays(GL_TRIANGLES, 0, doorDoorPlane.VBOsize);
                    }

                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		    //		    glStencilFunc(GL_LEQUAL, 1, 0xFF);
		    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

                    glStencilMask(0x00);

		    glDepthMask(GL_TRUE);
		    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

                    // higlight
                    {
		      glUseProgram(shadersId[borderShader]);

		      uniformMat4(borderShader, "model", wallData->wall->mat.m);
		      uniformVec3(borderShader, "borderColor", (vec3) { redColor });
		      uniformFloat(borderShader, "thick", 0.04f);

		      glDrawArrays(GL_TRIANGLES, 0, doorDoorPlane.VBOsize);

		      glUseProgram(shadersId[mainShader]);
                    }

                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    glBindVertexArray(0);

                    glBindTexture(GL_TEXTURE_2D, 0);

                    glDisable(GL_STENCIL_TEST);
                }
            }
        }

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, depthMaps);

	glActiveTexture(GL_TEXTURE0);
	renderScene(mainShader);

	((void (*)(void))instances[curInstance][render3DFunc])();

	// nav meshes drawing
	if(navPointsDraw){
	  glUseProgram(shadersId[lightSourceShader]);
	    
	  glBindBuffer(GL_ARRAY_BUFFER, navPointsMesh.VBO);
	  glBindVertexArray(navPointsMesh.VAO);

	  uniformVec3(lightSourceShader, "color", (vec3) { blueColor });

	  Matrix out = IDENTITY_MATRIX;
	  uniformMat4(lightSourceShader, "model", out.m);

	  glDrawArrays(GL_TRIANGLES, 0, navPointsMesh.vertexNum);

	  {
	    glBindBuffer(GL_ARRAY_BUFFER, 0);
	    glBindVertexArray(0);

	    Matrix out = IDENTITY_MATRIX;
	    uniformMat4(lightSourceShader, "model", out.m);
	  
	    glBindBuffer(GL_ARRAY_BUFFER, navPointsConnMesh.VBO);
	    glBindVertexArray(navPointsConnMesh.VAO);

	    glDrawArrays(GL_LINES, 0, navPointsConnMesh.vertexNum);
	  }

	  glBindBuffer(GL_ARRAY_BUFFER, 0);
	  glBindVertexArray(0);

	  glUseProgram(shadersId[mainShader]);
	}


	// higlight
	if(false && mouse.selectedType == mouseModelT){
	  {
	    Model* model = (Model*)mouse.selectedThing;
	    int name = model->name;

	    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	    //	    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	    //	    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	  
	    glUseProgram(shadersId[borderShader]);
      
	    //	    glDisable(GL_DEPTH_TEST);
      
	    uniformMat4(borderShader, "model", model->mat.m);
	    uniformVec3(borderShader, "borderColor", (vec3){ redColor });
	    glDrawArrays(GL_TRIANGLES, 0, loadedModels1D[name].size);

	    //	    glEnable(GL_DEPTH_TEST);

	    glUseProgram(shadersId[mainShader]);

	    glStencilMask(0x00);
	  }
	    
	  glBindBuffer(GL_ARRAY_BUFFER, 0);
	  glBindVertexArray(0);

	  glBindTexture(GL_TEXTURE_2D, 0);
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);
	 
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
	glBlitFramebuffer(0, 0, windowW, windowH, 0, 0, windowW, windowH, GL_COLOR_BUFFER_BIT, GL_NEAREST); 

	// render to fbo 
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
	//glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(shadersId[screenShader]);
	  
	uniformFloat(screenShader, "dof", -(dofPercent - 1.0f));

	float seed = (float)(rand() % 1000 + 1) / 1000.0f;
	uniformFloat(screenShader, "time", seed);


	glBindVertexArray(quad.VAO);
	glBindTexture(GL_TEXTURE_2D, screenTexture);
	glDrawArrays(GL_TRIANGLES, 0, 6);
      }



    // 2d ui drawing
    glDisable(GL_DEPTH_TEST);
    glUseProgram(shadersId[hudShader]);

   


    instances[curInstance][render2DFunc]();

    uint32_t endtime = GetTickCount();
    uint32_t deltatime = endtime - starttime;

    if (!(deltatime > (1000 / FPS))) {
      Sleep((1000 / FPS) - deltatime);
    }

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

    mouse.clickL = false;
    mouse.clickR = false;
    //    mouse.mouseDown = false;

    //	glFlush();
  
    SDL_GL_SwapWindow(window);


    
    if (deltatime != 0) {
      sprintf(windowTitle, game" BazoMetr: %d%% Save: %s.doomer", 1000 / deltatime, curSaveName); 
      SDL_SetWindowTitle(window, windowTitle);
    }
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

inline bool oppositeTileTo(vec2i XZ, Side side, vec2i* opTile, Side* opSide){
  Side oppositeSide = 0;
	      
  switch(side){
  case(top):{
    XZ.z--;
    oppositeSide = bot;
    break;
  }
  case(bot):{
    XZ.z++;
    oppositeSide = top;
    break;
  }
  case(left):{
    XZ.x--;
    oppositeSide = right;
    break;
  }
  case(right):{
    XZ.x++;
    oppositeSide = left;
    break;
  }
  default: return false;
  }

  if(XZ.x >= 0 && XZ.z >= 0 && XZ.x < gridX && XZ.z < gridZ){
    if(opTile){
      *opTile = XZ;
    }

    if(opSide){
      *opSide = oppositeSide;
    }
    
    return true;
  }
  
  return false;
}

/*inline bool radarCheck(vec3 point) {
  vec3 v = { point.x - camera1.pos.x, point.y - camera1.pos.y, point.z - camera1.pos.z };

     vec3 minusZ = { -camera1.x, -camera1.Z.y, -camera1.Z.z };
      
  float pcz = dotf3(v, minusZ);

  if(pcz > drawDistance){// || pcz < zNear){
    return false;
  }

  float pcy = dotf3(v, camera1.Y);
  float aux = pcz;// *tangFOV;
  
  if(pcy > aux || pcy < -aux){
    return false;
  }

  float pcx = dotf3(v, camera1.X);
  aux = aux * (windowW/windowH);

  if (pcx > aux || pcx < -aux){
    return false;
  }

  return true;
  // false - out camera
  // true - in camera
}*/

ModelInfo* loadOBJ(char* path, char* texturePath){
  fastObjMesh* mesh = fast_obj_read(path);

  if (!mesh) {
    printf("Failed to load \"%s\" \n", path);
    exit(0);
  } 

  ModelInfo* loadedModel = malloc(sizeof(ModelInfo));

  // vertecies
  vec3* verts = malloc(mesh->position_count * sizeof(vec3));
  loadedModel->vertices = malloc(mesh->index_count * sizeof(vec3));

  loadedModel->modelSizes = (vec3){0,0,0};
  vec3 minPoint = { FLT_MAX, FLT_MAX, FLT_MAX };

  int counter = 0;
  for (int i = 0; i < mesh->position_count * 3;i+=3){
    if(i==0) continue;

    minPoint.x = min(minPoint.x, mesh->positions[i]);
    minPoint.y = min(minPoint.y, mesh->positions[i+1]);
    minPoint.z = min(minPoint.z, mesh->positions[i+2]);

    loadedModel->modelSizes.x = max(loadedModel->modelSizes.x, mesh->positions[i]);
    loadedModel->modelSizes.y = max(loadedModel->modelSizes.y, mesh->positions[i+1]);
    loadedModel->modelSizes.z = max(loadedModel->modelSizes.z, mesh->positions[i+2]);
    
    verts[counter] = (vec3){mesh->positions[i], mesh->positions[i+1], mesh->positions[i+2]};
    counter++;
  }

  loadedModel->modelSizes.x -= minPoint.x;
  loadedModel->modelSizes.y -= minPoint.y;
  loadedModel->modelSizes.z -= minPoint.z;
  

  // UVs
  uv2* uvs = malloc(mesh->texcoord_count * sizeof(uv2));
  uv2* sortedUvs = malloc(mesh->index_count * sizeof(uv2));
  
  counter = 0;
  for( int i=0; i<mesh->texcoord_count * 2;i+=2){
    if(i==0) continue;

    uvs[counter] = (uv2){mesh->texcoords[i], mesh->texcoords[i+1]};
    counter++;
  }

  // normals
  vec3* normals = malloc(mesh->normal_count * sizeof(vec3));
  vec3* sortedNormals = malloc(mesh->index_count * sizeof(vec3));

  counter = 0;
  for( int i=0; i<mesh->normal_count * 3;i+=3){
    if(i==0) continue;

    normals[counter] = (vec3){mesh->normals[i], mesh->normals[i+1], mesh->normals[i+2]};
    counter++;
  }
  
  // TODO: Make these 3 index_count loops in one
  for(int i=0; i< mesh->index_count; i++){
    sortedNormals[i] = normals[mesh->indices[i].n - 1];
    loadedModel->vertices[i] = verts[mesh->indices[i].p - 1];
    sortedUvs[i] = uvs[mesh->indices[i].t - 1];
  }

  loadedModel->size = mesh->index_count;
  
  free(uvs);
  free(verts);
  free(normals); 

  fast_obj_destroy(mesh);
  
  float* modelVerts = malloc(sizeof(float) * 8 * loadedModel->size);

  glGenVertexArrays(1, &loadedModel->VAO);
  glBindVertexArray(loadedModel->VAO);

  glGenBuffers(1, &loadedModel->VBO);
  glBindBuffer(GL_ARRAY_BUFFER, loadedModel->VBO);
  
  int index = 0;
  
  for(int i=0;i<loadedModel->size * 8;i+=8){
    modelVerts[i] = loadedModel->vertices[index].x;
    modelVerts[i + 1] = loadedModel->vertices[index].y;
    modelVerts[i + 2] = loadedModel->vertices[index].z;

    modelVerts[i + 3] = sortedUvs[index].x;
    modelVerts[i + 4] = sortedUvs[index].y;

    modelVerts[i + 5] = sortedNormals[index].x;
    modelVerts[i + 6] = sortedNormals[index].y;
    modelVerts[i + 7] = sortedNormals[index].z;

    index++;
  }

  free(sortedNormals);
  free(sortedUvs);
  
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * loadedModel->size, modelVerts, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  loadedModel->entireVert = modelVerts;
  //  free(modelVerts);

  // set mat of loadedModels1D[name] to IDENTITY_MATRIX
  /*  loadedModel->mat.m[0] = 1;
      loadedModel->mat.m[5] = 1;
      loadedModel->mat.m[10] = 1;
      loadedModel->mat.m[15] = 1;

      scale(&loadedModel->mat.m, 0.25f, 0.25f, 0.25f);

      // To get some speed up i can getrid off one O(n) loop
      // simply finding min/max inside of parsing positions above
      calculateModelAABB(loadedModels1D[name]);*/
  
  // loadedModel->name = name;

  // load texture
  {
    SDL_Surface* texture = IMG_Load_And_Flip_Vertical(texturePath);
    
    if (!texture) {
      printf("Loading of texture \"%s\" failed", texturePath);
      exit(0);
    }

    createTexture(&loadedModel->tx, texture->w,texture->h, texture->pixels);

    GLenum err = glGetError();
    
    if (err != GL_NO_ERROR) { 
      printf("OpenGL error: %d\n", err); 
    }
  
    SDL_FreeSurface(texture);

    loadedModelsTxSize++;

    if(!loadedModelsTx){
      loadedModelsTx = malloc(loadedModelsTxSize * sizeof(int));
    }else{
      loadedModelsTx = realloc(loadedModelsTx, loadedModelsTxSize * sizeof(int));
    }

    loadedModelsTx[loadedModelsTxSize-1] = loadedModel->tx;
  }
  
  return loadedModel;
}

// it also assigns lb, rt to model 
void calculateModelAABB(Model* model){  
  model->lb = (vec3){FLT_MAX,FLT_MAX,FLT_MAX};
  model->rt = (vec3){-FLT_MAX,-FLT_MAX,-FLT_MAX};
  
  for (int i = 0; i < loadedModels1D[model->name].size; i++) {
    vec4 trasformedVert4 = mulmatvec4(model->mat, (vec4) { argVec3(loadedModels1D[model->name].vertices[i]), 1.0f });
    
    model->lb.x = min(model->lb.x, trasformedVert4.x);
    model->lb.y = min(model->lb.y, trasformedVert4.y);
    model->lb.z = min(model->lb.z, trasformedVert4.z);
    
    model->rt.x = max(model->rt.x, trasformedVert4.x);
    model->rt.y = max(model->rt.y, trasformedVert4.y);
    model->rt.z = max(model->rt.z, trasformedVert4.z);
  }

  model->centroid = (vec3){ (model->rt.x - model->lb.x) / 2.0f,  (model->rt.y - model->lb.y) / 2.0f,  (model->rt.z - model->lb.z) / 2.0f };
}

void calculateAABB(Matrix mat, float* vertexes, int vertexesSize, int attrSize, vec3* lb, vec3* rt){
  *lb = (vec3){FLT_MAX,FLT_MAX,FLT_MAX};
  *rt = (vec3){-FLT_MAX,-FLT_MAX,-FLT_MAX};

  // assumes that first 3 it vec3, last 2 its UV
  for (int i = 0; i < vertexesSize * attrSize; i+=attrSize) {
    vec4 trasformedVert4 = mulmatvec4(mat,(vec4){vertexes[i+0],vertexes[i+1],vertexes[i+2], 1.0f });
    
    lb->x = min(lb->x, trasformedVert4.x);
    lb->y = min(lb->y, trasformedVert4.y);
    lb->z = min(lb->z, trasformedVert4.z);
    
    rt->x = max(rt->x, trasformedVert4.x);
    rt->y = max(rt->y, trasformedVert4.y);
    rt->z = max(rt->z, trasformedVert4.z);
  }

  
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

bool loadSave(char* saveName){
  resetMouse();
  
  char* save = calloc((strlen(saveName) + strlen(".doomer")), sizeof(char));

  strcat(save, saveName);
  strcat(save, ".doomer");

  FILE* map = fopen(save, "r"); 

  if (!map) {
    free(save);
    return false;
  }

  // free cur loaded map
  if(grid){
    for (int y = 0; y < gridY; y++) {
      for (int z = 0; z < gridZ; z++) {
	for (int x = 0; x < gridX; x++) {
	  free(grid[y][z][x]);
	}

	free(grid[y][z]);
      }
      
      free(grid[y]);
    }
    
    free(grid);
    grid = NULL;

    if (curModels) {
      for(int i=0;i<curModelsSize;i++){
	destroyCharacter(curModels[i].characterId);
      }
      
      free(curModels);
      curModels = NULL;
      curModelsSize = 0;
    }
  }
  
  fscanf(map, "%d %d %d \n", &gridY, &gridZ, &gridX);
  //lightSourcePos = (vec3){ gridX/2, gridY/2, gridZ/2 };
  
  int textureOfGround = texture1DIndexByName("Zemlia1");

  if (textureOfGround == -1) {
    printf("Specify texture of ground");
    exit(-1);
  }

  int blockCounter,jointsCounter,wallCounter,tileCounter;
  int wallsCounter = -1;

  int topWallsCounter;
  int leftWallsCounter;

  int tlsCounter;
  int blckCounter;
  int jntCounter[4];

  /*
  fscanf(map, "Walls used: %d(lw%d rw%d tl%d bck%d joi(%d %d %d %d)) b:%d j:%d w:%d t:%d", &wallsCounter,
	 &leftWallsCounter,
	 &topWallsCounter,
	 &tlsCounter,
	 &blckCounter,
	 &jntCounter[0], &jntCounter[1], &jntCounter[2], &jntCounter[3], 
	 &blockCounter,
	 &jointsCounter,
	 &wallCounter,
	 &tileCounter);
*/

  
  fscanf(map, "Used tiles: %d leftW: %d rightW: %d aBlock: %d",
	 &wallsCounter,
	 &leftWallsCounter,
	 &topWallsCounter,
	 &blckCounter);


  printf("Walls: left %d top %d \n", leftWallsCounter, topWallsCounter);

  wallsStorage = malloc(sizeof(Wall*) * (topWallsCounter + leftWallsCounter));
  //  for(int i=0;i<basicSideCounter;i++){
  //}

  blocksStorage = malloc(sizeof(TileBlock*) * blckCounter);
  tilesStorage = malloc(sizeof(Tile*) * wallsCounter);

  initGrid(gridZ, gridY, gridZ);

  /*	{
	float texturedTileVerts[] = {
	bBlockW, 0.0f, bBlockD , 0.0f, 1.0f,
	0.0f, 0.0f, bBlockD , 1.0f, 1.0f,
	bBlockW, 0.0f, 0.0f , 0.0f, 0.0f, 
      
	0.0f, 0.0f, bBlockD , 1.0f, 1.0f,
	bBlockW, 0.0f, 0.0f , 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f , 1.0f, 0.0f, 
	};

	glGenVertexArrays(1, grid[y][z][x]->groundPair.VAO);
	glBindVertexArray(grid[y][z][x]->groundPair.VAO);

	glGenBuffers(1, grid[y][z][x]->groundPair.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, grid[y][z][x]->groundPair.VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(texturedTileVerts), texturedTileVerts, GL_STATIC_DRAW);

	size_t s = 5 * sizeof(float) + sizeof(vec4) * 4;
	  
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, s, NULL);
	glEnableVertexAttribArray(0);


	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, s, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	glEnableVertexAttribArray(2); 
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, s, s);
	glEnableVertexAttribArray(3); 
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, s, s + (1 * vec4Size));
	glEnableVertexAttribArray(4); 
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, s, s + (2 * vec4Size));
	glEnableVertexAttribArray(5); 
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, s, s + (3 * vec4Size));

	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);

	glBindVertexArray(0);
	}

	grid[y][z][x]->groundPair.VAO = tileMeshes[i-1].VAO;
	grid[y][z][x]->groundPair.VBO = tileMeshes[i-1].VBO;
	
	glBindBuffer(GL_ARRAY_BUFFER, tileMeshes[i-1].VBO);
	glBindVertexArray(tileMeshes[i-1].VAO); 

	glGenBuffers(1, grid[y][z][x]->groundVAO);
  */
  printf("waaaaa %d \n", wallsCounter);
  for (int i = 0; i < wallsCounter; i++) {
    int x = -1,y = -1,z = -1;
    
    fscanf(map, "\n%d %d %d ",&x,&y,&z); 
	 
    int sidesCounter = 0;

    grid[y][z][x] = calloc(1, sizeof(Tile));

    tilesStorage[tilesStorageSize] = grid[y][z][x];
    grid[y][z][x]->pos = (vec3)xyz_indexesToCoords(x,y,z);
    grid[y][z][x]->id = tilesStorageSize;
    //tilesStorageSize++;
    
    Tile* tile = grid[y][z][x]; 

    float groundLiftDELETEIT;

    fscanf(map, "%c %d ", &tile->tx, &sidesCounter);

    //    vec3 grid = xyz_indexesToCoords(x,y,z);

    //    tile->pos = grid;

    int tx = tile->tx;  

    if(tx != -1){
      geomentyByTxCounter[tx] += sizeof(float) * 8 * 6;
    }
    else if (y == 0) {
        tile->tx = textureOfGround;
    }

    Side side = -1; 
    int bufSize;

    int alligned = 0;
    int txHidden = 0;
	  
    WallType wType = -1;

    // walls
    for(int i2=0;i2< sidesCounter;i2++){
      fscanf(map, "%d %d ", &side, &wType);

      tile->wall[side] = malloc(sizeof(Wall));
      tile->wall[side]->id = wallsStorageSize;
      tile->wall[side]->tileId = tilesStorageSize;
      tile->wall[side]->side = i2;

      wallsStorage[wallsStorageSize] = tile->wall[side];
      wallsStorageSize++;
      
      tile->wall[side]->type = wType;
      tile->wall[side]->prevType = wType;
      
      tile->wall[side]->planes = calloc(planesInfo[tile->wall[side]->type], sizeof(Plane));

      for(int i2=0;i2<wallsVPairs[wType].planesNum;i2++){
	fscanf(map, "%d %d ", &tile->wall[side]->planes[i2].txIndex, &tile->wall[side]->planes[i2].hide);

	if(!tile->wall[side]->planes[i2].hide){
	  geomentyByTxCounter[tile->wall[side]->planes[i2].txIndex] += wallsVPairs[wType].pairs[i2].vertexNum * sizeof(float) * wallsVPairs[wType].pairs[i2].attrSize;
	}
      }

      for(int mat=0;mat<16;mat++){
	fscanf(map, "%f ", &tile->wall[side]->mat.m[mat]);
      }

      for (int i = 0; i < wallsVPairs[wType].planesNum; i++) {
	calculateAABB(tile->wall[side]->mat, wallsVPairs[wType].pairs[i].vBuf, wallsVPairs[wType].pairs[i].vertexNum, wallsVPairs[wType].pairs[i].attrSize, &tile->wall[side]->planes[i].lb, &tile->wall[side]->planes[i].rt);
      }
    }

    int blockExists;
    fscanf(map, "%d ", &blockExists);

    if(blockExists){
      TileBlock* newBlock = malloc(sizeof(TileBlock));

      blocksStorage[blocksStorageSize] = newBlock;
      newBlock->id = blocksStorageSize;
      blocksStorageSize++;
      
      int txIndex;
      int blockType;
      int rotateAngle;

      fscanf(map, "%d %d %d %d ",&blockType, &rotateAngle, &txIndex);
      
      newBlock->txIndex = txIndex;
      newBlock->rotateAngle = rotateAngle;
      newBlock->type = blockType;
      newBlock->tileId = tilesStorageSize;
      
      geomentyByTxCounter[newBlock->txIndex] += blocksVPairs[newBlock->type].pairs[0].vertexNum * sizeof(float) * blocksVPairs[newBlock->type].pairs[0].attrSize;

      //      newBlock->vpair.VBO

      //      newBlock->vpair.VBO = tileBlocksTempl[newBlock->type].vpair.VBO;
      //      newBlock->vpair.VAO = tileBlocksTempl[newBlock->type].vpair.VAO;

      for(int mat=0;mat<16;mat++){
	fscanf(map, "%f ", &newBlock->mat.m[mat]);
      }
      
      calculateAABB(newBlock->mat, blocksVPairs[newBlock->type].pairs[0].vBuf, blocksVPairs[newBlock->type].pairs[0].vertexNum, blocksVPairs[newBlock->type].pairs[0].attrSize, &newBlock->lb, &newBlock->rt);

      tile->block = newBlock;
    }

    tilesStorageSize++;
  }
    
  fscanf(map, "\nUsed models: %d\n", &curModelsSize);

  if (curModelsSize != 0) { 
    curModels = malloc(curModelsSize * sizeof(Model));

    for (int i = 0; i < curModelsSize; i++) {
      int name = -1; 
      fscanf(map, "%d ", &name);

      if (name >= loadedModelsSize || name < 0) {
	printf("Models parsing error, model name (%d) doesnt exist \n", name);
	exit(0);
      }
	   
      curModels[i].name = name;
      curModels[i].id = i;
      curModels[i].characterId = -1;

      fgetc(map); // read [

      for (int mat = 0; mat < 16; mat++) {
	fscanf(map, "%f ", &curModels[i].mat.m[mat]); 
      }

      calculateModelAABB(&curModels[i]);

      fgetc(map); // read ]\n

      char ch = fgetc(map);

      if (ch == '1') {  
	charactersSize++; 

	if (!characters) { 
	  characters = malloc(charactersSize * sizeof(Character));
	}
	else {
	  characters = realloc(characters, charactersSize * sizeof(Character));
	}

	memset(&characters[charactersSize - 1], 0, sizeof(Character));

	curModels[i].characterId = charactersSize - 1;
	characters->id = charactersSize - 1;

	char named[32];
	int nameType = -1;

	fscanf(map, "%s %d ", &named, &nameType);

	characters[charactersSize-1].name = malloc(sizeof(char) * (strlen(named) + 1));
	strcpy(characters[charactersSize-1].name, named);  

	deserializeDialogTree(&characters[charactersSize-1].dialogs, NULL, map);
	//	fgetc(map);
      }
      else if(ch != '\n'){
	fgetc(map);
      }
    }
  
  }

  fscanf(map, "\nUsed planes: %d\n", &picturesStorageSize);

  if (curModelsSize != 0) {
      picturesStorage = malloc(picturesStorageSize * sizeof(Model));

      for (int i = 0; i < picturesStorageSize; i++) {
          int tx = -1;
          float w, h;

          fscanf(map, "%d %f %f ", &tx, &w, &h);

          if (tx >= loadedTexturesCounter || tx < 0) {
              printf("Models parsing error, model name (%d) doesnt exist \n", tx);
              exit(0);
          }

          picturesStorage[i].w = w;
          picturesStorage[i].h = h;
          picturesStorage[i].txIndex = tx;
          picturesStorage[i].id = i;
          picturesStorage[i].characterId = -1;

          fgetc(map); // read [

          for (int mat = 0; mat < 16; mat++) {
              fscanf(map, "%f ", &picturesStorage[i].mat.m[mat]);
          }

          //   calculateModelAABB(&curModels[i]);

          fgetc(map); // read ]\n

          char ch = fgetc(map);

          if (ch == '1') {
              charactersSize++;

              if (!characters) {
                  characters = malloc(charactersSize * sizeof(Character));
              }
              else {
                  characters = realloc(characters, charactersSize * sizeof(Character));
              }

              memset(&characters[charactersSize - 1], 0, sizeof(Character));

              picturesStorage[i].characterId = charactersSize - 1;
              characters->id = charactersSize - 1;

              char named[32];
              int nameType = -1;

              fscanf(map, "%s %d ", &named, &nameType);

              characters[charactersSize - 1].name = malloc(sizeof(char) * (strlen(named) + 1));
              strcpy(characters[charactersSize - 1].name, named);

              deserializeDialogTree(&characters[charactersSize - 1].dialogs, NULL, map);
              //	fgetc(map);
          }
          else if (ch != '\n') {
              fgetc(map);
          }
      }

  }


  printf("Save %s loaded! \n", save);
  fclose(map);

  strcpy(curSaveName, saveName);
  free(save);

  initSnowParticles();

  for (int i = 0; i < loadedTexturesCounter; i++) {
      printf("%d: %d \n", i, geomentyByTxCounter[i]);
  }

  return true;
}

bool saveMap(char* saveName) {
    char* save = calloc((strlen(saveName) + strlen(".doomer")), sizeof(char));

    strcat(save, saveName);
    strcat(save, ".doomer");

    FILE* map = fopen(save, "w+");

    fprintf(map, "%d %d %d \n", gridY, gridZ, gridX);

    int wallsCounter = 0;
    vec3i* wallsIndexes = malloc(sizeof(vec3i) * gridY * gridX * gridZ);

    int textureOfGround = texture1DIndexByName("Zemlia1");

    if (textureOfGround == -1) {
        printf("Specify texture of ground");
        exit(-1);
    }

    int blockCounter = 0;
    int jointsCounter = 0;
    int wallCounter = 0;
    int wallCounterBySide[2] = { 0 };
    int tileCounter = 0;

    int blckCounter = 0;
    int jntCounter[4] = { 0 };
    int tlsCounter = 0;

    for (int y = 0; y < gridY; y++) {
        for (int z = 0; z < gridZ; z++) {
            for (int x = 0; x < gridX; x++) {
                Tile* tile = grid[y][z][x];

                if (!tile) continue;

                int tx = tile->tx;

                //	bool acceptTile = (y == 0 && type == texturedTile &&  tx2 != textureOfGround) || (y != 0 && type == texturedTile);

                if (tile->block) {
                    blckCounter++;
                }

                if (tx != -1) {
                    tlsCounter++;
                }

                if (tile->wall[0]) {
                    wallCounterBySide[0]++;
                }

                if (tile->wall[1]) {
                    wallCounterBySide[1]++;
                }

                if (tile->wall[0] || tile->wall[1]) {
                    wallsIndexes[wallsCounter] = (vec3i){ x,y,z };
                    wallsCounter++;
                    wallCounter++;
                }
                else if (tx != -1) {
                    wallsIndexes[wallsCounter] = (vec3i){ x,y,z };
                    wallsCounter++;
                    tileCounter++;
                }
                else if (tile->block) {
                    wallsIndexes[wallsCounter] = (vec3i){ x,y,z };
                    wallsCounter++;
                    blockCounter++;
                }
            }
        }
    }
    
    fprintf(map, "Used tiles: %d leftW: %d rightW: %d aBlock: %d",
	    wallsCounter,
	    wallCounterBySide[0],
	    wallCounterBySide[1],
	    blckCounter);

    for (int i = 0; i < wallsCounter; i++) {
        fprintf(map, "\n");

        int x = wallsIndexes[i].x;
        int y = wallsIndexes[i].y;
        int z = wallsIndexes[i].z;

        Tile* tile = grid[y][z][x];

        int sidesAvaible = 0;

        if (tile->wall[0]) {
            sidesAvaible++;
        }

        if (tile->wall[1]) {
            sidesAvaible++;
        }

        fprintf(map, "%d %d %d %c %d ", x, y, z, grid[y][z][x]->tx, sidesAvaible);

        // walls
      //  for(int i1=0;i1<basicSideCounter;i1++){
        for (int i1 = 0; i1 < 2; i1++) {

            if (tile->wall[i1]) {
                fprintf(map, "%d %d ", i1, tile->wall[i1]->type);

                // plane data save
                for (int i2 = 0; i2 < planesInfo[tile->wall[i1]->type]; i2++) {
                    fprintf(map, "%d %d ", tile->wall[i1]->planes[i2].txIndex,
			    tile->wall[i1]->planes[i2].hide);
                }

                for (int mat = 0; mat < 16; mat++) {
                    fprintf(map, "%f ", tile->wall[i1]->mat.m[mat]);
                }
            }
        }

        if (tile->block) {
            fprintf(map, "%d ", 1); // block exists
            fprintf(map, "%d %d %d %d ", tile->block->type, tile->block->rotateAngle, tile->block->txIndex);

            //      for(int i2=0;i2<tile->block->vertexesSize*5;i2++){
            //	fprintf(map, "%f ",tile->block->vertexes[i2]);
            //      }

            for (int mat = 0; mat < 16; mat++) {
                fprintf(map, "%f ", tile->block->mat.m[mat]);
            }
        }
        else {
            fprintf(map, "%d ", 0); // block doesnt exists
        }
    }

    free(wallsIndexes);

    fprintf(map, "\nUsed models: %d\n", curModelsSize);

    for (int i = 0; i < curModelsSize; i++) {
        fprintf(map, "%d ", curModels[i].name);

        fprintf(map, "[");

        for (int mat = 0; mat < 16; mat++) {
            fprintf(map, "%f ", curModels[i].mat.m[mat]);
        }

        fprintf(map, "]");

        if (curModels[i].characterId != -1) {
            fprintf(map, "1");
            fprintf(map, "%s %d ", characters[curModels[i].characterId].name ? characters[curModels[i].characterId].name : "None", characters[curModels[i].characterId].modelName);
            serializeDialogTree(&characters[curModels[i].characterId].dialogs, map);
            fprintf(map, "\n");
        }
        else {
            fprintf(map, "0\n");
        }
    }

    fprintf(map, "Used planes: %d\n", picturesStorageSize);

    for (int i = 0; i < picturesStorageSize; i++) {
        fprintf(map, "%d %f %f ", picturesStorage[i].txIndex, picturesStorage[i].w, picturesStorage[i].h);

        fprintf(map, "[");

        for (int mat = 0; mat < 16; mat++) {
            fprintf(map, "%f ", picturesStorage[i].mat.m[mat]);
        }

        fprintf(map, "]");

        if (picturesStorage[i].characterId != -1) {
            fprintf(map, "1");
            fprintf(map, "%s %d ", characters[picturesStorage[i].characterId].name ? characters[picturesStorage[i].characterId].name : "None", characters[picturesStorage[i].characterId].modelName);
            serializeDialogTree(&characters[picturesStorage[i].characterId].dialogs, map);
      fprintf(map, "\n");
    }
    else {
      fprintf(map, "0\n");
    }

  }

  // save characters
  /*  fprintf(map, "\Characters\n");
  
      for(int i=0; i<charactersSize; i++){
      fprintf(map, "%s %d ", characters[i].name ? characters[i].name : "None", characters[i].modelName);
      serializeDialogTree(&characters[i].dialogs, map);  
      }*/

  printf("Map saved!\n");

  fclose(map);

  return true;
}

bool createMap(int newX, int newY, int newZ){
  resetMouse();

  if(grid){
    for (int y = 0; y < gridY; y++) {
      for (int z = 0; z < gridZ; z++) {
	for (int x = 0; x < gridX; x++) {
	  free(grid[y][z][x]);
	}
	
	free(grid[y][z]);
      }
       
      free(grid[y]);
    }
    
    free(grid);
    grid = NULL;

    if (curModels) {
      free(curModels);
      curModels = NULL;
      curModelsSize = 0;
    }
  }

  int textureOfGround = texture1DIndexByName("Zemlia1");

  if (textureOfGround == -1) {
    printf("Specify texture of ground"); 
    exit(-1);
  }

  gridX = newX;
  gridY = newY;
  gridZ = newZ;
  
  initGrid(gridZ, gridY, gridZ);
  
  initSnowParticles();

  batchAllGeometry();

  return true;
}

void initSnowParticles(){
  if(snowParticle){
    free(snowParticle);
    free(snowMeshVertixes);
    snowParticle = NULL;
  }
  
  // init snow particles
  {
    FILE *snowConf = fopen("snow.txt","r");
    
    if(snowConf){
      fscanf(snowConf,"AMOUNT=%d\nSPEED=%f\n", &snowAmount, &snowSpeed);
      fclose(snowConf);
    }else{
      snowAmount = snowDefAmount;
      snowSpeed = snowGravity;
    }

    snowAmount *= 0;

    snowParticle = (Particle*)malloc(sizeof(Particle) * snowAmount);
    snowMeshVertixes = malloc(sizeof(float) * 2 * 3 * snowAmount);  

    for (int loop = 0; loop < snowAmount; loop++)
      {
	snowParticle[loop].active = true;

	snowParticle[loop].life = 1.0f;
	snowParticle[loop].fade = (float)(rand() % 100) / 1000.0f + 0.003f;

	snowParticle[loop].x = (float)(rand() % gridX) + (float)(rand() % 100 / 1000.0f);
	snowParticle[loop].y = (float)(rand() % (int)(gridY * floorH)) + (float)(rand() % 1000) / 1000.0f;
	snowParticle[loop].z = (float)(rand() % gridZ) + (float)(rand() % 100 / 1000.0f);
      }
  }


  printf("tiles %d \n",tilesStorageSize);
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

TileBlock* constructNewBlock(int type, int angle){
  TileBlock* newBlock = calloc(1, sizeof(TileBlock));

  newBlock->mat = IDENTITY_MATRIX;

  if(mouse.selectedType == mouseTileT){
    TileMouseData* tileData = (TileMouseData*) mouse.selectedThing;

    vec3 tile = tilesStorage[tileData->tileId]->pos;// xyz_indexesToCoords(tileData->grid.x, curFloor, tileData->grid.z);
    
    newBlock->mat.m[12] = tile.x;
    newBlock->mat.m[13] = tile.y;
    newBlock->mat.m[14] = tile.z;

    printf("newBlock %f %f %f \n", argVec3(tile));
	 
   // newBlock->tile = tileData->tile;
  }
  
  newBlock->rotateAngle = angle;
  newBlock->type = type;
  newBlock->txIndex = 0;

  calculateAABB(newBlock->mat, blocksVPairs[newBlock->type].pairs[0].vBuf, blocksVPairs[newBlock->type].pairs[0].vertexNum, blocksVPairs[newBlock->type].pairs[0].attrSize, &newBlock->lb, &newBlock->rt);

  return newBlock;
}

float* wallBySide(int* bufSize,Side side, float thick){
  float* buf = NULL;

  float w = 1;
  float d = 1;
  float h = 2;

  
  float t = (float)1/8;

  if(side == right){
    d = t;

    float capH = h * 0.12f;
    float botH = h * 0.4f;
	
    float verts[] = {
      // cap top
      0.0f, h, -t, 0.0f, 0.0f,
      w, h, -t,    t, 0.0f,
      0.0f,h, d,   0.0f, 1.0f,

      w, h, -t,    t, 0.0f,
      0.0f,h,d,    0.0f, 1.0f,
      w,h,d,       t, 1.0f,
		
      // cap bot
      0.0f, h-capH, -t, 0.0f, 0.0f,
      w,  h-capH, -t,    t, 0.0f,
      0.0f, h-capH, d,   0.0f, 1.0f,

      w,  h-capH, -t,    t, 0.0f,
      0.0f, h-capH,d,    0.0f, 1.0f,
      w, h-capH,d,       t, 1.0f,


      // cap back
      0.0f, h, -t,      0.0f, 1.0f,
      w, h, -t,         1.0f, 1.0f,
      0.0f, h -capH, -t,  0.0f, 0.0f,

      w, h, -t,         1.0f, 1.0f,
      0.0f, h -capH, -t,  0.0f, 0.0f,
      w, h -capH, -t,     1.0f, 0.0f,

      // cap left
      0.0f, h -capH, -t,  0.0f, 0.0f,
      0.0f, h, -t,     0.0f, 1.0f,
      0.0f, h , d,     t, 1.0f,

      0.0f, h, d,      t, 1.0f,
      0.0f, h -capH, -t,  0.0f, 0.0f,
      0.0f, h -capH, d,  t, 0.0f,

      // cap right
      w, h -capH, -t,     0.0f, 0.0f,
      w, h, -t,        0.0f, 1.0f,
      w, h , d,        t, 1.0f,

      w, h, d,         t, 1.0f,
      w, h -capH, -t,     0.0f, 0.0f,
      w, h -capH , d,     t, 0.0f,

      // cap front
      0.0f, h, d,      0.0f, 1.0f,
      w, h, d,         1.0f, 1.0f,
      0.0f, h -capH , d,  0.0f, 0.0f,

      w, h, d,         1.0f, 1.0f,
      0.0f, h -capH , d,  0.0f, 0.0f,
      w, h -capH , d,     1.0f, 0.0f,

      //
      // bot top
      0.0f, botH, -t, 0.0f, 0.0f,
      w, botH, -t,    t, 0.0f,
      0.0f,botH, d,   0.0f, 1.0f,

      w, botH, -t,    t, 0.0f,
      0.0f,botH,d,    0.0f, 1.0f,
      w,botH,d,       t, 1.0f,

      // bot back
      0.0f, botH, -t,      0.0f, 1.0f,
      w, botH, -t,         1.0f, 1.0f,
      0.0f, 0.0f , -t,  0.0f, 0.0f,

      w, botH, -t,         1.0f, 1.0f,
      0.0f, 0.0f , -t,  0.0f, 0.0f,
      w, 0.0f , -t,     1.0f, 0.0f,

      // bot left
      0.0f, 0.0f, -t,  0.0f, 0.0f,
      0.0f, botH, -t,     0.0f, 1.0f,
      0.0f, botH , d,     t, 1.0f,

      0.0f, botH, d,      t, 1.0f,
      0.0f, 0.0f, -t,  0.0f, 0.0f,
      0.0f, 0.0f , d,  t, 0.0f,

      // bot right
      w, 0.0f, -t,     0.0f, 0.0f,
      w, botH, -t,        0.0f, 1.0f,
      w, botH , d,        t, 1.0f,

      w, botH, d,         t, 1.0f,
      w, 0.0f, -t,     0.0f, 0.0f,
      w, 0.0f , d,     t, 0.0f,

      // bot front
      0.0f, botH, d,      0.0f, 1.0f,
      w, botH, d,         1.0f, 1.0f,
      0.0f, 0.0f , d,  0.0f, 0.0f,

      w, botH, d,         1.0f, 1.0f,
      0.0f, 0.0f , d,  0.0f, 0.0f,
      w, 0.0f , d,     1.0f, 0.0f
    };

    *bufSize = sizeof(verts);
    buf = malloc(sizeof(verts));
    memcpy(buf, verts, sizeof(verts));
    return buf;
  }else if(side == top){
    d = t;
    	
    float verts[] = {
      // top
      0.0f, h, -t, 0.0f, 0.0f,
      w, h, -t,    t, 0.0f,
      0.0f,h, d,   0.0f, 1.0f,

      w, h, -t,    t, 0.0f,
      0.0f,h,d,    0.0f, 1.0f,
      w,h,d,       t, 1.0f,

      // back
      0.0f, h, -t,      0.0f, 1.0f,
      w, h, -t,         1.0f, 1.0f,
      0.0f, 0.0f , -t,  0.0f, 0.0f,

      w, h, -t,         1.0f, 1.0f,
      0.0f, 0.0f , -t,  0.0f, 0.0f,
      w, 0.0f , -t,     1.0f, 0.0f,

      //left
      0.0f, 0.0f, -t,  0.0f, 0.0f,
      0.0f, h, -t,     0.0f, 1.0f,
      0.0f, h , d,     t, 1.0f,

      0.0f, h, d,      t, 1.0f,
      0.0f, 0.0f, -t,  0.0f, 0.0f,
      0.0f, 0.0f , d,  t, 0.0f,

      // right
      w, 0.0f, -t,     0.0f, 0.0f,
      w, h, -t,        0.0f, 1.0f,
      w, h , d,        t, 1.0f,

      w, h, d,         t, 1.0f,
      w, 0.0f, -t,     0.0f, 0.0f,
      w, 0.0f , d,     t, 0.0f,

      // front
      0.0f, h, d,      0.0f, 1.0f,
      w, h, d,         1.0f, 1.0f,
      0.0f, 0.0f , d,  0.0f, 0.0f,

      w, h, d,         1.0f, 1.0f,
      0.0f, 0.0f , d,  0.0f, 0.0f,
      w, 0.0f , d,     1.0f, 0.0f
    };

    *bufSize = sizeof(verts);
    buf = malloc(sizeof(verts));
    memcpy(buf, verts, sizeof(verts));
    return buf;
  }else if(side == bot){
        	
    float verts[] = {
      // top
      0.0f, h, d-t, 0.0f, 1.0f,
      w, h, d-t,    t, 1.0f,
      0.0f,h, d+t,   0.0f, 0.0f,

      w, h, d-t,    t, 1.0f,
      0.0f,h,d+t,    0.0f, 0.0f,
      w,h,d+t,       t, 0.0f,

      // back
      0.0f, h, d-t,      0.0f, 1.0f,
      w, h, d-t,         1.0f, 1.0f,
      0.0f, 0.0f , d-t,  0.0f, 0.0f,

      w, h, d-t,         1.0f, 1.0f,
      0.0f, 0.0f , d-t,  0.0f, 0.0f,
      w, 0.0f , d-t,     1.0f, 0.0f,

      //left
      0.0f, 0.0f, d-t,  0.0f, 0.0f,
      0.0f, h, d-t,     0.0f, 1.0f,
      0.0f, h , d+t,     t, 1.0f,

      0.0f, h, d+t,      t, 1.0f,
      0.0f, 0.0f, d-t,  0.0f, 0.0f,
      0.0f, 0.0f , d+t,  t, 0.0f,

      // right
      w, 0.0f, d-t,     0.0f, 0.0f,
      w, h, d-t,        0.0f, 1.0f,
      w, h , d+t,        t, 1.0f,

      w, h, d+t,         t, 1.0f,
      w, 0.0f, d-t,     0.0f, 0.0f,
      w, 0.0f , d+t,     t, 0.0f,

      // front
      0.0f, h, d+t,      0.0f, 1.0f,
      w, h, d+t,         1.0f, 1.0f,
      0.0f, 0.0f , d+t,  0.0f, 0.0f,

      w, h, d+t,         1.0f, 1.0f,
      0.0f, 0.0f , d+t,  0.0f, 0.0f,
      w, 0.0f , d+t,     1.0f, 0.0f
    };

    //  memcpy(wall, verts, sizeof(verts));
  };

  // return wall; 
}

void assembleHalfWallBlockVBO() {
  float w = 1;  
  float h = 0.8f;
  float texH = 2 * 0.5f * 0.8f;

  float capRatio = 0.12f;
  
  float t = (float)1 / 8;

  float d = t;
  
  // halfWallT
  {
      float frontPlane[] = {
      0.0f, 0.0f , -t,  0.0f, 0.0f,
      0.0f, h, -t,      0.0f, texH,
      w, h, -t,         1.0f, texH,

      0.0f, 0.0f , -t,  0.0f, 0.0f, // 1
      w, h, -t,         1.0f, texH, // 3
      w, 0.0f , -t,     1.0f, 0.0f, // 2
    };

    float topPlane[] = {
      w, h, -t,    1.0f, 0.0f,
      0.0f,h, d,   0.0f, capRatio, // 1
      0.0f, h, -t, 0.0f, 0.0f,

      w, h, -t,    1.0f, 0.0f,
      w,h,d,       1.0f, capRatio,
      0.0f,h,d,    0.0f, capRatio, // 1
    };

    float backPlane[] = {
      0.0f, 0.0f , d,  0.0f, 0.0f,
      w, h, d,         1.0f, texH,
      0.0f, h, d,      0.0f, texH,

      0.0f, 0.0f , d,  0.0f, 0.0f,
      w, 0.0f , d,     1.0f, 0.0f,
      w, h, d,         1.0f, texH,
    };
  
    float* wallPlanes[wPlaneCounter] = { [wTopPlane] = topPlane, [wFrontPlane] = frontPlane, [wBackPlane] = backPlane };
  
    int wallPlanesSize[wPlaneCounter] = { [wTopPlane] = sizeof(topPlane),[wFrontPlane] = sizeof(frontPlane),[wBackPlane] = sizeof(backPlane) };

    wallsVPairs[halfWallT].pairs = malloc(sizeof(VPair) * wPlaneCounter);    
    wallsVPairs[halfWallT].planesNum = wPlaneCounter;

    for(int i=0;i<wallsVPairs[halfWallT].planesNum;i++){
      attachNormalsToBuf(wallsVPairs[halfWallT].pairs, i, wallPlanesSize[i], wallPlanes[i]);
    }
  }
}
   
void assembleWallBlockVBO() {
  float w = 1;  
  float h = 2;

  float capRatio = 0.12f;
  
  float t = (float)1 / 8;

  float d = t;

  // norm WALL T
  {
    float frontPlane[] = {
      0.0f, 0.0f , -t,  0.0f, 0.0f,
      0.0f, h, -t,      0.0f, 1.0f,
      w, h, -t,         1.0f, 1.0f,

      0.0f, 0.0f , -t,  0.0f, 0.0f, // 1
      w, h, -t,         1.0f, 1.0f, // 3
      w, 0.0f , -t,     1.0f, 0.0f, // 2
    };

    float topPlane[] = {
      w, h, -t,    1.0f, 0.0f,
      0.0f,h, d,   0.0f, capRatio, // 1
      0.0f, h, -t, 0.0f, 0.0f,

      w, h, -t,    1.0f, 0.0f,
      w,h,d,       1.0f, capRatio,
      0.0f,h,d,    0.0f, capRatio, // 1
    };

    float backPlane[] = {
      0.0f, 0.0f , d,  0.0f, 1.0f,
      w, h, d,         1.0f, 0.0f,
      0.0f, h, d,      0.0f, 0.0f,

      0.0f, 0.0f , d,  0.0f, 1.0f,
      w, 0.0f , d,     1.0f, 1.0f,
      w, h, d,         1.0f, 0.0f,
    };

    /*    float closePlane[] = {
      0.0f, 0.0f , d,  0.0f, 1.0f,
      w, h, d,         1.0f, 0.0f,
      0.0f, h, d,      0.0f, 0.0f,

      0.0f, 0.0f , d,  0.0f, 1.0f,
      w, 0.0f , d,     1.0f, 1.0f,
      w, h, d,         1.0f, 0.0f,
    };*/
  
  
    float* wallPlanes[wPlaneCounter] = { [wTopPlane] = topPlane, [wFrontPlane] = frontPlane, [wBackPlane] = backPlane };
  
    int wallPlanesSize[wPlaneCounter] = { [wTopPlane] = sizeof(topPlane),[wFrontPlane] = sizeof(frontPlane),[wBackPlane] = sizeof(backPlane) };

    wallsVPairs[normWallT].pairs = malloc(sizeof(VPair) * (wPlaneCounter-1));    
    wallsVPairs[normWallT].planesNum = wPlaneCounter-1;

    for(int i=0;i<wallsVPairs[normWallT].planesNum;i++){
      attachNormalsToBuf(wallsVPairs[normWallT].pairs, i, wallPlanesSize[i], wallPlanes[i]);
    }
  }
  
  // LR WALL T
  {   
    float frontPlane[] = {
      0.0f, 0.0f , -t,  0.0f, 0.0f,
      0.0f, h, -t,      0.0f, 1.0f,
      w, h, -t,         1.0f, 1.0f,

      0.0f, 0.0f , -t,  0.0f, 0.0f, // 1
      w, h, -t,         1.0f, 1.0f, // 3
      w, 0.0f , -t,     1.0f, 0.0f, // 2

      // left ex
      -t, 0.0f , -d,  0.0f, 0.0f,
      0.0f, h, -d,         capRatio, 1.0f,
      -t, h, -d,      0.0f, 1.0f,

      -t, 0.0f , -d,  0.0f, 0.0f,
      0.0f, 0.0f , -d,     capRatio, 0.0f,
      0.0f, h, -d,         capRatio, 1.0f,

      // right ex
      w, 0.0f , -d,  0.0f, 0.0f,
      w+t, h, -d,         capRatio, 1.0f,
      w, h, -d,      0.0f, 1.0f,

      w, 0.0f , -d,  0.0f, 0.0f,
      w+t, 0.0f , -d,     capRatio, 0.0f,
      w+t, h, -d,         capRatio, 1.0f,
    };

    float closePlane[] = {
      // left ex
      -t, 0.0f , -d,      0.0f, 0.0f,
      t, h, d,          capRatio *2, 1.0f,
      -t, h, -d,          0.0f, 1.0f,

      -t, 0.0f , -d,      0.0f, 0.0f,
      t, 0.0f , d,     capRatio*2, 0.0f,
      t, h, d,         capRatio*2, 1.0f,
      
      // right ex
      w-t, 0.0f , d,      0.0f, 0.0f,
      w+t, h, -d,         capRatio *2, 1.0f,
      w-t, h, d,          0.0f, 1.0f,

      w-t, 0.0f , d,      0.0f, 0.0f,
      w+t, 0.0f , -d,     capRatio*2, 0.0f,
      w+t, h, -d,         capRatio*2, 1.0f,
    };

    float topPlane[] = {
      w+t, h, -t,    1.0f, 0.0f,
      t, h, d,   0.0f, capRatio, // 1
      -t, h, -t, 0.0f, 0.0f,

      w+t, h, -t,    1.0f, 0.0f,
      w-t,h,d,       1.0f, capRatio,
      t,h,d,    0.0f, capRatio, // 1
    };

    float backPlane[] = {
      t, 0.0f , d,  0.0f, 1.0f,
      w-t, h, d,         1.0f, 0.0f,
      t, h, d,      0.0f, 0.0f,

      t, 0.0f , d,  0.0f, 1.0f,
      w-t, 0.0f , d,     1.0f, 1.0f,
      w-t, h, d,         1.0f, 0.0f,
    };
  
    float* wallPlanes[wPlaneCounter] = { [wTopPlane] = topPlane, [wFrontPlane] = frontPlane, [wBackPlane] = backPlane, [wClosePlane] = closePlane };
  
    int wallPlanesSize[wPlaneCounter] = { [wTopPlane] = sizeof(topPlane),[wFrontPlane] = sizeof(frontPlane),[wBackPlane] = sizeof(backPlane),  [wClosePlane] = sizeof(closePlane) };

    wallsVPairs[LRWallT].pairs = malloc(sizeof(VPair) * wPlaneCounter);    
    wallsVPairs[LRWallT].planesNum = wPlaneCounter;

    for(int i=0;i<wallsVPairs[LRWallT].planesNum;i++){
      attachNormalsToBuf(wallsVPairs[LRWallT].pairs, i, wallPlanesSize[i], wallPlanes[i]);
    }
  }

  // L WALL T
  {   
    float frontPlane[] = {
      0.0f, 0.0f , -t,  0.0f, 0.0f,
      0.0f, h, -t,      0.0f, 1.0f,
      w, h, -t,         1.0f, 1.0f,

      0.0f, 0.0f , -t,  0.0f, 0.0f, // 1
      w, h, -t,         1.0f, 1.0f, // 3
      w, 0.0f , -t,     1.0f, 0.0f, // 2


      // left ex
      -t, 0.0f , -d,  0.0f, 0.0f,
      0.0f, h, -d,         capRatio, 1.0f,
      -t, h, -d,      0.0f, 1.0f,

      -t, 0.0f , -d,  0.0f, 0.0f,
      0.0f, 0.0f , -d,     capRatio, 0.0f,
      0.0f, h, -d,         capRatio, 1.0f,
    };

    float topPlane[] = {
      w, h, -t,    1.0f, 0.0f,
      t, h, d,   0.0f, capRatio, // 1
      -t, h, -t, 0.0f, 0.0f,

      w, h, -t,    1.0f, 0.0f,
      w,h,d,       1.0f, capRatio,
      t,h,d,    0.0f, capRatio, // 1
    };

    float backPlane[] = {
      t, 0.0f , d,  0.0f, 1.0f,
      w, h, d,         1.0f, 0.0f,
      t, h, d,      0.0f, 0.0f,

      t, 0.0f , d,  0.0f, 1.0f,
      w, 0.0f , d,     1.0f, 1.0f,
      w, h, d,         1.0f, 0.0f,
    };

    
    float closePlane[] = {
      // left ex
      -t, 0.0f , -d,      0.0f, 0.0f,
      t, h, d,          capRatio *2, 1.0f,
      -t, h, -d,          0.0f, 1.0f,

      -t, 0.0f , -d,      0.0f, 0.0f,
      t, 0.0f , d,     capRatio*2, 0.0f,
      t, h, d,         capRatio*2, 1.0f,
    };
  
    float* wallPlanes[wPlaneCounter] = { [wTopPlane] = topPlane, [wFrontPlane] = frontPlane, [wBackPlane] = backPlane, [wClosePlane] = closePlane };
  
    int wallPlanesSize[wPlaneCounter] = { [wTopPlane] = sizeof(topPlane),[wFrontPlane] = sizeof(frontPlane),[wBackPlane] = sizeof(backPlane),  [wClosePlane] = sizeof(closePlane) };

    wallsVPairs[LWallT].pairs = malloc(sizeof(VPair) * wPlaneCounter);    
    wallsVPairs[LWallT].planesNum = wPlaneCounter;

    for(int i=0;i<wallsVPairs[LWallT].planesNum;i++){
      attachNormalsToBuf(wallsVPairs[LWallT].pairs, i, wallPlanesSize[i], wallPlanes[i]);
    }
  }
  
  // R WALL T
  {
    float frontPlane[] = {
      0.0f, 0.0f , -t,  0.0f, 0.0f,
      0.0f, h, -t,      0.0f, 1.0f,
      w, h, -t,         1.0f, 1.0f,

      0.0f, 0.0f , -t,  0.0f, 0.0f, // 1
      w, h, -t,         1.0f, 1.0f, // 3
      w, 0.0f , -t,     1.0f, 0.0f, // 2

      // right ex
      w, 0.0f , -d,  0.0f, 0.0f,
      w+t, h, -d,         capRatio, 1.0f,
      w, h, -d,      0.0f, 1.0f,

      w, 0.0f , -d,  0.0f, 0.0f,
      w+t, 0.0f , -d,     capRatio, 0.0f,
      w+t, h, -d,         capRatio, 1.0f,
    };

     float topPlane[] = {
      w+t, h, -t,    1.0f, 0.0f,
      0.0f, h, d,   0.0f, capRatio, // 1
      0.0f, h, -t, 0.0f, 0.0f,

      w+t, h, -t,    1.0f, 0.0f,
      w-t,h,d,       1.0f, capRatio,
      0.0f,h,d,    0.0f, capRatio, // 1
    };

    float backPlane[] = {
      0.0f, 0.0f , d,  0.0f, 1.0f,
      w-t, h, d,         1.0f, 0.0f,
      0.0f, h, d,      0.0f, 0.0f,

      0.0f, 0.0f , d,  0.0f, 1.0f,
      w-t, 0.0f , d,     1.0f, 1.0f,
      w-t, h, d,         1.0f, 0.0f,
    };
      
    float closePlane[] = {
      // right ex
      w-t, 0.0f , d,      0.0f, 0.0f,
      w+t, h, -d,         capRatio *2, 1.0f,
      w-t, h, d,          0.0f, 1.0f,

      w-t, 0.0f , d,      0.0f, 0.0f,
      w+t, 0.0f , -d,     capRatio*2, 0.0f,
      w+t, h, -d,         capRatio*2, 1.0f,
    };
  
    float* wallPlanes[wPlaneCounter] = { [wTopPlane] = topPlane, [wFrontPlane] = frontPlane, [wBackPlane] = backPlane, [wClosePlane] = closePlane };
  
    int wallPlanesSize[wPlaneCounter] = { [wTopPlane] = sizeof(topPlane),[wFrontPlane] = sizeof(frontPlane),[wBackPlane] = sizeof(backPlane),  [wClosePlane] = sizeof(closePlane) };

    wallsVPairs[RWallT].pairs = malloc(sizeof(VPair) * wPlaneCounter);    
    wallsVPairs[RWallT].planesNum = wPlaneCounter;

    for(int i=0;i<wallsVPairs[RWallT].planesNum;i++){
      attachNormalsToBuf(wallsVPairs[RWallT].pairs, i, wallPlanesSize[i], wallPlanes[i]);
    }
  }
}

void assembleHideWallBlockVBO() {  // wallBlock buf
  float w = 1;
  float h = 0.15f;
  float hiddenH = 2.0f - 0.15f;//0.15f;

  float capRatio = 0.12f;

  float yTex = .5f * .15f;
  
  float t = (float)1 / 8;

  float d = t;

  {
    float frontPlaneHidden[] = {
      0.0f, h, -t,            0.0f, 0.0f,
      0.0f, hiddenH, -t,      0.0f, 0.0f,
      w, hiddenH, -t,         0.0f, 0.0f,

      0.0f, h, -t,            0.0f, 0.0f, // 1
      w, hiddenH, -t,         0.0f, 0.0f, // 3
      w, h, -t,               0.0f, 0.0f, // 2
    };
  
    float backPlaneHidden[] = {
      0.0f, h , d,           0.0f, 0.0f,
      w, hiddenH, d,         0.0f, 0.0f,
      0.0f, hiddenH, d,      0.0f, 0.0f,

      0.0f, h, d,            0.0f, 0.0f,
      w, h, d,               0.0f, 0.0f,
      w, hiddenH, d,         0.0f, 0.0f,
    };

   
    float frontPlane[] = {
      0.0f, 0.0f , -t,  0.0f, 0.0f,
      0.0f, h, -t,      0.0f, yTex,
      w, h, -t,         1.0f, yTex,

      0.0f, 0.0f , -t,  0.0f, 0.0f, // 1
      w, h, -t,         1.0f, yTex, // 3
      w, 0.0f , -t,     1.0f, 0.0f, // 2 
    };

    float topPlane[] = {
      w, h, -t,    1.0f, 0.0f,
      0.0f,h, d,   0.0f, capRatio, // 1
      0.0f, h, -t, 0.0f, 0.0f,

      w, h, -t,    1.0f, 0.0f,
      w,h,d,       1.0f, capRatio,
      0.0f,h,d,    0.0f, capRatio, // 1
    };

    float backPlane[] = {
      0.0f, 0.0f , d,  0.0f, 0.0f,
      w, h, d,         1.0f, yTex,
      0.0f, h, d,      0.0f, yTex,

      0.0f, 0.0f , d,  0.0f, 0.0f,
      w, 0.0f , d,     1.0f, 0.0f,
      w, h, d,         1.0f, yTex,
    };

    float* wallPlanes[wPlaneCounter] = {
      [wTopPlane] = topPlane,
      [wFrontPlane] = frontPlane,
      [wBackPlane] = backPlane,
    };
  
    int wallPlanesSize[wPlaneCounter] = {
      [wTopPlane] = sizeof(topPlane),
      [wFrontPlane] = sizeof(frontPlane),
      [wBackPlane] = sizeof(backPlane),
    };

    wallsVPairs[hiddenWallT].pairs = malloc(sizeof(VPair) * (wPlaneCounter-1));    
    wallsVPairs[hiddenWallT].planesNum = wPlaneCounter - 1;

    for(int i=0;i<wallsVPairs[hiddenWallT].planesNum;i++){
      attachNormalsToBuf(wallsVPairs[hiddenWallT].pairs, i, wallPlanesSize[i], wallPlanes[i]);
    }
  }

  // hiddenLRWallT
  {
    float frontPlane[] = {
      0.0f, 0.0f , -t,  0.0f, 0.0f,
      0.0f, h, -t,      0.0f, 1.0f,
      w, h, -t,         1.0f, 1.0f,

      0.0f, 0.0f , -t,  0.0f, 0.0f, // 1
      w, h, -t,         1.0f, 1.0f, // 3
      w, 0.0f , -t,     1.0f, 0.0f, // 2
    };

    float closePlane[] = {
      
      // left ex
      -t, 0.0f , -d,  0.0f, 0.0f,
      0.0f, h, -d,         capRatio, 1.0f,
      -t, h, -d,      0.0f, 1.0f,

      -t, 0.0f , -d,  0.0f, 0.0f,
      0.0f, 0.0f , -d,     capRatio, 0.0f,
      0.0f, h, -d,         capRatio, 1.0f,

      // right ex
      w, 0.0f , -d,  0.0f, 0.0f,
      w+t, h, -d,         capRatio, 1.0f,
      w, h, -d,      0.0f, 1.0f,

      w, 0.0f , -d,  0.0f, 0.0f,
      w+t, 0.0f , -d,     capRatio, 0.0f,
      w+t, h, -d,         capRatio, 1.0f,
    };

    float topPlane[] = {
      w+t, h, -t,    1.0f, 0.0f,
      t, h, d,   0.0f, capRatio, // 1
      -t, h, -t, 0.0f, 0.0f,

      w+t, h, -t,    1.0f, 0.0f,
      w-t,h,d,       1.0f, capRatio,
      t,h,d,    0.0f, capRatio, // 1
    };

    float backPlane[] = {
      t, 0.0f , d,  0.0f, 0.0f,
      w-t, h, d,         1.0f, 1.0f,
      t, h, d,      0.0f, 1.0f,

      t, 0.0f , d,  0.0f, 0.0f,
      w-t, 0.0f , d,     1.0f, 0.0f,
      w-t, h, d,         1.0f, 1.0f,
    };

    float* wallPlanes[wPlaneCounter] = {
      [wTopPlane] = topPlane,
      [wFrontPlane] = frontPlane,
      [wBackPlane] = backPlane,
      [wClosePlane] = closePlane,
    };
  
    int wallPlanesSize[wPlaneCounter] = {
      [wTopPlane] = sizeof(topPlane),
      [wFrontPlane] = sizeof(frontPlane),
      [wBackPlane] = sizeof(backPlane),
      [wClosePlane] = sizeof(closePlane),
    };

    wallsVPairs[hiddenLRWallT].pairs = malloc(sizeof(VPair) * wPlaneCounter);    
    wallsVPairs[hiddenLRWallT].planesNum = wPlaneCounter;

    for(int i=0;i<wallsVPairs[hiddenLRWallT].planesNum;i++){
      attachNormalsToBuf(wallsVPairs[hiddenLRWallT].pairs, i, wallPlanesSize[i], wallPlanes[i]);
    }
  }

  
  // RWallT
  {
    float frontPlane[] = {
      0.0f, 0.0f , -t,  0.0f, 0.0f,
      0.0f, h, -t,      0.0f, 1.0f,
      w, h, -t,         1.0f, 1.0f,

      0.0f, 0.0f , -t,  0.0f, 0.0f, // 1
      w, h, -t,         1.0f, 1.0f, // 3
      w, 0.0f , -t,     1.0f, 0.0f, // 2
    };

    float topPlane[] = {
      w+t, h, -t,    1.0f, 0.0f,
      0.0f, h, d,   0.0f, capRatio, // 1
      0.0f, h, -t, 0.0f, 0.0f,

      w+t, h, -t,    1.0f, 0.0f,
      w-t,h,d,       1.0f, capRatio,
      0.0f,h,d,    0.0f, capRatio, // 1
    };

    float backPlane[] = {
      0.0f, 0.0f , d,  0.0f, 0.0f,
      w-t, h, d,         1.0f, 1.0f,
      0.0f, h, d,      0.0f, 1.0f,

      0.0f, 0.0f , d,  0.0f, 0.0f,
      w-t, 0.0f , d,     1.0f, 0.0f,
      w-t, h, d,         1.0f, 1.0f,
    };

    float closePlane[] = {
      // right ex
      w, 0.0f , -d,  0.0f, 0.0f,
      w+t, h, -d,         capRatio, 1.0f,
      w, h, -d,      0.0f, 1.0f,

      w, 0.0f , -d,  0.0f, 0.0f,
      w+t, 0.0f , -d,     capRatio, 0.0f,
      w+t, h, -d,         capRatio, 1.0f,
    };

    float* wallPlanes[wPlaneCounter] = {
      [wTopPlane] = topPlane,
      [wFrontPlane] = frontPlane,
      [wBackPlane] = backPlane,
      [wClosePlane] = closePlane,
    };
  
    int wallPlanesSize[wPlaneCounter] = {
      [wTopPlane] = sizeof(topPlane),
      [wFrontPlane] = sizeof(frontPlane),
      [wBackPlane] = sizeof(backPlane),
      [wClosePlane] = sizeof(closePlane),
    };

    wallsVPairs[hiddenRWallT].pairs = malloc(sizeof(VPair) * wPlaneCounter);    
    wallsVPairs[hiddenRWallT].planesNum = wPlaneCounter;

    for(int i=0;i<wallsVPairs[hiddenRWallT].planesNum;i++){
      attachNormalsToBuf(wallsVPairs[hiddenRWallT].pairs, i, wallPlanesSize[i], wallPlanes[i]);
    }
  }

  // LWallT
  {
    float frontPlane[] = {
      0.0f, 0.0f , -t,  0.0f, 0.0f,
      0.0f, h, -t,      0.0f, 1.0f,
      w, h, -t,         1.0f, 1.0f,

      0.0f, 0.0f , -t,  0.0f, 0.0f, // 1
      w, h, -t,         1.0f, 1.0f, // 3
      w, 0.0f , -t,     1.0f, 0.0f, // 2
    };

    float topPlane[] = {
      w, h, -t,    1.0f, 0.0f,
      t, h, d,   0.0f, capRatio, // 1
      -t, h, -t, 0.0f, 0.0f,

      w, h, -t,    1.0f, 0.0f,
      w,h,d,       1.0f, capRatio,
      t,h,d,    0.0f, capRatio, // 1
    };

    float backPlane[] = {
      t, 0.0f , d,  0.0f, 0.0f,
      w, h, d,         1.0f, 1.0f,
      t, h, d,      0.0f, 1.0f,

      t, 0.0f , d,  0.0f, 0.0f,
      w, 0.0f , d,     1.0f, 0.0f,
      w, h, d,         1.0f, 1.0f,
    };

    float closePlane[] = {
      // left ex
      -t, 0.0f , -d,  0.0f, 0.0f,
      0.0f, h, -d,         capRatio, 1.0f,
      -t, h, -d,      0.0f, 1.0f,

      -t, 0.0f , -d,  0.0f, 0.0f,
      0.0f, 0.0f , -d,     capRatio, 0.0f,
      0.0f, h, -d,         capRatio, 1.0f,
    };

    float* wallPlanes[wPlaneCounter] = {
      [wTopPlane] = topPlane,
      [wFrontPlane] = frontPlane,
      [wBackPlane] = backPlane,
      [wClosePlane] = closePlane,
    };
  
    int wallPlanesSize[wPlaneCounter] = {
      [wTopPlane] = sizeof(topPlane),
      [wFrontPlane] = sizeof(frontPlane),
      [wBackPlane] = sizeof(backPlane),
      [wClosePlane] = sizeof(closePlane),
    };

    wallsVPairs[hiddenLWallT].pairs = malloc(sizeof(VPair) * wPlaneCounter);    
    wallsVPairs[hiddenLWallT].planesNum = wPlaneCounter;

    for(int i=0;i<wallsVPairs[hiddenLWallT].planesNum;i++){
      attachNormalsToBuf(wallsVPairs[hiddenLWallT].pairs, i, wallPlanesSize[i], wallPlanes[i]);
    }
  }
}


void assembleDoorBlockVBO() {  // wallBlock buf
  float w = 1;
  float h = 2;
  
  float capRatio = 0.12f;

  float capH = h * capRatio;

  float t = (float)1 / 8;
    
  float d = t;

  {
    float frontPlane[] = {
      // cap back
      w, h, -t,         1.0f, capRatio,
      0.0f, h -capH, -t,  0.0f, 0.0f,
      0.0f, h, -t,      0.0f, capRatio,

      w, h, -t,         1.0f, capRatio,
      w, h -capH, -t,     1.0f, 0.0f,
      0.0f, h -capH, -t,  0.0f, 0.0f,
    };

    float topPlane[] = {
      w, h, -t,    1.0f, 0.0f,
      0.0f,h, d,   0.0f, capRatio,
      0.0f, h, -t, 0.0f, 0.0f,

      w, h, -t,    1.0f, 0.0f,
      w,h,d,       1.0f, capRatio,
      0.0f,h,d,    0.0f, capRatio,
    };

    float innerSide[] = {
      // cap bot
      w,  h-capH, -t,    1.0f, 0.0f,
      0.0f, h-capH, d,   0.0f, capRatio,
      0.0f, h-capH, -t, 0.0f, 0.0f,

      w,  h-capH, -t,    1.0f, 0.0f,
      w, h-capH,d,       1.0f, capRatio,
      0.0f, h-capH,d,    0.0f, capRatio,
    };

    float backSide[] = {
      // cap front
      0.0f, h -capH , d,  0.0f, 0.0f,
      w, h, d,            1.0f, capRatio,
      0.0f, h, d,         0.0f, capRatio,

      0.0f, h -capH , d,  0.0f, 0.0f,
      w, h -capH , d,     1.0f, 0.0f,
      w, h, d,            1.0f, capRatio,
    };

    float doorPad = t/12;
      
    float centerDoorPlane[] = {
      // cap front
      w, h -capH, -doorPad,         1.0f, 1.0f,
      0.0f, 0.0f , -doorPad,        0.0f, 0.0f,
      0.0f, h -capH, -doorPad,      0.0f, 1.0f,

      w, h -capH, -doorPad,         1.0f, 1.0f,
      w, 0.0f , -doorPad,           1.0f, 0.0f,
      0.0f, 0.0f , -doorPad,        0.0f, 0.0f,
      
      // cap front
      0.0f, 0.0f , doorPad,        0.0f, 0.0f,
      w, h -capH, doorPad,         1.0f, 1.0f,
      0.0f, h -capH, doorPad,      0.0f, 1.0f,

      0.0f, 0.0f , doorPad,        0.0f, 0.0f,
      w, 0.0f , doorPad,           1.0f, 0.0f,
      w, h -capH, doorPad,         1.0f, 1.0f,
    };
    
    /*
      float leftPlane[] = {
      0.0f, 0.0f, -t,  0.0f, 0.0f,
      0.0f, h, -t,     0.0f, 1.0f,
      0.0f, h , d,     t, 1.0f,

      0.0f, h, d,      t, 1.0f,
      0.0f, 0.0f, -t,  0.0f, 0.0f,
      0.0f, 0.0f , d,  t, 0.0f,
      };

      float rightPlane[] = {
      w, 0.0f, -t,     0.0f, 0.0f,
      w, h, -t,        0.0f, 1.0f,
      w, h , d,        t, 1.0f,

      w, h, d,         t, 1.0f,
      w, 0.0f, -t,     0.0f, 0.0f,
      w, 0.0f , d,     t, 0.0f,
      };
    */

    // remember door plane for selection
    //for(int i=0;i<2;i++){
      glGenBuffers(1, &doorDoorPlane.VBO);
      glGenVertexArrays(1, &doorDoorPlane.VAO);
      
      glBindVertexArray(doorDoorPlane.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, doorDoorPlane.VBO);
      
      doorDoorPlane.VBOsize = 6;

      int newBufSize;
      float* newBuf;

	newBuf = createNormalBuffer(centerDoorPlane, sizeof(centerDoorPlane), &newBufSize);

      int vertNum = newBufSize / sizeof(float) / 8;
      
      glBufferData(GL_ARRAY_BUFFER, newBufSize, newBuf, GL_STATIC_DRAW);
      free(newBuf);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
      glEnableVertexAttribArray(1);

      glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
      glEnableVertexAttribArray(2);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    //}

  
    float* wallPlanes[doorPlaneCounter] = {
      [doorTopPlane] = topPlane,
      [doorFrontPlane] = frontPlane,
      [doorBackPlane] = backSide,
      [doorCenterPlane] = centerDoorPlane,
      [doorInnerTopPlane] = innerSide,
    };
  
    int wallPlanesSize[doorPlaneCounter] = {
      [doorTopPlane] = sizeof(topPlane),
      [doorFrontPlane] = sizeof(frontPlane),
      [doorBackPlane] = sizeof(backSide),
      [doorCenterPlane] = sizeof(centerDoorPlane),
      [doorInnerTopPlane] = sizeof(innerSide),
    };

    wallsVPairs[doorT].pairs = malloc(sizeof(VPair) * doorPlaneCounter);
    wallsVPairs[doorT].planesNum = doorPlaneCounter;
    
    for(int i=0;i<wallsVPairs[doorT].planesNum;i++){
      attachNormalsToBuf(wallsVPairs[doorT].pairs, i, wallPlanesSize[i], wallPlanes[i]);
    }
  }

  
  {
    float doorPad = t/12;
      
    float centerDoorPlane[] = {
      // cap front
      w, h -capH, -doorPad,         1.0f, 1.0f,
      0.0f, 0.0f , -doorPad,        0.0f, 0.0f,
      0.0f, h -capH, -doorPad,      0.0f, 1.0f,

      w, h -capH, -doorPad,         1.0f, 1.0f,
      w, 0.0f , -doorPad,           1.0f, 0.0f,
      0.0f, 0.0f , -doorPad,        0.0f, 0.0f,
      
      // cap front
      0.0f, 0.0f , doorPad,        0.0f, 0.0f,
      w, h -capH, doorPad,         1.0f, 1.0f,
      0.0f, h -capH, doorPad,      0.0f, 1.0f,

      0.0f, 0.0f , doorPad,        0.0f, 0.0f,
      w, 0.0f , doorPad,           1.0f, 0.0f,
      w, h -capH, doorPad,         1.0f, 1.0f,
    };
      
    float* wallPlanes[1] = {
      [doorCenterPlane] = centerDoorPlane,
    };
  
    int wallPlanesSize[1] = {
      [doorCenterPlane] = sizeof(centerDoorPlane),
    };

    wallsVPairs[hiddenDoorT].pairs = malloc(sizeof(VPair) * 1);
    wallsVPairs[hiddenDoorT].planesNum = 1;
    
    for(int i=0;i<wallsVPairs[hiddenDoorT].planesNum;i++){
      attachNormalsToBuf(wallsVPairs[hiddenDoorT].pairs, i, wallPlanesSize[i], wallPlanes[i]);
    }
  }
}

void assembleWindowBlockVBO(){
  float w = 1;
  float h = 2;

  float botRatio = 0.4f;
  float capRatio = 0.12f;

  float capH = h * 0.12f;
  float botH = h * 0.4f;

  float t = (float)1 / 8;
    
  float d = t;

  float padokonikRatio = 0.08f;
  float padokonikD = bBlockH * padokonikRatio;

  float padokonikMainH = bBlockH * 0.03f;

  float padokonikDownThingRatio = 0.02f;
  float padokonikDownThingH = bBlockH * padokonikDownThingRatio;
      
  float v = 1.0f - (capRatio + botRatio);

  float botFrontPlane[] = {
    // bot front
    0.0f, 0.0f , d,  0.0f, 0.0f,
    w, botH, d,         1.0f, botRatio,
    0.0f, botH, d,      0.0f, botRatio,

    0.0f, 0.0f , d,  0.0f, 0.0f,
    w, 0.0f , d,     1.0f, 0.0f,
    w, botH, d,         1.0f, botRatio,
  };

  float capFrontPlane[] = {
    // cap front
    0.0f, h -capH , d,  0.0f, 0.0f,
    w, h, d,            1.0f, capRatio,
    0.0f, h, d,         0.0f, capRatio,

    0.0f, h -capH , d,  0.0f, 0.0f,
    w, h -capH , d,     1.0f, 0.0f,
    w, h, d,            1.0f, capRatio,
  };
      
  float botBackSide[] = {
    // bot back
    w, botH, -t,         1.0f, botRatio,
    0.0f, 0.0f , -t,  0.0f, 0.0f,
    0.0f, botH, -t,      0.0f, botRatio,

    w, botH, -t,         1.0f, botRatio,
    w, 0.0f , -t,     1.0f, 0.0f,
    0.0f, 0.0f , -t,  0.0f, 0.0f,
  };
      
  float capBackSide[] = {
    // cap back
    w, h, -t,         1.0f, capRatio,
    0.0f, h -capH, -t,  0.0f, 0.0f,
    0.0f, h, -t,      0.0f, capRatio,

    w, h, -t,         1.0f, capRatio,
    w, h -capH, -t,     1.0f, 0.0f,
    0.0f, h -capH, -t,  0.0f, 0.0f,
  };
  
  float topSide[] = {
    // cap top
    w, h, -t,    1.0f, 0.0f,
    0.0f,h, d,   0.0f, capRatio,
    0.0f, h, -t, 0.0f, 0.0f,

    w, h, -t,    1.0f, 0.0f,
    w,h,d,       1.0f, capRatio,
    0.0f,h,d,    0.0f, capRatio,
  };

  float capInnerSide[] = {
    // cap bot
    0.0f, h-capH, d,   0.0f, capRatio,
    w,  h-capH, -t,    1.0f, 0.0f,
    0.0f, h-capH, -t,  0.0f, 0.0f,

    0.0f, h-capH,d,    0.0f, capRatio,
    w, h-capH,d,       1.0f, capRatio,
    w,  h-capH, -t,    1.0f, 0.0f,
  };

  float botInnerSide[] = {
    // bot top
    0.0f,botH, d,   0.0f, capRatio,
    w, botH, -t,    1.0f, 0.0f,
    0.0f, botH, -t, 0.0f, 0.0f,
    // with kamen2 1.5f looks also good 

    0.0f,botH,d,    0.0f, capRatio,
    w,botH,d,       1.0f, capRatio,
    w, botH, -t,    1.0f, 0.0f,
  };
  
  float innerLeftPlane[] = {
    // bot top
    0.0f,botH, d,         capRatio*2, 0.0f,
    0.0f, h-capH, -t,     0.0f, 1.0f,
    0.0f, h-capH, d,      capRatio*2, 1.0f,

    0.0f,botH,d,          capRatio*2, 0.0f,
    0.0f,botH,-t,         0.0f, 0.0f,
    0.0f, h-capH, -t,     0.0f, 1.0f,
  };

  float innerRightPlane[] = {
    // bot top
    w,botH, d,             capRatio*2, 0.0f,
    w, h-capH, -t,         0.0f, 1.0f,
    w, h-capH, d,          capRatio*2, 1.0f,

    w,botH,d,           capRatio*2, 0.0f,
    w,botH,-t,          0.0f, 0.0f,
    w, h-capH, -t,      0.0f, 1.0f,
  };

  float winPad = t/12;

  float windowPlaneFront[] = {
    // cap bot
    0.0f, botH, winPad,   1.0f, 0.0f,
    w,  h-capH, winPad,    0.0f, 1.0f,
    0.0f, h-capH, winPad, 1.0f, 1.0f,

    0.0f, botH, winPad,    1.0f, 0.0f,
    w, botH, winPad,       0.0f, 0.0f,
    w,  h-capH, winPad,    0.0f, 1.0f,
  };

  float windowPlaneBack[] = {
    // cap bot
    w,  h-capH, -winPad,    0.0f, 1.0f,
    0.0f, botH, -winPad,   1.0f, 0.0f,
    0.0f, h-capH, -winPad, 1.0f, 1.0f,

    w,  h-capH, -winPad,    0.0f, 1.0f,
    w, botH, -winPad,       0.0f, 0.0f,
    0.0f, botH, -winPad,    1.0f, 0.0f,
  };
      
  float frontWindowPlane[] = {
    // main olane
    w, botH, t,       1.0f, 0.0f,
    0.0f, botH - padokonikMainH, padokonikD+t, 0.0f, padokonikRatio,
    0.0f, botH, t,    0.0f, 0.0f,

    w, botH, t,       1.0f, 0.0f,
    w, botH - padokonikMainH, padokonikD+t, 1.0f, padokonikRatio,
    0.0f, botH - padokonikMainH, padokonikD+t, 0.0f, padokonikRatio,

    // down thing
    0.0f, botH - padokonikMainH - padokonikDownThingH, padokonikD+t, 0.0f, 0.0f,
    w, botH - padokonikMainH, padokonikD+t,                          1.0f, padokonikDownThingRatio,
    0.0f, botH - padokonikMainH, padokonikD+t,                       0.0f, padokonikDownThingRatio,

    0.0f, botH - padokonikMainH - padokonikDownThingH, padokonikD+t, 0.0f, 0.0f,
    w, botH - padokonikMainH - padokonikDownThingH, padokonikD+t,    1.0f, 0.0f,
    w, botH - padokonikMainH, padokonikD+t,                          1.0f, padokonikDownThingRatio,
  };
  
 float* wallPlanes[winPlaneCounter] = {
    [winFrontCapPlane] = capFrontPlane,
    [winFrontBotPlane] = botFrontPlane,
    [winBackCapPlane] = capBackSide,
    [winBackBotPlane] = botBackSide,

    [winInnerTopPlane] = capInnerSide,
    [winInnerBotPlane] = botInnerSide,
    [winInnerRightPlane] = innerRightPlane,
    [winInnerLeftPlane] = innerLeftPlane,
    
    [winTopPlane] = topSide,
    [winFrontPodokonik] = frontWindowPlane,

    [winCenterBackPlane] = windowPlaneBack,
    [winCenterFrontPlane] = windowPlaneFront
  };
  
  int wallPlanesSize[winPlaneCounter] = {
    [winFrontCapPlane] = sizeof(capFrontPlane),
    [winFrontBotPlane] = sizeof(botFrontPlane),
    [winBackCapPlane] = sizeof(capBackSide),
    [winBackBotPlane] = sizeof(botBackSide),

    [winInnerTopPlane] = sizeof(capInnerSide),
    [winInnerBotPlane] = sizeof(botInnerSide),
    [winInnerRightPlane] = sizeof(innerRightPlane),
    [winInnerLeftPlane] = sizeof(innerLeftPlane),

    [winTopPlane] = sizeof(topSide),
    [winFrontPodokonik] = sizeof(frontWindowPlane),

    [winCenterBackPlane] = sizeof(windowPlaneBack),
    [winCenterFrontPlane] = sizeof(windowPlaneFront)
  };

  wallsVPairs[windowT].pairs = malloc(sizeof(VPair) * winPlaneCounter);
  wallsVPairs[windowT].planesNum = winPlaneCounter;
    
  for(int i=0;i<wallsVPairs[windowT].planesNum;i++){
    attachNormalsToBuf(wallsVPairs[windowT].pairs, i, wallPlanesSize[i], wallPlanes[i]);
  }
}

void initGrid(int sx, int sy, int sz){
  int textureOfGround = texture1DIndexByName("Zemlia1");

  if (textureOfGround == -1) {
    printf("Specify texture of ground");
    exit(-1);
  }

  grid = malloc(sizeof(Tile***) * (sy));

  for (int y = 0; y < sy; y++) {
    grid[y] = malloc(sizeof(Tile**) * (sz));

    for (int z = 0; z < sz; z++) {
      grid[y][z] = calloc(gridX, sizeof(Tile*));

      for (int x = 0; x < sx; x++) {
	if (y == 0) {
	  grid[y][z][x] = calloc(1, sizeof(Tile));
	  
	//  grid[y][z][x]->type = texturedTileT;
	  grid[y][z][x]->tx = textureOfGround;

	  vec3 tile = xyz_indexesToCoords(x,y,z);

	  tilesStorageSize++;
	  
	  if(!tilesStorage){
	    tilesStorage = malloc(sizeof(Tile*));
	  }else{
	    tilesStorage = realloc(tilesStorage, sizeof(Tile*) * tilesStorageSize);
	  }

	  geomentyByTxCounter[textureOfGround] += sizeof(float) * 8 * 6;

	  tilesStorage[tilesStorageSize-1] = grid[y][z][x];
	  tilesStorage[tilesStorageSize-1]->id = tilesStorageSize-1;
	  
	  grid[y][z][x]->pos = tile;
	  //printf("%f %f %f \n", argVec3(tilesStorage[tilesStorageSize-1]->pos));
	}
      }
    }
  }
}

void rerenderShadowsForAllLights(){
  GLint curShader = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &curShader);

  glUseProgram(shadersId[dirShadowShader]);
  glEnable(GL_DEPTH_TEST);
    
  glViewport(0,0, SHADOW_WIDTH, SHADOW_HEIGHT);
  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

  batchAllGeometryNoHidden();
  
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
    
    glUseProgram(shadersId[mainShader]);
    sprintf(buf, "lightSpaceMatrix[%d]", i);
    uniformMat4(mainShader, buf, lightSpaceMatrix.m);
  }

  batchAllGeometry();
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

  batchAllGeometryNoHidden();
	    
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
    
  glUseProgram(shadersId[mainShader]);
  sprintf(buf, "lightSpaceMatrix[%d]", lightId);
  uniformMat4(mainShader, buf, lightSpaceMatrix.m);

  batchAllGeometry();
  glUseProgram(curShader);
}

void batchModels(){
  int* modelsBatchCounterByTx = calloc(loadedModelsTxSize, sizeof(int));
  //  loaded
  //int mappedTxToIndexes[loadedModelsTxSize] = {0};
  
  for(int i=0;i<curModelsSize;i++){
    modelsBatchCounterByTx[loadedModels1D[curModels[i].name].tx - loadedModels1D[0].tx] += loadedModels1D[curModels[i].name].size * 8 * sizeof(float);
    //mappedTxToIndexes[loadedModels1D[curModels[i].name].tx]
  }

  for (int i = 0; i < loadedModelsTxSize; i++) {
      modelsBatch[i].size = modelsBatchCounterByTx[i];

      if (modelsBatchCounterByTx[i] != 0) {
          if (!modelsBatch[i].verts) {
              modelsBatch[i].verts = malloc(modelsBatchCounterByTx[i]);
          }
          else {
              modelsBatch[i].verts = realloc(modelsBatch[i].verts, modelsBatchCounterByTx[i]);
          }
      }

      modelsBatch[i].tris = modelsBatchCounterByTx[i] / 8 / sizeof(float);
  }

  memset(modelsBatchCounterByTx, 0, sizeof(int) * loadedModelsTxSize);


  for (int i2 = 0; i2 < curModelsSize; i2++) {
      int txIndex = loadedModels1D[curModels[i2].name].tx - loadedModels1D[0].tx;

    for(int i=0;i<loadedModels1D[curModels[i2].name].size * 8;i+=8){
   //   modelsBatchCounterByTx[txIndex] += loadedModels1D[curModels[i2].name].size * 8 * sizeof(float);

      vec4 vert = { loadedModels1D[curModels[i2].name].entireVert[i], loadedModels1D[curModels[i2].name].entireVert[i+1], loadedModels1D[curModels[i2].name].entireVert[i+2], 1.0f };

      vec4 transf = mulmatvec4(curModels[i2].mat, vert);

      vec4 normal = { loadedModels1D[curModels[i2].name].entireVert[i+5], loadedModels1D[curModels[i2].name].entireVert[i+6], loadedModels1D[curModels[i2].name].entireVert[i+7], 1.0f };

      Matrix inversedModel = IDENTITY_MATRIX;
      inverse(curModels[i2].mat.m, inversedModel.m);

      Matrix trasposedAndInversedModel = IDENTITY_MATRIX;
      mat4transpose(trasposedAndInversedModel.m, inversedModel.m);
		  
      vec4 transfNormal = mulmatvec4(trasposedAndInversedModel, normal);

      modelsBatch[txIndex].verts[modelsBatchCounterByTx[txIndex]+i] = transf.x;   
      modelsBatch[txIndex].verts[modelsBatchCounterByTx[txIndex]+i+1] = transf.y;
      modelsBatch[txIndex].verts[modelsBatchCounterByTx[txIndex]+i+2] = transf.z;

      modelsBatch[txIndex].verts[modelsBatchCounterByTx[txIndex]+i+3] = loadedModels1D[curModels[i2].name].entireVert[i+3];
      modelsBatch[txIndex].verts[modelsBatchCounterByTx[txIndex]+i+4] = loadedModels1D[curModels[i2].name].entireVert[i+4];
		    
      modelsBatch[txIndex].verts[modelsBatchCounterByTx[txIndex]+i+5] = transfNormal.x;
      modelsBatch[txIndex].verts[modelsBatchCounterByTx[txIndex]+i+6] = transfNormal.y;
      modelsBatch[txIndex].verts[modelsBatchCounterByTx[txIndex]+i+7] = transfNormal.z;
    }
    
    modelsBatchCounterByTx[txIndex] += loadedModels1D[curModels[i2].name].size * 8;
    txIndex++;
  }

  for(int i=0;i<loadedModelsTxSize;i++){
    glBindVertexArray(modelsBatch[i].pairs.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, modelsBatch[i].pairs.VBO);
  
    glBufferData(GL_ARRAY_BUFFER, modelsBatch[i].size, modelsBatch[i].verts, GL_STATIC_DRAW);
  
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  free(modelsBatchCounterByTx);
}

void batchGeometry(){
    int a = 200;
free(a); free(a); free(a); 
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


void attachNormalsToBuf(VPair* VPairBuf, int plane, int bufSize, float* buf){
  glGenBuffers(1, &VPairBuf[plane].VBO);
  glGenVertexArrays(1, &VPairBuf[plane].VAO);

  glBindVertexArray(VPairBuf[plane].VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VPairBuf[plane].VBO);

  int oldVertSize = bufSize / sizeof(float) / 5;
  size_t newSize = bufSize + (sizeof(vec3) * oldVertSize);

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



  //  VPairBuf[plane].vBuf = malloc(bufSize);
  //  memcpy(VPairBuf[plane].vBuf, buf, bufSize);

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
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
 
  glBindTexture(GL_TEXTURE_2D, 0);
}; 

// TODO: Func->macro
void uniformVec3(int shader, char* var, vec3 value){
  int uni = glGetUniformLocation(shadersId[shader], var);
  glUniform3f(uni, argVec3(value));
};

void uniformVec2(int shader, char* var, vec2 value){
  int uni = glGetUniformLocation(shadersId[shader], var);
  glUniform2f(uni, argVec2(value));
};

void uniformMat4(int shader, char* var, float* mat){
  int uni = glGetUniformLocation(shadersId[shader], var);
  glUniformMatrix4fv(uni, 1, GL_FALSE, mat);
};

void uniformFloat(int shader, char* var, float value) {
  int uni = glGetUniformLocation(shadersId[shader], var);
  glUniform1f(uni, value);
};

void uniformInt(int shader, char* var, int value) {
  int uni = glGetUniformLocation(shadersId[shader], var);
  glUniform1i(uni,value);
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

  float* blocksBuffers[tileBlocksCounter] = {
    [roofBlockT]=roofBlock,
    [stepsBlockT]=texturedTileVerts,
    [angledRoofT]=angledRoof
  };

  int blocksBuffersSize[tileBlocksCounter] = {
    [roofBlockT]=sizeof(roofBlock),
    [stepsBlockT]=sizeof(texturedTileVerts),
    [angledRoofT]=sizeof(angledRoof)
  };

  
  for(int i=0;i<tileBlocksCounter;i++){
    blocksVPairs[i].planesNum = 1;
    blocksVPairs[i].pairs = calloc(blocksVPairs[i].planesNum, sizeof(VPair));

    for(int i2=0;i2<blocksVPairs[i].planesNum;i2++){
      attachNormalsToBuf(blocksVPairs[i].pairs, i2, blocksBuffersSize[i], blocksBuffers[i]);
    }
  }
}


// make render for shadow and for normal
void renderScene(GLuint curShader){
  Matrix out2 = IDENTITY_MATRIX;
   
  uniformMat4(curShader, "model", out2.m);

  for (int i = 0; i < loadedTexturesCounter; i++) {
    //      glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, loadedTextures1D[i].tx);

    glBindBuffer(GL_ARRAY_BUFFER, finalGeom[i].VBO);
    glBindVertexArray(finalGeom[i].VAO);

    glDrawArrays(GL_TRIANGLES, 0, finalGeom[i].tris);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  for (int i = 0; i < loadedModelsTxSize; i++) {
    glBindTexture(GL_TEXTURE_2D, loadedModelsTx[i]);

    glBindBuffer(GL_ARRAY_BUFFER, modelsBatch[i].pairs.VBO);
    glBindVertexArray(modelsBatch[i].pairs.VAO);

    glDrawArrays(GL_TRIANGLES, 0, modelsBatch[i].tris);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  /*
  for (int i = 0; i < curModelsSize; i++) {
    //    if(&curModels[i] == mouse.selectedThing && curShader == mainShader){
      //      continue;
      //    }
    
    int name = curModels[i].name;

    glBindTexture(GL_TEXTURE_2D, loadedModels1D[name].tx);

    glBindVertexArray(loadedModels1D[name].VAO);
    glBindBuffer(GL_ARRAY_BUFFER, loadedModels1D[name].VBO);

    uniformMat4(curShader, "model", curModels[i].mat.m);

    glDrawArrays(GL_TRIANGLES, 0, loadedModels1D[name].size);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
  }
  */
}

void checkMouseVSEntities(){  
  if (mouse.selectedType == mouseWallT || mouse.selectedType == mouseTileT) {
    free(mouse.selectedThing);
  }

  mouse.selectedThing = NULL;
  mouse.selectedType = 0;
    
  float minDistToCamera = 1000.0f;

  WallMouseData* intersWallData = malloc(sizeof(WallMouseData));
  TileMouseData* intersTileData = malloc(sizeof(TileMouseData));
	
  for(int i=0;i<batchedGeometryIndexesSize;i++){
    vec3i ind = batchedGeometryIndexes[i].indx;
    vec3 tile = vec3_indexesToCoords(ind);
    Tile* bBlock = grid[ind.y][ind.z][ind.x];

    // block
    if (bBlock->block != NULL){
      float intersectionDistance;
      bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, bBlock->block->lb, bBlock->block->rt, &mouse.gizmoPosOfInter, &intersectionDistance);

      if (isIntersect && minDistToCamera > intersectionDistance) {
	mouse.selectedThing = bBlock->block;
	mouse.selectedType = mouseBlockT;

	mouse.interDist = intersectionDistance;
	minDistToCamera = intersectionDistance;
      }
    }

    // tiles
    if(ind.y == curFloor){
      const vec3 rt = { tile.x + bBlockW, tile.y, tile.z + bBlockD };
      const vec3 lb = { tile.x, tile.y, tile.z };

      float intersectionDistance;
      vec3 intersection;

      bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, lb, rt, &intersection, &intersectionDistance);

      if (isIntersect && minDistToCamera > intersectionDistance) {
	//intersTileData->tile = bBlock;

	//intersTileData->grid = (vec2i){ ind.x,ind.z };
	intersTileData->intersection = intersection;

	mouse.selectedType = mouseTileT;
	mouse.selectedThing = intersTileData;

	minDistToCamera = intersectionDistance;
      }
    }

    if (ind.y >= curFloor) {
      // walls
      {
	for(int i2=0;i2<batchedGeometryIndexes[i].wallsSize;i2++){
	  int wallIndex = batchedGeometryIndexes[i].wallsIndexes[i2];
	  
	  WallType type = bBlock->wall[wallIndex]->type;
	  
	  float intersectionDistance;

	  for (int i3 = 0; i3 < wallsVPairs[type].planesNum; i3++) {
	    bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, bBlock->wall[wallIndex]->planes[i3].lb, bBlock->wall[wallIndex]->planes[i3].rt, NULL, &intersectionDistance);

	    if (isIntersect && minDistToCamera > intersectionDistance) {
	      intersWallData->side = wallIndex;

	      int tx = bBlock->wall[wallIndex]->planes[i3].txIndex;

	      intersWallData->txIndex = tx;
	    //  intersWallData->tile = bBlock;

	   //   intersWallData->type = type;
	      intersWallData->plane = i3;

	      mouse.selectedType = mouseWallT;
	      mouse.selectedThing = intersWallData;
	      
	      minDistToCamera = intersectionDistance;
	    }
	  }
	}
      }
    }
  }

  // lights
  for (int i2 = 0; i2 < lightsTypeCounter; i2++) {
    for (int i = 0; i < lightStorageSizeByType[i2]; i++) {
      float intersectionDistance;

      bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, lightStorage[i2][i].lb, lightStorage[i2][i].rt, NULL, &intersectionDistance);

      if (isIntersect && minDistToCamera > intersectionDistance) {
	mouse.selectedThing = &lightStorage[i2][i];
	mouse.selectedType = mouseLightT;
	
	minDistToCamera = intersectionDistance;
      }
    }
  }

  // models
  for (int i = 0; i < curModelsSize; i++) {
    float intersectionDistance = 0.0f;

    bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, curModels[i].lb, curModels[i].rt, NULL, &intersectionDistance);

    int name = curModels[i].name;

    if (isIntersect && minDistToCamera > intersectionDistance) {
      mouse.selectedThing = &curModels[i];
      mouse.selectedType = mouseModelT;

      minDistToCamera = intersectionDistance;
    }
  }

  // net tile
  if(curFloor != 0 && curInstance == editorInstance){
    for(int i=0;i<netTileSize;i+=2){
      const vec3 rt = { netTileAABB[i+1].x, curFloor, netTileAABB[i+1].z };
      const vec3 lb = { netTileAABB[i].x, curFloor, netTileAABB[i].z };

      float intersectionDistance;
      vec3 intersection;

      bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, lb, rt, &intersection, &intersectionDistance);

      if (isIntersect && minDistToCamera > intersectionDistance) {
          vec3i gridInd = xyz_coordsToIndexes(netTileAABB[i].x, curFloor, netTileAABB[i].z);
	//intersTileData->tile = grid[curFloor][gridInd.z][gridInd.x];

	//intersTileData->grid = (vec2i){ gridInd.x, gridInd.z };
	intersTileData->intersection = intersection;

	mouse.selectedType = mouseTileT;
	mouse.selectedThing = intersTileData;

	minDistToCamera = intersectionDistance;
      }
      }
  }
  
  if(mouse.selectedType != mouseTileT){
    free(intersTileData);
  }

  if(mouse.selectedType != mouseWallT){
    free(intersWallData);
  }
}


bool isAlreadyNavPoint(vec3 point){
  for(int i=0;i<navPointsSize;i++){
    if(point.x == navPoints[i].pos.x && point.y == navPoints[i].pos.y && point.z == navPoints[i].pos.z){
      return true;
    }
  }

  return false;
}

float texturedTileVerts[] = {
  bBlockW, 0.0f, bBlockD , 0.0f, 1.0f, -0.000000, 1.000000, -0.000000,
  0.0f, 0.0f, bBlockD , 1.0f, 1.0f, -0.000000, 1.000000, -0.000000, 
  bBlockW, 0.0f, 0.0f , 0.0f, 0.0f, -0.000000, 1.000000, -0.000000,
      
  0.0f, 0.0f, bBlockD , 1.0f, 1.0f, 0.000000, 1.000000, 0.000000,
  bBlockW, 0.0f, 0.0f , 0.0f, 0.0f, 0.000000, 1.000000, 0.000000,
  0.0f, 0.0f, 0.0f , 1.0f, 0.0f,  0.000000, 1.000000, 0.000000,
};


void batchAllGeometryNoHidden(){
  int vertexSize = 8;
  
  Geom* preGeom = malloc(sizeof(Geom) * loadedTexturesCounter);
  int* txLastIndex = calloc(loadedTexturesCounter, sizeof(int));
  
  for(int i=0;i<loadedTexturesCounter;i++){
    preGeom[i].buf = calloc(geomentyByTxCounter[i],1);
    preGeom[i].size = geomentyByTxCounter[i];
    finalGeom[i].tris = geomentyByTxCounter[i] / (float)vertexSize / sizeof(float);
  }
  
  // planes
  for(int i=0;i<picturesStorageSize;i++){
    int txIndex = picturesStorage[i].txIndex;

    for(int i2=0;i2< 6*vertexSize;i2 += vertexSize){
      vec4 vert = { planePairs.vBuf[i2], planePairs.vBuf[i2+1], planePairs.vBuf[i2+2], 1.0f };

      vec4 transf = mulmatvec4(picturesStorage[i].mat, vert);

      vec4 normal = { planePairs.vBuf[i2+5], planePairs.vBuf[i2+6], planePairs.vBuf[i2+7], 1.0f };

      Matrix inversedWallModel = IDENTITY_MATRIX;
      inverse(picturesStorage[i].mat.m, inversedWallModel.m);

      Matrix trasposedAndInversedWallModel = IDENTITY_MATRIX;
      mat4transpose(trasposedAndInversedWallModel.m, inversedWallModel.m);
		  
      vec4 transfNormal = mulmatvec4(trasposedAndInversedWallModel, normal);

      preGeom[txIndex].buf[txLastIndex[txIndex] + i2] = transf.x;
      preGeom[txIndex].buf[txLastIndex[txIndex] + i2 + 1] = transf.y;
      preGeom[txIndex].buf[txLastIndex[txIndex] + i2 + 2] = transf.z;

      preGeom[txIndex].buf[txLastIndex[txIndex] + i2 + 3] = planePairs.vBuf[i2 + 3];
      preGeom[txIndex].buf[txLastIndex[txIndex] + i2 + 4] = planePairs.vBuf[i2 + 4];

      preGeom[txIndex].buf[txLastIndex[txIndex] + i2 + 5] = transfNormal.x;
      preGeom[txIndex].buf[txLastIndex[txIndex] + i2 + 6] = transfNormal.y;
      preGeom[txIndex].buf[txLastIndex[txIndex]+ i2 + 7] = transfNormal.z;
    }

    txLastIndex[txIndex] += 6 * 8;
  }

  // tiles
  for(int i=0;i<tilesStorageSize;i++){
    int txIndex = tilesStorage[i]->tx;

    if (txIndex == -1){
      continue;
    }


    for(int i2=0;i2< 6*vertexSize;i2 += vertexSize){
      preGeom[txIndex].buf[txLastIndex[txIndex]+i2] = texturedTileVerts[i2] + tilesStorage[i]->pos.x; 
      preGeom[txIndex].buf[txLastIndex[txIndex]+i2+1] = texturedTileVerts[i2+1] + tilesStorage[i]->pos.y;
      preGeom[txIndex].buf[txLastIndex[txIndex]+i2+2] = texturedTileVerts[i2+2] + tilesStorage[i]->pos.z; 

      preGeom[txIndex].buf[txLastIndex[txIndex]+i2+3] = texturedTileVerts[i2+3];
      preGeom[txIndex].buf[txLastIndex[txIndex]+i2+4] = texturedTileVerts[i2+4];

      preGeom[txIndex].buf[txLastIndex[txIndex]+i2+5] = texturedTileVerts[i2+5];
      preGeom[txIndex].buf[txLastIndex[txIndex]+i2+6] = texturedTileVerts[i2+6];
      preGeom[txIndex].buf[txLastIndex[txIndex]+i2+7] = texturedTileVerts[i2+7];
    }

    txLastIndex[txIndex] += sizeof(texturedTileVerts) / sizeof(float);
  }

  // blocks
  for(int i=0;i<blocksStorageSize;i++){
    TileBlocksTypes type = blocksStorage[i]->type;
    int txIndex = blocksStorage[i]->txIndex;

    for(int i2=0;i2<blocksVPairs[type].planesNum;i2++){
      for(int i3=0;i3<blocksVPairs[type].pairs[i2].vertexNum * vertexSize;i3+=vertexSize){
	vec4 vert = { blocksVPairs[type].pairs[i2].vBuf[i3], blocksVPairs[type].pairs[i2].vBuf[i3+1], blocksVPairs[type].pairs[i2].vBuf[i3+2], 1.0f };

	vec4 transf = mulmatvec4(blocksStorage[i]->mat, vert);

	vec4 normal = { blocksVPairs[type].pairs[i2].vBuf[i3+5], blocksVPairs[type].pairs[i2].vBuf[i3+6], blocksVPairs[type].pairs[i2].vBuf[i3+7], 1.0f };

	Matrix inversedWallModel = IDENTITY_MATRIX;
	inverse(blocksStorage[i]->mat.m, inversedWallModel.m);

	Matrix trasposedAndInversedWallModel = IDENTITY_MATRIX;
	mat4transpose(trasposedAndInversedWallModel.m, inversedWallModel.m);
		  
	vec4 transfNormal = mulmatvec4(trasposedAndInversedWallModel, normal);

	preGeom[txIndex].buf[txLastIndex[txIndex] + i3] = transf.x;
	preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 1] = transf.y;
	preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 2] = transf.z;

	preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 3] = blocksVPairs[type].pairs[i2].vBuf[i3 + 3];
	preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 4] = blocksVPairs[type].pairs[i2].vBuf[i3 + 4];

	preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 5] = transfNormal.x;
	preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 6] = transfNormal.y;
	preGeom[txIndex].buf[txLastIndex[txIndex]+ i3 +7] = transfNormal.z;
      }

      txLastIndex[txIndex] += blocksVPairs[type].pairs[i2].vertexNum * vertexSize;
    }
  }

  // walls
  for(int i=0;i<wallsStorageSize;i++){
    WallType type = wallsStorage[i]->prevType;

    for (int i2 = 0; i2 < wallsVPairs[type].planesNum; i2++) {
      if (!wallsStorage[i]->planes[i2].hide) {
	int txIndex = wallsStorage[i]->planes[i2].txIndex;

	for (int i3 = 0; i3 < wallsVPairs[type].pairs[i2].vertexNum * vertexSize; i3 += vertexSize) {
	  vec4 vert = { wallsVPairs[type].pairs[i2].vBuf[i3], wallsVPairs[type].pairs[i2].vBuf[i3 + 1], wallsVPairs[type].pairs[i2].vBuf[i3 + 2], 1.0f };

	  vec4 transf = mulmatvec4(wallsStorage[i]->mat, vert);

	  vec4 normal = { wallsVPairs[type].pairs[i2].vBuf[i3 + 5], wallsVPairs[type].pairs[i2].vBuf[i3 + 6], wallsVPairs[type].pairs[i2].vBuf[i3 + 7], 1.0f };

	  Matrix inversedWallModel = IDENTITY_MATRIX;
	  inverse(wallsStorage[i]->mat.m, inversedWallModel.m); 

	  Matrix trasposedAndInversedWallModel = IDENTITY_MATRIX;
	  mat4transpose(trasposedAndInversedWallModel.m, inversedWallModel.m);

	  vec4 transfNormal = mulmatvec4(trasposedAndInversedWallModel, normal);

	  preGeom[txIndex].buf[txLastIndex[txIndex] + i3] = transf.x;
	  preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 1] = transf.y;
	  preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 2] = transf.z;

	  preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 3] = wallsVPairs[type].pairs[i2].vBuf[i3 + 3];
	  preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 4] = wallsVPairs[type].pairs[i2].vBuf[i3 + 4];

	  preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 5] = transfNormal.x;
	  preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 6] = transfNormal.y;
	  preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 7] = transfNormal.z;
	}

	txLastIndex[txIndex] += wallsVPairs[type].pairs[i2].vertexNum * vertexSize;
    }
    }
  }
  
  for(int i=0;i<loadedTexturesCounter;i++){ 
    glBindVertexArray(finalGeom[i].VAO);
    glBindBuffer(GL_ARRAY_BUFFER, finalGeom[i].VBO);

    glBufferData(GL_ARRAY_BUFFER, preGeom[i].size, preGeom[i].buf, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);  
    glBindVertexArray(0);

    free(preGeom[i].buf);
  }

  free(preGeom);
  free(txLastIndex);
}


void batchAllGeometry(){
  int vertexSize = 8;
  
  Geom* preGeom = malloc(sizeof(Geom) * loadedTexturesCounter);
  int* txLastIndex = calloc(loadedTexturesCounter, sizeof(int));
  
  for(int i=0;i<loadedTexturesCounter;i++){
    preGeom[i].buf = calloc(geomentyByTxCounter[i],1);
    preGeom[i].size = geomentyByTxCounter[i];
    finalGeom[i].tris = geomentyByTxCounter[i] / (float)vertexSize / sizeof(float);
  }
  
  // planes
  for(int i=0;i<picturesStorageSize;i++){
    int txIndex = picturesStorage[i].txIndex;

    for(int i2=0;i2< 6*vertexSize;i2 += vertexSize){
      vec4 vert = { planePairs.vBuf[i2], planePairs.vBuf[i2+1], planePairs.vBuf[i2+2], 1.0f };

      vec4 transf = mulmatvec4(picturesStorage[i].mat, vert);

      vec4 normal = { planePairs.vBuf[i2+5], planePairs.vBuf[i2+6], planePairs.vBuf[i2+7], 1.0f };

      Matrix inversedWallModel = IDENTITY_MATRIX;
      inverse(picturesStorage[i].mat.m, inversedWallModel.m);

      Matrix trasposedAndInversedWallModel = IDENTITY_MATRIX;
      mat4transpose(trasposedAndInversedWallModel.m, inversedWallModel.m);
		  
      vec4 transfNormal = mulmatvec4(trasposedAndInversedWallModel, normal);

      preGeom[txIndex].buf[txLastIndex[txIndex] + i2] = transf.x;
      preGeom[txIndex].buf[txLastIndex[txIndex] + i2 + 1] = transf.y;
      preGeom[txIndex].buf[txLastIndex[txIndex] + i2 + 2] = transf.z;

      preGeom[txIndex].buf[txLastIndex[txIndex] + i2 + 3] = planePairs.vBuf[i2 + 3];
      preGeom[txIndex].buf[txLastIndex[txIndex] + i2 + 4] = planePairs.vBuf[i2 + 4];

      preGeom[txIndex].buf[txLastIndex[txIndex] + i2 + 5] = transfNormal.x;
      preGeom[txIndex].buf[txLastIndex[txIndex] + i2 + 6] = transfNormal.y;
      preGeom[txIndex].buf[txLastIndex[txIndex]+ i2 + 7] = transfNormal.z;
    }

    txLastIndex[txIndex] += 6 * 8;
  }

  // tiles
  for(int i=0;i<tilesStorageSize;i++){
    int txIndex = tilesStorage[i]->tx;

    if (txIndex == -1){
      continue;
    } 


    for(int i2=0;i2< 6*vertexSize;i2 += vertexSize){
      preGeom[txIndex].buf[txLastIndex[txIndex]+i2] = texturedTileVerts[i2] + tilesStorage[i]->pos.x; 
      preGeom[txIndex].buf[txLastIndex[txIndex]+i2+1] = texturedTileVerts[i2+1] + tilesStorage[i]->pos.y;
      preGeom[txIndex].buf[txLastIndex[txIndex]+i2+2] = texturedTileVerts[i2+2] + tilesStorage[i]->pos.z; 

      preGeom[txIndex].buf[txLastIndex[txIndex]+i2+3] = texturedTileVerts[i2+3];
      preGeom[txIndex].buf[txLastIndex[txIndex]+i2+4] = texturedTileVerts[i2+4];

      preGeom[txIndex].buf[txLastIndex[txIndex]+i2+5] = texturedTileVerts[i2+5];
      preGeom[txIndex].buf[txLastIndex[txIndex]+i2+6] = texturedTileVerts[i2+6];
      preGeom[txIndex].buf[txLastIndex[txIndex]+i2+7] = texturedTileVerts[i2+7];
    }

    txLastIndex[txIndex] += sizeof(texturedTileVerts) / sizeof(float);
  }

  // blocks
  for(int i=0;i<blocksStorageSize;i++){
    TileBlocksTypes type = blocksStorage[i]->type;
    int txIndex = blocksStorage[i]->txIndex;

    for(int i2=0;i2<blocksVPairs[type].planesNum;i2++){
      for(int i3=0;i3<blocksVPairs[type].pairs[i2].vertexNum * vertexSize;i3+=vertexSize){
	vec4 vert = { blocksVPairs[type].pairs[i2].vBuf[i3], blocksVPairs[type].pairs[i2].vBuf[i3+1], blocksVPairs[type].pairs[i2].vBuf[i3+2], 1.0f };

	vec4 transf = mulmatvec4(blocksStorage[i]->mat, vert);

	vec4 normal = { blocksVPairs[type].pairs[i2].vBuf[i3+5], blocksVPairs[type].pairs[i2].vBuf[i3+6], blocksVPairs[type].pairs[i2].vBuf[i3+7], 1.0f };

	Matrix inversedWallModel = IDENTITY_MATRIX;
	inverse(blocksStorage[i]->mat.m, inversedWallModel.m);

	Matrix trasposedAndInversedWallModel = IDENTITY_MATRIX;
	mat4transpose(trasposedAndInversedWallModel.m, inversedWallModel.m);
		  
	vec4 transfNormal = mulmatvec4(trasposedAndInversedWallModel, normal);

	preGeom[txIndex].buf[txLastIndex[txIndex] + i3] = transf.x;
	preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 1] = transf.y;
	preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 2] = transf.z;

	preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 3] = blocksVPairs[type].pairs[i2].vBuf[i3 + 3];
	preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 4] = blocksVPairs[type].pairs[i2].vBuf[i3 + 4];

	preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 5] = transfNormal.x;
	preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 6] = transfNormal.y;
	preGeom[txIndex].buf[txLastIndex[txIndex]+ i3 +7] = transfNormal.z;
      }

      txLastIndex[txIndex] += blocksVPairs[type].pairs[i2].vertexNum * vertexSize;
    }
  }

  // walls
    for(int i=0;i<wallsStorageSize;i++){
      WallType type = wallsStorage[i]->type;

      for (int i2 = 0; i2 < wallsVPairs[type].planesNum; i2++) {
	if (!wallsStorage[i]->planes[i2].hide) {
	  int txIndex = wallsStorage[i]->planes[i2].txIndex;

	  printf("batch %s \n", wallTypeStr[wallsStorage[i]->type]);

	  if(wallsStorage[i]->type == hiddenWallT){
	    if(wallsStorage[i]->prevType == windowT){
	      if(i2 == whBackPlane){
		txIndex = wallsStorage[i]->planes[winFrontBotPlane].txIndex;
	      }else if(i2 == whFrontPlane){
		txIndex = wallsStorage[i]->planes[winBackBotPlane].txIndex;
	      }else if(i2 == whTopPlane){
		txIndex = wallsStorage[i]->planes[winTopPlane].txIndex;
	      }
	    }else if(wallsStorage[i]->prevType == normWallT || wallsStorage[i]->prevType == LRWallT){
	      if(i2 == whBackPlane){
		txIndex = wallsStorage[i]->planes[wBackPlane].txIndex;
	      }else if(i2 == whFrontPlane){
		txIndex = wallsStorage[i]->planes[wFrontPlane].txIndex;
	      }else if(i2 == whTopPlane){
		txIndex = wallsStorage[i]->planes[wTopPlane].txIndex;
	      }
	    }
	  }

	  for (int i3 = 0; i3 < wallsVPairs[type].pairs[i2].vertexNum * vertexSize; i3 += vertexSize) {
	    vec4 vert = { wallsVPairs[type].pairs[i2].vBuf[i3], wallsVPairs[type].pairs[i2].vBuf[i3 + 1], wallsVPairs[type].pairs[i2].vBuf[i3 + 2], 1.0f };

	    vec4 transf = mulmatvec4(wallsStorage[i]->mat, vert);

	    vec4 normal = { wallsVPairs[type].pairs[i2].vBuf[i3 + 5], wallsVPairs[type].pairs[i2].vBuf[i3 + 6], wallsVPairs[type].pairs[i2].vBuf[i3 + 7], 1.0f };

	    Matrix inversedWallModel = IDENTITY_MATRIX;
	    inverse(wallsStorage[i]->mat.m, inversedWallModel.m); 

	    Matrix trasposedAndInversedWallModel = IDENTITY_MATRIX;
	    mat4transpose(trasposedAndInversedWallModel.m, inversedWallModel.m);

	    vec4 transfNormal = mulmatvec4(trasposedAndInversedWallModel, normal);

	    preGeom[txIndex].buf[txLastIndex[txIndex] + i3] = transf.x;
	    preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 1] = transf.y;
	    preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 2] = transf.z;

	    preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 3] = wallsVPairs[type].pairs[i2].vBuf[i3 + 3];
	    preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 4] = wallsVPairs[type].pairs[i2].vBuf[i3 + 4];

	    preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 5] = transfNormal.x;
	    preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 6] = transfNormal.y;
	    preGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 7] = transfNormal.z;
	  }

	  txLastIndex[txIndex] += wallsVPairs[type].pairs[i2].vertexNum * vertexSize;
	}
      }
    }

    for(int i=0;i<loadedTexturesCounter;i++){ 
      glBindVertexArray(finalGeom[i].VAO);
      glBindBuffer(GL_ARRAY_BUFFER, finalGeom[i].VBO);

      //      printf("alloced size - %d  used size - %d \n", preGeom[i].size, txLastIndex[i] * sizeof(float));
      glBufferData(GL_ARRAY_BUFFER, preGeom[i].size, preGeom[i].buf, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
      glEnableVertexAttribArray(1);

      glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
      glEnableVertexAttribArray(2);

      glBindBuffer(GL_ARRAY_BUFFER, 0);  
      glBindVertexArray(0);

      free(preGeom[i].buf);
    }
  
    free(preGeom);
    free(txLastIndex);
}

void assembleNavigation(){
  if(navPoints){
    free(navPoints);
    navPoints = NULL;
  }

  navPointsSize = 0;
  
  for(int i=0;i<tilesStorageSize;i++){
    if(!tilesStorage[i]->block && tilesStorage[i]->tx != -1){
      int x = tilesStorage[i]->pos.x; int y = (int)tilesStorage[i]->pos.y; int z = (int)tilesStorage[i]->pos.z;

      vec3 tile = tilesStorage[i]->pos;//xyz_indexesToCoords((int)tilesStorage[i]->pos.x, (int)tilesStorage[i]->pos.y, (int)tilesStorage[i]->pos.z);

      bool farRightCor = tilesStorage[i]->wall[left]&& tilesStorage[i]->wall[top];
      bool nearRightCor = (x+1 < gridX && grid[y][z][x + 1] && grid[y][z][x+1]->wall[left]) && tilesStorage[i]->wall[top];

      bool farLeftCor = tilesStorage[i]->wall[left]&& (z+1 < gridZ && grid[y][z + 1][x] && grid[y][z+1][x]->wall[top]);
      bool nearLeftCor = (x+1 < gridX && grid[y][z][x + 1] && grid[y][z][x+1]->wall[left]) && (z+1 < gridZ && grid[y][z + 1][x] && grid[y][z+1][x]->wall[top]);
	    
      if ((farRightCor || nearRightCor || farLeftCor || nearLeftCor)) {
	navPointsSize++;
      }
	    
      if(tilesStorage[i]->wall[left] && tilesStorage[i]->wall[left]->type == doorT){
	navPointsSize++;
      }

      if(tilesStorage[i]->wall[top] && tilesStorage[i]->wall[top]->type == doorT){
	navPointsSize++;
      }
    }
  }


  printf("pre navPointsSize %d \n", navPointsSize);
  navPoints = calloc(navPointsSize, sizeof(NavCornerPoint));
  navPointsSize = 0;

  for(int i=0;i<tilesStorageSize;i++){
    if(!tilesStorage[i]->block && tilesStorage[i]->tx != -1) {
        int x = tilesStorage[i]->pos.x; int y = (int)tilesStorage[i]->pos.y; int z = (int)tilesStorage[i]->pos.z;
      
        vec3 tile = tilesStorage[i]->pos;//xyz_indexesToCoords((int)tilesStorage[i]->pos.x, (int)tilesStorage[i]->pos.y, (int)tilesStorage[i]->pos.z);

      bool farRightCor = tilesStorage[i]->wall[left]&& tilesStorage[i]->wall[top];
      bool nearRightCor = (x+1 < gridX && grid[y][z][x + 1] && grid[y][z][x+1]->wall[left]) && tilesStorage[i]->wall[top];

      bool farLeftCor = tilesStorage[i]->wall[left]&& (z+1 < gridZ && grid[y][z + 1][x] && grid[y][z+1][x]->wall[top]);
      bool nearLeftCor = (x+1 < gridX && grid[y][z][x + 1] && grid[y][z][x+1]->wall[left]) && (z+1 < gridZ && grid[y][z + 1][x] && grid[y][z+1][x]->wall[top]);
	    
      if ((farRightCor || nearRightCor || farLeftCor || nearLeftCor)) {
	if(farRightCor){
	  navPoints[navPointsSize].type = farRightCorT;
	  navPoints[navPointsSize].pos = (vec3){tile.x + 0.25f ,tile.y,tile.z + 0.25f};
	}else if(nearRightCor){
	  navPoints[navPointsSize].type = nearRightCorT;
	  navPoints[navPointsSize].pos = (vec3){tile.x + 0.75f ,tile.y,tile.z + 0.25f};
	}else if(farLeftCor){
	  navPoints[navPointsSize].type = farLeftCorT;
	  navPoints[navPointsSize].pos = (vec3){tile.x + 0.25f ,tile.y,tile.z + 0.75f};
	}else if(nearLeftCor){
	  navPoints[navPointsSize].type = nearLeftCorT;
	  navPoints[navPointsSize].pos = (vec3){tile.x + 0.75f ,tile.y,tile.z + 0.75f};
	}

	navPointsSize++;
      }
	    
      // doors detection
      if(tilesStorage[i]->wall[left] && tilesStorage[i]->wall[left]->type == doorT){
	navPoints[navPointsSize].type = doorFrameT;
	navPoints[navPointsSize].pos = (vec3){tile.x ,tile.y,tile.z + 0.5f};
	
	navPointsSize++;
      }

      if(tilesStorage[i]->wall[top] && tilesStorage[i]->wall[top]->type == doorT){
	navPoints[navPointsSize].type = doorFrameT;
	navPoints[navPointsSize].pos = (vec3){tile.x + 0.5f ,tile.y,tile.z};
	
	navPointsSize++;
      }
    }
  }

  printf("post navPointsSize %d \n", navPointsSize);

  vec3** triColl = NULL;
  int triCollSize = 0;

  //  /*
   int extentedNavPointsSize = navPointsSize;
  
   for (int i = 0; i < navPointsSize; i++) {
       //   if(navPoints[i].type == nearLeftCorT){

       if (navPoints[i].type == doorFrameT) {
           continue;

       }

       float exX = navPoints[i].pos.x;
       float exZ = navPoints[i].pos.z;

       int x = navPoints[i].pos.x;
       int z = navPoints[i].pos.z;
       int y = navPoints[i].pos.y;

       if (navPoints[i].type == farRightCorT) {
           exX++;
       }

       if (navPoints[i].type == farLeftCorT) {
           exX++;
       }

       if (navPoints[i].type != nearLeftCorT && navPoints[i].type != nearRightCorT) {
           while (exX < gridX && grid[y][z][(int)exX] && !grid[y][z][(int)exX]->wall[left]) {
               exX++;
           }

           vec3 n1 = { exX - 1, navPoints[i].pos.y, navPoints[i].pos.z };

           if (navPoints[i].type == farRightCorT) {
               n1.x += 0.5f;
           }

           if (navPoints[i].type == farLeftCorT) {
               n1.x += 0.5f;
           }

           if (n1.x != navPoints[i].pos.x && !isAlreadyNavPoint(n1)) {
               extentedNavPointsSize++;
               navPoints = realloc(navPoints, sizeof(navPoints[0]) * extentedNavPointsSize);
               navPoints[extentedNavPointsSize - 1].pos = n1;
               navPoints[extentedNavPointsSize - 1].used = false;

               if (navPoints[i].type == farRightCorT) {
                   navPoints[extentedNavPointsSize - 1].type = nearRightCorT;
               }

               if (navPoints[i].type == farLeftCorT) {
                   navPoints[extentedNavPointsSize - 1].type = nearLeftCorT;
               }

           }
       }

       exX = navPoints[i].pos.x;

       if (navPoints[i].type != farRightCorT && navPoints[i].type != farLeftCorT) {
           while (exX >= 0 && grid[y][z][(int)exX] && !grid[y][z][(int)exX]->wall[left]) {
               exX--;
           }

           //      vec3 n2 = {exX - 0.75f, navPoints[i].pos.y, navPoints[i].pos.z};
           vec3 n2 = { exX, navPoints[i].pos.y, navPoints[i].pos.z };

           if (navPoints[i].type == nearLeftCorT) {
               n2.x -= 0.5f;
           }

           if (navPoints[i].type == nearRightCorT) {
               n2.x -= 0.5f;
           }

           if (n2.x != navPoints[i].pos.x && !isAlreadyNavPoint(n2)) {
               extentedNavPointsSize++;
               navPoints = realloc(navPoints, sizeof(navPoints[0]) * extentedNavPointsSize);
               navPoints[extentedNavPointsSize - 1].pos = n2;
               navPoints[extentedNavPointsSize - 1].used = false;

               if (navPoints[i].type == nearRightCorT) {
                   navPoints[extentedNavPointsSize - 1].type = farRightCorT;
               }

               if (navPoints[i].type == nearLeftCorT) {
                   navPoints[extentedNavPointsSize - 1].type = farLeftCorT;
               }
           }
       }

       if (navPoints[i].type == farRightCorT) {
           exZ++;
       }

       if (navPoints[i].type == nearRightCorT) {
           exZ++;
       }

       if (navPoints[i].type != nearLeftCorT && navPoints[i].type != farLeftCorT) {
           while (exZ < gridZ && grid[y][(int)exZ][x] && !grid[y][(int)exZ][x]->wall[top]) {
               exZ++;
           }

           vec3 n3 = { navPoints[i].pos.x, navPoints[i].pos.y, exZ - 1 };

           if (navPoints[i].type == farRightCorT) {
               n3.z += 0.5f;
           }

           if (navPoints[i].type == nearRightCorT) {
               n3.z += 0.5f;
           }

           if (n3.z != navPoints[i].pos.z && !isAlreadyNavPoint(n3)) {
               extentedNavPointsSize++;
               navPoints = realloc(navPoints, sizeof(navPoints[0]) * extentedNavPointsSize);
               navPoints[extentedNavPointsSize - 1].pos = n3;
               navPoints[extentedNavPointsSize - 1].used = false;

               if (navPoints[i].type == farRightCorT) {
                   navPoints[extentedNavPointsSize - 1].type = farLeftCorT;
               }

               if (navPoints[i].type == nearRightCorT) {
                   navPoints[extentedNavPointsSize - 1].type = nearLeftCorT;
               }
           }
       }

       exZ = navPoints[i].pos.z;

       //exZ++;
       if (navPoints[i].type != farRightCorT && navPoints[i].type != nearRightCorT) {
           while (exZ >= 0 && grid[y][(int)exZ][x] && !grid[y][(int)exZ][x]->wall[top]) {
               exZ--;
           }

           vec3 n4 = { navPoints[i].pos.x, navPoints[i].pos.y, exZ - 0.5f };

           if (n4.z != navPoints[i].pos.z && !isAlreadyNavPoint(n4)) {
               extentedNavPointsSize++;
               navPoints = realloc(navPoints, sizeof(navPoints[0]) * extentedNavPointsSize);
               navPoints[extentedNavPointsSize - 1].pos = n4;
               navPoints[extentedNavPointsSize - 1].used = false;

               if (navPoints[i].type == farLeftCorT) {
                   navPoints[extentedNavPointsSize - 1].type = farRightCorT;
               }

               if (navPoints[i].type == nearLeftCorT) {
                   navPoints[extentedNavPointsSize - 1].type = nearRightCorT;
               }
           }
       }
   }
  
  navPointsSize = extentedNavPointsSize;
  
  for (int i = 0; i < navPointsSize; i++) {
      if (navPoints[i].used) {
          continue;
      }

      float lowestZ = 1000.0f;
      float lowestX = 1000.0f;

      vec3 lowestPointZ = { 0 };
      vec3 lowestPointX = { 0 };

      //  rectCollections[i] = calloc(4, sizeof(vec3));

      int zSetted = -1;
      int xSetted = -1;



      for (int i2 = 0; i2 < navPointsSize; i2++) {
          if (i2 == i) {
              continue;
          }

          if (navPoints[i2].used) {
              continue;
          }

          if (navPoints[i2].pos.y != navPoints[i].pos.y) {
              continue;
          }


          if (navPoints[i].type == nearLeftCorT) {
              if (navPoints[i2].type == nearRightCorT) {
                  //	  printf("con %d %d\n", navPoints[i2].pos.z == navPoints[i].pos.z, navPoints[i2].type == nearRightCorT);
                  printf("con 1:(z:%f x:%f) 2:(z:%f x:%f)\n", navPoints[i2].pos.z, navPoints[i2].pos.x, navPoints[i].pos.z, navPoints[i].pos.x);
              }

              if (navPoints[i2].pos.x == navPoints[i].pos.x && navPoints[i2].type == nearRightCorT) {
                  float dist = fabs(navPoints[i].pos.z - navPoints[i2].pos.z);

                  if (dist < lowestZ) {
                      lowestZ = dist;
                      //	    lowestPointX = navPoints[i].pos;
                      zSetted = i2;
                  }
              }
              else if (navPoints[i2].pos.z == navPoints[i].pos.z && navPoints[i2].type == farLeftCorT) {
                  float dist = fabs(navPoints[i].pos.x - navPoints[i2].pos.x);

                  if (dist < lowestX) {
                      lowestX = dist;
                      //	    lowestPointZ = navPoints[i].pos;
                      xSetted = i2;
                  }
              }
          }
          else if (navPoints[i].type == farLeftCorT) {
              if (navPoints[i2].pos.z == navPoints[i].pos.z && navPoints[i2].type == nearLeftCorT) {
                  float dist = fabs(navPoints[i].pos.x - navPoints[i2].pos.x);

                  if (dist < lowestX) {
                      lowestX = dist;
                      //	    lowestPointZ = navPoints[i].pos;
                      xSetted = i2;
                  }
              }
              else if (navPoints[i2].pos.x == navPoints[i].pos.x && navPoints[i2].type == farRightCorT) {
                  float dist = fabs(navPoints[i].pos.z - navPoints[i2].pos.z);

                  if (dist < lowestZ) {
                      lowestZ = dist;
                      //	    lowestPointX = navPoints[i].pos;
                      zSetted = i2;
                  }
              }
          }
          else if (navPoints[i].type == nearRightCorT) {
              //	printf("c1 %d c2 %d \n", navPoints[i2].pos.x == navPoints[i].pos.x, navPoints[i2].type == nearLeftCor);
              if (navPoints[i2].pos.x == navPoints[i].pos.x && navPoints[i2].type == nearLeftCorT) {
                  float dist = fabs(navPoints[i].pos.z - navPoints[i2].pos.z);

                  if (dist < lowestZ) {
                      lowestZ = dist;
                      //	    lowestPointX = navPoints[i].pos;
                      zSetted = i2;
                  }
              }
              else if (navPoints[i2].pos.z == navPoints[i].pos.z && navPoints[i2].type == farRightCorT) {
                  float dist = fabs(navPoints[i].pos.x - navPoints[i2].pos.x);

                  if (dist < lowestX) {
                      lowestX = dist;
                      //	    lowestPointZ = navPoints[i].pos;
                      xSetted = i2;
                  }
              }
          }
          else if (navPoints[i].type == farRightCorT) {
              if (navPoints[i2].pos.x == navPoints[i].pos.x && navPoints[i2].type == farLeftCorT) {
                  float dist = fabs(navPoints[i].pos.z - navPoints[i2].pos.z);

                  if (dist < lowestZ) {
                      lowestZ = dist;
                      //lowestPointX = navPoints[i].pos;
                      zSetted = i2;
                  }
              }
              else if (navPoints[i2].pos.z == navPoints[i].pos.z && navPoints[i2].type == nearRightCorT) {
                  float dist = fabs(navPoints[i].pos.x - navPoints[i2].pos.x);

                  if (dist < lowestX) {
                      lowestX = dist;
                      xSetted = i2;
                      //	    lowestPointZ = navPoints[i].pos;
                  }
              }
          }
      }

    //    printf("nav %d %f %f\n", zSetted, lowestX, lowestZ);
    
    navPoints[i].used = true;

    if(zSetted != -1 && xSetted != -1){
      navPoints[zSetted].used = true;
      navPoints[xSetted].used = true;

      triCollSize++;

      if(!triColl){
	triColl = malloc(sizeof(vec3*));
      }else{
	triColl = realloc(triColl, sizeof(vec3*) * triCollSize);
      }

      triColl[triCollSize-1] = malloc(sizeof(vec3) * 3);
      
      triColl[triCollSize-1][0]= navPoints[i].pos;
      triColl[triCollSize-1][1]= navPoints[zSetted].pos;
      triColl[triCollSize-1][2]= navPoints[xSetted].pos;
    }
  }

  // conn mesh
  {
    navPointsConnMesh.vBuf = malloc(sizeof(float) * (triCollSize) * 10 * 3);
    
    // asseble navPointsMesh
    int index = 0;
    for(int i=0;i<(triCollSize) * 10 * 3;i+=10*3){
      // 1
	navPointsConnMesh.vBuf[i + 0] = triColl[index][0].x;
	navPointsConnMesh.vBuf[i + 1] = triColl[index][0].y;
	navPointsConnMesh.vBuf[i + 2] = triColl[index][0].z;

	navPointsConnMesh.vBuf[i + 3] = triColl[index][1].x;
	navPointsConnMesh.vBuf[i + 4] = triColl[index][1].y;
	navPointsConnMesh.vBuf[i + 5] = triColl[index][1].z;

	// 2
	navPointsConnMesh.vBuf[i + 6] = triColl[index][1].x;
	navPointsConnMesh.vBuf[i + 7] = triColl[index][1].y;
	navPointsConnMesh.vBuf[i + 8] = triColl[index][1].z;

	navPointsConnMesh.vBuf[i + 9] = triColl[index][2].x;
	navPointsConnMesh.vBuf[i + 10] = triColl[index][2].y;
	navPointsConnMesh.vBuf[i + 11] = triColl[index][2].z;

	// 3
	navPointsConnMesh.vBuf[i + 12] = triColl[index][2].x;
	navPointsConnMesh.vBuf[i + 13] = triColl[index][2].y;
	navPointsConnMesh.vBuf[i + 14] = triColl[index][2].z;

	navPointsConnMesh.vBuf[i + 15] = triColl[index][0].x;
	navPointsConnMesh.vBuf[i + 16] = triColl[index][0].y;
	navPointsConnMesh.vBuf[i + 17] = triColl[index][0].z;

	// 4
	navPointsConnMesh.vBuf[i + 18] = triColl[index][1].x;
	navPointsConnMesh.vBuf[i + 19] = triColl[index][1].y;
	navPointsConnMesh.vBuf[i + 20] = triColl[index][1].z;
	
	navPointsConnMesh.vBuf[i + 21] = triColl[index][2].x;
	navPointsConnMesh.vBuf[i + 22] = triColl[index][0].y;
	navPointsConnMesh.vBuf[i + 23] = triColl[index][1].z;

	// 5
	navPointsConnMesh.vBuf[i + 24] = triColl[index][2].x;
	navPointsConnMesh.vBuf[i + 25] = triColl[index][2].y;
	navPointsConnMesh.vBuf[i + 26] = triColl[index][2].z;

	navPointsConnMesh.vBuf[i + 27] = triColl[index][2].x;
	navPointsConnMesh.vBuf[i + 28] = triColl[index][0].y;
	navPointsConnMesh.vBuf[i + 29] = triColl[index][1].z;

	printf("(%f %f %f) - (%f %f %f) | (%f %f %f) - (%f %f %f) | (%f %f %f) - (%f %f %f) \n",
	       navPointsConnMesh.vBuf[i + 0], navPointsConnMesh.vBuf[i + 1], navPointsConnMesh.vBuf[i + 2],
	       navPointsConnMesh.vBuf[i + 3],navPointsConnMesh.vBuf[i + 4], navPointsConnMesh.vBuf[i + 5],
	       navPointsConnMesh.vBuf[i + 6], navPointsConnMesh.vBuf[i + 7], navPointsConnMesh.vBuf[i + 8],
	       navPointsConnMesh.vBuf[i + 9], navPointsConnMesh.vBuf[i + 10], navPointsConnMesh.vBuf[i + 11],
	       navPointsConnMesh.vBuf[i + 12], navPointsConnMesh.vBuf[i + 13], navPointsConnMesh.vBuf[i + 14],
	       navPointsConnMesh.vBuf[i + 15], navPointsConnMesh.vBuf[i + 16], navPointsConnMesh.vBuf[i + 17]);
	
	index++;
    }

    glBindBuffer(GL_ARRAY_BUFFER, navPointsConnMesh.VBO);
    glBindVertexArray(navPointsConnMesh.VAO);

    navPointsConnMesh.vertexNum = (triCollSize) * 10;
    navPointsConnMesh.attrSize = 3;

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (triCollSize) * 10 * 3, navPointsConnMesh.vBuf, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);
  
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  // /*
  if(navPointsMesh.vBuf){
    free(navPointsMesh.vBuf);
    navPointsMesh.vBuf = NULL;
  }

  navPointsMesh.vBuf = malloc(sizeof(float) * 3 * cube.vertexNum * navPointsSize);
  
  // asseble navPointsMesh
  int index = 0;
  for(int i=0;i<navPointsSize*cube.vertexNum*3;i+=cube.vertexNum*3){
    for(int i2=0;i2<cube.vertexNum*3;i2+=3){
      navPointsMesh.vBuf[i + i2 + 0] = cube.vBuf[i2 + 0] + (float)navPoints[index].pos.x;
      navPointsMesh.vBuf[i + i2 + 1] = cube.vBuf[i2 + 1] + (float)navPoints[index].pos.y;
      navPointsMesh.vBuf[i + i2 + 2] = cube.vBuf[i2 + 2] + (float)navPoints[index].pos.z;

      //   printf("%f %f %f \n", navPointsMesh.vBuf[i + i2 + 0], navPointsMesh.vBuf[i + i2 + 1], navPointsMesh.vBuf[i + i2 + 2]);
    }
    
    index++;
  }

  glBindBuffer(GL_ARRAY_BUFFER, navPointsMesh.VBO);
  glBindVertexArray(navPointsMesh.VAO);

  navPointsMesh.vertexNum = navPointsSize * cube.vertexNum;
  navPointsMesh.attrSize = 3;

  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * cube.vertexNum * navPointsSize, navPointsMesh.vBuf, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  
}
