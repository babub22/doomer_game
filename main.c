#include "deps.h"
#include "linearAlg.h"
#include "main.h"

GLuint textVBO;
GLuint textVAO;

char curSaveName[CONSOLE_BUF_CAP];

Texture* loadedTextures1D;
Texture** loadedTextures2D;
char** loadedTexturesNames; // iter same as tex1D
int loadedTexturesCategoryCounter;
Category* loadedTexturesCategories;
int loadedTexturesCounter;
int longestTextureNameLen;
int longestTextureCategoryLen;

GLuint selectionRectVBO;
GLuint selectionRectVAO;

GLuint solidColorTx;
GLuint errorTx;

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

Tile*** grid;
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

GLuint objectsMenuTypeRectVBO;
GLuint objectsMenuTypeRectVAO;

Menu dialogViewer= { .type = dialogViewerT };
Menu objectsMenu = { .type = objectsMenuT };
Menu dialogEditor = { .type = dialogEditorT };
Menu blocksMenu = { .type = blocksMenuT };
Menu texturesMenu = { .type = texturesMenuT };
Menu planeCreatorMenu = { .type = planeCreatorT };

Plane* planeOnCreation;

int texturesMenuCurCategoryIndex = 0;

Plane* createdPlanes;
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

const float wallD = 0.0012f;

const Sizes wallsSizes[wallTypeCounter+1] = { {0}, {bBlockW,bBlockH,bBlockD}, {bBlockW,bBlockH * 0.4f,bBlockD}, {bBlockW,bBlockH,bBlockD}, {bBlockW,bBlockH,bBlockD}};

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

  // setup nettile buffer
  {
    // center mouse selection
    /*    glGenVertexArrays(1, &netTileVAO);
	  glBindVertexArray(netTileVAO);

	  glGenBuffers(1, &netTileVBO);
	  glBindBuffer(GL_ARRAY_BUFFER, netTileVBO);

    
	  float centerMouseSel[] = {
	  0.0f,0.0f,0.0f, 
	  borderArea * 2,0.0f,0.0f, 
	  borderArea * 2, 0.0f, borderArea * 2,

	  0.0f,0.0f,0.0f,
	  0.0f,0.0f,borderArea * 2,
	  borderArea * 2, 0.0f, borderArea * 2,
	  };
    
	  glBufferData(GL_ARRAY_BUFFER, sizeof(centerMouseSel), centerMouseSel, GL_STATIC_DRAW);

	  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	  glEnableVertexAttribArray(0);

	  glBindBuffer(GL_ARRAY_BUFFER, 0);
	  glBindVertexArray(0); */
    
    // left/right mouse


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
	tileBlocksTempl = malloc(sizeof(TileBlock));
      }else{
	tileBlocksTempl = realloc(tileBlocksTemplSize, sizeof(TileBlock) * tileBlocksTemplSize);
      }
      
      glGenVertexArrays(1, &tileBlocksTempl[tileBlocksTemplSize].vpair.VAO);
      glBindVertexArray(tileBlocksTempl[tileBlocksTemplSize].vpair.VAO);

      glGenBuffers(1, &tileBlocksTempl[tileBlocksTemplSize].vpair.VBO);
      glBindBuffer(GL_ARRAY_BUFFER, tileBlocksTempl[tileBlocksTemplSize].vpair.VBO);
      
      float roofBlock[] = {
	0.0f, floorH, 0.0f, 1.0f, 1.0f,
	bBlockW, floorH, 0.0f , 0.0f, 0.0f,
	0.0f, 0.0f, bBlockD , 1.0f, 0.0f,

	bBlockW, floorH, 0.0f , 0.0f, 1.0f,
	0.0f, 0.0f, bBlockD, 1.0f, 0.0f,
	bBlockW, 0.0f, bBlockD, 0.0f, 0.0f, 
      };
      
      tileBlocksTempl[tileBlocksTemplSize].vertexes = malloc(sizeof(roofBlock));
      memcpy(tileBlocksTempl[tileBlocksTemplSize].vertexes, roofBlock, sizeof(roofBlock));
      tileBlocksTempl[tileBlocksTemplSize].vertexesSize = 6;
      tileBlocksTempl[tileBlocksTemplSize].type = roofBlockT;

      tileBlocksTemplSize++;

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

    // stepsBlock
    {
      glGenVertexArrays(1, &stepsBlock.VAO);
      glBindVertexArray(stepsBlock.VAO);

      glGenBuffers(1, &stepsBlock.VBO);
      glBindBuffer(GL_ARRAY_BUFFER, stepsBlock.VBO);

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
	
	bBlockW, floorH - stepH * 2, stepW *2 , 0.0f, 0.0f,
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

      glBufferData(GL_ARRAY_BUFFER, sizeof(texturedTileVerts), texturedTileVerts, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
      glEnableVertexAttribArray(1);

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
	  bBlockW, -wallD, bBlockD , 0.0f, 1.0f,
	  0.0f, -wallD, bBlockD , 1.0f, 1.0f,
	  bBlockW, -wallD, 0.0f , 0.0f, 0.0f, 
      
	  0.0f, -wallD, bBlockD , 1.0f, 1.0f,
	  bBlockW, -wallD, 0.0f , 0.0f, 0.0f,
	  0.0f, -wallD, 0.0f , 1.0f, 0.0f, 
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(texturedTileVerts), texturedTileVerts, GL_STATIC_DRAW);
      }else if(i == fromOver){
	float texturedTileVerts[] = {
	  bBlockW, wallD, bBlockD , 0.0f, 1.0f,
	  0.0f, wallD, bBlockD , 1.0f, 1.0f,
	  bBlockW, wallD, 0.0f , 0.0f, 0.0f, 
      
	  0.0f, wallD, bBlockD , 1.0f, 1.0f,
	  bBlockW, wallD, 0.0f , 0.0f, 0.0f,
	  0.0f, wallD, 0.0f , 1.0f, 0.0f, 
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
  
  wallsLoadVAOandVBO();
  
  // snowflakes
  {
    glGenVertexArrays(1, &snowFlakesMeshVAO);
    glBindVertexArray(snowFlakesMeshVAO);

    glGenBuffers(1, &snowFlakesMeshVBO);
    glBindBuffer(GL_ARRAY_BUFFER, snowFlakesMeshVBO);


    /*
      float snowFlakesMeshVerts[] = {
      0.0f, 0.0f, 0.0f, 
      0.0f, 0.015f / 4.0f, 0.0f  };

      glBufferData(GL_ARRAY_BUFFER, sizeof(snowFlakesMeshVerts)
      ,snowFlakesMeshVerts, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);*/

    //    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
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

  int modelLoc = glGetUniformLocation(mainShader, "model");
  int projUni = glGetUniformLocation(mainShader, "proj");
  int viewLoc = glGetUniformLocation(mainShader, "view");

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
  
  int floor = 0;
  bool highlighting = 1;
  
  // init opengl
  {
    //  glEnable(GL_MULTISAMPLE);  
    
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);  
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_MULTISAMPLE);  
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
      printf("Cat %s %d \n", loadedTexturesCategories[i2].name,loadedTexturesCategories[i2].txWithThisCategorySize);
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


  /*
    GLuint carTx;

    {
    glGenTextures(1, &carTx);

    // -1 because of solidColorTx
    SDL_Surface* texture = IMG_Load("./assets/objs/car.png");

    if (!texture) {
    printf("Loading of texture .png\" failed");
    exit(0);
    }

    glBindTexture(GL_TEXTURE_2D, carTx);
      
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
  */ 
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

  int textureOfGround = texture1DIndexByName("Zemlia1");

  if (textureOfGround == -1) {
    printf("Specify texture of ground");
    exit(-1);
  }

  // load or init grid
  if(!loadSave("map")){
    grid = malloc(sizeof(Tile**) * (gridY));

    for (int y = 0; y < gridY; y++) {
      grid[y] = malloc(sizeof(Tile*) * (gridZ));

      for (int z = 0; z < gridZ; z++) {
	grid[y][z] = calloc(gridX, sizeof(Tile));

	for (int x = 0; x < gridX; x++) {
	  if (y == 0) {
	    setIn(grid[y][z][x].ground, 0, texturedTile);
	    setIn(grid[y][z][x].ground, 2, textureOfGround);
	  }
	  else {
	    setIn(grid[y][z][x].ground, 0, netTile);
	  }
	}
      }
    }

    printf("Map not found!\n");
  }

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
  drawDistance = gridX/2 * 0.2;
  glUniform1f(radius, drawDistance);

  ManipulationMode manipulationMode = -1;
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

    cameraSpeed = 0.5f * deltaTime;

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
	    Model* model = (Model*)mouse.focusedThing;

	    characters[model->characterId].curDialogIndex = 0;
	    characters[model->characterId].curDialog = &characters[model->characterId].dialogs;

	    dialogEditor.open = false;
	    curMenu = NULL;

	    if(tempTextInputStorageCursor!=0){
	      tempTextInputStorageCursor=0;
	      memset(tempTextInputStorage, 0, 512 * sizeof(char)); 
	    }
	  }else if(event.key.keysym.scancode == SDL_SCANCODE_T && !selectedTextInput){
	    Model* model = (Model*)mouse.focusedThing;

	    characters[model->characterId].curDialogIndex = 0;
	    characters[model->characterId].curDialog = &characters[model->characterId].dialogs;

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
	  }
	  case(SDL_SCANCODE_UP): {
	    if(manipulationMode != -1){
	      Matrix* mat = -1;

	      if (mouse.focusedType == mouseModelT) {
		Model* model = (Model*)mouse.focusedThing;
		mat = &model->mat;

	      }
	      else if (mouse.focusedType == mousePlaneT) {
		Plane* plane = (Plane*)mouse.focusedThing;
		mat = &plane->mat;
	      }


	      switch(manipulationMode){
	      case(TRANSFORM_Z):{
		mat->m[13] = mat->m[13] + manipulationStep;
		//  calculateModelAABB(model);
		break;
	      }
	      case(TRANSFORM_XY): {
		mat->m[12] = mat->m[12] - manipulationStep;
		//  calculateModelAABB(model);
		break;
	      }
	      case(SCALE): {
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

		//  calculateModelAABB(model);
		break;
	      }

	      default: break;
	      }
	    }
	    else if(mouse.brushThing && mouse.brushType == mouseBlockBrushT){
	      TileBlock* block = (TileBlock*) mouse.brushThing;

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

   	      vec3 pos = vec3_indexesToCoords(data->grid);
	      vec3 forw = {0,0,1};

	      if(data->side == left || data->side == right) {
		forw = (vec3){ 1,0,0 };
	      }

	      float dot1 = dotf3(pos, forw);
	      float dot2 = dotf3(curCamera->pos, forw);

	      bool cameraInFront = dot1 < dot2;

	      float dStep = 0.0f;

	      if (cameraInFront) {
		if (grid[data->grid.y][data->grid.z][data->grid.x].wallsPad[data->side] - manipulationStep >= -bBlockW / 2) {
		  dStep = -manipulationStep;
		}
	      }
	      else {
		if (grid[data->grid.y][data->grid.z][data->grid.x].wallsPad[data->side] + manipulationStep < bBlockW / 2) {
		  dStep = manipulationStep;
		}
	      }

	      grid[data->grid.y][data->grid.z][data->grid.x].wallsPad[data->side] += dStep;
	      
	      vec2i oppositeTile = {0};
	      int oppositeSide = -1;
	      
	      if(oppositeTileTo((vec2i) { data->grid.x, data->grid.z }, data->side, & oppositeTile, & oppositeSide)) {
		grid[data->grid.y][oppositeTile.z][oppositeTile.x].wallsPad[oppositeSide] += dStep;
	      }

	      /*
		Texture nextTx = 0;

		if (mouse.wallTx != texturesCounter - 1) {
		nextTx = mouse.wallTx + 1;
		}
		else {
		nextTx = 0;
		}

		setIn(grid[mouse.wallTile.y][mouse.wallTile.z][mouse.wallTile.x].wallsTx, mouse.wallSide, nextTx);*/
	    }
	    else if (mouse.selectedType == mouseWallT) {
	      TileMouseData* data = (TileMouseData*)mouse.selectedThing;

	      if(data->tile->groundLift + manipulationStep < floorH) {
		data->tile->groundLift += manipulationStep;
	      }
	      
	      /*
		GroundType type = valueIn(grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, 0);

		if (type != texturedTile) {
		setIn(grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, 0, texturedTile);
		setIn(grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, mouse.groundInter, 0);
		}
		else {
		Texture curTx = valueIn(grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, mouse.groundInter);

		if (curTx == texturesCounter - 1) {
		setIn(grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, 0, netTile);
		}
		else {
		setIn(grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, mouse.groundInter, curTx + 1);
		}
		}
	      */
	    }

	    break;
	  }
	  case(SDL_SCANCODE_DOWN): {
	    // TODO: if intersected tile + wall will work only tile changer
	    if(manipulationMode != -1){
	      Matrix* mat = -1;

	      if (mouse.focusedType == mouseModelT) {
		Model* model = (Model*)mouse.focusedThing;
		mat = &model->mat;

	      }
	      else if (mouse.focusedType == mousePlaneT) {
		Plane* plane = (Plane*)mouse.focusedThing;
		mat = &plane->mat;
	      }

	      switch(manipulationMode){ 
	      case(TRANSFORM_Z):{
		mat->m[13] = mat->m[13] - manipulationStep;
		//calculateModelAABB(model);
		break;
	      }
	      case(TRANSFORM_XY): {
		mat->m[12] = mat->m[12] + manipulationStep;
		// calculateModelAABB(model);
		break;
	      }
	      case(SCALE): {
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

		// calculateModelAABB(model);
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

	      vec3 pos = vec3_indexesToCoords(data->grid);
	      vec3 forw = {0,0,1};

	      if(data->side == left || data->side == right) {
		forw = (vec3){ 1,0,0 };
	      }

	      float dot1 = dotf3(pos, forw);
	      float dot2 = dotf3(curCamera->pos, forw);

	      bool cameraInFront = dot1 < dot2;

	      float dStep = 0.0f;

	      if (cameraInFront) {
		if (grid[data->grid.y][data->grid.z][data->grid.x].wallsPad[data->side] + manipulationStep < bBlockW / 2) {
		  dStep = manipulationStep; 
		}
	      }
	      else {
		if (grid[data->grid.y][data->grid.z][data->grid.x].wallsPad[data->side] - manipulationStep >= -bBlockW / 2) {
		  dStep = -manipulationStep;
		}
	      }

	      grid[data->grid.y][data->grid.z][data->grid.x].wallsPad[data->side] += dStep;

	      vec2i oppositeTile = { 0 };
	      int oppositeSide = -1;

	      if (oppositeTileTo((vec2i) { data->grid.x, data->grid.z }, data->side, & oppositeTile, & oppositeSide)) {
		grid[data->grid.y][oppositeTile.z][oppositeTile.x].wallsPad[oppositeSide] += dStep;
	      }
	    }
	    else if (mouse.selectedType == mouseTileT) {
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
	      if(mouse.selectedType == mousePlaneT || mouse.selectedType == mouseModelT || mouse.selectedType == mouseBlockT){
		mouse.focusedThing = mouse.selectedThing;
		mouse.focusedType = mouse.selectedType;
	      }
	    }else{
	      mouse.focusedThing = NULL;
	      mouse.focusedType = 0;
	      manipulationMode = -1;	
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
	    if(mouse.brushThing){
	      if (mouse.brushType == mouseBlockBrushT) {
		TileBlock* block = (TileBlock*) mouse.brushThing;
		free(block->vertexes);
		free(block);
	      }

	      mouse.brushThing = NULL;
	      mouse.brushType = 0;
	    }else{
	      drawDistance -= .1f / 2.0f;

	      if(enviromental.fog){
		glUniform1f(radius, drawDistance);
	      }
	    }

	    break;
	  }
	  case(SDL_SCANCODE_LEFT): {
	    if(manipulationMode != -1){
	      Matrix* mat = -1;

	      if (mouse.focusedType == mouseModelT) {
		Model* model = (Model*)mouse.focusedThing;
		mat = &model->mat;

	      }
	      else if (mouse.focusedType == mousePlaneT) {
		Plane* plane = (Plane*)mouse.focusedThing;
		mat = &plane->mat;
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

		// calculateModelAABB(model);
	      }
	      else if (manipulationMode == TRANSFORM_XY) {
		mat->m[14] = mat->m[14] + manipulationStep;
		//  calculateModelAABB(model);
	      }
	    }
	    break;
	  }case(SDL_SCANCODE_RIGHT): {
	      if(manipulationMode != -1){
		Matrix* mat = -1;

		if (mouse.focusedType == mouseModelT) {
		  Model* model = (Model*)mouse.focusedThing;
		  mat = &model->mat;

		}
		else if (mouse.focusedType == mousePlaneT) {
		  Plane* plane = (Plane*)mouse.focusedThing;
		  mat = &plane->mat;
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

		  //calculateModelAABB(model);
		}

		switch (manipulationMode) {
		case(TRANSFORM_XY): {
		  mat->m[14] = mat->m[14] - manipulationStep;
		  // calculateModelAABB(model);
		  break;
		}

		default: break;
		}
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
			Plane* plane = (Plane*)mouse.focusedThing;
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
		  }
		    case(SDL_SCANCODE_EQUALS): { 
		      if (floor < gridY - 1) {
			floor++;
		      }

		      break;
		    }
		      case(SDL_SCANCODE_MINUS): {
			if (floor != 0) {
			  floor--;
			}

			break;
		      }
			case(SDL_SCANCODE_V): {
			  enviromental.snow = !enviromental.snow;

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
			     if(!curMenu || curMenu->type == planeCreatorT){
			       if(planeOnCreation){
				 free(planeOnCreation);
				 planeOnCreation = NULL;
			       }
	       
			       planeCreatorMenu.open = !planeCreatorMenu.open;
			       curMenu = planeCreatorMenu.open ? &planeCreatorMenu : NULL;
			     }
	     
			     break;
			   }case(SDL_SCANCODE_T):{
			      if(!curMenu || curMenu->type == texturesMenuT){
				texturesMenu.open = !texturesMenu.open;
				curMenu = texturesMenu.open ? &texturesMenu : NULL;
			      }
	     
			      break;
			    }
			    case(SDL_SCANCODE_H): {
			      /* const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
	    
				 if(mouse.selectedType == mouseWallT && currentKeyStates[SDL_SCANCODE_LCTRL]){
				 WallMouseData* data = (WallMouseData*)mouse.selectedThing;

				 // Tile* tile =  &grid[floor][mouse.wallTile.z][mouse.wallTile.x];
				 Tile* oppositeGrid = NULL;

				 TileBlock* block = data->tile->block;
	      
				 vec2i oppositeTile;
				 Side oppositeSide;
	  
				 if (oppositeTileTo((vec2i) { data->grid.x, data->grid.z }, data->side, & oppositeTile, & oppositeSide)) {
				 oppositeGrid = &grid[floor][oppositeTile.z][oppositeTile.x];

				 if (!block) {
				 block = oppositeGrid->block;
				 }

				 if (block && block->type == roofBlockT) {
				 if (!data->tile->customWalls[data->side].alligned && !data->tile->customWalls[data->side].alligned) {
				 if (data->side == top) {
				 float xLeft = 0.0f;
				 float xRight = bBlockW;

				 float newH = block->vertexes[1];

				 float newV = map(newH, 0, 0.2, 0, 1);

				 printf("%f \n", newH * 10);

				 if (oppositeGrid) {
				 oppositeGrid->customWalls[oppositeSide].buf[1] = newH;
				 oppositeGrid->customWalls[oppositeSide].buf[4] = newV;

				 oppositeGrid->customWalls[oppositeSide].buf[6] = newH;
				 oppositeGrid->customWalls[oppositeSide].buf[9] = newV;

				 oppositeGrid->customWalls[oppositeSide].buf[16] = newH;
				 oppositeGrid->customWalls[oppositeSide].buf[19] = newV;

				 oppositeGrid->customWalls[oppositeSide].alligned = true;
				 }

				 data->tile->customWalls[data->side].buf[1] = newH;
				 data->tile->customWalls[data->side].buf[4] = newV;

				 data->tile->customWalls[data->side].buf[6] = newH;
				 data->tile->customWalls[data->side].buf[9] = newV;

				 data->tile->customWalls[data->side].buf[16] = newH;
				 data->tile->customWalls[data->side].buf[19] = newV;

				 data->tile->customWalls[data->side].alligned = true;
				 }
				 else if (data->side == bot) {
				 float xLeft = 0.0f;
				 float xRight = bBlockW;

				 float newH = block->vertexes[11];

				 float newV = map(newH, 0, 0.2, 0, 1);

				 printf("%f \n", newH * 10);

				 if (oppositeGrid) {
				 oppositeGrid->customWalls[oppositeSide].buf[1] = newH;
				 oppositeGrid->customWalls[oppositeSide].buf[4] = newV;

				 oppositeGrid->customWalls[oppositeSide].buf[6] = newH;
				 oppositeGrid->customWalls[oppositeSide].buf[9] = newV;

				 oppositeGrid->customWalls[oppositeSide].buf[16] = newH;
				 oppositeGrid->customWalls[oppositeSide].buf[19] = newV;

				 oppositeGrid->customWalls[oppositeSide].alligned = true;
				 }

				 data->tile->customWalls[data->side].buf[1] = newH;
				 data->tile->customWalls[data->side].buf[4] = newV;

				 data->tile->customWalls[data->side].buf[6] = newH;
				 data->tile->customWalls[data->side].buf[9] = newV;

				 data->tile->customWalls[data->side].buf[16] = newH;
				 data->tile->customWalls[data->side].buf[19] = newV;

				 data->tile->customWalls[data->side].alligned = true;
				 }
				 else if (data->side == left || data->side == right) {
				 float xLeft = 0.0f;
				 float xRight = bBlockW;

				 float newH = block->vertexes[1];
				 float backH = block->vertexes[11];

				 float newV = map(newH, 0, 0.2, 0, 1);
				 float backV = map(backH, 0, 0.2, 0, 1);

				 printf("%f \n", newH * 10);

				 if (oppositeGrid) {
				 oppositeGrid->customWalls[oppositeSide].buf[1] = backH; //newH;
				 oppositeGrid->customWalls[oppositeSide].buf[4] = backV;//newV;

				 oppositeGrid->customWalls[oppositeSide].buf[6] = newH;
				 oppositeGrid->customWalls[oppositeSide].buf[9] = newV;

				 oppositeGrid->customWalls[oppositeSide].buf[16] = newH;
				 oppositeGrid->customWalls[oppositeSide].buf[19] = newV;

				 oppositeGrid->customWalls[oppositeSide].alligned = true;
				 }

				 data->tile->customWalls[data->side].buf[1] = backH;
				 data->tile->customWalls[data->side].buf[4] = backV;

				 data->tile->customWalls[data->side].buf[6] = newH;
				 data->tile->customWalls[data->side].buf[9] = newV;

				 data->tile->customWalls[data->side].buf[16] = newH;
				 data->tile->customWalls[data->side].buf[19] = newV;

				 data->tile->customWalls[data->side].alligned = true;
				 }
				 }
				 else {
				 // back to default
				 memcpy(data->tile->customWalls[data->side].buf, customWallTemp[data->side], sizeof(float) * 6 * 5);
				 data->tile->customWalls[data->side].bufSize = 6 * 5;
				 data->tile->customWalls[data->side].alligned = false;

				 if (oppositeGrid) {
				 memcpy(oppositeGrid->customWalls[oppositeSide].buf, customWallTemp[oppositeSide], sizeof(float) * 6 * 5);
				 oppositeGrid->customWalls[oppositeSide].bufSize = 6*5;
				 oppositeGrid->customWalls[oppositeSide].alligned = false;
				 }

				 }
		  
				 // top -> x0 y1 z2
				 //
				 }	      
				 }

	      
				 }else{
				 highlighting = !highlighting;
				 }
	    
				 break;
				 }
				 case(SDL_SCANCODE_DELETE): {
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
				 }
				 }else if (mouse.selectedType == mouseWallT) {
				 // WallType type = (grid[mouse.wallTile.y][mouse.wallTile.z][mouse.wallTile.x].walls >> (mouse.wallSide * 8)) & 0xFF;
				 WallMouseData* data = (WallMouseData*)mouse.selectedThing;

				 Side oppositeSide = 0;
				 vec2i oppositeTile = { 0 };

				 if(grid[data->grid.y][data->grid.z][data->grid.x].customWalls[data->side].buf) {
				 grid[data->grid.y][data->grid.z][data->grid.x].customWalls[data->side].bufSize = 0;
				 free(grid[data->grid.y][data->grid.z][data->grid.x].customWalls[data->side].buf);
				 grid[data->grid.y][data->grid.z][data->grid.x].customWalls[data->side].buf = NULL;

				 if (oppositeTileTo((vec2i) { data->grid.x, data->grid.z }, data->side, & oppositeTile, & oppositeSide)) {
				 free(grid[data->grid.y][oppositeTile.z][oppositeTile.x].customWalls[oppositeSide].buf);
				 grid[data->grid.y][oppositeTile.z][oppositeTile.x].customWalls[oppositeSide].buf = NULL;
				 grid[data->grid.y][oppositeTile.z][oppositeTile.x].customWalls[oppositeSide].bufSize = 0;
				 }
				 }

				 }
			      */
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

	    if(mouse.focusedType == mouseModelT){
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

		  if (grid[floor][(int)tile.z][(int)tile.x].walls != 0) {
		  for (int side = 0; side < basicSideCounter; side++) {
		  WallType type = (grid[floor][(int)tile.z][(int)tile.x].walls >> (side * 8)) & 0xFF;
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
	    if (currentKeyStates[SDL_SCANCODE_0]) {
	      //mouse.brush = 0;
	    }

	    if (currentKeyStates[SDL_SCANCODE_1]) {
	      //mouse.brush = wallT;
	    }

	    // ~~~~~~~~~~~~~~~~~~
	    // Manipulate of focused model
	    //if(!mouse.focusedModel){
	    //manipulationMode = -1;
	    //}
      
	    if ((mouse.focusedType == mouseModelT || mouse.focusedType == mousePlaneT) && currentKeyStates[SDL_SCANCODE_LCTRL]) {

	      if(currentKeyStates[SDL_SCANCODE_P]){
		if(mouse.selectedType == mouseTileT){
		  Matrix* mat = -1;

		  if (mouse.focusedType == mouseModelT) {
		    Model* model = (Model*)mouse.focusedThing;
		    mat = &model->mat;

		  }
		  else if (mouse.focusedType == mousePlaneT) {
		    Plane* plane = (Plane*)mouse.focusedThing;
		    mat = &plane->mat;
		  }
		  TileMouseData* tileData = (TileMouseData*) mouse.selectedThing;
		  vec3 tile = xyz_indexesToCoords(tileData->grid.x, floor, tileData->grid.z);

		  mat->m[12] = tile.x;
		  mat->m[13] = tile.y;
		  mat->m[14] = tile.z;
	    
		  // calculateModelAABB(mouse.focusedModel);
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
		// Transform
		if(currentKeyStates[SDL_SCANCODE_Z]){
		  manipulationMode = TRANSFORM_Z;
		}else{
		  manipulationMode = TRANSFORM_XY;
		}
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
	  Matrix proj = perspective(rad(fov), windowW / windowH, 0.01f, 10.0f);
	  glUniformMatrix4fv(projUni, 1, GL_FALSE, proj.m);

	  Matrix view =  IDENTITY_MATRIX;
	  vec3 negPos = { -curCamera->pos.x, -curCamera->pos.y, -curCamera->pos.z };

	  translate(&view, argVec3(negPos));
	  rotateY(&view, rad(curCamera->yaw));
	  rotateX(&view, rad(curCamera->pitch));
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
	  
	{
	  glBindBuffer(GL_ARRAY_BUFFER, netTileVBO);
	  glBindVertexArray(netTileVAO);
	    
	  for (int x = 0; x < gridX; x++) {
	    for (int z = 0; z < gridZ; z++){

	      GroundType type = valueIn(grid[floor][z][x].ground, 0);

	      if(type == texturedTile){
		continue;
	      }
		
	      vec3 tile = {(float)x * bBlockW, (float)floor * floorH, (float)z * bBlockD};

	      /*
		const vec3 c0 = { tile.x, tile.y, tile.z };
		const vec3 c1 = { tile.x + bBlockW, tile.y, tile.z };
		const vec3 c2 = { tile.x, tile.y, tile.z + bBlockD };
		const vec3 c3 = { tile.x + bBlockW, tile.y, tile.z + bBlockD };

		vec3 tileCornesrs[] = { c0, c1, c2, c3 };

		int in=0;

		for (int k = 0; k < 4 && in==0; k++) {
		if (radarCheck(tileCornesrs[k]))
		in++;
		}
	    
		if(!in){
		continue;
		}*/
		
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

	if((mouse.focusedType != mouseWallT && mouse.selectedType == mouseWallT) || (mouse.focusedType != mouseTileT && mouse.selectedType == mouseTileT)){
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

	    glEnable(GL_BLEND);
	    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
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
	  glDisable(GL_BLEND);
	}

	// plane things
	{
	  // draw plane on cur creation
	  glBindVertexArray(planePairs.VAO);
	  glBindBuffer(GL_ARRAY_BUFFER, planePairs.VBO);

	  glEnable(GL_BLEND);
	  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
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

	    Plane plane = createdPlanes[i];
	
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

	    glEnable(GL_BLEND);
	    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
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

	  glDisable(GL_BLEND);
    
	  glBindVertexArray(0);
	  glBindBuffer(GL_ARRAY_BUFFER, 0);

	  glBindTexture(GL_TEXTURE_2D, 0);
	  glDisable(GL_BLEND);
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
		      
		  //	      float snowFlakesMeshVerts[] = {
		  //		0.0f, 0.0f, 0.0f, 
		  //		0.0f, 0.015f / 4.0f, 0.0f  };

		  //	      glDrawArrays(GL_LINES, 0, 2);
		  //		}

		  //		vec3i gridIndexes = xyz_indexesToCoords(x,y,z);
		  vec3i gridIndexes = xyz_coordsToIndexes(x, y, z);

		  GroundType type = -1;

		  if (gridIndexes.y < gridY - 1) {
		    type = valueIn(grid[gridIndexes.y + 1][gridIndexes.z][gridIndexes.x].ground, 0);
		  }

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
	      vec3 tile = xyz_indexesToCoords(x,y,z);

	      // block
	      if(grid[y][z][x].block != NULL){

		vec3 lb = { 1000,1000,1000 };
		vec3 rt = {0};

		for(int i3=0;i3<grid[y][z][x].block->vertexesSize;i3 += 5){
		  lb.x= min(lb.x, grid[y][z][x].block->vertexes[i3]);
		  lb.y= min(lb.y, grid[y][z][x].block->vertexes[i3+1]);
		  lb.z= min(lb.z, grid[y][z][x].block->vertexes[i3+2]);

		  rt.x= max(rt.x, grid[y][z][x].block->vertexes[i3]);
		  rt.y= max(rt.y, grid[y][z][x].block->vertexes[i3+1]);
		  rt.z= max(rt.z, grid[y][z][x].block->vertexes[i3+2]);
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
		  mouse.selectedThing = grid[y][z][x].block;
		  mouse.selectedType = mouseBlockT;

		  minDistToCamera = intersectionDistance;
		}
	    
		glBindVertexArray(grid[y][z][x].block->vpair.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, grid[y][z][x].block->vpair.VBO);
		
		glActiveTexture(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	      		
		Matrix out = IDENTITY_MATRIX;

		// translate without mult
		out.m[12] = tile.x;
		out.m[13] = tile.y; 
		out.m[14] = tile.z;
		
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out.m);

		glBufferData(GL_ARRAY_BUFFER, grid[y][z][x].block->vertexesSize * sizeof(float) * 5, grid[y][z][x].block->vertexes, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	  
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	      }
	  
	      // walls
	      if(true){
		for(int side=0;side<basicSideCounter;side++){

		  if(grid[y][z][x].customWalls[side].buf != NULL) {

		    vec3 lb = { 1000,1000,1000 };
		    vec3 rt = {0};

		    for(int i3=0;i3<grid[y][z][x].customWalls[side].bufSize;i3 += 5){
		      lb.x= min(lb.x, grid[y][z][x].customWalls[side].buf[i3]);
		      lb.y= min(lb.y, grid[y][z][x].customWalls[side].buf[i3+1]);
		      lb.z= min(lb.z, grid[y][z][x].customWalls[side].buf[i3+2]);

		      rt.x= max(rt.x, grid[y][z][x].customWalls[side].buf[i3]);
		      rt.y= max(rt.y, grid[y][z][x].customWalls[side].buf[i3+1]);
		      rt.z= max(rt.z, grid[y][z][x].customWalls[side].buf[i3+2]);
		    }

		    lb.x += tile.x;
		    lb.y += tile.y;
		    lb.z += tile.z;

		    rt.x += tile.x;
		    rt.y += tile.y ;
		    rt.z += tile.z ;

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
	    
		    GLuint txIndex = valueIn(grid[y][z][x].wallsTx, side);
	      
		    if(y >= floor){
		      float intersectionDistance;
		      bool isIntersect =  rayIntersectsTriangle(curCamera->pos, mouse.rayDir, lb, rt, NULL, &intersectionDistance);
		  
		      if(isIntersect && minDistToCamera  > intersectionDistance){
		    
			mouse.selectedType = mouseWallT;

			WallMouseData* data = malloc(sizeof(WallMouseData));
			data->side = side;
			data->grid = (vec3i){x,y,z};
			data->txIndex = txIndex;
			data->tile = &grid[y][z][x];
		    
			mouse.selectedThing = data;

			minDistToCamera = intersectionDistance;
			/*		    mouse.wallSide = side;
					    mouse.wallTile = (vec3i){x,y,z};
					    //mouse.wallType = type;
					    mouse.wallTx = tx;

					    minDistType = WallEl;*/
		      }
		    }

		    glActiveTexture(loadedTextures1D[txIndex].tx);
		    glBindTexture(GL_TEXTURE_2D, loadedTextures1D[txIndex].tx);
	      		
		    glBindVertexArray(customWallV.VAO);
		    glBindBuffer(GL_ARRAY_BUFFER, customWallV.VBO);

		    glBufferData(GL_ARRAY_BUFFER, grid[y][z][x].customWalls[side].bufSize * sizeof(float) * 5,  grid[y][z][x].customWalls[side].buf, GL_STATIC_DRAW);

		    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
		    glEnableVertexAttribArray(0);

		    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		    glEnableVertexAttribArray(1);
		    Matrix out = IDENTITY_MATRIX;

		    // translate without mult
		    if(side == left || side == right){
		      out.m[12] = tile.x + grid[y][z][x].wallsPad[side];
		      out.m[13] = tile.y;
		      out.m[14] = tile.z;
		    }else if(side == bot || side == top){
		      out.m[12] = tile.x;
		      out.m[13] = tile.y;
		      out.m[14] = tile.z + grid[y][z][x].wallsPad[side];
		    }
	      
		    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out.m);


		    glDrawArrays(GL_TRIANGLES, 0, grid[y][z][x].customWalls[side].bufSize / 5); // 5 - num of el vec3 + uv 

		    glBindTexture(GL_TEXTURE_2D, 0);
		    glBindBuffer(GL_ARRAY_BUFFER, 0);
		    glBindVertexArray(0);
		
		  }
		}
	      }

	      // tile inter
	      {
		GroundType type = valueIn(grid[y][z][x].ground, 0);

		// skip netTile on not cur floor
		//if(y != floor){
		//  continue;
		//}
	    
		const vec3 tile = xyz_indexesToCoords(x, floor, z);

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

		  if (y == floor) {
		    mouse.selectedType = mouseTileT;
		
		    TileMouseData* data = malloc(sizeof(TileMouseData));

		    data->tile = &grid[floor][z][x];
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

		  if (y == floor) {
		  minIntersectionDist = intersectionDistance;
		
		  mouse.selectedTile = &grid[y][z][x];
		  mouse.gridIntersect = (vec2i){x,z};
		  mouse.intersection = intersection;
		  mouse.groundInter = intersection.y <= curCamera->pos.y ? fromOver : fromUnder;
		 
		  minDistType = TileEl;
		  }
		  }
		*/

		// tile rendering
		if(type == texturedTile){
		  const vec3 tile = xyz_indexesToCoords(x,y,z);

		  for(int i=1;i<=2;i++){
		    if(y==0 && i == fromUnder){
		      continue;
		    }
		
		    int txIndex = valueIn(grid[y][z][x].ground, i);
		    float lift = grid[y][z][x].groundLift;
		  
		    glActiveTexture(loadedTextures1D[txIndex].tx);
		    glBindTexture(GL_TEXTURE_2D, loadedTextures1D[txIndex].tx);

		    glBindBuffer(GL_ARRAY_BUFFER, tileMeshes[i-1].VBO);
		    glBindVertexArray(tileMeshes[i-1].VAO);

		    Matrix out = IDENTITY_MATRIX;
		    //	  translate(&out, argVec3(tile));

		    // translate without mult
		    out.m[12] = tile.x;
		    out.m[13] = tile.y + lift;
		    out.m[14] = tile.z;

		    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out.m);

		    glDrawArrays(GL_TRIANGLES, 0, 6);

		    glBindTexture(GL_TEXTURE_2D, 0);
		    glBindBuffer(GL_ARRAY_BUFFER, 0);
		    glBindVertexArray(0);

		  }
		
		}
	      }
	    }
	  }
	}


	// setup context text
	{
	  if(mouse.focusedThing){
	    switch(mouse.focusedType){
	    case(mouseBlockT):{
	      sprintf(contextBelowText, blockContextText);
	      contextBelowTextH = 1;
	      break;
	    }case(mouseModelT):{
	       sprintf(contextBelowText, modelContextText);
	       contextBelowTextH = 2;
	   
	       break;
	     } case(mousePlaneT):{
		 sprintf(contextBelowText, "Text about plane");
		 contextBelowTextH = 1;
	   
		 break;
	       }
	    default: break;
	    }
	  }else if(mouse.selectedThing){
	    switch(mouse.selectedType){
	    case(mouseWallT):{
	      WallMouseData* data = (WallMouseData*) mouse.selectedThing;

	      bool itHasBlock = data->tile->block;
	      vec2i oppositeTile;
	  
	      if (!itHasBlock && oppositeTileTo((vec2i) { data->grid.x, data->grid.z }, data->side, &oppositeTile, NULL)) {
		Tile* tile = &grid[floor][oppositeTile.z][oppositeTile.x];
		itHasBlock = tile->block;
	      }

	      sprintf(contextBelowText, modelContextText, itHasBlock ? "[LCtrl + H] aling to block" : "");
	      contextBelowTextH = 1;
	  
	      break;
	    }
	    case(mouseTileT):{
	      sprintf(contextBelowText, "Tile text");
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
	  mouse.groundInter = -1;
	  }else if(minDistType == TileEl){
	  mouse.wallSide = -1;
	  mouse.wallTile = (vec3i){ -1,-1,-1 };
	  mouse.wallType = -1;
	  mouse.wallTx = mouse.wallTx = -1;
	  }*/

	// render mouse.brush
	if(mouse.selectedType == mouseTileT)
	  {
	    //	  const float borderArea = bBlockW/8;
	    TileMouseData* tileData = (TileMouseData*) mouse.selectedThing;

	    const vec3 tile = xyz_indexesToCoords(tileData->grid.x,floor, tileData->grid.z);
	
	    if(tileData->intersection.x < tile.x + borderArea) {
	      mouse.tileSide = left;
	    }
	    else if (tileData->intersection.x > tile.x + bBlockW - borderArea) {
	      mouse.tileSide = right;
	    }
	    else {
	      if (tileData->intersection.z < tile.z + borderArea) {
		mouse.tileSide = top;
	      }
	      else if (tileData->intersection.z > tile.z + bBlockD - borderArea) {
		mouse.tileSide = bot;
	      }
	      else if (tileData->intersection.z > (tile.z + bBlockD / 2) - borderArea && tileData->intersection.z < (tile.z + bBlockD / 2) + borderArea && tileData->intersection.x >(tile.x + bBlockW / 2) - borderArea && tileData->intersection.x < (tile.x +bBlockW/2) + borderArea){
		mouse.tileSide = center;
	      }else{
		mouse.tileSide = -1;
	      }
	    }

	    const float selectionW = borderArea * 3;

	    //	  glBegin(GL_TRIANGLES);

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

		printf("%d %d \n", mouse.brushType, mouse.tileSide);

	    if(mouse.brushType == mouseBlockBrushT && mouse.tileSide == center){
	      TileBlock* block = (TileBlock*) mouse.brushThing;

	      glBindVertexArray(block->vpair.VAO);
	      glBindBuffer(GL_ARRAY_BUFFER, block->vpair.VBO);

	      glActiveTexture(loadedTextures1D[0].tx);
		  glBindTexture(GL_TEXTURE_2D, loadedTextures1D[0].tx);

	      Matrix out = IDENTITY_MATRIX;

	      // translate without mult
	      out.m[12] = tile.x;
	      out.m[13] = tile.y;
	      out.m[14] = tile.z;

	      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out.m);

	      glBufferData(GL_ARRAY_BUFFER, block->vertexesSize * sizeof(float) * 5, block->vertexes, GL_STATIC_DRAW);

	      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
	      glEnableVertexAttribArray(0);

	      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	      glEnableVertexAttribArray(1);

	      glDrawArrays(GL_TRIANGLES, 0, 6);

	      glBindBuffer(GL_ARRAY_BUFFER, 0);
	      glBindVertexArray(0);
	    }
	    else if(mouse.brushType == mouseWallBrushT && mouse.tileSide != -1 && mouse.tileSide != center){
	      TileMouseData* tileData = (TileMouseData*)mouse.selectedThing;

	      Side oppositeSide = 0;
	      vec2i oppositeTile = {0};
	  
	      glBindVertexArray(wallMeshes[mouse.tileSide][wallT].VAO);
	      glBindBuffer(GL_ARRAY_BUFFER, wallMeshes[mouse.tileSide][wallT].VBO);
		
	      glActiveTexture(loadedTextures1D[0].tx);
		  glBindTexture(GL_TEXTURE_2D, loadedTextures1D[0].tx);
	      		
	      Matrix out = IDENTITY_MATRIX;

	      // translate without mult
	      out.m[12] = tile.x;
	      out.m[13] = tile.y;
	      out.m[14] = tile.z;
		
	      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out.m);

	      glDrawArrays(GL_TRIANGLES, 0, wallMeshes[mouse.tileSide][wallT].VBOsize);

	      if(oppositeTileTo(tileData->grid, mouse.tileSide,&oppositeTile,&oppositeSide)){
		Matrix out = IDENTITY_MATRIX;

		// translate without mult
		vec3 oppositePos = xyz_indexesToCoords(oppositeTile.x, floor, oppositeTile.z); 
	    
		out.m[12] = oppositePos.x;
		out.m[13] = oppositePos.y;
		out.m[14] = oppositePos.z;

	    
		glBindVertexArray(wallMeshes[oppositeSide][wallT].VAO);
		glBindBuffer(GL_ARRAY_BUFFER, wallMeshes[oppositeSide][wallT].VBO);
		
	    
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out.m);

	    
		glDrawArrays(GL_TRIANGLES, 0, wallMeshes[oppositeSide][wallT].VBOsize);
	      }

	      glBindTexture(GL_TEXTURE_2D, 0);
	      glBindBuffer(GL_ARRAY_BUFFER, 0);
	      glBindVertexArray(0);
	      
	      if(mouse.clickR){
		TileMouseData* tileData = (TileMouseData*)mouse.selectedThing;

		if(tileData->tile->customWalls[mouse.tileSide].buf == NULL) {
		  bool oppositeExtend[4] = { 0 };

		  bool itHasWall = false;
		  Tile* tile = tileData->tile;
	    
		  Side oppositeSide;
		  vec2i oppositeTile;

		  Side selectedSide = mouse.tileSide;
		  vec2i curTile = tileData->grid;

		  bool oppositeExists = oppositeTileTo(tileData->grid, mouse.tileSide, &oppositeTile, &oppositeSide);
	    
		  for(int i=0;i<2;i++){	      
		    if(selectedSide == top || selectedSide == bot){	      
		      if(tile->customWalls[right].buf){
			oppositeExtend[right] = true;
			itHasWall = true;
		      }
	      
		      if(tile->customWalls[left].buf){
			oppositeExtend[left]  = true;
			itHasWall = true;
		      }
		    }else if (selectedSide == left || selectedSide == right){
		      if(tile->customWalls[top].buf){
			oppositeExtend[top] = true;
			itHasWall = true;
		      }
	      		  
		      if(tile->customWalls[bot].buf) {
			oppositeExtend[bot] = true;
			itHasWall = true;
		      }	
		    }

		    if(itHasWall){
		      break;
		    }else{
		      if(i == 0 && oppositeExists){
			tile = &grid[floor][oppositeTile.z][oppositeTile.x];

			vec2i tempTile = oppositeTile;
			oppositeTile = tileData->grid;
			curTile = tempTile;

			Side tempSide = oppositeSide;
			oppositeSide = selectedSide;
			selectedSide = tempSide;
		      };
		    }
		  }

		  Side tempOppositeSide = 0;
		  vec2i tempOppositeTile = {0};
		  
		  setIn(grid[floor][curTile.z][curTile.x].wallsTx, selectedSide, 0); // first texture

		  grid[floor][(int)curTile.z][(int)curTile.x].customWalls[selectedSide].buf = malloc(sizeof(float) * 6 * 5);
		  grid[floor][(int)curTile.z][(int)curTile.x].customWalls[selectedSide].bufSize = 6 * 5;
		  memcpy(grid[floor][(int)curTile.z][(int)curTile.x].customWalls[selectedSide].buf, customWallTemp[selectedSide], sizeof(float) * 6 * 5);
		  
		  if(oppositeExists){
		    grid[floor][(int)oppositeTile.z][(int)oppositeTile.x].customWalls[oppositeSide].buf = malloc(sizeof(float) * 6 * 5);
		    grid[floor][(int)oppositeTile.z][(int)oppositeTile.x].customWalls[oppositeSide].bufSize = 6 * 5;
		    memcpy(grid[floor][(int)oppositeTile.z][(int)oppositeTile.x].customWalls[oppositeSide].buf, customWallTemp[oppositeSide], sizeof(float) * 6 * 5);

		    setIn(grid[floor][(int)oppositeTile.z][(int)oppositeTile.x].wallsTx, mouse.tileSide, 0); 
		  }

		  for(int i=0;i<basicSideCounter && oppositeExists;i++){
		    if(!oppositeExtend[i]) continue;
		  
		    bool itLeftRight = (oppositeSide == left || oppositeSide == right);
		    bool indexSecond = i == bot || i == left;

		    grid[floor][(int)oppositeTile.z][(int)oppositeTile.x].customWalls[oppositeSide].buf[values[itLeftRight][indexSecond][0]] += wallD * values[itLeftRight][indexSecond][3];
		    grid[floor][(int)oppositeTile.z][(int)oppositeTile.x].customWalls[oppositeSide].buf[values[itLeftRight][indexSecond][1]] += wallD * values[itLeftRight][indexSecond][3];
		    grid[floor][(int)oppositeTile.z][(int)oppositeTile.x].customWalls[oppositeSide].buf[values[itLeftRight][indexSecond][2]] += wallD * values[itLeftRight][indexSecond][3];

		    if(oppositeTileTo(curTile, i, &tempOppositeTile,&tempOppositeSide)){
		      grid[floor][(int)tempOppositeTile.z][(int)tempOppositeTile.x].customWalls[tempOppositeSide].buf[valuesOpposite[oppositeSide][0]] += wallD * valuesOpposite[oppositeSide][3];
		      grid[floor][(int)tempOppositeTile.z][(int)tempOppositeTile.x].customWalls[tempOppositeSide].buf[valuesOpposite[oppositeSide][1]] += wallD * valuesOpposite[oppositeSide][3];
		      grid[floor][(int)tempOppositeTile.z][(int)tempOppositeTile.x].customWalls[tempOppositeSide].buf[valuesOpposite[oppositeSide][2]] += wallD * valuesOpposite[oppositeSide][3];
		    }
	      
		  }
	    
		}
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

	    GroundType type = valueIn(tileData->tile->ground, 0);

	    //	printf("%d \n", type);
	
	    //	if(type != 0){
	    vec3 tile = xyz_indexesToCoords(tileData->grid.x, floor, tileData->grid.z);

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
	vec3 centrPos = { player.pos.x, player.pos.y + (floor * bBlockH), player.pos.z };

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
	if(!curMenu){
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
	if(mouse.brushThing && !curMenu){
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

		setIn(wallData->tile->wallsTx, wallData->side, texture->index1D);
	      }else if(mouse.selectedType == mouseTileT){
		TileMouseData* tileData = (TileMouseData*)mouse.selectedThing;
	    
		setIn(tileData->tile->ground, 0, texturedTile);
		setIn(tileData->tile->ground, tileData->groundInter, texture->index1D);
	      }else if(mouse.selectedType == mouseBlockT){
		TileBlock* block = (TileBlock*)mouse.selectedThing;

		block->txIndex = texture->index1D;
	      }else if(mouse.selectedType == mousePlaneT){
		Plane* plane = (Plane*)mouse.selectedThing;

		plane->txIndex = texture->index1D;
	      }
	      }
	  
	      break;
	    }case(mouseBlockBrushT):{
	       if (mouse.clickR && mouse.selectedType == mouseTileT) {
		 TileMouseData* tileData = (TileMouseData*) mouse.selectedThing;
		 TileBlock* block = (TileBlock*) mouse.brushThing;

		 tileData->tile->block = block;

		 mouse.brushThing = constructNewBlock(block->type);
	       }
	       
	       break;
	    }
	    default: break;
	    }
	}

	// render selected or focused thing
	if(mouse.focusedThing && !curMenu){
	  char buf[164];
      
	  switch(mouse.selectedType){
	  case(mouseModelT):{
	    Model* data = (Model*)mouse.selectedThing;
	  
	    sprintf(buf, "Focused model: %s [F] to focus", loadedModels1D[data->name].name);
	
	    break;
	  }case(mouseBlockT):{
	     TileBlock* data = (TileBlock*)mouse.selectedThing;
	  
	     sprintf(buf, "Focused block: %s [F] to focus", tileBlocksStr[data->type]);
	
	     break;
	   }case(mousePlaneT):{
	      Plane* data = (Plane*)mouse.selectedThing;
	  
	      sprintf(buf, "Focused plane id: %d [F] to focus", data->id);
	
	      break;
	    }
	  default: break;
	  }

	  renderText(buf, -1.0f, 1.0f - baseYPad, 1.0f);
	}else if(mouse.selectedThing && !curMenu){
	  char buf[164];

	  switch(mouse.selectedType){
	  case(mouseModelT):{
	    Model* data = (Model*)mouse.selectedThing;
	  
	    sprintf(buf, "Selected model: %s", loadedModels1D[data->name].name);
	
	    break;
	  }case(mouseWallT):{
	     WallMouseData* data = (WallMouseData*)mouse.selectedThing;
	  
	     sprintf(buf, "Selected wall: [%s] with tx: [%s]", sidesToStr[data->side], loadedTexturesNames[data->txIndex]);
	
	     break;
	   }case(mouseBlockT):{
	      TileBlock* data = (TileBlock*)mouse.selectedThing;
	  
	      sprintf(buf, "Selected block: [%s]", tileBlocksStr[data->type]);
	
	      break;
	    }case(mousePlaneT):{
	       Plane* data = (Plane*)mouse.selectedThing;
	  
	       sprintf(buf, "Selected plane: [ID: %d] tx: [%s]", data->id, loadedTexturesNames[data->txIndex]);
	
	       break;
	     }case(mouseTileT):{
		TileMouseData* data = (TileMouseData*)mouse.selectedThing;
	  
		sprintf(buf, "Selected tile tx: [%s]", loadedTexturesNames[valueIn(data->tile->ground, data->groundInter)]);
	
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
		vec3 tile = xyz_indexesToCoords(tileData->grid.x, floor, tileData->grid.z);
	    
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

	      mouse.brushThing = constructNewBlock(selectedIndex-1);
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
	    planeOnCreation = calloc(1, sizeof(Plane));

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
		  createdPlanes = malloc(sizeof(Plane));
		}else{
		  createdPlanes = realloc(createdPlanes, (createdPlanesSize+1) * sizeof(Plane));
		}

		memcpy(&createdPlanes[createdPlanesSize],planeOnCreation ,sizeof(Plane));

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
	    Plane* plane = (Plane*)mouse.focusedThing;
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
	    Plane* plane = (Plane*)mouse.focusedThing;
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
		  //	      printf("Alloc %s \n", *selectedTextInput->buf);

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
			    Plane* plane = (Plane*)mouse.focusedThing;
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
			  //		      printf("Alloc\n");
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
	if(!curMenu){
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

	  renderText(contextBelowText, -1.0f, -1.0f + letterH, 1.0f); 
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

    void renderCube(vec3 pos, float w, float h, float d, float r, float g, float b){
      glActiveTexture(solidColorTx);
      glBindTexture(GL_TEXTURE_2D, solidColorTx);
      setSolidColorTx(r, g, b, 1.0f);
  
      glBegin(GL_LINES);

      glVertex3f(pos.x, pos.y, pos.z);
      glVertex3f(pos.x, pos.y + h, pos.z);

      glVertex3f(pos.x ,pos.y, pos.z);
      glVertex3f(pos.x + w,pos.y, pos.z);

      glVertex3f(pos.x ,pos.y, pos.z);
      glVertex3f(pos.x ,pos.y, pos.z+d);

      glVertex3f(pos.x ,pos.y+ h, pos.z);
      glVertex3f(pos.x + w,pos.y+ h, pos.z);

      glVertex3f(pos.x ,pos.y+h, pos.z);
      glVertex3f(pos.x ,pos.y+h, pos.z+d);

      glVertex3f(pos.x + w,pos.y+h, pos.z);
      glVertex3f(pos.x + w,pos.y, pos.z);

      glVertex3f(pos.x ,pos.y+h, pos.z+d);
      glVertex3f(pos.x ,pos.y, pos.z+d);

      glVertex3f(pos.x ,pos.y+h, pos.z+d);
      glVertex3f(pos.x + w,pos.y+h, pos.z+d);

      glVertex3f(pos.x + w,pos.y+h, pos.z);
      glVertex3f(pos.x + w,pos.y+h, pos.z+d);

      glVertex3f(pos.x + w,pos.y+h, pos.z+d);
      glVertex3f(pos.x + w,pos.y, pos.z+d);
  
      glVertex3f(pos.x + w,pos.y+h, pos.z);
      glVertex3f(pos.x + w,pos.y+h, pos.z+d);

      glVertex3f(pos.x ,pos.y, pos.z+d);
      glVertex3f(pos.x + w,pos.y, pos.z+d);
  
      glVertex3f(pos.x + w,pos.y, pos.z);
      glVertex3f(pos.x + w,pos.y, pos.z+d);  

      glBindTexture(GL_TEXTURE_2D, 0);
  
      glEnd();
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

    // records VAO and VBO into wallMeshes
    void wallsLoadVAOandVBO(){
      for(int side=0;side< basicSideCounter; side++){
	float borderPosZ = 0.0f;
	float borderPosX = 0.0f;

	float bordersPad = selBorderD / 5.0f;
    
	switch(side){
	case(bot):{
	  borderPosZ -= bordersPad;
	  borderPosZ -= bordersPad;
	  borderPosZ -= bordersPad;
	  borderPosZ -= bordersPad;
	  break;
	}
	case(top):{
	  borderPosZ += bordersPad;
	  borderPosZ += bordersPad;
	  borderPosZ += bordersPad;
	  borderPosZ += bordersPad;
	  break;
	}
	case(left):{
	  borderPosX += bordersPad;
	  borderPosX += bordersPad;
	  borderPosX += bordersPad;
	  borderPosX += bordersPad;

	  break;
	}
	case(right):{
	  borderPosX -= bordersPad;
	  borderPosX -= bordersPad;
	  borderPosX -= bordersPad;
	  borderPosX -= bordersPad;
	  break;
	}
	default: break;
	}

      
	for(int type=1; type <wallTypeCounter; type++){
	  vec3* wallPos = wallPosBySide(side, wallsSizes[type].h, wallD, bBlockD, bBlockW);

	  // load VBO and VAO for highlighting
	  if(type == halfWallT || type == wallT){
	    if(type == halfWallT){
	      glGenVertexArrays(1, &halfWallHighlight[side].VAO);
	      glBindVertexArray(halfWallHighlight[side].VAO);

	      glGenBuffers(1, &halfWallHighlight[side].VBO);
	      glBindBuffer(GL_ARRAY_BUFFER, halfWallHighlight[side].VBO);
	    }else{
	      glGenVertexArrays(1, &wallHighlight[side].VAO);
	      glBindVertexArray(wallHighlight[side].VAO);

	      glGenBuffers(1, &wallHighlight[side].VBO);
	      glBindBuffer(GL_ARRAY_BUFFER, wallHighlight[side].VBO);
	    }

	    if(side == top || side == bot){
	      float wallBorder[] = {
		// top
		wallPos[0].x + borderPosX, wallPos[0].y, wallPos[0].z + borderPosZ,
		wallPos[1].x + borderPosX, wallPos[1].y, wallPos[1].z + borderPosZ,
		wallPos[0].x + borderPosX, wallPos[0].y - selBorderT, wallPos[0].z + borderPosZ,

		wallPos[1].x + borderPosX, wallPos[1].y, wallPos[1].z + borderPosZ,
		wallPos[0].x + borderPosX, wallPos[0].y - selBorderT, wallPos[0].z + borderPosZ,
		wallPos[1].x + borderPosX, wallPos[1].y - selBorderT, wallPos[1].z + borderPosZ,
  
		//bot
		wallPos[3].x + borderPosX, wallPos[3].y + selBorderT, wallPos[3].z + borderPosZ,
		wallPos[2].x + borderPosX, wallPos[2].y + selBorderT, wallPos[2].z + borderPosZ,
		wallPos[3].x + borderPosX, wallPos[3].y, wallPos[3].z + borderPosZ,

		wallPos[2].x + borderPosX, wallPos[3].y + selBorderT, wallPos[2].z + borderPosZ,
		wallPos[2].x + borderPosX, wallPos[2].y, wallPos[2].z + borderPosZ,
		wallPos[3].x + borderPosX, wallPos[3].y, wallPos[3].z + borderPosZ,
    
		// left
		wallPos[0].x + borderPosX,wallPos[0].y - selBorderT,wallPos[0].z + borderPosZ,
		wallPos[0].x + borderPosX + selBorderT,wallPos[0].y - selBorderT,wallPos[0].z + borderPosZ,
		wallPos[3].x + borderPosX,wallPos[3].y + selBorderT,wallPos[3].z + borderPosZ,

		wallPos[0].x + borderPosX + selBorderT,wallPos[0].y - selBorderT,wallPos[0].z + borderPosZ,
		wallPos[3].x + borderPosX,wallPos[3].y + selBorderT,wallPos[3].z + borderPosZ,
		wallPos[3].x + borderPosX + selBorderT,wallPos[3].y + selBorderT,wallPos[3].z + borderPosZ,

		// right
		wallPos[1].x + borderPosX - selBorderT,wallPos[1].y - selBorderT,wallPos[1].z + borderPosZ,
		wallPos[1].x + borderPosX,wallPos[1].y - selBorderT,wallPos[1].z + borderPosZ,
		wallPos[2].x + borderPosX - selBorderT,wallPos[2].y + selBorderT,wallPos[2].z + borderPosZ,

		wallPos[1].x + borderPosX,wallPos[1].y - selBorderT,wallPos[1].z + borderPosZ,
		wallPos[2].x + borderPosX,wallPos[2].y + selBorderT,wallPos[2].z + borderPosZ,
		wallPos[2].x + borderPosX - selBorderT,wallPos[2].y + selBorderT,wallPos[2].z + borderPosZ,
	      };

	      glBufferData(GL_ARRAY_BUFFER, sizeof(wallBorder), wallBorder, GL_STATIC_DRAW);

	      if(type == halfWallT){
		halfWallHighlight[side].VBOsize = 6 * 4;
	      }else{
		wallHighlight[side].VBOsize = 6 * 4;
	      }
	    }else if(side == left || side == right){
	      float wallBorder[] = {
		// top
		wallPos[0].x + borderPosX, wallPos[0].y, wallPos[0].z + borderPosZ,
		wallPos[1].x + borderPosX, wallPos[1].y, wallPos[1].z + borderPosZ,
		wallPos[0].x + borderPosX, wallPos[0].y - selBorderT, wallPos[0].z + borderPosZ,

		wallPos[1].x + borderPosX, wallPos[1].y, wallPos[1].z + borderPosZ,
		wallPos[0].x + borderPosX, wallPos[0].y - selBorderT, wallPos[0].z + borderPosZ,
		wallPos[1].x + borderPosX, wallPos[1].y - selBorderT, wallPos[1].z + borderPosZ,
  
		//bot
		wallPos[3].x + borderPosX, wallPos[3].y + selBorderT, wallPos[3].z + borderPosZ,
		wallPos[2].x + borderPosX, wallPos[2].y + selBorderT, wallPos[2].z + borderPosZ,
		wallPos[3].x + borderPosX, wallPos[3].y, wallPos[3].z + borderPosZ,

		wallPos[2].x + borderPosX, wallPos[3].y + selBorderT, wallPos[2].z + borderPosZ,
		wallPos[2].x + borderPosX, wallPos[2].y, wallPos[2].z + borderPosZ,
		wallPos[3].x + borderPosX, wallPos[3].y, wallPos[3].z + borderPosZ,
	    
		// left
		wallPos[0].x + borderPosX,wallPos[0].y - selBorderT,wallPos[0].z + borderPosZ,
		wallPos[0].x + borderPosX,wallPos[0].y - selBorderT,wallPos[0].z + borderPosZ - selBorderT,
		wallPos[3].x + borderPosX,wallPos[3].y + selBorderT,wallPos[3].z + borderPosZ,

		wallPos[0].x + borderPosX,wallPos[0].y - selBorderT,wallPos[0].z + borderPosZ - selBorderT,
		wallPos[3].x + borderPosX,wallPos[3].y + selBorderT,wallPos[3].z + borderPosZ,
		wallPos[3].x + borderPosX,wallPos[3].y + selBorderT,wallPos[3].z + borderPosZ - selBorderT,

		// right
		wallPos[1].x + borderPosX,wallPos[1].y - selBorderT,wallPos[1].z + borderPosZ + selBorderT,
		wallPos[1].x + borderPosX,wallPos[1].y - selBorderT,wallPos[1].z + borderPosZ,
		wallPos[2].x + borderPosX,wallPos[2].y + selBorderT,wallPos[2].z + borderPosZ + selBorderT,

		wallPos[1].x + borderPosX,wallPos[1].y - selBorderT,wallPos[1].z + borderPosZ,
		wallPos[2].x + borderPosX,wallPos[2].y + selBorderT,wallPos[2].z + borderPosZ,
		wallPos[2].x + borderPosX,wallPos[2].y + selBorderT,wallPos[2].z + borderPosZ + selBorderT,
	      };
	      glBufferData(GL_ARRAY_BUFFER, sizeof(wallBorder), wallBorder, GL_STATIC_DRAW);
	  
	      if(type == halfWallT){
		halfWallHighlight[side].VBOsize = 6 * 4;
	      }else{
		wallHighlight[side].VBOsize = 6 * 4;
	      }
	
	      wallMeshes[side][type-1].VBOsize = 6 * 4;
	    }

	    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	    glEnableVertexAttribArray(0);

	    glBindBuffer(GL_ARRAY_BUFFER, 0);
	    glBindVertexArray(0);
	  }
	
	  glGenVertexArrays(1, &wallMeshes[side][type-1].VAO);
	  glBindVertexArray(wallMeshes[side][type-1].VAO);

	  glGenBuffers(1, &wallMeshes[side][type-1].VBO);
	  glBindBuffer(GL_ARRAY_BUFFER, wallMeshes[side][type-1].VBO);

	  if(type == wallT){
	    float verts[] = {
	      argVec3(wallPos[0]), 0.0f, 1.0f,
	      argVec3(wallPos[1]), 1.0f, 1.0f,
	      argVec3(wallPos[3]), 0.0f, 0.0f, 
      
	      argVec3(wallPos[1]), 1.0f, 1.0f,
	      argVec3(wallPos[2]), 1.0f, 0.0f,
	      argVec3(wallPos[3]), 0.0f, 0.0f, 
	    };

	    if(!customWallTemp[side]){
	      customWallTemp[side] = malloc(sizeof(verts));
	      memcpy(customWallTemp[side], &verts, sizeof(verts));
	      printf("Vert %d \n", sizeof(verts));
	    }

	    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	  
	    wallMeshes[side][type-1].VBOsize = 6;
	  }
	  /*
	    else if (type == halfWallFenceT) {
	    float fencePlankW = bBlockW / 8;
	
	    float verts[] = {
	    argVec3(wallPos[0]), 0.0f, .4f,
	    wallPos[0].x + fencePlankW, wallPos[0].y, wallPos[0].z, 1.0f, .4f,
	    wallPos[0].x, 0.0f, wallPos[0].z, 0.0f, 0.0f, 

	    wallPos[0].z + fencePlankW * 2, 0.0f, .4f,
	    wallPos[0].z + fencePlankW, wallPos[0].y, wallPos[0].z + fencePlankW, 1.0f, .4f,
	    wallPos[0].z, 0.0f, wallPos[0].z, 0.0f, 0.0f, 
      
	  
	    argVec3(wallPos[1]), 1.0f, .4f,
	    argVec3(wallPos[2]), 1.0f, 0.0f,
	    argVec3(wallPos[3]), 0.0f, 0.0f, 
	    };
	  
	    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	  
	    wallMeshes[side][type-1].VBOsize = 6;
	    }
	  */
	  else if(type == halfWallT){
	    float verts[] = {
	      argVec3(wallPos[0]), 0.0f, .4f,
	      argVec3(wallPos[1]), 1.0f, .4f,
	      argVec3(wallPos[3]), 0.0f, 0.0f, 
      
	      argVec3(wallPos[1]), 1.0f, .4f,
	      argVec3(wallPos[2]), 1.0f, 0.0f,
	      argVec3(wallPos[3]), 0.0f, 0.0f, 
	    };
	  
	    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	  
	    wallMeshes[side][type-1].VBOsize = 6;
	  }else if(type == windowT){
	    const float windowBotH = bBlockH * 0.35f;
	   
	    if(side == left || side == right){
	      float verts[] = {
		// top plank
		argVec3(wallPos[0]), 0.0f, 1.0f,
		argVec3(wallPos[1]), 1.0f, 1.0f,
		wallPos[0].x, wallPos[0].y - doorTopPad, wallPos[0].z, 0.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h), 
      
		argVec3(wallPos[1]), 1.0f, 1.0f,
		wallPos[1].x, wallPos[1].y - doorTopPad, wallPos[1].z, 1.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),
		wallPos[0].x, wallPos[0].y - doorTopPad, wallPos[0].z, 0.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),

		// left plank
		wallPos[0].x, wallPos[0].y - doorTopPad, wallPos[0].z,
		0.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),
    
		wallPos[0].x, wallPos[0].y - doorTopPad, wallPos[0].z - doorPad/2,
		0.0f + (doorPad/2 / wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),

		wallPos[3].x, wallPos[3].y + windowBotH, wallPos[3].z,
		0.0f, 0.0f + (windowBotH / wallsSizes[doorFrameT].h),

		//
		wallPos[0].x, wallPos[0].y - doorTopPad, wallPos[0].z - doorPad/2,
		0.0f + (doorPad/2 / wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),

		wallPos[3].x, wallPos[3].y + windowBotH, wallPos[3].z - doorPad/2,
		0.0f + (doorPad/2 / wallsSizes[doorFrameT].w), 0.0f + (windowBotH / wallsSizes[doorFrameT].h),

		wallPos[3].x, wallPos[3].y + windowBotH, wallPos[3].z,
		0.0f, 0.0f + (windowBotH / wallsSizes[doorFrameT].h),

		// right
		wallPos[1].x, wallPos[1].y - doorTopPad, wallPos[1].z + doorPad/2,
		1.0f - (doorPad/2 / wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),

		wallPos[1].x, wallPos[1].y - doorTopPad, wallPos[1].z,
		1.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),
      
		wallPos[2].x, wallPos[2].y + windowBotH, wallPos[2].z + doorPad/2,
		1.0f - (doorPad/2 / wallsSizes[doorFrameT].w) , 0.0f + (windowBotH / wallsSizes[doorFrameT].h),

		//
		wallPos[1].x, wallPos[1].y - doorTopPad, wallPos[1].z,
		1.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),
      
		wallPos[2].x, wallPos[2].y + windowBotH, wallPos[2].z,
		1.0f, 0.0f + (windowBotH / wallsSizes[doorFrameT].h),
    
		wallPos[2].x, wallPos[2].y + windowBotH, wallPos[2].z + doorPad/2,
		1.0f - (doorPad/2 / wallsSizes[doorFrameT].w) , 0.0f + (windowBotH / wallsSizes[doorFrameT].h),
      
		// bot
		wallPos[3].x, wallPos[3].y + windowBotH, wallPos[3].z,
		0.0f, 0.0f + (windowBotH / wallsSizes[doorFrameT].h),

		wallPos[2].x, wallPos[2].y + windowBotH, wallPos[2].z,
		1.0f, 0.0f + (windowBotH / wallsSizes[doorFrameT].h),

		wallPos[3].x, wallPos[3].y, wallPos[3].z,
		0.0f, 0.0f,
  
		wallPos[2].x, wallPos[2].y + windowBotH, wallPos[2].z,
		1.0f, 0.0f + (windowBotH / wallsSizes[doorFrameT].h),
  
		wallPos[2].x, wallPos[2].y, wallPos[2].z,1.0f, 0.0f,
  
		wallPos[3].x, wallPos[3].y, wallPos[3].z,0.0f, 0.0f,
	      };
	  
	      glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	    }else{
	      float verts[] = {
		// top plank
		argVec3(wallPos[0]), 0.0f, 1.0f,
		argVec3(wallPos[1]), 1.0f, 1.0f,
		wallPos[0].x, wallPos[0].y - doorTopPad, wallPos[0].z, 0.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h), 
      
		argVec3(wallPos[1]), 1.0f, 1.0f,
		wallPos[1].x, wallPos[1].y - doorTopPad, wallPos[1].z, 1.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),
		wallPos[0].x, wallPos[0].y - doorTopPad, wallPos[0].z, 0.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),

		// left plank
		wallPos[0].x, wallPos[0].y - doorTopPad, wallPos[0].z, 0.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),
		wallPos[0].x +  doorPad/2, wallPos[0].y - doorTopPad, wallPos[0].z, 0.0f + (doorPad/2 / wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),
		wallPos[3].x, wallPos[3].y + windowBotH, wallPos[3].z, 0.0f, 0.0f + (windowBotH / wallsSizes[doorFrameT].h),

		wallPos[0].x + doorPad/2, wallPos[0].y - doorTopPad, wallPos[0].z,
		0.0f + (doorPad/2 / wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),
		wallPos[3].x + doorPad/2, wallPos[3].y + windowBotH, wallPos[3].z,
		0.0f + (doorPad/2 / wallsSizes[doorFrameT].w), 0.0f + (windowBotH / wallsSizes[doorFrameT].h),
		wallPos[3].x, wallPos[3].y + windowBotH, wallPos[3].z,
		0.0f, 0.0f + (windowBotH / wallsSizes[doorFrameT].h),
      
		// right
		wallPos[1].x - doorPad/2, wallPos[1].y - doorTopPad, wallPos[0].z,
		1.0f - (doorPad/2 / wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),
		wallPos[1].x, wallPos[1].y - doorTopPad, wallPos[0].z,
		1.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),
		wallPos[2].x - doorPad/2, wallPos[2].y + windowBotH, wallPos[2].z,
		1.0f - (doorPad/2 / wallsSizes[doorFrameT].w) , 0.0f + (windowBotH / wallsSizes[doorFrameT].h),

		//
		wallPos[1].x, wallPos[1].y - doorTopPad, wallPos[0].z,
		1.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),
      
		wallPos[2].x, wallPos[2].y + windowBotH, wallPos[2].z,
		1.0f, 0.0f + (windowBotH / wallsSizes[doorFrameT].h),
    
		wallPos[2].x - doorPad/2, wallPos[2].y + windowBotH, wallPos[2].z,
		1.0f - (doorPad/2 / wallsSizes[doorFrameT].w) , 0.0f + (windowBotH / wallsSizes[doorFrameT].h),

		// bot
		wallPos[3].x, wallPos[3].y + windowBotH, wallPos[3].z,
		0.0f, 0.0f + (windowBotH / wallsSizes[doorFrameT].h),

		wallPos[2].x, wallPos[2].y + windowBotH, wallPos[2].z,
		1.0f, 0.0f + (windowBotH / wallsSizes[doorFrameT].h),

		wallPos[3].x, wallPos[3].y, wallPos[3].z,
		0.0f, 0.0f,
  
		wallPos[2].x, wallPos[2].y + windowBotH, wallPos[2].z,
		1.0f, 0.0f + (windowBotH / wallsSizes[doorFrameT].h),
  
		wallPos[2].x, wallPos[2].y, wallPos[2].z,1.0f, 0.0f,
  
		wallPos[3].x, wallPos[3].y, wallPos[3].z,0.0f, 0.0f,
	      };


	      glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	    }

	  
	    wallMeshes[side][type-1].VBOsize = 6 * 4;
	    //printf("Window size: %d \n", sizeof(verts) / sizeof(float));
	  }else if(type == doorFrameT){
	    if(side == left || side == right){
	      float verts[] = {
		// top
		argVec3(wallPos[0]), 0.0f, 1.0f,
	  
		argVec3(wallPos[1]), 1.0f, 1.0f,
	  
		wallPos[0].x, wallPos[0].y - doorTopPad, wallPos[0].z, 0.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),

		argVec3(wallPos[1]), 1.0f, 1.0f,
	  
		wallPos[1].x, wallPos[1].y - doorTopPad, wallPos[1].z,1.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),
	  
		wallPos[0].x, wallPos[0].y - doorTopPad, wallPos[0].z,0.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),

		// left
		wallPos[0].x, wallPos[0].y - doorTopPad, wallPos[0].z, 0.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),
	    
		wallPos[0].x, wallPos[0].y - doorTopPad, wallPos[0].z - doorPad/2, 0.0f + ((doorPad/2)/ wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),
	    
		wallPos[3].x, wallPos[3].y, wallPos[3].z, 0.0f, 0.0f,
	    

		wallPos[0].x, wallPos[0].y - doorTopPad, wallPos[0].z - doorPad/2, 0.0f + ((doorPad/2)/ wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),

		wallPos[3].x, wallPos[3].y, wallPos[3].z - doorPad/2, 0.0f + ((doorPad/2)/ wallsSizes[doorFrameT].w), 0.0f - (doorTopPad / wallsSizes[doorFrameT].h),
	    
		wallPos[3].x, wallPos[3].y, wallPos[3].z, 0.0f, 0.0f,
  
		// right
		wallPos[1].x, wallPos[1].y - doorTopPad, wallPos[1].z + doorPad/2, 1.0f - ((doorPad/2)/ wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),

		wallPos[1].x, wallPos[1].y - doorTopPad, wallPos[1].z, 1.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),

		wallPos[2].x, wallPos[2].y, wallPos[2].z, 1.0f, 0.0f,

		wallPos[1].x, wallPos[1].y - doorTopPad, wallPos[1].z + doorPad/2, 1.0f - ((doorPad/2)/ wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),
	    
		wallPos[2].x, wallPos[2].y, wallPos[2].z, 1.0f, 0.0f,

		wallPos[2].x, wallPos[2].y, wallPos[2].z + doorPad/2, 1.0f - ((doorPad/2)/ wallsSizes[doorFrameT].w), 0.0f,
	      };

	  
	      glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	    }else{
	      float verts[] = {
		// side
		argVec3(wallPos[0]), 0.0f, 1.0f,
	  
		argVec3(wallPos[1]), 1.0f, 1.0f,
	  
		wallPos[0].x, wallPos[0].y - doorTopPad, wallPos[0].z, 0.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),

		argVec3(wallPos[1]), 1.0f, 1.0f,
	  
		wallPos[1].x, wallPos[1].y - doorTopPad, wallPos[1].z,1.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),
	  
		wallPos[0].x, wallPos[0].y - doorTopPad, wallPos[0].z,0.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),

		// left
		wallPos[0].x, wallPos[0].y - doorTopPad, wallPos[0].z, 0.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),
	    
		wallPos[0].x + doorPad/2, wallPos[0].y - doorTopPad, wallPos[0].z, 0.0f + ((doorPad/2)/ wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),
	    
		wallPos[3].x, wallPos[3].y, wallPos[3].z, 0.0f, 0.0f,
	    

		wallPos[0].x + doorPad/2, wallPos[0].y - doorTopPad, wallPos[0].z, 0.0f + ((doorPad/2)/ wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),

		wallPos[3].x + doorPad/2, wallPos[3].y, wallPos[3].z, 0.0f + ((doorPad/2)/ wallsSizes[doorFrameT].w), 0.0f - (doorTopPad / wallsSizes[doorFrameT].h),
	    
		wallPos[3].x, wallPos[3].y, wallPos[3].z, 0.0f, 0.0f,
  
		// right
		wallPos[1].x - doorPad/2, wallPos[1].y - doorTopPad, wallPos[1].z, 1.0f - ((doorPad/2)/ wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),

		wallPos[1].x, wallPos[1].y - doorTopPad, wallPos[1].z, 1.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),

		wallPos[2].x, wallPos[2].y, wallPos[2].z, 1.0f, 0.0f,

		wallPos[1].x - doorPad/2, wallPos[1].y - doorTopPad, wallPos[1].z, 1.0f - ((doorPad/2)/ wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h),
	    
		wallPos[2].x, wallPos[2].y, wallPos[2].z, 1.0f, 0.0f,

		wallPos[2].x - doorPad/2, wallPos[2].y, wallPos[2].z, 1.0f - ((doorPad/2)/ wallsSizes[doorFrameT].w), 0.0f,
	      };

	  
	      glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	    }
	  
	    wallMeshes[side][type-1].VBOsize = 6 * 3;
	  }
      
	  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
	  glEnableVertexAttribArray(0);

	  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	  glEnableVertexAttribArray(1);

	  glBindBuffer(GL_ARRAY_BUFFER, 0); 
	  glBindVertexArray(0);
    
	  free(wallPos);
	}
      }
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

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
      glBufferData(GL_ARRAY_BUFFER, symblosSize, symbols, GL_STATIC_DRAW);
      free(symbols);

      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1 , 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (2 * sizeof(float)));
      glEnableVertexAttribArray(1);
      
      glDrawArrays(GL_TRIANGLES, 0, symblosSize / 16);
  
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
      glDisable(GL_BLEND);
  
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

      grid = malloc(sizeof(Tile**) * (gridY));

      for (int y = 0; y < gridY; y++) {
	grid[y] = malloc(sizeof(Tile*) * (gridZ));
	for (int z = 0; z < gridZ; z++) {
	  grid[y][z] = calloc(gridX, sizeof(Tile));
      
	  for (int x = 0; y==0 && x < gridX; x++) {
	    setIn(grid[y][z][x].ground, 0, texturedTile);
	    setIn(grid[y][z][x].ground, 2, textureOfGround);
	  }
	  
	}
      } 
   
      int wallsCounter = -1; 
      fscanf(map, "Walls used: %d", &wallsCounter);  
   
      for (int i = 0; i < wallsCounter; i++) {
	int x = -1,y = -1,z = -1;
    
	fscanf(map, "\n%d %d %d ",&x,&y,&z);

	int sidesCounter = 0;
      
	fscanf(map, "%d %d %d ", &grid[y][z][x].wallsTx, &grid[y][z][x].ground, &sidesCounter);

	GroundType type = valueIn(grid[y][z][x].ground, 0);
    
	if(y == 0 && (type == netTile || type == 0)){
	  setIn(grid[y][z][x].ground, 0, texturedTile);
	  setIn(grid[y][z][x].ground, 2, textureOfGround);
	}
      
	Tile* tile = &grid[y][z][x];

	Side side = -1;
	int bufSize = -1; 

	for(int i2=0;i2<sidesCounter;i2++){
	  fscanf(map, "%d %d ", &side, &bufSize);
	  tile->customWalls[side].buf = malloc(sizeof(float) * bufSize);  
	  tile->customWalls[side].bufSize = bufSize; 

	  for(int i2=0;i2<bufSize;i2++){
	    fscanf(map, "%f ", &tile->customWalls[side].buf[i2]);
	  }
	}
      
      }
  

      /*for (int y = 0; y < gridY; y++) {
	grid[y] = malloc(sizeof(Tile*) * (gridZ));

	for (int z = 0; z < gridZ; z++) {
	grid[y][z] = calloc(gridX, sizeof(Tile));

	for (int x = 0; x < gridX; x++) {
	Tile tile = grid[y][z][x];
	GroundType type = valueIn(tile.ground, 0);

	if(type == netTile || type == 0 || (!tile.customWalls[0].buf && !tile.customWalls[1].buf && !tile.customWalls[2].buf && !tile.customWalls[3].buf)){
	continue;
	}
	
	fprintf(map, "%d %d %d ",x,y,z, grid[y][z][x].wallsTx, grid[y][z][x].ground);
	
	fprintf(map, "%d %d ",x,y,z, grid[y][z][x].wallsTx, grid[y][z][x].ground);

	for(int i=0;i<basicSideCounter;i++){
	if(tile.customWalls[i].buf){
	fprintf(map, "%d %d ",i,tile.customWalls[i].bufSize);

	for(int i2=0;i2<tile.customWalls[i].bufSize;i2++){
	fprintf(map, "%f ",tile.customWalls[i].buf[i2]);
	}
	    
	}
	}

	fprintf(map, "\n");
	//fscanf(map, "[Wls: %d, WlsTx %d, Grd: %d]", &grid[y][z][x].walls, &grid[y][z][x].wallsTx, &grid[y][z][x].ground);

	GroundType type = valueIn(grid[y][z][x].ground, 0);

	if(type == netTile || type == 0){
	if(y == 0){
	setIn(grid[y][z][x].ground, 0, texturedTile);
	setIn(grid[y][z][x].ground, 2, frozenGround);
	}else{
	setIn(grid[y][z][x].ground, 0, netTile);
	}
	}

	fgetc(map); // read ,
	}
	}
	fgetc(map); // read \n
	}*/
    
      fscanf(map, "\nUsed models: %d\n", &curModelsSize);

      if (curModelsSize != 0) { 
	curModels = malloc(curModelsSize * sizeof(Model));

	for (int i = 0; i < curModelsSize; i++) {
	  /*	  if(curModels[i].name == yalinka){
	  // TODO: Make here some enum to string function
	  fprintf(map, "Yalinka[%d] ", yalinka);
	  }*/
	  int name = -1;
	  fscanf(map, "%d ", &name);

	  if (name >= loadedModelsSize || name < 0) {
	    printf("Models parsing error, model name (%d) doesnt exist \n", name);
	    exit(0);
	  }

	  //curModels[i] = malloc(sizeof(Model));
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

      for (int y = 0; y < gridY; y++) {
	for (int z = 0; z < gridZ; z++) {
	  for (int x = 0; x < gridX; x++) {
	    Tile tile = grid[y][z][x];
	    GroundType type = valueIn(tile.ground, 0);

	    if(tile.customWalls[0].buf || tile.customWalls[1].buf || tile.customWalls[2].buf || tile.customWalls[3].buf){
	      wallsIndexes[wallsCounter] = (vec3i){x,y,z};
	      wallsCounter++;
	    }
	    else if (y != 0 && type == texturedTile) {
	      wallsIndexes[wallsCounter] = (vec3i){ x,y,z };
	      wallsCounter++;
	    }

	  }
	}
      }

      fprintf(map, "Walls used: %d", wallsCounter);
  
      for (int i = 0; i < wallsCounter; i++) {
	fprintf(map, "\n");
      
	int x = wallsIndexes[i].x;
	int y = wallsIndexes[i].y; 
	int z = wallsIndexes[i].z;
    
	Tile tile = grid[y][z][x];

	int sidesAvaible = 0;
    
	if(tile.customWalls[0].buf){
	  sidesAvaible++;
	}

	if(tile.customWalls[1].buf){
	  sidesAvaible++;
	}

	if(tile.customWalls[2].buf){
	  sidesAvaible++;
	}

	if(tile.customWalls[3].buf){
	  sidesAvaible++;
	}
    
	fprintf(map, "%d %d %d %d %d %d ",x,y,z, grid[y][z][x].wallsTx, grid[y][z][x].ground, sidesAvaible);

	for(int i1=0;i1<basicSideCounter;i1++){
	  if(tile.customWalls[i1].buf){
	    fprintf(map, "%d %d ",i1,tile.customWalls[i1].bufSize);

	    for(int i2=0;i2<tile.customWalls[i1].bufSize;i2++){
	      fprintf(map, "%f ",tile.customWalls[i1].buf[i2]);
	    }
	    
	  }
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
    
      grid = malloc(sizeof(Tile**) * (newY));

      for (int y = 0; y < newY; y++) {
	grid[y] = malloc(sizeof(Tile*) * (newZ));

	for (int z = 0; z < newZ; z++) {
	  grid[y][z] = calloc(newX, sizeof(Tile));

	  for (int x = 0; x < newX; x++) {
	    if (y == 0) {
	      setIn(grid[y][z][x].ground, 0, texturedTile);
	      setIn(grid[y][z][x].ground, 2, textureOfGround);
	    }
	    else {
	      setIn(grid[y][z][x].ground, 0, netTile);
	    }
	  }
	}
      }

      gridX = newX;
      gridY = newY;
      gridZ = newZ;
  
      initSnowParticles();

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

TileBlock* constructNewBlock(int type){
  TileBlock* newBlock = calloc(1, sizeof(TileBlock));
  memcpy(newBlock, &tileBlocksTempl[type], sizeof(TileBlock));

  newBlock->vertexes = malloc(newBlock->vertexesSize * sizeof(float) * 5);

  memcpy(newBlock->vertexes, tileBlocksTempl[type].vertexes, newBlock->vertexesSize * sizeof(float) * 5);

  return newBlock;
}
