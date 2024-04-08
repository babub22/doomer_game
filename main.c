#include "deps.h"
#include "linearAlg.h"
#include "main.h"

GLuint textVBO;
GLuint textVAO;

int modelLoc;
int lightModelLoc;

VPair tileOver;

bool hints = true;

float* tiles;
size_t tilesTris;
size_t tileMatsBuf; // buf id

int gTilesCounter;
Wall2* walls;
int gWallsCounter;

VPair* batchTextures;

char curSaveName[CONSOLE_BUF_CAP];

Texture* loadedTextures1D;
Texture** loadedTextures2D;
char** loadedTexturesNames; // iter same as tex1D
int loadedTexturesCategoryCounter;
Category* loadedTexturesCategories; 
int loadedTexturesCounter;
int longestTextureNameLen;
int longestTextureCategoryLen;

BlockInfo wallsVPairs[wallTypeCounter];

GLuint selectionRectVBO;
GLuint selectionRectVAO;

GLuint solidColorTx;
GLuint errorTx;
GLuint emptyTx;

VPair hudRect;

VPair planePairs;

Object** objsStore;
size_t objsStoreSize;

GLuint fontAtlas;

Character* characters;
size_t charactersSize;

char contextBelowText[500];
float contextBelowTextH;

// avaible/loaded models
ModelInfo* loadedModels1D;
ModelInfo** loadedModels2D;
size_t loadedModelsSize;

TextInput* selectedTextInput;
char tempTextInputStorage[512];
int tempTextInputStorageCursor;

TextInput dialogEditorNameInput;

VPair roofBlock;

VPair customWallV;
float* customWallTemp[4];

VPair stepsBlock;

// placed/created models
Model* curModels;
size_t curModelsSize;

Tile**** grid;
int gridX = 120;
int gridY = 15;
int gridZ = 120;

const float letterCellW = .04f;
const float letterCellH = .07f;

const float letterW = .04f / 1.9f;
const float letterH = .07f;

Camera camera1 = { .target={ 0.0f, 0.0f, 0.0f }, .yaw = -90.0f };
Camera camera2 = { .target={ 0.0f, 0.0f, 0.0f }, .pitch = -14.0f, .yaw = -130.0f };

Camera* curCamera = &camera1;
Mouse mouse;

Particle* snowParticle;
int snowAmount;
float snowSpeed;
float fov;

float borderArea;

VPair cube;

GLuint objectsMenuTypeRectVBO;
GLuint objectsMenuTypeRectVAO;

Menu dialogViewer= { .type = dialogViewerT };
Menu objectsMenu = { .type = objectsMenuT };
Menu dialogEditor = { .type = dialogEditorT };
Menu blocksMenu = { .type = blocksMenuT };
Menu texturesMenu = { .type = texturesMenuT };
Menu planeCreatorMenu = { .type = planeCreatorT };

Picture* planeOnCreation;

int texturesMenuCurCategoryIndex = 0;

int curFloor;

Picture* createdPlanes;
int createdPlanesSize;

Menu* curMenu;

int* dialogEditorHistory;
int dialogEditorHistoryLen;
int dialogEditorHistoryCursor;

ModelType objectsMenuSelectedType = objectModelType;
float objectsMenuWidth = -1.0f + 1.0f / 4.0f;

int consoleBufferCursor;
char consoleBuffer[CONSOLE_BUF_CAP];
bool consoleHasResponse;
char consoleResponse[CONSOLE_BUF_CAP * 5];
Menu console;
float consoleH = 1.0f - (1.0f * .05f);

EnviromentalConfig enviromental = { true, true };

const float wallD = 0.05f;

const Sizes wallsSizes[wallTypeCounter+1] = { {0}, {bBlockW * 1,bBlockH * 1,bBlockD * 1}, {bBlockW * 1,bBlockH * 1 * 0.4f,bBlockD * 1}, {bBlockW * 1,bBlockH * 1,bBlockD * 1} };

const float doorH = bBlockH * 0.85f;
const float doorPad =  bBlockW / 4;

const float doorTopPad = bBlockH - bBlockH * 0.85f;

float zNear = 0.075f;

float drawDistance;

const float windowW = 1280.0f;
const float windowH = 720.0f;

float INCREASER = 1.0f;

float tangFOV = 0.0f;

MeshBuffer** wallMeshes;

// if will be more than halfWall type that has different height
// from other walls make them Meshbuffer**
MeshBuffer* wallHighlight;
MeshBuffer* halfWallHighlight;

MeshBuffer* tileMeshes;
MeshBuffer* tileHighlight;

GLuint mainShader;
GLuint hudShader;
GLuint lightSourceShader;
  
GLuint cursorVBO;
GLuint cursorVAO;

int texture1DIndexByName(char* txName){
  for(int i=0;i<loadedTexturesCounter;i++){
    if(strcmp(txName, loadedTexturesNames[i]) == 0){
      return i;
    }
  }

  return -1;
};

int main(int argc, char* argv[]) {
  borderArea = (float)bBlockW/8;
  
  SDL_Init(SDL_INIT_VIDEO);

  char windowTitle[100] = game;
  SDL_Window* window = SDL_CreateWindow(windowTitle,
					SDL_WINDOWPOS_CENTERED,
					SDL_WINDOWPOS_CENTERED,
					windowW, windowH,
					SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
					);
  // | SDL_WINDOW_RESIZABLE

  SDL_WarpMouseInWindow(window, windowW/2.0f, windowH/2.0f);
  SDL_SetRelativeMouseMode(SDL_TRUE);
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
    glGenBuffers(1, &cursorVBO);
    glGenVertexArrays(1, &cursorVAO);
    
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

  // for cubes
  {
    glGenBuffers(1, &cube.VBO);
    glGenVertexArrays(1, &cube.VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  assembleWallBlockVBO();
  assembleWindowBlockVBO();
  assembleDoorBlockVBO();
  assembleWallJointVBO();

  // plane 3d
  {
    glGenBuffers(1, &planePairs.VBO);
    glGenVertexArrays(1, &planePairs.VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  // custom walls
  {
    glGenBuffers(1, &customWallV.VBO);
    glGenVertexArrays(1, &customWallV.VAO);

    
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

      float* editorPoints = uiRectPercentage(f(1/8), f(7/8), f(7/8), f(1/8));

      dialogEditor.rect = (UIRect){ editorPoints[0], editorPoints[1], editorPoints[20] - editorPoints[0], editorPoints[21] - editorPoints[1]};

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
    dialogEditor.buttons = malloc(dialogEditorButtonsCounter *  sizeof(UIRect));
    
    // dialog nameInput editor
    { 
      glGenVertexArrays(1, &dialogEditor.vpairs[charNameInput].VAO);
      glBindVertexArray(dialogEditor.vpairs[charNameInput].VAO);

      glGenBuffers(1, &dialogEditor.vpairs[charNameInput].VBO);
      glBindBuffer(GL_ARRAY_BUFFER, dialogEditor.vpairs[charNameInput].VBO);
      
      float* nameInput = uiRectPoints(dialogEditor.rect.x + letterCellW * (strlen(dialogEditorCharNameTitle) + 1), dialogEditor.rect.y - letterH / 2, 33 * letterW, letterH);
      
      dialogEditor.textInputs[charNameInput].rect = (UIRect){ dialogEditor.rect.x + letterCellW * (strlen(dialogEditorCharNameTitle) + 1), dialogEditor.rect.y - letterH / 2, 33 * letterW, letterH};

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
      
      dialogEditor.textInputs[replicaInput].rect = (UIRect){ dialogEditor.rect.x + 0.01f, dialogEditor.rect.y - 0.25f, letterCellW * 37, letterCellH * 3 + 0.05f};

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
      
      for(int i=answerInput1;i<dialogEditorInputsCounter;i++){
	// next buttons
	{
	  int nextButIndex = (i - 2) + nextButton1;
	  
	  glGenVertexArrays(1, &dialogEditor.buttonsPairs[nextButIndex].VAO);
	  glBindVertexArray(dialogEditor.buttonsPairs[nextButIndex].VAO);

	  glGenBuffers(1, &dialogEditor.buttonsPairs[nextButIndex].VBO);
	  glBindBuffer(GL_ARRAY_BUFFER, dialogEditor.buttonsPairs[nextButIndex].VBO);

	  float* buttonPoints = uiRectPoints(baseX + answerInputW + 0.02f + letterCellW, baseY - ((i-1) * (answerInputH + 0.03f)), letterCellW, answerInputH);

	  dialogEditor.buttons[nextButIndex] = (UIRect){ baseX + answerInputW + 0.02f + letterCellW, baseY - ((i-1) * (answerInputH + 0.03f)), letterCellW, answerInputH };
      
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

	  float* buttonPoints = uiRectPoints(baseX + answerInputW + 0.01f, baseY - ((i-1) * (answerInputH + 0.03f)), letterCellW, answerInputH);

	  dialogEditor.buttons[minusButIndex] = (UIRect){baseX + answerInputW + 0.01f, baseY - ((i-1) * (answerInputH + 0.03f)), letterCellW, answerInputH };
      
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
	if(addButIndex != addButton5+1)
	  {
	    glGenVertexArrays(1, &dialogEditor.buttonsPairs[addButIndex].VAO);
	    glBindVertexArray(dialogEditor.buttonsPairs[addButIndex].VAO);

	    glGenBuffers(1, &dialogEditor.buttonsPairs[addButIndex].VBO);
	    glBindBuffer(GL_ARRAY_BUFFER, dialogEditor.buttonsPairs[addButIndex].VBO);

	    float* buttonPoints = uiRectPoints(baseX + 0.02f, baseY - letterCellH - ((i-1) * (answerInputH + 0.03f)) - 0.02f, letterCellW, answerInputH);

	    dialogEditor.buttons[addButIndex] = (UIRect){ baseX + 0.02f, baseY - letterCellH - ((i-1) * (answerInputH + 0.03f)) - 0.02f, letterCellW, answerInputH };
      
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

	  float* answerInpt = uiRectPoints(baseX, baseY - ((i-1) * (answerInputH + 0.03f)), answerInputW, answerInputH);

	  dialogEditor.textInputs[i].rect = (UIRect){baseX, baseY - ((i-1) * (answerInputH + 0.03f)), answerInputW, answerInputH };

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
    //    dialogViewer.buttonsPairs = malloc(sizeof(UIRect) * dialogEditorButtonsCounter);
    
    // dialog viewer background

    {
      glGenVertexArrays(1, &dialogViewer.VAO);
      glBindVertexArray(dialogViewer.VAO);

      glGenBuffers(1, &dialogViewer.VBO);
      glBindBuffer(GL_ARRAY_BUFFER, dialogViewer.VBO);

      float xLeftPad = .025f;

      float* dialogViewerPoints = uiRectPoints(dialogEditor.rect.x - xLeftPad, -0.1f - 0.01f , dialogEditor.rect.w + .03f, (6*letterCellH + 0.02f) + 0.05f + 5 * letterCellH);

      dialogViewer.rect = (UIRect){ dialogEditor.rect.x - xLeftPad, -0.1f - 0.01f , dialogEditor.rect.w + .03f, (6*letterCellH + 0.02f) + 0.05f + 5 * letterCellH };

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
      float baseY = - (0.02f + 6 * letterCellH);

      for(int i=0;i<answerBut6+1;i++){
	//	glGenVertexArrays(1, &dialogViewer.buttonsPairs[i].VAO);
	//	glBindVertexArray(dialogViewer.buttonsPairs[i].VAO);

	//	glGenBuffers(1, &dialogViewer.buttonsPairs[i].VBO);
	//	glBindBuffer(GL_ARRAY_BUFFER, dialogViewer.buttonsPairs[i].VBO);

	float* answerButton = uiRectPoints(dialogViewer.rect.x + 0.03f ,baseY - (i*(letterCellH + 0.01f)), dialogEditor.textInputs[answerInput1].rect.w, letterCellH);

	dialogViewer.buttons[i] = (UIRect){ dialogViewer.rect.x + 0.03f ,baseY - (i*(letterCellH + 0.01f)), dialogEditor.textInputs[answerInput1].rect.w, letterCellH };

	//	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (6 * 4), answerButton, GL_STATIC_DRAW);
	free(answerButton);
      
	/*glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	  glEnableVertexAttribArray(0);

	  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
	  glEnableVertexAttribArray(1);*/
      }

      //      glBindBuffer(GL_ARRAY_BUFFER, 0);
      //      glBindVertexArray(0);
    }
    
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

    // bojectsMenu
    {

      glGenVertexArrays(1, &objectsMenu.VAO);
      glBindVertexArray(objectsMenu.VAO);

      glGenBuffers(1, &objectsMenu.VBO);
      glBindBuffer(GL_ARRAY_BUFFER, objectsMenu.VBO);

    
      float menuPoints[] = {
	-1.0f, 1.0f, 1.0f, 0.0f,
	objectsMenuWidth, 1.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 0.0f,

	objectsMenuWidth, 1.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 0.0f,
	objectsMenuWidth, -1.0f, 0.0f, 1.0f };

      glBufferData(GL_ARRAY_BUFFER, sizeof(menuPoints), menuPoints, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }

    // top/bot mouse
    glGenVertexArrays(1, &netTileVAO);
    glBindVertexArray(netTileVAO);

    glGenBuffers(1, &netTileVBO);
    glBindBuffer(GL_ARRAY_BUFFER, netTileVBO);

    const vec3 c0 = { 0.0f, 0.0f, 0.0f };
    const vec3 c1 = { bBlockW, 0.0f, 0.0f };
    const vec3 c3 = { bBlockW, 0.0f, bBlockD };
    const vec3 c2 = { 0.0f, 0.0f, bBlockD };
    
    float netTileVerts[] = {
      argVec3(c0), 
      argVec3(c1), 
      argVec3(c0),
      argVec3(c2),       
      argVec3(c2),
      argVec3(c3),
      argVec3(c1),
      argVec3(c3) };

    glBufferData(GL_ARRAY_BUFFER, sizeof(netTileVerts), netTileVerts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // highlight tiles
    tileHighlight = malloc(2 * sizeof(MeshBuffer));
    
    for(int i=1; i<=2;i++){
      glGenVertexArrays(1, &tileHighlight[i-1].VAO);
      glBindVertexArray(tileHighlight[i-1].VAO);

      glGenBuffers(1, &tileHighlight[i-1].VBO);
      glBindBuffer(GL_ARRAY_BUFFER, tileHighlight[i-1].VBO);


      if(i == fromUnder){
	float tileHighlighting[] = {
	  // right
	  0.0f + wallsSizes[wallT].w, -wallD - selTileBorderH, 0.0f + wallsSizes[wallT].d - selBorderT,
	  0.0f + wallsSizes[wallT].w, -wallD - selTileBorderH, 0.0f + selBorderT,
	  0.0f + wallsSizes[wallT].w - selBorderT, -wallD - selTileBorderH, 0.0f + selBorderT,

	  0.0f + wallsSizes[wallT].w, -wallD - selTileBorderH, 0.0f + wallsSizes[wallT].d - selBorderT,
	  0.0f + wallsSizes[wallT].w - selBorderT, -wallD - selTileBorderH, 0.0f + wallsSizes[wallT].d - selBorderT,
	  0.0f + wallsSizes[wallT].w - selBorderT, -wallD - selTileBorderH, 0.0f + selBorderT,

	  // left
	  0.0f + selBorderT, -wallD - selTileBorderH, 0.0f + wallsSizes[wallT].d - selBorderT,
	  0.0f + selBorderT, -wallD - selTileBorderH, 0.0f + selBorderT,
	  0.0f, -wallD - selTileBorderH, 0.0f + selBorderT,

	  0.0f + selBorderT, -wallD - selTileBorderH, 0.0f + wallsSizes[wallT].d - selBorderT,
	  0.0f, -wallD - selTileBorderH, 0.0f + wallsSizes[wallT].d - selBorderT,
	  0.0f, -wallD - selTileBorderH, 0.0f + selBorderT,

	  // bot
	  0.0f + wallsSizes[wallT].w, -wallD - selTileBorderH, 0.0f + wallsSizes[wallT].d,
	  0.0f, -wallD - selTileBorderH, 0.0f + wallsSizes[wallT].d,
	  0.0f + wallsSizes[wallT].w, -wallD - selTileBorderH, 0.0f + wallsSizes[wallT].d - selBorderT,

	  0.0f, -wallD - selTileBorderH, 0.0f + wallsSizes[wallT].d,
	  0.0f, -wallD - selTileBorderH, 0.0f + wallsSizes[wallT].d - selBorderT,
	  0.0f + wallsSizes[wallT].w, -wallD - selTileBorderH, 0.0f + wallsSizes[wallT].d - selBorderT,

	  // top
	  0.0f + wallsSizes[wallT].w, -wallD - selTileBorderH, 0.0f + selBorderT,
	  0.0f, -wallD - selTileBorderH, 0.0f + selBorderT,
	  0.0f, -wallD - selTileBorderH, 0.0f,

	  0.0f + wallsSizes[wallT].w, -wallD - selTileBorderH, 0.0f + selBorderT,
	  0.0f, -wallD - selTileBorderH, 0.0f,
	  0.0f + wallsSizes[wallT].w, -wallD - selTileBorderH, 0.0f,
	};

	tileHighlight[i-1].VBOsize = 6 * 4;
	glBufferData(GL_ARRAY_BUFFER, sizeof(tileHighlighting), tileHighlighting, GL_STATIC_DRAW);
      }else if(i == fromOver){
	float tileHighlighting[] = {
	  // right
	  0.0f + wallsSizes[wallT].w, wallD + selTileBorderH, 0.0f + wallsSizes[wallT].d - selBorderT,
	  0.0f + wallsSizes[wallT].w, wallD + selTileBorderH, 0.0f + selBorderT,
	  0.0f + wallsSizes[wallT].w - selBorderT, 	wallD + selTileBorderH, 0.0f + selBorderT,

	  0.0f + wallsSizes[wallT].w, 	wallD + selTileBorderH, 0.0f + wallsSizes[wallT].d - selBorderT,
	  0.0f + wallsSizes[wallT].w - selBorderT, 	wallD + selTileBorderH, 0.0f + wallsSizes[wallT].d - selBorderT,
	  0.0f + wallsSizes[wallT].w - selBorderT, 	wallD + selTileBorderH, 0.0f + selBorderT,

	  // left
	  0.0f + selBorderT, wallD + selTileBorderH, 0.0f + wallsSizes[wallT].d - selBorderT,
	  0.0f + selBorderT, wallD + selTileBorderH, 0.0f + selBorderT,
	  0.0f, wallD + selTileBorderH, 0.0f + selBorderT,

	  0.0f + selBorderT, 	wallD + selTileBorderH, 0.0f + wallsSizes[wallT].d - selBorderT,
	  0.0f, wallD + selTileBorderH, 0.0f + wallsSizes[wallT].d - selBorderT,
	  0.0f, wallD + selTileBorderH, 0.0f + selBorderT,

	  // bot
	  0.0f + wallsSizes[wallT].w, wallD + selTileBorderH, 0.0f + wallsSizes[wallT].d,
	  0.0f, wallD + selTileBorderH, 0.0f + wallsSizes[wallT].d,
	  0.0f + wallsSizes[wallT].w, wallD + selTileBorderH, 0.0f + wallsSizes[wallT].d - selBorderT,

	  0.0f, wallD + selTileBorderH, 0.0f + wallsSizes[wallT].d,
	  0.0f, wallD + selTileBorderH, 0.0f + wallsSizes[wallT].d - selBorderT,
	  0.0f + wallsSizes[wallT].w, wallD + selTileBorderH, 0.0f + wallsSizes[wallT].d - selBorderT,

	  // top
	  0.0f + wallsSizes[wallT].w, wallD + selTileBorderH, 0.0f + selBorderT,
	  0.0f, wallD + selTileBorderH, 0.0f + selBorderT,
	  0.0f, wallD + selTileBorderH, 0.0f,

	  0.0f + wallsSizes[wallT].w, wallD + selTileBorderH, 0.0f + selBorderT,
	  0.0f, wallD + selTileBorderH, 0.0f,
	  0.0f + wallsSizes[wallT].w, wallD + selTileBorderH, 0.0f,
	};

	tileHighlight[i-1].VBOsize = 6 * 4;
	glBufferData(GL_ARRAY_BUFFER, sizeof(tileHighlighting), tileHighlighting, GL_STATIC_DRAW);
      }

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }

    // roof
    {
      if(!tileBlocksTempl) {
	tileBlocksTempl = malloc(3 * sizeof(TileBlock));
      }else{
	//	tileBlocksTempl = realloc(tileBlocksTemplSize, sizeof(TileBlock) * tileBlocksTemplSize);
      }
      
      tileBlocksTemplSize+= 3;
      
      
      glGenVertexArrays(1, &tileBlocksTempl[0].vpair.VAO);
      glBindVertexArray(tileBlocksTempl[0].vpair.VAO);

      glGenBuffers(1, &tileBlocksTempl[0].vpair.VBO);
      glBindBuffer(GL_ARRAY_BUFFER, tileBlocksTempl[0].vpair.VBO);

      float t = (float)1 / 8;
      float capRatio = 0.12f;
      
      float roofBlock[] = {
	// main part
	0.0f, floorH, 0.0f,        0.0f, 1.0f,
	bBlockW, floorH, 0.0f ,    1.0f, 1.0f,
	0.0f, 0.0f, bBlockD + t,   0.0f, 0.0f,

	bBlockW, floorH, 0.0f ,      1.0f, 1.0f,
	0.0f, 0.0f, bBlockD + t,     0.0f, 0.0f,
	bBlockW, 0.0f, bBlockD + t,  1.0f, 0.0f,

	// cap
	0.0f, 0.0f, bBlockD + t,         0.0f, 0.0f,
	bBlockW, 0.0f, bBlockD + t,      1.0f, 0.0f,
	0.0f, -t, bBlockD + t + t,       0.0f, capRatio,

	bBlockW, 0.0f, bBlockD + t,      1.0f, 0.0f,
	0.0f, -t, bBlockD + t + t,       0.0f, capRatio,
	bBlockW, -t,  bBlockD + t + t,   1.0f, capRatio,



	/*
	// left
	0.0f, floorH, 0.0f,   0.0f, 1.0f,
	0.0f, 0.0f, bBlockD + t , 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f ,    1.0f, 1.0f,
	
	// right
	bBlockW, floorH, 0.0f,   0.0f, 1.0f,
	bBlockW, 0.0f, bBlockD + t , 0.0f, 0.0f,
	bBlockW, 0.0f, 0.0f ,    1.0f, 1.0f,*/
	
	// front
      };
      
      tileBlocksTempl[0].vertexes = malloc(sizeof(roofBlock));
      memcpy(tileBlocksTempl[0].vertexes, roofBlock, sizeof(roofBlock));
      tileBlocksTempl[0].vertexesSize = 6 + 6;
      tileBlocksTempl[0].type = roofBlockT;

      // steps
      glGenVertexArrays(1, &tileBlocksTempl[1].vpair.VAO);
      glBindVertexArray(tileBlocksTempl[1].vpair.VAO);

      glGenBuffers(1, &tileBlocksTempl[1].vpair.VBO);
      glBindBuffer(GL_ARRAY_BUFFER, tileBlocksTempl[1].vpair.VBO);

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

      tileBlocksTempl[1].vertexes = malloc(sizeof(texturedTileVerts));
      memcpy(tileBlocksTempl[1].vertexes, texturedTileVerts, sizeof(texturedTileVerts));
      tileBlocksTempl[1].vertexesSize = 8 * 6;
      tileBlocksTempl[1].type = stepsBlockT;

      // angled roof
      glGenVertexArrays(1, &tileBlocksTempl[2].vpair.VAO);
      glBindVertexArray(tileBlocksTempl[2].vpair.VAO);

      glGenBuffers(1, &tileBlocksTempl[2].vpair.VBO);
      glBindBuffer(GL_ARRAY_BUFFER, tileBlocksTempl[2].vpair.VBO);

      float angledRoof[] = {
	0.0f - t, 0.0f, 0.0f-t,            1.0f, 0.0f,
	bBlockW, 0.0f, 0.0f -t,        0.0f, 0.0f,
	bBlockW, floorH, bBlockD,   0.0f, 1.0f,

	0.0f - t, 0.0f, 0.0f -t,           1.0f, 1.0f,
	0.0f - t, 0.0f, bBlockD,         0.0f, 0.0f,
	bBlockW, floorH, bBlockD,    0.0f, 1.0f,

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

      tileBlocksTempl[2].vertexes = malloc(sizeof(angledRoof));
      memcpy(tileBlocksTempl[2].vertexes, angledRoof, sizeof(angledRoof));
      tileBlocksTempl[2].vertexesSize = 6 + 6 * 2;
      tileBlocksTempl[2].type = angledRoofT;

      


      // TODO: I can win some peft to now calculate new buffer if transforms things == default
      /*
	glBufferData(GL_ARRAY_BUFFER, sizeof(roofBlock), roofBlock, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);*/

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }
    
    // textured tiels
    tileMeshes = malloc(2 * sizeof(MeshBuffer));

    for(int i=1; i<=2;i++){
      glGenVertexArrays(1, &tileMeshes[i-1].VAO);
      glBindVertexArray(tileMeshes[i-1].VAO);

      glGenBuffers(1, &tileMeshes[i-1].VBO);
      glBindBuffer(GL_ARRAY_BUFFER, tileMeshes[i-1].VBO);

      if(i == fromUnder){
	float texturedTileVerts[] = {
	  bBlockW, -wallD/2, bBlockD , 0.0f, 1.0f,
	  0.0f, -wallD/2, bBlockD , 1.0f, 1.0f,
	  bBlockW, -wallD/2, 0.0f , 0.0f, 0.0f, 
      
	  0.0f, -wallD/2, bBlockD , 1.0f, 1.0f,
	  bBlockW, -wallD/2, 0.0f , 0.0f, 0.0f,
	  0.0f, -wallD/2, 0.0f , 1.0f, 0.0f, 
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(texturedTileVerts), texturedTileVerts, GL_STATIC_DRAW);
      }else if(i == fromOver){
	float texturedTileVerts[] = {
	  bBlockW, 0.0f, bBlockD , 0.0f, 1.0f,
	  0.0f, 0.0f, bBlockD , 1.0f, 1.0f,
	  bBlockW, 0.0f, 0.0f , 0.0f, 0.0f, 
      
	  0.0f, 0.0f, bBlockD , 1.0f, 1.0f,
	  bBlockW, 0.0f, 0.0f , 0.0f, 0.0f,
	  0.0f, 0.0f, 0.0f , 1.0f, 0.0f, 
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(texturedTileVerts), texturedTileVerts, GL_STATIC_DRAW);
      }

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);


      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }
  }

  // walls
  wallMeshes = malloc(basicSideCounter * sizeof(MeshBuffer*));
  wallHighlight = malloc(basicSideCounter * sizeof(MeshBuffer));
  halfWallHighlight = malloc(basicSideCounter * sizeof(MeshBuffer));
  
  for(int i=0;i<basicSideCounter;i++){
    wallMeshes[i] = malloc((wallTypeCounter - 1) * sizeof(MeshBuffer));
  }
  
  // wallsLoadVAOandVBO();
  
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

  // load shaders and apply it
  {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, "fog.vert");
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, "fog.frag");

    mainShader = glCreateProgram();
    glAttachShader(mainShader, vertexShader);
    glAttachShader(mainShader, fragmentShader);

    // Link the shader mainShaderram
    glLinkProgram(mainShader);

    // Check for linking errors
    GLint linkStatus; 
    glGetProgramiv(mainShader, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE) {
      GLint logLength;
      glGetProgramiv(mainShader, GL_INFO_LOG_LENGTH, &logLength);  
      char* log = (char*)malloc(logLength);
      glGetProgramInfoLog(mainShader, logLength, NULL, log);
      fprintf(stderr, "Failed to link mainShaderram: %s\n", log);
      free(log);
      return 1;
    }

  }

  modelLoc = glGetUniformLocation(mainShader, "model");
  int projUni = glGetUniformLocation(mainShader, "proj");
  int viewLoc = glGetUniformLocation(mainShader, "view");

  int lightColor = glGetUniformLocation(mainShader, "lightColor");
  //  glUniform3f(lightColor, 1.0f, 1.0f, 1.0f);

  // load shaders and apply it
  {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, "hud.vert");
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, "hud.frag");

    hudShader = glCreateProgram();
    glAttachShader(hudShader, fragmentShader);
    glAttachShader(hudShader, vertexShader);

    // Link the shader hudShaderram
    glLinkProgram(hudShader);

    // Check for linking errors
    GLint linkStatus;
    glGetProgramiv(hudShader, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE) {
      GLint logLength;
      glGetProgramiv(hudShader, GL_INFO_LOG_LENGTH, &logLength);
      char* log = (char*)malloc(logLength);
      glGetProgramInfoLog(hudShader, logLength, NULL, log);
      fprintf(stderr, "Failed to link hudShaderram: %s\n", log);
      free(log);
      return 1;
    }
  }


    // load shaders and apply it
  {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, "lightSource.vert");
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, "lightSource.frag");

    lightSourceShader = glCreateProgram();
    glAttachShader(lightSourceShader, fragmentShader);
    glAttachShader(lightSourceShader, vertexShader);

    // Link the shader lightSourceShaderram
    glLinkProgram(lightSourceShader);

    // Check for linking errors
    GLint linkStatus;
    glGetProgramiv(lightSourceShader, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE) {
      GLint logLength;
      glGetProgramiv(lightSourceShader, GL_INFO_LOG_LENGTH, &logLength);
      char* log = (char*)malloc(logLength);
      glGetProgramInfoLog(lightSourceShader, logLength, NULL, log);
      fprintf(stderr, "Failed to link hudShaderram: %s\n", log);
      free(log);
      return 1;
    }
  }

  lightModelLoc = glGetUniformLocation(lightSourceShader, "model");
  int lightProjUni = glGetUniformLocation(lightSourceShader, "proj");
  int lightViewLoc = glGetUniformLocation(lightSourceShader, "view");

  int orthoLoc = glGetUniformLocation(hudShader, "ortho");
  int hudColorLoc = glGetUniformLocation(hudShader, "u_Color");
  
  int viewportLoc = glGetUniformLocation(hudShader, "viewport");

  glUseProgram(mainShader);
  glUniform2f(viewportLoc, windowW, windowH);
  
  vec3 fogColor = {0.5f, 0.5f, 0.5f};
  glClearColor(argVec3(fogColor), 1.0f);
    
  const float doorW =  bBlockW - doorPad;

  const float doorFrameH = 0.2f;

  float zoom = 0.0f;
  
  bool highlighting = 1;
  
  // init opengl
  {
    //  glEnable(GL_MULTISAMPLE);  
    
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    //glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);  
    glDepthFunc(GL_LEQUAL);
    //  glDepthMask(GL_FALSE);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

    //    glEnable(GL_MULTISAMPLE);  
  }

  // load purple black texture to detect mistakes
  {
    glGenTextures(1, &errorTx);

    glBindTexture(GL_TEXTURE_2D, errorTx);
      
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 12,
		 1, 0, GL_RGBA,
		 GL_UNSIGNED_BYTE, color);
      
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  // make texture to hide
  {
    glGenTextures(1, &emptyTx);
    glBindTexture(GL_TEXTURE_2D, emptyTx);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLubyte color[] = {
      0, 0, 0, 0,
    };
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1,
		 1, 0, GL_RGBA,
		 GL_UNSIGNED_BYTE, color);

    glBindTexture(GL_TEXTURE_2D, 0);
  }

  // load 1x1 texture to rende ricolors
  {
    glGenTextures(1, &solidColorTx);
    glBindTexture(GL_TEXTURE_2D, solidColorTx);
      
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLubyte color[4] = {255, 255, 255, 255};
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1,
		 1, 0, GL_RGBA,
		 GL_UNSIGNED_BYTE, color);
      
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  
  // load textures
  {
    
    FILE* texSpecs = fopen(texturesFolder"texSpec.txt", "r"); 

    if (!texSpecs) {
      printf("Missing \"%s\"", texturesFolder"texSpec.txt");
      exit(-1);
    }
  
    char ch;
    while((ch = fgetc(texSpecs)) != EOF)  if(ch == '\n') loadedTexturesCounter++;

    loadedTexturesCounter++; 

    rewind(texSpecs);
   
    char category[300];
    char fileName[300]; 
  
    for(int i=0;i<loadedTexturesCounter;i++){ 
      fscanf(texSpecs, "%s %s\n", &fileName, &category); 

      bool exists = false;
      for(int i2=0;i2<loadedTexturesCategoryCounter;i2++){
	if(strcmp(loadedTexturesCategories[i2].name, category)==0){
	  loadedTexturesCategories[i2].txWithThisCategorySize++;
	  exists = true;
	  break;
	}
      }

      if(!exists){
	if(!loadedTexturesCategories){
	  loadedTexturesCategories = malloc(sizeof(Category));
	}else{
	  loadedTexturesCategories = realloc(loadedTexturesCategories, sizeof(Category) * (loadedTexturesCategoryCounter + 1));
	};

	loadedTexturesCategories[loadedTexturesCategoryCounter].index = loadedTexturesCategoryCounter;

	int newCategoryLen = strlen(category);

	longestTextureCategoryLen = max(newCategoryLen, longestTextureCategoryLen);
	
	loadedTexturesCategories[loadedTexturesCategoryCounter].name = malloc(sizeof(char) * (newCategoryLen+1));
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

    for(int i2=0;i2<loadedTexturesCategoryCounter;i2++){
      loadedTextures2D[i2] = malloc(sizeof(Texture) * loadedTexturesCategories[i2].txWithThisCategorySize);
    }
  
    for(int i=0;i<loadedTexturesCounter;i++){    
      fscanf(texSpecs, "%s %s\n", &fileName, &category);

      loadedTexturesNames[i] = malloc(sizeof(char) * (strlen(fileName)+1));
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

      glGenTextures(1, &txId);
    
      glBindTexture(GL_TEXTURE_2D, txId);
      
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->w,
		   texture->h, 0, GL_RGBA,
		   GL_UNSIGNED_BYTE, texture->pixels);

      SDL_FreeSurface(texture);

      glBindTexture(GL_TEXTURE_2D, 0);

      int categoryIndex = -1;
    
      for(int i2=0;i2<loadedTexturesCategoryCounter;i2++){
	if(strcmp(loadedTexturesCategories[i2].name, category)==0){
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

  geometry = calloc(loadedTexturesCounter, sizeof(Geometry));

  for(int i=0;i<loadedTexturesCounter;i++){
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

  // load 3d models
  {
    FILE* objsSpecs = fopen("./assets/objs/ObjsSpecs.txt", "r");
      
    if(!objsSpecs){
      printf("ObjsSpecs.txt was not found! \n");
    }else{
      char textureName[50];
      char objName[50];
      char typeStr[10];

      int charsCounter = 0;
      int objsCounter = 0;

      while(fscanf(objsSpecs, "%s %s %s\n", objName, textureName, typeStr) != EOF){	 
	if(strcmp(typeStr, "Char")==0){
	  charsCounter++;
	}else if(strcmp(typeStr, "Obj")==0){
	  objsCounter++;
	}else{
	  printf("Model %s has wrong type %s \n", objName, typeStr);
	  exit(0);
	}
      };

      loadedModels1D = malloc(sizeof(ModelInfo) * (charsCounter + objsCounter));

      loadedModels2D = malloc(sizeof(ModelInfo*) * modelTypeCounter);
      loadedModels2D[objectModelType] = malloc(sizeof(ModelInfo) * objsCounter);
      loadedModels2D[characterModelType] = malloc(sizeof(ModelInfo) * objsCounter);

      rewind(objsSpecs);

      while(fscanf(objsSpecs, "%s %s %s\n", objName, textureName, typeStr) != EOF){	
	char *fullObjPath = malloc(strlen(objName) + strlen(objsFolder) + 1);
	  
	strcpy(fullObjPath, objsFolder);
	strcat(fullObjPath, objName);

	char *fullTxPath = malloc(strlen(textureName) + strlen(objsFolder) + 1);
	  
	strcpy(fullTxPath, objsFolder);
	strcat(fullTxPath, textureName);

	ModelType type = -1;
	
	for(int i2=0;i2<modelTypeCounter;i2++){
	  if(strcmp(typeStr, modelsTypesInfo[i2].str) == 0){
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
  
  // tang of fov calculations
  fov = editorFOV;
  tangFOV = tanf(rad(fov) * 0.5);
  
  mouse = (Mouse){  .h = 0.005f, .w = 0.005f, .interDist = 1000.0f  };

  float dist = sqrt(1 / 3.0);
  bool cameraMode = true;

  float testFOV = editorFOV;


  // load or init grid
  if(!loadSave("map")){
    initGrid(gridX, gridY, gridZ);

    printf("Map not found!\n");
  }

  collectTilesMats();

  // set up camera
  GLint cameraPos = glGetUniformLocation(mainShader, "cameraPos");
  {
    camera1.pos = (vec3)xyz_indexesToCoords(gridX/2, 2, gridZ/2);
    camera2.pos = (vec3)xyz_indexesToCoords(gridX/2, 2, gridZ/2);
    camera1.up = (vec3){ 0.0f, 1.0f, 0.0f };
    camera2.up = (vec3){ 0.0f, 1.0f, 0.0f };

    glUniform3f(cameraPos, camera1.pos.x, camera1.pos.y, camera1.pos.z);
  }
  
  // set draw distance to gridX/2
  GLint radius = glGetUniformLocation(mainShader, "radius");
  drawDistance = 10;
  glUniform1f(radius, drawDistance);

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

  while (!quit) {
    uint32_t starttime = GetTickCount();
	 
    clock_t currentFrame = clock();
    deltaTime = (double)(currentFrame - lastFrame) / CLOCKS_PER_SEC;
    lastFrame = currentFrame;

    cameraSpeed = 10.0f * deltaTime;

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
	quit = true;
      }

      if (event.type == SDL_KEYDOWN) {
	if(event.key.keysym.scancode == SDL_SCANCODE_F5 && !selectedTextInput) {
	  saveMap(curSaveName);
	}else if(console.open){
	  consoleHasResponse = false;
	  
	  if(event.key.keysym.scancode == SDL_SCANCODE_RETURN){
	    char copy_consoleBuffer[CONSOLE_BUF_CAP];
	    strcpy(copy_consoleBuffer, consoleBuffer);

	    char* str_tok = strtok(copy_consoleBuffer, " ");
	    
	    if (str_tok) {
	      if(strcmp(str_tok, "help") == 0){
		strcpy(consoleResponse, "help - all available commands\nsave <file_name> - save current map in new file\nload <file_name> - load map from file\ncreate <x> <y> <z> <file_name> - to create new world with size x, y, z");
	      }else if(strcmp(str_tok, "load") == 0){
		str_tok = strtok(NULL, " ");

		if(str_tok){
		  if(loadSave(str_tok)){
		    sprintf(consoleResponse, "Save \"%s\" was successfully loaded", str_tok);
		  }else{
		    sprintf(consoleResponse, "Save \"%s\" doesnt exist", str_tok);
		  }
		}else{
		  strcpy(consoleResponse, "Provide name of save to load \"load <file_name>\"");
		}
	      }else if(strcmp(str_tok, "save") == 0){
		// get save arg
		str_tok = strtok(NULL, " ");

		if(str_tok){
		  if(saveMap(str_tok)){
		    sprintf(consoleResponse, "Save \"%s\" was successfully saved", str_tok);
		  }else{
		    // sprintf(consoleResponse, "Save \"%s\" was successfully saved", str_tok);
		  }
		}else{
		  strcpy(consoleResponse, "Provide name for save \"save <file_name>\"");
		}
		
	      }else if(strcmp(str_tok, "create") == 0){
		// get save arg
		str_tok = strtok(NULL, " ");

		int x, y, z;

		bool generalMistake = false;

		if(str_tok){
		  x = atoi(str_tok);

		  if(x<=0){
		    sprintf(consoleResponse, "Incorrect x(%d) value, x must be > 0", x);
		  }else{
		    str_tok = strtok(NULL, " ");

		    if(str_tok){
		      y = atoi(str_tok);

		      if(y<=9){
			sprintf(consoleResponse, "Incorrect y(%d) value, y must be >= 10", x);
		      }else{
			str_tok = strtok(NULL, " ");

			if(str_tok){
			  z = atoi(str_tok);

			  if(z <= 0){
			    sprintf(consoleResponse, "Incorrect z(%d) value, z must be > 0", x);
			  }else{
			    str_tok = strtok(NULL, " ");
			    
			    if(str_tok){
			      strcpy(curSaveName, str_tok);
				  
			      createMap(x, y, z);
			      sprintf(consoleResponse, "Map was craeted with size x: %d y: %d z:%d", x,y,z);
			    }else{
			      generalMistake = true;
			    }
			  }
			}else{
			  generalMistake = true;
			}
		      }
		    }else{
		      generalMistake = true;
		    }
		  }
		}else{
		  generalMistake = true;
		}

		if(generalMistake){
		  strcpy(consoleResponse, "From to create new map \"create <x> <y> <z> <file_name>\"");
		}
	      }else{
		sprintf(consoleResponse, "Command \"%s\"  doesnt exist\nWrite \"help\" to get all available commands", str_tok);
	      }

	      consoleHasResponse = true;
	    }
	  }else if(event.key.keysym.scancode == SDL_SCANCODE_F1){
	    console.open = false;
	  }else if(consoleBufferCursor > 0 && event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE){
	    consoleBufferCursor--;
	    consoleBuffer[consoleBufferCursor] = 0;
	  }else if(consoleBufferCursor < CONSOLE_BUF_CAP - 1){
	    if(event.key.keysym.scancode >= 4 && event.key.keysym.scancode <= 39){
	      consoleBuffer[consoleBufferCursor] = sdlScancodesToACII[event.key.keysym.scancode];
	      consoleBufferCursor++;
	    }else if(event.key.keysym.scancode == SDL_SCANCODE_SPACE){
	      bool prevCharIsntSpace = consoleBufferCursor > 0 && consoleBuffer[consoleBufferCursor - 1] != ' ';

	      if(prevCharIsntSpace){
		consoleBuffer[consoleBufferCursor] = ' ';
		consoleBufferCursor++;
	      }
	    }
	  }
	}else if(dialogViewer.open){
	  if(event.key.keysym.scancode == SDL_SCANCODE_T){
	    Model* model = (Model*) mouse.focusedThing;

	    characters[model->characterId].curDialogIndex = 0;
	    characters[model->characterId].curDialog = &characters[model->characterId].dialogs;

	    dialogViewer.open = false;
	    dialogEditor.open = true;
	    curMenu = &dialogEditor;

	    dialogEditor.textInputs[replicaInput].buf = &characters[model->characterId].curDialog->replicaText;
	    dialogEditor.textInputs[charNameInput].buf = &characters[model->characterId].name;

	    for (int i = 0; i < characters[model->characterId].dialogs.answersSize; i++) {
	      dialogEditor.textInputs[i + answerInput1].buf = &characters[model->characterId].curDialog->answers[i].text;
	    }
		 
	  }
	}else if(curMenu && curMenu->type == dialogEditorT){
	  if(event.key.keysym.scancode == SDL_SCANCODE_B && !selectedTextInput){
	    int characterId = -1;

	    if (mouse.focusedType == mouseModelT) {
	      Model* model = (Model*)mouse.focusedThing;
	      characterId = model->characterId;

	    }
	    else if (mouse.focusedType == mousePlaneT) {
	      Picture* plane = (Picture*)mouse.focusedThing;
	      characterId = plane->characterId;
	    }

	    characters[characterId].curDialogIndex = 0;
	    characters[characterId].curDialog = &characters[characterId].dialogs;

	    dialogEditor.open = false;
	    curMenu = NULL;

	    if(tempTextInputStorageCursor!=0){
	      tempTextInputStorageCursor=0;
	      memset(tempTextInputStorage, 0, 512 * sizeof(char)); 
	    }
	  }else if(event.key.keysym.scancode == SDL_SCANCODE_T && !selectedTextInput){
	    int characterId = -1;

	    if (mouse.focusedType == mouseModelT) {
	      Model* model = (Model*)mouse.focusedThing;
	      characterId = model->characterId;

	    }
	    else if (mouse.focusedType == mousePlaneT) {
	      Picture* plane = (Picture*)mouse.focusedThing;
	      characterId = plane->characterId;
	    }

	    characters[characterId].curDialogIndex = 0;
	    characters[characterId].curDialog = &characters[characterId].dialogs;

	    dialogViewer.open = true;
	    dialogEditor.open = false;
	    curMenu = &dialogViewer;

	    if(tempTextInputStorageCursor!=0){
	      tempTextInputStorageCursor=0;
	      memset(tempTextInputStorage, 0, 512 * sizeof(char)); 
	    }
	  }else{
	    if(selectedTextInput){
	      const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
	      
	      if(false ){//event.key.keysym.scancode == SDL_SCANCODE_RETURN && *selectedTextInput->buf){
		*selectedTextInput->buf = malloc(sizeof(char) * (strlen(*selectedTextInput->buf)+1));
		strcpy(*selectedTextInput->buf, tempTextInputStorage);

		selectedTextInput->active = false;
		selectedTextInput = NULL; 
	    
		tempTextInputStorageCursor=0;
		memset(tempTextInputStorage, 0, 512 * sizeof(char)); 
	      }else if(currentKeyStates[SDL_SCANCODE_LCTRL] && currentKeyStates[SDL_SCANCODE_BACKSPACE]){
		int lastSpacePos = lastCharPos(tempTextInputStorage, ' ');

		if(lastSpacePos != -1){
		  // TODO: On realease strcut() leads to some strange exception
		  for (int i = lastSpacePos; i < tempTextInputStorageCursor-1; i++) {
		    tempTextInputStorage[i] = '\0';
		  }
		  //strcut(tempTextInputStorage,lastSpacePos ,tempTextInputStorageCursor - 1);    
		  tempTextInputStorageCursor = lastSpacePos;
		}else{
		  memset(tempTextInputStorage, 0, 512 * sizeof(char));
		  tempTextInputStorageCursor = 0;
		}
	      }else if(currentKeyStates[SDL_SCANCODE_LCTRL] && currentKeyStates[SDL_SCANCODE_V]){
		char * clipboardStr = SDL_GetClipboardText();

		cleanString(clipboardStr);

		int clipboardStrLen = strlen(clipboardStr);
		int diff = (tempTextInputStorageCursor + clipboardStrLen) - selectedTextInput->charsLimit;  

		if (diff < clipboardStrLen) {
		  for (int i = 0;i< clipboardStrLen;i++) {
		    char ch = clipboardStr[i];
		    
		    int pos = lastCharPos(tempTextInputStorage, '\n');
		  
		    if(selectedTextInput->rect.w/letterW<= tempTextInputStorageCursor - pos){
		      tempTextInputStorage[tempTextInputStorageCursor] = '\n';
		      tempTextInputStorageCursor++;  
		    }

		    tempTextInputStorage[tempTextInputStorageCursor] = ch;
		    tempTextInputStorageCursor++;

		    if(tempTextInputStorageCursor >= selectedTextInput->charsLimit) break;
		  }
		}

		SDL_free(clipboardStr);
	      }else if(currentKeyStates[SDL_SCANCODE_LCTRL] && currentKeyStates[SDL_SCANCODE_C]){
		if(tempTextInputStorageCursor != 0){
		  SDL_SetClipboardText(tempTextInputStorage);
		}
	      }else if(tempTextInputStorageCursor > 0 && event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE){
		tempTextInputStorageCursor--;
		tempTextInputStorage[tempTextInputStorageCursor] = 0;
	      }else if(tempTextInputStorageCursor < selectedTextInput->charsLimit){
		if(event.key.keysym.scancode >= 4 && event.key.keysym.scancode <= 39 || event.key.keysym.scancode == 55){
		  int pos = lastCharPos(tempTextInputStorage, '\n');
		  
		  if(selectedTextInput->rect.w/(letterCellW/1.9f)<= tempTextInputStorageCursor - pos){
		    tempTextInputStorage[tempTextInputStorageCursor] = '\n';
		    tempTextInputStorageCursor++;  
		  }
		  
		  char newChar = sdlScancodesToACII[event.key.keysym.scancode];

		  if (currentKeyStates[SDL_SCANCODE_LSHIFT]){
		    if(event.key.keysym.scancode <= 29){
		      newChar -= 32;
		    }else if(event.key.keysym.scancode == 36){ // '?'
		      newChar = 63;
		    }else if(event.key.keysym.scancode == 30){ // '!'
		      newChar = 33;
		    }else if(event.key.keysym.scancode == 33){ // '$'
		      newChar = 36;
		    }else if(event.key.keysym.scancode == 31){ // '@'
		      newChar = 64;
		    }else if(event.key.keysym.scancode == 32){ // '#'
		      newChar = 35;
		    }else if(event.key.keysym.scancode == 34){ // '%'
		      newChar = 37;
		    }
		  }
		  
		  tempTextInputStorage[tempTextInputStorageCursor] = newChar;
		  tempTextInputStorageCursor++;
		}else if(event.key.keysym.scancode == SDL_SCANCODE_SPACE){
		  bool prevCharIsntSpace = tempTextInputStorageCursor > 0 && tempTextInputStorage[tempTextInputStorageCursor - 1] != ' '; 

		  if(prevCharIsntSpace){
		    tempTextInputStorage[tempTextInputStorageCursor] = ' ';
		    tempTextInputStorageCursor++;
		  } 
		}
	      }else if(tempTextInputStorageCursor > 0 && event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE){
		tempTextInputStorageCursor--;
		tempTextInputStorage[tempTextInputStorageCursor] = 0;
	      }
	    }
	  }
	}else{       
	  switch (event.key.keysym.scancode) {
	  case(SDL_SCANCODE_F1):{
	    console.open = true;
	  
	    break;
	  }	 case(SDL_SCANCODE_F2):{
		   hints = !hints;
	  
		   break;
		 }	  case(SDL_SCANCODE_F3):{
			    enviromental.snow = !enviromental.snow;
	  
			    break;
			  }
	  case(SDL_SCANCODE_UP): {
	    if(manipulationMode != 0 && (mouse.focusedType == mouseModelT || mouse.focusedType == mousePlaneT)){
	      Matrix* mat = -1;
	      bool itPlane = false;

	      Model* model = NULL;
	      
	      if (mouse.focusedType == mouseModelT) {
		model = (Model*)mouse.focusedThing;
		mat = &model->mat;

	      }
	      else if (mouse.focusedType == mousePlaneT) {
		Picture* plane = (Picture*)mouse.focusedThing;
		mat = &plane->mat;
		itPlane = true;
	      }


	      switch(manipulationMode){
	      case(TRANSFORM_Z):{
		mat->m[13] = mat->m[13] + manipulationStep;

		if(model){
		  calculateModelAABB(model);
		}
		
		break;
	      }
	      case(TRANSFORM_XY): {
		mat->m[12] = mat->m[12] - manipulationStep;

		if(model){
		  calculateModelAABB(model);
		}
		
		break;
	      }
	      case(SCALE): {
		if(itPlane){
		  Picture* plane = (Picture*)mouse.focusedThing;
		  plane->h += 0.01f;
		  /*
		    float xTemp = mat->m[12];
		    float yTemp = mat->m[13];
		    float zTemp = mat->m[14];

		    mat->m[12] = 0;
		    mat->m[13] = 0;
		    mat->m[14] = -zTemp;

		    //		  scale(mat->m, 1.0f, manipulationScaleStep, 1.0f);

		    mat->m[12] = xTemp;
		    mat->m[13] = yTemp;
		    mat->m[14] = zTemp;*/
		}else{
		  float xTemp = mat->m[12];
		  float yTemp = mat->m[13];
		  float zTemp = mat->m[14];

		  mat->m[12] = 0;
		  mat->m[13] = 0;
		  mat->m[14] = -zTemp;

		  scale(mat->m, manipulationScaleStep, manipulationScaleStep, manipulationScaleStep);

		  mat->m[12] = xTemp;
		  mat->m[13] = yTemp;
		  mat->m[14] = zTemp;
		}

		if(model){
		  calculateModelAABB(model);
		}
		
		break;
	      }

	      default: break;
	      }
	    }
	    else if(mouse.brushType == mouseBlockBrushT || mouse.selectedType == mouseBlockT){
	      TileBlock* block = NULL;

	      if(mouse.brushType == mouseBlockBrushT){
		block = (TileBlock*) mouse.brushThing;
	      }else if(mouse.selectedType == mouseBlockT){
		block = (TileBlock*) mouse.selectedThing;
	      }

	      if(block->type == roofBlockT) { 
		const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL); 

		if (currentKeyStates[SDL_SCANCODE_LCTRL] && block->vertexes[11] + manipulationStep <= floorH) {
		  block->vertexes[11] += manipulationStep;
		  block->vertexes[21] += manipulationStep;
		  block->vertexes[26] += manipulationStep;  
		}
		else if (!currentKeyStates[SDL_SCANCODE_LCTRL] && block->vertexes[1] + manipulationStep <= floorH) { 
		  block->vertexes[1] += manipulationStep;
		  block->vertexes[6] += manipulationStep;
		  block->vertexes[16] += manipulationStep;
		}
		
	      }
	    }
	    else if (mouse.selectedType == mouseWallT) {
	      WallMouseData* data = (WallMouseData*) mouse.selectedThing;

	      const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

	      Matrix* mat = &data->tile->walls[data->side].mat;

	      if(currentKeyStates[SDL_SCANCODE_LCTRL]){
		scale(mat, 1.0f, 1.05f, 1.0f);		
	      }else{
		if(data->side == bot || data->side == top){
		  mat->m[14] += .05f;
		}else{
		  mat->m[12] += .05f;
		}
	      }

	      for(int i=0;i<wallsVPairs[data->type].planesNum;i++){
		calculateAABB(*mat, wallsVPairs[data->type].pairs[i].vBuf, wallsVPairs[data->type].pairs[i].vertexNum, &data->tile->walls[data->side].planes[i].lb, &data->tile->walls[data->side].planes[i].rt);
	      }
	    }else if (mouse.selectedType == mouseTileT) {
	      TileMouseData* data = (TileMouseData*)mouse.selectedThing;

	      if(data->tile->groundLift + manipulationStep < floorH - 0.01f) {
		data->tile->groundLift += manipulationStep;
	      }
	      
	      /*
		GroundType type = valueIn(grid[curFloor][mouse.gridIntersect.z][mouse.gridIntersect.x]->ground, 0);

		if (type != texturedTile) {
		setIn(grid[curFloor][mouse.gridIntersect.z][mouse.gridIntersect.x]->ground, 0, texturedTile);
		setIn(grid[curFloor][mouse.gridIntersect.z][mouse.gridIntersect.x]->ground, mouse->groundInter, 0);
		}
		else {
		Texture curTx = valueIn(grid[curFloor][mouse.gridIntersect.z][mouse.gridIntersect.x]->ground, mouse->groundInter);

		if (curTx == texturesCounter - 1) {
		setIn(grid[curFloor][mouse.gridIntersect.z][mouse.gridIntersect.x]->ground, 0, netTile);
		}
		else {
		setIn(grid[curFloor][mouse.gridIntersect.z][mouse.gridIntersect.x]->ground, mouse->groundInter, curTx + 1);
		}
		}
	      */
	    }

	    break;
	  }
	  case(SDL_SCANCODE_DOWN): {
	    // TODO: if intersected tile + wall will work only tile changer
	    if(manipulationMode != 0 && (mouse.focusedType == mouseModelT || mouse.focusedType == mousePlaneT)){
	      Matrix* mat = -1;
	      bool isPlane = false;
	      Model* model = NULL;
	      
	      if (mouse.focusedType == mouseModelT) {
		model = (Model*)mouse.focusedThing;
		mat = &model->mat;

	      }
	      else if (mouse.focusedType == mousePlaneT) {
		Picture* plane = (Picture*)mouse.focusedThing;
		mat = &plane->mat;
		isPlane = true;
	      }

	      switch(manipulationMode){ 
	      case(TRANSFORM_Z):{
		mat->m[13] = mat->m[13] - manipulationStep;

		if(model){
		  calculateModelAABB(model);
		}
		
		break;
	      }
	      case(TRANSFORM_XY): {
		mat->m[12] = mat->m[12] + manipulationStep;

		if(model){
		  calculateModelAABB(model);
		}
		
		break;
	      }
	      case(SCALE): {
		if(isPlane){
		  Picture* plane = (Picture*)mouse.focusedThing;
		  plane->h -= 0.01f;
		}else{
		  float xTemp = mat->m[12];
		  float yTemp = mat->m[13];
		  float zTemp = mat->m[14];

		  mat->m[12] = 0;
		  mat->m[13] = 0;
		  mat->m[14] = -zTemp;

		  scale(mat->m, 1.0f / manipulationScaleStep, 1.0f / manipulationScaleStep, 1.0f / manipulationScaleStep);

		  mat->m[12] = xTemp;
		  mat->m[13] = yTemp;
		  mat->m[14] = zTemp;
		}
		
		if(model){
		  calculateModelAABB(model);
		}
		
		break;
	      }

	      default: break;
	      }
	    }else if(mouse.brushType == mouseBlockBrushT){
	      TileBlock* block = (TileBlock*) mouse.brushThing;
	      

	      if(block->type == roofBlockT) {
		const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

		if (currentKeyStates[SDL_SCANCODE_LCTRL] && block->vertexes[11] - manipulationStep >= 0) {
		  block->vertexes[11] -= manipulationStep;
		  block->vertexes[21] -= manipulationStep;
		  block->vertexes[26] -= manipulationStep;
		}
		else if (!currentKeyStates[SDL_SCANCODE_LCTRL] && block->vertexes[1] - manipulationStep >= 0) {
		  block->vertexes[1] -= manipulationStep;
		  block->vertexes[6] -= manipulationStep;
		  block->vertexes[16] -= manipulationStep;
		}
		
	      }
	    }else if (mouse.selectedType == mouseWallT) {
	      WallMouseData* data = (WallMouseData*)mouse.selectedThing;
	      
	      const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

	      Matrix* mat = &data->tile->walls[data->side].mat;

	      if(currentKeyStates[SDL_SCANCODE_LCTRL]){
		scale(mat, 1.0f, 1/1.05f, 1.0f);
	      }else{
		if(data->side == bot || data->side == top){
		  mat->m[14] -= .05f;
		}else{
		  mat->m[12] -= .05f;
		}
	      }
	      
	    for(int i=0;i<wallsVPairs[data->type].planesNum;i++){
	      calculateAABB(*mat, wallsVPairs[data->type].pairs[i].vBuf, wallsVPairs[data->type].pairs[i].vertexNum, &data->tile->walls[data->side].planes[i].lb, &data->tile->walls[data->side].planes[i].rt);
	    }
	    }else if (mouse.selectedType == mouseTileT) {
	      TileMouseData* data = (TileMouseData*) mouse.selectedThing;

	      if(data->tile->groundLift - manipulationStep >= 0) {
		data->tile->groundLift -= manipulationStep;
	      }
	    }

	    break;
	  }
	  case(SDL_SCANCODE_Q): {
	    curCamera->pos.y += .01f ;
	  
	    break;
	  }
	  case(SDL_SCANCODE_F): {
	    if(!mouse.focusedThing){
	      if(mouse.selectedType == mousePlaneT || mouse.selectedType == mouseModelT){
		mouse.focusedThing = mouse.selectedThing;
		mouse.focusedType = mouse.selectedType;
	      }
	    }else{
	      mouse.focusedThing = NULL;
	      mouse.focusedType = 0;
	      manipulationMode = 0;
	      manipulationStep = 0.01f;
	    }
	    
	    /*
	      if(mouse.focusedModel && mouse.selectedModel && mouse.focusedModel->id == mouse.selectedModel->id){
	      mouse.focusedModel = NULL;
	      }else{
	      mouse.focusedModel = mouse.selectedModel;
	      }*/
	  
	    break;
	  }
	  case(SDL_SCANCODE_E): {
	    curCamera->pos.y -= .01f;

	    break;
	  } 
	  case(SDL_SCANCODE_Z): {
	    drawDistance += .1f / 2.0f;

	    if(enviromental.fog){
	      glUniform1f(radius, drawDistance);
	    }

	    break;
	  }
	  case(SDL_SCANCODE_X): {
	    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
			      
	    if(!currentKeyStates[SDL_SCANCODE_LCTRL]){
	      if(mouse.brushThing || mouse.brushType){
		if (mouse.brushType == mouseBlockBrushT) {
		  TileBlock* block = (TileBlock*) mouse.brushThing;
		  free(block->vertexes);
		  free(block);
		}

		if (mouse.brushType == mouseWallBrushT && mouse.brushThing) {
		  free(mouse.brushThing);
		}

		mouse.brushThing = NULL;
		mouse.brushType = 0;
	      }else{
		drawDistance -= .1f / 2.0f;

		if(enviromental.fog){
		  glUniform1f(radius, drawDistance);
		}
	      }
	    }

	    break;
	  }
	  case(SDL_SCANCODE_LEFT): {
	    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
	    
	    if(manipulationMode != 0 && (mouse.focusedType == mouseModelT || mouse.focusedType == mousePlaneT)){  
	      Matrix* mat = -1; 
	      bool isPlane = false;
	      Model* model = NULL;

	      if (mouse.focusedType == mouseModelT) {
		model = (Model*)mouse.focusedThing;
		mat = &model->mat;

	      }
	      else if (mouse.focusedType == mousePlaneT) {
		Picture* plane = (Picture*)mouse.focusedThing;
		mat = &plane->mat;
		isPlane = true;
	      }

	      float xTemp = mat->m[12];
	      float yTemp = mat->m[13];
	      float zTemp = mat->m[14];

	      if (manipulationMode == ROTATE_Y || manipulationMode == ROTATE_X || manipulationMode == ROTATE_Z) {
		mat->m[12] = 0;
		mat->m[13] = 0;
		mat->m[14] = -zTemp;

		if (manipulationMode == ROTATE_Y) {
		  rotateY(mat->m, -rad(1.0f));
		}
		else if (manipulationMode == ROTATE_X) {
		  rotateX(mat->m, -rad(1.0f));
		}
		else if (manipulationMode == ROTATE_Z) {
		  rotateZ(mat->m, -rad(1.0f));
		}

		mat->m[12] = xTemp;
		mat->m[13] = yTemp;
		mat->m[14] = zTemp;

		if(model){
		  calculateModelAABB(model);
		}
	      }else if(manipulationMode == SCALE && isPlane){
		Picture* plane = (Picture*)mouse.focusedThing;
		plane->w -= 0.01f;
	      }else if (manipulationMode == TRANSFORM_XY) {
		mat->m[14] = mat->m[14] + manipulationStep;
		
		if(model){
		  calculateModelAABB(model);
		}
	      }
	    }else if (mouse.selectedType == mouseWallT) { // + left W
	      /*	      WallMouseData* data = (WallMouseData*)mouse.selectedThing;
	      
	      bool index1S = data->side == right || data->side == left; // 1 - ctrl 0 - norm
	      bool rightIsDefault = data->tile->customWalls[data->side].buf[rightWallMap[index1S][0]] <= 0;
	      
	      if(rightIsDefault){
		float uvStep = (manipulationStep * 10);

		int sign = currentKeyStates[SDL_SCANCODE_LCTRL] ? 1 : -1; // 1 - ctrl 0 - norm

		float dStep = sign * manipulationStep;

		if((sign == 1 && data->tile->customWalls[data->side].buf[leftWallMap[index1S][0]] - wallD + dStep <= bBlockW) || (sign == -1 && data->tile->customWalls[data->side].buf[leftWallMap[index1S][0]] - wallD + dStep >= 0.01f)) {
		  float dUV = sign * uvStep;
		  
		  data->tile->customWalls[data->side].buf[leftWallMap[index1S][0]] += dStep;
		  data->tile->customWalls[data->side].buf[leftWallMap[index1S][1]] += dStep;
		  data->tile->customWalls[data->side].buf[leftWallMap[index1S][2]] += dStep;

		  data->tile->customWalls[data->side].buf[leftWallMap[index1S][3]] += dUV;
		  data->tile->customWalls[data->side].buf[leftWallMap[index1S][4]] += dUV;
		  data->tile->customWalls[data->side].buf[leftWallMap[index1S][5]] += dUV;

		  vec2i oppositeTile = { 0 };
		  int oppositeSide = -1;

		  if (oppositeTileTo((vec2i) { data->grid.x, data->grid.z }, data->side, &oppositeTile, &oppositeSide)) {
		    grid[data->grid.y][oppositeTile.z][oppositeTile.x].customWalls[oppositeSide].buf[leftWallMap[index1S][0]] += dStep;
		    grid[data->grid.y][oppositeTile.z][oppositeTile.x].customWalls[oppositeSide].buf[leftWallMap[index1S][1]] += dStep;
		    grid[data->grid.y][oppositeTile.z][oppositeTile.x].customWalls[oppositeSide].buf[leftWallMap[index1S][2]] += dStep;
		
		    grid[data->grid.y][oppositeTile.z][oppositeTile.x].customWalls[oppositeSide].buf[leftWallMap[index1S][3]] += dUV;
		    grid[data->grid.y][oppositeTile.z][oppositeTile.x].customWalls[oppositeSide].buf[leftWallMap[index1S][4]] += dUV;
		    grid[data->grid.y][oppositeTile.z][oppositeTile.x].customWalls[oppositeSide].buf[leftWallMap[index1S][5]] += dUV;
		  }
		}
	      }*/
	    }
	    
	    break;
	  }case(SDL_SCANCODE_RIGHT): {
	     const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

	     if(manipulationMode != 0 && (mouse.focusedType == mouseModelT || mouse.focusedType == mousePlaneT)){
	       Matrix* mat = -1;
	       bool isPlane = false;
	       Model* model = NULL;
		  
	       if (mouse.focusedType == mouseModelT) {
		 model = (Model*)mouse.focusedThing;
		 mat = &model->mat;

	       }
	       else if (mouse.focusedType == mousePlaneT) {
		 Picture* plane = (Picture*)mouse.focusedThing;
		 mat = &plane->mat;
		 isPlane = true;
	       }

	       float xTemp = mat->m[12];
	       float yTemp = mat->m[13];
	       float zTemp = mat->m[14];

	       if (manipulationMode == ROTATE_Y || manipulationMode == ROTATE_X || manipulationMode == ROTATE_Z) {
		 mat->m[12] = 0;
		 mat->m[13] = 0;
		 mat->m[14] = -zTemp;

		 if (manipulationMode == ROTATE_Y) {
		   rotateY(mat->m, rad(1.0f));
		 }
		 else if (manipulationMode == ROTATE_X) {
		   rotateX(mat->m, rad(1.0f));
		 }
		 else if (manipulationMode == ROTATE_Z) {
		   rotateZ(mat->m, rad(1.0f));
		 }

		 mat->m[12] = xTemp;
		 mat->m[13] = yTemp;
		 mat->m[14] = zTemp;

		 if(model){
		   calculateModelAABB(model);
		 }
	       }else if(manipulationMode == SCALE && isPlane){
		 Picture* plane = (Picture*)mouse.focusedThing;
		 plane->w += 0.01f;
	       }

	       switch (manipulationMode) {
	       case(TRANSFORM_XY): {
		 mat->m[14] = mat->m[14] - manipulationStep;
		 
		 if(model){
		   calculateModelAABB(model);
		 }
		 
		 break;
	       }

	       default: break;
	       }
	     }else if (mouse.selectedType == mouseWallT) { // + right W
	       /*	       WallMouseData* data = (WallMouseData*)mouse.selectedThing;
	      
	       bool index1S = data->side == right || data->side == left;
	       bool leftIsDefault = data->tile->customWalls[data->side].buf[leftWallMap[index1S][0]] >= bBlockW;

	       if(leftIsDefault){
		 float uvStep = (manipulationStep * 10);

		 int sign = currentKeyStates[SDL_SCANCODE_LCTRL] ? -1 : 1;
		
		 float dStep = sign * manipulationStep;

		 if((sign == 1 && data->tile->customWalls[data->side].buf[rightWallMap[index1S][0]] + dStep <= bBlockW - dStep) || (sign == -1 && data->tile->customWalls[data->side].buf[rightWallMap[index1S][0]] - wallD + dStep >= dStep)) {
		   float dUV = sign * uvStep;
		  
		   data->tile->customWalls[data->side].buf[rightWallMap[index1S][0]] += dStep;
		   data->tile->customWalls[data->side].buf[rightWallMap[index1S][1]] += dStep;
		   data->tile->customWalls[data->side].buf[rightWallMap[index1S][2]] += dStep;

		   data->tile->customWalls[data->side].buf[rightWallMap[index1S][3]] += dUV;
		   data->tile->customWalls[data->side].buf[rightWallMap[index1S][4]] += dUV;
		   data->tile->customWalls[data->side].buf[rightWallMap[index1S][5]] += dUV;

		   vec2i oppositeTile = { 0 };
		   int oppositeSide = -1;

		   if (oppositeTileTo((vec2i) { data->grid.x, data->grid.z }, data->side, &oppositeTile, &oppositeSide)) {
		     grid[data->grid.y][oppositeTile.z][oppositeTile.x].customWalls[oppositeSide].buf[rightWallMap[index1S][0]] += dStep;
		     grid[data->grid.y][oppositeTile.z][oppositeTile.x].customWalls[oppositeSide].buf[rightWallMap[index1S][1]] += dStep;
		     grid[data->grid.y][oppositeTile.z][oppositeTile.x].customWalls[oppositeSide].buf[rightWallMap[index1S][2]] += dStep;
		
		     grid[data->grid.y][oppositeTile.z][oppositeTile.x].customWalls[oppositeSide].buf[rightWallMap[index1S][3]] += dUV;
		     grid[data->grid.y][oppositeTile.z][oppositeTile.x].customWalls[oppositeSide].buf[rightWallMap[index1S][4]] += dUV;
		     grid[data->grid.y][oppositeTile.z][oppositeTile.x].customWalls[oppositeSide].buf[rightWallMap[index1S][5]] += dUV;
		   }
		 }
		 }*/
	     }

	     break;
	   }case(SDL_SCANCODE_SPACE): {
	      cameraMode = !cameraMode;

	      curCamera = cameraMode ? &camera1 : &camera2;

	      break;
	    }
	  case(SDL_SCANCODE_O): {
	    if(!curMenu || curMenu->type == objectsMenuT){
	      objectsMenu.open = !objectsMenu.open;
	      curMenu = objectsMenu.open ? &objectsMenu : NULL;
	    }
	    
	    break;
	  }case(SDL_SCANCODE_B): {
	     if(mouse.focusedType == mouseModelT || mouse.focusedType == mousePlaneT){
	       int* characterId = -1;

	       if (mouse.focusedType == mouseModelT) {
		 Model* model = (Model*)mouse.focusedThing;
		 characterId = &model->characterId;

	       }
	       else if (mouse.focusedType == mousePlaneT) {
		 Picture* plane = (Picture*)mouse.focusedThing;
		 characterId = &plane->characterId;
	       }

	       if (*characterId == -1) {
		 if (characters == NULL) {
		   characters = malloc(sizeof(Character));
		 }
		 else {
		   characters = realloc(characters, (charactersSize + 1) * sizeof(Character));
		   //memset(&characters[charactersSize].dialogs,0,sizeof(Dialog));	
		 }
		 memset(&characters[charactersSize], 0, sizeof(Character));

		 characters[charactersSize].id = charactersSize;
		 characters[charactersSize].modelId = -1;// mouse.focusedModel->id;
		 characters[charactersSize].modelName = -1;// mouse.focusedModel->name;
		 characters[charactersSize].curDialog = &characters[charactersSize].dialogs;

		 characters[charactersSize].curDialog->answersSize = 1;
		 characters[charactersSize].curDialog->answers = calloc(1, sizeof(Dialog));
		 //		printf("Alloc\n");

		 *characterId = characters[charactersSize].id;

		 charactersSize++;
	       }

	       dialogEditor.open = true;
	       curMenu = &dialogEditor;

	       Character* editedCharacter = &characters[*characterId];
	      
	       editedCharacter->curDialog = &editedCharacter->dialogs; 
	       editedCharacter->curDialogIndex = 0;

	       dialogEditor.textInputs[replicaInput].buf = &editedCharacter->curDialog->replicaText;
	       dialogEditor.textInputs[charNameInput].buf = &editedCharacter->name;

	       for(int i=0;i<editedCharacter->dialogs.answersSize;i++){
		 dialogEditor.textInputs[i+answerInput1].buf = &editedCharacter->curDialog->answers[i].text; 
	       }

	     }else if(!curMenu || curMenu->type == blocksMenuT){
	       blocksMenu.open = !blocksMenu.open;
	       curMenu = blocksMenu.open ? &blocksMenu : NULL;
	     }
	    
	     break;
	   }case(SDL_SCANCODE_R): { 
	      const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

	      if (mouse.selectedType == mouseBlockT || mouse.brushType == mouseBlockBrushT) {
		if(!currentKeyStates[SDL_SCANCODE_X] && !currentKeyStates[SDL_SCANCODE_Y] && !currentKeyStates[SDL_SCANCODE_Z]){
		  if(mouse.selectedType == mouseBlockT){
		    TileBlock* block = (TileBlock*) mouse.selectedThing;

		    if(block->rotateAngle == 270){
		      block->rotateAngle = 0;
		    }else{
		      block->rotateAngle += 90;
		    }
			
		    float xTemp = block->mat.m[12];
		    float yTemp = block->mat.m[13];
		    float zTemp = block->mat.m[14];
			
		    rotateY(block->mat.m, rad(90.0f));
			
		    block->mat.m[12] = xTemp;
		    block->mat.m[13] = yTemp;
		    block->mat.m[14] = zTemp;

		    if(block->rotateAngle == 90){
		      block->mat.m[14] += bBlockW;
		    }else if(block->rotateAngle == 180){
		      block->mat.m[12] += bBlockW;
		    }else if(block->rotateAngle == 270){
		      block->mat.m[14] -= bBlockD;
		    }else{
		      block->mat.m[12] -= bBlockW;
		    }
		  }

		  if(mouse.brushType == mouseBlockBrushT){
		    TileBlock* block = (TileBlock*) mouse.brushThing;

		    if(block->rotateAngle == 270){
		      block->rotateAngle = 0;
		    }else{
		      block->rotateAngle += 90;
		    }

		    float xTemp = block->mat.m[12];
		    float yTemp = block->mat.m[13];
		    float zTemp = block->mat.m[14];
			
		    rotateY(block->mat.m, rad(90.0f));
			
		    block->mat.m[12] = xTemp;
		    block->mat.m[13] = yTemp;
		    block->mat.m[14] = zTemp;

		    if(block->rotateAngle == 90){
		      block->mat.m[14] += bBlockW;
		    }else if(block->rotateAngle == 180){
		      block->mat.m[12] += bBlockW;
		    }else if(block->rotateAngle == 270){
		      block->mat.m[14] -= bBlockD;
		    }else{
		      block->mat.m[12] -= bBlockW;
		    }
		  }
		}}

	      break;
	    }case(SDL_SCANCODE_2):{
	       if(mouse.brushThing && mouse.brushType == mouseBlockBrushT){
		 free(mouse.brushThing);
		 mouse.brushThing = NULL;
	       }else if(mouse.brushType == mouseWallBrushT){
		 WallType* type = mouse.brushThing;

		 if(type == windowT){
		   mouse.brushType = 0;
		   free(mouse.brushThing);
		   mouse.brushThing = NULL;
		 }else{
		   *type = windowT;
		 }
	       }else{
		 mouse.brushType = mouseWallBrushT;

		 WallType* type = malloc(sizeof(WallType));
		 *type = windowT;
		 
		 mouse.brushThing = type;
	       }
	       
	    break;
	  }case(SDL_SCANCODE_1):{
	       if(mouse.brushThing && mouse.brushType == mouseBlockBrushT){
		 free(mouse.brushThing);
		 mouse.brushThing = NULL;
	       }else if(mouse.brushType == mouseWallBrushT){
		 WallType* type = mouse.brushThing;

		 if(type == wallT){
		   mouse.brushType = 0;
		   free(mouse.brushThing);
		   mouse.brushThing = NULL;
		 }else{
		   *type = wallT;
		 }
	       }else{
		 mouse.brushType = mouseWallBrushT;

		 WallType* type = malloc(sizeof(WallType));
		 *type = wallT;
		 
		 mouse.brushThing = type;
	       }
	      
	       break;
	     }case(SDL_SCANCODE_3):{
	       if(mouse.brushThing && mouse.brushType == mouseBlockBrushT){
		 free(mouse.brushThing);
		 mouse.brushThing = NULL;
	       }else if(mouse.brushType == mouseWallBrushT){
		 WallType* type = mouse.brushThing;

		 if(type == doorT){
		   mouse.brushType = 0;
		   free(mouse.brushThing);
		   mouse.brushThing = NULL;
		 }else{
		   *type = doorT;
		 }
	       }else{
		 mouse.brushType = mouseWallBrushT;

		 WallType* type = malloc(sizeof(WallType));
		 *type = doorT;
		 
		 mouse.brushThing = type;
	       }
	      
	       break;
	     }
	  case(SDL_SCANCODE_C):{
		/*		const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
		
				if(currentKeyStates[SDL_SCANCODE_LCTRL]){
				if (mouse.selectedType == mouseWallT) {
				mouse.brushType = mouseWallBrushT;
				mouse.brushThing = mouse.selectedThing;
				}
				}*/
		
		break;
	      }case(SDL_SCANCODE_EQUALS): { 
		 if (curFloor < gridY - 1) {
		   curFloor++;
		 }

		 break; 
	       }
	  case(SDL_SCANCODE_MINUS): {
	    if (curFloor != 0) {
	      curFloor--;
	    }

	    break;
	  }
	  case(SDL_SCANCODE_V): {
	    /*const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

	      if(currentKeyStates[SDL_SCANCODE_LCTRL]){
	      if(mouse.brushType == mouseWallBrushT && mouse.brushThing){
	      WallMouseData* dist = (WallMouseData*) mouse.selectedThing;
	      WallMouseData* source = (WallMouseData*) mouse.brushThing;

	      //		int tx = valueIn(source->tile->wallsTx, source->side);
	      //	setIn(dist->tile->wallsTx, dist->side, tx);

	      dist->tile->customWalls[dist->side].bufSize = source->tile->customWalls[source->side].bufSize;
	      dist->tile->customWalls[dist->side].buf = realloc(dist->tile->customWalls[dist->side].buf, dist->tile->customWalls[dist->side].bufSize * sizeof(float));
	      memcpy(dist->tile->customWalls[dist->side].buf, customWallTemp[data->side], sizeof(float) * 6 * 5);[source->side].buf, sizeof(float) * source->tile->customWalls[source->side].bufSize);

		
	      };
	      }*/

	    break;
	  }
	  case(SDL_SCANCODE_N): {
	    // TODO: Come up way to turn on/off fog
	    enviromental.fog = !enviromental.fog;

	    if(enviromental.fog){
	      glUniform1f(radius, drawDistance);
	    }else{
	      glUniform1f(radius, 200.0f);
	    }
				  
	    break;
	  }case(SDL_SCANCODE_P): {
	     const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
			      
	     if((!curMenu || curMenu->type == planeCreatorT) && !currentKeyStates[SDL_SCANCODE_LCTRL]){
	       if(planeOnCreation){
		 free(planeOnCreation);
		 planeOnCreation = NULL;
	       }
	       
	       planeCreatorMenu.open = !planeCreatorMenu.open;
	       curMenu = planeCreatorMenu.open ? &planeCreatorMenu : NULL;
	     }
	     
	     break;
	   }case(SDL_SCANCODE_T):{
	      const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
			      
	      if((!curMenu || curMenu->type == texturesMenuT) && !currentKeyStates[SDL_SCANCODE_LCTRL]){
		texturesMenu.open = !texturesMenu.open;
		curMenu = texturesMenu.open ? &texturesMenu : NULL;
	      }
	     
	      break;
	    }
	  case(SDL_SCANCODE_H): {
	    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

	    if(mouse.selectedType == mouseWallT){
	      WallMouseData* data = (WallMouseData*)mouse.selectedThing;

	      if(data->type == wallJointT){
		data->tile->jointExist[data->side] = !data->tile->jointExist[data->side];
	      }else{
		data->tile->walls[data->side].planes[data->plane].hide = !data->tile->walls[data->side].planes[data->plane].hide;
	      }

	      collectTilesMats();
	    }
	    
	    highlighting = !highlighting;
	    
	    break;
	  }
	  case(SDL_SCANCODE_DELETE): {
	    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
	    
	    if(!currentKeyStates[SDL_SCANCODE_LCTRL]){

	      if(mouse.focusedThing){
	      
		if (mouse.focusedType == mouseModelT) {
		  Model* model = (Model*)mouse.focusedThing;
		  int index = 0;

		  // clear dialogs
		  int charId = curModels[model->id].characterId;

		  destroyCharacter(charId);

		  for (int i = 0; i < curModelsSize; i++) {
		    if (curModels[i].id == model->id){
		      continue;
		    }

		    curModels[index] = curModels[i];
		    index++;
		  }

		  curModelsSize--;
		  curModels = realloc(curModels, curModelsSize * sizeof(Model));
	    
		  mouse.focusedThing = NULL;
		  mouse.focusedType = 0;

		  collectTilesMats();
		}else if (mouse.focusedType == mousePlaneT) {
		  Picture* panel = (Picture*)mouse.focusedThing;
		  int index = 0;

		  // clear dialogs
		  int charId = panel->characterId;

		  destroyCharacter(charId);

		  for (int i = 0; i < curModelsSize; i++) {
		    if (createdPlanes[i].id == panel->id){
		      continue;
		    }

		    createdPlanes[index] = createdPlanes[i];
		    index++;
		  }
		  
		  createdPlanesSize--;
		  createdPlanes = realloc(createdPlanes, createdPlanesSize * sizeof(Model));
	    
		  mouse.focusedThing = NULL;
		  mouse.focusedType = 0;

		  collectTilesMats();
		}
	      }else if (mouse.selectedType == mouseWallT) {
		// WallType type = (grid[mouse.wallTile.y][mouse.wallTile.z][mouse.wallTile.x]->walls >> (mouse.wallSide * 8)) & 0xFF;
		WallMouseData* data = (WallMouseData*)mouse.selectedThing;

		free(data->tile->walls[data->side].planes);
		data->tile->walls[data->side].planes = NULL;

		collectTilesMats();
	      }else if (mouse.selectedType == mouseTileT) {
		// WallType type = (grid[mouse.wallTile.y][mouse.wallTile.z][mouse.wallTile.x]->walls >> (mouse.wallSide * 8)) & 0xFF;
		TileMouseData* data = (TileMouseData*)mouse.selectedThing;

		setIn(data->tile->ground, data->groundInter, 0);
		setIn(data->tile->ground, 0, netTile);

		collectTilesMats();
	      }else if (mouse.selectedType == mouseBlockT) {
		TileBlock* data = (TileBlock*)mouse.selectedThing;

		data->tile->block = NULL;
	
		free(data->vertexes);
		free(data);

		collectTilesMats();
	      }
	    }
			      
	    break;
	  }
	  default: break;
	  }
	}
      }

      if (event.type == SDL_MOUSEBUTTONDOWN) {
	if (event.button.button == SDL_BUTTON_LEFT) {
	  mouse.clickL = true;
	}

	if (event.button.button == SDL_BUTTON_RIGHT) {
	  mouse.clickR = true;
	}
      }

      if(event.type == SDL_MOUSEWHEEL){
	mouse.wheel = event.wheel.y;

	if((mouse.focusedType == mouseModelT || mouse.focusedType == mousePlaneT)){
	  const float dStep = 0.0001f;
	  
	  if(event.wheel.y > 0){
	    manipulationStep += dStep;
	  }else if(event.wheel.y < 0 && manipulationStep - dStep >= dStep){
	    manipulationStep -= dStep;
	  }
	
	  manipulationScaleStep = (manipulationStep * 5) + 1;
	}
      }
      
      if (event.type == SDL_MOUSEMOTION) {
	if(curMenu){
	  float x = -1.0 + 2.0 * (event.motion.x / windowW);
	  float y = -(-1.0 + 2.0 * (event.motion.y / windowH));

	  mouse.cursor.x = x;
	  mouse.cursor.z = y;  
	}

	if (curCamera && !curMenu) { 
	  mouse.screenPos.x = windowW / 2;
	  mouse.screenPos.z = windowH / 2; 

	  float xoffset = event.motion.xrel;
	  float yoffset = -event.motion.yrel;

	  const float sensitivity = 0.1f;
	  xoffset *= sensitivity;
	  yoffset *= sensitivity;

	  // calculate yaw, pitch
	  {
	    curCamera->yaw += xoffset;
	    curCamera->pitch += yoffset;

	    if (curCamera->pitch > 89.0f)
	      curCamera->pitch = 89.0f;
	    if (curCamera->pitch < -89.0f)
	      curCamera->pitch = -89.0f;

	    if (curCamera->yaw > 180.0f)
	      curCamera->yaw = -180.0f;
	    if (curCamera->yaw < -180.0f)
	      curCamera->yaw = 180.0f; 
	    
	    curCamera->dir.x = cos(rad(curCamera->yaw)) * cos(rad(curCamera->pitch));
	    curCamera->dir.y = sin(rad(curCamera->pitch));
	    curCamera->dir.z = sin(rad(curCamera->yaw)) * cos(rad(curCamera->pitch));
	    
	    curCamera->front = normalize3(curCamera->dir);

	    //	    curCamera->Z = normalize3((vec3) { curCamera->front.x * -1.0f, curCamera->front.y * -1.0f, curCamera->front.z * -1.0f });
	    //	    curCamera->X = normalize3(cross3(curCamera->Z, curCamera->up));
	    //	    curCamera->Y = (vec3){ 0,dotf3(curCamera->X, curCamera->Z),0 };
	  }
	}
      }
    }

    if(!console.open && !curMenu){
      const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

      if (currentKeyStates[SDL_SCANCODE_W])
	{
	  if (curCamera) {//cameraMode){
	    vec3 normFront = normalize3(cross3(curCamera->front, curCamera->up));
		    
	    curCamera->pos.x -= cameraSpeed * normFront.x;
	    curCamera->pos.y -= cameraSpeed * normFront.y;
	    curCamera->pos.z -= cameraSpeed * normFront.z;
	  }
	  else {
	    /*
	      float dx = speed * sin(rad(player.angle));
	      float dz = speed * cos(rad(player.angle));

	      vec2 tile = { (player.max.x + dx) / bBlockW, (player.max.z + dz) / bBlockD };

	      bool isIntersect = false;

	      if (grid[curFloor][(int)tile.z][(int)tile.x]->walls != 0) {
	      for (int side = 0; side < basicSideCounter; side++) {
	      WallType type = (grid[curFloor][(int)tile.z][(int)tile.x]->walls >> (side * 8)) & 0xFF;
	      vec3 pos = { (float)(int)tile.x * bBlockW, 0.0f,  (float)(int)tile.z * bBlockD };

	      if (type == wallT) {
	      vec3 rt = { 0 };
	      vec3 lb = { 0 };

	      switch (side) {
	      case(top): {
	      lb = (vec3){ pos.x, pos.y, pos.z };
	      rt = (vec3){ pos.x + bBlockW, pos.y + bBlockH, pos.z + wallD };

	      break;
	      }
	      case(bot): {
	      lb = (vec3){ pos.x, pos.y, pos.z + bBlockD };
	      rt = (vec3){ pos.x + bBlockW, pos.y + bBlockH, pos.z + bBlockD + wallD };

	      break;
	      }
	      case(left): {
	      lb = (vec3){ pos.x, pos.y, pos.z };
	      rt = (vec3){ pos.x + wallD, pos.y + bBlockH, pos.z + bBlockD };

	      break;
	      }
	      case(right): {
	      lb = (vec3){ pos.x + bBlockW, pos.y, pos.z };
	      rt = (vec3){ pos.x + bBlockW,pos.y + bBlockH, pos.z + bBlockD };

	      break;
	      }
	      default: break;
	      }

	      if (player.min.x + dx <= rt.x &&
	      player.max.x + dx >= lb.x &&
	      player.min.y <= rt.y &&
	      player.max.y >= lb.y &&
	      player.min.z + dz <= rt.z &&
	      player.max.z + dz >= lb.z) {
	      isIntersect = true;
	      break;
	      }

	      }
	      }
	      }

	      if (!isIntersect) {
	      player.pos.x += dx;
	      player.pos.z += dz;
	      }

	    */
	  }
	}
      else if (currentKeyStates[SDL_SCANCODE_S])
	{
	  if (curCamera) {//cameraMode){
		    
	    vec3 normFront = normalize3(cross3(curCamera->front, curCamera->up));

	    curCamera->pos.x += cameraSpeed * normFront.x;
	    curCamera->pos.y += cameraSpeed * normFront.y;
	    curCamera->pos.z += cameraSpeed * normFront.z;

	    //	  glUniform3f(cameraPos, argVec3(curCamera->pos));
	  }
	  else {
	    player.pos.x -= speed * sin(rad(player.angle));
	    player.pos.z -= speed * cos(rad(player.angle));
	  }
	}
      else if (currentKeyStates[SDL_SCANCODE_D])
	{
	  if (curCamera) {//cameraMode){
	    curCamera->pos.x += cameraSpeed * curCamera->front.x;
	    curCamera->pos.z += cameraSpeed * curCamera->front.z;

	    //	  glUniform3f(cameraPos, argVec3(curCamera->pos));
	  }
	}
      else if (currentKeyStates[SDL_SCANCODE_A])
	{
	  if (curCamera) {//cameraMode){
	    vec3 normFront = normalize3(cross3(curCamera->front, curCamera->up));

	    curCamera->pos.x -= cameraSpeed * curCamera->front.x;
	    //			  curCamera->pos.y -= cameraSpeed * curCamera->front.y;
	    curCamera->pos.z -= cameraSpeed * curCamera->front.z;
	    //	  glUniform3f(cameraPos, argVec3(curCamera->pos));
	  }
	}

      {

	// ~~~~~~~~~~~~~~~~~~
	// Manipulate of focused model
	//if(!mouse.focusedModel){
	//manipulationMode = -1;
	//}
      
	if ((mouse.focusedType == mouseModelT || mouse.focusedType == mousePlaneT) && currentKeyStates[SDL_SCANCODE_LCTRL]) {

	  if(currentKeyStates[SDL_SCANCODE_P]){
	    if(mouse.selectedType == mouseTileT){
	      Matrix* mat = -1;
	      Model* model = NULL;

	      if (mouse.focusedType == mouseModelT) {
		model = (Model*)mouse.focusedThing;
		mat = &model->mat;

	      }
	      else if (mouse.focusedType == mousePlaneT) {
		Picture* plane = (Picture*)mouse.focusedThing;
		mat = &plane->mat;
	      }
	      TileMouseData* tileData = (TileMouseData*) mouse.selectedThing;
	      vec3 tile = xyz_indexesToCoords(tileData->grid.x, curFloor, tileData->grid.z);

	      mat->m[12] = tile.x;
	      mat->m[13] = tile.y;
	      mat->m[14] = tile.z;

	      if(model){
		calculateModelAABB(model);
	      }
	    }
	  
	  }else if(currentKeyStates[SDL_SCANCODE_R]){
	    // Rotate
	    if (currentKeyStates[SDL_SCANCODE_X]){
	      manipulationMode = ROTATE_X;
	    }else if (currentKeyStates[SDL_SCANCODE_Y]){
	      manipulationMode = ROTATE_Y;
	    }else if (currentKeyStates[SDL_SCANCODE_Z]){
	      manipulationMode = ROTATE_Z;
	    } 
	  }else if(currentKeyStates[SDL_SCANCODE_T]){
	    manipulationMode = TRANSFORM_XY;
	  }else if (currentKeyStates[SDL_SCANCODE_Z]) {
	    manipulationMode = TRANSFORM_Z;
	  }
	  else if(currentKeyStates[SDL_SCANCODE_G]){
	    // Scale
	    manipulationMode = SCALE;
	  }
	}
      }
    }

    mouse.tileSide = -1;
    //  mouse.brushBlock = NULL;

    mouse.selectedThing = NULL;
    mouse.selectedType = 0;


    glClear(GL_COLOR_BUFFER_BIT |
	    GL_DEPTH_BUFFER_BIT);

    // send proj and view mat to shader
    {
      Matrix proj = perspective(rad(fov), windowW / windowH, 0.01f, 100.0f);

      Matrix view =  IDENTITY_MATRIX;
      vec3 negPos = { -curCamera->pos.x, -curCamera->pos.y, -curCamera->pos.z };

      translate(&view, argVec3(negPos));
      rotateY(&view, rad(curCamera->yaw));
      rotateX(&view, rad(curCamera->pitch));

      glUseProgram(lightSourceShader);
      glUniformMatrix4fv(lightProjUni, 1, GL_FALSE, proj.m);
      glUniformMatrix4fv(lightViewLoc, 1, GL_FALSE, view.m);

      glUseProgram(mainShader);
      glUniformMatrix4fv(projUni, 1, GL_FALSE, proj.m);
      glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view.m);

      // modif camera pos to shader
      // vec3 updatedPos = (vec3){ argVec3(mulmatvec4(&view, &(vec4){argVec3(curCamera->pos), 1.0f })) };
      glUniform3f(cameraPos, 0,0,0);// argVec3(updatedPos)); // updated pos always 0, 0, 0

      vec3 front  = ((vec3){ view.m[8], view.m[9], view.m[10] });

      curCamera->Z = normalize3((vec3) { front.x * -1.0f, front.y * 1.0f, front.z * 1.0f });
      curCamera->X = normalize3(cross3(curCamera->Z, curCamera->up));
      curCamera->Y = (vec3){ 0,dotf3(curCamera->X, curCamera->Z),0 };

      // cursor things
      {
	float x = (2.0f * mouse.screenPos.x) / windowW - 1.0f;
	float y = 1.0f - (2.0f * mouse.screenPos.z) / windowH;
	float z = 1.0f;
	vec4 rayClip = { x, y, -1.0, 1.0 };

	Matrix inversedProj = IDENTITY_MATRIX;
      
	inverse(proj.m, inversedProj.m);
	vec4 ray_eye = mulmatvec4(inversedProj, rayClip);

	ray_eye.z = -1.0f;
	ray_eye.w = 0.0f;

	Matrix inversedView = IDENTITY_MATRIX;

	inverse(view.m, inversedView.m);

	vec4 ray_wor = mulmatvec4(inversedView, ray_eye);
	mouse.rayDir = normalize3((vec3) { argVec3(ray_wor) });

	//normalize4(&ray_wor);

      }
    }

    Matrix out = IDENTITY_MATRIX;
    
    glActiveTexture(solidColorTx);
    glBindTexture(GL_TEXTURE_2D, solidColorTx);
    setSolidColorTx(darkPurple, 1.0f);

    out.m[12] = curCamera->pos.x;
    out.m[13] = curCamera->pos.y;
    out.m[14] = curCamera->pos.z;
		
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out.m); 

    if(curFloor != 0)
    {
      glBindBuffer(GL_ARRAY_BUFFER, netTileVBO);
      glBindVertexArray(netTileVAO);
      
      for (int x = 0; x < gridX; x++) {
	for (int z = 0; z < gridZ; z++){
	  vec3 tile = {(float)x * bBlockW, (float)curFloor * floorH, (float)z * bBlockD};
	
	  Matrix out = IDENTITY_MATRIX;
	
	  // translate without mult
	  out.m[12] = tile.x;
	  out.m[13] = tile.y;
	  out.m[14] = tile.z;
		
	  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out.m);

	  glDrawArrays(GL_LINES, 0, 10);
	}
      }


      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      glBindTexture(GL_TEXTURE_2D, 0); 
    }

    /*
    // axises    
    if (false)
    {
    glBegin(GL_LINES);

    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(1.0, 0.0, 0.0);

    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 1.0, 0.0);

    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 1.0);

    glEnd();
    }*/

    if((mouse.focusedType != mouseWallT && mouse.brushType != mouseBlockBrushT &&  mouse.selectedType == mouseWallT) || (mouse.focusedType != mouseTileT && mouse.selectedType == mouseTileT)){
      free(mouse.selectedThing);
    }

    mouse.selectedThing = NULL;
    mouse.selectedType = 0;

    float minDistToCamera = 1000.0f;
	  
    // test 3d model rendering
    {
      // TODO: maybe sort models in curModels by type/mesh
      // and call bindBuffer/bindAttr outside of loop 
      for(int i=0;i<curModelsSize;i++){
	float intersectionDistance = 0.0f;

	bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, curModels[i].lb, curModels[i].rt, NULL, &intersectionDistance);

	int name = curModels[i].name;

	glActiveTexture(loadedModels1D[name].tx);
	glBindTexture(GL_TEXTURE_2D, loadedModels1D[name].tx);

	if(isIntersect && minDistToCamera > intersectionDistance){
	  mouse.selectedThing = &curModels[i];
	  mouse.selectedType = mouseModelT;

	  minDistToCamera = intersectionDistance;
	}

	glBindVertexArray(loadedModels1D[name].VAO);
	glBindBuffer(GL_ARRAY_BUFFER, loadedModels1D[name].VBO);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, curModels[i].mat.m);

	glDrawArrays(GL_TRIANGLES, 0, loadedModels1D[name].size);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//	 vec3* minMax = minMaxPair(curModels[i].vertices, curModels[i].size);

	 
	// draw AABB
	if(highlighting){

	  glActiveTexture(solidColorTx);
	  glBindTexture(GL_TEXTURE_2D, solidColorTx);
	  setSolidColorTx(greenColor, 1.0f);

	  Matrix out = IDENTITY_MATRIX;

	  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out.m);

	  glBegin(GL_LINES);

	
	  glVertex3f(argVec3(curModels[i].lb));
	  glVertex3f(argVec3(curModels[i].rt));
	
	  glVertex3f(curModels[i].rt.x,curModels[i].rt.y,curModels[i].rt.z);
	  glVertex3f(curModels[i].rt.x,curModels[i].lb.y,curModels[i].rt.z);

	  glVertex3f(curModels[i].lb.x,curModels[i].lb.y,curModels[i].lb.z);
	  glVertex3f(curModels[i].lb.x,curModels[i].rt.y,curModels[i].lb.z); 
	
	  glVertex3f(curModels[i].rt.x,curModels[i].rt.y,curModels[i].lb.z);
	  glVertex3f(curModels[i].rt.x,curModels[i].lb.y,curModels[i].lb.z); 
	 
	  glVertex3f(curModels[i].lb.x,curModels[i].rt.y,curModels[i].rt.z);
	  glVertex3f(curModels[i].lb.x,curModels[i].lb.y,curModels[i].rt.z);
	
	  glEnd();

	  setSolidColorTx(darkPurple, 1.0f);
	}
      }

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      glBindTexture(GL_TEXTURE_2D, 0);
    }

    // plane things
    {
      // draw plane on cur creation
      glBindVertexArray(planePairs.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, planePairs.VBO);

      if(planeOnCreation){
	glActiveTexture(errorTx);
	glBindTexture(GL_TEXTURE_2D, errorTx);

	float w = planeOnCreation->w /2;
	float h = planeOnCreation->h /2;

	float planeModel[] = {
	  -w, h, 0.0f, 0.0f, 1.0f,
	  w, h, 0.0f , 1.0f, 1.0f,
	  -w, -h, 0.0f , 0.0f, 0.0f,
	
	  w, h, 0.0f , 1.0f, 1.0f,
	  -w, -h, 0.0f, 0.0f, 0.0f,
	  w, -h, 0.0f, 1.0f, 0.0f
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(planeModel), planeModel, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, planeOnCreation->mat.m);

	glDrawArrays(GL_TRIANGLES, 0, 6);
      }

      // draw planes
      for(int i=0;i<createdPlanesSize;i++){
	float intersectionDistance;

	Picture plane = createdPlanes[i];
	
	float w = plane.w /2;
	float h = plane.h /2;
	
	vec3 lb = { -w, -h, -wallD };
	vec3 rt = { w, h, wallD };

	lb.x += plane.mat.m[12];
	lb.y += plane.mat.m[13];
	lb.z += plane.mat.m[14];
	
	rt.x += plane.mat.m[12];
	rt.y += plane.mat.m[13];
	rt.z += plane.mat.m[14];
	
	bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, lb, rt, NULL, &intersectionDistance);
	
	if(isIntersect && minDistToCamera > intersectionDistance){
	  mouse.selectedThing = &createdPlanes[i];
	  mouse.selectedType = mousePlaneT;

	  minDistToCamera = intersectionDistance;
	}

	glActiveTexture(loadedTextures1D[plane.txIndex].tx);
	glBindTexture(GL_TEXTURE_2D, loadedTextures1D[plane.txIndex].tx);
	

	float planeModel[] = {
	  -w, h, 0.0f, 0.0f, 1.0f,
	  w, h, 0.0f , 1.0f, 1.0f,
	  -w, -h, 0.0f , 0.0f, 0.0f,
	
	  w, h, 0.0f , 1.0f, 1.0f,
	  -w, -h, 0.0f, 0.0f, 0.0f,
	  w, -h, 0.0f, 1.0f, 0.0f
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(planeModel), planeModel, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, plane.mat.m);

	glDrawArrays(GL_TRIANGLES, 0, 6);
      }

      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      glBindTexture(GL_TEXTURE_2D, 0);
    }

    // snowParticles rendering
    if(enviromental.snow)
      {
	glActiveTexture(solidColorTx);
	glBindTexture(GL_TEXTURE_2D, solidColorTx);

	setSolidColorTx(whiteColor, 1.0f);

	glBindVertexArray(snowFlakesMeshVAO);
	glBindBuffer(GL_ARRAY_BUFFER, snowFlakesMeshVBO); 

	int vertexIndex=0;
	for (int loop = 0; loop < snowAmount; loop++)
	  {
	    if (snowParticle[loop].active) {
	      float x = snowParticle[loop].x;
	      float y = snowParticle[loop].y;
	      float z = snowParticle[loop].z;

	      snowMeshVertixes[vertexIndex] = x;
	      snowMeshVertixes[vertexIndex+1] = y;
	      snowMeshVertixes[vertexIndex+2] = z;
	      
	      snowMeshVertixes[vertexIndex+3] = x;
	      snowMeshVertixes[vertexIndex+4] = y - 0.015f / 4.0f;
	      snowMeshVertixes[vertexIndex+5] = z;

	      vertexIndex += 6;
		      
	      vec3i gridIndexes = xyz_coordsToIndexes(x, y, z);

	      GroundType type = -1;

	      //	      if (gridIndexes.y < gridY - 1) {
		//		type = valueIn(grid[gridIndexes.y + 1][gridIndexes.z][gridIndexes.x]->ground, 0);
	      //	      }

	      if (snowParticle[loop].y < 0.0f || type == texturedTile) {
		snowParticle[loop].life -= snowParticle[loop].fade / 10.0f; 

		if (snowParticle[loop].life < 0.0f) {
		  snowParticle[loop].active = true;
		  snowParticle[loop].life = 1.0f;
		  snowParticle[loop].fade = (float)(rand() % 100) / 1000.0f + 0.003f;

		  snowParticle[loop].x = (float)(rand() % gridX / 10.0f) + (float)(rand() % 100 / 1000.0f);
		  snowParticle[loop].y = (gridY - 1) * floorH;
		  snowParticle[loop].z = (float)(rand() % gridZ / 10.0f) + (float)(rand() % 100 / 1000.0f);
		}
	      }
	      else {
		snowParticle[loop].y += snowSpeed / (1000 / 2.0f);
	      }
	    }
	  }
	/*
	  float x = snowParticle[loop].x;
	  float y = snowParticle[loop].y;
	  float z = snowParticle[loop].z;
	*/
	Matrix out = IDENTITY_MATRIX;

	out.m[12] = 0.0;
	out.m[13] = 0.0;
	out.m[14] = 0.0f;
		
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out.m);
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * 3 * snowAmount
		     , snowMeshVertixes, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);
	
	glDrawArrays(GL_LINES, 0, 2 * snowAmount);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	    
	glBindTexture(GL_TEXTURE_2D, 0);
      }

    // main loop of rendering wall/tiles and intersection detecting
    //    ElementType minDistType = -1;
    //    float minIntersectionDist = 1000.0f;

    for (int x = 0; x < gridX; x++) {
      for (int y = 0; y < gridY; y++) {
	for (int z = 0; z < gridZ; z++) { 
	  if (grid[y][z][x]) {
	  vec3 tile = xyz_indexesToCoords(x,y,z);

	  // block
	  if(grid[y][z][x]->block != NULL){

	    vec3 lb = { 1000,1000,1000 };
	    vec3 rt = {0};

	    for(int i3=0;i3<grid[y][z][x]->block->vertexesSize*5;i3 += 5){
	      lb.x= min(lb.x, grid[y][z][x]->block->vertexes[i3]);
	      lb.y= min(lb.y, grid[y][z][x]->block->vertexes[i3+1]);
	      lb.z= min(lb.z, grid[y][z][x]->block->vertexes[i3+2]);

	      rt.x= max(rt.x, grid[y][z][x]->block->vertexes[i3]);
	      rt.y= max(rt.y, grid[y][z][x]->block->vertexes[i3+1]);
	      rt.z= max(rt.z, grid[y][z][x]->block->vertexes[i3+2]);
	    }

	    lb.x += tile.x;
	    lb.y += tile.y;
	    lb.z += tile.z;

	    rt.x += tile.x;
	    rt.y += tile.y;
	    rt.z += tile.z;
		
	    float intersectionDistance;
	    bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, lb, rt, NULL, &intersectionDistance);
	
	    if(isIntersect && minDistToCamera > intersectionDistance){
	      mouse.selectedThing = grid[y][z][x]->block;
	      mouse.selectedType = mouseBlockT;

	      minDistToCamera = intersectionDistance;
	    }
	  }
	  
	  // walls
	  if(true){
	    bool atLeastOneWall = false;
	    
	    for(int side=0;side<basicSideCounter;side++){

	      if(grid[y][z][x]->walls[side].planes != NULL) {
		atLeastOneWall = true;
		// wall in/out camera
		int in=0;

		//	for (int k = 0; k < 4 && in==0; k++) {
		//	  if (radarCheck(wallPos[k]))
		//    in++;
		//	}
	    
		//	if(!in){
		//	  free(wallPos);
		//	  continue;
		//	}

		// wall drawing
	    
		//GLuint txIndex = valueIn(grid[y][z][x]->wallsTx, side);
		WallType type = grid[y][z][x]->walls[side].type;
		WallMouseData* data = malloc(sizeof(WallMouseData));
	      
		if(y >= curFloor){
		  float intersectionDistance;
		  bool atLeastOneIntersect = false; 
		  
		  for(int i=0;i<wallsVPairs[type].planesNum;i++){ 
		    bool isIntersect =  rayIntersectsTriangle(curCamera->pos, mouse.rayDir, grid[y][z][x]->walls[side].planes[i].lb, grid[y][z][x]->walls[side].planes[i].rt, NULL, &intersectionDistance);
 
		    if(isIntersect && minDistToCamera  > intersectionDistance){
		      atLeastOneIntersect = true;
			
		      data->side = side;
		      data->grid = (vec3i){x,y,z};

			  int tx = grid[y][z][x]->walls[side].planes[i].txIndex;

		      data->txIndex = tx;
		      data->tile = grid[y][z][x];

		      data->type = type;
		      data->plane = i;

		      mouse.selectedType = mouseWallT;
		      mouse.selectedThing = data;

		      minDistToCamera = intersectionDistance;
		    }
		      
		  }

		  if(!atLeastOneIntersect){
		    free(data);
		    data = NULL;
		  }
		}
	      }
	    }

	      // joints
	      if(y >= curFloor){
		float intersectionDistance;
		bool atLeastOneIntersect = false;

		WallMouseData* data = malloc(sizeof(WallMouseData));
		  
		for(int i2=0;i2<4;i2++){
		  if(grid[y][z][x]->jointExist[i2] == false){
		    continue;
		  }

		  for(int i=0;i<jointPlaneCounter;i++){	      
		    bool isIntersect =  rayIntersectsTriangle(curCamera->pos, mouse.rayDir, grid[y][z][x]->joint[i2][i].lb, grid[y][z][x]->joint[i2][i].rt, NULL, &intersectionDistance);
 
		    if(isIntersect && minDistToCamera  > intersectionDistance){
		      atLeastOneIntersect = true;
			
		      data->side = i2;
		      data->grid = (vec3i){x,y,z};
		      data->txIndex = grid[y][z][x]->joint[i2][i].txIndex;
		      data->tile = grid[y][z][x];

		      data->type = wallJointT;
		      data->plane = i;

		      mouse.selectedType = mouseWallT;
		      mouse.selectedThing = data;

		      minDistToCamera = intersectionDistance;
		    }
		      
		  }
		}

		if(!atLeastOneIntersect){
		  free(data);
		  data = NULL;
		}
	      }
	  }

	  }
	  // tile inter
	  {

	    // skip netTile on not cur curFloor
	    //if(y != curFloor){
	    //  continue;
	    //}
	    
	    const vec3 tile = xyz_indexesToCoords(x, curFloor, z);

	    const vec3 rt = { tile.x + bBlockW, tile.y, tile.z + bBlockD };
	    const vec3 lb = { tile.x, tile.y, tile.z };

	    /*
	      const vec3 tileCornesrs[4] = { rt, lb, { lb.x, lb.y, lb.z + bBlockD }, { lb.x + bBlockW, lb.y, lb.z } };

	      int in=0;

	      for (int k = 0; k < 4 && in==0; k++) {
	      if (radarCheck(tileCornesrs[k]))
	      in++;
	      }
	    
	      if(!in){
	      continue;
	      }
	    */
	    
	    float intersectionDistance = 0.0f;
	    vec3 intersection = {0};

	    bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir,lb,rt, &intersection, &intersectionDistance);

	    if(isIntersect && minDistToCamera  > intersectionDistance){
	      if (y == curFloor) {
		mouse.selectedType = mouseTileT;
		
		TileMouseData* data = malloc(sizeof(TileMouseData));

		data->tile = grid[y][z][x];
		
		data->grid = (vec2i){x,z};
		data->intersection = intersection;
		data->groundInter = intersection.y <= curCamera->pos.y ? fromOver : fromUnder;

		mouse.selectedThing = data;
		
		minDistToCamera = intersectionDistance;
	      }
	    }
	    // }

	    /*
	      if(isIntersect && minIntersectionDist > intersectionDistance){

	      if (y == curFloor) {
	      minIntersectionDist = intersectionDistance;
		
	      mouse.selectedTile = grid[y][z][x];
	      mouse.gridIntersect = (vec2i){x,z};
	      mouse.intersection = intersection;
	      mouse->groundInter = intersection.y <= curCamera->pos.y ? fromOver : fromUnder;
		 
	      minDistType = TileEl;
	      }
	      }
	    */

	    // tile rendering
	    /*	    if(grid[y][z][x]){
	      GroundType type = valueIn(grid[y][z][x]->ground, 0);
	      
	      if(type == texturedTile){
		const vec3 tile = xyz_indexesToCoords(x,y,z);

		for(int i=1;i<=2;i++){
		  if(y==0 && i == fromUnder){
		    continue;
		  }
		
		  int txIndex = valueIn(grid[y][z][x]->ground, i);
		  float lift = grid[y][z][x]->groundLift;
		  
		  glActiveTexture(loadedTextures1D[txIndex].tx);
		  glBindTexture(GL_TEXTURE_2D, loadedTextures1D[txIndex].tx);

		  glBindBuffer(GL_ARRAY_BUFFER, tileMeshes[i-1].VBO);
		  glBindVertexArray(tileMeshes[i-1].VAO);

		  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, grid[y][z][x]->groundMat.m);

		  glDrawArrays(GL_TRIANGLES, 0, 6);

		  glBindTexture(GL_TEXTURE_2D, 0);
		  glBindBuffer(GL_ARRAY_BUFFER, 0);
		  glBindVertexArray(0);

		}
		
	      }
	    }*/

	  }
	}
      }
    }


    glUseProgram(lightSourceShader);
    
    renderCube(0.5f, 0.5f, 0.5f);
    
    glUseProgram(mainShader);

    glUniform3f(lightColor, 1.0f, 1.0f, 1.0f);
    
    for(int i=0;i<loadedTexturesCounter;i++){
      glBindTexture(GL_TEXTURE_2D, loadedTextures1D[i].tx);

      glBindBuffer(GL_ARRAY_BUFFER, geometry[i].pairs.VBO);
      glBindVertexArray(geometry[i].pairs.VAO);

      Matrix out2 = IDENTITY_MATRIX;
      
      out2.m[12] = 0.0;
      out2.m[13] = 0.0;
      out2.m[14] = 0.0f;

      //      printf("%f %f %f \n", (float)(rand() % 10) / 10.0f, (float)(rand() % 10) / 10.0f, (float)(rand() % 10) / 10.0f);
      //glUniform3f(lightColor, 1.0f, 1.0f, 1.0f);
      
      //      glUniform3f(lightColor, (float)(rand() % 10) / 10.0f, (float)(rand() % 10) / 10.0f, (float)(rand() % 10) / 10.0f);
      
      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out2.m);

      glDrawArrays(GL_TRIANGLES, 0, geometry[i].tris);

      glBindTexture(GL_TEXTURE_2D, 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }


    // setup context text
    {
      if(mouse.focusedThing){
	switch(mouse.focusedType){
	case(mouseModelT):{
	  sprintf(contextBelowText, modelContextText);
	  contextBelowTextH = 2;
	   
	  break;
	} case(mousePlaneT):{
	    sprintf(contextBelowText, planeContextText);
	    contextBelowTextH = 2;
	   
	    break;
	  }
	default: break;
	}
      }else if(mouse.selectedThing){
	switch(mouse.selectedType){
	case(mouseWallT):{
	  //WallMouseData* data = (WallMouseData*) mouse.selectedThing;
	  /*
	  bool itHasBlock = data->tile->block;
	  vec2i oppositeTile;
	  
	  if (!itHasBlock && oppositeTileTo((vec2i) { data->grid.x, data->grid.z }, data->side, &oppositeTile, NULL)) {
	    Tile* tile = grid[curFloor][oppositeTile.z][oppositeTile.x];
	    itHasBlock = tile->block;
	  }*/

	  //sprintf(contextBelowText, selectedWallContextText, itHasBlock ? "[LCtrl + H] aling to block" : "");
	  //contextBelowTextH = 1;
	  
	  break;
	}case(mouseTileT):{
	   sprintf(contextBelowText, tileContextText);
	   contextBelowTextH = 1;
	  
	   break;
	 }case(mouseBlockT):{
	    sprintf(contextBelowText, blockContextText);
	    contextBelowTextH = 1;
	  
	    break;
	  }
	default: break;
	}
      }else{
	sprintf(contextBelowText, generalContextText);
	contextBelowTextH = 1; 
      }
    }

    /*
      if(minDistType == WallEl){
      mouse.selectedTile = NULL;
      mouse.gridIntersect = (vec2i){ -1,-1 };

      mouse.intersection = (vec3){ -1,-1 };
      mouse->groundInter = -1;
      }else if(minDistType == TileEl){
      mouse.wallSide = -1;
      mouse.wallTile = (vec3i){ -1,-1,-1 };
      mouse.wallType = -1;
      mouse.wallTx = mouse.wallTx = -1;
      }*/

    // render mouse.brush
    if(mouse.selectedType == mouseTileT)
      {
	TileMouseData* tileData = (TileMouseData*) mouse.selectedThing;

	const vec3 tile = xyz_indexesToCoords(tileData->grid.x,curFloor, tileData->grid.z);
	
	if(tileData->intersection.x < tile.x + borderArea && tileData->intersection.x >= tile.x - borderArea) {
	  mouse.tileSide = left;
	}
	else {
	  if (tileData->intersection.z < tile.z + borderArea && tileData->intersection.z >= tile.z - borderArea) {
	    mouse.tileSide = top;
	  }
	  else if (tileData->intersection.z > (tile.z + bBlockD / 2) - borderArea && tileData->intersection.z < (tile.z + bBlockD / 2) + borderArea && tileData->intersection.x >(tile.x + bBlockW / 2) - borderArea && tileData->intersection.x < (tile.x +bBlockW/2) + borderArea){
	    mouse.tileSide = center;
	  }else{
	    mouse.tileSide = -1;
	  }
	}

	const float selectionW = borderArea * 3;

	if(mouse.tileSide != -1){
	  glBindVertexArray(selectionRectVAO);
	  glBindBuffer(GL_ARRAY_BUFFER, selectionRectVBO);

	  Matrix out = IDENTITY_MATRIX;

	  out.m[12] = tile.x;
	  out.m[13] = tile.y;
	  out.m[14] = tile.z;
		
	  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out.m);
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);

	static vec3 prevTile = {-1,-1,-1};

	if(mouse.brushType == mouseBlockBrushT){
	  TileBlock* block = (TileBlock*) mouse.brushThing;
	  vec3 tile = xyz_indexesToCoords(tileData->grid.x, curFloor, tileData->grid.z);

	  if((prevTile.x != -1 && prevTile.y != -1 && prevTile.z != -1)
	     && (prevTile.x != tile.x || prevTile.y != tile.y || prevTile.z != tile.z)){
	    block->mat = IDENTITY_MATRIX;

	    rotateY(block->mat.m, rad(block->rotateAngle)); 
			
	    block->mat.m[12] = tile.x;
	    block->mat.m[13] = tile.y;
	    block->mat.m[14] = tile.z;

	    block->tile = tileData->tile;
		
	    bool itWalRotatedAround = false;
		
	    if(block->rotateAngle == 90){
	      block->mat.m[14] += bBlockW;
	    }
	    else if (block->rotateAngle == 180) {
	      block->mat.m[12] += bBlockW;
	      block->mat.m[14] += bBlockW;
	    }
	    else if (block->rotateAngle == 270) {
	      block->mat.m[12] += bBlockD;
		  
	      itWalRotatedAround = true;
	    }
	    else if(itWalRotatedAround){
	      block->mat.m[12] -= bBlockW;
	      itWalRotatedAround = false;
	    }
	  }

	  prevTile.x = tile.x;
	  prevTile.y = tile.y;
	  prevTile.z = tile.z;  

	  glBindVertexArray(block->vpair.VAO); 
	  glBindBuffer(GL_ARRAY_BUFFER, block->vpair.VBO);  

	  glActiveTexture(loadedTextures1D[block->txIndex].tx); 
	  glBindTexture(GL_TEXTURE_2D, loadedTextures1D[block->txIndex].tx);

	  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, block->mat.m);

	  glBufferData(GL_ARRAY_BUFFER, block->vertexesSize * sizeof(float) * 5, block->vertexes, GL_STATIC_DRAW); 

	  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
	  glEnableVertexAttribArray(0);

	  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	  glEnableVertexAttribArray(1);

	  glDrawArrays(GL_TRIANGLES, 0, block->vertexesSize);

	  glBindBuffer(GL_ARRAY_BUFFER, 0);
	  glBindVertexArray(0);
	}
	else if(mouse.brushType == mouseWallBrushT && mouse.tileSide != -1 && mouse.tileSide != center){
	  TileMouseData* tileData = (TileMouseData*)mouse.selectedThing;

	  vec2i curTile = tileData->grid;
	  Side selectedSide = mouse.tileSide;

	  WallType* type = mouse.brushThing;

	  Wall wal = {0};

	  // setup wall
	  {
	    memset(&wal, 0, sizeof(Wall));

	    wal.type = *type;
	    wal.mat = IDENTITY_MATRIX;

	    wal.mat.m[12] = tile.x;
	    wal.mat.m[13] = tile.y;
	    wal.mat.m[14] = tile.z;
	  }
	  
	  static const int rotationPad[4][3] = {
	    [bot]= { 180, 1, 1 },
	    [top]= { 0, 0, 0  },
	    [left]= { 270, 0, 0 }, 
	    [right]= { 90, 1, 1}//{ 180, 14, 1, 12, 1 }
	  };

	  // rotate wall to selectedSide
	  {
	    Matrix* mat = &wal.mat;

	    float xTemp = mat->m[12];
	    float yTemp = mat->m[13];
	    float zTemp = mat->m[14];

	    rotateY(wal.mat.m, rad(rotationPad[selectedSide][0]));

	    mat->m[12] = xTemp;
	    mat->m[13] = yTemp;
	    mat->m[14] = zTemp;

	    mat->m[12] += rotationPad[selectedSide][2];
	    mat->m[14] += rotationPad[selectedSide][1];
	  }

	  // brush phantom
	  {
	    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, wal.mat.m);
	    
	    glBindTexture(GL_TEXTURE_2D, loadedTextures1D[0].tx);
	  
	    for(int i=0;i<wallsVPairs[wal.type].planesNum;i++){
	      glBindVertexArray(wallsVPairs[wal.type].pairs[i].VAO);
	      glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[wal.type].pairs[i].VBO);

	      glDrawArrays(GL_TRIANGLES, 0, wallsVPairs[wal.type].pairs[i].vertexNum);
	      
	      glBindVertexArray(0);
	      glBindBuffer(GL_ARRAY_BUFFER, 0);
	    }
	    
	    glBindTexture(GL_TEXTURE_2D, 0);
	    glBindVertexArray(0);
	    glBindBuffer(GL_ARRAY_BUFFER, 0);
	  }
	  
	  vec3 tile = xyz_indexesToCoords(tileData->grid.x,curFloor,tileData->grid.z);

	  Tile* opTile = NULL;

	  Side oppositeSide = 0; 
	  vec2i oppositeTile = {0};  
	  
	  if(oppositeTileTo(tileData->grid, mouse.tileSide,&oppositeTile,&oppositeSide)){
	    opTile = grid[curFloor][oppositeTile.z][oppositeTile.x];
	  }

	  if(mouse.clickR &&
	     (!opTile || !opTile->walls[oppositeSide].planes)){ 
	    
	    wal.planes = calloc(wallsVPairs[wal.type].planesNum, sizeof(Plane));

	    if(!tileData->tile){
	      grid[curFloor][tileData->grid.z][tileData->grid.x] = calloc(1,sizeof(Tile));
	      tileData->tile = grid[curFloor][tileData->grid.z][tileData->grid.x];
	    }
	    
	    for(int i=0;i<wallsVPairs[wal.type].planesNum;i++){
	      calculateAABB(wal.mat, wallsVPairs[wal.type].pairs[i].vBuf, wallsVPairs[wal.type].pairs[i].vertexNum, &wal.planes[i].lb, &wal.planes[i].rt);
	    }

	    if(selectedSide == left){
	      bool botInner = tileData->tile->walls[top].planes;

	      bool botOuter = tileData->grid.x - 1 >= 0 && grid[curFloor][tileData->grid.z][tileData->grid.x - 1] && grid[curFloor][tileData->grid.z][tileData->grid.x - 1]->walls[top].planes;

	      bool botPar = tileData->grid.z + 1 < gridZ && grid[curFloor][tileData->grid.z + 1][tileData->grid.x] && grid[curFloor][tileData->grid.z + 1][tileData->grid.x]->walls[left].planes;

	      if(botPar){
		grid[curFloor][tileData->grid.z + 1][tileData->grid.x]->jointExist[top] = false;

		if (grid[curFloor][tileData->grid.z + 1][tileData->grid.x - 1]) {
		grid[curFloor][tileData->grid.z + 1][tileData->grid.x - 1]->jointExist[right] = false;
		}
	      }

	      bool topPar = tileData->grid.x - 1 >= 0 && grid[curFloor][tileData->grid.z - 1][tileData->grid.x] && grid[curFloor][tileData->grid.z - 1][tileData->grid.x]->walls[left].planes;
	      
	      if(topPar){
			  if (grid[curFloor][tileData->grid.z - 1][tileData->grid.x - 1]) {
		grid[curFloor][tileData->grid.z - 1][tileData->grid.x-1]->jointExist[left] = false;
			  }
		grid[curFloor][tileData->grid.z][tileData->grid.x]->jointExist[bot] = false;
	      }

	      if((!botInner || !botOuter) && !topPar){
		if(botInner){
		  tileData->tile->jointExist[top] = true;
		  setupAABBAndMatForJoint(tileData->grid, top);
		}

		if(botOuter){
		  if(!grid[curFloor][tileData->grid.z][tileData->grid.x - 1]){
		    grid[curFloor][tileData->grid.z][tileData->grid.x - 1] = calloc(1, sizeof(Tile));
		  }
		  
		  grid[curFloor][tileData->grid.z][tileData->grid.x - 1]->jointExist[right] = true;
		  setupAABBAndMatForJoint((vec2i){ tileData->grid.x - 1, tileData->grid.z }, right);
		}
	      }

	      bool topOuter = tileData->grid.z + 1 < gridZ && grid[curFloor][tileData->grid.z + 1][tileData->grid.x] && grid[curFloor][tileData->grid.z + 1][tileData->grid.x]->walls[top].planes;
	      
	      bool topInner = (tileData->grid.z + 1 < gridZ) && (tileData->grid.x - 1 >= 0) && grid[curFloor][tileData->grid.z + 1][tileData->grid.x - 1] && grid[curFloor][tileData->grid.z + 1][tileData->grid.x - 1]->walls[top].planes;
	      
	      if((!topInner || !topOuter) && !botPar){
		if(topInner){
		  if(!grid[curFloor][tileData->grid.z][tileData->grid.x-1]){
		    grid[curFloor][tileData->grid.z][tileData->grid.x-1] = calloc(1, sizeof(Tile));
		  }

		  grid[curFloor][tileData->grid.z][tileData->grid.x-1]->jointExist[left] = true;
		  setupAABBAndMatForJoint((vec2i){ tileData->grid.x - 1, tileData->grid.z }, left);
		}

		if(topOuter){
		  if(!grid[curFloor][tileData->grid.z+1][tileData->grid.x]){
		    grid[curFloor][tileData->grid.z+1][tileData->grid.x] = calloc(1, sizeof(Tile));
		  }
		  
		  grid[curFloor][tileData->grid.z+1][tileData->grid.x]->jointExist[bot] = true;
		  setupAABBAndMatForJoint((vec2i){ tileData->grid.x, tileData->grid.z + 1 }, bot);
		}
	      } 
	    }else if(selectedSide == top){
	      bool leftInner = tileData->tile->walls[left].planes;
	      
	      bool leftOuter = tileData->grid.z - 1 >= 0 && grid[curFloor][tileData->grid.z - 1][tileData->grid.x] && grid[curFloor][tileData->grid.z - 1][tileData->grid.x]->walls[left].planes;

	      bool leftPar = tileData->grid.x - 1 >= 0 && grid[curFloor][tileData->grid.z][tileData->grid.x - 1] && grid[curFloor][tileData->grid.z][tileData->grid.x - 1]->walls[top].planes;

	      if(leftPar){
		grid[curFloor][tileData->grid.z][tileData->grid.x - 1]->jointExist[right] = false;

		if (grid[curFloor][tileData->grid.z - 1][tileData->grid.x - 1]) {
		grid[curFloor][tileData->grid.z-1][tileData->grid.x-1]->jointExist[left] = false; 
		}
	      }

	      bool rightPar = tileData->grid.x + 1 < gridX && grid[curFloor][tileData->grid.z][tileData->grid.x + 1] && grid[curFloor][tileData->grid.z][tileData->grid.x + 1]->walls[top].planes;

	      if(rightPar){
		grid[curFloor][tileData->grid.z][tileData->grid.x + 1]->jointExist[bot] = false;
		grid[curFloor][tileData->grid.z][tileData->grid.x + 1]->jointExist[top] = false;
	      }

	      if((!leftInner || !leftOuter) && !leftPar){
		if(leftInner){
		  printf("Inner\n");
		  tileData->tile->jointExist[top] = true;
		  setupAABBAndMatForJoint(tileData->grid, top);
		}

		if(leftOuter){
		  printf("Out\n");
		  tileData->tile->jointExist[bot] = true;
		  setupAABBAndMatForJoint(tileData->grid, bot);
		}
	      }

	      bool rightInner = tileData->grid.x + 1 < gridX && grid[curFloor][tileData->grid.z][tileData->grid.x +1] && grid[curFloor][tileData->grid.z][tileData->grid.x +1]->walls[left].planes;
	      
	      bool rightOuter = (tileData->grid.z - 1 >= 0) && (tileData->grid.x + 1 < gridX) && grid[curFloor][tileData->grid.z - 1][tileData->grid.x + 1] && grid[curFloor][tileData->grid.z - 1][tileData->grid.x + 1]->walls[left].planes;

	      if((!rightInner || !rightOuter) && !rightPar){
		if(rightInner){
		  tileData->tile->jointExist[right] = true;
		  setupAABBAndMatForJoint(tileData->grid, right);
		}

		if(rightOuter){
		  if(!grid[curFloor][tileData->grid.z - 1][tileData->grid.x]){
		    grid[curFloor][tileData->grid.z - 1][tileData->grid.x] = calloc(1, sizeof(Tile));
		  }
		  
		  grid[curFloor][tileData->grid.z - 1][tileData->grid.x]->jointExist[left] = true;
		  setupAABBAndMatForJoint((vec2i){ tileData->grid.x, tileData->grid.z - 1 }, left);
		}
	      } 
	    }

	    memcpy(&grid[curFloor][(int)curTile.z][(int)curTile.x]->walls[selectedSide], &wal, sizeof(Wall));

	    collectTilesMats();
	  }
	}
      }

    // higlight drawing
    {
      glActiveTexture(solidColorTx);
      glBindTexture(GL_TEXTURE_2D, solidColorTx);
      setSolidColorTx(redColor, 1.0f);

      // higlight intersected wall with min dist
      if(highlighting && mouse.selectedType == mouseWallT)
	{
	  WallMouseData* data = (WallMouseData*) mouse.selectedThing;
	  //mouse.interDist = minIntersectionDist;
      
	  vec3 pos = xyz_indexesToCoords(data->grid.x, data->grid.y, data->grid.z);
		
	  Matrix out = IDENTITY_MATRIX;

	  // translate without mult
	  out.m[12] = pos.x;
	  out.m[13] = pos.y;
	  out.m[14] = pos.z;
		
	  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out.m);

	  glBindVertexArray(wallHighlight[data->side].VAO);
	  glBindBuffer(GL_ARRAY_BUFFER, wallHighlight[data->side].VBO);
	  glDrawArrays(GL_TRIANGLES, 0, wallHighlight[data->side].VBOsize);
	  //vec3* wallPos = wallPosBySide(pos, mouse.wallSide, bBlockH, wallD, bBlockD, bBlockW);
	  //  renderWallBorder(wallPos,mouse.wallSide, selBorderT, redColor);
	}else{
	mouse.interDist = 0.0f;
      }

      //highlight intersected tile
  
      if(highlighting && mouse.selectedType == mouseTileT){
	TileMouseData* tileData = (TileMouseData*)mouse.selectedThing;

	vec3 tile = xyz_indexesToCoords(tileData->grid.x, curFloor, tileData->grid.z);

	glBindVertexArray(tileHighlight[tileData->groundInter - 1].VAO);
	glBindBuffer(GL_ARRAY_BUFFER, tileHighlight[tileData->groundInter-1].VBO);
		
	Matrix out = IDENTITY_MATRIX;

	// translate without mult
	out.m[12] = tile.x;
	out.m[13] = tile.y;
	out.m[14] = tile.z;
		
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out.m);

	glDrawArrays(GL_TRIANGLES, 0, tileHighlight[tileData->groundInter-1].VBOsize);

	//}
      }
    
      glBindTexture(GL_TEXTURE_2D, 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }

    // process logic of objs
    for(int i=0;i<objsStoreSize;i++){
      Object* obj = objsStore[i];

      if(obj->anim.frames != 0){
	obj->anim.frames--;
      }
    }
    
    /*
    // render player
    {
    if(mouse.intersection.x != -1 && mouse.intersection.z != -1){
    player.angle = atan2(mouse.intersection.x - (player.pos.x + 0.1f/2), mouse.intersection.z - (player.pos.z + 0.1f/2)) * 180 / M_PI;
    }

    const float headH = player.h / 6;
    const float bodyD = player.d * 0.75f;

    float modelview[16];

    glPushMatrix();
    glTranslatef(player.pos.x,0.0f, player.pos.z + (entityD * 0.75f)/2);
    glRotatef(player.angle, 0.0f, 1.0f, 0.0f);
    glTranslatef(-player.pos.x, -player.pos.y, -player.pos.z - (entityD * 0.75f)/2);

    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
      
    // draw humanoid
    {
    vec3 centrPos = { player.pos.x, player.pos.y + (curFloor * bBlockH), player.pos.z };

    // head
    renderCube((vec3){ player.pos.x - headH/2, centrPos.y + player.h - headH, centrPos.z }, headH, headH, headH, greenColor);

    // body
    renderCube((vec3){ player.pos.x - (player.w/3)/2, centrPos.y, centrPos.z }, player.w/3, player.h - headH, bodyD ,greenColor);

    const float armH = 0.08f;

    // r arm
    renderCube((vec3){ player.pos.x - ( (player.w/3)/2) * 3, player.h + centrPos.y - armH - headH - 0.01f, centrPos.z }, player.w/3, armH , bodyD ,greenColor);

    // l arm
    renderCube((vec3){ player.pos.x +  (player.w/3)/2, player.h + centrPos.y - armH - headH - 0.01f, centrPos.z }, player.w/3, armH, bodyD ,greenColor);

    glPopMatrix();
    }
    */


    // 2d ui drawing
    glDisable(GL_DEPTH_TEST);
    glUseProgram(hudShader);

    // aim render
    if(!curMenu && hints){
      glBindVertexArray(hudRect.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, hudRect.VBO);

      glActiveTexture(solidColorTx);
      glBindTexture(GL_TEXTURE_2D, solidColorTx);
      setSolidColorTx(blackColor, 1.0f);

      float aimW = 0.003f;
      float aimH = 0.006f;

      float aim[] = {
	-aimW, aimH, 1.0f, 0.0f,
	aimW, aimH, 1.0f, 1.0f,
	-aimW, -aimH, 0.0f, 0.0f,

	aimW, aimH, 1.0f, 1.0f,
	-aimW, -aimH, 0.0f, 0.0f,
	aimW, -aimH, 0.0f, 1.0f };

      glBufferData(GL_ARRAY_BUFFER, sizeof(aim), aim, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
      glEnableVertexAttribArray(1);

      glDrawArrays(GL_TRIANGLES, 0, 6);
    
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }

    float baseYPad = 0.0f;
    
    // brush handle usage
    if((mouse.brushType || mouse.brushThing) && !curMenu && hints){
      baseYPad = letterH;
      
      char buf[130];
      
      sprintf(buf, "On brush [%s] [X] to discard", mouseBrushTypeStr[mouse.brushType]);

      /*
	{
	glActiveTexture(solidColorTx);
	glBindTexture(GL_TEXTURE_2D, solidColorTx);
	setSolidColorTx(blackColor, 1.0f);
	
	glBindVertexArray(hudRect.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, hudRect.VBO);

	float backgroundW = (strlen(buf) * letterW) + letterW;
	    
	float brushNotif[] = {
	-1.0f, 1.0f, 0.0f, 1.0f,
	-1.0f + backgroundW, 1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f - letterH, 0.0f, 0.0f,

	-1.0f + backgroundW, 1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f - letterH, 1.0f, 0.0f,
	-1.0f + backgroundW, 1.0f - letterH, 0.0f, 0.0f, 
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(brushNotif), brushNotif, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLES, 0, 6);
    
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	} */


      renderText(buf, -1.0f, 1.0f, 1.0f);

      switch(mouse.brushType){
      case(mouseTextureBrushT):{
	if(mouse.clickL){ 
	  Texture* texture = (Texture*)mouse.brushThing;
	  
	  if(mouse.selectedType == mouseWallT){
	    WallMouseData* wallData = (WallMouseData*)mouse.selectedThing;

	    if(wallData->type == windowT && wallData->plane <= winInnerBotPlane){
	      const int winPlanePairs[6] = {
		[winFrontCapPlane] = winFrontBotPlane,
		[winFrontBotPlane] = winFrontCapPlane,
	      
		[winBackBotPlane] = winBackCapPlane,
		[winBackCapPlane] = winBackBotPlane,
	      
		[winInnerTopPlane] = winInnerBotPlane,
		[winInnerBotPlane] = winInnerTopPlane, 
	      };

	      wallData->tile->walls[wallData->side].planes[winPlanePairs[wallData->plane]].txIndex = texture->index1D;  
	    }
	    
	    if(wallData->type == wallJointT){
	      wallData->tile->joint[wallData->side][wallData->plane].txIndex = texture->index1D;
	    }else{
	      wallData->tile->walls[wallData->side].planes[wallData->plane].txIndex = texture->index1D;
	    }

	    collectTilesMats();
	  }else if(mouse.selectedType == mouseTileT){
	    TileMouseData* tileData = (TileMouseData*)mouse.selectedThing; 
	    
	    setIn(tileData->tile->ground, 0, texturedTile);
	    setIn(tileData->tile->ground, tileData->groundInter, texture->index1D);
	    collectTilesMats();
	  }else if(mouse.selectedType == mouseBlockT){
	    TileBlock* block = (TileBlock*)mouse.selectedThing;

	    block->txIndex = texture->index1D;
	    collectTilesMats();
	  }else if(mouse.selectedType == mousePlaneT){
	    Picture* plane = (Picture*)mouse.selectedThing;

	    plane->txIndex = texture->index1D;
	    collectTilesMats();
	  }
	}
	  
	break;
      }case(mouseBlockBrushT):{
	 if (mouse.clickR && mouse.selectedType == mouseTileT) {
	   TileMouseData* tileData = (TileMouseData*) mouse.selectedThing;
	   TileBlock* block = (TileBlock*) mouse.brushThing;

	   tileData->tile->block = block;
		 
	   mouse.brushThing = constructNewBlock(block->type, block->rotateAngle);
	   collectTilesMats();
	 }
	       
	 break;
       }
      default: break;
      }
    }

    // render selected or focused thing
    if(mouse.focusedThing && !curMenu && hints){
      char buf[164];
      
      switch(mouse.focusedType){
      case(mouseModelT):{
	Model* data = (Model*)mouse.focusedThing;
	  
	sprintf(buf, "Focused model: [%s] Mode: [%s] Step:[%f]", loadedModels1D[data->name].name, manipulationModeStr[manipulationMode], manipulationStep);
	
	break;
      }case(mousePlaneT):{
	 Picture* data = (Picture*)mouse.focusedThing;
	  
	 sprintf(buf, "Focused plane id: [%d] Mode: [%s] Step: [%f]", data->id, manipulationModeStr[manipulationMode], manipulationStep);
	
	 break;
       }
      default: break;
      }

      renderText(buf, -1.0f, 1.0f - baseYPad, 1.0f);
    }else if(mouse.selectedThing && !curMenu && hints){
      char buf[164];

      switch(mouse.selectedType){
      case(mouseModelT):{
	Model* data = (Model*)mouse.selectedThing;
	  
	sprintf(buf, "Selected model: %s", loadedModels1D[data->name].name);
	
	break;
      }case(mouseWallT):{
	 WallMouseData* data = (WallMouseData*)mouse.selectedThing;
	 
	 char* plane = "NULL";

	 if(data->type == doorT){
	   plane = doorPlanesStr[data->plane];
	 }else if(data->type == wallT){
	   plane = wallPlanesStr[data->plane];
	 }else if(data->type == windowT){
	   plane = windowPlanesStr[data->plane];
	 }else if(data->type == wallJointT){
	   plane = wallJointPlanesStr[data->plane];
	 }
	 
	 sprintf(buf, "Selected wall: [%s] type: [%s] plane: [%s] with tx: [%s]",
		 sidesToStr[data->side],
		 wallTypeStr[data->type],
		 plane,
		 loadedTexturesNames[data->txIndex]);
	
	 break;
       }case(mouseBlockT):{
	  TileBlock* data = (TileBlock*)mouse.selectedThing;
	  
	  sprintf(buf, "Selected block: [%s]", tileBlocksStr[data->type]);
	
	  break;
	}case(mousePlaneT):{
	   Picture* data = (Picture*)mouse.selectedThing;
	  
	   sprintf(buf, "Selected plane: [ID: %d] tx: [%s]", data->id, loadedTexturesNames[data->txIndex]);
	
	   break;
	 }case(mouseTileT):{
	    TileMouseData* data = (TileMouseData*)mouse.selectedThing;

	    if(data->tile){
	      int tileTx = valueIn(data->tile->ground, data->groundInter);
	      sprintf(buf, "Selected tile tx: [%s]", loadedTexturesNames[tileTx]);
	    }else{
	      sprintf(buf, "Selected tile (NULL)");  
	    }
	
	    break; 
	  }
      default: break;
      }
       
      renderText(buf, -1.0f, 1.0f - baseYPad, 1.0f);    
    }
    
    // render objects menu
    if(curMenu && curMenu->type == objectsMenuT){
      glActiveTexture(solidColorTx);
      glBindTexture(GL_TEXTURE_2D, solidColorTx);
      setSolidColorTx(blackColor, 1.0f);
      
      glBindVertexArray(objectsMenu.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, objectsMenu.VBO);

      glDrawArrays(GL_TRIANGLES, 0, 6);
    
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      float mouseY = mouse.cursor.z + 0.06f;
      
      int selectedIndex = (1.0f-((mouseY +1.0f)/2.0f)) / (letterH/2);
      selectedIndex++;
      
      if(mouse.cursor.x <= objectsMenuWidth && selectedIndex <= modelsTypesInfo[objectsMenuSelectedType].counter){
	if(mouse.clickL){
	  mouse.focusedThing = NULL;
	  mouse.focusedType = 0;
	  
	  curModelsSize++;

	  if(curModels){ 
	    curModels = realloc(curModels, curModelsSize * sizeof(Model)); 
	  }else{
	    curModels = malloc(sizeof(Model)); 
	  }

	  curModels[curModelsSize-1].id = curModelsSize-1;

	  int index1D = loadedModels2D[objectsMenuSelectedType][selectedIndex - 1].index1D;

	  curModels[curModelsSize-1].name = index1D;
	  curModels[curModelsSize-1].characterId = -1; 

	  // if type == char add new character
	  
	  
	  curModels[curModelsSize-1].mat = IDENTITY_MATRIX;  

	  scale(&curModels[curModelsSize-1].mat, 0.25f, 0.25f, 0.25f); 

	  if(mouse.selectedType == mouseTileT){
	    TileMouseData* tileData =  (TileMouseData*) mouse.selectedThing;
	    vec3 tile = xyz_indexesToCoords(tileData->grid.x, curFloor, tileData->grid.z);
	    
	    curModels[curModelsSize-1].mat.m[12] = tile.x;
	    curModels[curModelsSize-1].mat.m[13] = tile.y;
	    curModels[curModelsSize-1].mat.m[14] = tile.z;
	  }
	  
	  calculateModelAABB(&curModels[curModelsSize-1]);
	  
	  objectsMenu.open = false;
	  curMenu = NULL;
	};

	setSolidColorTx(redColor, 1.0f);
      
	glBindVertexArray(selectionRectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, selectionRectVBO);
	
	float cursorH = 0.06f;
	float cursorW = 0.02f;
	  
	float  selectionRect[] = {
	  -1.0f, 1.0f - (selectedIndex-1) * letterH, 1.0f, 0.0f,
	  objectsMenuWidth, 1.0f - (selectedIndex-1) * letterH, 1.0f, 1.0f,
	  -1.0f, 1.0f - (selectedIndex) * letterH, 0.0f, 0.0f,

	  objectsMenuWidth, 1.0f - (selectedIndex-1) * letterH, 1.0f, 1.0f,
	  -1.0f,  1.0f - selectedIndex * letterH, 0.0f, 0.0f,
	  objectsMenuWidth,  1.0f - selectedIndex * letterH, 0.0f, 1.0f };

	glBufferData(GL_ARRAY_BUFFER, sizeof(selectionRect), selectionRect, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
      }

      // types of models
      {
	glActiveTexture(solidColorTx);
	glBindTexture(GL_TEXTURE_2D, solidColorTx);
	
	float cursorH = 0.06f;
	float cursorW = 0.02f;

	float typeButtonW = (1.0f + objectsMenuWidth) / 2;

	float baseX = objectsMenuWidth;

	// change selection
	{
	  if((mouse.cursor.x >= baseX && mouse.cursor.x <= baseX + typeButtonW) && selectedIndex <= modelTypeCounter){
	    if(mouse.clickL){
	      objectsMenuSelectedType = selectedIndex - 1; 
	    }
	  }
	}
	
	glBindVertexArray(objectsMenuTypeRectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, objectsMenuTypeRectVBO);
	  
	for(int i=0; i<modelTypeCounter;i++){
	  if(i == objectsMenuSelectedType){
	    setSolidColorTx(redColor, 1.0f);
	  }else{
	    setSolidColorTx(blackColor, 1.0f);
	  }

	  float typeRect[] = {
	    baseX, 1.0f - (i) * letterH, 1.0f, 0.0f,
	    baseX + typeButtonW, 1.0f - (i) * letterH, 1.0f, 1.0f,
	    baseX, 1.0f - (i+1) * letterH, 0.0f, 0.0f,

	    baseX + typeButtonW, 1.0f - (i) * letterH, 1.0f, 1.0f,
	    baseX, 1.0f - (i+1) * letterH, 0.0f, 0.0f,
	    baseX + typeButtonW, 1.0f - (i+1) * letterH, 0.0f, 1.0f
	  };

	  glBufferData(GL_ARRAY_BUFFER, sizeof(typeRect), typeRect, GL_STATIC_DRAW);

	  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	  glEnableVertexAttribArray(0);

	  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	  glEnableVertexAttribArray(1);

	  glDrawArrays(GL_TRIANGLES, 0, 6);
	  
	}
	// TODO: Come up how to merge these two loops
	// it throws exeption if put it into loop above
	for (int i = 0; i < modelTypeCounter; i++) {
	  renderText(modelsTypesInfo[i].str, baseX, 1.0f - ((i) * letterH), 1.0f);
	}
      }

      for(int i=0;i<modelsTypesInfo[objectsMenuSelectedType].counter;i++){
	renderText(loadedModels2D[objectsMenuSelectedType][i].name, -1.0f, 1.0f - (i * letterH), 1);
      }
    }else if(curMenu && curMenu->type == blocksMenuT){
      glActiveTexture(solidColorTx);
      glBindTexture(GL_TEXTURE_2D, solidColorTx);
      setSolidColorTx(blackColor, 1.0f);
      
      glBindVertexArray(objectsMenu.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, objectsMenu.VBO);

      glDrawArrays(GL_TRIANGLES, 0, 6);
    
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      float mouseY = mouse.cursor.z + 0.06f;
      
      int selectedIndex = (1.0f-((mouseY +1.0f)/2.0f)) / (letterH/2);
      selectedIndex++;
      
      if(mouse.cursor.x <= objectsMenuWidth && selectedIndex <= tileBlocksCounter){
	if(mouse.clickL){
	  if(mouse.brushType == mouseBlockBrushT){
	    TileBlock* block = (TileBlock*)mouse.brushThing;
	    free(block->vertexes);
	    free(block);
		
	    mouse.brushType = 0;
	    mouse.brushThing = NULL;
	  }

	  if(mouse.brushType == mouseBlockBrushT){
	    free(mouse.brushThing);
	  }

	  mouse.brushThing = constructNewBlock(selectedIndex-1, 0);
	  mouse.brushType = mouseBlockBrushT;
	  
	  curMenu->open = false; 
	  curMenu = NULL;
	};

	setSolidColorTx(redColor, 1.0f);
      
	glBindVertexArray(selectionRectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, selectionRectVBO);
	
	float cursorH = 0.06f;
	float cursorW = 0.02f;
	  
	float  selectionRect[] = {
	  -1.0f, 1.0f - (selectedIndex-1) * letterH, 1.0f, 0.0f,
	  objectsMenuWidth, 1.0f - (selectedIndex-1) * letterH, 1.0f, 1.0f,
	  -1.0f, 1.0f - (selectedIndex) * letterH, 0.0f, 0.0f,

	  objectsMenuWidth, 1.0f - (selectedIndex-1) * letterH, 1.0f, 1.0f,
	  -1.0f,  1.0f - selectedIndex * letterH, 0.0f, 0.0f,
	  objectsMenuWidth,  1.0f - selectedIndex * letterH, 0.0f, 1.0f };

	glBufferData(GL_ARRAY_BUFFER, sizeof(selectionRect), selectionRect, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
      }

      for(int i=0;i<tileBlocksCounter;i++){
	renderText(tileBlocksStr[i], -1.0f, 1.0f - (i * letterH), 1);
      }
    }else if(curMenu && curMenu->type == texturesMenuT){
      glActiveTexture(solidColorTx);
      glBindTexture(GL_TEXTURE_2D, solidColorTx);
      setSolidColorTx(blackColor, 1.0f);

      glBindVertexArray(hudRect.VAO); 
      glBindBuffer(GL_ARRAY_BUFFER, hudRect.VBO);

      float textureSideW = (longestTextureNameLen * letterW) + letterW;

      float categorySideW = (longestTextureNameLen * letterW) + letterW;
      float categorySideH = loadedTexturesCategoryCounter * letterH;
      
      char buf[150];

      // texture selection
      {
	float textureSide[] = {
	  -1.0f, 1.0f, 1.0f, 0.0f,
	  -1.0f + textureSideW, 1.0f, 1.0f, 1.0f,
	  -1.0f, -1.0f, 0.0f, 0.0f,

	  -1.0f + textureSideW, 1.0f, 1.0f, 1.0f,
	  -1.0f, -1.0f, 0.0f, 0.0f,
	  -1.0f + textureSideW, -1.0f, 0.0f, 1.0f
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(textureSide), textureSide, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLES, 0, 6);
      }

      // category selection
      {
	float categorySide[] = {
	  -1.0f + textureSideW, 1.0f, 1.0f, 0.0f,
	  -1.0f + textureSideW + categorySideW, 1.0f, 1.0f, 1.0f,
	  -1.0f + textureSideW, 1.0f - categorySideH, 0.0f, 0.0f,

	  -1.0f + textureSideW + categorySideW, 1.0f, 1.0f, 1.0f,
	  -1.0f + textureSideW, 1.0f - categorySideH, 0.0f, 0.0f,
	  -1.0f + textureSideW + categorySideW, 1.0f - categorySideH, 0.0f, 1.0f
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(categorySide), categorySide, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLES, 0, 6);
      }
    
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
      
      float mouseY = mouse.cursor.z + 0.06f;
      int selectedIndex = (1.0f-((mouseY +1.0f)/2.0f)) / (letterH/2);

      // 1 - textureSide
      // 2 - textureCategorySide
      int selectedPart = 0;

      float xStart = -1.0f;
      float xEnd = -1.0f + textureSideW;

      if(mouse.cursor.x <= -1.0f + textureSideW){
	selectedPart = 1;
      }else if(mouse.cursor.x > -1.0f + textureSideW && mouse.cursor.x <= -1.0f + textureSideW + categorySideW){
	xStart = -1.0f + textureSideW;
	xEnd = -1.0f + textureSideW + categorySideW;
      
	selectedPart = 2;
      }

      int curCategoryTexturesSize = loadedTexturesCategories[texturesMenuCurCategoryIndex].txWithThisCategorySize;

      if((selectedPart == 1 && selectedIndex < curCategoryTexturesSize) || (selectedPart == 2 && selectedIndex < loadedTexturesCategoryCounter)){

	// selection draw
	{	    
	  setSolidColorTx(redColor, 1.0f);
      
	  glBindVertexArray(selectionRectVAO);
	  glBindBuffer(GL_ARRAY_BUFFER, selectionRectVBO);
	
	  float cursorH = 0.06f;
	  float cursorW = 0.02f;
	  
	  float  selectionRect[] = {
	    xStart, 1.0f - selectedIndex * letterH, 1.0f, 0.0f,
	    xEnd, 1.0f - selectedIndex * letterH, 1.0f, 1.0f,
	    xStart, 1.0f - (selectedIndex + 1) * letterH, 0.0f, 0.0f,

	    xEnd, 1.0f - selectedIndex * letterH, 1.0f, 1.0f,
	    xStart,  1.0f - (selectedIndex + 1) * letterH, 0.0f, 0.0f,
	    xEnd,  1.0f - (selectedIndex + 1) * letterH, 0.0f, 1.0f };

	  glBufferData(GL_ARRAY_BUFFER, sizeof(selectionRect), selectionRect, GL_STATIC_DRAW);

	  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	  glEnableVertexAttribArray(0);

	  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
	  glEnableVertexAttribArray(1);

	  glDrawArrays(GL_TRIANGLES, 0, 6);
	}
     
	if(mouse.clickL){
	  if(selectedPart == 2){
	    texturesMenuCurCategoryIndex = selectedIndex;
	  }else if(selectedPart == 1){
	    
	    if(mouse.brushType == mouseBlockBrushT){
	      free(mouse.brushThing);
	    }
	    mouse.brushType = mouseTextureBrushT;
	    mouse.brushThing = &loadedTextures2D[texturesMenuCurCategoryIndex][selectedIndex];

	    curMenu->open = false;
	    curMenu = NULL;
	  }
	}      
      }

      for(int i=0;i< loadedTexturesCategories[texturesMenuCurCategoryIndex].txWithThisCategorySize;i++){
	int index = loadedTextures2D[texturesMenuCurCategoryIndex][i].index1D; 
	renderText(loadedTexturesNames[index],-1.0f, 1.0f - (i * letterH),1.0f);
      }

      for(int i=0;i<loadedTexturesCategoryCounter;i++){
	renderText(loadedTexturesCategories[i].name,-1.0f + textureSideW, 1.0f - (i * letterH),1.0f);
      }
      
    }else if(curMenu && curMenu->type == planeCreatorT){
      if (!planeOnCreation) {
	planeOnCreation = calloc(1, sizeof(Picture));

	Matrix out = IDENTITY_MATRIX;

	vec3 normFront = normalize3(cross3(curCamera->front, curCamera->up));

	float dist = 0.3f;
	
	out.m[12] = curCamera->pos.x + dist * mouse.rayDir.x;
	out.m[13] = curCamera->pos.y;
	out.m[14] = curCamera->pos.z + (dist/2) * mouse.rayDir.z;

	planeOnCreation->w = 0.1f;
	planeOnCreation->h = 0.1f;

	planeOnCreation->characterId = -1;
	
	planeOnCreation->mat = out; 
      }

      glActiveTexture(solidColorTx);
      glBindTexture(GL_TEXTURE_2D, solidColorTx);
      setSolidColorTx(blackColor, 1.0f);

      glBindVertexArray(hudRect.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, hudRect.VBO);

      char buf[100];
      sprintf(buf, "- + W: %f \n- + H: %f", planeOnCreation->w, planeOnCreation->h);

      float creatorW = (strlen(buf)/2.0f * letterW) + letterW;
      float creatorH = 0.17f + letterH;

      float creatorMenu[] = {
	-1.0f, 1.0f, 1.0f, 0.0f,
	-1.0f + creatorW, 1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f - creatorH, 0.0f, 0.0f,

	-1.0f + creatorW, 1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f - creatorH, 0.0f, 0.0f,
	-1.0f + creatorW, 1.0f - creatorH, 0.0f, 1.0f
      };

      glBufferData(GL_ARRAY_BUFFER, sizeof(creatorMenu), creatorMenu, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
      glEnableVertexAttribArray(1);

      glDrawArrays(GL_TRIANGLES, 0, 6);
    
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      float mouseY = mouse.cursor.z + 0.06f;
      
      int selectedIndex = (1.0f-((mouseY +1.0f)/2.0f)) / (letterH/2);
      int selectedSign = 0;

      float xLeftFrame = 0;
      float xRightFrame = 0;
      
      selectedIndex++;

      if(selectedIndex == 1 || selectedIndex == 2){
	if(mouse.cursor.x >= -1.0f && mouse.cursor.x <= -1.0f + letterW * 2){
	  xLeftFrame = -1.0f;
	  xRightFrame = -1.0f + letterW * 2;
	
	  selectedSign = -1;
	}else if(mouse.cursor.x > -1.0f + letterW * 2 && mouse.cursor.x <= -1.0f + letterW * 4){
	  xLeftFrame = -1.0f + letterW * 2;
	  xRightFrame = -1.0f + letterW * 4;
	
	  selectedSign = 1;
	}
      }else if(selectedIndex == 3){
	if(mouse.cursor.x >= -1.0f && mouse.cursor.x <= -1.0f + strlen("Create") * letterW + letterW){
	  xLeftFrame = -1.0f;
	  xRightFrame = -1.0f + strlen("Create") * letterW + letterW;

	  selectedSign = 2; // create button
	}
      }

      
      if(selectedSign != 0 && selectedIndex <= 3){
	if(mouse.clickL){
	  if(selectedIndex == 1){
	    planeOnCreation->w += 0.01f * selectedSign;
	  }else if(selectedIndex == 2){
	    planeOnCreation->h += 0.01f * selectedSign;
	  }else if(selectedIndex == 3 && selectedSign == 2){
	    if(!createdPlanes){
	      createdPlanes = malloc(sizeof(Picture));
	    }else{
	      createdPlanes = realloc(createdPlanes, (createdPlanesSize+1) * sizeof(Picture));
	    }

	    memcpy(&createdPlanes[createdPlanesSize],planeOnCreation ,sizeof(Picture));

	    createdPlanes[createdPlanesSize].id = createdPlanesSize;
	    createdPlanes[createdPlanesSize].characterId = -1;
	    
	    createdPlanesSize++;

	    free(planeOnCreation);
	    planeOnCreation = NULL;

	    curMenu->open = false;
	    curMenu = NULL;
	  }
	};

	setSolidColorTx(redColor, 1.0f);
      
	glBindVertexArray(selectionRectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, selectionRectVBO);

	if(selectedIndex == 3){
	  xLeftFrame = -1.0f;
	  xRightFrame = -1.0f + strlen("Create") * letterW + letterW;
	}

	float  selectionRect[] = {
	  xLeftFrame, 1.0f - (selectedIndex-1) * letterH, 1.0f, 0.0f,
	  xRightFrame, 1.0f - (selectedIndex-1) * letterH, 1.0f, 1.0f,
	  xLeftFrame, 1.0f - (selectedIndex) * letterH, 0.0f, 0.0f,

	  xRightFrame, 1.0f - (selectedIndex-1) * letterH, 1.0f, 1.0f,
	  xLeftFrame,  1.0f - selectedIndex * letterH, 0.0f, 0.0f,
	  xRightFrame,  1.0f - selectedIndex * letterH, 0.0f, 1.0f };

	glBufferData(GL_ARRAY_BUFFER, sizeof(selectionRect), selectionRect, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
      }

      renderText(buf, -1.0f, 1.0f, 1);
      renderText("Create", -1.0f, 1.0f - letterH * 2, 1);

    }else if(curMenu && curMenu->type == dialogViewerT){
      int characterId = -1;

      if (mouse.focusedType == mouseModelT) {
	Model* model = (Model*)mouse.focusedThing;
	characterId = model->characterId;

      }
      else if (mouse.focusedType == mousePlaneT) {
	Picture* plane = (Picture*)mouse.focusedThing;
	characterId = plane->characterId;
      }

      Character* editedCharacter = &characters[characterId];
      Dialog* curDialog = editedCharacter->curDialog;

      // dialog viewer background
      {
	setSolidColorTx(blackColor, 1.0f);
      
	glBindVertexArray(dialogViewer.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, dialogViewer.VBO);

	glDrawArrays(GL_TRIANGLES, 0, 6);
    
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
      }

      char str[dialogEditorAnswerInputLimit + 10];
      
      // name things
      {
	if(!editedCharacter->name){
	  strcpy(str, "[none]");
	}else{
	  strcpy(str, editedCharacter->name);
	}
	
	strcat(str, ":");
	
	float baseY = dialogViewer.rect.y + letterH + 0.02f;
	
	glBindVertexArray(hudRect.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, hudRect.VBO);

	float nameW = (strlen(str) + 1) * letterW;
	    
	float answerSelection[] = {
	  dialogViewer.rect.x, baseY, 1.0f, 0.0f,
	  dialogViewer.rect.x + nameW, baseY, 1.0f, 1.0f,
	  dialogViewer.rect.x, baseY - letterH, 0.0f, 0.0f,

	  dialogViewer.rect.x + nameW, baseY, 1.0f, 1.0f,
	  dialogViewer.rect.x, baseY - letterH, 0.0f, 0.0f,
	  dialogViewer.rect.x + nameW, baseY - letterH, 0.0f, 1.0f };

	glBufferData(GL_ARRAY_BUFFER, sizeof(answerSelection), answerSelection, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLES, 0, 6);
    
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	renderText(str, dialogViewer.rect.x, dialogViewer.rect.y + letterH + 0.02f, 1.0f);
      }
      
      renderText(curDialog->replicaText, dialogViewer.rect.x + 0.02f, dialogViewer.rect.y - 0.02f, 1.0f);

      float mouseY = mouse.cursor.z + 0.06f;
      bool selectedNewAnswer = false;
      
      // dialog answers buttons
      for(int i=0;i< curDialog->answersSize;i++){
	bool isEnd = false;
	
	if (curDialog->answers[i].answersSize == 0){
	  isEnd = true;
	  strcpy(str, "[end]");

	  if(curDialog->answers[i].text){
	    strcat(str, curDialog->answers[i].text);
	  }
	}else{
	  if(curDialog->answers[i].text)
	    strcpy(str, curDialog->answers[i].text);
	  else{
	    strcpy(str, "[empty]");
	  }
	}
	
	float answerLen = strlen(str) +1;
	float selectionW = (answerLen * letterW) + letterCellW;

	if(dialogViewer.buttons[i].x <= mouse.cursor.x && dialogViewer.buttons[i].x + selectionW >= mouse.cursor.x && dialogViewer.buttons[i].y >= mouseY && dialogViewer.buttons[i].y - dialogViewer.buttons[i].h <= mouseY){

	  if(mouse.clickL){
	    selectedNewAnswer = true;
	  }
	  
	  setSolidColorTx(redColor, 1.0f);
	    
	  glBindVertexArray(hudRect.VAO);
	  glBindBuffer(GL_ARRAY_BUFFER, hudRect.VBO);
	    
	  float answerSelection[] = {
	    dialogViewer.buttons[i].x, dialogViewer.buttons[i].y, 1.0f, 0.0f,
	    dialogViewer.buttons[i].x + selectionW, dialogViewer.buttons[i].y, 1.0f, 1.0f,
	    dialogViewer.buttons[i].x, dialogViewer.buttons[i].y - dialogViewer.buttons[i].h, 0.0f, 0.0f,

	    dialogViewer.buttons[i].x + selectionW, dialogViewer.buttons[i].y, 1.0f, 1.0f,
	    dialogViewer.buttons[i].x, dialogViewer.buttons[i].y - dialogViewer.buttons[i].h, 0.0f, 0.0f,
	    dialogViewer.buttons[i].x + selectionW, dialogViewer.buttons[i].y - dialogViewer.buttons[i].h, 0.0f, 1.0f };

	  glBufferData(GL_ARRAY_BUFFER, sizeof(answerSelection), answerSelection, GL_STATIC_DRAW);

	  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	  glEnableVertexAttribArray(0);

	  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	  glEnableVertexAttribArray(1);

	  glDrawArrays(GL_TRIANGLES, 0, 6);
    
	  glBindBuffer(GL_ARRAY_BUFFER, 0);
	  glBindVertexArray(0);  
	}

	renderText("-", dialogViewer.buttons[i].x, dialogViewer.buttons[i].y, 1.0f);
	renderText(str, dialogViewer.buttons[i].x + letterCellW, dialogViewer.buttons[i].y, 1.0f);

	if(selectedNewAnswer){
	  if(isEnd){
	    editedCharacter->curDialogIndex = 0;
	    editedCharacter->curDialog = &editedCharacter->dialogs;

	    dialogViewer.open = false;
	    dialogEditor.open = true;
	    curMenu = &dialogEditor;
	    
	    dialogEditor.textInputs[replicaInput].buf = &editedCharacter->curDialog->replicaText;
	    dialogEditor.textInputs[charNameInput].buf = &editedCharacter->name;

	    for(int i=0;i<editedCharacter->dialogs.answersSize;i++){
	      dialogEditor.textInputs[i+answerInput1].buf = &editedCharacter->curDialog->answers[i].text; 
	    }

	    
	    break;
	  }
	  
	  editedCharacter->curDialog = &curDialog->answers[i];
	  editedCharacter->curDialogIndex++;
	  curDialog = editedCharacter->curDialog;
	  break;
	}
      }
    }else if(curMenu && curMenu->type == dialogEditorT){
      int characterId = -1;

      if (mouse.focusedType == mouseModelT) {
	Model* model = (Model*)mouse.focusedThing;
	characterId = model->characterId;

      }
      else if (mouse.focusedType == mousePlaneT) {
	Picture* plane = (Picture*)mouse.focusedThing;
	characterId = plane->characterId;
      }

      Character* editedCharacter = &characters[characterId];
      Dialog* curDialog = editedCharacter->curDialog;
      
      glActiveTexture(solidColorTx);
      glBindTexture(GL_TEXTURE_2D, solidColorTx);

      // dialog editor background
      {
	setSolidColorTx(blackColor, 1.0f);
      
	glBindVertexArray(dialogEditor.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, dialogEditor.VBO);

	glDrawArrays(GL_TRIANGLES, 0, 6);
    
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
      }

      // activation handle
      {
	if(mouse.clickL){
	  bool found = false;
	  float mouseY = mouse.cursor.z + 0.06f;

	  
	  if (selectedTextInput) {
	    selectedTextInput->active = false;

	    // save text in prev text input
	    if(tempTextInputStorageCursor != 0){
	      *selectedTextInput->buf = malloc(sizeof(char) * (strlen(tempTextInputStorage)+1));
	      strcpy(*selectedTextInput->buf, tempTextInputStorage); 

	      tempTextInputStorageCursor = 0;
	      memset(tempTextInputStorage, 0, 512 * sizeof(char)); 
	    }
	  }

	  selectedTextInput = NULL;	
	  
	  for(int i=0;i<answerInput1 + curDialog->answersSize;i++){
	    if(dialogEditor.textInputs[i].rect.x <= mouse.cursor.x && dialogEditor.textInputs[i].rect.x + dialogEditor.textInputs[i].rect.w >= mouse.cursor.x && dialogEditor.textInputs[i].rect.y >= mouseY && dialogEditor.textInputs[i].rect.y - dialogEditor.textInputs[i].rect.h <= mouseY){
	      selectedTextInput = &dialogEditor.textInputs[i];
	      dialogEditor.textInputs[i].active = true;

	      // new selectedInput had text use it for tempTextStorage
	      if(*selectedTextInput->buf){
		tempTextInputStorageCursor = strlen(*selectedTextInput->buf); 
		strcpy(tempTextInputStorage,*selectedTextInput->buf);  

		free(*selectedTextInput->buf); 
		*selectedTextInput->buf = NULL;
	      }

	      found = true;
	      break;
	    }
	  }
	  if(found == false){
	    // mouse vs buttons
	    for(int i=0; i<dialogEditorButtonsCounter; i++){
	      if(dialogEditor.buttons[i].x <= mouse.cursor.x && dialogEditor.buttons[i].x + dialogEditor.buttons[i].w >= mouse.cursor.x && dialogEditor.buttons[i].y >= mouseY && dialogEditor.buttons[i].y - dialogEditor.buttons[i].h <= mouseY){
		
		// is add button
		if(i >= addButton1 && i<= addButton5){
		  if(i - addButton1 == curDialog->answersSize -1){
		    curDialog->answersSize++;
		    curDialog->answers = realloc(curDialog->answers, curDialog->answersSize * sizeof(Dialog));
		    memset(&curDialog->answers[curDialog->answersSize-1], 0 , sizeof(Dialog));
		    
		    // TODO: Get rid of this loop and use code below instead
		    for(int i=0;i<curDialog->answersSize;i++){
		      dialogEditor.textInputs[i+answerInput1].buf = &curDialog->answers[i].text; 
		    }
		  }
		}else if(i >= minusButton1 && i<= minusButton6){
		  if(i - minusButton1 < curDialog->answersSize){
		    int answerIndex = i - minusButton1;

		    destroyDialogsFrom(&curDialog->answers[answerIndex]);
		    
		    int index = 0; 
		    for(int i=0;i<curDialog->answersSize;i++){
		      if(i == answerIndex){
			continue;
		      }
			
		      curDialog->answers[index] = curDialog->answers[i]; 
		      index++;
		    }
		    
		    curDialog->answersSize--;
		    curDialog->answers = realloc(curDialog->answers, curDialog->answersSize * sizeof(Dialog));
			 
		    if(editedCharacter->historySize == 0 && curDialog->answersSize == 0){    
		      destroyCharacter(editedCharacter->id); 
				
		      int* characterId = -1;

		      if (mouse.focusedType == mouseModelT) {
			Model* model = (Model*)mouse.focusedThing;
			characterId = &model->characterId;

		      }
		      else if (mouse.focusedType == mousePlaneT) {
			Picture* plane = (Picture*)mouse.focusedThing;
			characterId = &plane->characterId;
		      }

		      *characterId = -1;
		      
		      dialogEditor.open = false;
		      curMenu = NULL;
		      break; 
		    }

		    if(curDialog->answersSize == 0 && editedCharacter->historySize != 0){ 
		      editedCharacter->curDialogIndex--;

		      // keep history
		      {
			editedCharacter->historySize--;
			editedCharacter->dialogHistory = realloc(editedCharacter->dialogHistory, sizeof(int) * editedCharacter->historySize);
		      }

		      Dialog* prevDialog = NULL;

		      prevDialog = &editedCharacter->dialogs;

		      for(int i=0;i<editedCharacter->historySize;i++){
			prevDialog = &prevDialog->answers[editedCharacter->dialogHistory[i]];
		      }
		  

		      editedCharacter->curDialog = prevDialog;
		      curDialog = editedCharacter->curDialog;

		      if (editedCharacter->curDialogIndex == 0) {
			dialogEditor.textInputs[charNameInput].buf = &editedCharacter->name;
		      }

		      dialogEditor.textInputs[replicaInput].buf = &editedCharacter->curDialog->replicaText;

		      for (int i = 0; i < curDialog->answersSize; i++) {
			dialogEditor.textInputs[i + answerInput1].buf = &curDialog->answers[i].text;
		      }
		      break;

		    }
		    
		    // TODO: Get rid of this loop and use code below instead
		    for(int i=0;i<curDialog->answersSize;i++){
		      dialogEditor.textInputs[i+answerInput1].buf = &curDialog->answers[i].text; 
		    }
		  }
		}else if(i >= nextButton1 && i<= nextButton6){
		  if(i - nextButton1 < curDialog->answersSize){
		    int answerIndex = i - nextButton1;
		     
		    editedCharacter->curDialogIndex++;
		    editedCharacter->curDialog = &curDialog->answers[answerIndex];
		    curDialog = editedCharacter->curDialog;


		    // keep history
		    {
		      editedCharacter->historySize++; 

		      if(!editedCharacter->dialogHistory){
			editedCharacter->dialogHistory = malloc(sizeof(int));
		      }else{
			editedCharacter->dialogHistory = realloc(editedCharacter->dialogHistory, sizeof(int) * editedCharacter->historySize);
		      }

		      editedCharacter->dialogHistory[editedCharacter->historySize-1] = answerIndex; 
		    }

		    if(!curDialog->answers){ 
		      curDialog->answersSize = 1;
		      curDialog->answers = calloc(1, sizeof(Dialog));
		    }

		    dialogEditor.textInputs[replicaInput].buf = &curDialog->replicaText;

		    for(int i=0;i<curDialog->answersSize;i++){ 
		      dialogEditor.textInputs[i+answerInput1].buf = &curDialog->answers[i].text;
		    }
		  
		  }
		}else if(i == prevDialogButton && editedCharacter->curDialogIndex > 0){
		  editedCharacter->curDialogIndex--;

		  // keep history
		  {
		    editedCharacter->historySize--;
		    editedCharacter->dialogHistory = realloc(editedCharacter->dialogHistory, sizeof(int) * editedCharacter->historySize);
		  }

		  Dialog* prevDialog = NULL;

		  prevDialog = &editedCharacter->dialogs;

		  for(int i=0;i<editedCharacter->historySize;i++){
		    prevDialog = &prevDialog->answers[editedCharacter->dialogHistory[i]];
		  }
		  

		  editedCharacter->curDialog = prevDialog;
		  curDialog = editedCharacter->curDialog;

		  if (editedCharacter->curDialogIndex == 0) {
		    dialogEditor.textInputs[charNameInput].buf = &editedCharacter->name;
		  }

		  dialogEditor.textInputs[replicaInput].buf = &editedCharacter->curDialog->replicaText;

		  for(int i=0;i<curDialog->answersSize;i++){
		    dialogEditor.textInputs[i+answerInput1].buf = &curDialog->answers[i].text;
		  }

		  break;
		}
		
	      }
	    }
	  }
	}
      }


      // dialog char input
      if(editedCharacter->curDialogIndex == 0)
	{	
	  if(dialogEditor.textInputs[charNameInput].active){
	    setSolidColorTx(redColor, 1.0f);
	  }else{
	    setSolidColorTx(greenColor, 1.0f);
	  }
      
	  glBindVertexArray(dialogEditor.vpairs[charNameInput].VAO);
	  glBindBuffer(GL_ARRAY_BUFFER, dialogEditor.vpairs[charNameInput].VBO);

	  glDrawArrays(GL_TRIANGLES, 0, 6);
    
	  glBindBuffer(GL_ARRAY_BUFFER, 0);
	  glBindVertexArray(0);
	
	  renderText(editedCharacter->name,dialogEditor.textInputs[charNameInput].rect.x, dialogEditor.textInputs[charNameInput].rect.y ,1.0f);
	  renderText("Entity name:",dialogEditor.rect.x, dialogEditor.textInputs[charNameInput].rect.y ,1.0f);
	}

      // prev player answer
      if(editedCharacter->curDialogIndex != 0)
	{
	  renderText("Player said:",dialogEditor.rect.x + letterCellW * 2, dialogEditor.rect.y, 1.0f);
	  renderText(curDialog->text,dialogEditor.rect.x + letterCellW * 2, dialogEditor.rect.y - letterH, 1.0f);
	}

      // prev button
      if(editedCharacter->curDialogIndex != 0)
	{	  
	  glBindVertexArray(dialogEditor.buttonsPairs[prevDialogButton].VAO);
	  glBindBuffer(GL_ARRAY_BUFFER, dialogEditor.buttonsPairs[prevDialogButton].VBO);

	  glDrawArrays(GL_TRIANGLES, 0, 6);
    
	  glBindBuffer(GL_ARRAY_BUFFER, 0);
	  glBindVertexArray(0);

	  renderText("<",dialogEditor.buttons[prevDialogButton].x, dialogEditor.buttons[prevDialogButton].y ,1.0f);
	  glBindTexture(GL_TEXTURE_2D, solidColorTx);
	}

      // dialog replica input
      {	
	if(dialogEditor.textInputs[replicaInput].active){
	  setSolidColorTx(redColor, 1.0f);
	}else{
	  setSolidColorTx(greenColor, 1.0f);
	}
      
	glBindVertexArray(dialogEditor.vpairs[replicaInput].VAO);
	glBindBuffer(GL_ARRAY_BUFFER, dialogEditor.vpairs[replicaInput].VBO);

	glDrawArrays(GL_TRIANGLES, 0, 6);
    
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	renderText(curDialog->replicaText,dialogEditor.textInputs[replicaInput].rect.x, dialogEditor.textInputs[replicaInput].rect.y ,1.0f);

	if(editedCharacter->curDialogIndex == 0){ 
	  renderText("Initial entity phrase:",dialogEditor.rect.x, dialogEditor.textInputs[replicaInput].rect.y + letterCellH ,1.0f);
	}else{
	  renderText("Entity answer:",dialogEditor.rect.x, dialogEditor.textInputs[replicaInput].rect.y + letterCellH ,1.0f);
	}
	
      }

      // new answer input
      {
	renderText("Player answers:",dialogEditor.rect.x, dialogEditor.textInputs[answerInput1].rect.y + letterCellH ,1.0f);
	
	// it will known from Dialog struct amount of answers for this replic
	for(int i=answerInput1;i<curDialog->answersSize + answerInput1;i++){	  
	  if(dialogEditor.textInputs[i].active){
	    setSolidColorTx(redColor, 1.0f);
	  }else{
	    setSolidColorTx(greenColor, 1.0f);
	  }

	  int answersIndex = i-2;

	  // next button
	  {
	    int nextButIndex = (i-2) + 7;
	  
	    glBindVertexArray(dialogEditor.buttonsPairs[nextButIndex].VAO);
	    glBindBuffer(GL_ARRAY_BUFFER, dialogEditor.buttonsPairs[nextButIndex].VBO);

	    glDrawArrays(GL_TRIANGLES, 0, 6);
    
	    glBindBuffer(GL_ARRAY_BUFFER, 0);
	    glBindVertexArray(0);

	    renderText(">",dialogEditor.buttons[nextButIndex].x, dialogEditor.buttons[nextButIndex].y ,1.0f);
	    glBindTexture(GL_TEXTURE_2D, solidColorTx);
	  }

	  // minus button
	  {
	    int minusButIndex = (i-2) + minusButton1;
	    
	    glBindVertexArray(dialogEditor.buttonsPairs[minusButIndex].VAO);
	    glBindBuffer(GL_ARRAY_BUFFER, dialogEditor.buttonsPairs[minusButIndex].VBO);

	    glDrawArrays(GL_TRIANGLES, 0, 6);
    
	    glBindBuffer(GL_ARRAY_BUFFER, 0);
	    glBindVertexArray(0);
	    
	    renderText("-",dialogEditor.buttons[minusButIndex].x, dialogEditor.buttons[minusButIndex].y ,1.0f);
	    glBindTexture(GL_TEXTURE_2D, solidColorTx);
	  }

	  // add buttons
	  int addButIndex = (i - 2) + addButton1;
	  
	  if(addButIndex != addButton5+1){
	    if(addButIndex - addButton1 == curDialog->answersSize -1){
	      glBindVertexArray(dialogEditor.buttonsPairs[addButIndex].VAO);
	      glBindBuffer(GL_ARRAY_BUFFER, dialogEditor.buttonsPairs[addButIndex].VBO);

	      glDrawArrays(GL_TRIANGLES, 0, 6);
    
	      glBindBuffer(GL_ARRAY_BUFFER, 0);
	      glBindVertexArray(0);

	      renderText("+",dialogEditor.buttons[addButIndex].x, dialogEditor.buttons[addButIndex].y ,1.0f);
	      glBindTexture(GL_TEXTURE_2D, solidColorTx);
	    }
	  }

	  // answer input
	  {
	    glBindVertexArray(dialogEditor.vpairs[i].VAO);
	    glBindBuffer(GL_ARRAY_BUFFER, dialogEditor.vpairs[i].VBO);

	    glDrawArrays(GL_TRIANGLES, 0, 6);
    
	    glBindBuffer(GL_ARRAY_BUFFER, 0);
	    glBindVertexArray(0); 

	    renderText(curDialog->answers[answersIndex].text,dialogEditor.textInputs[i].rect.x, dialogEditor.textInputs[i].rect.y ,1.0f);

	  }
	}

	// selected text input text render
	if(selectedTextInput){
	  renderText(tempTextInputStorage,selectedTextInput->rect.x, selectedTextInput->rect.y ,1.0f);
	}
      }
    }

    // render context text
    if(!curMenu && hints){
      // black backGround drawins
      {
	glActiveTexture(solidColorTx);
	glBindTexture(GL_TEXTURE_2D, solidColorTx);
	setSolidColorTx(blackColor, 1.0f);
	
	glBindVertexArray(hudRect.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, hudRect.VBO);
	    
	float answerSelection[] = {
	  -1.0f, -1.0f + (letterH * contextBelowTextH), 0.0f, 1.0f,
	  1.0f, -1.0f + (letterH * contextBelowTextH), 1.0f, 1.0f,
	  -1.0f, -1.0f, 0.0f, 0.0f,

	  1.0f, -1.0f + (letterH * contextBelowTextH), 1.0f, 1.0f,
	  -1.0f, -1.0f, 1.0f, 0.0f,
	  1.0f, -1.0f, 0.0f, 0.0f, 
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(answerSelection), answerSelection, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLES, 0, 6);
    
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
      }

      renderText(contextBelowText, -1.0f, -1.0f + letterH * contextBelowTextH, 1.0f); 
    }

    // render cursor
    if(curMenu)
      {
	glActiveTexture(solidColorTx);
	glBindTexture(GL_TEXTURE_2D, solidColorTx);
	setSolidColorTx(whiteColor, 1.0f);
      
	glBindVertexArray(cursorVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cursorVBO);
	
	float cursorH = 0.06f;
	float cursorW = 0.02f;
	
	float cursorPoint[] = {
	  mouse.cursor.x + cursorW * 0.05f, mouse.cursor.z + cursorH, 0.0f, 0.0f,
	  mouse.cursor.x + cursorW, mouse.cursor.z + cursorH/2.0f, 0.0f, 0.0f,
	  mouse.cursor.x, mouse.cursor.z + cursorH / 4.0f, 0.0f, 0.0f, 

	  // TODO: If i try to render TRIANGLE with 3 in glDrawArrays instead of 6 + 0.0... here
	  // it causes artifacts
	  //	  0.0f,0.0f,0.0f,0.0f,
	  //  0.0f,0.0f,0.0f,0.0f,
	  // 0.0f,0.0f,0.0f,0.0f,
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(cursorPoint), cursorPoint, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
      }


    if(console.open){
      glActiveTexture(solidColorTx);
      glBindTexture(GL_TEXTURE_2D, solidColorTx);
      setSolidColorTx(blackColor, 1.0f);
      
      glBindVertexArray(console.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, console.VBO);

      glDrawArrays(GL_TRIANGLES, 0, 6);
    
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      // ">" symbol pinging animation
      {
#define cycleDuratation 40

	static int frameCounter = cycleDuratation;

	if(frameCounter >= cycleDuratation / 2.0f){
	  renderText("> ", -1.0f, 1.0f, 1.0f);
	  frameCounter--;
	}else{
	  if(frameCounter == 0){
	    frameCounter = cycleDuratation;
	  }

	  frameCounter--;
	}
      }

      renderText(consoleBuffer, -1.0f + letterCellW / 2.0f, 1.0f, 1.0f);

      if(consoleHasResponse){
	renderText(consoleResponse, -1.0f + letterCellW / 2.0f, 1.0f - consoleH, 1.0f);

      }
    }
	
    glUseProgram(mainShader);
    glEnable(GL_DEPTH_TEST);

    mouse.clickL = false;
    mouse.clickR = false;

    glFlush();
  
    SDL_GL_SwapWindow(window);

    uint32_t endtime = GetTickCount();
    uint32_t deltatime = endtime - starttime;

    if (!(deltatime > (1000 / FPS))) {
      Sleep((1000 / FPS) - deltatime);
    }
    
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

vec3* wallPosBySide(Side side, float wallH, float wallD, float tileD, float tileW){
  // should be free() after wall rendered 
  vec3* wallPos = malloc(sizeof(vec3) * 4);
  
  switch(side){
  case(top):{
    wallPos[0] =(vec3){0.0f, 0.0f + wallH, 0.0f + wallD};
    wallPos[1] =(vec3){0.0f + tileW,0.0f + wallH, 0.0f + wallD};
    wallPos[2] =(vec3){0.0f + tileW,0.0f, 0.0f + wallD};
    wallPos[3] =(vec3){0.0f, 0.0f, 0.0f + wallD};

    break;
  }
  case(bot):{
    wallPos[0] =(vec3){0.0f, 0.0f + wallH, 0.0f + tileD - wallD};
    wallPos[1] =(vec3){0.0f + tileW,0.0f + wallH, 0.0f + tileD - wallD};
    wallPos[2] =(vec3){0.0f + tileW,0.0f, 0.0f + tileD - wallD};
    wallPos[3] =(vec3){0.0f, 0.0f, 0.0f + tileD - wallD};
    
    break;
  }
  case(left):{
    wallPos[0] =(vec3){0.0f + wallD, 0.0f + wallH, 0.0f + tileD};
    wallPos[1] =(vec3){0.0f + wallD, 0.0f + wallH, 0.0f};
    wallPos[2] =(vec3){0.0f + wallD, 0.0f, 0.0f};
    wallPos[3] =(vec3){0.0f + wallD,0.0f, 0.0f + tileD};
  
    break;
  }
  case(right):{
    wallPos[0] =(vec3){0.0f + tileW - wallD,0.0f + wallH, 0.0f + tileD};
    wallPos[1] =(vec3){0.0f + tileW - wallD, 0.0f + wallH, 0.0f};
    wallPos[2] =(vec3){0.0f + tileW - wallD, 0.0f, 0.0f};
    wallPos[3] =(vec3){0.0f + tileW - wallD,0.0f, 0.0f + tileD};
  
    break;
  }
  default: break;
  }

  return wallPos;
}

void renderCube(float w, float h, float d){
  glActiveTexture(solidColorTx);
  glBindTexture(GL_TEXTURE_2D, solidColorTx);
  setSolidColorTx(1.0f,1.0f,1.0f, 1.0f);

  glBindBuffer(GL_ARRAY_BUFFER, cube.VBO);
  glBindVertexArray(cube.VAO);

  float verts[] = {
    // bot
    0.0f, 0.0f, 0.0f,
    w, 0.0f, 0.0f,
    w, 0.0f, d,

    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, d,
    w, 0.0f, d,

    // top
    0.0f, h, 0.0f,
    w, h, 0.0f,
    w, h, d,

    0.0f, h, 0.0f,
    0.0f, h, d,
    w, h, d,

    // left
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, d,
    0.0f, h, 0.0f,

    0.0f, h, 0.0f,
    0.0f, 0.0f, d,
    0.0f, h, d,

    // right
    w, 0.0f, 0.0f,
    w, 0.0f, d,
    w, h, 0.0f,

    w, h, 0.0f,
    w, 0.0f, d,
    w, h, d,

    // front
    0.0f, 0.0f, 0.0f,
    w, 0.0f, 0.0f,
    w, h, 0.0f,

    0.0f, 0.0f, 0.0f,
    w, h, 0.0f,
    0.0f, h, 0.0f,

    // back
    0.0f, 0.0f, d,
    w, 0.0f, d,
    w, h, d,

    0.0f, 0.0f, d,
    w, h, d,
    0.0f, h, d,
  };

  Matrix out2 = IDENTITY_MATRIX;
      
  out2.m[12] = gridX / 2;
  out2.m[13] = gridY / 2;
  out2.m[14] = gridZ / 2;

  //  glUniform3f(lightColor, (float)(rand() % 10) / 10.0f, (float)(rand() % 10) / 10.0f, (float)(rand() % 10) / 10.0f);
  
  glUniformMatrix4fv(lightModelLoc, 1, GL_FALSE, out2.m);

  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);

  glDrawArrays(GL_TRIANGLES, 0, sizeof(verts) / sizeof(float) / 3);

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
    fprintf(stderr, "Failed to compile shader: %s\n", log);
    free(log);
    glDeleteShader(shader);
    return 0;
  }

  return shader;
}

void addObjToStore(Object* obj){
  objsStoreSize++;
  
  if(objsStore == NULL){
    objsStore = malloc(sizeof(Object*));
  }else{
    objsStore = realloc(objsStore, objsStoreSize * sizeof(Object*));
  }

  // to O(1) lookuping
  obj->id = objsStoreSize-1;
  objsStore[objsStoreSize-1] = obj;
}

Object* doorConstructor(vec3 pos, bool opened){
  Object* newDoor = calloc(1,sizeof(Object));
  newDoor->pos = pos;
  newDoor->type = doorObj;
	    
  DoorInfo* doorInfo = malloc(sizeof(DoorInfo));
  doorInfo->opened = opened;
		  
  newDoor->objInfo = doorInfo;

  return newDoor;
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

  if(pcz > drawDistance || pcz < zNear){
    return false;
  }

  float pcy = dotf3(v, camera1.Y);
  float aux = pcz * tangFOV;
  
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

  //  vec3 lb = { 1000, 1000, 1000 };
  //  vec3 rt = { 0,0,0 };
  
  int counter = 0;
  for (int i = 0; i < mesh->position_count * 3;i+=3){
    if(i==0) continue;

    verts[counter] = (vec3){mesh->positions[i], mesh->positions[i+1], mesh->positions[i+2]};
    counter++;
  }
  

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


  free(modelVerts);

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
    glGenTextures(1, &loadedModel->tx);

    // -1 because of solidColorTx
    //    SDL_Surface* texture = IMG_Load(texturePath);
    SDL_Surface* texture = IMG_Load_And_Flip_Vertical(texturePath);
    
    if (!texture) {
      printf("Loading of texture \"%s\" failed", texturePath);
      exit(0);
    }

    glBindTexture(GL_TEXTURE_2D, loadedModel->tx);
      
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->w,
		 texture->h, 0, GL_RGBA,
		 GL_UNSIGNED_BYTE, texture->pixels);

    GLenum err = glGetError();
    
    if (err != GL_NO_ERROR) {
      printf("OpenGL error: %d\n", err); 
    }
  
    SDL_FreeSurface(texture);
      
    glBindTexture(GL_TEXTURE_2D, 0); 
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

void calculateAABB(Matrix mat, float* vertexes, int vertexesSize, vec3* lb, vec3* rt){
  *lb = (vec3){FLT_MAX,FLT_MAX,FLT_MAX};
  *rt = (vec3){-FLT_MAX,-FLT_MAX,-FLT_MAX};

  // assumes that first 3 it vec3, last 2 its UV
  for (int i = 0; i < vertexesSize * 5; i+=5) {
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

    fscanf(map, "%d %f %d ", &tile->ground, &tile->groundLift, &sidesCounter);

    tile->groundMat = IDENTITY_MATRIX;

    vec3 grid = xyz_indexesToCoords(x,y,z);
	
    tile->groundMat.m[12] = grid.x;
	tile->groundMat.m[13] = grid.y;
    tile->groundMat.m[14] = grid.z;

    GroundType type = valueIn(tile->ground, 0);

    GroundType tx2 = valueIn(tile->ground, 2);  
    
    if(y == 0 && (type == netTile || type == 0)){
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
	calculateAABB(tile->walls[side].mat, wallsVPairs[wType].pairs[i].vBuf, wallsVPairs[wType].pairs[i].vertexNum, &tile->walls[side].planes[i].lb, &tile->walls[side].planes[i].rt);
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
	  calculateAABB(tile->jointsMat[jointSide], wallsVPairs[wallJointT].pairs[i].vBuf, wallsVPairs[wallJointT].pairs[i].vertexNum, &tile->joint[jointSide][i].lb, &tile->joint[jointSide][i].rt);
	}
      }
    }

    int blockExists;
    fscanf(map, "%d ", &blockExists);

    if(blockExists){
      TileBlock* newBlock = malloc(sizeof(TileBlock));

      int vertexesSize; // must be div by 5
      int txIndex;
      int blockType;
      int rotateAngle;

      fscanf(map, "%d %d %d %d ",&blockType, &rotateAngle, &txIndex, &vertexesSize);

      newBlock->vertexes = malloc(sizeof(float) * vertexesSize);
      newBlock->vertexesSize = vertexesSize / 5;
      
      newBlock->txIndex = txIndex;
      newBlock->rotateAngle = rotateAngle;
      newBlock->type = blockType;
      newBlock->tile = tile;

      newBlock->vpair.VBO = tileBlocksTempl[newBlock->type].vpair.VBO;
      newBlock->vpair.VAO = tileBlocksTempl[newBlock->type].vpair.VAO;

      for(int i2=0;i2<vertexesSize;i2++){
	fscanf(map, "%f ", &newBlock->vertexes[i2]);
      }

      for(int mat=0;mat<16;mat++){
	fscanf(map, "%f ", &newBlock->mat.m[mat]);
      }

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

	bool acceptTile = (y == 0 && type == texturedTile &&  tx2 != textureOfGround) || tile->groundLift > 0 || (y != 0 && type == texturedTile);

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

    fprintf(map, "%d %d %d %d %f %d ",x,y,z, grid[y][z][x]->ground, grid[y][z][x]->groundLift, sidesAvaible);

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
      fprintf(map, "%d %d %d %d ",tile->block->type, tile->block->rotateAngle, tile->block->txIndex, tile->block->vertexesSize * 5);

      for(int i2=0;i2<tile->block->vertexesSize*5;i2++){
	fprintf(map, "%f ",tile->block->vertexes[i2]);
      }

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

  collectTilesMats();

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

    snowParticle = (Particle*)malloc(sizeof(Particle) * snowAmount);
    snowMeshVertixes = malloc(sizeof(float) * 2 * 3 * snowAmount);  

    for (int loop = 0; loop < snowAmount; loop++)
      {
	snowParticle[loop].active = true;

	snowParticle[loop].life = 1.0f;
	snowParticle[loop].fade = (float)(rand() % 100) / 1000.0f + 0.003f;

	snowParticle[loop].x = (float)(rand() % gridX / 10.0f) + (float)(rand() % 100 / 1000.0f);
	snowParticle[loop].y = (float)(rand() % (int)(gridY * floorH)) + (float)(rand() % 1000) / 1000.0f;
	snowParticle[loop].z = (float)(rand() % gridZ / 10.0f) + (float)(rand() % 100 / 1000.0f);
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
  memcpy(newBlock, &tileBlocksTempl[type], sizeof(TileBlock));

  newBlock->vertexes = malloc(newBlock->vertexesSize * sizeof(float) * 5);

  newBlock->mat = IDENTITY_MATRIX;

  if(mouse.selectedType == mouseTileT){
    TileMouseData* tileData = (TileMouseData*) mouse.selectedThing;

    vec3 tile = xyz_indexesToCoords(tileData->grid.x, curFloor, tileData->grid.z);
    
    newBlock->mat.m[12] = tile.x;
    newBlock->mat.m[13] = tile.y;
    newBlock->mat.m[14] = tile.z;
	 
    newBlock->tile = tileData->tile;
  }
  
  newBlock->rotateAngle = angle;
  newBlock->type = type;
  newBlock->txIndex = 0;
  
  memcpy(newBlock->vertexes, tileBlocksTempl[type].vertexes, newBlock->vertexesSize * sizeof(float) * 5);

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

  {
    wallsVPairs[wallJointT].pairs = malloc(sizeof(VPair) * jointPlaneCounter);
    wallsVPairs[wallJointT].planesNum = jointPlaneCounter;

    // top
    {
      glGenBuffers(1, &wallsVPairs[wallJointT].pairs[jointTopPlane].VBO);
      glGenVertexArrays(1, &wallsVPairs[wallJointT].pairs[jointTopPlane].VAO);

      glBindVertexArray(wallsVPairs[wallJointT].pairs[jointTopPlane].VAO);
      glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[wallJointT].pairs[jointTopPlane].VBO);

      wallsVPairs[wallJointT].pairs[jointTopPlane].vertexNum = 6;

      float topPlane[] = {
	w, h, -t,         0.0f, 0.0f,
	w+t, h, -t,    capRatio, 0.0f,
	w,h, 0.0f,          0.0f, capRatio,

	w+t, h, -t,    capRatio, 0.0f,
	w,h, 0.0f,           0.0f, capRatio,
	w+t,h,0.0f,         capRatio, capRatio,
      };

      wallsVPairs[wallJointT].pairs[jointTopPlane].vBuf = malloc(sizeof(topPlane));
      memcpy(wallsVPairs[wallJointT].pairs[jointTopPlane].vBuf, topPlane, sizeof(topPlane));

      glBufferData(GL_ARRAY_BUFFER, sizeof(topPlane), topPlane, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }

    // front
    {
      glGenBuffers(1, &wallsVPairs[wallJointT].pairs[jointFrontPlane].VBO);
      glGenVertexArrays(1, &wallsVPairs[wallJointT].pairs[jointFrontPlane].VAO);

      glBindVertexArray(wallsVPairs[wallJointT].pairs[jointFrontPlane].VAO);
      glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[wallJointT].pairs[jointFrontPlane].VBO);

      wallsVPairs[wallJointT].pairs[jointFrontPlane].vertexNum = 6;

      float backPlane[] = {
	w+t, h, -t,           capRatio, 1.0f,
	w, h, -t,             0.0f, 1.0f,
	w+t, 0.0f , -t,       capRatio, 0.0f,

	w, h, -t,             0.0f, 1.0f,
	w+t, 0.0f , -t,       capRatio, 0.0f,
	w, 0.0f , -t,         0.0f, 0.0f,
      };

      wallsVPairs[wallJointT].pairs[jointFrontPlane].vBuf = malloc(sizeof(backPlane));
      memcpy(wallsVPairs[wallJointT].pairs[jointFrontPlane].vBuf, backPlane, sizeof(backPlane));

      glBufferData(GL_ARRAY_BUFFER, sizeof(backPlane), backPlane, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }

    //left
    {
      glGenBuffers(1, &wallsVPairs[wallJointT].pairs[jointSidePlane].VBO);
      glGenVertexArrays(1, &wallsVPairs[wallJointT].pairs[jointSidePlane].VAO);

      glBindVertexArray(wallsVPairs[wallJointT].pairs[jointSidePlane].VAO);
      glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[wallJointT].pairs[jointSidePlane].VBO);

      wallsVPairs[wallJointT].pairs[jointSidePlane].vertexNum = 6;

      float e = w + t;
      
      float leftPlane[] = {
	e, 0.0f, 0.0f,  0.0f, 0.0f,
	e, h, 0.0f,     0.0f, 1.0f,
	e, h , -t,     t, 1.0f,

	e, h, -t,      t, 1.0f,
	e, 0.0f, 0.0f,  0.0f, 0.0f,
	e, 0.0f , -t,  t, 0.0f,
      };

      wallsVPairs[wallJointT].pairs[jointSidePlane].vBuf = malloc(sizeof(leftPlane));
      memcpy(wallsVPairs[wallJointT].pairs[jointSidePlane].vBuf, leftPlane, sizeof(leftPlane));

      glBufferData(GL_ARRAY_BUFFER, sizeof(leftPlane), leftPlane, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }
  }

}

void assembleWallBlockVBO() {  // wallBlock buf
  float w = 1;
  float h = 2;

  float capRatio = 0.12f;
  
  float t = (float)1 / 8;

  float d = t;   
   
  {
    wallsVPairs[wallT].pairs = malloc(sizeof(VPair) * wPlaneCounter);    
    wallsVPairs[wallT].planesNum = wPlaneCounter;

    // top
    {
      glGenBuffers(1, &wallsVPairs[wallT].pairs[wTopPlane].VBO);
      glGenVertexArrays(1, &wallsVPairs[wallT].pairs[wTopPlane].VAO);

      glBindVertexArray(wallsVPairs[wallT].pairs[wTopPlane].VAO);
      glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[wallT].pairs[wTopPlane].VBO);

      wallsVPairs[wallT].pairs[wTopPlane].vertexNum = 6;

      float topPlane[] = {
	0.0f, h, -t, 0.0f, 0.0f,
	w, h, -t,    1.0f, 0.0f,
	0.0f,h, d,   0.0f, capRatio,

	w, h, -t,    1.0f, 0.0f,
	0.0f,h,d,    0.0f, capRatio,
	w,h,d,       1.0f, capRatio,
      };

      wallsVPairs[wallT].pairs[wTopPlane].vBuf = malloc(sizeof(topPlane));
      memcpy(wallsVPairs[wallT].pairs[wTopPlane].vBuf, topPlane, sizeof(topPlane));

      glBufferData(GL_ARRAY_BUFFER, sizeof(topPlane), topPlane, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }

    // back
    {
      glGenBuffers(1, &wallsVPairs[wallT].pairs[wBackPlane].VBO);
      glGenVertexArrays(1, &wallsVPairs[wallT].pairs[wBackPlane].VAO);

      glBindVertexArray(wallsVPairs[wallT].pairs[wBackPlane].VAO);
      glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[wallT].pairs[wBackPlane].VBO);

      wallsVPairs[wallT].pairs[wBackPlane].vertexNum = 6;

      float backPlane[] = {
	0.0f, h, -t,      0.0f, 1.0f,
	w, h, -t,         1.0f, 1.0f,
	0.0f, 0.0f , -t,  0.0f, 0.0f,

	w, h, -t,         1.0f, 1.0f,
	0.0f, 0.0f , -t,  0.0f, 0.0f,
	w, 0.0f , -t,     1.0f, 0.0f,
      };

      wallsVPairs[wallT].pairs[wBackPlane].vBuf = malloc(sizeof(backPlane));
      memcpy(wallsVPairs[wallT].pairs[wBackPlane].vBuf, backPlane, sizeof(backPlane));

      glBufferData(GL_ARRAY_BUFFER, sizeof(backPlane), backPlane, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }
    /*
    //left
    {
      glGenBuffers(1, &wallsVPairs[wallT].pairs[wLeftPlane].VBO);
      glGenVertexArrays(1, &wallsVPairs[wallT].pairs[wLeftPlane].VAO);

      glBindVertexArray(wallsVPairs[wallT].pairs[wLeftPlane].VAO);
      glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[wallT].pairs[wLeftPlane].VBO);

      wallsVPairs[wallT].pairs[wLeftPlane].vertexNum = 6 * 2;

      float leftPlane[] = {
	0.0f, 0.0f, -t,  0.0f, 0.0f,
	0.0f, h, -t,     0.0f, 1.0f,
	0.0f, h , d,     t, 1.0f,

	0.0f, h, d,      t, 1.0f,
	0.0f, 0.0f, -t,  0.0f, 0.0f,
	0.0f, 0.0f , d,  t, 0.0f,
      };

      wallsVPairs[wallT].pairs[wLeftPlane].vBuf = malloc(sizeof(leftPlane));
      memcpy(wallsVPairs[wallT].pairs[wLeftPlane].vBuf, leftPlane, sizeof(leftPlane));

      glBufferData(GL_ARRAY_BUFFER, sizeof(leftPlane), leftPlane, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }

    // right
    {
      glGenBuffers(1, &wallsVPairs[wallT].pairs[wRightPlane].VBO);
      glGenVertexArrays(1, &wallsVPairs[wallT].pairs[wRightPlane].VAO);

      glBindVertexArray(wallsVPairs[wallT].pairs[wRightPlane].VAO);
      glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[wallT].pairs[wRightPlane].VBO);

      wallsVPairs[wallT].pairs[wRightPlane].vertexNum = 6 * 2;

      float rightPlane[] = {
	w, 0.0f, -t,     0.0f, 0.0f,
	w, h, -t,        0.0f, 1.0f,
	w, h , d,        t, 1.0f,

	w, h, d,         t, 1.0f,
	w, 0.0f, -t,     0.0f, 0.0f,
	w, 0.0f , d,     t, 0.0f,
      };

      wallsVPairs[wallT].pairs[wRightPlane].vBuf = malloc(sizeof(rightPlane));
      memcpy(wallsVPairs[wallT].pairs[wRightPlane].vBuf, rightPlane, sizeof(rightPlane));

      glBufferData(GL_ARRAY_BUFFER, sizeof(rightPlane), rightPlane, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }*/

    // front
    {
      glGenBuffers(1, &wallsVPairs[wallT].pairs[wFrontPlane].VBO);
      glGenVertexArrays(1, &wallsVPairs[wallT].pairs[wFrontPlane].VAO);

      glBindVertexArray(wallsVPairs[wallT].pairs[wFrontPlane].VAO);
      glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[wallT].pairs[wFrontPlane].VBO);

      wallsVPairs[wallT].pairs[wFrontPlane].vertexNum = 6;

      float frontPlane[] = {
	0.0f, h, d,      0.0f, 1.0f,
	w, h, d,         1.0f, 1.0f,
	0.0f, 0.0f , d,  0.0f, 0.0f,

	w, h, d,         1.0f, 1.0f,
	0.0f, 0.0f , d,  0.0f, 0.0f,
	w, 0.0f , d,     1.0f, 0.0f
      };

      wallsVPairs[wallT].pairs[wFrontPlane].vBuf = malloc(sizeof(frontPlane));
      memcpy(wallsVPairs[wallT].pairs[wFrontPlane].vBuf, frontPlane, sizeof(frontPlane));
	   
      glBufferData(GL_ARRAY_BUFFER, sizeof(frontPlane), frontPlane, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }
  }
}

void assembleDoorBlockVBO() {  // wallBlock buf
  float w = 1;
  float h = 2;
  
  float capRatio = 0.12f;

  float capH = h * capRatio;
  //  float botH = h * 0.4f;

  float t = (float)1 / 8;
    
  float d = t;

  {
    wallsVPairs[doorT].pairs = malloc(sizeof(VPair) * doorPlaneCounter);
    wallsVPairs[doorT].planesNum = doorPlaneCounter;

    // top
    {
      glGenBuffers(1, &wallsVPairs[doorT].pairs[doorTopPlane].VBO);
      glGenVertexArrays(1, &wallsVPairs[doorT].pairs[doorTopPlane].VAO);

      glBindVertexArray(wallsVPairs[doorT].pairs[doorTopPlane].VAO);
      glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[doorT].pairs[doorTopPlane].VBO);

      wallsVPairs[doorT].pairs[doorTopPlane].vertexNum = 6;

      float topPlane[] = {
	0.0f, h, -t, 0.0f, 0.0f,
	w, h, -t,    1.0f, 0.0f,
	0.0f,h, d,   0.0f, capRatio,

	w, h, -t,    1.0f, 0.0f,
	0.0f,h,d,    0.0f, capRatio,
	w,h,d,       1.0f, capRatio,
      };

      wallsVPairs[doorT].pairs[doorTopPlane].vBuf = malloc(sizeof(topPlane));
      memcpy(wallsVPairs[doorT].pairs[doorTopPlane].vBuf, topPlane, sizeof(topPlane));

      glBufferData(GL_ARRAY_BUFFER, sizeof(topPlane), topPlane, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }

    // back
    {
      glGenBuffers(1, &wallsVPairs[doorT].pairs[doorBackPlane].VBO);
      glGenVertexArrays(1, &wallsVPairs[doorT].pairs[doorBackPlane].VAO);

      glBindVertexArray(wallsVPairs[doorT].pairs[doorBackPlane].VAO);
      glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[doorT].pairs[doorBackPlane].VBO);

      wallsVPairs[doorT].pairs[doorBackPlane].vertexNum = 6;

      float backSide[] = {
	// cap back
	0.0f, h, -t,      0.0f, capRatio,
	w, h, -t,         1.0f, capRatio,
	0.0f, h -capH, -t,  0.0f, 0.0f,

	w, h, -t,         1.0f, capRatio,
	0.0f, h -capH, -t,  0.0f, 0.0f,
	w, h -capH, -t,     1.0f, 0.0f,
      };

      wallsVPairs[doorT].pairs[doorBackPlane].vBuf = malloc(sizeof(backSide));
      memcpy(wallsVPairs[doorT].pairs[doorBackPlane].vBuf, backSide, sizeof(backSide));

      glBufferData(GL_ARRAY_BUFFER, sizeof(backSide), backSide, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }

    // inner top
    {
      glGenBuffers(1, &wallsVPairs[doorT].pairs[doorInnerTopPlane].VBO);
      glGenVertexArrays(1, &wallsVPairs[doorT].pairs[doorInnerTopPlane].VAO);

      glBindVertexArray(wallsVPairs[doorT].pairs[doorInnerTopPlane].VAO);
      glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[doorT].pairs[doorInnerTopPlane].VBO);

      wallsVPairs[doorT].pairs[doorInnerTopPlane].vertexNum = 6;

      float innerSide[] = {
	// cap bot
	0.0f, h-capH, -t, 0.0f, 0.0f,
	w,  h-capH, -t,    1.0f, 0.0f,
	0.0f, h-capH, d,   0.0f, capRatio,

	w,  h-capH, -t,    1.0f, 0.0f,
	0.0f, h-capH,d,    0.0f, capRatio,
	w, h-capH,d,       1.0f, capRatio,
      };
      
      wallsVPairs[doorT].pairs[doorInnerTopPlane].vBuf = malloc(sizeof(innerSide));
      memcpy(wallsVPairs[doorT].pairs[doorInnerTopPlane].vBuf, innerSide, sizeof(innerSide));

      glBufferData(GL_ARRAY_BUFFER, sizeof(innerSide), innerSide, GL_STATIC_DRAW);
		
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL); 
      glEnableVertexAttribArray(0);
		
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }

    /*
    //left
    {
    glGenBuffers(1, &wallsVPairs[doorT].pairs[wLeftPlane].VBO);
    glGenVertexArrays(1, &wallsVPairs[doorT].pairs[wLeftPlane].VAO);

    glBindVertexArray(wallsVPairs[doorT].pairs[wLeftPlane].VAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[doorT].pairs[wLeftPlane].VBO);

    wallsVPairs[doorT].pairs[wLeftPlane].vertexNum = 6 * 2;

    float leftPlane[] = {
    0.0f, 0.0f, -t,  0.0f, 0.0f,
    0.0f, h, -t,     0.0f, 1.0f,
    0.0f, h , d,     t, 1.0f,

    0.0f, h, d,      t, 1.0f,
    0.0f, 0.0f, -t,  0.0f, 0.0f,
    0.0f, 0.0f , d,  t, 0.0f,
    };

    wallsVPairs[doorT].pairs[wLeftPlane].vBuf = malloc(sizeof(leftPlane));
    memcpy(wallsVPairs[doorT].pairs[wLeftPlane].vBuf, leftPlane, sizeof(leftPlane));

    glBufferData(GL_ARRAY_BUFFER, sizeof(leftPlane), leftPlane, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    }*/

    /*
    // right
    {
    glGenBuffers(1, &wallsVPairs[doorT].pairs[wRightPlane].VBO);
    glGenVertexArrays(1, &wallsVPairs[doorT].pairs[wRightPlane].VAO);

    glBindVertexArray(wallsVPairs[doorT].pairs[wRightPlane].VAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[doorT].pairs[wRightPlane].VBO);

    wallsVPairs[doorT].pairs[wRightPlane].vertexNum = 6 * 2;

    float rightPlane[] = {
    w, 0.0f, -t,     0.0f, 0.0f,
    w, h, -t,        0.0f, 1.0f,
    w, h , d,        t, 1.0f,

    w, h, d,         t, 1.0f,
    w, 0.0f, -t,     0.0f, 0.0f,
    w, 0.0f , d,     t, 0.0f,
    };

    wallsVPairs[doorT].pairs[wRightPlane].vBuf = malloc(sizeof(rightPlane));
    memcpy(wallsVPairs[doorT].pairs[wRightPlane].vBuf, rightPlane, sizeof(rightPlane));

    glBufferData(GL_ARRAY_BUFFER, sizeof(rightPlane), rightPlane, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    }*/

    // front
    {
      glGenBuffers(1, &wallsVPairs[doorT].pairs[doorFrontPlane].VBO);
      glGenVertexArrays(1, &wallsVPairs[doorT].pairs[doorFrontPlane].VAO);

      glBindVertexArray(wallsVPairs[doorT].pairs[doorFrontPlane].VAO);
      glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[doorT].pairs[doorFrontPlane].VBO);

      wallsVPairs[doorT].pairs[doorFrontPlane].vertexNum = 6;
      
      float frontPlane[] = {
	// cap front
	0.0f, h, d,      0.0f, capRatio,
	w, h, d,         1.0f, capRatio,
	0.0f, h -capH , d,  0.0f, 0.0f,

	w, h, d,         1.0f, capRatio,
	0.0f, h -capH , d,  0.0f, 0.0f,
	w, h -capH , d,     1.0f, 0.0f,
      };

      wallsVPairs[doorT].pairs[doorFrontPlane].vBuf = malloc(sizeof(frontPlane));
      memcpy(wallsVPairs[doorT].pairs[doorFrontPlane].vBuf, frontPlane, sizeof(frontPlane));
	   
      glBufferData(GL_ARRAY_BUFFER, sizeof(frontPlane), frontPlane, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }
  }

  // center
    {
      glGenBuffers(1, &wallsVPairs[doorT].pairs[doorCenterPlane].VBO);
      glGenVertexArrays(1, &wallsVPairs[doorT].pairs[doorCenterPlane].VAO);

      glBindVertexArray(wallsVPairs[doorT].pairs[doorCenterPlane].VAO);
      glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[doorT].pairs[doorCenterPlane].VBO);

      wallsVPairs[doorT].pairs[doorCenterPlane].vertexNum = 6;
      
      float centerPlane[] = {
	// cap front
	0.0f, h -capH, 0.0f,      0.0f, 1.0f,
	w, h -capH, 0.0f,         1.0f, 1.0f,
	0.0f, 0.0f , 0.0f,  0.0f, 0.0f,

	w, h -capH, 0.0f,         1.0f, 1.0f,
	0.0f, 0.0f , 0.0f,  0.0f, 0.0f,
	w, 0.0f , 0.0f,     1.0f, 0.0f,
      };

      wallsVPairs[doorT].pairs[doorCenterPlane].vBuf = malloc(sizeof(centerPlane));
      memcpy(wallsVPairs[doorT].pairs[doorCenterPlane].vBuf, centerPlane, sizeof(centerPlane));
	   
      glBufferData(GL_ARRAY_BUFFER, sizeof(centerPlane), centerPlane, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }
 // }
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

  wallsVPairs[windowT].pairs = malloc(sizeof(VPair) * winPlaneCounter);
  wallsVPairs[windowT].planesNum = winPlaneCounter;
  
  // front bot
  {
    glGenBuffers(1, &wallsVPairs[windowT].pairs[winFrontBotPlane].VBO);
    glGenVertexArrays(1, &wallsVPairs[windowT].pairs[winFrontBotPlane].VAO);
    
    glBindVertexArray(wallsVPairs[windowT].pairs[winFrontBotPlane].VAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[windowT].pairs[winFrontBotPlane].VBO);

    wallsVPairs[windowT].pairs[winFrontBotPlane].vertexNum = 6;

    float frontPlane[] = {
      // bot front
      0.0f, botH, d,      0.0f, botRatio,
      w, botH, d,         1.0f, botRatio,
      0.0f, 0.0f , d,  0.0f, 0.0f,

      w, botH, d,         1.0f, botRatio,
      0.0f, 0.0f , d,  0.0f, 0.0f,
      w, 0.0f , d,     1.0f, 0.0f
    };
      
    wallsVPairs[windowT].pairs[winFrontBotPlane].vBuf = malloc(sizeof(frontPlane));
    memcpy(wallsVPairs[windowT].pairs[winFrontBotPlane].vBuf, frontPlane, sizeof(frontPlane));
      
    glBufferData(GL_ARRAY_BUFFER, sizeof(frontPlane), frontPlane, GL_STATIC_DRAW);
		
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL); 
    glEnableVertexAttribArray(0);
		
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  // front cap
  {
    glGenBuffers(1, &wallsVPairs[windowT].pairs[winFrontCapPlane].VBO);
    glGenVertexArrays(1, &wallsVPairs[windowT].pairs[winFrontCapPlane].VAO);
    
    glBindVertexArray(wallsVPairs[windowT].pairs[winFrontCapPlane].VAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[windowT].pairs[winFrontCapPlane].VBO);

    wallsVPairs[windowT].pairs[winFrontCapPlane].vertexNum = 6;

    float frontPlane[] = {
      // cap front
      0.0f, h, d,      0.0f, capRatio,
      w, h, d,         1.0f, capRatio,
      0.0f, h -capH , d,  0.0f, 0.0f,

      w, h, d,         1.0f, capRatio,
      0.0f, h -capH , d,  0.0f, 0.0f,
      w, h -capH , d,     1.0f, 0.0f,
    };
      
    wallsVPairs[windowT].pairs[winFrontCapPlane].vBuf = malloc(sizeof(frontPlane));
    memcpy(wallsVPairs[windowT].pairs[winFrontCapPlane].vBuf, frontPlane, sizeof(frontPlane));
      
    glBufferData(GL_ARRAY_BUFFER, sizeof(frontPlane), frontPlane, GL_STATIC_DRAW);
		
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL); 
    glEnableVertexAttribArray(0);
		
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  
  // back
  {
    glGenBuffers(1, &wallsVPairs[windowT].pairs[winBackBotPlane].VBO);
    glGenVertexArrays(1, &wallsVPairs[windowT].pairs[winBackBotPlane].VAO);

    glBindVertexArray(wallsVPairs[windowT].pairs[winBackBotPlane].VAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[windowT].pairs[winBackBotPlane].VBO);

    wallsVPairs[windowT].pairs[winBackBotPlane].vertexNum = 6;
      
    float backSide[] = {
      // bot back
      0.0f, botH, -t,      0.0f, botRatio,
      w, botH, -t,         1.0f, botRatio,
      0.0f, 0.0f , -t,  0.0f, 0.0f,

      w, botH, -t,         1.0f, botRatio,
      0.0f, 0.0f , -t,  0.0f, 0.0f,
      w, 0.0f , -t,     1.0f, 0.0f,
    };

    wallsVPairs[windowT].pairs[winBackBotPlane].vBuf = malloc(sizeof(backSide));
    memcpy(wallsVPairs[windowT].pairs[winBackBotPlane].vBuf, backSide, sizeof(backSide));
      
    glBufferData(GL_ARRAY_BUFFER, sizeof(backSide), backSide, GL_STATIC_DRAW);
		
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL); 
    glEnableVertexAttribArray(0);
		
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  // back cap
  {
    glGenBuffers(1, &wallsVPairs[windowT].pairs[winBackCapPlane].VBO);
    glGenVertexArrays(1, &wallsVPairs[windowT].pairs[winBackCapPlane].VAO);

    glBindVertexArray(wallsVPairs[windowT].pairs[winBackCapPlane].VAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[windowT].pairs[winBackCapPlane].VBO);

    wallsVPairs[windowT].pairs[winBackCapPlane].vertexNum = 6;
      
    float backSide[] = {
      // cap back
      0.0f, h, -t,      0.0f, capRatio,
      w, h, -t,         1.0f, capRatio,
      0.0f, h -capH, -t,  0.0f, 0.0f,

      w, h, -t,         1.0f, capRatio,
      0.0f, h -capH, -t,  0.0f, 0.0f,
      w, h -capH, -t,     1.0f, 0.0f,
    };

    wallsVPairs[windowT].pairs[winBackCapPlane].vBuf = malloc(sizeof(backSide));
    memcpy(wallsVPairs[windowT].pairs[winBackCapPlane].vBuf, backSide, sizeof(backSide));
      
    glBufferData(GL_ARRAY_BUFFER, sizeof(backSide), backSide, GL_STATIC_DRAW);
		
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL); 
    glEnableVertexAttribArray(0);
		
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  /*
  // right
  {
    glGenBuffers(1, &wallsVPairs[windowT].pairs[winRightPlane].VBO);
    glGenVertexArrays(1, &wallsVPairs[windowT].pairs[winRightPlane].VAO);

    glBindVertexArray(wallsVPairs[windowT].pairs[winRightPlane].VAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[windowT].pairs[winRightPlane].VBO);

    wallsVPairs[windowT].pairs[winRightPlane].vertexNum = 6 * 2;
	    
    float rightSide[] = {
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

    wallsVPairs[windowT].pairs[winRightPlane].vBuf = malloc(sizeof(rightSide));
    memcpy(wallsVPairs[windowT].pairs[winRightPlane].vBuf, rightSide, sizeof(rightSide));
      
    glBufferData(GL_ARRAY_BUFFER, sizeof(rightSide), rightSide, GL_STATIC_DRAW);
		
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL); 
    glEnableVertexAttribArray(0);
		
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  // left
  {
    glGenBuffers(1, &wallsVPairs[windowT].pairs[winLeftPlane].VBO);
    glGenVertexArrays(1, &wallsVPairs[windowT].pairs[winLeftPlane].VAO);

    glBindVertexArray(wallsVPairs[windowT].pairs[winLeftPlane].VAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[windowT].pairs[winLeftPlane].VBO);

    wallsVPairs[windowT].pairs[winLeftPlane].vertexNum = 6 * 2;
      
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

    };
      
    wallsVPairs[windowT].pairs[winLeftPlane].vBuf = malloc(sizeof(leftSide));
    memcpy(wallsVPairs[windowT].pairs[winLeftPlane].vBuf, leftSide, sizeof(leftSide));
      
    glBufferData(GL_ARRAY_BUFFER, sizeof(leftSide), leftSide, GL_STATIC_DRAW);
		
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL); 
    glEnableVertexAttribArray(0);
		
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }*/

  // top
  {
    glGenBuffers(1, &wallsVPairs[windowT].pairs[winTopPlane].VBO);
    glGenVertexArrays(1, &wallsVPairs[windowT].pairs[winTopPlane].VAO);

    glBindVertexArray(wallsVPairs[windowT].pairs[winTopPlane].VAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[windowT].pairs[winTopPlane].VBO);
      
    wallsVPairs[windowT].pairs[winTopPlane].vertexNum = 6;
      
    float topSide[] = {
      // cap top
      0.0f, h, -t, 0.0f, 0.0f,
      w, h, -t,    1.0f, 0.0f,
      0.0f,h, d,   0.0f, capRatio,

      w, h, -t,    1.0f, 0.0f,
      0.0f,h,d,    0.0f, capRatio,
      w,h,d,       1.0f, capRatio,
    };
            
    wallsVPairs[windowT].pairs[winTopPlane].vBuf = malloc(sizeof(topSide));
    memcpy(wallsVPairs[windowT].pairs[winTopPlane].vBuf, topSide, sizeof(topSide));
      
    glBufferData(GL_ARRAY_BUFFER, sizeof(topSide), topSide, GL_STATIC_DRAW);
		
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL); 
    glEnableVertexAttribArray(0);
		
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  // inner top
  {
    glGenBuffers(1, &wallsVPairs[windowT].pairs[winInnerTopPlane].VBO);
    glGenVertexArrays(1, &wallsVPairs[windowT].pairs[winInnerTopPlane].VAO);

    glBindVertexArray(wallsVPairs[windowT].pairs[winInnerTopPlane].VAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[windowT].pairs[winInnerTopPlane].VBO);

    wallsVPairs[windowT].pairs[winInnerTopPlane].vertexNum = 6;

    float innerSide[] = {
      // cap bot
      0.0f, h-capH, -t,  0.0f, 0.0f,
      w,  h-capH, -t,    1.0f, 0.0f,
      0.0f, h-capH, d,   0.0f, capRatio,

      w,  h-capH, -t,    1.0f, 0.0f,
      0.0f, h-capH,d,    0.0f, capRatio,
      w, h-capH,d,       1.0f, capRatio,
      /*
      // bot top
      0.0f, botH, -t, 0.0f, 0.0f,
      w, botH, -t,    t, 0.0f,
      0.0f,botH, d,   0.0f, 1.0f,

      w, botH, -t,    t, 0.0f,
      0.0f,botH,d,    0.0f, 1.0f,
      w,botH,d,       t, 1.0f,*/

    };
      
    wallsVPairs[windowT].pairs[winInnerTopPlane].vBuf = malloc(sizeof(innerSide));
    memcpy(wallsVPairs[windowT].pairs[winInnerTopPlane].vBuf, innerSide, sizeof(innerSide));

    glBufferData(GL_ARRAY_BUFFER, sizeof(innerSide), innerSide, GL_STATIC_DRAW);
		
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL); 
    glEnableVertexAttribArray(0);
		
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }
  
  // inner bot
  {
    glGenBuffers(1, &wallsVPairs[windowT].pairs[winInnerBotPlane].VBO);
    glGenVertexArrays(1, &wallsVPairs[windowT].pairs[winInnerBotPlane].VAO);

    glBindVertexArray(wallsVPairs[windowT].pairs[winInnerBotPlane].VAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[windowT].pairs[winInnerBotPlane].VBO);

    wallsVPairs[windowT].pairs[winInnerBotPlane].vertexNum = 6;

    float innerSide[] = {
      // bot top
      0.0f, botH, -t, 0.0f, 0.0f,
      w, botH, -t,    1.0f, 0.0f,
      0.0f,botH, d,   0.0f, capRatio,
      // with kamen2 1.5f looks also good 

      w, botH, -t,    1.0f, 0.0f,
      0.0f,botH,d,    0.0f, capRatio,
      w,botH,d,       1.0f, capRatio,

    };
      
    wallsVPairs[windowT].pairs[winInnerBotPlane].vBuf = malloc(sizeof(innerSide));
    memcpy(wallsVPairs[windowT].pairs[winInnerBotPlane].vBuf, innerSide, sizeof(innerSide));

    glBufferData(GL_ARRAY_BUFFER, sizeof(innerSide), innerSide, GL_STATIC_DRAW);
		
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL); 
    glEnableVertexAttribArray(0);
		
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }


  // center(window)
  {
    glGenBuffers(1, &wallsVPairs[windowT].pairs[winCenterPlane].VBO);
    glGenVertexArrays(1, &wallsVPairs[windowT].pairs[winCenterPlane].VAO);

    glBindVertexArray(wallsVPairs[windowT].pairs[winCenterPlane].VAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[windowT].pairs[winCenterPlane].VBO);

    wallsVPairs[windowT].pairs[winCenterPlane].vertexNum = 6;

    float v = 1.0f - (capRatio + botRatio);
      
    float windowPlane[] = {
      // cap bot
      0.0f, h-capH, 0.0f, 0.0f, 0.0f,
      w,  h-capH, 0.0f,    1.0f, 0.0f,
      0.0f, botH, 0.0f,   0.0f, v,

      w,  h-capH, 0.0f,    1.0f, 0.0f,
      0.0f, botH, 0.0f,    0.0f, v,
      w, botH, 0.0f,       1.0f, v,
    };
      
    wallsVPairs[windowT].pairs[winCenterPlane].vBuf = malloc(sizeof(windowPlane));
    memcpy(wallsVPairs[windowT].pairs[winCenterPlane].vBuf, windowPlane, sizeof(windowPlane));

    glBufferData(GL_ARRAY_BUFFER, sizeof(windowPlane), windowPlane, GL_STATIC_DRAW);
		
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL); 
    glEnableVertexAttribArray(0);
		
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  float padokonikRatio = 0.08f;
  float padokonikD = bBlockH * padokonikRatio;

  float padokonikMainH = bBlockH * 0.03f;

  float padokonikDownThingRatio = 0.02f;
  float padokonikDownThingH = bBlockH * padokonikDownThingRatio;
      
  // back podokonik
  {
    glGenBuffers(1, &wallsVPairs[windowT].pairs[winBackPodokonik].VBO);
    glGenVertexArrays(1, &wallsVPairs[windowT].pairs[winBackPodokonik].VAO);

    glBindVertexArray(wallsVPairs[windowT].pairs[winBackPodokonik].VAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[windowT].pairs[winBackPodokonik].VBO);

    wallsVPairs[windowT].pairs[winBackPodokonik].vertexNum = 6 * 2;

    float v = 1.0f - (capRatio + botRatio);

    float windowPlane[] = {
      // main olane
      0.0f, botH, -t,    0.0f, 0.0f,
      w, botH, -t,       1.0f, 0.0f,
      0.0f, botH - padokonikMainH, -padokonikD -t, 0.0f, padokonikRatio,

      w, botH, -t,       1.0f, 0.0f,
      0.0f, botH - padokonikMainH, -padokonikD -t, 0.0f, padokonikRatio,
      w, botH - padokonikMainH, -padokonikD-t, 1.0f, padokonikRatio,

      // down thing
      0.0f, botH - padokonikMainH, -padokonikD -t,                       0.0f, padokonikDownThingRatio,
      w, botH - padokonikMainH, -padokonikD -t,                          1.0f, padokonikDownThingRatio,
      0.0f, botH - padokonikMainH - padokonikDownThingH, -padokonikD -t, 0.0f, 0.0f,

      w, botH - padokonikMainH, -padokonikD -t,                          1.0f, padokonikDownThingRatio,
      0.0f, botH - padokonikMainH - padokonikDownThingH, -padokonikD -t, 0.0f, 0.0f,
      w, botH - padokonikMainH - padokonikDownThingH, -padokonikD -t,    1.0f, 0.0f,
    };
      
    wallsVPairs[windowT].pairs[winBackPodokonik].vBuf = malloc(sizeof(windowPlane));
    memcpy(wallsVPairs[windowT].pairs[winBackPodokonik].vBuf, windowPlane, sizeof(windowPlane));

    glBufferData(GL_ARRAY_BUFFER, sizeof(windowPlane), windowPlane, GL_STATIC_DRAW);
		
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL); 
    glEnableVertexAttribArray(0);
		
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  // front podokonik
  {
    glGenBuffers(1, &wallsVPairs[windowT].pairs[winFrontPodokonik].VBO);
    glGenVertexArrays(1, &wallsVPairs[windowT].pairs[winFrontPodokonik].VAO);

    glBindVertexArray(wallsVPairs[windowT].pairs[winFrontPodokonik].VAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallsVPairs[windowT].pairs[winFrontPodokonik].VBO);

    wallsVPairs[windowT].pairs[winFrontPodokonik].vertexNum = 6 * 2;

    float v = 1.0f - (capRatio + botRatio);
      
    float windowPlane[] = {
      // main olane
      0.0f, botH, t,    0.0f, 0.0f,
      w, botH, t,       1.0f, 0.0f,
      0.0f, botH - padokonikMainH, padokonikD+t, 0.0f, padokonikRatio,

      w, botH, t,       1.0f, 0.0f,
      0.0f, botH - padokonikMainH, padokonikD+t, 0.0f, padokonikRatio,
      w, botH - padokonikMainH, padokonikD+t, 1.0f, padokonikRatio,

      // down thing
      0.0f, botH - padokonikMainH, padokonikD+t,                       0.0f, padokonikDownThingRatio,
      w, botH - padokonikMainH, padokonikD+t,                          1.0f, padokonikDownThingRatio,
      0.0f, botH - padokonikMainH - padokonikDownThingH, padokonikD+t, 0.0f, 0.0f,

      w, botH - padokonikMainH, padokonikD+t,                          1.0f, padokonikDownThingRatio,
      0.0f, botH - padokonikMainH - padokonikDownThingH, padokonikD+t, 0.0f, 0.0f,
      w, botH - padokonikMainH - padokonikDownThingH, padokonikD+t,    1.0f, 0.0f,
    };
      
    wallsVPairs[windowT].pairs[winFrontPodokonik].vBuf = malloc(sizeof(windowPlane));
    memcpy(wallsVPairs[windowT].pairs[winFrontPodokonik].vBuf, windowPlane, sizeof(windowPlane));

    glBufferData(GL_ARRAY_BUFFER, sizeof(windowPlane), windowPlane, GL_STATIC_DRAW);
		
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL); 
    glEnableVertexAttribArray(0);
		
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
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
    calculateAABB(grid[curFloor][vec.z][vec.x]->jointsMat[side], wallsVPairs[wallJointT].pairs[i].vBuf, wallsVPairs[wallJointT].pairs[i].vertexNum, &grid[curFloor][vec.z][vec.x]->joint[side][i].lb, &grid[curFloor][vec.z][vec.x]->joint[side][i].rt); 
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
	  
	  grid[y][z][x]->groundMat = IDENTITY_MATRIX;

	  vec3 tile = xyz_indexesToCoords(x,y,z);
	
	  grid[y][z][x]->groundMat.m[12] = tile.x;
	  grid[y][z][x]->groundMat.m[13] = tile.y;
	  grid[y][z][x]->groundMat.m[14] = tile.z;
	}
      }
    }
  }
}

void collectTilesMats(){
  float texturedTileVerts[] = {
    bBlockW, 0.0f, bBlockD , 0.0f, 1.0f,
    0.0f, 0.0f, bBlockD , 1.0f, 1.0f,
    bBlockW, 0.0f, 0.0f , 0.0f, 0.0f, 
      
    0.0f, 0.0f, bBlockD , 1.0f, 1.0f,
    bBlockW, 0.0f, 0.0f , 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f , 1.0f, 0.0f, 
  };
  
  int* geomentyByTxCounter = calloc(loadedTexturesCounter, sizeof(int));
  
  for (int y = 0; y < gridY; y++) {
    for (int z = 0; z < gridZ; z++) {
      for (int x = 0; x < gridX; x++) {
	if(grid[y][z][x]){
	  GroundType type = valueIn(grid[y][z][x]->ground, 0);

	  if(type == texturedTile){
	    int txIndex = valueIn(grid[y][z][x]->ground, 2);
	    
	    geomentyByTxCounter[txIndex] += sizeof(texturedTileVerts);
	  }

	  // block
	  if(grid[y][z][x]->block){
	    TileBlocksTypes type = grid[y][z][x]->block->type;
		int txIndex = grid[y][z][x]->block->txIndex;

	    geomentyByTxCounter[txIndex] += tileBlocksTempl[type].vertexesSize * sizeof(float) * 5;
	  }

	  // walls
	  for(int i=0;i<4;i++){
	    if(grid[y][z][x]->walls[i].planes){
	      WallType type = grid[y][z][x]->walls[i].type;
	      
	      for(int i2=0;i2<wallsVPairs[type].planesNum;i2++){
		if(!grid[y][z][x]->walls[i].planes[i2].hide){
		  int txIndex = grid[y][z][x]->walls[i].planes[i2].txIndex;
		  geomentyByTxCounter[txIndex] += wallsVPairs[type].pairs[i2].vertexNum * sizeof(float) * 5;
		}
	      }
	    }

	    if(grid[y][z][x]->jointExist[i]){
	      for(int i2=0;i2<wallsVPairs[wallJointT].planesNum;i2++){
		int txIndex = grid[y][z][x]->joint[i][i2].txIndex;
		geomentyByTxCounter[txIndex] += wallsVPairs[wallJointT].pairs[i2].vertexNum * sizeof(float) * 5;
	      }
	    }
	  }

	  //	  int txIndex = valueIn(grid[y][z][x]->ground, 2);
	    
	  //	  geomentyByTxCounter[txIndex] += sizeof(texturedTileVerts);
	}
      }
    }
  }

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

    geometry[i].tris = geomentyByTxCounter[i] / 5 / sizeof(float);
    printf("Tris: %d Size %d \n", geometry[i].tris, geometry[i].size);
  }

  free(geomentyByTxCounter);
  geomentyByTxCounter = calloc(loadedTexturesCounter, sizeof(int));

  for (int y = 0; y < gridY; y++) {
    for (int z = 0; z < gridZ; z++) {
      for (int x = 0; x < gridX; x++) {
	if (grid[y][z][x]) {
	  GroundType type = valueIn(grid[y][z][x]->ground, 0);

	  if (type == texturedTile) {
	    vec3 tile = xyz_indexesToCoords(x,y,z);
	    int txIndex = valueIn(grid[y][z][x]->ground, 2);

	    for(int i=0;i<6*5;i+=5){
	      geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i] = texturedTileVerts[i] + tile.x; 
	      geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+1] = texturedTileVerts[i+1] + tile.y;
	      geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+2] = texturedTileVerts[i+2] + tile.z;

	      geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+3] = texturedTileVerts[i+3];
	      geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+4] = texturedTileVerts[i+4];
	    }

	    geomentyByTxCounter[txIndex]+=6*5;
	  }
	  
	  // block
	  if(grid[y][z][x]->block){
	    TileBlocksTypes type = grid[y][z][x]->block->type;
	    int txIndex = grid[y][z][x]->block->txIndex;

	    for(int i=0;i<tileBlocksTempl[type].vertexesSize * 5;i+=5){
	      vec4 vert = { tileBlocksTempl[type].vertexes[i], tileBlocksTempl[type].vertexes[i+1], tileBlocksTempl[type].vertexes[i+2], 1.0f };

	      vec4 transf = mulmatvec4(grid[y][z][x]->block->mat, vert);

	      geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i] = transf.x; 
	      geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+1] = transf.y;
	      geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+2] = transf.z;

	      geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+3] = tileBlocksTempl[type].vertexes[i+3];
	      geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+4] = tileBlocksTempl[type].vertexes[i+4];
	    }
	    
	    geomentyByTxCounter[txIndex] += tileBlocksTempl[type].vertexesSize * 5;
	  }

	  // walls
	  for(int i3=0;i3<4;i3++){
	    if(grid[y][z][x]->walls[i3].planes){
	      WallType type = grid[y][z][x]->walls[i3].type;
	      
	      for(int i2=0;i2<wallsVPairs[type].planesNum;i2++){
		if(!grid[y][z][x]->walls[i3].planes[i2].hide){
		  int txIndex = grid[y][z][x]->walls[i3].planes[i2].txIndex;
		  
		  for(int i=0;i<wallsVPairs[type].pairs[i2].vertexNum * 5;i+=5){
		    vec4 vert = { wallsVPairs[type].pairs[i2].vBuf[i], wallsVPairs[type].pairs[i2].vBuf[i+1], wallsVPairs[type].pairs[i2].vBuf[i+2], 1.0f };

		    vec4 transf = mulmatvec4(grid[y][z][x]->walls[i3].mat, vert);

		    geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i] = transf.x; 
		    geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+1] = transf.y;
		    geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+2] = transf.z;

		    geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+3] = wallsVPairs[type].pairs[i2].vBuf[i+3];
		    geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+4] = wallsVPairs[type].pairs[i2].vBuf[i+4];
		  }

		  geomentyByTxCounter[txIndex] += wallsVPairs[type].pairs[i2].vertexNum * 5;
		}
	      }
	    }

	    if(grid[y][z][x]->jointExist[i3]){
	      for(int i2=0;i2<wallsVPairs[wallJointT].planesNum;i2++){
		int txIndex = grid[y][z][x]->joint[i3][i2].txIndex;
		
		for(int i=0;i<wallsVPairs[wallJointT].pairs[i2].vertexNum * 5;i+=5){
		  vec4 vert = { wallsVPairs[wallJointT].pairs[i2].vBuf[i], wallsVPairs[wallJointT].pairs[i2].vBuf[i+1], wallsVPairs[wallJointT].pairs[i2].vBuf[i+2], 1.0f };

		  vec4 transf = mulmatvec4(grid[y][z][x]->jointsMat[i3], vert);

		  geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i] = transf.x; 
		  geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+1] = transf.y;
		  geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+2] = transf.z;

		  geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+3] = wallsVPairs[wallJointT].pairs[i2].vBuf[i+3];
		  geometry[txIndex].verts[geomentyByTxCounter[txIndex]+i+4] = wallsVPairs[wallJointT].pairs[i2].vBuf[i+4];
		}
		
		geomentyByTxCounter[txIndex] += wallsVPairs[wallJointT].pairs[i2].vertexNum * 5;
	      }
	    }
	  }
	}
      }
    }
  }

  for(int i=0;i<loadedTexturesCounter;i++){
    glBindVertexArray(geometry[i].pairs.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, geometry[i].pairs.VBO);
  
    glBufferData(GL_ARRAY_BUFFER, geometry[i].size, geometry[i].verts, GL_STATIC_DRAW);
  
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  free(geomentyByTxCounter);
}
