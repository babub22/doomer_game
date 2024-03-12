#include "deps.h"
#include "linearAlg.h"
#include "main.h"

GLuint textVBO;
GLuint textVAO;

char curSaveName[CONSOLE_BUF_CAP];

GLuint selectionRectVBO;
GLuint selectionRectVAO;

GLuint selectionRectVBO;
GLuint selectionRectVAO;


Object** objsStore;
size_t objsStoreSize;

GLuint fontAtlas;

Character* characters;
size_t charactersSize;


// avaible/loaded models
ModelInfo* loadedModels1D;
ModelInfo** loadedModels2D;
size_t loadedModelsSize;

TextInput* selectedTextInput;
TextInput dialogEditorNameInput;

// placed/created models
Model* curModels;
size_t curModelsSize;

Tile*** grid;
int gridX = 120;
int gridY = 15;
int gridZ = 120;

const float letterW = .04f;
const float letterH = .07f;

GLuint mappedTextures[texturesCounter + particlesCounter];

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

Menu dialogEditor;

Menu objectsMenu;
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

// accepts percents
float* uiRect(float x, float y, float w, float h){
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

  for(int i =0;i<6*4;i++){
    printf("%f ", stackRect[i]);
  }

  memcpy(rect, stackRect, sizeof(stackRect));
	
  return rect;
}

int main(int argc, char* argv[]) {
  borderArea = (float)bBlockW/8;
  
  SDL_Init(SDL_INIT_VIDEO);

  char windowTitle[100] = game;
  SDL_Window* window = SDL_CreateWindow(windowTitle,
					SDL_WINDOWPOS_CENTERED,
					SDL_WINDOWPOS_CENTERED,
					windowW, windowH,
					SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

  SDL_WarpMouseInWindow(window, windowW/2.0f, windowH/2.0f);
  SDL_SetRelativeMouseMode(SDL_TRUE);
  SDL_GLContext context = SDL_GL_CreateContext(window);
  
  SDL_ShowCursor(SDL_DISABLE);

  GLuint netTileVBO, netTileVAO;
  GLuint texturedTileVBO, texturedTileVAO;
  GLuint snowFlakeVBO, snowFlakeVAO;
  GLuint centerMouseSelVBO, centerMouseSelVAO;

  glewInit();

  // cursor buffers
  {
    glGenBuffers(1, &cursorVBO);
    glGenVertexArrays(1, &cursorVAO);
    
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
    
      float* dialogEditor = uiRect(f(1/8), f(7/8), f(7/8), f(1/8));

      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (6 * 4), dialogEditor, GL_STATIC_DRAW);
      free(dialogEditor);
      
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }

     // dialog nameInput editor
    { 
      glGenVertexArrays(1, &dialogEditorNameInput.VAO);
      glBindVertexArray(dialogEditorNameInput.VAO);

      glGenBuffers(1, &dialogEditorNameInput.VBO);
      glBindBuffer(GL_ARRAY_BUFFER, dialogEditorNameInput.VBO);
      
      float* nameInput = uiRect(f(1/8) * 0.8f, f(7/8) * 0.9f, f(7/8) * 0.2f, f(7/8) * 0.86f);

      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (6 * 4), nameInput, GL_STATIC_DRAW);
      free(nameInput);
      
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
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
    glGenVertexArrays(1, &snowFlakeVAO);
    glBindVertexArray(snowFlakeVAO);

    glGenBuffers(1, &snowFlakeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, snowFlakeVBO);

    
    float snowFlakeVerts[] = {
      0.0f, 0.0f, 0.0f, 
      0.0f, 0.015f / 4.0f, 0.0f  };

    glBufferData(GL_ARRAY_BUFFER, sizeof(snowFlakeVerts)
		 ,snowFlakeVerts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);  
    glDepthFunc(GL_LEQUAL);
  }
    
  // load textures
  {
    glGenTextures(texturesCounter, mappedTextures);

    // -1 because of solidColorTx
    for(int i=0;i<solidColorTx - 2;i++){
      char path[60];
      sprintf(path, texturesFolder"%d.png",i);
    
      SDL_Surface* texture = IMG_Load(path);

      if (!texture) {
	printf("Loading of texture \"%d.png\" failed", i);
	exit(0);
      }

      glBindTexture(GL_TEXTURE_2D, i);
      
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->w,
		   texture->h, 0, GL_RGBA,
		   GL_UNSIGNED_BYTE, texture->pixels);

      SDL_FreeSurface(texture);
      
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  }

  // load 1x1 texture to rende ricolors
  {
    glBindTexture(GL_TEXTURE_2D, solidColorTx);
      
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //    GLubyte bm[1] = {0x00};

    GLubyte color[4] = {255, 255, 255, 255};
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1,
		 1, 0, GL_RGBA,
		 GL_UNSIGNED_BYTE, color);
      
    glBindTexture(GL_TEXTURE_2D, 0);
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
    SDL_Surface* texture = IMG_Load("./iosevka-bold.png");

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
  
  mouse = (Mouse){  .h = 0.005f, .w = 0.005f, .brush = 0, .interDist = 1000.0f  };

  float dist = sqrt(1 / 3.0);
  bool cameraMode = true;

  float testFOV = editorFOV;

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
	    setIn(grid[y][z][x].ground, 2, frozenGround);
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
	if(console.open){
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
	}else{       
	  switch (event.key.keysym.scancode) {
	  case(SDL_SCANCODE_F1):{
	    console.open = true;
	  
	    break;
	  }
	  case(SDL_SCANCODE_UP): {
	    if(manipulationMode != -1){
	      switch(manipulationMode){
	      case(TRANSFORM_Z):{
		mouse.focusedModel->mat.m[13] = mouse.focusedModel->mat.m[13] + 0.001f;
		calculateModelAABB(mouse.focusedModel);
		break;
	      }
	      case(TRANSFORM_XY):{
		mouse.focusedModel->mat.m[12] = mouse.focusedModel->mat.m[12] - 0.01f;
		calculateModelAABB(mouse.focusedModel);
		break;
	      }
	      case(SCALE):{
		scale(mouse.focusedModel->mat.m, 1.05f, 1.05f, 1.05f);
		calculateModelAABB(mouse.focusedModel);
		break;
	      }

	      default: break;
	      }
	    }
	    else if (mouse.wallType != -1) {
	      Texture nextTx = 0;

	      if (mouse.wallTx != texturesCounter - 1) {
		nextTx = mouse.wallTx + 1;
	      }
	      else {
		nextTx = 0;
	      }

	      setIn(grid[mouse.wallTile.y][mouse.wallTile.z][mouse.wallTile.x].wallsTx, mouse.wallSide, nextTx);
	    }
	    else if (mouse.groundInter != -1) {
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
	    }

	    break;
	  }
	  case(SDL_SCANCODE_DOWN): {
	    // TODO: if intersected tile + wall will work only tile changer
	    if(manipulationMode != -1){
	      switch(manipulationMode){ 
	      case(TRANSFORM_Z):{
		mouse.focusedModel->mat.m[13] = mouse.focusedModel->mat.m[13] - 0.001f;   
		calculateModelAABB(mouse.focusedModel); 
		break;
	      }
	      case(TRANSFORM_XY):{
		mouse.focusedModel->mat.m[12] = mouse.focusedModel->mat.m[12] + 0.01f;    
		calculateModelAABB(mouse.focusedModel);
		break;
	      }
	      case(SCALE):{
		scale(mouse.focusedModel->mat.m, 1.0f/1.05f, 1.0f/1.05f, 1.0f/1.05f);
		calculateModelAABB(mouse.focusedModel);
		break;
	      }

	      default: break;
	      }
	    }else if (mouse.wallType != -1) {
	      Texture prevTx = 0;

	      if (mouse.wallTx != 0) {
		prevTx = mouse.wallTx - 1;
	      }
	      else {
		prevTx = texturesCounter - 1;
	      }

	      setIn(grid[mouse.wallTile.y][mouse.wallTile.z][mouse.wallTile.x].wallsTx, mouse.wallSide, prevTx);
	    }
	    else if (mouse.groundInter != -1) {
	      GroundType type = valueIn(grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, 0);

	      if (type != texturedTile) {
		setIn(grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, 0, texturedTile);
		setIn(grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, mouse.groundInter, texturesCounter - 1);
	      }
	      else {
		Texture curTx = valueIn(grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, mouse.groundInter);

		if (curTx == 0) {
		  setIn(grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, 0, netTile);
		}
		else {
		  setIn(grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, mouse.groundInter, curTx - 1);
		}
	      }
	    }

	    break;
	  }
	  case(SDL_SCANCODE_Q): {
	    curCamera->pos.y += .01f ;
	  
	    break;
	  }
	  case(SDL_SCANCODE_F): {
	    if(mouse.selectedModel && !mouse.focusedModel){
	      mouse.focusedModel = mouse.selectedModel;
	    }else if(mouse.focusedModel){
	      mouse.focusedModel = NULL;
	    };
	  
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
	    drawDistance -= .1f / 2.0f;

	    if(enviromental.fog){
	      glUniform1f(radius, drawDistance);
	    }

	    break;
	  }
	  case(SDL_SCANCODE_LEFT): {
	    if(manipulationMode != -1){
	      switch(manipulationMode){
	      case(ROTATE_Y):{
		rotateY(mouse.focusedModel->mat.m, -rad(1.0f));
		calculateModelAABB(mouse.focusedModel);
		break;
	      }
	      case(ROTATE_Z):{
		rotateZ(mouse.focusedModel->mat.m, -rad(1.0f));
		calculateModelAABB(mouse.focusedModel);
		break;
	      }
	      case(ROTATE_X):{
		rotateX(mouse.focusedModel->mat.m, -rad(1.0f));
		calculateModelAABB(mouse.focusedModel);
		break;
	      }
	      case(TRANSFORM_XY):{
		mouse.focusedModel->mat.m[14] = mouse.focusedModel->mat.m[14] + 0.01f;
		calculateModelAABB(mouse.focusedModel);
		break;
	      }
		      
	      default: break;
	      }
	    }else if (mouse.wallType != -1) {
	      WallType prevType = 0;

	      if (mouse.wallType != 1) {
		prevType = mouse.wallType - 1;
	      }
	      else {
		prevType = wallTypeCounter - 1;
	      }

	      Side oppositeSide = 0;
	      vec2i oppositeTile = { 0 };

	      setIn(grid[mouse.wallTile.y][mouse.wallTile.z][mouse.wallTile.x].walls, mouse.wallSide, prevType);

	      if (oppositeTileTo((vec2i) { mouse.wallTile.x, mouse.wallTile.z }, mouse.wallSide, & oppositeTile, & oppositeSide)) {
		setIn(grid[mouse.wallTile.y][oppositeTile.z][oppositeTile.x].walls, oppositeSide, prevType);
	      }
	    }

	    break;
	  }
	  case(SDL_SCANCODE_RIGHT): {
	    if(manipulationMode != -1){
	      switch(manipulationMode){
	      case(ROTATE_Y):{
		rotateY(mouse.focusedModel->mat.m, rad(1.0f));
		calculateModelAABB(mouse.focusedModel);
		break;
	      }
	      case(ROTATE_Z):{
		rotateZ(mouse.focusedModel->mat.m, rad(1.0f));
		calculateModelAABB(mouse.focusedModel);
		break;
	      }
	      case(ROTATE_X):{
		rotateX(mouse.focusedModel->mat.m, rad(1.0f));
		calculateModelAABB(mouse.focusedModel);
		break;
	      }
	      case(TRANSFORM_XY):{
		mouse.focusedModel->mat.m[14] = mouse.focusedModel->mat.m[14] - 0.01f;
		calculateModelAABB(mouse.focusedModel);
		break;
	      }

	      default: break;
	      }
	    }else if (mouse.wallType != -1) {
	      WallType nextType = 0;

	      if (mouse.wallType != wallTypeCounter - 1) {
		nextType = mouse.wallType + 1;
	      }
	      else {
		nextType = 1;
	      }

	      Side oppositeSide = 0;
	      vec2i oppositeTile = { 0 };

	      setIn(grid[mouse.wallTile.y][mouse.wallTile.z][mouse.wallTile.x].walls, mouse.wallSide, nextType);

	      if (oppositeTileTo((vec2i) { mouse.wallTile.x, mouse.wallTile.z }, mouse.wallSide, & oppositeTile, & oppositeSide)) {
		setIn(grid[mouse.wallTile.y][oppositeTile.z][oppositeTile.x].walls, oppositeSide, nextType);
	      }
	    }

	    break;
	  }


	  case(SDL_SCANCODE_SPACE): {
	    cameraMode = !cameraMode;

	    curCamera = cameraMode ? &camera1 : &camera2;

	    break;
	  }
	  case(SDL_SCANCODE_O): {
	    objectsMenu.open = !objectsMenu.open; 
	    break;
	  }
	  case(SDL_SCANCODE_B): {
	    if(mouse.focusedModel && loadedModels1D[mouse.focusedModel->name].type == characterModelType){
	      dialogEditor.open = !dialogEditor.open; 
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
	  }
	  case(SDL_SCANCODE_F5): {
	    saveMap(curSaveName);
	    break;
	  }
	  case(SDL_SCANCODE_H): {
	    highlighting = !highlighting;
	    break;
	  }
	  case(SDL_SCANCODE_DELETE): {
	    if(mouse.selectedModel){
	      int index = 0;

	      for(int i=0;i<curModelsSize;i++){ 
		if(curModels[i].id == mouse.selectedModel->id){
		  continue;
		}

		curModels[index] = curModels[i];
		index++;
	      }

	      curModelsSize--;
	      curModels = realloc(curModels, curModelsSize * sizeof(Model));
	    
	    }else if (mouse.wallSide != -1) {
	      WallType type = (grid[mouse.wallTile.y][mouse.wallTile.z][mouse.wallTile.x].walls >> (mouse.wallSide * 8)) & 0xFF;

	      Side oppositeSide = 0;
	      vec2i oppositeTile = { 0 };
	      grid[mouse.wallTile.y][mouse.wallTile.z][mouse.wallTile.x].walls &= ~(0xFF << (mouse.wallSide * 8));

	      if (oppositeTileTo((vec2i) { mouse.wallTile.x, mouse.wallTile.z }, mouse.wallSide, & oppositeTile, & oppositeSide)) {
		grid[mouse.wallTile.y][oppositeTile.z][oppositeTile.x].walls &= ~(0xFF << (oppositeSide * 8));
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

      mouse.wheel = event.wheel.y;
      if (event.type == SDL_MOUSEMOTION) {

	if(objectsMenu.open || dialogEditor.open){
	  float x = -1.0 + 2.0 * (event.motion.x / windowW);
	  float y = -(-1.0 + 2.0 * (event.motion.y / windowH));

	  mouse.cursor.x = x;
	  mouse.cursor.z = y;  
	}

	if (curCamera && !objectsMenu.open && !dialogEditor.open) { 
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

    if(!console.open){

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
	  mouse.brush = 0;
	}

	if (currentKeyStates[SDL_SCANCODE_1]) {
	  mouse.brush = wallT;
	}

	if (currentKeyStates[SDL_SCANCODE_2]) {
	  mouse.brush = halfWallT;
	}

	if (currentKeyStates[SDL_SCANCODE_3]) {
	  mouse.brush = doorFrameT;
	}

	if (currentKeyStates[SDL_SCANCODE_4]) {
	  mouse.brush = windowT;
	}

	// ~~~~~~~~~~~~~~~~~~
	// Manipulate of focused model
	if(!mouse.focusedModel){
	  manipulationMode = -1;
	}
      
	if (mouse.focusedModel && currentKeyStates[SDL_SCANCODE_LCTRL]){
	  if(currentKeyStates[SDL_SCANCODE_P] && mouse.focusedModel){
	    if(mouse.gridIntersect.x != -1 && mouse.gridIntersect.z != -1){
	      vec3 tile = xyz_indexesToCoords(mouse.gridIntersect.x, floor, mouse.gridIntersect.z);

	      mouse.focusedModel->mat.m[12] = tile.x;
	      mouse.focusedModel->mat.m[13] = tile.y;
	      mouse.focusedModel->mat.m[14] = tile.z;
	    
	      calculateModelAABB(mouse.focusedModel);
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

    mouse.selectedTile = NULL;
    mouse.gridIntersect = (vec2i){ -1,-1 };

    mouse.wallSide = -1;
    mouse.wallTile = (vec3i){ -1,-1,-1 };
    mouse.wallType = -1;
    mouse.wallTx = mouse.wallTx = -1;
    mouse.tileSide = -1;
    mouse.intersection = (vec3){ -1,-1 };
    mouse.groundInter = -1;

    mouse.selectedModel = NULL;


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
		
	  vec3 tile = xyz_indexesToCoords(x,floor,z);

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

	  
    // test 3d model rendering
    {

      //   setSolidColorTx(darkPurple, 1.0f);

      // TODO: maybe sort models in curModels by type/mesh
      
      // and call bindBuffer/bindAttr outside of loop 
      for(int i=0;i<curModelsSize;i++){
	bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, curModels[i].lb, curModels[i].rt, NULL, NULL);

	int name = curModels[i].name;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glActiveTexture(loadedModels1D[name].tx);
	glBindTexture(GL_TEXTURE_2D, loadedModels1D[name].tx);

	if(isIntersect){
	  mouse.selectedModel = &curModels[i];
	  //	  setSolidColorTx(redColor, 1.0f);
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

    // snowParticles rendering
    if(enviromental.snow)
      {
	glActiveTexture(solidColorTx);
	glBindTexture(GL_TEXTURE_2D, solidColorTx);

	setSolidColorTx(whiteColor, 1.0f);

	glBindVertexArray(snowFlakeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, snowFlakeVBO);

	for (int loop = 0; loop < snowAmount; loop++)
	  {
	    if (snowParticle[loop].active) {
	      float x = snowParticle[loop].x;
	      float y = snowParticle[loop].y;
	      float z = snowParticle[loop].z;

	      //	if (radarCheck((vec3) { x, y, z })) {
	      Matrix out = IDENTITY_MATRIX;

	      // translate without mult
	      out.m[12] = x;
	      out.m[13] = y;
	      out.m[14] = z;
		
	      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out.m);

	      glDrawArrays(GL_LINES, 0, 2);
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
		  snowParticle[loop].y = (gridY - 1) * bBlockH;
		  snowParticle[loop].z = (float)(rand() % gridZ / 10.0f) + (float)(rand() % 100 / 1000.0f);
		}
	      }
	      else {
		snowParticle[loop].y += snowSpeed / (1000 / 2.0f);
	      }
	    }
	  }

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	    
	glBindTexture(GL_TEXTURE_2D, 0);
      }

    // main loop of rendering wall/tiles and intersection detecting
    ElementType minDistType = -1;
    float minIntersectionDist = 1000.0f;
    
    for (int x = 0; x < gridX; x++) {
      for (int y = 0; y < gridY; y++) {
	for (int z = 0; z < gridZ; z++) {
	  vec3 tile = xyz_indexesToCoords(x,y,z);
	  // walls
	  if(grid[y][z][x].walls !=0){
	    for(int side=0;side<basicSideCounter;side++){
	      WallType type = (grid[y][z][x].walls >> (side*8)) & 0xFF;

	      if(type == 0){
		continue;
	      }
	      
	      vec3* wallPos = wallPosBySide(side, wallsSizes[type].h, wallD, bBlockD, bBlockW);

	      vec3 lb = {0};
	      vec3 rt = {0};
		
	      lb.x = tile.x + min(wallPos[3].x, wallPos[2].x);
	      lb.y = tile.y + wallPos[3].y;
	      lb.z = tile.z + min(wallPos[3].z, wallPos[2].z);

	      rt.x = tile.x + max(wallPos[0].x, wallPos[1].x);
	      rt.y = tile.y + wallPos[0].y;
	      rt.z = tile.z + max(wallPos[0].z, wallPos[1].z);

	      free(wallPos);
	    
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
	    
	      Texture tx = valueIn(grid[y][z][x].wallsTx, side);
	      
	      if(y >= floor){
		float intersectionDistance = 0.0f;
		bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, lb, rt, NULL, &intersectionDistance);

		if(isIntersect && minIntersectionDist > intersectionDistance){
		  minIntersectionDist = intersectionDistance;
		
		  mouse.wallSide = side;
		  mouse.wallTile = (vec3i){x,y,z};
		  mouse.wallType = type;
		  mouse.wallTx = tx;

		  minDistType = WallEl;
		}
	      }
	    
	
	      glBindVertexArray(wallMeshes[side][type-1].VAO);
	      glBindBuffer(GL_ARRAY_BUFFER, wallMeshes[side][type-1].VBO);
		
	      glActiveTexture(tx);
	      glBindTexture(GL_TEXTURE_2D, tx);
	      		
	      Matrix out = IDENTITY_MATRIX;

	      // translate without mult
	      out.m[12] = tile.x;
	      out.m[13] = tile.y;
	      out.m[14] = tile.z;
		
	      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out.m);


	      glDrawArrays(GL_TRIANGLES, 0, GL_ARRAY_BUFFER, wallMeshes[side][type-1].VBOsize);

	      glBindTexture(GL_TEXTURE_2D, 0);
	      glBindBuffer(GL_ARRAY_BUFFER, 0);
	      glBindVertexArray(0);
		
	    }
	  }

	  // tile inter
	  {
	    GroundType type = valueIn(grid[y][z][x].ground, 0);

	    // skip netTile on not cur floor
	    if(type == netTile && y != floor){
	      continue;
	    }
	    
	    const vec3 tile = { (float)x / 10, (float) y * bBlockH, (float)z / 10 };

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
	
	    if(isIntersect && minIntersectionDist > intersectionDistance){
	      minIntersectionDist = intersectionDistance;

	      if (y == floor) {
		mouse.selectedTile = &grid[y][z][x];
		mouse.gridIntersect = (vec2i){x,z};
		mouse.intersection = intersection;
		mouse.groundInter = intersection.y <= curCamera->pos.y ? fromOver : fromUnder;

		minDistType = TileEl;
		//  continue;
	      }
	    }
	
	    // tile rendering
	    if(type == texturedTile){
	      for(int i=1;i<=2;i++){
		if(y==0 && i == fromUnder){
		  continue;
		}
		
		glBindBuffer(GL_ARRAY_BUFFER, tileMeshes[i-1].VBO);
		glBindVertexArray(tileMeshes[i-1].VAO);

		Texture tx = valueIn(grid[y][z][x].ground, i);
		  
		glActiveTexture(tx);
		glBindTexture(GL_TEXTURE_2D, tx);

		Matrix out = IDENTITY_MATRIX;
		//	  translate(&out, argVec3(tile));

		// translate without mult
		out.m[12] = tile.x;
		out.m[13] = tile.y;
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
    }

    // render mouse.brush
    if(mouse.selectedTile)
      {
	//	  const float borderArea = bBlockW/8;
      
	const int x = mouse.gridIntersect.x;
	const int z = mouse.gridIntersect.z;

	const vec3 tile = xyz_indexesToCoords(x,floor,z);

	if(mouse.intersection.x < tile.x + borderArea){
	  mouse.tileSide = left;
	}else if(mouse.intersection.x > tile.x + bBlockW - borderArea){
	  mouse.tileSide = right;
	}else{
	  if(mouse.intersection.z < tile.z +borderArea){
	    mouse.tileSide = top;
	  }
	  else if(mouse.intersection.z > tile.z + bBlockD - borderArea){
	    mouse.tileSide = bot;
	  }else if(mouse.intersection.z > (tile.z + bBlockD/2) - borderArea && mouse.intersection.z < (tile.z + bBlockD/2) + borderArea && mouse.intersection.x >(tile.x + bBlockW/2) - borderArea && mouse.intersection.x < (tile.x +bBlockW/2) + borderArea){
	    mouse.tileSide = center;
	  }else{
	    mouse.tileSide = -1;
	  }
	}

	const float selectionW = borderArea * 3;

	glActiveTexture(solidColorTx);
	glBindTexture(GL_TEXTURE_2D, solidColorTx);
	setSolidColorTx(darkPurple, 1.0f);

	//	  glBegin(GL_TRIANGLES);
	
	if(mouse.tileSide == center){
	  vec3 pos = { tile.x + bBlockD / 2 - borderArea, tile.y + selBorderD/2.0f, tile.z + bBlockD / 2 - borderArea };

	  glBindVertexArray(netTileVAO);
	  glBindBuffer(GL_ARRAY_BUFFER, netTileVBO);

	  Matrix out = IDENTITY_MATRIX;

	  // translate without mult
	  out.m[12] = pos.x;
	  out.m[13] = pos.y;
	  out.m[14] = pos.z;
		
	  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out.m);

	  glDrawArrays(GL_TRIANGLES, 0, 6);
	}else if(mouse.tileSide == top || mouse.tileSide == bot){
	  float zPos = tile.z;
	      
	  if(mouse.tileSide == top){
	    zPos -= borderArea / 2;
	  }else{
	    zPos += bBlockD - borderArea/2;
	  }

	  vec3 pos = { tile.x + borderArea + selectionW / 2, tile.y + selBorderD/2.0f, zPos };

	  //    glVertex3f(pos.x, pos.y, pos.z);
	  //   glVertex3f(pos.x + borderArea * 3, pos.y, pos.z);
	  //	    glVertex3f(pos.x + borderArea * 3,pos.y, pos.z + borderArea);

	  //    glVertex3f(pos.x ,pos.y, pos.z);
	  //    glVertex3f(pos.x,pos.y, pos.z + borderArea);
	  //    glVertex3f(pos.x + borderArea * 3,pos.y, pos.z + borderArea);
	}else if(mouse.tileSide == left || mouse.tileSide == right){
	  float xPos = tile.x;
	      
	  if(mouse.tileSide == right){
	    xPos += bBlockW - borderArea/2;
	  }else{
	    xPos -= borderArea/2;
	  }

	  vec3 pos = {xPos, tile.y + selBorderD/2.0f, tile.z + bBlockW/2 - selectionW / 2};

	  //   glVertex3f(pos.x, pos.y, pos.z);
	  // glVertex3f(pos.x + borderArea, pos.y, pos.z);
	  // glVertex3f(pos.x + borderArea,pos.y, pos.z + borderArea * 3);

	  //   glVertex3f(pos.x ,pos.y, pos.z);
	  //   glVertex3f(pos.x,pos.y, pos.z + borderArea * 3);
	  // glVertex3f(pos.x + borderArea,pos.y, pos.z + borderArea * 3);
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	  
	//      glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	if(mouse.tileSide != -1 && mouse.tileSide != center){
	    
	  Side oppositeSide = 0;
	  vec2i oppositeTile = {0};

	  vec3* wallPos = wallPosBySide(mouse.tileSide, wallsSizes[mouse.brush].h, wallD, bBlockD, bBlockW);

	  switch(mouse.brush){
	  case(windowT):{
	    //renderWindow(wallPos,0);
	    break;
	  }
	  case(doorFrameT):{
	    //renderDoorFrame(wallPos,0);
	    break;
	  }
	  default: {
	    //	  renderWall(wallPos, 0);
	    break;
	  };
	  }
	      
	  if(oppositeTileTo((vec2i){x, z}, mouse.tileSide,&oppositeTile,&oppositeSide)){
	    //  vec3* wallPos = wallPosBySide((vec3){(float)oppositeTile.x / 10, tile.y, (float)oppositeTile.z / 10}, oppositeSide, wallsSizes[mouse.brush].h, wallD, bBlockD, bBlockW);
	    vec3* wallPos = NULL;
	    switch(mouse.brush){
	    case(windowT):{
	      //renderWindow(wallPos,0);
	      break;
	    }
	    case(doorFrameT):{
	      //renderDoorFrame(wallPos,0);
	      break;
	    }
	    default: {
	      // renderWall(wallPos, 0);
	      break;
	    };
	    }
	  }

	  if(mouse.clickR){
	    // delete if something exist on this place
	    {
	      WallType type = (mouse.selectedTile->walls >> (mouse.tileSide*8)) & 0xFF;

	      if(type){
		mouse.selectedTile->walls &= ~(0xFF << (mouse.tileSide * 8));
	      }
	    }

	    Side oppositeSide = 0;
	    vec2i oppositeTile = {0};

	    setIn(grid[floor][z][x].walls, mouse.tileSide, mouse.brush);
	    setIn(grid[floor][z][x].wallsTx, mouse.tileSide, 0); // first texture
		  
	    if(oppositeTileTo((vec2i){x, z}, mouse.tileSide,&oppositeTile,&oppositeSide)){
	      grid[floor][(int)oppositeTile.z][(int)oppositeTile.x].walls |= (mouse.brush << oppositeSide*8);
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
      if(highlighting && mouse.wallSide != -1)
	{
	  //mouse.interDist = minIntersectionDist;
      
	  vec3 pos = xyz_indexesToCoords(mouse.wallTile.x, mouse.wallTile.y, mouse.wallTile.z);
		
	  Matrix out = IDENTITY_MATRIX;

	  // translate without mult
	  out.m[12] = pos.x;
	  out.m[13] = pos.y;
	  out.m[14] = pos.z;
		
	  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out.m);

	  if(mouse.wallType == halfWallT){
	    glBindVertexArray(halfWallHighlight[mouse.wallSide].VAO);
	    glBindBuffer(GL_ARRAY_BUFFER, halfWallHighlight[mouse.wallSide].VBO);
	    glDrawArrays(GL_TRIANGLES, 0, GL_ARRAY_BUFFER, halfWallHighlight[mouse.wallSide].VBOsize);
	    // vec3* wallPos = wallPosBySide(pos, mouse.wallSide, bBlockH * 0.4f, wallD, bBlockD, bBlockW);
	    //renderWallBorder(wallPos,mouse.wallSide, selBorderT, redColor);
	  }
	  else {
	    glBindVertexArray(wallHighlight[mouse.wallSide].VAO);
	    glBindBuffer(GL_ARRAY_BUFFER, wallHighlight[mouse.wallSide].VBO);
	    glDrawArrays(GL_TRIANGLES, 0, GL_ARRAY_BUFFER, wallHighlight[mouse.wallSide].VBOsize);
	    //vec3* wallPos = wallPosBySide(pos, mouse.wallSide, bBlockH, wallD, bBlockD, bBlockW);
	    //  renderWallBorder(wallPos,mouse.wallSide, selBorderT, redColor);
	  } 
	}else{
	mouse.interDist = 0.0f;
      }

      //highlight intersected tile
  
      if(highlighting && mouse.groundInter != -1){
	GroundType type = valueIn(mouse.selectedTile->ground, 0);

	if(type != 0){
	  vec3 tile = xyz_indexesToCoords(mouse.gridIntersect.x, floor, mouse.gridIntersect.z);

	  glBindVertexArray(tileHighlight[mouse.groundInter-1].VAO);
	  glBindBuffer(GL_ARRAY_BUFFER, tileHighlight[mouse.groundInter-1].VBO);
		
	  Matrix out = IDENTITY_MATRIX;

	  // translate without mult
	  out.m[12] = tile.x;
	  out.m[13] = tile.y;
	  out.m[14] = tile.z;
		
	  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out.m);

	  glDrawArrays(GL_TRIANGLES, 0, GL_ARRAY_BUFFER, tileHighlight[mouse.groundInter-1].VBOsize);

	}
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


    // render objects menu
    if(objectsMenu.open){
      glActiveTexture(solidColorTx);
      glBindTexture(GL_TEXTURE_2D, solidColorTx);
      setSolidColorTx(blackColor, 1.0f);
      
      glBindVertexArray(objectsMenu.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, objectsMenu.VBO);

      glDrawArrays(GL_TRIANGLES, 0, GL_ARRAY_BUFFER, 6);
    
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
      
      int selectedIndex = modelsTypesInfo[0].counter - ( ((mouse.cursor.z + 0.5f) - 0.06f)  / .1f);  

      if(mouse.cursor.x <= objectsMenuWidth && selectedIndex <= modelsTypesInfo[objectsMenuSelectedType].counter){
	if(mouse.clickL){
	  curModelsSize++;

	  if(curModels){ 
	    curModels = realloc(curModels, curModelsSize * sizeof(Model));
	  }else{
	    curModels = malloc(sizeof(Model)); 
	  }

	  curModels[curModelsSize-1].id = curModelsSize-1;

	  int index1D = loadedModels2D[objectsMenuSelectedType][selectedIndex - 1].index1D;

	  curModels[curModelsSize-1].name = index1D;

	  // if type == char add new character
	  if(loadedModels1D[index1D].type == characterModelType) {
	    if(characters == NULL){
	      characters = malloc(sizeof(Character));
	    }else{
	      characters = realloc(characters, (charactersSize+1) * sizeof(Character));
	    }

	    characters[charactersSize].id = charactersSize;
	    characters[charactersSize].modelId = curModels[curModelsSize-1].id;
	    characters[charactersSize].modelName = curModels[curModelsSize-1].name;
	    characters[charactersSize].dialogs = NULL;
	    characters[charactersSize].name = NULL;
	    
	    charactersSize++;
	  }
	  
	  curModels[curModelsSize-1].mat = IDENTITY_MATRIX;

	  scale(&curModels[curModelsSize-1].mat, 0.25f, 0.25f, 0.25f); 
	  if(mouse.gridIntersect.x != -1 && mouse.gridIntersect.z != -1){
	    vec3 tile = xyz_indexesToCoords(mouse.gridIntersect.x, floor, mouse.gridIntersect.z);
	    
	    curModels[curModelsSize-1].mat.m[12] = tile.x;
	    curModels[curModelsSize-1].mat.m[13] = tile.y;
	    curModels[curModelsSize-1].mat.m[14] = tile.z;
	  }
	  
	  calculateModelAABB(&curModels[curModelsSize-1]);
	  
	  objectsMenu.open = false;
	};

	setSolidColorTx(redColor, 1.0f);
      
	glBindVertexArray(selectionRectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, selectionRectVBO);
	
	float cursorH = 0.06f;
	float cursorW = 0.02f;
	  
	float  selectionRect[] = {
	  -1.0f, 1.0f - (selectedIndex-1) * 0.1f, 1.0f, 0.0f,
	  objectsMenuWidth, 1.0f - (selectedIndex-1) * 0.1f, 1.0f, 1.0f,
	  -1.0f, 1.0f - (selectedIndex) * 0.1f, 0.0f, 0.0f,

	  objectsMenuWidth, 1.0f - (selectedIndex-1) * 0.1f, 1.0f, 1.0f,
	  -1.0f,  1.0f - selectedIndex * 0.1f, 0.0f, 0.0f,
	  objectsMenuWidth,  1.0f - selectedIndex * 0.1f, 0.0f, 1.0f };

	glBufferData(GL_ARRAY_BUFFER, sizeof(selectionRect), selectionRect, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLES, 0, GL_ARRAY_BUFFER, 6);

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
	float typeButtonH = 0.1f;

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
	    baseX, 1.0f - (i) * typeButtonH, 1.0f, 0.0f,
	    baseX + typeButtonW, 1.0f - (i) * typeButtonH, 1.0f, 1.0f,
	    baseX, 1.0f - (i+1) * typeButtonH, 0.0f, 0.0f,

	    baseX + typeButtonW, 1.0f - (i) * typeButtonH, 1.0f, 1.0f,
	    baseX, 1.0f - (i+1) * typeButtonH, 0.0f, 0.0f,
	    baseX + typeButtonW, 1.0f - (i+1) * typeButtonH, 0.0f, 1.0f };

	  glBufferData(GL_ARRAY_BUFFER, sizeof(typeRect), typeRect, GL_STATIC_DRAW);

	  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	  glEnableVertexAttribArray(0);

	  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	  glEnableVertexAttribArray(1);

	  glDrawArrays(GL_TRIANGLES, 0, GL_ARRAY_BUFFER, 6);
	  
	}
      // TODO: Come up how to merge these two loops
      // it throws exeption if put it into loop above
      for (int i = 0; i < modelTypeCounter; i++) {
	renderText(modelsTypesInfo[i].str, baseX, 1.0f - ((i) * typeButtonH), 1.0f);
      }
      }


      for(int i=0;i<modelsTypesInfo[objectsMenuSelectedType].counter;i++){
	renderText(loadedModels2D[objectsMenuSelectedType][i].name, -1.0f, 1.0f - (i * 0.1f), 1);
      }
    }else if(dialogEditor.open){
      glActiveTexture(solidColorTx);
      glBindTexture(GL_TEXTURE_2D, solidColorTx);
      setSolidColorTx(blackColor, 1.0f);
      
      glBindVertexArray(dialogEditor.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, dialogEditor.VBO);

      glDrawArrays(GL_TRIANGLES, 0, GL_ARRAY_BUFFER, 6);
    
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      glBindTexture(GL_TEXTURE_2D, solidColorTx);
      setSolidColorTx(greenColor, 1.0f);
      
      glBindVertexArray(dialogEditorNameInput.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, dialogEditorNameInput.VBO);

      glDrawArrays(GL_TRIANGLES, 0, GL_ARRAY_BUFFER, 6);
    
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }else if(mouse.focusedModel){ 
      char buf[100];

      sprintf(buf, "Focused model [%s-%d] Mode: %s", loadedModels1D[mouse.focusedModel->name].name, mouse.focusedModel->id, manipulationMode!= -1 ?manipulationModeStr[manipulationMode] : "None");

      renderText(buf, -1.0f, 1.0f, 1.0f);
    }

    // render cursor
    if(objectsMenu.open || dialogEditor.open)
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
	  0.0f,0.0f,0.0f,0.0f,
	  0.0f,0.0f,0.0f,0.0f,
	  0.0f,0.0f,0.0f,0.0f,
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(cursorPoint), cursorPoint, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLES, 0, GL_ARRAY_BUFFER, 6);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
      }


    if(console.open){
      glActiveTexture(solidColorTx);
      glBindTexture(GL_TEXTURE_2D, solidColorTx);
      setSolidColorTx(blackColor, 1.0f);
      
      glBindVertexArray(console.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, console.VBO);

      glDrawArrays(GL_TRIANGLES, 0, GL_ARRAY_BUFFER, 6);
    
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

      renderText(consoleBuffer, -1.0f + letterW / 2.0f, 1.0f, 1.0f);

      if(consoleHasResponse){
	renderText(consoleResponse, -1.0f + letterW / 2.0f, 1.0f - consoleH, 1.0f);

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
      sprintf(windowTitle, game" FPS: %d Save: %s.doomer", 1000 / deltatime, curSaveName);
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

      if(type == wallT || type == halfWallT){
	float verts[] = {
	  argVec3(wallPos[0]), 0.0f, 1.0f,
	  argVec3(wallPos[1]), 1.0f, 1.0f,
	  argVec3(wallPos[3]), 0.0f, 0.0f, 
      
	  argVec3(wallPos[1]), 1.0f, 1.0f,
	  argVec3(wallPos[2]), 1.0f, 0.0f,
	  argVec3(wallPos[3]), 0.0f, 0.0f, 
	};

	  
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	  
	wallMeshes[side][type-1].VBOsize = 6;
	//	  printf("Wall size: %d \n", (sizeof(verts) / (sizeof(float) * 3)));
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
    SDL_Surface* texture = IMG_Load(texturePath);

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
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBindVertexArray(textVAO);
  glBindBuffer(GL_ARRAY_BUFFER, textVBO);
  glActiveTexture(fontAtlas);
  glBindTexture(GL_TEXTURE_2D, fontAtlas);

  int iter = 0;
  int padCounter = 0;
  char ch = text[iter];

  x += letterW;
  //x += letterH;
  
  while(ch){
    if(ch == '\n'){
      y-=letterH;
      padCounter = 0;

      iter++;
      ch = text[iter];

      continue;
    }
    
    float lettersPad = padCounter * (letterW / 1.9);
    int index = ch - 33;
    vec2i pos = englLettersMap[index];
  
    float baseY = atlasStep * pos.z;
    float baseX = atlasStep * pos.x;

    float symbol[] = {
      x + lettersPad, y, baseX, baseY + atlasStep, // left top
      x - letterW + lettersPad, y, baseX + atlasStep, baseY + atlasStep, // right top

      x + lettersPad, y - letterH, baseX, baseY, // right bot

      x - letterW + lettersPad, y, baseX + atlasStep, baseY + atlasStep, // right top

      x + lettersPad, y - letterH, baseX, baseY, // right bot
      x - letterW + lettersPad, y - letterH, baseX + atlasStep, baseY
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(symbol), symbol, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1 , 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
      
    glDrawArrays(GL_TRIANGLES, 0, GL_ARRAY_BUFFER, 6);
  
    iter++;
    padCounter++;
    ch = text[iter];
  }
  
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  
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
  //  resetMouse();
  
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
      free(curModels);
      curModels = NULL;
      curModelsSize = 0;
    }
  }
  
  int sizeX, sizeZ, sizeY;
  fscanf(map, "%d %d %d \n", &gridY, &gridZ, &gridX);
      
  grid = malloc(sizeof(Tile**) * (gridY));

  for (int y = 0; y < gridY; y++) {
    grid[y] = malloc(sizeof(Tile*) * (gridZ));

    for (int z = 0; z < gridZ; z++) {
      grid[y][z] = malloc(sizeof(Tile) * (gridX));

      for (int x = 0; x < gridX; x++) {
	fscanf(map, "[Wls: %d, WlsTx %d, Grd: %d]", &grid[y][z][x].walls, &grid[y][z][x].wallsTx, &grid[y][z][x].ground);

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
  }
    
  fscanf(map, "\nUsed models: %d\n", &curModelsSize);

  if(curModelsSize != 0){
    curModels = malloc(curModelsSize * sizeof(Model));
    
    for(int i=0; i<curModelsSize; i++){
      /*	  if(curModels[i].name == yalinka){
      // TODO: Make here some enum to string function
      fprintf(map, "Yalinka[%d] ", yalinka);
      }*/
      int name = -1;
      fscanf(map, "%d ", &name);

      if(name >= loadedModelsSize || name < 0){
	printf("Models parsing error, model name (%d) doesnt exist \n", name);
	exit(0); 
      }   
	  
      //curModels[i] = malloc(sizeof(Model));
      curModels[i].name = name;    
      curModels[i].id = i;  
	   
      //	  memcpy(curModels[i], models[name], sizeof(Model));

      fgetc(map); // read [

      for(int mat=0;mat<16;mat++){
	fscanf(map, "%f ", &curModels[i].mat.m[mat]);
      }

      calculateModelAABB(&curModels[i]);
	  
      fgetc(map); // read ]\n
    }
  }

  fscanf(map, "\nCharacters: %d\n", &charactersSize);

  characters = malloc(sizeof(Character) * charactersSize);
   
  for(int i=0; i<charactersSize; i++){ 
    int name = -1;
    int dialogs = -1;
    fscanf(map, "%d %d %d %d\n", &characters[i].modelId, &characters[i].modelName, &name, &dialogs);
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

  for (int y = 0; y < gridY; y++) {
    for (int z = 0; z < gridZ; z++) {
      for (int x = 0; x < gridX; x++) {
	fprintf(map, "[Wls: %d, WlsTx %d, Grd: %d],", grid[y][z][x].walls, grid[y][z][x].wallsTx, grid[y][z][x].ground);
      }
    }

    fprintf(map, "\n");
  }

  fprintf(map, "\nUsed models: %d\n",curModelsSize);
	
  for(int i=0; i<curModelsSize; i++){    
    fprintf(map, "%d ", curModels[i].name);

    fprintf(map, "[");

    for(int mat=0;mat<16;mat++){
      fprintf(map, "%f ", curModels[i].mat.m[mat]);
    }
	  
    fprintf(map, "]\n");
  }

  // save characters
  fprintf(map, "\Characters: %d\n", charactersSize);
  
  for(int i=0; i<charactersSize; i++){
    fprintf(map, "%d %d %d %d\n", characters[i].modelId, characters[i].modelName, 0, 0);
  }
  
  printf("Map saved!\n");

  fclose(map);

  return true;
}

bool createMap(int newX, int newY, int newZ){
  //  resetMouse();
  
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
    
  grid = malloc(sizeof(Tile**) * (newY));

  for (int y = 0; y < newY; y++) {
    grid[y] = malloc(sizeof(Tile*) * (newZ));

    for (int z = 0; z < newZ; z++) {
      grid[y][z] = calloc(newX, sizeof(Tile));

      for (int x = 0; x < newX; x++) {
	if (y == 0) {
	  setIn(grid[y][z][x].ground, 0, texturedTile);
	  setIn(grid[y][z][x].ground, 2, frozenGround);
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

    for (int loop = 0; loop < snowAmount; loop++)
      {
	snowParticle[loop].active = true;

	snowParticle[loop].life = 1.0f;
	snowParticle[loop].fade = (float)(rand() % 100) / 1000.0f + 0.003f;

	snowParticle[loop].x = (float)(rand() % gridX / 10.0f) + (float)(rand() % 100 / 1000.0f);
	snowParticle[loop].y = (float)(rand() % (int)(gridY * bBlockH)) + (float)(rand() % 1000) / 1000.0f;
	snowParticle[loop].z = (float)(rand() % gridZ / 10.0f) + (float)(rand() % 100 / 1000.0f);
      }
  }
}
