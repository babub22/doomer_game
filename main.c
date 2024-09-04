#include "deps.h"
#include "linearAlg.h"
#include "main.h"
#include "editor.h"
#include "game.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

uint32_t skyboxVAO, skyboxVBO, cubemapTexture;

bool collisionToDraw = false;

MeshBuffer AABBBoxes;

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

GeomFin* finalGeom;
int* txLastIndex;

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

const void(*stancilHighlight[mouseSelectionTypesCounter])() = {
    [0] = noHighlighting,
    [mouseModelT] = modelHighlighting,
    [mouseBlockT] = noHighlighting,
    [mousePlaneT] = noHighlighting,
    [mouseTileT] = noHighlighting,
    [mouseLightT] = noHighlighting,
    [mouseMirrorT]= noHighlighting,
};

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

const char* wallTypeStr[] = {
    [normWallT] = "Wall",[RWallT] = "RWall", [LWallT] = "LWall",[LRWallT] = "LRWall",[windowT] = "Window",[doorT] = "Door"
};

const char* tileBlocksStr[] = { [roofBlockT] = "Roof",[stepsBlockT] = "Steps",[angledRoofT] = "Angle Roof" };

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

const char* sidesToStr[] = { [top] = "Top",[bot] = "Bot",[right] = "Right",[left] = "Left" };

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
    // remember tx of ground
   

    
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

    // markers VBO VAO
    for(int i=0;i<markersCounter;i++){
	glGenVertexArrays(1, &markersBufs[i].VAO);
	glBindVertexArray(markersBufs[i].VAO);

	glGenBuffers(1, &markersBufs[i].VBO);
	glBindBuffer(GL_ARRAY_BUFFER, markersBufs[i].VBO);

	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
    }

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

    // snow tiles mesh
    {
	for(int i=0;i< layersCounter;i++){
	    glGenBuffers(1, &snowTilesMesh[i].VBO);
	    glGenVertexArrays(1, &snowTilesMesh[i].VAO);

	    glBindBuffer(GL_ARRAY_BUFFER, 0);
	    glBindVertexArray(0);
	}
    }

    {
	glGenBuffers(1, &windowWindowsMesh.VBO);
	glGenVertexArrays(1, &windowWindowsMesh.VAO);
    }

    allocateAstar(gridX, gridY, gridZ);

    // 2d free rect
    {
	glGenBuffers(1, &dirPointerLine.VBO);
	glGenVertexArrays(1, &dirPointerLine.VAO);

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

    {
	glGenBuffers(1, &snowMesh.VBO);
	glGenVertexArrays(1, &snowMesh.VAO);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
    }

    {
	for(int i=0;i< layersCounter;i++){
	    glGenBuffers(1, &navigationTilesMesh[i].VBO);
	    glGenVertexArrays(1, &navigationTilesMesh[i].VAO);

	    glBindBuffer(GL_ARRAY_BUFFER, 0);
	    glBindVertexArray(0);
	}
    }

    // entities boxes
    for(int i=0;i<entityTypesCounter;i++){
	glGenBuffers(1, &entitiesBatch[i].VBO);
	glGenVertexArrays(1, &entitiesBatch[i].VAO);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
    }

    {
	glGenBuffers(1, &lastFindedPath.VBO);
	glGenVertexArrays(1, &lastFindedPath.VAO);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);        
    }

    {
	glGenBuffers(1, &selectedCollisionTileBuf.VBO);
	glGenVertexArrays(1, &selectedCollisionTileBuf.VAO);

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

    // init VBO and VAO for type
    {
	glGenBuffers(1, &objectsMenuTypeRectVBO);
	glGenVertexArrays(1, &objectsMenuTypeRectVAO);

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

    // setup proj mat for mirrors
    {
	mirrorProj = perspective(rad(45.0f), 1.0f, 0.01f, 1000.0f);
	//mirrorProj = orthogonal(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
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

    loadShaders();

    uniformFloat(mainShader, "screenW", fakeWinW);
    uniformFloat(mainShader, "screenH", fakeWinH);
    
    /* for (int i = 0; i < shadersCounter; i++) {
       int nameLen = strlen(shadersFileNames[i]) + 1;

       char* vertFileName = malloc(sizeof(char) * (nameLen + strlen(".vert") + 1));
       char* fragFileName = malloc(sizeof(char) * (nameLen + strlen(".frag") + 1));
       char* geomFileName = malloc(sizeof(char) * (nameLen + strlen(".geom") + 1));

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
       }*/

    //    glUseProgram(shadersId[mainShader]);
    //    glUniform2f(viewportLoc, windowW, windowH);
    //    uniformVec2(mainShader,"", (vec2){windowW, windowH})

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
	// main fbo
	{
	    
	    glGenFramebuffers(1, &fbo);
	    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	    glGenTextures(1, &textureColorBufferMultiSampled);
	    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
	    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, fakeWinW, fakeWinH, GL_TRUE);
	    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);

	    unsigned int rbo;
	    glGenRenderbuffers(1, &rbo);
	    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, fakeWinW, fakeWinH);
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

    {
	SDL_Surface* texture = IMG_Load_And_Flip_Vertical(texturesFolder"icedWindow.png");

	if (!texture) {
	    printf("Loading of texture \"%s\" failed", texturesFolder"icedWindow.png");
	    exit(-1);
	}

	createTexture(&windowGlassId, texture->w, texture->h, texture->pixels);  
	SDL_FreeSurface(texture);
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

	    loadedTextures2D[categoryIndex][index2D].w = texture->w;
	    loadedTextures2D[categoryIndex][index2D].h = texture->h;
      
	    loadedTextures1D[i].w = texture->w;
	    loadedTextures1D[i].h = texture->h;
      
	    SDL_FreeSurface(texture);

	    loadedTextures1D[i].tx = txId;
	    loadedTextures1D[i].index2D = index2D;
	    loadedTextures1D[i].index1D = i;
	    loadedTextures1D[i].categoryIndex = categoryIndex;
	}

	free(indexesTrackerFor2DTex);
    }
    {
	txOfGround = texture1DIndexByName("Zemlia1");
    }


//    loadFBXModel("./assets/Doomer.gltf");
    loadGLTFModel("./assets/objs/Doomer.gltf");
    
    glGenBuffers(1, &AABBBoxes.VBO);
    glGenVertexArrays(1, &AABBBoxes.VAO);
    
    loadGLTFScene("./assets/base.gltf");
    bindObjectsAABBBoxes();
    loadCubemap();


    // place player
    {
	Entity* entity = calloc(1,sizeof(Entity));
	entity->type = playerEntityT;

	// redo with specs
	entity->model = calloc(1,sizeof(ModelData));
	entity->model->data = &modelsData[0];

	entity->model->nodes = malloc(sizeof(GLTFNode)* entity->model->data->nodesSize);
	entity->model->tempTransforms = malloc(sizeof(float)* 10 * entity->model->data->nodesSize);
    
//	printf("transf: %d nodes: %d \n",
//	       sizeof(float)* 10 * entity->model->data->nodesSize ,
//	       sizeof(GLTFNode)* entity->model->data->nodesSize);
    
	memcpy(entity->model->nodes,
	       entity->model->data->nodes,
	       sizeof(GLTFNode)* entity->model->data->nodesSize);

	entity->model->jointsMats = malloc(sizeof(Matrix)*entity->model->data->jointsIdxsSize);
    
	entity->mat = IDENTITY_MATRIX;
	
	entity->mat.m[12] = .0f;
	entity->mat.m[13] = 5.0f;
	entity->mat.m[14] = .0f;

	int* tempAABB = malloc(sizeof(int) * aabbEntitiesSize);
	int tempAABBSize = 0;

	vec4 playerPoint = { entity->mat.m[12], entity->mat.m[13], entity->mat.m[14], 1.0f};

	{
	    for(int i=0;i<aabbEntitiesSize;++i){
		if(playerPoint.x > aabbEntities[i].col.lb.x && playerPoint.x < aabbEntities[i].col.rt.x &&
		   playerPoint.y > aabbEntities[i].col.lb.y && playerPoint.y < aabbEntities[i].col.rt.y &&
		   playerPoint.z > aabbEntities[i].col.lb.z && playerPoint.z < aabbEntities[i].col.rt.z)
		{
		    tempAABB[tempAABBSize] = i;
		    tempAABBSize++;
		}
	    }

	    if(tempAABBSize != 0){
		for(int i=0;i<tempAABBSize;++i){
		    Object obj = (*(aabbEntities[tempAABB[i]].buf))[aabbEntities[tempAABB[i]].bufId];
		    int infoId = obj.infoId;
		    
		    Matrix inversed;

		    inverse(obj.mat.m, inversed.m);
		    vec4 invPlayerPoint = mulmatvec4(inversed, playerPoint);
		    vec2 player2DPos = {invPlayerPoint.x, invPlayerPoint.z};// no height

		    for(int i2=0;i2<objectsInfo[infoId].meshesSize;++i2){
			for(int i3=0;i3<objectsInfo[infoId].meshes[i2].posBufSize/9; i3++) {
			    int index = i3*3*3; // 9

			    vec2 a = {objectsInfo[infoId].meshes[i2].posBuf[index+0],objectsInfo[infoId].meshes[i2].posBuf[index+2]};
			    vec2 b = {objectsInfo[infoId].meshes[i2].posBuf[index+3],objectsInfo[infoId].meshes[i2].posBuf[index+5]};
			    vec2 c = {objectsInfo[infoId].meshes[i2].posBuf[index+6],objectsInfo[infoId].meshes[i2].posBuf[index+8]};

			    int as_x = player2DPos.x - a.x;
			    int as_y = player2DPos.z - a.z;

			    bool s_ab = (b.x - a.x) * as_y - (b.z - a.z) * as_x > 0;
			    bool inside = true;

			    if ((c.x - a.x) * as_y - (c.z - a.z) * as_x > 0 == s_ab) 
				inside = false;
			    if ((c.x - b.x) * (player2DPos.z - b.z) - (c.z - b.z)*(player2DPos.x - b.x) > 0 != s_ab) 
				inside = false;

			    printf("Tri %d -  v1(%f %f %f) v2(%f %f %f) v3(%f %f %f)\n", i3,
				   objectsInfo[infoId].meshes[i2].posBuf[index+0], objectsInfo[infoId].meshes[i2].posBuf[index+1],
				   objectsInfo[infoId].meshes[i2].posBuf[index+2], 
				
					objectsInfo[infoId].meshes[i2].posBuf[index+3],
				   objectsInfo[infoId].meshes[i2].posBuf[index+4], objectsInfo[infoId].meshes[i2].posBuf[index+5],

				   objectsInfo[infoId].meshes[i2].posBuf[index+6], objectsInfo[infoId].meshes[i2].posBuf[index+7],
				   objectsInfo[infoId].meshes[i2].posBuf[index+8]);

			    if(inside){
				printf("\nIntersected with: v1(%f %f %f) v2(%f %f %f) v3(%f %f %f)\n",
					objectsInfo[infoId].meshes[i2].posBuf[index + 0], objectsInfo[infoId].meshes[i2].posBuf[index + 1],
					objectsInfo[infoId].meshes[i2].posBuf[index + 2],

					objectsInfo[infoId].meshes[i2].posBuf[index + 3],
					objectsInfo[infoId].meshes[i2].posBuf[index + 4], objectsInfo[infoId].meshes[i2].posBuf[index + 5],

					objectsInfo[infoId].meshes[i2].posBuf[index + 6], objectsInfo[infoId].meshes[i2].posBuf[index + 7],
					objectsInfo[infoId].meshes[i2].posBuf[index + 8]);
				
				printf("Player pos: v1(%f %f) \n", player2DPos.x, player2DPos.z);

//				float avgH = (objectsInfo[infoId].meshes[i2].posBuf[index+1] + objectsInfo[infoId].meshes[i2].posBuf[index+4] + objectsInfo[infoId].meshes[i2].posBuf[index+7]) / 3.0f;

				float avgH = min(min(objectsInfo[infoId].meshes[i2].posBuf[index+1], objectsInfo[infoId].meshes[i2].posBuf[index+4]), objectsInfo[infoId].meshes[i2].posBuf[index+7]);

				playerPoint = (vec4){player2DPos.x, avgH, player2DPos.z, 1.0f};
				playerPoint = mulmatvec4(obj.mat, playerPoint);
				
				break;
			    }
			    
			    objectsInfo[infoId].meshes[i2].posBuf;

			}
		    }
		}
	    }
	}

	entity->mat.m[12] = playerPoint.x;
	entity->mat.m[13] = playerPoint.y;
	entity->mat.m[14] = playerPoint.z;
    
	entity->dir = (vec3){ .0f, .0f, 1.0f};

	entityStorageSize[entity->type]++;

	if(!entityStorage[entity->type]){
	    entityStorage[entity->type] = malloc(sizeof(Entity));
	}

	memcpy(&entityStorage[entity->type][entityStorageSize[entity->type]-1], entity, sizeof(Entity));
    }
    
    // load 3d models
    /*{313
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
      }*/

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

//    Geom* preGeom = malloc(sizeof(Geom) * loadedTexturesCounter);
    txLastIndex = calloc(loadedTexturesCounter, sizeof(int));
    finalGeom = malloc(sizeof(GeomFin) * loadedTexturesCounter);
    
    for (int i = 0; i < loadedTexturesCounter; i++) {
	glGenVertexArrays(1, &finalGeom[i].VAO);
	glGenBuffers(1, &finalGeom[i].VBO);
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

    initSnowParticles();

    const float entityH = 0.17f;
    const float entityW = 0.1f / 2.0f;
    const float entityD = entityH / 6;

    vec3 initPos = { 0.3f + 0.1f / 2, 0.0f, 0.3f + 0.1f / 2 }; //+ 0.1f/2 - (entityD * 0.75f)/2 };

    bool quit = false;

    clock_t lastFrame = clock();
    float deltaTime;

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
	Uint32 start_time = SDL_GetTicks();

	glErrorCheck();
      
	uint32_t starttime = SDL_GetPerformanceCounter();//GetTickCount();

	clock_t currentFrame = clock();
	deltaTime = (double)(currentFrame - lastFrame) / CLOCKS_PER_SEC;
	lastFrame = currentFrame;

	// cameraSpeed = 10.0f * deltaTime;


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
			generateNavTiles();
		
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

	mouse.tileSide = -1;

	((void (*)(int))instances[curInstance][mouseVSFunc])(mainShader);    
	((void (*)(float))instances[curInstance][preFrameFunc])(deltaTime);
    
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

		    entityStorage[playerEntityT][0].model->time += deltaTime;
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
		timeAcc += elapsedMs;
		
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
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
		glBindVertexArray(AABBBoxes.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, AABBBoxes.VBO);

		Matrix out = IDENTITY_MATRIX;
		uniformMat4(lightSourceShader, "model", out.m);
		uniformVec3(lightSourceShader, "color", (vec3) { greenColor });
		    
		glUseProgram(shadersId[lightSourceShader]);
		glDrawArrays(GL_TRIANGLES, 0, AABBBoxes.VBOsize);
	    
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
	    {
		static float dMove = 0.0005f;

		int index = 0;
		for (int i = 0; i < snowParticles.size; i += 2) {
		    if (snowParticles.buf[i].y > 0) {
			snowParticles.buf[i].y -= 0.0025f;
			snowParticles.buf[i + 1].y -= 0.0025f;
		    }
		    else {
			if (snowParticles.action[index] != WAITING_Y) {
			    snowParticles.timers[index] = 360;
			    snowParticles.action[index] = WAITING_Y;
			}
		    }

		    if (snowParticles.timers[index] == 0) {
			if (snowParticles.action[index] == WAITING_Y) {
			    snowParticles.buf[i].y = (gridY * 2) - 0.025f;
			    snowParticles.buf[i + 1].y = gridY * 2;

			    snowParticles.action[index] = rand() % snowDirCounter;
			}
			else {
			    snowParticles.speeds[index] = (rand() % 6) / 10000.0f;
			    float maxDist;

			    snowParticles.action[index] = oppositeSnowDir[snowParticles.action[index]];

			    if (snowParticles.action[index] == X_PLUS_SNOW) {
				maxDist = ((int)snowParticles.buf[i].x + 1) - snowParticles.buf[i].x;
			    }
			    else if (snowParticles.action[index] == X_MINUS_SNOW) {
				maxDist = snowParticles.buf[i].x - (int)snowParticles.buf[i].x;
			    }
			    else if (snowParticles.action[index] == Z_MINUS_SNOW) {
				maxDist = snowParticles.buf[i].z - (int)snowParticles.buf[i].z;
			    }
			    else if (snowParticles.action[index] == Z_PLUS_SNOW) {
				maxDist = ((int)snowParticles.buf[i].z + 1) - snowParticles.buf[i].z;
			    }

			    maxDist -= snowParticles.speeds[index];

			    snowParticles.timers[index] = maxDist / dMove;
			}
		    }
		    else {
			snowParticles.timers[index]--;

			if (snowParticles.action[index] != WAITING_Y) {
			    float dX = 0.0f;
			    float dZ = 0.0f;

			    if (snowParticles.action[index] == X_PLUS_SNOW) {
				dX = snowParticles.speeds[index];
			    }
			    else if (snowParticles.action[index] == X_MINUS_SNOW) {
				dX = -snowParticles.speeds[index];
			    }
			    else if (snowParticles.action[index] == Z_MINUS_SNOW) {
				dZ = -snowParticles.speeds[index];
			    }
			    else if (snowParticles.action[index] == Z_PLUS_SNOW) {
				dZ = snowParticles.speeds[index];
			    }

			    snowParticles.buf[i].x += dX;
			    snowParticles.buf[i + 1].x += dX;

			    snowParticles.buf[i].z += dZ;
			    snowParticles.buf[i + 1].z += dZ;
			}
		    }

		    index++;
		}

		glBindVertexArray(snowMesh.VAO); 
		glBindBuffer(GL_ARRAY_BUFFER, snowMesh.VBO);

		glBufferData(GL_ARRAY_BUFFER,
			     sizeof(vec3) * snowParticles.size, snowParticles.buf, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	    }
	    
	    // nav meshes drawing
	    glUseProgram(shadersId[lightSourceShader]);
	    
	    {
		glBindBuffer(GL_ARRAY_BUFFER, lastFindedPath.VBO);
		glBindVertexArray(lastFindedPath.VAO);

		Matrix out = IDENTITY_MATRIX;
		uniformMat4(lightSourceShader, "model", out.m);
		uniformVec3(lightSourceShader, "color", (vec3) { redColor });
		    
		glDrawArrays(GL_LINES, 0, lastFindedPath.VBOsize);
	    }
	    
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
    
	/*if (deltatime != 0) {
	  sprintf(windowTitle, game" BazoMetr: %d%% Save: %s.doomer", 1000 / deltatime, curSaveName); 
	  SDL_SetWindowTitle(window, windowTitle);
	  }*/

	float delta = SDL_GetTicks()-start_time;
        elapsedMs = delta / 1000.0f;
	//printf("Frame ms: %f \n", elapsedMs);
	
	if((1000/FPS)>delta){
            SDL_Delay((1000/FPS)-(SDL_GetTicks()-start_time));
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
    //  resetMouse();

    printf("Start\n");
  
    char* save = calloc((strlen(saveName) + strlen(".doomer") + 2), sizeof(char));

    strcat(save, saveName);
    strcat(save, ".doomer");

    FILE* map = fopen(save, "r");

    printf("base\n");

    if (!map) {
	free(save);
	return false;
    }

    for(int i=0;i<curModelsSize;i++){
	//  destroyCharacter(curModels[i].characterId);
    }

    for(int i=0;i<wallsStorageSize;i++){
	free(wallsStorage[i]->planes);
	free(wallsStorage[i]);
    }

    for(int i=0;i<blocksStorageSize;i++){
	free(blocksStorage[i]);
    }

    if(curModels){
	free(curModels);
    }

    if(wallsStorage){
	free(wallsStorage);
    }

    if(blocksStorage){
	free(blocksStorage);
    }

    if(picturesStorage){
	free(picturesStorage);
    }

    if(tilesStorage){
	free(tilesStorage);
    }

    memset(geomentyByTxCounter, 0, loadedTexturesCounter * sizeof(size_t));

    for(int i=0;i<lightsTypeCounter;i++){
	free(lightStorage[i]);
    }

    int gX, gY, gZ;
  
    fscanf(map, "%d %d %d \n", &gX, &gY, &gZ);

    allocateGrid(gX, gY, gZ);
  
    fscanf(map, "tiles: %d walls: %d blocks: %d pictures: %d models: %d \n",
	   &tilesStorageSize,
	   &wallsStorageSize,
	   &blocksStorageSize,
	   &picturesStorageSize,
	   &curModelsSize);

    tilesStorage = calloc(1, sizeof(Tile) * tilesStorageSize);
    wallsStorage = malloc(sizeof(Wall*) * wallsStorageSize);
    blocksStorage = malloc(sizeof(TileBlock*) * blocksStorageSize);
    picturesStorage = malloc(sizeof(Picture) * picturesStorageSize);
    curModels = malloc(sizeof(Model) * curModelsSize);


    // walls
    for (int i = 0; i < wallsStorageSize; i++) { 
	Wall* newWall = malloc(sizeof(Wall));
	int sideMat;
	vec3 pos;

	fscanf(map, "%d %d %d %d (%f %f %f) ", &sideMat, &newWall->side, &newWall->type, &newWall->prevType, &pos.x, &pos.y, &pos.z);

	newWall->sideForMat = sideMat;

	newWall->planes = malloc(sizeof(Plane) * wallsVPairs[newWall->prevType].planesNum); 

	memcpy(newWall->mat.m, hardWallMatrices[newWall->sideForMat].m, sizeof(float) * 16);

	newWall->mat.m[12] += pos.x;
	newWall->mat.m[13] = pos.y;
	newWall->mat.m[14] += pos.z;

	for(int i2=0;i2<wallsVPairs[newWall->prevType].planesNum;i2++){ 
	    fscanf(map, "%d ", &newWall->planes[i2].txIndex);
	    geomentyByTxCounter[newWall->planes[i2].txIndex] += wallsVPairs[newWall->prevType].pairs[i2].vertexNum * sizeof(float) * wallsVPairs[newWall->prevType].pairs[i2].attrSize;
	    calculateAABB(newWall->mat, wallsVPairs[newWall->prevType].pairs[i2].vBuf, wallsVPairs[newWall->prevType].pairs[i2].vertexNum, wallsVPairs[newWall->prevType].pairs[i2].attrSize, &newWall->planes[i2].lb, &newWall->planes[i2].rt);
	}
	placedWallCounter[newWall->type]++;

	newWall->id = i;
	wallsStorage[i] = newWall;

	fscanf(map, "\n");
    }

    // blocks
    for (int i = 0; i < blocksStorageSize; i++) {
	TileBlock* newBlock = malloc(sizeof(TileBlock));
    
	fscanf(map, "%d %d %d ", &newBlock->txIndex, &newBlock->rotateAngle, &newBlock->type);

	for(int i2=0;i2<16;i2++){
	    fscanf(map, "%d ", &newBlock->mat.m[i2]);
	}

	newBlock->id = i;
	blocksStorage[i] = newBlock;

	geomentyByTxCounter[newBlock->txIndex] += blocksVPairs[newBlock->type].pairs[0].vertexNum * sizeof(float) * blocksVPairs[newBlock->type].pairs[0].attrSize;
	calculateAABB(newBlock->mat, blocksVPairs[newBlock->type].pairs[0].vBuf, blocksVPairs[newBlock->type].pairs[0].vertexNum, blocksVPairs[newBlock->type].pairs[0].attrSize, &newBlock->lb, &newBlock->rt);
      
	fscanf(map, "\n");
    }

    // tiles
    for (int i = 0; i < tilesStorageSize; i++) {
	int tileTx;
	fscanf(map, "(%f %f %f) %d ", &tilesStorage[i].pos.x, &tilesStorage[i].pos.y, &tilesStorage[i].pos.z, &tileTx);

	tilesStorage[i].tx = (int8_t)tileTx;
	tilesStorage[i].id = i;

	geomentyByTxCounter[tilesStorage[i].tx] += sizeof(float) * 8 * 6;    

	int blockId;
	int blockTx;
    
	fscanf(map, "%d %d ", &blockId, &blockTx);

	if(blockId != -1){
	    tilesStorage[i].block = blocksStorage[blockId]; 
	    tilesStorage[i].block->txIndex = blockTx;
	    tilesStorage[i].block->tileId = i;
	}

	int tId;
	fscanf(map, "%d ", &tId);
    
	if(tId != -1){
	    tilesStorage[i].wall[top] = wallsStorage[tId];
	    tilesStorage[i].wall[top]->tileId = i;
	}

	int lId;
	fscanf(map, "%d ", &lId);
    
	if(lId != -1){
	    tilesStorage[i].wall[left] = wallsStorage[lId];
	    tilesStorage[i].wall[left]->tileId = i;
	}

	fscanf(map, "\n");
    }

    // pictures
    for (int i = 0; i < picturesStorageSize; i++) {
	int charId;
	fscanf(map, "%d %d ", &picturesStorage[i].txIndex, &charId); 
	picturesStorage[i].characterId = -1;

	for(int i2=0;i2<16;i2++){
	    fscanf(map, "%f ", &picturesStorage[i].mat.m[i2]);
	}

	picturesStorage[i].id = i;

	geomentyByTxCounter[picturesStorage[i].txIndex] += 6 * 8 * sizeof(float);
	calculateAABB(picturesStorage[i].mat,
		      planePairs.vBuf, planePairs.vertexNum, planePairs.attrSize,
		      &picturesStorage[i].lb, &picturesStorage[i].rt);
      
	fscanf(map, "\n");
    }
    //exit(1488);

    // cur models
    for (int i = 0; i < curModelsSize; i++) {
	int charId;
	fscanf(map, "%d %d ", &curModels[i].name, &charId);
	curModels[i].characterId = -1;

	for(int i2=0;i2<16;i2++){
	    fscanf(map, "%f ", &curModels[i].mat.m[i2]);
	}

	curModels[i].id = i;

	calculateModelAABB(&curModels[i]);
      
	fscanf(map, "\n");
    }

    // lights
    for(int i2=0;i2<lightsTypeCounter;i2++){
	char buf[100];
	fscanf(map, "%s - %d ", buf, &lightStorageSizeByType[i2]);
	fscanf(map, "\n");

	printf("%s %d \n", buf, lightStorageSizeByType[i2]);

	if (lightStorageSizeByType[i2] == 0) {
	    continue;
	}
    
	lightStorage[i2] = malloc(sizeof(Light) * lightStorageSizeByType[i2]);
      
	for(int i=0;i<lightStorageSizeByType[i2];i++){
	    int offValue;
	    vec3i color;
      
	    fscanf(map, "c(%d %d %d) %d %f %f %d ", &color.x, &color.y, &color.z, &offValue, &lightStorage[i2][i].rad, &lightStorage[i2][i].cutOff, &lightStorage[i2][i].curLightPresetIndex);
      
	    lightStorage[i2][i].off = (bool)offValue;
	    lightStorage[i2][i].r = color.x;
	    lightStorage[i2][i].g = color.y;
	    lightStorage[i2][i].b = color.z;

	    lightStorage[i2][i].type = i2;
	
	    for(int i3=0;i3<16;i3++){
		fscanf(map, "%f ", &lightStorage[i2][i].mat.m[i3]);
	    }

	    calculateAABB(lightStorage[i2][i].mat, cube.vBuf, cube.vertexNum, cube.attrSize, &lightStorage[i2][i].lb, &lightStorage[i2][i].rt);

	    fscanf(map, "\n");
	}
    }

    uniformLights();

    if(lightStorageSizeByType[dirLightShadowT] != 0){
	rerenderShadowsForAllLights();
    }

    
    batchModels();

    printf("Save %s loaded! \n", save);  
    fclose(map);

    strcpy(curSaveName, saveName);
    free(save);
  
    return true;
}

bool saveMap(char* saveName) {
    char* save = calloc((strlen(saveName) + strlen(".doomer") + 2), sizeof(char));

    strcat(save, saveName);
    strcat(save, ".doomer");

    FILE* map = fopen(save, "w+");

    fprintf(map, "%d %d %d \n", gridY, gridZ, gridX);
    
    fprintf(map, "tiles: %d walls: %d blocks: %d pictures: %d models: %d \n",
	    tilesStorageSize,
	    wallsStorageSize,
	    blocksStorageSize,
	    picturesStorageSize,
	    curModelsSize);

    for (int i = 0; i < wallsStorageSize; i++) {
	fprintf(map, "%d %d %d %d (%f %f %f) ", wallsStorage[i]->sideForMat, wallsStorage[i]->side, wallsStorage[i]->type, wallsStorage[i]->prevType, argVec3(tilesStorage[wallsStorage[i]->tileId].pos));

	for(int i2=0;i2<wallsVPairs[wallsStorage[i]->prevType].planesNum;i2++){
	    fprintf(map, "%d ", wallsStorage[i]->planes[i2].txIndex);
	}

	fprintf(map, "\n");
    }
    
    for (int i = 0; i < blocksStorageSize; i++) {
	fprintf(map, "%d %d %d ", blocksStorage[i]->txIndex, blocksStorage[i]->rotateAngle, blocksStorage[i]->type);

	for(int i2=0;i2<16;i2++){
	    fprintf(map, "%d ", blocksStorage[i]->mat.m[i2]);
	}

	fprintf(map, "\n");
    }

    for (int i = 0; i < tilesStorageSize; i++) {
	fprintf(map, "(%f %f %f) %d ", argVec3(tilesStorage[i].pos), tilesStorage[i].tx);

	if(tilesStorage[i].block){
	    fprintf(map, "%d %d ", tilesStorage[i].block->id, tilesStorage[i].block->txIndex);
	}else{
	    fprintf(map, "%d %d ", -1, -1);
	}

	if(tilesStorage[i].wall[top]){
	    fprintf(map, "%d ", tilesStorage[i].wall[top]->id);
	}else{
	    fprintf(map, "%d ", -1);
	}

	if(tilesStorage[i].wall[left]){
	    fprintf(map, "%d ", tilesStorage[i].wall[left]->id);
	}else{
	    fprintf(map, "%d ", -1);
	}

	fprintf(map, "\n");
    }


    for (int i = 0; i < picturesStorageSize; i++) {
	fprintf(map, "%d %d ", picturesStorage[i].txIndex, picturesStorage[i].characterId);

	for(int i2=0;i2<16;i2++){
	    fprintf(map, "%f ", picturesStorage[i].mat.m[i2]);
	}
      
	fprintf(map, "\n");
    }
 
    for (int i = 0; i < curModelsSize; i++) {
	fprintf(map, "%d %d ", curModels[i].name, -1);

	for(int i2=0;i2<16;i2++){
	    fprintf(map, "%f ", curModels[i].mat.m[i2]);
	}
      
	fprintf(map, "\n");
    }

    for(int i2=0;i2<lightsTypeCounter;i2++){
	fprintf(map, "%s - %d ", lightTypesStr[i2], lightStorageSizeByType[i2]);
	fprintf(map, "\n");
      
	for(int i=0;i<lightStorageSizeByType[i2];i++){
	    fprintf(map, "c(%d %d %d) %d %f %f %d ", lightStorage[i2][i].r, lightStorage[i2][i].g, lightStorage[i2][i].b, lightStorage[i2][i].off, lightStorage[i2][i].rad, lightStorage[i2][i].cutOff, lightStorage[i2][i].curLightPresetIndex);
	
	    for(int i3=0;i3<16;i3++){
		fprintf(map, "%f ", lightStorage[i2][i].mat.m[i3]);
	    }

	    fprintf(map, "\n");
	}
    }
    
    /*    for (int i = 0; i < blocksStorageSize; i++) {
	  fprintf(map, "%d %d %d %d ", blocksStorage[i]->tileId, wallsStorage[i]->type, wallsStorage[i]->prevType, wallsStorage[i]->tileId);

	  for(int i2=0;i2<wallsVPairs[wallsStorage[i]->type].planesNum;i2++){
	  fprintf(map, "%d ", wallsStorage[i]->planes[i2].txIndex);
	  }

	  fprintf(map, "\n");
	  }*/

    
    /*
      for (int i = 0; i < blocksStorageSize; i++) {

      }
    

      for (int i = 0; i < 0; i++) {
      fprintf(map, "\n");

      int x = 0;// wallsIndexes[i].x;
      int y = 0;//wallsIndexes[i].y;
      int z = 0;//wallsIndexes[i].z;

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

      //free(wallsIndexes);

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
    */
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

//    txOfGround = texture1DIndexByName("Zemlia1");

    if (txOfGround == -1) {
	printf("Specify texture of ground"); 
	exit(-1);
    }

    gridX = newX;
    gridY = newY;
    gridZ = newZ;
  
    defaultGrid(gridZ, gridY, gridZ);
  
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

	vec3 tile = tilesStorage[tileData->tileId].pos;// xyz_indexesToCoords(tileData->grid.x, curFloor, tileData->grid.z);
    
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

float* wallBySide(int* size,Side side, float thick){
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

	*size = sizeof(verts);
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

	*size = sizeof(verts);
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

void allocateCollisionGrid(int gX, int gY, int gZ){
    if(collisionGrid){
	for (int y = 0; y < gridY*3; y++) {
	    for (int z = 0; z < gridZ*3; z++) {
		free(collisionGrid[y][z]);
	    }
      
	    free(collisionGrid[y]);
	}
    
	free(collisionGrid);
    }
    
    collisionGrid = malloc(sizeof(uint8_t**) * gY);

    for (int y = 0; y < gY; y++) {
	collisionGrid[y] = malloc(sizeof(uint8_t*) * gZ *3);

	for (int z = 0; z < gZ * 3; z++) {
	    collisionGrid[y][z] = calloc(sizeof(uint8_t), gX * 3);
	}
    }
}

void allocateGrid(int gX, int gY, int gZ){ 
    if(grid){
	for (int y = 0; y < gridY; y++) {
	    for (int z = 0; z < gridZ; z++) {
		free(grid[y][z]);
	    }
      
	    free(grid[y]);
	}
    
	free(grid);
	grid = NULL;
    }
    
    grid = malloc(sizeof(Tile***) * gY);

    for (int y = 0; y < gY; y++) {
	grid[y] = malloc(sizeof(Tile**) * gZ);

	for (int z = 0; z < gZ; z++) {
	    grid[y][z] = calloc(gX, sizeof(Tile*));
	}
    }

    gridX = gX;
    gridY = gY;
    gridZ = gZ;
}

void defaultGrid(int gX, int gY, int gZ){
//    txOfGround = texture1DIndexByName("Zemlia1");

    if (txOfGround == -1) {
	printf("Specify texture of ground");
	exit(-1);
    }

    tilesStorage = malloc(sizeof(Tile) * gX * gZ);
    tilesStorageSize = 0;
  
    for (int y = 0; y < gY; y++) {
	for (int z = 0; z < gZ; z++) {
	    for (int x = 0; x < gX; x++) {
		if (y == 0) {
		    vec3 tile = xyz_indexesToCoords(x,y,z);
		    //	  printf("%f %f %f \n", argVec3(tile));

		    tilesStorage[tilesStorageSize].tx = txOfGround;
		    tilesStorage[tilesStorageSize].pos = tile;
		    tilesStorage[tilesStorageSize].id = tilesStorageSize;

		    tilesStorage[tilesStorageSize].block = NULL;
		    tilesStorage[tilesStorageSize].wall[top] = NULL;
		    tilesStorage[tilesStorageSize].wall[left] = NULL;

		    geomentyByTxCounter[txOfGround] += sizeof(float) * 8 * 6;
	  
		    // grid[y][z][x] = &tilesStorage[tilesStorageSize];
		    tilesStorageSize++;
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
//	    float pos[3];
	    
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
					 [cgltf_animation_path_type_weights] = "Weithg",[cgltf_animation_path_type_weights+1] = "ERROR" };


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

		assert(data->nodes[nodeIndex].mesh->primitives[i2].attributes->type == cgltf_attribute_type_position);
		objectsInfo[i].meshes[i2].posBuf = malloc(data->nodes[nodeIndex].mesh->primitives[i2].attributes->data->buffer_view->size);
		objectsInfo[i].meshes[i2].posBufSize = data->nodes[nodeIndex].mesh->primitives[i2].attributes->data->buffer_view->size / sizeof(float);

		fseek(fo, data->nodes[nodeIndex].mesh->primitives[i2].attributes->data->buffer_view->offset, SEEK_SET);
		fread(objectsInfo[i].meshes[i2].posBuf, data->nodes[nodeIndex].mesh->primitives[i2].attributes->data->buffer_view->size, 1, fo);

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

/*
    loadedMeshesIndxSize = 0;
    for(int i=0;i<data->nodes_count;++i){
	Matrix transformMat;
	
	{
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

	    if (data->nodes[i].has_matrix) {
		printf("has mat\n");
	    }
	    
	    transformMat = gltfTRS(t);
	}

	//data->nodes[i]

	
	if(data->nodes[i].light){
		
	    vec3 color = { data->nodes[i].light->color[0], data->nodes[i].light->color[1], data->nodes[i].light->color[2] };
	    createLight(color,
			data->nodes[i].light->type,
			data->nodes[i].light->intensity, transformMat.m);
	    continue;
	}
	
	char* token = strtok(data->nodes[i].name, ".");

	Object** targetBuffer = NULL;
	int* targetBufferSize = NULL;
	
	if(strcmp(token, "water") == 0){
	    targetBuffer = &waterSurfaces;
	    targetBufferSize = &waterSurfacesSize;
	}else{
	    targetBuffer = &objects;
	    targetBufferSize = &objectsSize;
	}

	int targetSize = *targetBufferSize;
	
	if(!(*targetBuffer)){
	    *targetBuffer = malloc(sizeof(Object));
	}else{
	    *targetBuffer = realloc(*targetBuffer, sizeof(Object) * ((targetSize)+1));
	}

	(*targetBuffer)[targetSize].mat = transformMat;

	//for(int i=0;i<)

//	int attrLen = data->nodes[i].att;
	int idxLen = data->nodes[i].mesh->primitives->indices->count;

	bool exist = false;
	int idInInfo = -1;
	for(int i2=0;i2< loadedMeshesIndxSize;++i2){
	    if(loadedMeshesIndx[i2] == idxLen) {
		idInInfo = i2;
		exist = true;
		break;
	    }
	}

	if(!exist){
	    loadedMeshesIndx[loadedMeshesIndxSize] = idxLen;
	    loadedMeshesIndxSize++;

	    if(!objectsInfo){
		objectsInfo = malloc(sizeof(ObjectInfo));
	    }else{
		objectsInfo = realloc(objectsInfo,
				      sizeof(ObjectInfo) * (objectsInfoSize +1));
	    }

	    objectsInfo[objectsInfoSize].name = malloc(sizeof(char) * (strlen(token)+1));
	    strcpy(objectsInfo[objectsInfoSize].name, token);

	    {
		objectsInfo[objectsInfoSize].meshesSize = data->nodes[i].mesh->primitives_count;
		objectsInfo[objectsInfoSize].meshes = malloc(sizeof(Mesh) * objectsInfo[objectsInfoSize].meshesSize);

		for(int i2=0;i2<objectsInfo[objectsInfoSize].meshesSize;++i2){
		    size_t attrPad = 0;

		    for(int i4=0;i4<data->nodes[i].mesh->primitives[i2].attributes_count;++i4){			   
			int vecLen = GLTFmapSize[data->nodes[i].mesh->primitives[i2].attributes[i4].data->type];
			attrPad += vecLen;
		    }

		    sceneWeight += attrPad * data->nodes[i].mesh->primitives[i2].indices->count;

		    printf("Attr size: %d \n", attrPad);

		    int maxUsedSize = 0;

		    int indexSize = GLTFtypeSize[data->nodes[i].mesh->primitives[i2].indices->component_type];
		    for (int i3 = 0; i3 < data->nodes[i].mesh->primitives[i2].indices->count;++i3) {
			uint16_t index;
			fseek(fo, data->nodes[i].mesh->primitives[i2].indices->buffer_view->offset + i3 * indexSize, SEEK_SET);
			fread(&index, indexSize, 1, fo);

			for(int i4=0;i4<data->nodes[i].mesh->primitives[i2].attributes_count;++i4){
			    int compSize = GLTFtypeSize[data->nodes[i].mesh->primitives[i2].attributes[i4].data->component_type];
			    int vecLen = GLTFmapSize[data->nodes[i].mesh->primitives[i2].attributes[i4].data->type];

			    for(int i5=0;i5<vecLen;++i5){
				fseek(fo, data->nodes[i].mesh->primitives[i2].attributes[i4].data->buffer_view->offset
				      + compSize*(vecLen * index + i5), SEEK_SET);

				int index = (i3*attrPad)+i5+GLTFattrPad[data->nodes[i].mesh->primitives[i2].attributes[i4].type];
		
				if(data->nodes[i].mesh->primitives[i2].attributes[i4].data->component_type
				   !=cgltf_component_type_r_32f){
				    uint8_t temp;
				    fread(&temp, compSize, 1, fo);
		    
				    mesh[index] = (float)temp;
				}else{
				    fread(&mesh[index], compSize, 1, fo);
				}
			    }
			}
		    }
		    
		    glGenVertexArrays(1, &objectsInfo[objectsInfoSize].meshes[i2].VAO);
		    glGenBuffers(1, &objectsInfo[objectsInfoSize].meshes[i2].VBO);

		    glBindVertexArray(objectsInfo[objectsInfoSize].meshes[i2].VAO);
		    glBindBuffer(GL_ARRAY_BUFFER, objectsInfo[objectsInfoSize].meshes[i2].VBO);
    
		    objectsInfo[objectsInfoSize].meshes[i2].VBOSize = data->nodes[i].mesh->primitives[i2].indices->count;
		    glBufferData(GL_ARRAY_BUFFER, attrPad * data->nodes[i].mesh->primitives[i2].indices->count * sizeof(float)
				 , mesh, GL_STATIC_DRAW);

		    int itemPad[] = { 3, 2, 3, 4, 4};

		    int pad = 0;
		    for(int i3=0;i3<data->nodes[i].mesh->primitives[i2].attributes_count;++i3){
			glVertexAttribPointer(i3, itemPad[i3], GL_FLOAT, GL_FALSE, attrPad * sizeof(float), pad * sizeof(float));
			glEnableVertexAttribArray(i3);

			pad+= itemPad[i3];
		    }

		    glBindBuffer(GL_ARRAY_BUFFER, 0);
		    glBindVertexArray(0);

		    // texture
		    objectsInfo[objectsInfoSize].meshes[i2].VBOSize = data->nodes[i].mesh->primitives[i2].indices->count;
				    
		    {
			sprintf(buf, "%s%s", assetsFolder,
				data->nodes[i].mesh->primitives[i2].material->pbr_metallic_roughness.base_color_texture.texture->image->uri);
			SDL_Surface* texture = IMG_Load(buf);

    
			if (!texture) {
			    printf("Loading of texture \"%s\" failed", buf);
			    exit(0);
			}

			sceneWeight += texture->w * texture->h * texture->format->BytesPerPixel;

			createTexture(&objectsInfo[objectsInfoSize].meshes[i2].tx, texture->w,texture->h, texture->pixels);
  
			SDL_FreeSurface(texture);
		    }
		}
	    }
	    
	    idInInfo = objectsInfoSize;	    
	    objectsInfoSize++;
	}

	(*targetBuffer)[targetSize].infoId = idInInfo;

	// aabb
	{
	    aabbEntities[aabbEntitiesSize].col.rt = (vec3){ -FLT_MAX, -FLT_MAX, -FLT_MAX };
	    aabbEntities[aabbEntitiesSize].col.lb = (vec3){ FLT_MAX, FLT_MAX, FLT_MAX };

	    for(int i2=0;i2<data->nodes[i].mesh->primitives_count;++i2){
		if (data->nodes[i].mesh->primitives[i2].attributes_count == 0) continue;

		assert(data->nodes[i].mesh->primitives[i2].attributes->type == cgltf_attribute_type_position);
		    
		int compSize = GLTFtypeSize[data->nodes[i].mesh->primitives[i2].attributes->data->component_type];
		int vecLen = GLTFmapSize[data->nodes[i].mesh->primitives[i2].attributes->data->type];

		int posLen =  data->nodes[i].mesh->primitives[i2].attributes->data->buffer_view->size / vecLen / compSize;

		float pos[3];
		int vecSize = vecLen * compSize;

		for (int i3 = 0; i3 < posLen; ++i3) {	
		    fseek(fo, data->nodes[i].mesh->primitives[i2].indices->buffer_view->offset + i3 * vecSize, SEEK_SET);
		    fread(pos, vecSize, 1, fo);

		    vec4 transfPos = mulmatvec4((*targetBuffer)[*targetBufferSize].mat, (vec4) { pos[0], pos[1], pos[2], 1.0f });

		    aabbEntities[aabbEntitiesSize].col.lb.x = min(aabbEntities[aabbEntitiesSize].col.lb.x, transfPos.x);
		    aabbEntities[aabbEntitiesSize].col.lb.y = min(aabbEntities[aabbEntitiesSize].col.lb.y, transfPos.y);
		    aabbEntities[aabbEntitiesSize].col.lb.z = min(aabbEntities[aabbEntitiesSize].col.lb.z, transfPos.z);

		    aabbEntities[aabbEntitiesSize].col.rt.x = max(aabbEntities[aabbEntitiesSize].col.rt.x, transfPos.x);
		    aabbEntities[aabbEntitiesSize].col.rt.y = max(aabbEntities[aabbEntitiesSize].col.rt.y, transfPos.y);
		    aabbEntities[aabbEntitiesSize].col.rt.z = max(aabbEntities[aabbEntitiesSize].col.rt.z, transfPos.z);
		}

		aabbEntitiesSize++;
	    }
	}
	
	(*targetBufferSize)++;
    }
*/
    free(mesh);
    fclose(fo);
    cgltf_free(data);

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
    return;
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
    mouse.selectedThing = NULL;
    mouse.selectedType = 0;
}

float texturedTileVerts[] = {
    bBlockW, 0.0f, bBlockD , 0.0f, 1.0f, -0.000000, 1.000000, -0.000000,
    0.0f, 0.0f, bBlockD , 1.0f, 1.0f, -0.000000, 1.000000, -0.000000, 
    bBlockW, 0.0f, 0.0f , 0.0f, 0.0f, -0.000000, 1.000000, -0.000000,
      
    0.0f, 0.0f, bBlockD , 1.0f, 1.0f, 0.000000, 1.000000, 0.000000,
    bBlockW, 0.0f, 0.0f , 0.0f, 0.0f, 0.000000, 1.000000, 0.000000,
    0.0f, 0.0f, 0.0f , 1.0f, 0.0f,  0.000000, 1.000000, 0.000000,
};

void noHighlighting(){

}

void modelHighlighting(){
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

void generateShowAreas(){
    bool** snowGrid = malloc(sizeof(bool*) * gridZ);

    for(int i=0;i<gridZ;i++){
	snowGrid[i] = calloc(sizeof(bool), gridX);
    }

    int tilesCounter[2] = { 0 };
    
    for(int i=0;i<tilesStorageSize;i++){
	tilesCounter[tilesStorage[i].tx != txOfGround]++;
	snowGrid[(int)tilesStorage[i].pos.z][(int)tilesStorage[i].pos.x] = tilesStorage[i].tx != txOfGround;
    }

    vec3* bufs[layersCounter];
    int size = gridX * gridZ;

    bufs[deniedLayerT] = malloc(sizeof(vec3) * tilesCounter[deniedLayerT] * 6);
    bufs[acceptedLayerT] = malloc(sizeof(vec3) * tilesCounter[acceptedLayerT] * 6);

    int index = 0;
    memset(tilesCounter, 0, sizeof(int) * 2);

    float y = 0.1f;
    
    for(int z=0;z<gridZ;z++){
	for(int x=0;x<gridX;x++){
	    bufs[snowGrid[z][x]][(tilesCounter[snowGrid[z][x]]*6) + 0] = (vec3){ x + bBlockW, y, z };
	    bufs[snowGrid[z][x]][(tilesCounter[snowGrid[z][x]]*6) + 1] = (vec3){ x, y, z + bBlockD };
	    bufs[snowGrid[z][x]][(tilesCounter[snowGrid[z][x]]*6) + 2] = (vec3){ x, y, z };

	    //

	    bufs[snowGrid[z][x]][(tilesCounter[snowGrid[z][x]]*6) + 3] = (vec3){ x + bBlockW, y, z };
	    bufs[snowGrid[z][x]][(tilesCounter[snowGrid[z][x]]*6) + 4] = (vec3){ x + bBlockW, y, z + bBlockD };
	    bufs[snowGrid[z][x]][(tilesCounter[snowGrid[z][x]]*6) + 5] = (vec3){ x, y, z + bBlockD };
	    
	    tilesCounter[snowGrid[z][x]]++;
	}
    }
    
    // int 3*Y*X*Z

    if(snowParticles.buf){
	free(snowParticles.buf);

	free(snowParticles.timers);
	free(snowParticles.action);
	free(snowParticles.speeds);
    }

    int maxY = gridY * 2;

    int oneColParticles = 15; // was 5
    int particlesSize = tilesCounter[acceptedLayerT] * oneColParticles * 6;
    int uqParticlesSize = particlesSize / 6;

    snowParticles.timers = calloc(sizeof(int),  uqParticlesSize);
    snowParticles.action = malloc(sizeof(SnowAction) * uqParticlesSize);
    snowParticles.speeds = malloc(sizeof(float) * uqParticlesSize);

    for(int i=0;i<uqParticlesSize;i++){
	snowParticles.action[i] = rand() % snowDirCounter;
    }
    
    memset(snowParticles.timers, 0, sizeof(int) *  uqParticlesSize);

    snowParticles.buf = malloc(sizeof(vec3) * particlesSize);
    
    snowParticles.size = 0;// = tilesCounter[acceptedLayerT] * gridY;

    float snowFH = 0.02f; // was 0.01f
    
    index = 0;
//    for(int y=0;y<maxY;y++){
    for(int i=0;i<tilesCounter[acceptedLayerT];i++){
	vec3 lb = bufs[acceptedLayerT][i*6 + 2];

	for(int i2=0;i2<oneColParticles;i2++){
	    //	float fY = y + ((float)(rand() % 1000) / 1000.0f);
	    float fY = ((float)(rand() % (maxY * 100))) / 100.0f;
	    float fZ = lb.z + ((float)(rand() % 1000) / 1000.0f);
	    float fX = lb.x + ((float)(rand() % 1000) / 1000.0f);
 
	    snowParticles.buf[snowParticles.size] = (vec3){ fX, fY, fZ };
	    snowParticles.buf[snowParticles.size+1] = (vec3){ fX, fY+snowFH, fZ };
	    /*snowParticles.buf[snowParticles.size+2] = (vec3){ fX+snowFH, fY+snowFH, fZ };

	      snowParticles.buf[snowParticles.size+3] = (vec3){ fX, fY, fZ };
	      snowParticles.buf[snowParticles.size+4] = (vec3){ fX+snowFH, fY+snowFH, fZ };
	      snowParticles.buf[snowParticles.size+5] = (vec3){ fX+snowFH, fY, fZ };*/

	    snowParticles.size+=2;
	    index++;
	}
	    
	// from x, z

	// to x + bBlockW, z + bBlockD

    }
    //  }

    
    
    //  for(int i=0;i<snowParticles.size;i+=2){
//	snowParticles.buf[i+1] = (vec3){ snowParticles.buf[i].x , snowParticles.buf[i].y + 0.01f, snowParticles.buf[i].z };
//    }

    {
	glBindVertexArray(snowMesh.VAO); 
	glBindBuffer(GL_ARRAY_BUFFER, snowMesh.VBO);

	snowMesh.VBOsize = snowParticles.size;

	glBufferData(GL_ARRAY_BUFFER,
		     sizeof(vec3) * snowParticles.size, snowParticles.buf, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
    }

    for(int i=0;i<layersCounter;i++){
	glBindVertexArray(snowTilesMesh[i].VAO); 
	glBindBuffer(GL_ARRAY_BUFFER, snowTilesMesh[i].VBO);

	snowTilesMesh[i].VBOsize = 6 * tilesCounter[i];

	glBufferData(GL_ARRAY_BUFFER,
		     sizeof(vec3) * 6 * tilesCounter[i], bufs[i], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
		  
	free(bufs[i]);
    }

    for(int i=0;i<gridZ;i++){
	free(snowGrid[i]);
    }
	
    free(snowGrid);
}

void generateNavTiles(){
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

void findPath(vec2i start, vec2i dist, int y, int* size){
    clock_t t;
    t = clock();
    
    // astar
    {
	int openCellsTop = 0;

	bool pathFound = false;

	for (int z = 0; z < gridZ*3; z++) {
	    memset(closedList[z], 0 , sizeof(bool) * gridX*3);
	    
	    for (int x = 0; x < gridX*3; x++) {
		cellsDetails[z][x].g = FLT_MAX;
		cellsDetails[z][x].h = FLT_MAX;
		cellsDetails[z][x].f = FLT_MAX;
		cellsDetails[z][x].parX = -1;
		cellsDetails[z][x].parZ = -1;
		
		openCells[z*gridZ*3+x].f = FLT_MAX;
	    }
	}  

	cellsDetails[start.z][start.x].g = 0.0f;
	cellsDetails[start.z][start.x].h = 0.0f;
	cellsDetails[start.z][start.x].f = 0.0f;
	cellsDetails[start.z][start.x].parX = start.x;
	cellsDetails[start.z][start.x].parZ = start.z;

	openCells[openCellsTop] = (AstarOpenCell){
	    cellsDetails[start.z][start.x].f, .x = start.x, .z= start.z
	};
	openCellsTop++;
					    
	while(openCellsTop != 0){
	    int curIndex = 0;
	    for (int i = 1; i < openCellsTop; i++) {
		if (openCells[i].f < openCells[curIndex].f) {
		    curIndex = i;
		}
	    }

	    AstarOpenCell cur = openCells[curIndex];
	    int index = 0;
	    for (int i = 0; i < openCellsTop; i++) {
		if(i==curIndex){
		    continue;
		}
		
		openCells[index] = openCells[i];
		index++;
	    }
	    openCellsTop--;
						
	    closedList[cur.z][cur.x] = true;

	    for(int dz=-1;dz<2;dz++){
		for(int dx=-1;dx<2;dx++){
		    if(dx==0 && dz == 0){
			continue;
		    }

		    vec2i mCur = { cur.x + dx, cur.z + dz };
							
		    if(mCur.x < 0 || mCur.x >= (gridX * 3)){
			continue;
		    }

		    if(mCur.z < 0 || mCur.z >= (gridZ * 3)){
			continue;
		    } 

		    if(mCur.z == dist.z && mCur.x == dist.x){

//			printf("pre dest: %d %d dest: %d %d \n", cur.z, cur.x, dist.z, dist.x);
							    
			cellsDetails[mCur.z][mCur.x].parX = cur.x;
			cellsDetails[mCur.z][mCur.x].parZ = cur.z;
							    
//			printf("Destination found!: ");
			pathFound = true;

			int diZ = dist.z;
			int diX = dist.x;

			int pathSize = 0;
							    
			while (!(diZ == start.z && diX == start.x)) {
			    pathSize++;
			    int tZ = cellsDetails[diZ][diX].parZ;
			    int tX = cellsDetails[diZ][diX].parX;
			    diZ = tZ;
			    diX = tX;
			}

			vec2i* path = malloc(sizeof(vec2i) * pathSize);
			pathSize = 0;

			diZ = dist.z;
			diX = dist.x;
								
			while (!(diZ == start.z && diX == start.x)) {
			    path[pathSize].z = diZ;
			    path[pathSize].x = diX;
			    pathSize++;

			    int tZ = cellsDetails[diZ][diX].parZ;
			    int tX = cellsDetails[diZ][diX].parX;
								
			    diZ = tZ;
			    diX = tX;
			}

			float div = 1.0f / 3.0f;
				    
			if(entityStorage[playerEntityT][0].path){
			    free(entityStorage[playerEntityT][0].path);
			}
							    
			entityStorage[playerEntityT][0].path = malloc(sizeof(vec3) * ((pathSize)*2));
			entityStorage[playerEntityT][0].pathSize = pathSize*2;
			entityStorage[playerEntityT][0].curPath = 0;
			entityStorage[playerEntityT][0].frame = 0;							    

			entityStorage[playerEntityT][0].path[0] = (vec3){ start.x * div + div/2.0f, (float)y + 0.2f, start.z * div + div/2.0f };
			
			entityStorage[playerEntityT][0].path[1] = (vec3){ path[pathSize-1].x * div + div/2.0f, (float)y + 0.2f, path[pathSize-1].z * div + div/2.0f };

			int index = 2;
			for(int i=pathSize-1;i>0;i--){
			    entityStorage[playerEntityT][0].path[index] = (vec3){ path[i].x * div + div/2.0f, (float)y + 0.2f, path[i].z * div + div/2.0f };
			    index++;
								
			    entityStorage[playerEntityT][0].path[index] = (vec3){ path[i-1].x * div + div/2.0f, (float)y + 0.2f, path[i-1].z * div + div/2.0f };
			    index++;
			}

			glBindVertexArray(lastFindedPath.VAO); 
			glBindBuffer(GL_ARRAY_BUFFER, lastFindedPath.VBO);

			lastFindedPath.VBOsize = (pathSize)*2;

			glBufferData(GL_ARRAY_BUFFER,
				     sizeof(vec3) * (pathSize*2), entityStorage[playerEntityT][0].path, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		  
			free(path);
							    
			break;
		    }else if(!closedList[mCur.z][mCur.x]
			     && !collisionGrid[y][mCur.z][mCur.x]){
			float gNew = cellsDetails[cur.z][cur.x].g + 1.0f;
			float hNew = sqrtf((mCur.z - dist.z) * (mCur.z - dist.z) + (mCur.x - dist.x) * (mCur.x - dist.x));
			float fNew = gNew + hNew;

			if(cellsDetails[mCur.z][mCur.x].f == FLT_MAX ||
			   cellsDetails[mCur.z][mCur.x].f > fNew){

			    openCells[openCellsTop] = (AstarOpenCell){
				fNew, .z = mCur.z, .x = mCur.x
			    };
			    openCellsTop++;

			    assert(openCellsTop < gridX*3*gridZ*3);
								
			    cellsDetails[mCur.z][mCur.x].g = gNew;
			    cellsDetails[mCur.z][mCur.x].h = hNew;
			    cellsDetails[mCur.z][mCur.x].f = fNew;
			    cellsDetails[mCur.z][mCur.x].parX = cur.x;
			    cellsDetails[mCur.z][mCur.x].parZ = cur.z;
			}
		    }
							
		}
	    }

	    if (pathFound) break;

	}
    }

    t = clock() - t; 
    double time_taken = ((double)t)/CLOCKS_PER_SEC;
    printf("Astar speed: %lf \n", time_taken);
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

    int meshSize = cubeSize * aabbEntitiesSize; // + cubeSize for player AABB
   
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
	/*
	  float x,y,z;

	  vec4 q1 = aabbEntities[i].rot;
	
	  if (q1.w > 1) {
	  q1 = normalize4(q1);
	  }
	
	  float angle = 2 * acosf(q1.w);
	  float s = sqrtf(1-q1.w*q1.w);
	
	  if (s < 0.001) {
	  x = q1.x;
	  y = q1.y;
	  z = q1.z;
	  } else {
	  x = q1.x / s; 
	  y = q1.y / s;
	  z = q1.z / s;
	  }

	  vec3 orient = rotateVectorByQuaternion(aabbEntities[i].extents, aabbEntities[i].rot);
	  w	
	  float lowX = aabbEntities[i].center.x - aabbEntities[i].extents.x * aabbEntities[i].rot.x;
	  float lowY = aabbEntities[i].center.y - aabbEntities[i].extents.y * aabbEntities[i].rot.y;
	  float lowZ = aabbEntities[i].center.z - aabbEntities[i].extents.z * aabbEntities[i].rot.z;

	  float highX = aabbEntities[i].center.x + aabbEntities[i].extents.x * aabbEntities[i].rot.x;
	  float highY = aabbEntities[i].center.y + aabbEntities[i].extents.y * aabbEntities[i].rot.y;
	  float highZ = aabbEntities[i].center.z + aabbEntities[i].extents.z * aabbEntities[i].rot.z;
	*/
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
