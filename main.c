#include "deps.h"
#include "linearAlg.h"
#include "main.h"
#include "editor.h"
#include "game.h"

#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"

int renderCapYLayer;
EngineInstance curInstance = editorInstance;

int navPointsSize;
vec3* navPoints;
VPair navPointsMesh;

//const int SHADOW_WIDTH = 128, SHADOW_HEIGHT = 128;
const int SHADOW_WIDTH = 512, SHADOW_HEIGHT = 512;  
unsigned int depthMapFBO;
unsigned int depthCubemap;

void glErrorCheck(){
  GLenum er = glGetError();

  if(er != GL_NO_ERROR){
    printf("\nEr: %d %d\n\n", __LINE__, er);
  }
}

vec3 lightPos;// = {}

float near_plane;
float far_plane;

VPair circle;

const void(*instances[instancesCounter][funcsCounter])() = {
  [editorInstance] = {
    [render2DFunc] = editor2dRender,
    [render3DFunc] = editor3dRender,
    [preLoopFunc] = editorPreLoop,
    [preFrameFunc] = editorPreFrame,
    [matsSetup] = editorMatsSetup,
    [eventFunc] = editorEvents,
    [onSetFunc] = editorOnSetInstance,
  },
  [gameInstance] = {
    [render2DFunc] = game2dRender,
    [render3DFunc] = game3dRender,
    [preLoopFunc] = gamePreLoop,
    [preFrameFunc] = gamePreFrame,
    [matsSetup] = gameMatsSetup,
    [eventFunc] = gameEvents,
    [onSetFunc] = gameOnSetInstance,
  }
};

// ~~~~~~~~~~~~~~~~~
const char* instancesStr[] = { [editorInstance]="Editor", [gameInstance]="Game" };

const char* wallTypeStr[] = {
  [wallT] = "Wall",[windowT] = "Window",[doorT] = "Door",
  [wallJointT] = "Joint"
};

const char* tileBlocksStr[] = { [roofBlockT] = "Roof",[stepsBlockT] = "Steps",[angledRoofT] = "Angle Roof" };

const char* lightTypesStr[] = { [shadowPointLightT] = "Point light(shadow)",[pointLightT] = "Point light" };

const char* wallPlanesStr[] = {
  [wTopPlane] = "Top plane",
  [wFrontPlane] = "Front plane",
  [wBackPlane] = "Back plane",
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

  [winFrontPodokonik] = "Front-padokonik",
  [winBackPodokonik] = "Back-padokonik",
};

const int planesInfo[wallTypeCounter] = {
  [wallT] = wPlaneCounter,
  [windowT] = winPlaneCounter,
  [doorT] = doorPlaneCounter,
  [wallJointT] = jointPlaneCounter,
};

const char* wallJointPlanesStr[] = {
  [jointTopPlane] = "Top",
  [jointFrontPlane] = "Front",
  [jointSidePlane] = "Side"
};

const char* doorPlanesStr[] = {
  [winTopPlane] = "Top plane",
  [doorFrontPlane] = "Front plane",
  [doorBackPlane] = "Back plane",
  [doorCenterBackPlane] = "Center-back plane" ,
  [doorCenterFrontPlane] = "Center-front plane" ,
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
  [characterModelType] = {"Char", 0}
};

const char* shadersFileNames[] = { "lightSource", "hud", "fog", "borderShader", "screenShader", [pointShadowShader] = "pointShadowDepth" };
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

Light lightDef = { .color = rgbToGl(253.0f, 244.0f, 220.0f), .dir = {0,-1, 0}, .curLightPresetIndex = 7 };

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

typedef struct{
  vec3i indx;

  uint8_t wallsSize;
  uint8_t* wallsIndexes;

  uint8_t jointsSize;
  uint8_t* jointsIndexes;
} BatchedTile;

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
  lightDef.rad = cosf(rad(12.5f));
  lightDef.cutOff = cosf(rad(17.5f));

  SDL_Init(SDL_INIT_VIDEO);

    
  SDL_DisplayMode DM;
  SDL_GetCurrentDisplayMode(0, &DM);

  SDL_Rect r1;

  SDL_GetDisplayUsableBounds(0, &r1);

  windowW = DM.w;
  windowH = DM.h;

  printf("def %d %d \n", windowH, windowW);
  //    printf(" %d %d \n",);
  printf("Usable %d %d \n", r1.h, r1.w);
    

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

  // circle buf
  {
    circle.vBuf = malloc(sizeof(float) * 3 * 180 * 2);
    circle.vertexNum = 180 * 2;
    circle.attrSize = 3;

    int index = 0;
    vec3 circleA = { 0 };
    
    float r = 1.0f;
    float h = 1.0f;
    float k = 1.0f;
    
    for (int i = 0; i < 180; i++)
      {
	circleA.x = r * cos(i) - h;
    circleA.y = r * sin(i) + k;
	
	circle.vBuf[index+0] = circleA.x + k;
	circle.vBuf[index+1] = circleA.y - h;
	circle.vBuf[index+2] = 0;
	
	index+=3;
    
    circleA.x = r * cos(i + 0.1) - h;
    circleA.y = r * sin(i + 0.1) + k;
	
	circle.vBuf[index+0] = circleA.x + k;
	circle.vBuf[index+1] = circleA.y - h;
	circle.vBuf[index+2] = 0;
	
	//	glVertex3f(circle.x + k,circle.y - h,0);
	index+=3;
      }

    glGenVertexArrays(1, &circle.VAO);
    glBindVertexArray(circle.VAO);

    glGenBuffers(1, &circle.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, circle.VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 360, circle.vBuf, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);
    
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

  assembleWallBlockVBO();
  assembleWindowBlockVBO();
  assembleDoorBlockVBO();
  assembleWallJointVBO();

  assembleBlocks();

  // plane 3d
  {
    glGenBuffers(1, &planePairs.VBO);
    glGenVertexArrays(1, &planePairs.VAO);

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

    //    glEnable(GL_CULL_FACE);
    //    glCullFace(GL_FRONT);

	
    //	glEnable(GL_CULL_FACE);

    //	   glEnable(GL_TEXTURE_2D);
    //    glShadeModel(GL_SMOOTH);
    //glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    //    glDepthFunc(GL_LESS);
    
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    //    glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
    //    glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
      glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    
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


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
	if (strcmp(typeStr, "Char") == 0) {
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
      loadedModels2D[characterModelType] = malloc(sizeof(ModelInfo) * objsCounter);

      rewind(objsSpecs);

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

	loadedModel->type = type;
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
    // create depth cubemap texture
    //	glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, depthCubemap);
	
    //    for (unsigned int i = 0; i < 6; ++i){
      //      glTexImage2D(GL_TEXTURE_CUBE_MAP_ARRAY_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
      //    }
	
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 6 * 6, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    //	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    ///	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

	
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      printf("main fbo creation failed! With %d \n", glCheckFramebufferStatus(GL_FRAMEBUFFER));
      exit(0);
    }
	
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);
    //	glActiveTexture(GL_TEXTURE0);
  }

  geometry = calloc(loadedTexturesCounter, sizeof(Geometry));

  for (int i = 0; i < loadedTexturesCounter; i++) {
    glGenVertexArrays(1, &geometry[i].pairs.VAO);
    glBindVertexArray(geometry[i].pairs.VAO);

    glGenBuffers(1, &geometry[i].pairs.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, geometry[i].pairs.VBO);

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
  batchGeometry();
  batchModels();

  // set up camera
  GLint cameraPos = glGetUniformLocation(shadersId[mainShader], "cameraPos");
  {
    camera1.pos = (vec3)xyz_indexesToCoords(gridX / 2, 2, gridZ / 2);
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
  
  glUseProgram(shadersId[pointShadowShader]);
  uniformFloat(pointShadowShader, "far_plane", far_plane);
  
  glUseProgram(shadersId[mainShader]);
  uniformInt(mainShader, "colorMap", 0); 
  uniformInt(mainShader, "depthMapsArray", 1);
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
	/*                if (event.key.keysym.scancode == SDL_SCANCODE_F11){
			  fullScreen = !fullScreen;

			  if (fullScreen) {
			  SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

			  }
			  else {
			  SDL_SetWindowFullscreen(window, 0);
			  }
			  }*/

	if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
	  exit(1);
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

    ((void (*)(float))instances[curInstance][preFrameFunc])(deltaTime);

    mouse.tileSide = -1;
    checkMouseVSEntities();

    //   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    /*    glStencilMask(0xff);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glStencilMask(0x00);
	
    // render to depth cubemap
    glUseProgram(shadersId[pointShadowShader]);
    glEnable(GL_DEPTH_TEST);
    
    //    if(lightsStore[shadowPointLightT])
    glViewport(0,0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);*/

    /*    int index = 0;
    for(int i=0;i<lightsStoreSizeByType[shadowPointLightT];i++){
      //      if(lightsStore[shadowPointLightT][i].off){
	//	continue;
	//      }      
      
	glStencilMask(0xff);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilMask(0x00);

	Matrix shadowProj = perspective(rad(90.0f), 1.0f, near_plane, far_plane); 

	vec3 negPos = { -lightsStore[shadowPointLightT][i].mat.m[12], -lightsStore[shadowPointLightT][i].mat.m[13], -lightsStore[shadowPointLightT][i].mat.m[14] };

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

	  uniformMat4(pointShadowShader, buf, shadowTransforms.m);
	}
      }

    if(lightsStoreSizeByType[shadowPointLightT] > 0){
	glActiveTexture(GL_TEXTURE0);
	renderScene(pointShadowShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);   
    }*/

    
    //  if (lightsStore)
      {
	glViewport(0, 0, windowW, windowH);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glStencilMask(0xff);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT  | GL_STENCIL_BUFFER_BIT);
	glStencilMask(0x00);
	
	glEnable(GL_DEPTH_TEST);
	
	((void (*)(int))instances[curInstance][matsSetup])(mainShader);

	glUseProgram(shadersId[mainShader]); 
		
        glUniform3f(cameraPos, argVec3(curCamera->pos));
        uniformFloat(mainShader, "far_plane", far_plane);

	//	vec3 modelXLight = {lightsStore[0].mat.m[12], lightsStore[0].mat.m[13], lightsStore[0].mat.m[14]};
        //uniformVec3(mainShader, "lightPoss", modelXLight);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, depthCubemap);

	glActiveTexture(GL_TEXTURE0);
	renderScene(mainShader);

	((void (*)(void))instances[curInstance][render3DFunc])();

	// nav meshes drawing
	{
	  glUseProgram(shadersId[lightSourceShader]);
	    
	  glBindBuffer(GL_ARRAY_BUFFER, navPointsMesh.VBO);
	  glBindVertexArray(navPointsMesh.VAO);

	  uniformVec3(lightSourceShader, "color", (vec3) { blueColor });

	  Matrix out = IDENTITY_MATRIX;
	  uniformMat4(lightSourceShader, "model", out.m);

	  glDrawArrays(GL_TRIANGLES, 0, navPointsMesh.vertexNum);

	  glBindBuffer(GL_ARRAY_BUFFER, 0);
	  glBindVertexArray(0);

	  glUseProgram(shadersId[mainShader]);
	}

	
	// highlight selected model with stencil
	if(true)
	{
	  if(mouse.selectedType == mouseModelT){
	    glStencilFunc(GL_ALWAYS, 1, 0xFF);
	    glStencilMask(0xFF);

	    Model* model = (Model*)mouse.selectedThing;
	    int name = model->name;
	    
	    glBindTexture(GL_TEXTURE_2D, loadedModels1D[name].tx);

	    glBindVertexArray(loadedModels1D[name].VAO);
	    glBindBuffer(GL_ARRAY_BUFFER, loadedModels1D[name].VBO);

	    uniformMat4(mainShader, "model", model->mat.m);

	    glDrawArrays(GL_TRIANGLES, 0, loadedModels1D[name].size);

	    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	    glStencilMask(0x00);
	    
	    // higlight
	    {
	      glUseProgram(shadersId[borderShader]);
	    
	      //	      glStencilFunc(GL_NOTEQUAL	, 1, 0xFF);
	      //	      glStencilMask(0x00);
      
	      glDisable(GL_DEPTH_TEST);
      
	      uniformMat4(borderShader, "model", model->mat.m);
	      uniformVec3(borderShader, "borderColor", (vec3){ redColor });
	      glDrawArrays(GL_TRIANGLES, 0, loadedModels1D[name].size);

	      glEnable(GL_DEPTH_TEST);

	      glUseProgram(shadersId[mainShader]);

	      glStencilMask(0x00);
	    }
	    
	    glBindBuffer(GL_ARRAY_BUFFER, 0);
	    glBindVertexArray(0);

	    glBindTexture(GL_TEXTURE_2D, 0);
	  }else if(false && mouse.selectedType == mouseLightT){
	    glStencilFunc(GL_ALWAYS, 1, 0xFF);
	    glStencilMask(0xFF);
	    
	    Light* light = (Light*)mouse.selectedThing;

	    glUseProgram(shadersId[lightSourceShader]);
	    
	    uniformVec3(lightSourceShader, "color", light->color);

	    glBindBuffer(GL_ARRAY_BUFFER, cube.VBO);
	    glBindVertexArray(cube.VAO);

	    uniformMat4(lightSourceShader, "model", light->mat.m);

	    glDrawArrays(GL_TRIANGLES, 0, cube.vertexNum);
	    
	    // higlight
	    {
	      //	      glUseProgram(shadersId[lightSourceShader]);
	    
	      glStencilFunc(GL_NOTEQUAL	, 1, 0xFF);
	      glStencilMask(0x00);
      
	      glDisable(GL_DEPTH_TEST);

	      //Matrix out = IDENTITY_MATRIX;
	      //  memcpy(out.m, light->mat.m, sizeof(float) * 16);

	      scale(&light->mat, 1.9f,1.9f,1.9f);
      
	      uniformMat4(borderShader, "model", light->mat.m);

	      scale(&light->mat, 1.0f/1.9f,1.0f/1.9f,1.0f/1.9f);
	      
	      uniformVec3(borderShader, "color", (vec3){ redColor }); 
	      glDrawArrays(GL_TRIANGLES, 0, cube.vertexNum);

	      glEnable(GL_DEPTH_TEST);

	      glUseProgram(shadersId[mainShader]);

	      glStencilMask(0x00);
	    }

	    glBindTexture(GL_TEXTURE_2D, 0);
      
	    glBindBuffer(GL_ARRAY_BUFFER, 0); 
	    glBindVertexArray(0); 
	  }
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

  //  glUniformMatrix4fv(lightModelLoc, 1, GL_FALSE, lightsStore[lightId].mat.m);



     
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

inline bool radarCheck(vec3 point){
  vec3 v = { point.x - camera1.pos.x, point.y - camera1.pos.y, point.z - camera1.pos.z };

  vec3 minusZ = { -camera1.Z.x, -camera1.Z.y, -camera1.Z.z };
      
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
}

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
  
  fscanf(map, "Walls used: %d b:%d j:%d w:%d t:%d", &wallsCounter, &blockCounter,&jointsCounter,&wallCounter,&tileCounter);

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
   
  for (int i = 0; i < wallsCounter; i++) {
    int x = -1,y = -1,z = -1;
    
    fscanf(map, "\n%d %d %d ",&x,&y,&z); 
	 
    int sidesCounter = 0;

    grid[y][z][x] = calloc(1, sizeof(Tile));
    Tile* tile = grid[y][z][x]; 

    float groundLiftDELETEIT;

    fscanf(map, "%d %f %d ", &tile->ground, &groundLiftDELETEIT, &sidesCounter);

    vec3 grid = xyz_indexesToCoords(x,y,z);

    tile->pos = grid;

    GroundType type = valueIn(tile->ground, 0);

    GroundType tx2 = valueIn(tile->ground, 2);  
    
    if(y == 0 && (type == netTileT || type == 0)){
      setIn(tile->ground, 0, texturedTile); 
      setIn(tile->ground, 2, textureOfGround);
    }

    Side side = -1; 
    int bufSize;

    int alligned = 0;
    int txHidden = 0;
	  
    WallType wType = -1;

    // walls
    for(int i2=0;i2<sidesCounter;i2++){  
      fscanf(map, "%d %d ", &side, &wType);

      tile->walls[side].type = wType;       
      tile->walls[side].planes = calloc(planesInfo[tile->walls[side].type], sizeof(Plane));

      for(int i2=0;i2<wallsVPairs[wType].planesNum;i2++){
	fscanf(map, "%d %d ", &tile->walls[side].planes[i2].txIndex, &tile->walls[side].planes[i2].hide);
      }

      for(int mat=0;mat<16;mat++){
	fscanf(map, "%f ", &tile->walls[side].mat.m[mat]);
      }

      for (int i = 0; i < wallsVPairs[wType].planesNum; i++) {
	calculateAABB(tile->walls[side].mat, wallsVPairs[wType].pairs[i].vBuf, wallsVPairs[wType].pairs[i].vertexNum, wallsVPairs[wType].pairs[i].attrSize, &tile->walls[side].planes[i].lb, &tile->walls[side].planes[i].rt);
      }
    }

    // joints
    for(int i2=0;i2<basicSideCounter;i2++){
      int jointSide = -1;
      int exist = -1;
      
      fscanf(map, "%d %d ", &jointSide, &exist);

      tile->jointExist[jointSide] = exist;

      if(exist){
	for(int i2=0;i2<jointPlaneCounter;i2++){
	  fscanf(map, "%d %d ", &tile->joint[jointSide][i2].txIndex, &tile->joint[jointSide][i2].hide);
	}

	for(int mat=0;mat<16;mat++){
	  fscanf(map, "%f ", &tile->jointsMat[jointSide].m[mat]);
	}
	
	for (int i = 0; i < wallsVPairs[wallJointT].planesNum; i++) {
	  calculateAABB(tile->jointsMat[jointSide], wallsVPairs[wallJointT].pairs[i].vBuf, wallsVPairs[wallJointT].pairs[i].vertexNum, wallsVPairs[wallJointT].pairs[i].attrSize, &tile->joint[jointSide][i].lb, &tile->joint[jointSide][i].rt);
	}
      }
    }

    int blockExists;
    fscanf(map, "%d ", &blockExists);

    if(blockExists){
      TileBlock* newBlock = malloc(sizeof(TileBlock));

      int txIndex;
      int blockType;
      int rotateAngle;

      fscanf(map, "%d %d %d %d ",&blockType, &rotateAngle, &txIndex);
      
      newBlock->txIndex = txIndex;
      newBlock->rotateAngle = rotateAngle;
      newBlock->type = blockType;
      newBlock->tile = tile;

      //      newBlock->vpair.VBO

      //      newBlock->vpair.VBO = tileBlocksTempl[newBlock->type].vpair.VBO;
      //      newBlock->vpair.VAO = tileBlocksTempl[newBlock->type].vpair.VAO;

      for(int mat=0;mat<16;mat++){
	fscanf(map, "%f ", &newBlock->mat.m[mat]);
      }
      
      calculateAABB(newBlock->mat, blocksVPairs[newBlock->type].pairs[0].vBuf, blocksVPairs[newBlock->type].pairs[0].vertexNum, blocksVPairs[newBlock->type].pairs[0].attrSize, &newBlock->lb, &newBlock->rt);

      tile->block = newBlock;
    }
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

  fscanf(map, "\nUsed planes: %d\n", &createdPlanesSize);

  if (curModelsSize != 0) { 
    createdPlanes = malloc(createdPlanesSize * sizeof(Model));

    for (int i = 0; i < createdPlanesSize; i++) {
      int tx = -1;
      float w, h;
      
      fscanf(map, "%d %f %f ", &tx, &w, &h);

      if (tx >= loadedTexturesCounter || tx < 0) {
	printf("Models parsing error, model name (%d) doesnt exist \n", tx);
	exit(0);
      }

      createdPlanes[i].w = w;
      createdPlanes[i].h = h;
      createdPlanes[i].txIndex = tx;
      createdPlanes[i].id = i;
      createdPlanes[i].characterId = -1;

      fgetc(map); // read [

      for (int mat = 0; mat < 16; mat++) {
	fscanf(map, "%f ", &createdPlanes[i].mat.m[mat]); 
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

	createdPlanes[i].characterId = charactersSize - 1;
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

 
  printf("Save %s loaded! \n", save);
  fclose(map);
  
  strcpy(curSaveName, saveName);
  free(save);
   
  initSnowParticles();
  
  return true;
}

bool saveMap(char *saveName){
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
  int tileCounter = 0;

  for (int y = 0; y < gridY; y++) {
    for (int z = 0; z < gridZ; z++) {
      for (int x = 0; x < gridX; x++) {
	Tile* tile = grid[y][z][x];

	if (!tile) continue;

	GroundType type = valueIn(tile->ground, 0);
	int tx1 = valueIn(tile->ground, 1);
	int tx2 = valueIn(tile->ground, 2);

	bool acceptTile = (y == 0 && type == texturedTile &&  tx2 != textureOfGround) || (y != 0 && type == texturedTile);

	if(tile->walls[0].planes || tile->walls[1].planes || tile->walls[2].planes || tile->walls[3].planes){
	  wallsIndexes[wallsCounter] = (vec3i){x,y,z};
	  wallsCounter++;
	  wallCounter++;
	}else if (tile->jointExist[0] || tile->jointExist[1] || tile->jointExist[2] || tile->jointExist[3]) {
	  wallsIndexes[wallsCounter] = (vec3i){ x,y,z };
	  wallsCounter++;
	  jointsCounter++;
	}else if (acceptTile) {
	  wallsIndexes[wallsCounter] = (vec3i){ x,y,z };
	  wallsCounter++;
	  tileCounter++;
	}else if (tile->block) {
	  wallsIndexes[wallsCounter] = (vec3i){ x,y,z };
	  wallsCounter++;
	  blockCounter++;
	}
      }
    }
  }

  fprintf(map, "Walls used: %d b:%d j:%d w:%d t:%d", blockCounter+jointsCounter+wallCounter+tileCounter, blockCounter,jointsCounter,wallCounter,tileCounter);
  
  for (int i = 0; i < wallsCounter; i++) {
    fprintf(map, "\n");
      
    int x = wallsIndexes[i].x;
    int y = wallsIndexes[i].y; 
    int z = wallsIndexes[i].z;
    
    Tile* tile = grid[y][z][x];

    int sidesAvaible = 0;
    
    if(tile->walls[0].planes){
      sidesAvaible++;
    }

    if(tile->walls[1].planes){
      sidesAvaible++;
    }

    if(tile->walls[2].planes){
      sidesAvaible++;
    }

    if(tile->walls[3].planes){
      sidesAvaible++;
    }

    float liftDELETEIT;

    fprintf(map, "%d %d %d %d %f %d ",x,y,z, grid[y][z][x]->ground, &liftDELETEIT, sidesAvaible);

    // walls
    for(int i1=0;i1<basicSideCounter;i1++){
      if(tile->walls[i1].planes){
	fprintf(map, "%d %d ",i1,tile->walls[i1].type);

	// plane data save
	for(int i2=0;i2<planesInfo[tile->walls[i1].type];i2++){
	  fprintf(map, "%d %d ", tile->walls[i1].planes[i2].txIndex,
		  tile->walls[i1].planes[i2].hide);
	}

	for(int mat=0;mat<16;mat++){
	  fprintf(map, "%f ", tile->walls[i1].mat.m[mat]);
	}
      }
    }

    // joints
    for(int i1=0;i1<basicSideCounter;i1++){
      fprintf(map, "%d %d ",i1, tile->jointExist[i1]);
      
      if(tile->jointExist[i1]){
	// plane data save
	for(int i2=0;i2<jointPlaneCounter;i2++){
	  fprintf(map, "%d %d ",tile->joint[i1][i2].txIndex,
		  tile->joint[i1][i2].hide);
	}

	for(int mat=0;mat<16;mat++){
	  fprintf(map, "%f ", tile->jointsMat[i1].m[mat]);
	}
      }
    }

    if(tile->block){
      fprintf(map, "%d ", 1); // block exists
      fprintf(map, "%d %d %d %d ",tile->block->type, tile->block->rotateAngle, tile->block->txIndex);

      //      for(int i2=0;i2<tile->block->vertexesSize*5;i2++){
      //	fprintf(map, "%f ",tile->block->vertexes[i2]);
      //      }

      for(int mat=0;mat<16;mat++){
	fprintf(map, "%f ", tile->block->mat.m[mat]);
      }
    }else{
      fprintf(map, "%d ", 0); // block doesnt exists
    }
  }

  free(wallsIndexes);

  fprintf(map, "\nUsed models: %d\n",curModelsSize);
	
  for(int i=0; i<curModelsSize; i++){    
    fprintf(map, "%d ", curModels[i].name);

    fprintf(map, "[");

    for(int mat=0;mat<16;mat++){
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

  fprintf(map, "Used planes: %d\n",createdPlanesSize);
  
  for(int i=0; i<createdPlanesSize; i++){    
    fprintf(map, "%d %f %f ", createdPlanes[i].txIndex, createdPlanes[i].w, createdPlanes[i].h);

    fprintf(map, "[");

    for(int mat=0;mat<16;mat++){
      fprintf(map, "%f ", createdPlanes[i].mat.m[mat]);
    }
	  
    fprintf(map, "]");

    if (createdPlanes[i].characterId != -1) { 
      fprintf(map, "1");
      fprintf(map, "%s %d ", characters[createdPlanes[i].characterId].name ? characters[createdPlanes[i].characterId].name : "None", characters[createdPlanes[i].characterId].modelName);
      serializeDialogTree(&characters[createdPlanes[i].characterId].dialogs, map);  
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

  batchGeometry();

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

    vec3 tile = xyz_indexesToCoords(tileData->grid.x, curFloor, tileData->grid.z);
    
    newBlock->mat.m[12] = tile.x;
    newBlock->mat.m[13] = tile.y;
    newBlock->mat.m[14] = tile.z;

    printf("newBlock %f %f %f \n", argVec3(tile));
	 
    newBlock->tile = tileData->tile;
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

void assembleWallJointVBO(){  
  float w = 1;
  float h = 2;

  float capRatio = 0.12f;
  
  float t = (float)1 / 8;

  float d = t;

  float backPlane[] = {
    w+t, 0.0f , -t,       capRatio, 0.0f,
    w, h, -t,             0.0f, 1.0f,
    w+t, h, -t,           capRatio, 1.0f,

    w+t, 0.0f , -t,       capRatio, 0.0f,
    w, 0.0f , -t,         0.0f, 0.0f,
    w, h, -t,             0.0f, 1.0f,
  };
  
  float topPlane[] = {
    w+t, h, -t,    capRatio, 0.0f,
    w,h, 0.0f,          0.0f, capRatio,
    w, h, -t,         0.0f, 0.0f,

    w+t, h, -t,    capRatio, 0.0f,
    w+t,h,0.0f,         capRatio, capRatio,
    w,h, 0.0f,           0.0f, capRatio,
  };

  float e = w + t;
      
  float leftPlane[] = {
    e, 0.0f, 0.0f,  0.0f, 0.0f,
    e, h , -t,     t, 1.0f,
    e, h, 0.0f,     0.0f, 1.0f,

    e, 0.0f, 0.0f,  0.0f, 0.0f,
    e, 0.0f , -t,  t, 0.0f,
    e, h, -t,      t, 1.0f,
  };

  float* wallPlanes[jointPlaneCounter] = { [jointFrontPlane] =  backPlane, [jointTopPlane] = topPlane, [jointSidePlane] = leftPlane  };
  
  int wallPlanesSize[jointPlaneCounter] = { [jointFrontPlane] = sizeof(backPlane), [jointTopPlane] = sizeof(topPlane), [jointSidePlane] = sizeof(leftPlane) };

  wallsVPairs[wallJointT].pairs = malloc(sizeof(VPair) * jointPlaneCounter);
  wallsVPairs[wallJointT].planesNum = jointPlaneCounter;
    
  for(int i=0;i<wallsVPairs[wallJointT].planesNum;i++){
    attachNormalsToBuf(wallsVPairs[wallJointT].pairs, i, wallPlanesSize[i], wallPlanes[i]);
  }
}

void assembleWallBlockVBO() {  // wallBlock buf
  float w = 1;
  float h = 2;

  float capRatio = 0.12f;
  
  float t = (float)1 / 8;

  float d = t;   
   
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
    0.0f, 0.0f , d,  0.0f, 0.0f,
    w, h, d,         1.0f, 1.0f,
    0.0f, h, d,      0.0f, 1.0f,

    0.0f, 0.0f , d,  0.0f, 0.0f,
    w, 0.0f , d,     1.0f, 0.0f,
    w, h, d,         1.0f, 1.0f,
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

  float* wallPlanes[wPlaneCounter] = { [wTopPlane] = topPlane, [wFrontPlane] = frontPlane, [wBackPlane] = backPlane };
  int wallPlanesSize[wPlaneCounter] = { [wTopPlane] = sizeof(topPlane), [wFrontPlane] = sizeof(frontPlane), [wBackPlane] = sizeof(backPlane) };

  wallsVPairs[wallT].pairs = malloc(sizeof(VPair) * wPlaneCounter);    
  wallsVPairs[wallT].planesNum = wPlaneCounter;

  for(int i=0;i<wallsVPairs[wallT].planesNum;i++){
    attachNormalsToBuf(wallsVPairs[wallT].pairs, i, wallPlanesSize[i], wallPlanes[i]);
  }
  /*
    {
    // top
    {
    glGenBuffers(1, &wallsVPairs[wallT].pairs[wTopPlane].VBO);
    glGenVertexArrays(1, &wallsVPairs[wallT].pairs[wTopPlane].VAO);

    glBindVertexArray(wallsVPairs[wallT].pairs[wTopPlane].VAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[wallT].pairs[wTopPlane].VBO);

    wallsVPairs[wallT].pairs[wTopPlane].vertexNum = 6;

    //wallsVPairs[wallT].pairs[wTopPlane].vBuf = malloc(sizeof(topPlane) + (sizeof(vec3) * vNum));
    wallsVPairs[wallT].pairs[wTopPlane].vBuf = malloc(sizeof(topPlane));
      
    for(int i=0;i<sizeof(topPlane)/sizeof(float);i+=5*3){
    vec3 a = (vec3){ topPlane[i], topPlane[i+1], topPlane[i+2] };
    vec3 b = (vec3){ topPlane[i+5], topPlane[i+6], topPlane[i+7] };
    vec3 c = (vec3){ topPlane[i+10], topPlane[i+11], topPlane[i+12] };

    vec3 norm = calculateNormal(a,b,c);
    printf("%f %f %f \n", argVec3(norm));
    }

    int vNum = sizeof(topPlane) / sizeof(float) / 5;
      
    memcpy(wallsVPairs[wallT].pairs[wTopPlane].vBuf, topPlane, sizeof(topPlane));

    glBufferData(GL_ARRAY_BUFFER, sizeof(topPlane), topPlane, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    }
  */
}

void assembleDoorBlockVBO() {  // wallBlock buf
  float w = 1;
  float h = 2;
  
  float capRatio = 0.12f;

  float capH = h * capRatio;
  //  float botH = h * 0.4f;

  float t = (float)1 / 8;
    
  float d = t;

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
    0.0f,h, d,   0.0f, capRatio,
    w, h, -t,    1.0f, 0.0f,
    0.0f, h, -t, 0.0f, 0.0f,

    0.0f,h,d,    0.0f, capRatio,
    w,h,d,       1.0f, capRatio,
    w, h, -t,    1.0f, 0.0f,
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
      
  float centerBackPlane[] = {
    // cap front
    w, h -capH, -doorPad,         1.0f, 1.0f,
    0.0f, 0.0f , -doorPad,        0.0f, 0.0f,
    0.0f, h -capH, -doorPad,      0.0f, 1.0f,

    w, h -capH, -doorPad,         1.0f, 1.0f,
    w, 0.0f , -doorPad,           1.0f, 0.0f,
    0.0f, 0.0f , -doorPad,        0.0f, 0.0f,
  };

  float centerFrontPlane[] = {
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

  
  float* wallPlanes[doorPlaneCounter] = {
    [doorTopPlane] = topPlane,
    [doorFrontPlane] = frontPlane,
    [doorBackPlane] = backSide,
    [doorCenterFrontPlane] = centerFrontPlane,
    [doorCenterBackPlane] = centerBackPlane,
    [doorInnerTopPlane] = innerSide,
  };
  
  int wallPlanesSize[doorPlaneCounter] = {
    [doorTopPlane] = sizeof(topPlane),
    [doorFrontPlane] = sizeof(frontPlane),
    [doorBackPlane] = sizeof(backSide),
    [doorCenterFrontPlane] = sizeof(centerFrontPlane),
    [doorCenterBackPlane] = sizeof(centerBackPlane),
    [doorInnerTopPlane] = sizeof(innerSide),
  };

  wallsVPairs[doorT].pairs = malloc(sizeof(VPair) * doorPlaneCounter);
  wallsVPairs[doorT].planesNum = doorPlaneCounter;
    
  for(int i=0;i<wallsVPairs[doorT].planesNum;i++){
    attachNormalsToBuf(wallsVPairs[doorT].pairs, i, wallPlanesSize[i], wallPlanes[i]);
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
	    
  /*float rightSide[] = {
  // cap right
  w, h -capH, -t,     0.0f, 0.0f,
  w, h, -t,        0.0f, 1.0f,
  w, h , d,        t, 1.0f,

  w, h, d,         t, 1.0f,
  w, h -capH, -t,     0.0f, 0.0f,
  w, h -capH , d,     t, 0.0f,

  // bot right
  w, 0.0f, -t,     0.0f, 0.0f,
  w, botH, -t,        0.0f, 1.0f,
  w, botH , d,        t, 1.0f,

  w, botH, d,         t, 1.0f,
  w, 0.0f, -t,     0.0f, 0.0f,
  w, 0.0f , d,     t, 0.0f,
  };
      
  float leftSide[] = {
  // cap left
  0.0f, h -capH, -t,  0.0f, 0.0f,
  0.0f, h, -t,     0.0f, 1.0f,
  0.0f, h , d,     t, 1.0f,

  0.0f, h, d,      t, 1.0f,
  0.0f, h -capH, -t,  0.0f, 0.0f,
  0.0f, h -capH, d,  t, 0.0f,
      
  // bot left
  0.0f, 0.0f, -t,  0.0f, 0.0f,
  0.0f, botH, -t,     0.0f, 1.0f,
  0.0f, botH , d,     t, 1.0f,

  0.0f, botH, d,      t, 1.0f,
  0.0f, 0.0f, -t,  0.0f, 0.0f,
  0.0f, 0.0f , d,  t, 0.0f,
  };*/
      
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

  float backWindowPlane[] = {
    // main olane
    w, botH, -t,       1.0f, 0.0f,
    0.0f, botH - padokonikMainH, -padokonikD -t, 0.0f, padokonikRatio,
    0.0f, botH, -t,    0.0f, 0.0f,

    w, botH, -t,       1.0f, 0.0f,
    w, botH - padokonikMainH, -padokonikD-t, 1.0f, padokonikRatio,
    0.0f, botH - padokonikMainH, -padokonikD -t, 0.0f, padokonikRatio,

    // down thing
    w, botH - padokonikMainH, -padokonikD -t,                          1.0f, padokonikDownThingRatio,
    0.0f, botH - padokonikMainH - padokonikDownThingH, -padokonikD -t, 0.0f, 0.0f,
    0.0f, botH - padokonikMainH, -padokonikD -t,                       0.0f, padokonikDownThingRatio,

    w, botH - padokonikMainH, -padokonikD -t,                          1.0f, padokonikDownThingRatio,
    w, botH - padokonikMainH - padokonikDownThingH, -padokonikD -t,    1.0f, 0.0f,
    0.0f, botH - padokonikMainH - padokonikDownThingH, -padokonikD -t, 0.0f, 0.0f,
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
    [winTopPlane] = topSide,
    [winFrontPodokonik] = frontWindowPlane,
    [winBackPodokonik] = backWindowPlane,
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
    [winTopPlane] = sizeof(topSide),
    [winFrontPodokonik] = sizeof(frontWindowPlane),
    [winBackPodokonik] = sizeof(backWindowPlane),
    [winCenterBackPlane] = sizeof(windowPlaneBack),
    [winCenterFrontPlane] = sizeof(windowPlaneFront)
  };

  wallsVPairs[windowT].pairs = malloc(sizeof(VPair) * winPlaneCounter);
  wallsVPairs[windowT].planesNum = winPlaneCounter;
    
  for(int i=0;i<wallsVPairs[windowT].planesNum;i++){
    attachNormalsToBuf(wallsVPairs[windowT].pairs, i, wallPlanesSize[i], wallPlanes[i]);
  }
}


void setupAABBAndMatForJoint(vec2i vec, Side side){
  static const int pad[4][3] = {
    [top] = { 90, 0, 1 },
    [right] = { 0, 0, 0 },

    [bot] = { 180, 1, 0 },
    [left] = { 270, 1, 0 }
  };

  grid[curFloor][vec.z][vec.x]->jointsMat[side] = IDENTITY_MATRIX;

  grid[curFloor][vec.z][vec.x]->jointsMat[side].m[12] = vec.x + pad[side][1];
  grid[curFloor][vec.z][vec.x]->jointsMat[side].m[13] = curFloor;
  grid[curFloor][vec.z][vec.x]->jointsMat[side].m[14] = vec.z + pad[side][2];

  float xTemp = grid[curFloor][vec.z][vec.x]->jointsMat[side].m[12];
  float yTemp = grid[curFloor][vec.z][vec.x]->jointsMat[side].m[13];
  float zTemp = grid[curFloor][vec.z][vec.x]->jointsMat[side].m[14];

  rotateY(grid[curFloor][vec.z][vec.x]->jointsMat[side].m, rad(pad[side][0]));

  grid[curFloor][vec.z][vec.x]->jointsMat[side].m[12] = xTemp;
  grid[curFloor][vec.z][vec.x]->jointsMat[side].m[13] = yTemp;
  grid[curFloor][vec.z][vec.x]->jointsMat[side].m[14] = zTemp;

  for (int i = 0; i < jointPlaneCounter; i++) {
    calculateAABB(grid[curFloor][vec.z][vec.x]->jointsMat[side], wallsVPairs[wallJointT].pairs[i].vBuf, wallsVPairs[wallJointT].pairs[i].vertexNum, wallsVPairs[wallJointT].pairs[i].attrSize, &grid[curFloor][vec.z][vec.x]->joint[side][i].lb, &grid[curFloor][vec.z][vec.x]->joint[side][i].rt); 
  }
};

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
	  
	  setIn(grid[y][z][x]->ground, 0, texturedTile);
	  setIn(grid[y][z][x]->ground, 2, textureOfGround);

	  vec3 tile = xyz_indexesToCoords(x,y,z);
	
	  grid[y][z][x]->pos = tile;
	}
      }
    }
  }
}

void rerenderShadowsForAllLights(){
  int index = 0;
  for(int i=0;i<lightsStoreSizeByType[shadowPointLightT];i++){
    //      if(lightsStore[shadowPointLightT][i].off){
    //	continue;
    //      }      
      
    glStencilMask(0xff);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glStencilMask(0x00);

    Matrix shadowProj = perspective(rad(90.0f), 1.0f, near_plane, far_plane); 

    vec3 negPos = { -lightsStore[shadowPointLightT][i].mat.m[12], -lightsStore[shadowPointLightT][i].mat.m[13], -lightsStore[shadowPointLightT][i].mat.m[14] };

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

      uniformMat4(pointShadowShader, buf, shadowTransforms.m);
    }
  }

  if(lightsStoreSizeByType[shadowPointLightT] > 0){
    //    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    
    glActiveTexture(GL_TEXTURE0);
    renderScene(pointShadowShader);

    glCullFace(GL_BACK);
    //    glDisable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);   
  }
}

void rerenderShadowForLight(int lightId){
  GLint curShader = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &curShader);

  glUseProgram(shadersId[pointShadowShader]);
  glEnable(GL_DEPTH_TEST);
    
  //    if(lightsStore[shadowPointLightT])
  glViewport(0,0, SHADOW_WIDTH, SHADOW_HEIGHT);
  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

  int index = 0;
      
  glStencilMask(0xff);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glStencilMask(0x00);

  Matrix shadowProj = perspective(rad(90.0f), 1.0f, near_plane, far_plane); 

  vec3 negPos = { -lightsStore[shadowPointLightT][lightId].mat.m[12], -lightsStore[shadowPointLightT][lightId].mat.m[13], -lightsStore[shadowPointLightT][lightId].mat.m[14] };

  static const vec3 shadowRotation[6] = { {180.0f, 90.0f, 0.0f}, {180.0f, -90.0f, 0.0f}, {90.0f, 0.0f, 0.0f},
					  {-90.0f, 0.0f, 0.0f}, {180.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, 180.0f}};
	
  for (int i2 = 6 * lightId; i2 < 6 * (lightId+1); ++i2){	      
    Matrix viewMat = IDENTITY_MATRIX;

    translate(&viewMat, argVec3(negPos));

    rotateX(&viewMat, rad(shadowRotation[i2 - (6 * lightId)].x));
    rotateY(&viewMat, rad(shadowRotation[i2 - (6 * lightId)].y));
    rotateZ(&viewMat, rad(shadowRotation[i2 - (6 * lightId)].z));

    Matrix shadowTransforms = multiplymat4(viewMat, shadowProj);

    char buf[128];

    sprintf(buf, "shadowMatrices[%d]", i2);

    uniformMat4(pointShadowShader, buf, shadowTransforms.m);
  }

  glActiveTexture(GL_TEXTURE0);
  renderScene(pointShadowShader);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

  printf("%d first obj tx \n", loadedModels1D[0].tx);

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
  float texturedTileVerts[] = {
    bBlockW, 0.0f, bBlockD , 0.0f, 1.0f, -0.000000, 1.000000, -0.000000,
    0.0f, 0.0f, bBlockD , 1.0f, 1.0f, -0.000000, 1.000000, -0.000000, 
    bBlockW, 0.0f, 0.0f , 0.0f, 0.0f, -0.000000, 1.000000, -0.000000,
      
    0.0f, 0.0f, bBlockD , 1.0f, 1.0f, 0.000000, 1.000000, 0.000000,
    bBlockW, 0.0f, 0.0f , 0.0f, 0.0f, 0.000000, 1.000000, 0.000000,
    0.0f, 0.0f, 0.0f , 1.0f, 0.0f,  0.000000, 1.000000, 0.000000,
  };
  
  int* geomentyByTxCounter = calloc(loadedTexturesCounter, sizeof(int));
  const int vertexSize = 8;

  if (batchedGeometryIndexes) {
    for (int i = 0; i < batchedGeometryIndexesSize; i++) {
      if (batchedGeometryIndexes[i].wallsSize > 0) {
	free(batchedGeometryIndexes[i].wallsIndexes);
	batchedGeometryIndexes[i].wallsSize = 0;
      }

      if (batchedGeometryIndexes[i].jointsSize > 0) {
	free(batchedGeometryIndexes[i].jointsIndexes);
	batchedGeometryIndexes[i].jointsSize = 0;
      }
    }
  }

  navPointsSize=0;
  
  for (int y = 0; y < renderCapYLayer; y++) {
    for (int z = 0; z < gridZ; z++) {
      for (int x = 0; x < gridX; x++) {
	if(grid[y][z][x]){
	  bool batchedIndexWasAssigned = false;
	    
	  GroundType type = valueIn(grid[y][z][x]->ground, 0);

	  if(type == texturedTile){
	    int txIndex = valueIn(grid[y][z][x]->ground, 2);
	    
	    geomentyByTxCounter[txIndex] += sizeof(texturedTileVerts);

	    if(!grid[y][z][x]->block){
	      //navPointsSize++;
	    }
	    
	    if(!batchedIndexWasAssigned){
	      batchedGeometryIndexesSize++;
	      batchedIndexWasAssigned= true;

	    }
	  }

	  // block
	  if(grid[y][z][x]->block){
	    if(!batchedIndexWasAssigned){
	      batchedGeometryIndexesSize++;
	      batchedIndexWasAssigned= true;
	    }

	    TileBlocksTypes type = grid[y][z][x]->block->type;
	    int txIndex = grid[y][z][x]->block->txIndex;
	    
	    for(int i2=0;i2<blocksVPairs[type].planesNum;i2++){
	      //if(!grid[y][z][x]->blockplanes[i2].hide){
	      geomentyByTxCounter[txIndex] += blocksVPairs[type].pairs[i2].vertexNum * sizeof(float) * vertexSize;
	    }
	  }

	  // walls
	  for(int i=0;i<basicSideCounter;i++){
	    if(grid[y][z][x]->walls[i].planes){
	      if(!batchedIndexWasAssigned){
		batchedGeometryIndexesSize++;
		batchedIndexWasAssigned= true;
	      }
		
	      WallType type = grid[y][z][x]->walls[i].type;
	      
	      for(int i2=0;i2<wallsVPairs[type].planesNum;i2++){
		if(!grid[y][z][x]->walls[i].planes[i2].hide){
		  int txIndex = grid[y][z][x]->walls[i].planes[i2].txIndex;
		  geomentyByTxCounter[txIndex] += wallsVPairs[type].pairs[i2].vertexNum * sizeof(float) * vertexSize;
		}
	      }
	    }else{
	      GroundType type = valueIn(grid[y][z][x]->ground, 0);

	      if(type == texturedTile){
		navPointsSize++;
	      }
	    }

	    if(grid[y][z][x]->jointExist[i]){
	      if(!batchedIndexWasAssigned){
		batchedGeometryIndexesSize++;
		batchedIndexWasAssigned= true;
	      }
		
	      for(int i2=0;i2<wallsVPairs[wallJointT].planesNum;i2++){
		int txIndex = grid[y][z][x]->joint[i][i2].txIndex;
		geomentyByTxCounter[txIndex] += wallsVPairs[wallJointT].pairs[i2].vertexNum * sizeof(float) * vertexSize;
	      }
	    }
	  }



	  
	}
      }
    }
  }


  if(!navPoints){
    navPoints = malloc(sizeof(vec3) * navPointsSize);
  }else{
    navPoints = realloc(navPoints, sizeof(vec3) * navPointsSize);
  }

  navPointsSize = 0;

  printf("pre batchedGeometryIndexes: %d \n", batchedGeometryIndexesSize);

  if(!batchedGeometryIndexes){
    batchedGeometryIndexes = calloc(batchedGeometryIndexesSize, sizeof(BatchedTile));
  }else{
    
    
    batchedGeometryIndexes = realloc(batchedGeometryIndexes, sizeof(BatchedTile) * batchedGeometryIndexesSize);
    memset(batchedGeometryIndexes, 0, sizeof(BatchedTile) * batchedGeometryIndexesSize);
  }

  batchedGeometryIndexesSize = 0;
  

  for(int i=0;i<loadedTexturesCounter;i++){  
    if(geomentyByTxCounter[i] != 0){
      geometry[i].size = geomentyByTxCounter[i];

      if(geometry[i].verts){
	geometry[i].verts = realloc(geometry[i].verts, geomentyByTxCounter[i]);
      }else{
	geometry[i].verts = malloc(geomentyByTxCounter[i]);
      }
    }else{
      geometry[i].size = 0;
    }

    geometry[i].tris = geomentyByTxCounter[i] / vertexSize / sizeof(float);
    printf("Tris: %d Size %d \n", geometry[i].tris, geometry[i].size);
  }

  free(geomentyByTxCounter);
  geomentyByTxCounter = calloc(loadedTexturesCounter, sizeof(int));

  for (int y = 0; y < renderCapYLayer; y++) {
    for (int z = 0; z < gridZ; z++) {
      for (int x = 0; x < gridX; x++) {
	if (grid[y][z][x]) {
	  bool batchedIndexWasAssigned = false;
	  
	  GroundType type = valueIn(grid[y][z][x]->ground, 0);

	  if (type == texturedTile) {
	    //	    if(!batchedIndexWasAssigned){
	    batchedIndexWasAssigned = true;
	    batchedGeometryIndexes[batchedGeometryIndexesSize].indx = (vec3i){x,y,z}; 
	    //	      batchedGeometryIndexesSize++;
	    //	    }

	      
	    vec3 tile = xyz_indexesToCoords(x,y,z);

	    //	    if(!grid[y][z][x]->block){
	      //   navPoints[navPointsSize] = (vec3){tile.x+0.5f,tile.y,tile.z+0.5f};
	      //	      navPointsSize++;
	      //    }
	    
	    int txIndex = valueIn(grid[y][z][x]->ground, 2); 

	    for(int i=0;i<6*vertexSize;i+=vertexSize){
	      geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i] = texturedTileVerts[i] + tile.x; 
	      geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+1] = texturedTileVerts[i+1] + tile.y;
	      geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+2] = texturedTileVerts[i+2] + tile.z; 

	      geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+3] = texturedTileVerts[i+3];
	      geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+4] = texturedTileVerts[i+4];

	      geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+5] = texturedTileVerts[i+5];
	      geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+6] = texturedTileVerts[i+6];
	      geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+7] = texturedTileVerts[i+7];
	    }

	    geomentyByTxCounter[txIndex] += sizeof(texturedTileVerts) / sizeof(float);
	  }
	  
	  // block
	  if(grid[y][z][x]->block){
	    //	    if(!batchedIndexWasAssigned){
	    batchedIndexWasAssigned = true;
	    batchedGeometryIndexes[batchedGeometryIndexesSize].indx = (vec3i){x,y,z}; 
	    //	      batchedGeometryIndexesSize++;
	    //	    }
	      
	    TileBlocksTypes type = grid[y][z][x]->block->type;
	    int txIndex = grid[y][z][x]->block->txIndex;

	    for(int i2=0;i2<blocksVPairs[type].planesNum;i2++){
	      //	      if(!grid[y][z][x]->walls[i3].planes[i2].hide){
	      //int txIndex = grid[y][z][x]->block.planes[i2].txIndex;
		  
	      for(int i=0;i<blocksVPairs[type].pairs[i2].vertexNum * vertexSize;i+=vertexSize){
		vec4 vert = { blocksVPairs[type].pairs[i2].vBuf[i], blocksVPairs[type].pairs[i2].vBuf[i+1], blocksVPairs[type].pairs[i2].vBuf[i+2], 1.0f };

		vec4 transf = mulmatvec4(grid[y][z][x]->block->mat, vert);

		vec4 normal = { blocksVPairs[type].pairs[i2].vBuf[i+5], blocksVPairs[type].pairs[i2].vBuf[i+6], blocksVPairs[type].pairs[i2].vBuf[i+7], 1.0f };

		Matrix inversedWallModel = IDENTITY_MATRIX;
		inverse(grid[y][z][x]->block->mat.m, inversedWallModel.m);

		Matrix trasposedAndInversedWallModel = IDENTITY_MATRIX;
		mat4transpose(trasposedAndInversedWallModel.m, inversedWallModel.m);
		  
		vec4 transfNormal = mulmatvec4(trasposedAndInversedWallModel, normal);

		geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i] = transf.x; 
		geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+1] = transf.y;
		geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+2] = transf.z;

		geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+3] = blocksVPairs[type].pairs[i2].vBuf[i+3];
		geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+4] = blocksVPairs[type].pairs[i2].vBuf[i+4];
		    
		geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+5] = transfNormal.x;
		geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+6] = transfNormal.y;
		geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+7] = transfNormal.z;
	      }

	      geomentyByTxCounter[txIndex] += blocksVPairs[type].pairs[i2].vertexNum * vertexSize;
	      //}
	    }
	  }

	  // walls
	  for(int i3=0;i3<basicSideCounter;i3++){
          if (grid[y][z][x]->walls[i3].planes) {
              // remember wall that exist
              {
                  batchedGeometryIndexes[batchedGeometryIndexesSize].wallsSize++;
                  int newSize = batchedGeometryIndexes[batchedGeometryIndexesSize].wallsSize;

                  if (!batchedGeometryIndexes[batchedGeometryIndexesSize].wallsIndexes) {
                      batchedGeometryIndexes[batchedGeometryIndexesSize].wallsIndexes = malloc(sizeof(uint8_t));
                  }
                  else {
                      batchedGeometryIndexes[batchedGeometryIndexesSize].wallsIndexes = realloc(batchedGeometryIndexes[batchedGeometryIndexesSize].wallsIndexes, sizeof(uint8_t) * newSize);
                  }

                  batchedGeometryIndexes[batchedGeometryIndexesSize].wallsIndexes[newSize - 1] = i3;
              }

              //	      if(!batchedIndexWasAssigned){
              batchedIndexWasAssigned = true;
              batchedGeometryIndexes[batchedGeometryIndexesSize].indx = (vec3i){ x,y,z };

              //		batchedGeometryIndexesSize++;
              //	      }

              WallType type = grid[y][z][x]->walls[i3].type;

              for (int i2 = 0; i2 < wallsVPairs[type].planesNum; i2++) {
                  if (!grid[y][z][x]->walls[i3].planes[i2].hide) {
                      int txIndex = grid[y][z][x]->walls[i3].planes[i2].txIndex;

                      for (int i = 0; i < wallsVPairs[type].pairs[i2].vertexNum * vertexSize; i += vertexSize) {
                          vec4 vert = { wallsVPairs[type].pairs[i2].vBuf[i], wallsVPairs[type].pairs[i2].vBuf[i + 1], wallsVPairs[type].pairs[i2].vBuf[i + 2], 1.0f };

                          vec4 transf = mulmatvec4(grid[y][z][x]->walls[i3].mat, vert);

                          vec4 normal = { wallsVPairs[type].pairs[i2].vBuf[i + 5], wallsVPairs[type].pairs[i2].vBuf[i + 6], wallsVPairs[type].pairs[i2].vBuf[i + 7], 1.0f };

                          Matrix inversedWallModel = IDENTITY_MATRIX;
                          inverse(grid[y][z][x]->walls[i3].mat.m, inversedWallModel.m);

                          Matrix trasposedAndInversedWallModel = IDENTITY_MATRIX;
                          mat4transpose(trasposedAndInversedWallModel.m, inversedWallModel.m);

                          vec4 transfNormal = mulmatvec4(trasposedAndInversedWallModel, normal);

                          geometry[txIndex].verts[geomentyByTxCounter[txIndex] + i] = transf.x;
                          geometry[txIndex].verts[geomentyByTxCounter[txIndex] + i + 1] = transf.y;
                          geometry[txIndex].verts[geomentyByTxCounter[txIndex] + i + 2] = transf.z;

                          geometry[txIndex].verts[geomentyByTxCounter[txIndex] + i + 3] = wallsVPairs[type].pairs[i2].vBuf[i + 3];
                          geometry[txIndex].verts[geomentyByTxCounter[txIndex] + i + 4] = wallsVPairs[type].pairs[i2].vBuf[i + 4];

                          geometry[txIndex].verts[geomentyByTxCounter[txIndex] + i + 5] = transfNormal.x;
                          geometry[txIndex].verts[geomentyByTxCounter[txIndex] + i + 6] = transfNormal.y;
                          geometry[txIndex].verts[geomentyByTxCounter[txIndex] + i + 7] = transfNormal.z;
                      }

                      geomentyByTxCounter[txIndex] += wallsVPairs[type].pairs[i2].vertexNum * vertexSize;
                  }
              }
          }

	  
	  GroundType type = valueIn(grid[y][z][x]->ground, 0);

	  if(!grid[y][z][x]->walls[i3].planes && type == texturedTile){
	    static const vec2 paddd[4] = {
	      [top] = {0.5f,0.0f}, [left] = { 1.0f, 0.5f }, [right] = { 0.0f, 0.5f }, [bot] = { 0.5f, 1.0f } 
	    };

	    vec3 tile = xyz_indexesToCoords(x, y, z);

	    navPoints[navPointsSize] = (vec3){tile.x+paddd[i3].x,tile.y,tile.z+paddd[i3].z};
	    navPointsSize++;
	  }

	  if(grid[y][z][x]->jointExist[i3]){
	      // remember wall that exist
	      {
                batchedGeometryIndexes[batchedGeometryIndexesSize].jointsSize++;
		int newSize = batchedGeometryIndexes[batchedGeometryIndexesSize].jointsSize;

		if(!batchedGeometryIndexes[batchedGeometryIndexesSize].jointsIndexes){
		  batchedGeometryIndexes[batchedGeometryIndexesSize].jointsIndexes = malloc(sizeof(uint8_t));
		}else{
		  batchedGeometryIndexes[batchedGeometryIndexesSize].jointsIndexes = realloc(batchedGeometryIndexes[batchedGeometryIndexesSize].jointsIndexes, sizeof(uint8_t) * newSize);
		}
		
		batchedGeometryIndexes[batchedGeometryIndexesSize].jointsIndexes[newSize-1] = i3;
	      }
	      
	      //	      if(!batchedIndexWasAssigned){
	      batchedIndexWasAssigned = true;
	      batchedGeometryIndexes[batchedGeometryIndexesSize].indx = (vec3i){x,y,z}; 
	      //		batchedGeometryIndexesSize++;
	      //	      }
		
	      for(int i2=0;i2<wallsVPairs[wallJointT].planesNum;i2++){
		int txIndex = grid[y][z][x]->joint[i3][i2].txIndex;
		
		for(int i=0;i<wallsVPairs[wallJointT].pairs[i2].vertexNum * vertexSize;i+=vertexSize){
		  vec4 vert = { wallsVPairs[wallJointT].pairs[i2].vBuf[i], wallsVPairs[wallJointT].pairs[i2].vBuf[i+1], wallsVPairs[wallJointT].pairs[i2].vBuf[i+2], 1.0f };

		  vec4 transf = mulmatvec4(grid[y][z][x]->jointsMat[i3], vert);

		  vec4 normal = { wallsVPairs[wallJointT].pairs[i2].vBuf[i+5], wallsVPairs[wallJointT].pairs[i2].vBuf[i+6], wallsVPairs[wallJointT].pairs[i2].vBuf[i+7], 1.0f };

		  Matrix inversedWallModel = IDENTITY_MATRIX;
		  inverse(grid[y][z][x]->jointsMat[i3].m, inversedWallModel.m);

		  Matrix trasposedAndInversedWallModel = IDENTITY_MATRIX;
		  mat4transpose(trasposedAndInversedWallModel.m, inversedWallModel.m);
		  
		  vec4 transfNormal = mulmatvec4(trasposedAndInversedWallModel, normal);

		  geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i] = transf.x; 
		  geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+1] = transf.y;
		  geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+2] = transf.z;

		  geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+3] = wallsVPairs[wallJointT].pairs[i2].vBuf[i+3];
		  geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+4] = wallsVPairs[wallJointT].pairs[i2].vBuf[i+4];

		  
		  geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+5] = transfNormal.x;
		  geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+6] = transfNormal.y;
		  geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+7] = transfNormal.z;
		}
		
		geomentyByTxCounter[txIndex] += wallsVPairs[wallJointT].pairs[i2].vertexNum * vertexSize;
	      }
	    }
	  }

	  if(batchedIndexWasAssigned){
	    batchedGeometryIndexesSize++;
	  }
	}
	
      }
    }
  }

  for(int i=0;i<loadedTexturesCounter;i++){
    glBindVertexArray(geometry[i].pairs.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, geometry[i].pairs.VBO);
  
    glBufferData(GL_ARRAY_BUFFER, geometry[i].size, geometry[i].verts, GL_STATIC_DRAW);
  
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  printf("post batchedGeometryIndexesSize: %d (%d(%d) bytes) \n", batchedGeometryIndexesSize, batchedGeometryIndexesSize * sizeof(Tile), sizeof(Tile));
    
  //    for(int i=0;i<batchedGeometryIndexesSize;i++){
  //      printf("%d %d %d \n",argVec3(batchedGeometryIndexes[i]));
  //   }

  free(geomentyByTxCounter);
  
  if(!navPointsMesh.vBuf){
    navPointsMesh.vBuf = malloc(sizeof(float) * 3 * cube.vertexNum * navPointsSize);
  }else{
    navPointsMesh.vBuf = realloc(navPointsMesh.vBuf,sizeof(float) * 3 * cube.vertexNum * navPointsSize);
  }

  // asseble navPointsMesh
  int index = 0;
  for(int i=0;i<navPointsSize*cube.vertexNum*3;i+=cube.vertexNum*3){
    for(int i2=0;i2<cube.vertexNum*3;i2+=3){
      navPointsMesh.vBuf[i + i2 + 0] = cube.vBuf[i2 + 0] + (float)navPoints[index].x;
      navPointsMesh.vBuf[i + i2 + 1] = cube.vBuf[i2 + 1] + (float)navPoints[index].y;
      navPointsMesh.vBuf[i + i2 + 2] = cube.vBuf[i2 + 2] + (float)navPoints[index].z;

      //   printf("%f %f %f \n", navPointsMesh.vBuf[i + i2 + 0], navPointsMesh.vBuf[i + i2 + 1], navPointsMesh.vBuf[i + i2 + 2]);
    }
    
    index++;
  }

  glGenBuffers(1, &navPointsMesh.VBO);
  glGenVertexArrays(1, &navPointsMesh.VAO);

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

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 
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

    glBindBuffer(GL_ARRAY_BUFFER, geometry[i].pairs.VBO);
    glBindVertexArray(geometry[i].pairs.VAO);

    glDrawArrays(GL_TRIANGLES, 0, geometry[i].tris);

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
    if (bBlock->block != NULL) {
      float intersectionDistance;
      bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, bBlock->block->lb, bBlock->block->rt, NULL, &intersectionDistance);

      if (isIntersect && minDistToCamera > intersectionDistance) {
	mouse.selectedThing = bBlock->block;
	mouse.selectedType = mouseBlockT;

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
	intersTileData->tile = bBlock;

	intersTileData->grid = (vec2i){ ind.x,ind.z };
	intersTileData->intersection = intersection;
	intersTileData->groundInter = intersection.y <= curCamera->pos.y ? fromOver : fromUnder;

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
	  
	  WallType type = bBlock->walls[wallIndex].type;
	  
	  float intersectionDistance;

	  for (int i3 = 0; i3 < wallsVPairs[type].planesNum; i3++) {
	    bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, bBlock->walls[wallIndex].planes[i3].lb, bBlock->walls[wallIndex].planes[i3].rt, NULL, &intersectionDistance);

	    if (isIntersect && minDistToCamera > intersectionDistance) {
	      intersWallData->side = wallIndex;
	      intersWallData->grid = ind;

	      int tx = bBlock->walls[wallIndex].planes[i3].txIndex;

	      intersWallData->txIndex = tx;
	      intersWallData->tile = bBlock;

	      intersWallData->type = type;
	      intersWallData->plane = i3;

	      mouse.selectedType = mouseWallT;
	      mouse.selectedThing = intersWallData;
	      
	      minDistToCamera = intersectionDistance;
	    }
	  }
	}
      }

      // joints
      {
	for (int i2 = 0; i2 < batchedGeometryIndexes[i].jointsSize; i2++) {
	  int jointIndex = batchedGeometryIndexes[i].jointsIndexes[i2];
	  float intersectionDistance;
	    
	  for (int i3 = 0; i3 < jointPlaneCounter; i3++) {
	    bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, bBlock->joint[jointIndex][i3].lb, bBlock->joint[jointIndex][i3].rt, NULL, &intersectionDistance);

	    if (isIntersect && minDistToCamera > intersectionDistance) {

	      intersWallData->side = jointIndex;
	      intersWallData->grid = ind;
	      intersWallData->txIndex = bBlock->joint[jointIndex][i3].txIndex;
	      intersWallData->tile = bBlock;

	      intersWallData->type = wallJointT;
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
    for (int i = 0; i < lightsStoreSizeByType[i2]; i++) {
      float intersectionDistance;

      bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, lightsStore[i2][i].lb, lightsStore[i2][i].rt, NULL, &intersectionDistance);

      if (isIntersect && minDistToCamera > intersectionDistance) {
	mouse.selectedThing = &lightsStore[i2][i];
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
	intersTileData->tile = grid[curFloor][gridInd.z][gridInd.x];

	intersTileData->grid = (vec2i){ gridInd.x, gridInd.z };
	intersTileData->intersection = intersection;
	intersTileData->groundInter = intersection.y <= curCamera->pos.y ? fromOver : fromUnder;

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
