#include "deps.h"
#include "linearAlg.h"
#include "main.h"

Object** objsStore;
size_t objsStoreSize;

// avaible/loaded models
Model** models;
size_t modelsSize;

// placed/created models
Model** curModels;
size_t curModelsSize;

int gridX = 120;
int gridY = 15;
int gridZ = 120;

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

GLuint prog;

bool sphereInter(vec3 centroid){
  float t0, t1;

  float tca = dotf3(centroid, mouse.rayDir);
  // if (tca < 0) return false;
  vec3 centroid2 = { centroid.x - tca * tca, centroid.y - tca * tca, centroid.z - tca * tca};

  float d2 = dotf3(centroid, centroid) - tca* tca;

  if (d2 > 0.1f) return false;

  float thc = sqrt(0.1f - d2);
  t0 = tca - thc;
  t1 = tca + thc;

  if (t0 > t1){
    float temp = t0;
    t0 = t1;
    t1 = temp;
  };

  if (t0 < 0) {
    t0 = t1; // if t0 is negative, let's use t1 instead
    if (t0 < 0) return false; // both t0 and t1 are negative
  }


  // t = t0;

  return true;
};

int main(int argc, char* argv[]) {
  borderArea = (float)bBlockW/8;
  
  
  SDL_Init(SDL_INIT_VIDEO);

  char windowTitle[] = game;
  SDL_Window* window = SDL_CreateWindow(windowTitle,
					SDL_WINDOWPOS_CENTERED,
					SDL_WINDOWPOS_CENTERED,
					windowW, windowH,
					SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

  
  SDL_SetRelativeMouseMode(SDL_TRUE);
  SDL_GLContext context = SDL_GL_CreateContext(window);
  
  SDL_ShowCursor(SDL_DISABLE);

  GLuint netTileVBO, netTileVAO;
  GLuint texturedTileVBO, texturedTileVAO;
  GLuint snowFlakeVBO, snowFlakeVAO;
  GLuint centerMouseSelVBO, centerMouseSelVAO;

  glewInit();

  // load 3d models
  {
    models = malloc(sizeof(Model));
    modelsSize++;
    models[yalinka] = loadOBJ("./assets/objs/Car.obj", yalinka);
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

    prog = glCreateProgram();
    glAttachShader(prog, vertexShader);
    glAttachShader(prog, fragmentShader);

    // Link the shader program
    glLinkProgram(prog);

    // Check for linking errors
    GLint linkStatus;
    glGetProgramiv(prog, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE) {
      GLint logLength;
      glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
      char* log = (char*)malloc(logLength);
      glGetProgramInfoLog(prog, logLength, NULL, log);
      fprintf(stderr, "Failed to link program: %s\n", log);
      free(log);
      return 1;
    }

    glUseProgram(prog);
  }
  
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
    glEnable(GL_BLEND);
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

  GLuint carTx;
  glGenTextures(1, &carTx);

  // -1 because of solidColorTx
  SDL_Surface* texture = IMG_Load("./assets/objs/car_snow_red.png");

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

  // tang of fov calculations
  fov = editorFOV;
  tangFOV = tanf(rad(fov) * 0.5);
  
  mouse = (Mouse){ .h = 0.005f, .w = 0.005f, .brush = 0, .interDist = 1000.0f  };

  float dist = sqrt(1 / 3.0);
  bool cameraMode = true;

  float testFOV = editorFOV;

  Tile*** grid = NULL;

  // load or init grid
  {
    FILE* map = fopen("map.doomer", "r");

    if (map == NULL) {
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
    else {
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
	curModels = malloc(curModelsSize * sizeof(Model*));
    
	for(int i=0; i<curModelsSize; i++){
	  /*	  if(curModels[i]->name == yalinka){
	  // TODO: Make here some enum to string function
	  fprintf(map, "Yalinka[%d] ", yalinka);
	  }*/
	  int name = -1;
	  fscanf(map, "%d ", &name);

	  if(name >= modelsCounter || name < 0){
	    printf("Models parsing error, model name (%d) doesnt exist \n", name);
	    exit(0);
	  }
	  
	  curModels[i] = malloc(sizeof(Model));
	  memcpy(curModels[i], models[name], sizeof(Model));

	  fgetc(map); // read [

	  for(int mat=0;mat<16;mat++){
	    fscanf(map, "%f ", &curModels[i]->mat.m[mat]);
	  }

	  fgetc(map); // read ]\n
	}

	printf("Map loaded! \n");
	fclose(map);
      }
    }
  }

  // set up camera
  GLint cameraPos = glGetUniformLocation(prog, "cameraPos");
  {
    camera1.pos = (vec3)xyz_indexesToCoords(gridX/2, 2, gridZ/2);
    camera2.pos = (vec3)xyz_indexesToCoords(gridX/2, 2, gridZ/2);
    camera1.up = (vec3){ 0.0f, 1.0f, 0.0f };
    camera2.up = (vec3){ 0.0f, 1.0f, 0.0f };
    
    glUniform3f(cameraPos, camera1.pos.x, camera1.pos.y, camera1.pos.z);
  }
  
  // set draw distance to gridX/2
  GLint radius = glGetUniformLocation(prog, "radius");
  drawDistance = gridX/2 * 0.2;
  glUniform1f(radius, drawDistance);

  ManipulationMode manipulationMode = -1;

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
	switch (event.key.keysym.scancode) {
	case(SDL_SCANCODE_UP): {
	  if(manipulationMode != -1){
	    switch(manipulationMode){
	    case(TRANSFORM_Z):{
	      curModels[curModelsSize-1]->mat.m[13] = curModels[curModelsSize-1]->mat.m[13] + 0.05f;
	      calculateModelAABB(curModels[curModelsSize-1]);
	      break;
	    }
	    case(TRANSFORM_XY):{
	      curModels[curModelsSize-1]->mat.m[12] = curModels[curModelsSize-1]->mat.m[12] - 0.05f;
	      calculateModelAABB(curModels[curModelsSize-1]);
	      break;
	    }
	    case(SCALE):{
	      scale(curModels[curModelsSize-1]->mat.m, 1.05f, 1.05f, 1.05f);
	      calculateModelAABB(curModels[curModelsSize-1]);
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
	      curModels[curModelsSize-1]->mat.m[13] = curModels[curModelsSize-1]->mat.m[13] - 0.05f;
	      calculateModelAABB(curModels[curModelsSize-1]);
	      break;
	    }
	    case(TRANSFORM_XY):{
	      curModels[curModelsSize-1]->mat.m[12] = curModels[curModelsSize-1]->mat.m[12] + 0.05f;
	      calculateModelAABB(curModels[curModelsSize-1]);
	      break;
	    }
	    case(SCALE):{
	      scale(curModels[curModelsSize-1]->mat.m, 1.0f/1.05f, 1.0f/1.05f, 1.0f/1.05f);
	      calculateModelAABB(curModels[curModelsSize-1]);
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
	      rotateY(curModels[curModelsSize-1]->mat.m, -rad(1.0f));
	      calculateModelAABB(curModels[curModelsSize-1]);
	      break;
	    }
	    case(ROTATE_Z):{
	      rotateZ(curModels[curModelsSize-1]->mat.m, -rad(1.0f));
	      calculateModelAABB(curModels[curModelsSize-1]);
	      break;
	    }
	    case(ROTATE_X):{
	      rotateX(curModels[curModelsSize-1]->mat.m, -rad(1.0f));
	      calculateModelAABB(curModels[curModelsSize-1]);
	      break;
	    }
	    case(TRANSFORM_XY):{
	      curModels[curModelsSize-1]->mat.m[14] = curModels[curModelsSize-1]->mat.m[14] + 0.05f;
	      calculateModelAABB(curModels[curModelsSize-1]);
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
	      rotateY(curModels[curModelsSize-1]->mat.m, rad(1.0f));
	      calculateModelAABB(curModels[curModelsSize-1]);
	      break;
	    }
	    case(ROTATE_Z):{
	      rotateZ(curModels[curModelsSize-1]->mat.m, rad(1.0f));
	      calculateModelAABB(curModels[curModelsSize-1]);
	      break;
	    }
	    case(ROTATE_X):{
	      rotateX(curModels[curModelsSize-1]->mat.m, rad(1.0f));
	      calculateModelAABB(curModels[curModelsSize-1]);
	      break;
	    }
	    case(TRANSFORM_XY):{
	      curModels[curModelsSize-1]->mat.m[14] = curModels[curModelsSize-1]->mat.m[14] - 0.05f;
	      calculateModelAABB(curModels[curModelsSize-1]);
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
	  curModelsSize++;

	  if(curModels){
	    curModels = realloc(curModels, curModelsSize * sizeof(Model*));
	  }else{
	    curModels = malloc(sizeof(Model*));
	  }

	  curModels[curModelsSize-1] = malloc(sizeof(Model));
	  memcpy(curModels[curModelsSize-1], models[yalinka], sizeof(Model));

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
	  FILE* map = fopen("map.doomer", "w");

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
	    /*	  if(curModels[i]->name == yalinka){
	    // TODO: Make here some enum to string function
	    fprintf(map, "Yalinka[%d] ", yalinka);
	    }*/
	    fprintf(map, "%d ", yalinka);

	    fprintf(map, "[");

	    for(int mat=0;mat<16;mat++){
	      fprintf(map, "%f ", curModels[i]->mat.m[mat]);
	    }
	  
	    fprintf(map, "]\n");
	  }

	  printf("Map saved!\n");

	  fclose(map);
	  break;
	}
	case(SDL_SCANCODE_H): {
	  highlighting = !highlighting;
	  break;
	}
	case(SDL_SCANCODE_DELETE): {
	  if (mouse.wallSide != -1) {
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
	mouse.screenPos.x = event.motion.x;
	mouse.screenPos.z = event.motion.y;

	if (curCamera) {
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
      if(!mouse.selectedModel){
	manipulationMode = -1;
      }
      
      if (mouse.selectedModel && currentKeyStates[SDL_SCANCODE_LCTRL]){
	if (currentKeyStates[SDL_SCANCODE_R]){
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

    printf("Mode: %d \n", manipulationMode);

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

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int modelLoc = glGetUniformLocation(prog, "model");

    // send proj and view mat to shader
    {
      Matrix projMat = perspective(rad(fov), windowW / windowH, 0.1f, 100.0f);

      int proj = glGetUniformLocation(prog, "proj");
      glUniformMatrix4fv(proj, 1, GL_FALSE, projMat.m);

      Matrix view =  IDENTITY_MATRIX;
      vec3 negPos = { -curCamera->pos.x, -curCamera->pos.y, -curCamera->pos.z };

      translate(&view, argVec3(negPos));
      rotateY(&view, rad(curCamera->yaw));
      rotateX(&view, rad(curCamera->pitch));
      int viewLoc = glGetUniformLocation(prog, "view");
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
      
	inverse(projMat.m, inversedProj.m);
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
	bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, curModels[i]->lb, curModels[i]->rt, NULL, NULL);

	glActiveTexture(carTx);
	glBindTexture(GL_TEXTURE_2D, carTx);

	if(isIntersect){
	  mouse.selectedModel = curModels[i];
	  //	  setSolidColorTx(redColor, 1.0f);
	}

	glBindVertexArray(curModels[i]->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, curModels[i]->VBO);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, curModels[i]->mat.m);

	glDrawArrays(GL_TRIANGLES, 0, curModels[i]->size);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//	 vec3* minMax = minMaxPair(curModels[i]->vertices, curModels[i]->size);

	 
	// draw AABB
	glActiveTexture(solidColorTx);
	glBindTexture(GL_TEXTURE_2D, solidColorTx);
	setSolidColorTx(greenColor, 1.0f);

	Matrix out = IDENTITY_MATRIX;

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out.m);

	//	printf("max %f %f %f\n", argVec3(minMax[0]));
	//	printf("min %f %f %f\n", argVec3(minMax[1]));

	glBegin(GL_LINES);

	
	glVertex3f(argVec3(curModels[i]->lb));
	glVertex3f(argVec3(curModels[i]->rt));
	
	glVertex3f(curModels[i]->rt.x,curModels[i]->rt.y,curModels[i]->rt.z);
	glVertex3f(curModels[i]->rt.x,curModels[i]->lb.y,curModels[i]->rt.z);

	glVertex3f(curModels[i]->lb.x,curModels[i]->lb.y,curModels[i]->lb.z);
	glVertex3f(curModels[i]->lb.x,curModels[i]->rt.y,curModels[i]->lb.z);
	
	glVertex3f(curModels[i]->rt.x,curModels[i]->rt.y,curModels[i]->lb.z);
	glVertex3f(curModels[i]->rt.x,curModels[i]->lb.y,curModels[i]->lb.z);

	glVertex3f(curModels[i]->lb.x,curModels[i]->rt.y,curModels[i]->rt.z);
	glVertex3f(curModels[i]->lb.x,curModels[i]->lb.y,curModels[i]->rt.z);
	
	glEnd();

	setSolidColorTx(darkPurple, 1.0f);
      }

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      glBindTexture(GL_TEXTURE_2D, 0);
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

    // recalculate AABB
    {
    vec3 minPoint = (vec3){player.pos.x - player.w/2, player.pos.y, player.pos.z };
    vec3 maxPoint = (vec3){player.pos.x - player.w/2 + player.w, player.pos.y + player.h, player.pos.z + player.d };

    vec3 vert[8] = { minPoint, {minPoint.x + player.w, minPoint.y, minPoint.z},
    {minPoint.x, minPoint.y, minPoint.z + player.d},
    {minPoint.x + player.w, minPoint.y, minPoint.z + player.d}, maxPoint,
    {maxPoint.x - player.w, maxPoint.y, maxPoint.z},
    {maxPoint.x, maxPoint.y, maxPoint.z - player.d},
    {maxPoint.x - player.w, maxPoint.y, maxPoint.z - player.d}
    };

    player.min = vert[0];
    player.max = vert[0];
	
    for(int i=0; i<8;i++){
    vert[i] = matrixMultPoint(modelview, vert[i]);
	    
    player.min.x = min(player.min.x, vert[i].x);
    player.min.y = min(player.min.y, vert[i].y);
    player.min.z = min(player.min.z, vert[i].z);
	    
    player.max.x = max(player.max.x, vert[i].x);
    player.max.y = max(player.max.y, vert[i].y);
    player.max.z = max(player.max.z, vert[i].z);
    }
	
    glBegin(GL_LINES);

    glVertex3f(vert[0].x,vert[0].y,vert[0].z);
    glVertex3f(vert[1].x,vert[1].y, vert[1].z);

    glVertex3f(vert[1].x,vert[1].y,vert[1].z);
    glVertex3f(vert[2].x,vert[2].y,vert[2].z);

    glVertex3f(vert[0].x,vert[0].y,vert[0].z);
    glVertex3f(vert[2].x,vert[2].y,vert[2].z);
	
    glVertex3f(vert[2].x,vert[2].y,vert[2].z);
    glVertex3f(vert[3].x,vert[3].y,vert[3].z);

    glColor3d(blueColor);
    glVertex3f(player.min.x,player.min.y,player.min.z );
    glVertex3f(player.min.x,player.max.y,player.min.z );

    glColor3d(greenColor);
    glVertex3f(player.max.x,player.min.y,player.max.z );
    glVertex3f(player.max.x,player.max.y,player.max.z );

    glEnd();

    }
    }
    */
    
    // cursor world projection
    //      {
    /*
      vec3d start = {0};
      vec3d end = {0};

      double matModelView[16], matProjection[16];
      
      glGetIntegerv( GL_VIEWPORT, viewport );
      int viewport[4]; 
      glGetDoublev( GL_MODELVIEW_MATRIX, matModelView ); 
      glGetDoublev( GL_PROJECTION_MATRIX, matProjection ); 

      double winX = (double)mouse.screenPos.x; 
      double winY = viewport[3] - (double)mouse.screenPos.z; 

      gluUnProject(winX, winY, 0, matModelView, matProjection, 
      viewport, &start.x, &start.y, &start.z);

      gluUnProject(winX, winY,  1, matModelView, matProjection, 
      viewport, &end.x, &end.y, &end.z);


      mouse.start = vec3dToVec3(start);
      mouse.end = vec3dToVec3(end);*/
    //      }
		
    // renderCursor
    {
      Matrix orthProj = orthogonal(0.0f, windowW, 0.0f, windowH);
    
      glBegin(GL_LINES);

      float relWindowX = mouse.w * windowW;
      float relWindowY = mouse.h * windowH;

      glVertex2f(mouse.screenPos.x - relWindowX, mouse.screenPos.z + relWindowY);
      glVertex2f(mouse.screenPos.x + relWindowX, mouse.screenPos.z - relWindowY);

      glVertex2f(mouse.screenPos.x + relWindowX, mouse.screenPos.z + relWindowY);
      glVertex2f(mouse.screenPos.x - relWindowX, mouse.screenPos.z - relWindowY);

      glEnd();
    }
    /*
    
      glFlush();
    */

    mouse.clickL = false;
    mouse.clickR = false;
  
    SDL_GL_SwapWindow(window);

    uint32_t endtime = GetTickCount();
    uint32_t deltatime = endtime - starttime;

    if (!(deltatime > (1000 / FPS))) {
      Sleep((1000 / FPS) - deltatime);
    }
    
    if (deltatime != 0) {
      sprintf(windowTitle, game" FPS: %d", 1000 / deltatime);
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
//}

Model* loadOBJ(char* path, ModelName name){
  fastObjMesh* mesh = fast_obj_read(path);

  if (!mesh) {
    printf("Failed to load \"%s\" \n", path);
    exit(0);
  }

  Model* object = malloc(sizeof(Model));
  object->size = mesh->index_count;

  // vertecies
  vec3* verts = malloc(mesh->position_count * sizeof(vec3));
  object->vertices = malloc(mesh->index_count * sizeof(vec3));

  //  vec3 lb = { 1000, 1000, 1000 };
  //  vec3 rt = { 0,0,0 };
  
  int counter = 0;
  for (unsigned int i = 0; i < mesh->position_count * 3;i+=3){
    if(i==0) continue;

    verts[counter] = (vec3){mesh->positions[i], mesh->positions[i+1], mesh->positions[i+2]};

    /*
      lb.x = min(lb.x, verts[counter].x);
      lb.y = min(lb.y, verts[counter].y);
      lb.z = min(lb.z, verts[counter].z);
    
      rt.x = max(rt.x, verts[counter].x);
      rt.y = max(rt.y, verts[counter].y);
      rt.z = max(rt.z, verts[counter].z);
    */
    
    counter++;
  }
  

  // UVs
  uv2* uvs = malloc(mesh->texcoord_count * sizeof(uv2));
  uv2* sortedUvs = malloc(mesh->index_count * sizeof(uv2));
  
  counter = 0;
  for( unsigned int i=0; i<mesh->texcoord_count * 2;i+=2){
    if(i==0) continue;

    uvs[counter] = (uv2){mesh->texcoords[i], mesh->texcoords[i+1]};
    printf("%f %f \n", uvs[counter].x, uvs[counter].y);
    counter++;
  }

  // normals
  vec3* normals = malloc(mesh->normal_count * sizeof(vec3));
  vec3* sortedNormals = malloc(mesh->index_count * sizeof(vec3));

  counter = 0;
  for( unsigned int i=0; i<mesh->normal_count * 3;i+=3){
    if(i==0) continue;

    normals[counter] = (vec3){mesh->normals[i], mesh->normals[i+1], mesh->normals[i+2]};
    counter++;
  }
  
  // TODO: Make these 3 index_count loops in one
  for(int i=0; i< mesh->index_count; i++){
    sortedNormals[i] = normals[mesh->indices[i].n - 1];
    object->vertices[i] = verts[mesh->indices[i].p - 1];
    sortedUvs[i] = uvs[mesh->indices[i].t - 1];
  }
  
  free(uvs);
  free(verts);
  free(normals); 

  fast_obj_destroy(mesh);
  
  float* modelVerts = malloc(sizeof(float) * 8 * object->size);

  glGenVertexArrays(1, &object->VAO);
  glBindVertexArray(object->VAO);

  glGenBuffers(1, &object->VBO);
  glBindBuffer(GL_ARRAY_BUFFER, object->VBO);
  
  int index = 0;
  
  for(int i=0;i<object->size * 8;i+=8){
    modelVerts[i] = object->vertices[index].x;
    modelVerts[i + 1] = object->vertices[index].y;
    modelVerts[i + 2] = object->vertices[index].z;

    modelVerts[i + 3] = sortedUvs[index].x;
    modelVerts[i + 4] = sortedUvs[index].y;

    modelVerts[i + 5] = sortedNormals[index].x;
    modelVerts[i + 6] = sortedNormals[index].y;
    modelVerts[i + 7] = sortedNormals[index].z;

    index++;
  }

  for(int i=0;i<object->size * 8;i+=8){
    printf("%d vec %f %f %f uv %f %f norm %f %f %f \n",     i,modelVerts[i],     modelVerts[i+1],    modelVerts[i+2],     modelVerts[i+3],     modelVerts[i+4],     modelVerts[i+5],     modelVerts[i+6],     modelVerts[i+7]);
  }

  free(sortedNormals);
  free(sortedUvs);

  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * object->size, modelVerts, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  free(modelVerts);

  // set mat of object to IDENTITY_MATRIX
  object->mat.m[0] = 1;
  object->mat.m[5] = 1;
  object->mat.m[10] = 1;
  object->mat.m[15] = 1;

  scale(&object->mat.m, 0.25f, 0.25f, 0.25f);

  // To get some speed up i can getrid off one O(n) loop
  // simply finding min/max inside of parsing positions above
  calculateModelAABB(object);
  
  object->name = name;
  
  return object;
}

// it also assigns lb, rt to model
void calculateModelAABB(Model* model){
  model->lb = (vec3){1000,1000,1000};
  model->rt = (vec3){0,0,0};
  
  for (int i = 0; i < model->size; i++) {
    vec4 trasformedVert4 = mulmatvec4(model->mat, (vec4) { argVec3(model->vertices[i]), 1.0f });
    
    model->lb.x = min(model->lb.x, trasformedVert4.x);
    model->lb.y = min(model->lb.y, trasformedVert4.y);
    model->lb.z = min(model->lb.z, trasformedVert4.z);
    
    model->rt.x = max(model->rt.x, trasformedVert4.x);
    model->rt.y = max(model->rt.y, trasformedVert4.y);
    model->rt.z = max(model->rt.z, trasformedVert4.z);
  }
}
