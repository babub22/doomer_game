#include "deps.h"
#include "linearAlg.h"
#include "main.h"
#include "editor.h"
#include "game.h"
#include "ufbx.h"

#include "gameMap.h"
#include "editorMap.h"

#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

bool showHiddenWalls = true;

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

MeshBuffer doorDoorPlane;

const void(*stancilHighlight[mouseSelectionTypesCounter])() = {
    [0] = noHighlighting,
    [mouseModelT] = modelHighlighting,
    [mouseWallT] = doorFrameHighlighting,
    [mouseBlockT] = noHighlighting,
    [mousePlaneT] = noHighlighting,
    [mouseTileT] = noHighlighting,
    [mouseLightT] = noHighlighting,
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
    [gameMapInstance] = {
	[render2DFunc] = gameMap2dRender,
	[render3DFunc] = gameMap3dRender,
	[preLoopFunc] = gameMapPreLoop,
	[preFrameFunc] = gameMapPreFrame,
	[matsSetup] = gameMapMatsSetup,
	[eventFunc] = gameMapEvents,
	[onSetFunc] = gameMapOnSetInstance,
	[mouseVSFunc] = gameMapMouseVS,
	[renderCursorFunc] = gameMapRenderCursor
    },
    [editorMapInstance] = {
	[render2DFunc] = editorMap2dRender,
	[render3DFunc] = editorMap3dRender,
	[preLoopFunc] = editorMapPreLoop,
	[preFrameFunc] = editorMapPreFrame,
	[matsSetup] = editorMapMatsSetup,
	[eventFunc] = editorMapEvents,
	[onSetFunc] = editorMapOnSetInstance,
	[mouseVSFunc] = editorMapMouseVS,
	[renderCursorFunc] = editorMapRenderCursor
    }
};

// ~~~~~~~~~~~~~~~~~
const char* instancesStr[] = { [editorInstance]="Editor", [gameInstance]="Game", [editorMapInstance]="Editor Map", [gameMapInstance]="Game Map" };

const char* markersStr[] = { [locationExitMarkerT]="Exit marker" };

const char* wallTypeStr[] = {
    [normWallT] = "Wall",[RWallT] = "RWall", [LWallT] = "LWall",[LRWallT] = "LRWall",[windowT] = "Window",[doorT] = "Door"
};

const char* tileBlocksStr[] = { [roofBlockT] = "Roof",[stepsBlockT] = "Steps",[angledRoofT] = "Angle Roof" };

const char* lightTypesStr[] = { [pointLightT] = "PointLight", [dirLightShadowT] = "DirLight(shadow)", [dirLightT] = "DirLight" };

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
    [winCenterPlane] = "Center plane" ,

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
    [mouseMarkerBrushT] = "Marker",
    [mouseEntityBrushT] = "Entity",
};

const char* manipulationModeStr[] = { "None","Rotate_X", "Rotate_Y", "Rotate_Z", "Transform_XY", "Transform_Z", "Scale" };

const char* entityTypeStr[] = { [playerEntityT] = "Player entity" };

ModelsTypesInfo modelsTypesInfo[] = {
    [objectModelType] = {"Obj",0},
    [playerModelT] = {"Player", 0}
};

const char* shadersFileNames[] = { "lightSource", "hud", "fog", "borderShader", "screenShader", [dirShadowShader] = "dirShadowDepth", [UIShader] = "UI", [UITxShader] = "UITxShader", [UITransfShader] = "UITransf", [UITransfTx] = "UITransfTx", [UITransfColor] = "UITransfColor", [animShader] = "animModels", [snowShader] = "snowShader", [windowShader] = "windowShader" };

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
    }

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
	glEnable(GL_MULTISAMPLE);

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
  //  loadGLTFModel("./assets/Doomer.gltf");
    
    // load 3d models
    /*{
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
  
    allocateGrid(120, 8, 120);
    allocateCollisionGrid(120, 8, 120);
    
    defaultGrid(gridX, gridY, gridZ);
    //  }

    //    lightPos = (vec3){gridX /2.0f,2.0f,gridZ/2.0f};

    //  renderCapYLayer = gridY;
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

    bool quit = false;

    clock_t lastFrame = clock();
    float deltaTime;

    float cameraSpeed = speed;
    SDL_Event event;

    //  for(int i=0;i<instancesCounter;i++){
    ((void (*)(void))instances[curInstance][preLoopFunc])();
    ((void (*)(void))instances[editorMapInstance][preLoopFunc])();    
    //  }

    near_plane = 0.01f;
    far_plane  = 120.0f;
  
    glUseProgram(shadersId[dirShadowShader]);
    uniformFloat(dirShadowShader, "far_plane", far_plane);

    glUseProgram(shadersId[windowShader]);
    uniformInt(snowShader, "colorMap", 0); 
    
    glUseProgram(shadersId[snowShader]);
    uniformInt(snowShader, "colorMap", 0); 
    uniformInt(snowShader, "shadowMap", 1);
    uniformFloat(snowShader, "far_plane", far_plane);
  
    glUseProgram(shadersId[mainShader]);
    uniformInt(mainShader, "colorMap", 0); 
    uniformInt(mainShader, "shadowMap", 1);
    uniformFloat(mainShader, "far_plane", far_plane);

    glUseProgram(shadersId[animShader]);
    uniformInt(animShader, "colorMap", 0); 
    uniformInt(animShader, "shadowMap", 1);
    uniformFloat(animShader, "far_plane", far_plane);


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

		if(event.key.keysym.scancode == SDL_SCANCODE_F4){
		    showHiddenWalls = !showHiddenWalls;
	  
		    if(showHiddenWalls){
			batchAllGeometry();
		    }else{
			batchAllGeometryNoHidden();
		    }
		}

		if (event.key.keysym.scancode == SDL_SCANCODE_F3) {
		    if(navPointsDraw){
			navPointsDraw = !navPointsDraw;
		    }else{
			generateNavTiles();
		
			navPointsDraw = true;
		    }
		}

		if (event.key.keysym.scancode == SDL_SCANCODE_F4) {
		    if(snowAreas){
			snowAreas = !snowAreas;
		    }else{
			generateShowAreas();
		
			snowAreas = true;
		    }
		}


		if (event.key.keysym.scancode == SDL_SCANCODE_M && !selectedTextInput2) {
		    const EngineInstance instanceTrasferTable[4] = {
			[gameMapInstance] = gameInstance,
			[editorMapInstance] = editorInstance,
			[editorInstance] = editorMapInstance,
			[gameInstance] = gameMapInstance,
		    };

		    curInstance = instanceTrasferTable[curInstance];
		    ((void (*)(void))instances[curInstance][onSetFunc])();
		}

		if (event.key.keysym.scancode == SDL_SCANCODE_SPACE && !selectedTextInput2) {
		    if(curInstance == gameMapInstance){
			curInstance = editorMapInstance;
		    }else if(curInstance == editorMapInstance){
			curInstance = gameMapInstance;
		    }else{
	  
			if (curInstance >= gameInstance) {
			    curInstance = 0;
			}
			else {
			    curInstance++;
			}
		    }
                     
		    ((void (*)(void))instances[curInstance][onSetFunc])();
		}
	    }

      
	    ((void (*)(SDL_Event))instances[curInstance][eventFunc])(event);
	}

	mouse.tileSide = -1;
	//    checkMouseVSEntities();
	((void (*)(int))instances[curInstance][mouseVSFunc])(mainShader);
    
	((void (*)(float))instances[curInstance][preFrameFunc])(deltaTime);
    
	//  if (lightStorage)
	{
	    glViewport(0, 0, windowW, windowH);
	    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	    glEnable(GL_DEPTH_TEST);
	
	    ((void (*)(int))instances[curInstance][matsSetup])(mainShader);

	    glUseProgram(shadersId[snowShader]); 
	    uniformVec3(snowShader, "cameraPos", curCamera->pos);
	    
	    glUseProgram(shadersId[mainShader]); 
	    glUniform3f(cameraPos, argVec3(curCamera->pos));

//	    uniformFloat(mainShader, "far_plane", far_plane);

      
	    ((void (*)(void))instances[curInstance][render3DFunc])();

	    // windows 
	    if(windowWindowsMesh.VBOsize){
		glEnable(GL_BLEND);
		
		glBindTexture(GL_TEXTURE_2D, windowGlassId);
		
		glUseProgram(shadersId[windowShader]);
		
		glBindBuffer(GL_ARRAY_BUFFER, windowWindowsMesh.VBO);
		glBindVertexArray(windowWindowsMesh.VAO);

		Matrix out = IDENTITY_MATRIX;
		uniformMat4(windowShader, "model", out.m);
		    
		glDrawArrays(GL_TRIANGLES, 0, windowWindowsMesh.VBOsize);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		glBindVertexArray(0);

		glDisable(GL_BLEND);
	    }

	    // draw snow
	    {
		glUseProgram(shadersId[snowShader]);

		{
		    glEnable(GL_BLEND);
		    glBindBuffer(GL_ARRAY_BUFFER, snowMesh.VBO);
		    glBindVertexArray(snowMesh.VAO);

		    Matrix out = IDENTITY_MATRIX;
		    uniformMat4(snowShader, "model", out.m);
		    
		    glDrawArrays(GL_TRIANGLES, 0, snowMesh.VBOsize);

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
//	    if(false)
	    {
		static float dMove = 0.0005f;
		
		int index = 0;
		for(int i=0;i<snowParticles.size;i+=6){
		    if(snowParticles.buf[i].y > 0){
			for(int i2=0;i2<6;i2++){
			    snowParticles.buf[i+i2].y -= 0.0025f;
			}
//			snowParticles.buf[i+1].y -= 0.0025f;
		    }else{
			if(snowParticles.action[index] != WAITING_Y){
			    snowParticles.timers[index] = 120;
			    snowParticles.action[index] = WAITING_Y;
			}
		    }

		    if(snowParticles.timers[index] == 0){
			if(snowParticles.action[index] == WAITING_Y){
			    for(int i2=0;i2<6;i2++){
				snowParticles.buf[i+i2].y = (gridY * 2) - 0.025f;
			    }
//			    snowParticles.buf[i+1].y = gridY * 2;

			    snowParticles.action[index] = rand() % snowDirCounter;
			}else{
			    snowParticles.speeds[index] = (rand() % 6) / 10000.0f;
			    float maxDist;
			
			    snowParticles.action[index] = oppositeSnowDir[snowParticles.action[index]];
			
			    if(snowParticles.action[index] == X_PLUS_SNOW){
				maxDist = ((int)snowParticles.buf[i+2].x + 1) - snowParticles.buf[i+2].x;
			    }else if(snowParticles.action[index] == X_MINUS_SNOW){
				maxDist = snowParticles.buf[i+2].x - (int)snowParticles.buf[i+2].x;
			    }else if(snowParticles.action[index] == Z_MINUS_SNOW){
				maxDist = snowParticles.buf[i+1].z - (int)snowParticles.buf[i+1].z;
			    }else if(snowParticles.action[index] == Z_PLUS_SNOW){
				maxDist = ((int)snowParticles.buf[i+1].z + 1) - snowParticles.buf[i+1].z;
			    }

			    maxDist -= snowParticles.speeds[index];
			
			    snowParticles.timers[index] = maxDist / dMove;
			}
		    }else{
			snowParticles.timers[index]--;

			if(snowParticles.action[index] != WAITING_Y){
			    float dX = 0.0f;
			    float dZ = 0.0f;
			
			    if(snowParticles.action[index] == X_PLUS_SNOW){
				dX = snowParticles.speeds[index];
			    }else if(snowParticles.action[index] == X_MINUS_SNOW){
				dX = -snowParticles.speeds[index];
			    }else if(snowParticles.action[index] == Z_MINUS_SNOW){
				dZ = -snowParticles.speeds[index];
			    }else if(snowParticles.action[index] == Z_PLUS_SNOW){
				dZ = snowParticles.speeds[index];
			    }

			    for(int i2=0;i2<6;i2++){
				snowParticles.buf[i+i2].x += dX;
				snowParticles.buf[i+i2].z += dZ;
			    }

//			    snowParticles.buf[i+1].x += dX;

//			    snowParticles.buf[i+1].z += dZ;
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

	    // test anim
	    if(entityStorageSize[playerEntityT] != 0){
		static int curAnimStage = 0;
		static int frame = 0;

		if(frame == 30){
		    glUseProgram(shadersId[animShader]);

		    for(int i=0;i<boneAnimIndexedSize[curAnimStage];i++){
			if(boneAnimIndexed[curAnimStage][i]->action == cgltf_animation_path_type_translation){
			    bones[boneAnimIndexed[curAnimStage][i]->boneId].trans = (vec3){
				argVec3(boneAnimIndexed[curAnimStage][i]->value) };
			}else if(boneAnimIndexed[curAnimStage][i]->action == cgltf_animation_path_type_rotation){
			    bones[boneAnimIndexed[curAnimStage][i]->boneId].rot = boneAnimIndexed[curAnimStage][i]->value;
			}else if(boneAnimIndexed[curAnimStage][i]->action == cgltf_animation_path_type_scale){
			    bones[boneAnimIndexed[curAnimStage][i]->boneId].scale = (vec3){
				argVec3(boneAnimIndexed[curAnimStage][i]->value) };
			}
		    }

		    for(int i=0;i<bonesSize;i++){
				printf("T: %f %f %f R: %f %f %f %f S: %f %f %f \n", argVec3(bones[i].trans), argVec4(bones[i].rot), argVec3(bones[i].scale));
			Matrix T = IDENTITY_MATRIX;
			T.m[12] = bones[i].trans.x;
			T.m[13] = bones[i].trans.y;
			T.m[14] = bones[i].trans.z;
			
			Matrix R =  IDENTITY_MATRIX;
			R = mat4_from_quat(bones[i].rot);

			Matrix S = IDENTITY_MATRIX;
			scale(&S, argVec3(bones[i].scale));
			
			bones[i].matrix = multiplymat4(multiplymat4(T, R), S);
			
			//bones[i].matrix = multiplymat4(S, multiplymat4(R, T));
		    }

		    updateChildBonesMats(bones[0].id);

		    char buf[64];
		    for(int i=0;i<bonesSize;i++){
			Matrix res = multiplymat4(bones[i].matrix, bones[i].inversedMat);
//			Matrix res = multiplymat4(inversedMats[i], bones[i].matrix);
			
			sprintf(buf, "finalBonesMatrices[%d]", i);
			uniformMat4(animShader, buf, res.m);
		    }

		    glUseProgram(shadersId[mainShader]);

		    curAnimStage++;

		    if(curAnimStage == timesCounter-1){
			curAnimStage = 0;
		    }
		    
		    frame = 0;
		}

		frame++;
	    }

	    // nav meshes drawing
	    if(navPointsDraw){
		glUseProgram(shadersId[lightSourceShader]);

		if(selectedCollisionTileIndex != -1){
		    glBindBuffer(GL_ARRAY_BUFFER, selectedCollisionTileBuf.VBO);
		    glBindVertexArray(selectedCollisionTileBuf.VAO);

		    Matrix out = IDENTITY_MATRIX;
		    uniformMat4(lightSourceShader, "model", out.m);
		    uniformVec3(lightSourceShader, "color", (vec3) { yellowColor });
		    
		    glDrawArrays(GL_TRIANGLES, 0, selectedCollisionTileBuf.VBOsize);
		}
		
		{
		    glBindBuffer(GL_ARRAY_BUFFER, lastFindedPath.VBO);
		    glBindVertexArray(lastFindedPath.VAO);

		    Matrix out = IDENTITY_MATRIX;
		    uniformMat4(lightSourceShader, "model", out.m);
		    uniformVec3(lightSourceShader, "color", (vec3) { redColor });
		    
		    glDrawArrays(GL_LINES, 0, lastFindedPath.VBOsize);
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

	    /*
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
	      }*/


	    //      glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);
	 
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

    batchAllGeometry();
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

    float topMat[] = {
	1.000000, 0.000000, 0.000000, 0.000000,
	0.000000, 1.000000, 0.000000, 0.000000,
	0.000000, 0.000000, 1.000000, 0.000000,
	0.000000, 0.000000, 0.000000, 1.000000
    };

    float rightMat[] = {
	-0.000000, 0.000000, -1.000000, 0.000000,
	0.000000, 1.000000, 0.000000, 0.000000,
	1.000000, 0.000000, -0.000000, 0.000000,
	1.000000, 0.000000, 1.000000, 1.000000
    };

    float leftMat[] = {
	-0.000000, 0.000000, 1.000000, 0.000000,
	0.000000, 1.000000, 0.000000, 0.000000,
	-1.000000, 0.000000, -0.000000, 0.000000,
	0.000000, 0.000000, 0.000000, 1.000000
    };
  
    float botMat[] = {
	-1.000000, 0.000000, 0.000000, 0.000000,
	0.000000, 1.000000, 0.000000, 0.000000,
	-0.000000, 0.000000, -1.000000, 0.000000,
	1.000000, 0.000000, 1.000000, 1.000000
    };
  
    memcpy(hardWallMatrices[left].m, leftMat, sizeof(float) * 16);
    memcpy(hardWallMatrices[bot].m, botMat, sizeof(float) * 16);
    memcpy(hardWallMatrices[top].m, topMat, sizeof(float) * 16);
    memcpy(hardWallMatrices[right].m, rightMat, sizeof(float) * 16);
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

    float winPad = t/6;

    float wStart = 0.4f * 1.0f;
    float wEnd = wStart + 0.1f;
    
    float windowPlaneFront[] = {
	// cap bot
	/*0.0f, botH, winPad,   1.0f, 0.0f,
	w,  h-capH, winPad,    0.0f, 1.0f,
	0.0f, h-capH, winPad, 1.0f, 1.0f,

	0.0f, botH, winPad,    1.0f, 0.0f,
	w, botH, winPad,       0.0f, 0.0f,
	w,  h-capH, winPad,    0.0f, 1.0f,*/

	// front plane
	wStart, botH, winPad,   1.0f, 0.0f,
	wEnd,  h-capH, winPad,    0.0f, 1.0f,
	wStart, h-capH, winPad, 1.0f, 1.0f,

	wStart, botH, winPad,    1.0f, 0.0f,
	wEnd, botH, winPad,       0.0f, 0.0f,
	wEnd,  h-capH, winPad,    0.0f, 1.0f,

	// front start
	wStart, botH, 0.0f,   1.0f, 0.0f,
	wStart,  h-capH, winPad,    0.0f, 1.0f,
	wStart, h-capH, 0.0f, 1.0f, 1.0f,

	wStart, botH, 0.0f,    1.0f, 0.0f,
	wStart, botH, winPad,       0.0f, 0.0f,
	wStart,  h-capH, winPad,    0.0f, 1.0f,
	
	// front end 
	wEnd, botH, 0.0f,   1.0f, 0.0f,
	wEnd,  h-capH, winPad,    0.0f, 1.0f,
	wEnd, h-capH, 0.0f, 1.0f, 1.0f,

	wEnd, botH, 0.0f,    1.0f, 0.0f,
	wEnd, botH, winPad,       0.0f, 0.0f,
	wEnd,  h-capH, winPad,    0.0f, 1.0f,

	// back front
	wStart, botH, -winPad,   1.0f, 0.0f,
	wEnd,  h-capH, -winPad,    0.0f, 1.0f,
	wStart, h-capH, -winPad, 1.0f, 1.0f,

	wStart, botH, -winPad,    1.0f, 0.0f,
	wEnd, botH, -winPad,       0.0f, 0.0f,
	wEnd,  h-capH, -winPad,    0.0f, 1.0f,

	// back start
	wStart, botH, 0.0f,   1.0f, 0.0f,
	wStart,  h-capH, -winPad,    0.0f, 1.0f,
	wStart, h-capH, 0.0f, 1.0f, 1.0f,

	wStart, botH, 0.0f,    1.0f, 0.0f,
	wStart, botH, -winPad,       0.0f, 0.0f,
	wStart,  h-capH, -winPad,    0.0f, 1.0f,
	
	// back end 
	wEnd, botH, 0.0f,   1.0f, 0.0f,
	wEnd,  h-capH, -winPad,    0.0f, 1.0f,
	wEnd, h-capH, 0.0f, 1.0f, 1.0f,

	wEnd, botH, 0.0f,    1.0f, 0.0f,
	wEnd, botH, -winPad,       0.0f, 0.0f,
	wEnd,  h-capH, -winPad,    0.0f, 1.0f,

	/*
	// left window
	0.0f, botH, winPad,         1.0f, 0.0f,
	wStart,  h-capH, winPad,    0.0f, 1.0f,
	0.0f, h-capH, winPad,       1.0f, 1.0f,

	0.0f, botH, winPad,         1.0f, 0.0f,
	wStart, botH, winPad,       0.0f, 0.0f,
	wStart,  h-capH, winPad,    0.0f, 1.0f,

	// right window
	wEnd, botH, winPad,         1.0f, 0.0f,
        w,  h-capH, winPad,    0.0f, 1.0f,
	wEnd, h-capH, winPad,       1.0f, 1.0f,

	wEnd, botH, winPad,         1.0f, 0.0f,
        w, botH, winPad,       0.0f, 0.0f,
        w,  h-capH, winPad,    0.0f, 1.0f,
	*/
	    };

    float windowPlane[] = {
	// left window
	0.0f, botH, winPad,         1.0f, 0.0f,
	wStart,  h-capH, winPad,    0.0f, 1.0f,
	0.0f, h-capH, winPad,       1.0f, 1.0f,

	0.0f, botH, winPad,         1.0f, 0.0f,
	wStart, botH, winPad,       0.0f, 0.0f,
	wStart,  h-capH, winPad,    0.0f, 1.0f,

	// right window
	wEnd, botH, winPad,         1.0f, 0.0f,
	w,  h-capH, winPad,    0.0f, 1.0f,
	wEnd, h-capH, winPad,       1.0f, 1.0f,

	wEnd, botH, winPad,         1.0f, 0.0f,
	w, botH, winPad,       0.0f, 0.0f,
	w,  h-capH, winPad,    0.0f, 1.0f,
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

	[winCenterPlane] = windowPlaneFront,
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

	[winCenterPlane] = sizeof(windowPlaneFront),
    };

    wallsVPairs[windowT].pairs = malloc(sizeof(VPair) * winPlaneCounter);
    wallsVPairs[windowT].planesNum = winPlaneCounter;

    attachNormalsToBuf(&windowGlassPair, 0, sizeof(windowPlane), windowPlane);
    
    for(int i=0;i<wallsVPairs[windowT].planesNum;i++){
	attachNormalsToBuf(wallsVPairs[windowT].pairs, i, wallPlanesSize[i], wallPlanes[i]);
    }
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
    
	glUseProgram(shadersId[snowShader]);
	sprintf(buf, "lightSpaceMatrix[%d]", i);
	uniformMat4(snowShader, buf, lightSpaceMatrix.m);
	
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
    
    glUseProgram(shadersId[snowShader]);
    sprintf(buf, "lightSpaceMatrix[%d]", lightId);
    uniformMat4(snowShader, buf, lightSpaceMatrix.m);
    
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
  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
 
    glBindTexture(GL_TEXTURE_2D, 0);
}; 

// TODO: Func->macro
void uniformVec4(int shader, char* var, vec4 value){ 
    int uni = glGetUniformLocation(shadersId[shader], var);
    glUniform4f(uni, argVec4(value));
};

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

void loadGLTFModel(char* name){
    cgltf_options options = {0};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, name, &data);

    if (result != cgltf_result_success){
	exit(-1);
    }

    int mapSize[] = { [cgltf_type_vec3] = 3,[cgltf_type_vec4] = 4,[cgltf_type_vec2] = 2 };
    int typeSize[] = { [cgltf_component_type_r_32f] = sizeof(float), [cgltf_component_type_r_8u] = sizeof(uint8_t)};
    
    modelInfo2 = malloc(sizeof(ModelInfo2));
    modelInfo2->mesh.VBOsize = data->meshes->primitives->indices->count;

    FILE* fo = NULL;

    // get path of .bin file
    {
	char* binPath = malloc(sizeof(char) * (strlen(name) + 1));
	strcpy(binPath, name);

	for (int i = 0; i < strlen(binPath); i++) {
	    if (i != 0 && binPath[i] == '.') {
		binPath[i] = '\0';
		binPath = realloc(binPath, sizeof(binPath) * (strlen(binPath) + 1));
		strcat(binPath, ".bin");
		break;
	    }
	}

        fo = fopen(binPath, "rb");
	free(binPath);
    }

    glGenVertexArrays(1, &modelInfo2->mesh.VAO);
    glGenBuffers(1, &modelInfo2->mesh.VBO);

    glBindVertexArray(modelInfo2->mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, modelInfo2->mesh.VBO);

    // parse model attr
    {
	fseek(fo, data->meshes->primitives->indices->buffer_view->offset, SEEK_SET);
	uint16_t* indicies = malloc(data->meshes->primitives->indices->buffer_view->size);
	fread(indicies, data->meshes->primitives->indices->buffer_view->size, 1, fo);

	int rawAttrCount = data->meshes->primitives->attributes[0].data->buffer_view->size / typeSize[data->meshes->primitives->attributes[0].data->component_type] 
		    / mapSize[data->meshes->primitives->attributes[0].data->type];
	ModelAttr* rawAttr = malloc(sizeof(ModelAttr) * rawAttrCount);

    	for (int i = 0; i < data->meshes->primitives->attributes_count; i++) {
	    fseek(fo, data->meshes->primitives->attributes[i].data->buffer_view->offset, SEEK_SET);

	    float* buf = NULL;
	    uint8_t* buf2 = NULL;

	    if (i == 3) {
		buf2 = malloc(data->meshes->primitives->attributes[i].data->buffer_view->size);
		fread(buf2, data->meshes->primitives->attributes[i].data->buffer_view->size, 1, fo);
	    }
	    else {
		buf = malloc(data->meshes->primitives->attributes[i].data->buffer_view->size);
		fread(buf, data->meshes->primitives->attributes[i].data->buffer_view->size, 1, fo);
	    }

	    int attrSize = data->meshes->primitives->attributes[i].data->buffer_view->size /
		typeSize[data->meshes->primitives->attributes[i].data->component_type];

	    int index = 0;
	    
	    for (int i2 = 0; i2 < attrSize; i2 += mapSize[data->meshes->primitives->attributes[i].data->type]) {
		if(i==0){
		    rawAttr[index].pos.x = buf[i2];
		    rawAttr[index].pos.y = buf[i2 + 1];
		    rawAttr[index].pos.z = buf[i2 + 2];
		}else if(i==1){
		    rawAttr[index].uv.x = buf[i2];
		    rawAttr[index].uv.z = buf[i2 + 1];
		}else if(i==2){
		    rawAttr[index].norm.x = buf[i2];
		    rawAttr[index].norm.y = buf[i2 + 1];
		    rawAttr[index].norm.z = buf[i2 + 2];
		}else if(i==3){
		    rawAttr[index].bonesId.x = buf2[i2];
		    rawAttr[index].bonesId.y = buf2[i2 + 1];
		    rawAttr[index].bonesId.z = buf2[i2 + 2];
		    rawAttr[index].bonesId.w = buf2[i2 + 3];
		}else if(i==4){
		    rawAttr[index].weights.x = buf[i2];
		    rawAttr[index].weights.y = buf[i2 + 1];
		    rawAttr[index].weights.z = buf[i2 + 2];
		    rawAttr[index].weights.w = buf[i2 + 3];
		}
		
		index++;
	    }

	    if(buf2){
		free(buf2);
	    }else{
		free(buf);
	    }
	}


	ModelAttr* model = malloc(sizeof(ModelAttr) * data->meshes->primitives->indices->count);

	for(int i=0;i< data->meshes->primitives->indices->count;i++){	    
	    model[i].pos = rawAttr[indicies[i]].pos;
	    model[i].uv = rawAttr[indicies[i]].uv;
	    model[i].norm = rawAttr[indicies[i]].norm;

	    model[i].bonesId = rawAttr[indicies[i]].bonesId;
	    model[i].weights = rawAttr[indicies[i]].weights;
	}
    
	for(int i=0;i< data->meshes->primitives->indices->count;i++){
	    printf("pos: %f %f %f uv: %f %f norm %f %f %f\n bonesId: %d %d %d %d weights: %f %f %f %f \n\n",
		   argVec3(model[i].pos), argVec2(model[i].uv), argVec3(model[i].norm),
		   argVec4(model[i].bonesId),argVec4(model[i].weights));
	}
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(ModelAttr) * data->meshes->primitives->indices->count, model, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ModelAttr), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ModelAttr), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ModelAttr), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glVertexAttribIPointer(3, 4, GL_INT, sizeof(ModelAttr), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(ModelAttr), (void*)(8 * sizeof(float) + 4 * sizeof(int)));
	glEnableVertexAttribArray(4);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	free(rawAttr);
	free(model);
    }

    // parse bones
    {
	bonesSize = data->skins[0].joints_count;
	bones = malloc(sizeof(BoneInfo) * bonesSize);

	inversedMats = malloc(sizeof(Matrix) * bonesSize);
	fseek(fo, data->skins->inverse_bind_matrices->buffer_view->offset, SEEK_SET);
	fread(inversedMats, data->skins->inverse_bind_matrices->buffer_view->size, 1, fo);

	glUseProgram(shadersId[animShader]);
	
	char buf[64];
	for (int i = 0; i < data->skins[0].joints_count; i++) {
	    bones[i].id = i;
	    bones[i].name = malloc(sizeof(char) * (strlen(data->skins[0].joints[i]->name) + 1));
	    strcpy(bones[i].name, data->skins[0].joints[i]->name);

	    bones[i].matrix = IDENTITY_MATRIX;

	    {
		Matrix T = IDENTITY_MATRIX;
		T.m[12] = data->skins[0].joints[i]->translation[0];
		T.m[13] = data->skins[0].joints[i]->translation[1];
		T.m[14] = data->skins[0].joints[i]->translation[2];

		Matrix R = IDENTITY_MATRIX;
		vec4  rot = {
		    data->skins[0].joints[i]->rotation[0],
		    data->skins[0].joints[i]->rotation[1],
		    data->skins[0].joints[i]->rotation[2],
		    data->skins[0].joints[i]->rotation[3]
		};
//		R = mat4_from_quat(rot);

		Matrix S = IDENTITY_MATRIX;
		scale(&S, data->skins[0].joints[i]->scale[0], data->skins[0].joints[i]->scale[1], data->skins[0].joints[i]->scale[2]);

		bones[i].matrix = multiplymat4(multiplymat4(T, R), S);
	    }
	    
	    bones[i].inversedMat = inversedMats[i];

	    Matrix res = multiplymat4(bones[i].matrix, bones[i].inversedMat);

	    sprintf(buf, "finalBonesMatrices[%d]", i);
	    uniformMat4(animShader, buf, res.m);


	    /*
	      Matrix ownMat = IDENTITY_MATRIX;
	    if(data->skins[0].joints[i]->has_matrix){
		memcpy(ownMat.m, data->skins[0].joints[i]->matrix, sizeof(float) * 16);
	    }

	    Matrix traslMat = IDENTITY_MATRIX;

	    if(data->skins[0].joints[i]->has_translation){
		vec3 traslsatePar = { data->skins[0].joints[i]->translation[0],
				      data->skins[0].joints[i]->translation[1],
				      data->skins[0].joints[i]->translation[2] };
	    
		translate(&traslMat, argVec3(traslsatePar));
	    }

	    Matrix rotateMat =  IDENTITY_MATRIX;
	    if(data->skins[0].joints[i]->has_rotation){
		vec4 rotation = {data->skins[0].joints[i]->rotation[0],data->skins[0].joints[i]->rotation[1], data->skins[0].joints[i]->rotation[2], data->skins[0].joints[i]->rotation[3]};
		rotateMat =  mat4_from_quat(rotation);
	    }


	    Matrix scaleMat = IDENTITY_MATRIX;
	    if(data->skins[0].joints[i]->has_scale){
		vec3 scalePar = { data->skins[0].joints[i]->scale[0],
				  data->skins[0].joints[i]->scale[1],
				  data->skins[0].joints[i]->scale[2]
		};
	    
		scale(&scaleMat, argVec3(scalePar));
	    }
	    */

	    //bones[i].defMat = multiplymat4(multiplymat4(multiplymat4(multiplymat4(traslMat, rotateMat), scaleMat), ownMat), inverseBindMatrix);
	    
	    //sprintf(buf, "finalBonesMatrices[%d]", i);
	    //uniformMat4(animShader, buf, bones[i].defMat.m);
	}

//	free(inversedMats);

	// set child info
	for (int i = 0;i<data->skins[0].joints_count; i++) {
	    bones[i].childSize = data->skins[0].joints[i]->children_count;
	    bones[i].childIds = malloc(sizeof(int) * bones[i].childSize);

	    bones[i].parentId = -1;

	    // set parent id 
	    if(data->skins[0].joints[i]->parent->name){
		int boneId = -1;

		for(int i3=0;i3<bonesSize;i3++){
		    if(strcmp(bones[i3].name, data->skins[0].joints[i]->parent->name)==0){
			boneId = bones[i3].id;
			break;
		    }
		}

		bones[i].parentId = boneId;
	    }
	    
	    for(int i2=0;i2<bones[i].childSize;i2++){
		int boneId = -1;

		for(int i3=0;i3<bonesSize;i3++){
		    if(strcmp(bones[i3].name, data->skins[0].joints[i]->children[i2]->name)==0){
			boneId = bones[i3].id;
			break;
		    }
		}
		
		bones[i].childIds[i2] = boneId;
	    }
	}

	traverseBones(bones[0].id);
    }

    // parse anims
    {
	int animationSize = 0;

	for (int i = 0; i < data->animations->samplers_count; i++) {
	    animationSize += data->animations->channels[i].sampler->input->buffer_view->size / sizeof(float);
	}

	BoneAction* boneActions = malloc(sizeof(BoneAction) * animationSize);

	int index = 0;

	char* strType[] = {[cgltf_interpolation_type_linear] = "Linear",
			   [cgltf_interpolation_type_step] = "Step",
			   [cgltf_interpolation_type_cubic_spline] = "Cubic", [cgltf_interpolation_type_cubic_spline+1] = "ERROR" };

	char* actionTypeStr[] = { [cgltf_animation_path_type_invalid] = "INVALID" ,
				  [cgltf_animation_path_type_translation] = "Translation",
				  [cgltf_animation_path_type_rotation] = "Rotation",
				  [cgltf_animation_path_type_scale] = "Scale",
				  [cgltf_animation_path_type_weights] = "Weithg",[cgltf_animation_path_type_weights+1] = "ERROR" };

	for (int i = 0; i < data->animations->samplers_count; i++) {
	    float* time = malloc(data->animations->channels[i].sampler->input->buffer_view->size);
	    float* transform = malloc(data->animations->channels[i].sampler->output->buffer_view->size);

	    fseek(fo, data->animations->channels[i].sampler->input->buffer_view->offset, SEEK_SET);
	    fread(time, data->animations->channels[i].sampler->input->buffer_view->size, 1, fo);

	    fseek(fo, data->animations->channels[i].sampler->output->buffer_view->offset, SEEK_SET);
	    fread(transform, data->animations->channels[i].sampler->output->buffer_view->size, 1, fo);


	    int boneId = -1;

	    for(int i2=0;i2<bonesSize;i2++){
		if(strcmp(bones[i2].name, data->animations->channels[i].target_node->name)==0){
		    boneId = bones[i2].id;
		    break;
		}
	    }

	    assert(boneId != -1);

	    int index2 = 0;
		
	    for (int i2 = 0; i2 < data->animations->channels[i].sampler->output->buffer_view->size / sizeof(float);
		 i2 += mapSize[data->animations->channels[i].sampler->output->type]) {

		boneActions[index].time = time[index2];
		boneActions[index].boneId = boneId;

		boneActions[index].action = data->animations->channels[i].target_path;
		boneActions[index].interp = data->animations->channels[i].sampler->interpolation;

		boneActions[index].value = (vec4){
		    transform[i2], transform[i2 + 1], transform[i2 + 2], transform[i2 + 3]
		};

		if(mapSize[data->animations->channels[i].sampler->output->type] == 3){
		    //  printf("%s: t: %f transf: %f %f %f \n", data->animations->channels[i].target_node->name, time[index2], transform[i2], transform[i2 + 1], transform[i2 + 2]);
		}else{
//		    printf("%s: t: %f rot: %f %f %f %f \n", data->animations->channels[i].target_node->name, time[index2], transform[i2], transform[i2 + 1], transform[i2 + 2], transform[i2 + 3]);
		}
			
		index2++;
		index++;
	    }

			/*
	    for (int i2 = 0; i2 < data->animations->channels[i].sampler->input->buffer_view->size / sizeof(float); i2++) {
		boneActions[index].time = time[i2];
		boneActions[index].boneId = boneId;

		boneActions[index].action = data->animations->channels[i].target_path;
		boneActions[index].interp = data->animations->channels[i].sampler->interpolation;
		    
	    }*/

	    free(time);
	}

	qsort(boneActions, animationSize, sizeof(BoneAction), sortBonesByTime);

	timesCounter = 0;
	float curTime = boneActions[0].time;

	for(int i=0;i<animationSize;i++){
	    if(curTime != boneActions[i].time || i == animationSize - 1){
		timesCounter++;
		curTime = boneActions[i].time;
	    }
	}

	boneAnimIndexedSize = malloc(sizeof(int) * timesCounter);

	curTime = boneActions[0].time;
	timesCounter = 0;
	int endOfPrev = 0;

	for (int i = 0; i < animationSize; i++) {
		if (curTime != boneActions[i].time || i == animationSize - 1) {
			boneAnimIndexedSize[timesCounter] = i - endOfPrev;

			endOfPrev = i;
			timesCounter++;
			curTime = boneActions[i].time;
		}
	}

	boneAnimIndexed = malloc(sizeof(BoneAction**) * timesCounter);

	for (int i = 0; i < timesCounter; i++) {
	    boneAnimIndexed[i] = malloc(sizeof(BoneAction*) * boneAnimIndexedSize[i]);
	}
	
	curTime = boneActions[0].time;
	timesCounter = 0;
	endOfPrev = 0;

	for(int i=0;i<animationSize;i++){
	    if(curTime != boneActions[i].time){			
		int index = 0;
		
		for(int i2=endOfPrev;i2<i;i2++){
		    boneAnimIndexed[timesCounter][index] = &boneActions[i2];
		    boneActions[i2].index = timesCounter;
		    index++;
		}

		endOfPrev = i;
		timesCounter++;
		curTime = boneActions[i].time;
	    }else if(i == animationSize-1){
		int index = 0;
		
		for(int i2=endOfPrev;i2<(i+1);i2++){
		    boneAnimIndexed[timesCounter][index] = &boneActions[i2];
		    boneActions[i2].index = timesCounter;
		    index++;
		}
		
		timesCounter++;
	    }
	}
	
//	/*
	for(int i=0;i<timesCounter;i++){
	    for(int i2=0;i2<boneAnimIndexedSize[i];i2++){
		//	printf("%d: bone: %d: time: %f \n", i, boneAnimIndexed[i][i2]->boneId, boneAnimIndexed[i][i2]->time);
	    }
	}//*/

	for(int i=0;i<animationSize;i++){
//	    printf("%d: bone: %d: time: %f trasf %s-%s: %f %f %f %f \n", boneActions[i].index ,boneActions[i].boneId, boneActions[i].time,
//		   actionTypeStr[boneActions[i].action], strType[boneActions[i].interp], argVec4(boneActions[i].value));
	}
    }
    
    fclose(fo);
    cgltf_free(data);
}

void loadFBXModel(char* name){
    cgltf_options options = {0};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, name, &data);

    int vertexSize = sizeof(vec3) * 2 + sizeof(vec2) + sizeof(vec4) + sizeof(vec4i);
    
    if (result == cgltf_result_success){
	modelInfo2 = malloc(sizeof(ModelInfo2));
	modelInfo2->mesh.VBOsize = data->meshes->primitives->indices->count;

	char* binPath = malloc(sizeof(char) * (strlen(name) + 1));
	strcpy(binPath, name);

	for (int i = 0; i < strlen(binPath); i++) {
	    if (i != 0 && binPath[i] == '.') {
		binPath[i] = '\0';
		binPath = realloc(binPath, sizeof(binPath) * (strlen(binPath) + 1));
		strcat(binPath, ".bin");
		break;
	    }
	}

	FILE* fo = fopen(binPath, "rb");
	free(binPath);

	glGenVertexArrays(1, &modelInfo2->mesh.VAO);
	glGenBuffers(1, &modelInfo2->mesh.VBO);

	glBindVertexArray(modelInfo2->mesh.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, modelInfo2->mesh.VBO);

	int size = 0;

	int mapSize[] = { [cgltf_type_vec3] = 3,[cgltf_type_vec4] = 4,[cgltf_type_vec2] = 2 };
	int typeSize[] = { [cgltf_component_type_r_32f] = sizeof(float), [cgltf_component_type_r_8u] = sizeof(uint8_t)};

	for (int i = 0; i < data->meshes->primitives->attributes_count; i++) {
	    size += typeSize[data->meshes->primitives->attributes[i].data->component_type] * mapSize[data->meshes->primitives->attributes[i].data->type];
	}

	fseek(fo, data->meshes->primitives->indices->buffer_view->offset, SEEK_SET);
	uint16_t* indicies = malloc(data->meshes->primitives->indices->buffer_view->size);
	fread(indicies, data->meshes->primitives->indices->buffer_view->size, 1, fo);

	for(int i=0;i<data->meshes->primitives->indices->buffer_view->size/sizeof(uint16_t);i++){
	    //printf("%d \n", indicies[i]);
	}

	char buf[128];	
	glUseProgram(shadersId[animShader]);

	bonesSize = data->skins[0].joints_count;
	bones = malloc(sizeof(BoneInfo) * bonesSize);

	Matrix inverseBindMatrix = IDENTITY_MATRIX;

	fseek(fo, data->skins[0].inverse_bind_matrices->buffer_view->offset, SEEK_SET);
	//float* inverseMat = malloc(data->skins[0].inverse_bind_matrices->buffer_view->size);
	fread(inverseBindMatrix.m, sizeof(float) * 16, 1, fo);

	
	for (int i = 0;i<data->skins[0].joints_count; i++) {
	    bones[i].id = i;
	    bones[i].name = malloc(sizeof(char) * (strlen(data->skins[0].joints[i]->name)+1));
	    strcpy(bones[i].name, data->skins[0].joints[i]->name);
			
//	    printf("%s  - %d \n", bones[i].name, strcmp(bones[i].name, data->skins[0].joints[i]->name));

	    Matrix ownMat = IDENTITY_MATRIX;
	    if(data->skins[0].joints[i]->has_matrix){
		memcpy(ownMat.m, data->skins[0].joints[i]->matrix, sizeof(float) * 16);
	    }
//	    memcpy(bones[i].defMat.m, data->skins[0].joints[i]->matrix, sizeof(float) * 16);


	    Matrix traslMat = IDENTITY_MATRIX;

	    if(data->skins[0].joints[i]->has_translation){
		vec3 traslsatePar = { data->skins[0].joints[i]->translation[0],
				      data->skins[0].joints[i]->translation[1],
				      data->skins[0].joints[i]->translation[2] };
	    
		translate(&traslMat, argVec3(traslsatePar));
	    }

	    Matrix rotateMat =  IDENTITY_MATRIX;
	    if(data->skins[0].joints[i]->has_rotation){
		vec4 rotation = {data->skins[0].joints[i]->rotation[0],data->skins[0].joints[i]->rotation[1], data->skins[0].joints[i]->rotation[2], data->skins[0].joints[i]->rotation[3]};
		rotateMat =  mat4_from_quat(rotation);
	    }


	    Matrix scaleMat = IDENTITY_MATRIX;
	    if(data->skins[0].joints[i]->has_scale){
		vec3 scalePar = { data->skins[0].joints[i]->scale[0],
				  data->skins[0].joints[i]->scale[1],
				  data->skins[0].joints[i]->scale[2]
		};
	    
		scale(&scaleMat, argVec3(scalePar));
	    }

	    bones[i].defMat = multiplymat4(multiplymat4(multiplymat4(multiplymat4(traslMat, rotateMat), scaleMat), ownMat), inverseBindMatrix);

	    
	    //bones[i].defMat = multiplymat4(multiplymat4(multiplymat4(traslMat, IDENTITY_MATRIX), IDENTITY_MATRIX), ownMat);
	    

	    /*
	    Matrix scaleMat = IDENTITY_MATRIX;
		
	    scaleMat.m[0] = data->skins[0].joints[i]->scale[0];
	    scaleMat.m[5] = data->skins[0].joints[i]->scale[1];
	    scaleMat.m[10] = data->skins[0].joints[i]->scale[2];
		
	    bones[i].defMat = multiplymat4(bones[i].defMat, scaleMat);

	    Matrix transfMat = IDENTITY_MATRIX;

	    transfMat.m[12] = data->skins[0].joints[i]->translation[0];
	    transfMat.m[13] = data->skins[0].joints[i]->translation[1];
	    transfMat.m[14] = data->skins[0].joints[i]->translation[2];

	    bones[i].defMat = multiplymat4(bones[i].defMat, transfMat);

	    vec4 rotation = {data->skins[0].joints[i]->rotation[0],data->skins[0].joints[i]->rotation[1], data->skins[0].joints[i]->rotation[2], data->skins[0].joints[i]->rotation[3]};
	    Matrix rotateMat =  mat4_from_quat(rotation);

	    bones[i].defMat = multiplymat4(bones[i].defMat, rotateMat);

	    bones[i].defMat = multiplymat4(bones[i].defMat, ownMat);*/
	    
//	    sprintf(buf, "finalBonesMatrices[%d]", i);
//	    uniformMat4(animShader, buf, bones[i].defMat.m);
	}	

	int attributeSizeCap = data->meshes->primitives->attributes[0].data->buffer_view->size / typeSize[data->meshes->primitives->attributes[0].data->component_type] 
		/ mapSize[data->meshes->primitives->attributes[0].data->type];

	vec3* pos = malloc(sizeof(vec3) * attributeSizeCap);
	vec2* uv = malloc(sizeof(vec2) * attributeSizeCap);
	vec3* norm = malloc(sizeof(vec3) * attributeSizeCap);

	vec4i* bonesId = malloc(sizeof(vec4i) * attributeSizeCap);
	vec4* weights = malloc(sizeof(vec4) * attributeSizeCap);

	int offset[] = { [0] = 0, [1]=3,[2]=5,[3]=8,[4]=12  };

	for (int i = 0; i < data->meshes->primitives->attributes_count; i++) {
	    fseek(fo, data->meshes->primitives->attributes[i].data->buffer_view->offset, SEEK_SET);

	    float* buf = NULL;
	    uint8_t* buf2 = NULL;

	    if (i == 3) {
		buf2 = malloc(data->meshes->primitives->attributes[i].data->buffer_view->size);
		fread(buf2, data->meshes->primitives->attributes[i].data->buffer_view->size, 1, fo);
	    }
	    else {
		buf = malloc(data->meshes->primitives->attributes[i].data->buffer_view->size);
		fread(buf, data->meshes->primitives->attributes[i].data->buffer_view->size, 1, fo);
	    }

	    int attrSize = data->meshes->primitives->attributes[i].data->buffer_view->size /
		typeSize[data->meshes->primitives->attributes[i].data->component_type];

	    int index = 0;
	    
	    for (int i2 = 0; i2 < attrSize; i2 += mapSize[data->meshes->primitives->attributes[i].data->type]) {
		if(i==0){
		    pos[index].x = buf[i2];
		    pos[index].y = buf[i2 + 1];
		    pos[index].z = buf[i2 + 2];
		}else if(i==1){
		    uv[index].x = buf[i2];
		    uv[index].z = buf[i2 + 1];
		}else if(i==2){
		    norm[index].x = buf[i2];
		    norm[index].y = buf[i2 + 1];
		    norm[index].z = buf[i2 + 2];
		}else if(i==3){
		    bonesId[index].x = buf2[i2];
		    bonesId[index].y = buf2[i2 + 1];
		    bonesId[index].z = buf2[i2 + 2];
		    bonesId[index].w = buf2[i2 + 3];
		}else if(i==4){
		    weights[index].x = buf[i2];
		    weights[index].y = buf[i2 + 1];
		    weights[index].z = buf[i2 + 2];
		    weights[index].w = buf[i2 + 3];
		}
		
		index++;
	    }

	    if(buf2){
		free(buf2);
	    }else{
		free(buf);
	    }
	}

        ModelAttr* model = malloc(sizeof(ModelAttr) * data->meshes->primitives->indices->count);

	for(int i=0;i< data->meshes->primitives->indices->count;i++){	    
	    model[i].pos = pos[indicies[i]];
	    model[i].uv = uv[indicies[i]];
	    model[i].norm = norm[indicies[i]];

	    model[i].bonesId = bonesId[indicies[i]];
	    model[i].weights = weights[indicies[i]];
	}

	/*for(int i=0;i< data->meshes->primitives->indices->count;i++){
	    printf("pos: %f %f %f uv: %f %f norm %f %f %f\n bonesId: %d %d %d %d weights: %f %f %f %f \n\n",
		   argVec3(model[i].pos), argVec2(model[i].uv), argVec3(model[i].norm),
		   argVec4(model[i].bonesId),argVec4(model[i].weights));
		   }*/

	glBufferData(GL_ARRAY_BUFFER, sizeof(ModelAttr) * data->meshes->primitives->indices->count, model, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ModelAttr), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ModelAttr), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ModelAttr), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glVertexAttribIPointer(3, 4, GL_INT, sizeof(ModelAttr), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(ModelAttr), (void*)(8 * sizeof(float) + 4 * sizeof(int)));
	glEnableVertexAttribArray(4);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	assert(data->animations->samplers_count == data->animations->channels_count);

	int animationSize = 0;

	for (int i = 0; i < data->animations->samplers_count; i++) {
	    animationSize += data->animations->channels[i].sampler->input->buffer_view->size / sizeof(float);
	}

	BoneAction* boneActions = malloc(sizeof(BoneAction) * animationSize);

	int index = 0;

	char* strType[] = {[cgltf_interpolation_type_linear] = "Linear",
			   [cgltf_interpolation_type_step] = "Step",
			   [cgltf_interpolation_type_cubic_spline] = "Cubic", [cgltf_interpolation_type_cubic_spline+1] = "ERROR" };

	char* actionTypeStr[] = { [cgltf_animation_path_type_invalid] = "INVALID" ,
				  [cgltf_animation_path_type_translation] = "Translation",
				  [cgltf_animation_path_type_rotation] = "Rotation",
				  [cgltf_animation_path_type_scale] = "Scale",
				  [cgltf_animation_path_type_weights] = "Weithg",[cgltf_animation_path_type_weights+1] = "ERROR" };

	for (int i = 0; i < data->animations->samplers_count; i++) {
		float* time = malloc(data->animations->channels[i].sampler->input->buffer_view->size);
		float* transform = malloc(data->animations->channels[i].sampler->output->buffer_view->size);

		fseek(fo, data->animations->channels[i].sampler->input->buffer_view->offset, SEEK_SET);
		fread(time, data->animations->channels[i].sampler->input->buffer_view->size, 1, fo);

		fseek(fo, data->animations->channels[i].sampler->output->buffer_view->offset, SEEK_SET);
		fread(transform, data->animations->channels[i].sampler->output->buffer_view->size, 1, fo);


		int boneId = -1;

		for(int i2=0;i2<bonesSize;i2++){
		    if(strcmp(bones[i2].name, data->animations->channels[i].target_node->name)==0){
			boneId = bones[i2].id;
			break;
		    }
		}

		assert(boneId != -1);
			
		for (int i2 = 0; i2 < data->animations->channels[i].sampler->input->buffer_view->size / sizeof(float); i2++) {
			boneActions[index].time = time[i2];
			boneActions[index].boneId = boneId;

			boneActions[index].action = data->animations->channels[i].target_path;
			boneActions[index].interp = data->animations->channels[i].sampler->interpolation;

			if (data->animations->channels[i].sampler->output->type == 3) {
			    //boneActions[index].tValue = (vec4){transform[i2], transform[i2 + 1], transform[i2 + 2], 0.0f};
			}
			else {
			    //boneActions[index].tValue = (vec4){ transform[i2], transform[i2 + 1], transform[i2 + 2], transform[i2 + 3] };
			}

			//	bones[boneId].defMat;

			Matrix scaleMat = IDENTITY_MATRIX;
			Matrix transfMat = IDENTITY_MATRIX;
			Matrix rotate = IDENTITY_MATRIX;
		    
		    

		    
		    //boneActions[index].mat.m;
		    
//		    printf("%d for bone: %d: time: %f trasf %s-%s: %f %f %f \n", i, boneId, time[i2], actionTypeStr[data->animations->channels[i].target_path], strType[data->animations->channels[i].sampler->interpolation], transform[i2], transform[i2 + 1], transform[i2 + 2]);
		    index++;
		}

		free(time);
	}

	qsort(boneActions, animationSize, sizeof(BoneAction), sortBonesByTime);

	for(int i=0;i<animationSize;i++){
	  //  printf("bone: %d: time: %f trasf %s-%s: %f %f %f %f \n", boneActions[i].boneId, boneActions[i].time, actionTypeStr[boneActions[i].action], strType[boneActions[i].interp], argVec4(boneActions[i].tValue));
	}
	
	fclose(fo);
	cgltf_free(data);

	free(bonesId);
	free(weights);
	free(pos);
	free(uv);
	free(norm);
	free(model);
    }

    
    /*
    char** bonesStrs = NULL;
    int bonesSize = 0;
    
    ufbx_scene* scene = ufbx_load_file(name, NULL, NULL);

    for (size_t i = 0; i < scene->nodes.count; i++) {
	ufbx_node *node = scene->nodes.data[i];
	if (node->is_root) continue;

	if(node->bone) bonesSize++;
    }

    bonesStrs = malloc(sizeof(char*) * bonesSize);
    bones = malloc(sizeof(BoneInfo) * bonesSize);
    
	int lastBoneIndex = 0;
			      
    for (size_t i = 0; i < scene->nodes.count; i++) {
	ufbx_node *node = scene->nodes.data[i];
	if (node->is_root) continue;

	if (node->bone) {
		int boneId = -1;

//		if (boneId == -1) {
			bones[i].id = lastBoneIndex;

			bonesStrs[i] = malloc(sizeof(char) * node->bone->name.length);
			strcpy(bonesStrs[i], node->bone->name.data);

			lastBoneIndex++;

			bones[i].offset = IDENTITY_MATRIX;
			for (int i2 = 0; i2 < 12; i2++) {
			    bones[i].offset.m[i2] = node->bone->instances.data[0]->geometry_to_node.v[i2];
			}


//			boneId = bones[i].id;

//		}
	}

	// num_vertices;

	if (node->mesh) {
	    modelInfo2 = malloc(sizeof(ModelInfo2));
	
	    modelInfo2->buf = malloc(sizeof(float) * 8 * node->mesh->num_indices);
	    modelInfo2->mesh.VBOsize = node->mesh->num_indices;

	    for(int i=0;i< node->mesh->num_indices;i++){
		int vertIndx = node->mesh->vertex_indices.data[i];
		int index = i * 8;

		modelInfo2->buf[index+0] = node->mesh->vertex_position.values.data[vertIndx].x;
		modelInfo2->buf[index+1] = node->mesh->vertex_position.values.data[vertIndx].y;
		modelInfo2->buf[index+2] = node->mesh->vertex_position.values.data[vertIndx].z;

		modelInfo2->buf[index+3] = node->mesh->vertex_uv.values.data[vertIndx].x;
		modelInfo2->buf[index+4] = node->mesh->vertex_uv.values.data[vertIndx].y;
	    
		modelInfo2->buf[index+5] = node->mesh->vertex_normal.values.data[vertIndx].x;
		modelInfo2->buf[index+6] = node->mesh->vertex_normal.values.data[vertIndx].y;
		modelInfo2->buf[index+7] = node->mesh->vertex_normal.values.data[vertIndx].z;
	    }

	    glGenVertexArrays(1, &modelInfo2->mesh.VAO);
	    glGenBuffers(1, &modelInfo2->mesh.VBO);
	    
	    glBindVertexArray(modelInfo2->mesh.VAO);
	    glBindBuffer(GL_ARRAY_BUFFER, modelInfo2->mesh.VBO);

	    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * node->mesh->num_indices,
			modelInfo2->buf, GL_STATIC_DRAW);

	    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), NULL);
	    glEnableVertexAttribArray(0);

	    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	    glEnableVertexAttribArray(1);

	    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	    glEnableVertexAttribArray(2);

	    glBindBuffer(GL_ARRAY_BUFFER, 0);
	    glBindVertexArray(0);
	}
    }
*/
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
//  if(curInstance == editorInstance){
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
	int txIndex = tilesStorage[i].tx;

	if (txIndex == -1){
	    continue;
	}


	for(int i2=0;i2< 6*vertexSize;i2 += vertexSize){
	    preGeom[txIndex].buf[txLastIndex[txIndex]+i2] = texturedTileVerts[i2] + tilesStorage[i].pos.x; 
	    preGeom[txIndex].buf[txLastIndex[txIndex]+i2+1] = texturedTileVerts[i2+1] + tilesStorage[i].pos.y;
	    preGeom[txIndex].buf[txLastIndex[txIndex]+i2+2] = texturedTileVerts[i2+2] + tilesStorage[i].pos.z; 

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
	    // if (!wallsStorage[i]->planes[i2].hide) {
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
	    //}
	}
    }
  
    for(int i=0;i<loadedTexturesCounter;i++){ 
	glBindVertexArray(finalGeom[i].VAO);
	glBindBuffer(GL_ARRAY_BUFFER, finalGeom[i].VBO);

	if (!preGeom[i].size) {
	    continue;
	}

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
  
    //  Geom* preGeom = malloc(sizeof(Geom) * loadedTexturesCounter);
//    int* txLastIndex = calloc(loadedTexturesCounter, sizeof(int));

    memset(txLastIndex, 0, sizeof(int) * loadedTexturesCounter);
  
    for(int i=0;i<loadedTexturesCounter;i++){
	finalGeom[i].buf = calloc(geomentyByTxCounter[i],1);
	finalGeom[i].size = geomentyByTxCounter[i];
	finalGeom[i].tris = geomentyByTxCounter[i] / (float)vertexSize / sizeof(float);
    }

    if(placedWallCounter[windowT]){
	if(windowWindowsBatch){
	    free(windowWindowsBatch);
	}
	
	windowWindowsBatch = malloc((sizeof(float) * 8) *
				    windowGlassPair.vertexNum * placedWallCounter[windowT]);
	
	int wCounter = 0;
        for(int i=0;i<wallsStorageSize;i++){
	    if(wallsStorage[i]->type == windowT){
		for(int i2=0;i2<windowGlassPair.vertexNum;i2++){
		    int index = i2 * 8;
		    
		    vec4 vert = { windowGlassPair.vBuf[index], windowGlassPair.vBuf[index + 1], windowGlassPair.vBuf[index + 2], 1.0f };

		    vec4 transf = mulmatvec4(wallsStorage[i]->mat, vert);

		    vec4 normal = { windowGlassPair.vBuf[index + 5], windowGlassPair.vBuf[index + 6], windowGlassPair.vBuf[index + 7], 1.0f };

		    Matrix inversedWallModel = IDENTITY_MATRIX;
		    inverse(wallsStorage[i]->mat.m, inversedWallModel.m); 

		    Matrix trasposedAndInversedWallModel = IDENTITY_MATRIX;
		    mat4transpose(trasposedAndInversedWallModel.m, inversedWallModel.m);

		    vec4 transfNormal = mulmatvec4(trasposedAndInversedWallModel, normal);

		    windowWindowsBatch[(index + wCounter)] = transf.x;
		    windowWindowsBatch[(index + wCounter) + 1] = transf.y;
		    windowWindowsBatch[(index + wCounter) + 2] = transf.z;

		    windowWindowsBatch[(index + wCounter) + 3] = windowGlassPair.vBuf[index + 3];
		    windowWindowsBatch[(index + wCounter) + 4] = windowGlassPair.vBuf[index + 4];

		    windowWindowsBatch[(index + wCounter) + 5] = transfNormal.x;
		    windowWindowsBatch[(index + wCounter) + 6] = transfNormal.y;
		    windowWindowsBatch[(index + wCounter) + 7] = transfNormal.z;

		    printf("v: %f %f %f uv: %f %f n: %f %f %f \n", windowWindowsBatch[index + wCounter], windowWindowsBatch[index + wCounter+1], windowWindowsBatch[index + wCounter+2],
			   windowWindowsBatch[index + wCounter+3], windowWindowsBatch[index + wCounter+4], windowWindowsBatch[index + wCounter+5], windowWindowsBatch[index + wCounter+6],
			   windowWindowsBatch[index + wCounter+7]);
		}
		
		wCounter+= windowGlassPair.vertexNum*8;
	    }
	}

		//for(int)

	glBindVertexArray(windowWindowsMesh.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, windowWindowsMesh.VBO);

	windowWindowsMesh.VBOsize = windowGlassPair.vertexNum * placedWallCounter[windowT];
	glBufferData(GL_ARRAY_BUFFER, (sizeof(float) * 8) *
				    windowGlassPair.vertexNum * placedWallCounter[windowT], windowWindowsBatch, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);  
	glBindVertexArray(0);
    }

    for(int i=0;i<windowGlassPair.vertexNum * placedWallCounter[windowT];i++){
	int index = i * 8;
	printf("v: %f %f %f uv: %f %f n: %f %f %f \n", windowWindowsBatch[index], windowWindowsBatch[index+1], windowWindowsBatch[index+2],
	       windowWindowsBatch[index+3], windowWindowsBatch[index+4], windowWindowsBatch[index+5], windowWindowsBatch[index+6],
	    windowWindowsBatch[index+7]);
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

	    finalGeom[txIndex].buf[txLastIndex[txIndex] + i2] = transf.x;
	    finalGeom[txIndex].buf[txLastIndex[txIndex] + i2 + 1] = transf.y;
	    finalGeom[txIndex].buf[txLastIndex[txIndex] + i2 + 2] = transf.z;

	    finalGeom[txIndex].buf[txLastIndex[txIndex] + i2 + 3] = planePairs.vBuf[i2 + 3];
	    finalGeom[txIndex].buf[txLastIndex[txIndex] + i2 + 4] = planePairs.vBuf[i2 + 4];

	    finalGeom[txIndex].buf[txLastIndex[txIndex] + i2 + 5] = transfNormal.x;
	    finalGeom[txIndex].buf[txLastIndex[txIndex] + i2 + 6] = transfNormal.y;
	    finalGeom[txIndex].buf[txLastIndex[txIndex]+ i2 + 7] = transfNormal.z;
	}

	txLastIndex[txIndex] += 6 * 8;
    }

    // tiles
    for(int i=0;i<tilesStorageSize;i++){
	int txIndex = tilesStorage[i].tx;

	if (txIndex == -1){
	    continue;
	} 

	for(int i2=0;i2< 6*vertexSize;i2 += vertexSize){
	    finalGeom[txIndex].buf[txLastIndex[txIndex]+i2] = texturedTileVerts[i2] + tilesStorage[i].pos.x; 
	    finalGeom[txIndex].buf[txLastIndex[txIndex]+i2+1] = texturedTileVerts[i2+1] + tilesStorage[i].pos.y;
	    finalGeom[txIndex].buf[txLastIndex[txIndex]+i2+2] = texturedTileVerts[i2+2] + tilesStorage[i].pos.z; 

	    finalGeom[txIndex].buf[txLastIndex[txIndex]+i2+3] = texturedTileVerts[i2+3];
	    finalGeom[txIndex].buf[txLastIndex[txIndex]+i2+4] = texturedTileVerts[i2+4];

	    finalGeom[txIndex].buf[txLastIndex[txIndex]+i2+5] = texturedTileVerts[i2+5];
	    finalGeom[txIndex].buf[txLastIndex[txIndex]+i2+6] = texturedTileVerts[i2+6];
	    finalGeom[txIndex].buf[txLastIndex[txIndex]+i2+7] = texturedTileVerts[i2+7];
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

		finalGeom[txIndex].buf[txLastIndex[txIndex] + i3] = transf.x;
		finalGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 1] = transf.y;
		finalGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 2] = transf.z;

		finalGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 3] = blocksVPairs[type].pairs[i2].vBuf[i3 + 3];
		finalGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 4] = blocksVPairs[type].pairs[i2].vBuf[i3 + 4];

		finalGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 5] = transfNormal.x;
		finalGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 6] = transfNormal.y;
		finalGeom[txIndex].buf[txLastIndex[txIndex]+ i3 +7] = transfNormal.z;
	    }

	    txLastIndex[txIndex] += blocksVPairs[type].pairs[i2].vertexNum * vertexSize;
	}
    }

    // walls
    for(int i=0;i<wallsStorageSize;i++){
	WallType type = wallsStorage[i]->type;

	for (int i2 = 0; i2 < wallsVPairs[type].planesNum; i2++) {
	    //if (!wallsStorage[i]->planes[i2].hide) {
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

		finalGeom[txIndex].buf[txLastIndex[txIndex] + i3] = transf.x;
		finalGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 1] = transf.y;
		finalGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 2] = transf.z;

		finalGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 3] = wallsVPairs[type].pairs[i2].vBuf[i3 + 3];
		finalGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 4] = wallsVPairs[type].pairs[i2].vBuf[i3 + 4];

		finalGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 5] = transfNormal.x;
		finalGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 6] = transfNormal.y;
		finalGeom[txIndex].buf[txLastIndex[txIndex] + i3 + 7] = transfNormal.z;
	    }
        
	    txLastIndex[txIndex] += wallsVPairs[type].pairs[i2].vertexNum * vertexSize;
	    //}
	}
    }

    for(int i=0;i<loadedTexturesCounter;i++){ 
	glBindVertexArray(finalGeom[i].VAO);
	glBindBuffer(GL_ARRAY_BUFFER, finalGeom[i].VBO);

	printf("alloced size - %d  used size - %d \n", finalGeom[i].size, txLastIndex[i] * sizeof(float));
	glBufferData(GL_ARRAY_BUFFER, finalGeom[i].size, finalGeom[i].buf, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);  
	glBindVertexArray(0);
	
	printf("last i: %d \n", i);
	free(finalGeom[i].buf);
    }
}

void noHighlighting(){

}

void doorFrameHighlighting(){
    WallMouseData* wallData = (WallMouseData*)mouse.selectedThing;
    int type = wallData->wall->type;
    int plane = wallData->plane;

    if ((type == hiddenDoorT || type == doorT) && plane == doorCenterPlane) {
	glEnable(GL_STENCIL_TEST);

	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
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

    int oneColParticles = 5;
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

    float snowFH = 0.01f;
    
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
		snowParticles.buf[snowParticles.size+2] = (vec3){ fX+snowFH, fY+snowFH, fZ };

		snowParticles.buf[snowParticles.size+3] = (vec3){ fX, fY, fZ };
		snowParticles.buf[snowParticles.size+4] = (vec3){ fX+snowFH, fY+snowFH, fZ };
		snowParticles.buf[snowParticles.size+5] = (vec3){ fX+snowFH, fY, fZ };

		snowParticles.size+=6;
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
    for(int i=0;i<layersCounter;i++){
	collisionLayersSize[i] = 0;
    }

    float square[] = {
	1.0f,  0.0f, 0.0f,   
	0.0f,  0.0f, 1.0f,  
	0.0f,  0.0f, 0.0f,

	1.0f,  0.0f,  0.0f,   
	1.0f,  0.0f,  1.0f,      
	0.0f,  0.0f,  1.0f,
    };
	  
    vec3* acceptedTiles = malloc(sizeof(square) * 9 * tilesStorageSize);
    vec3* deniedTiles = malloc(sizeof(square) * 9 * tilesStorageSize);
    CollisionTile* tiles = malloc(sizeof(CollisionTile) * tilesStorageSize * 9);

    float netPad[] = { 0.0f, 1.0f / 3.0f, 1.0f - (1.0f / 3.0f), 1.0f };

    int squareIndxPad = 6 * 9;

    float squarePad = 0.02f;
    int tilesCounter = 0;
	  
    for(int i=0; i<tilesStorageSize; i++){
	for(int row=0;row<3;row++){
	    for(int col=0;col<3;col++){
		tiles[tilesCounter].h = tilesStorage[i].pos.y;
		
		tiles[tilesCounter].rt = (vec2) { 1.0f * netPad[row+1] - squarePad + tilesStorage[i].pos.x,
						  1.0f * netPad[col+1] - squarePad + tilesStorage[i].pos.z };

		tiles[tilesCounter].lb = (vec2){ 1.0f * netPad[row] + squarePad + tilesStorage[i].pos.x,
						 1.0f * netPad[col] + squarePad + tilesStorage[i].pos.z };

		tiles[tilesCounter].f = false;
		collisionLayersSize[acceptedLayerT]++;
		tilesCounter++;
	    }
	}
    }


    for(int i2=0;i2<wallsStorageSize;i2++){
	for(int i=0;i<tilesCounter;i++){
	    if(tiles[i].f == true){
		continue;
	    }
	    
	    bool isIntersect = false;
	
	    vec2 wallRt = { wallsStorage[i2]->planes[wTopPlane].rt.x, wallsStorage[i2]->planes[wTopPlane].rt.z };
	    vec2 wallLb = { wallsStorage[i2]->planes[wTopPlane].lb.x, wallsStorage[i2]->planes[wTopPlane].lb.z };

	    isIntersect = wallLb.x <= tiles[i].rt.x &&
		wallRt.x >= tiles[i].lb.x &&
		wallLb.z <= tiles[i].rt.z &&
		wallRt.z >= tiles[i].lb.z;

	    if(isIntersect){
		tiles[i].f = true;

		float div = 1.0f / 3.0f;
		
		collisionGrid[tiles[i].h][(int)(tiles[i].lb.z/div)][(int)(tiles[i].lb.x/div)] = true;
		collisionLayersSize[acceptedLayerT]--;
	    }
	}
    }

    if(acceptedCollisionTilesAABB){
	free(acceptedCollisionTilesAABB);
    }

    acceptedCollisionTilesAABB = malloc(sizeof(AABB) * collisionLayersSize[acceptedLayerT]);

    printf("Before %d \n", collisionLayersSize[acceptedLayerT]);

    collisionLayersSize[acceptedLayerT] = 0;

    for(int i=0;i<tilesCounter;i++){
	float y = tiles[i].h + 0.1f;
	
	if(tiles[i].f){
	    int index = collisionLayersSize[deniedLayerT];
	    
	    deniedTiles[(index*6) + 0] = (vec3){ tiles[i].rt.x, y, tiles[i].lb.z };

	    deniedTiles[(index*6) + 1] = (vec3){ tiles[i].lb.x, y, tiles[i].rt.z };

	    deniedTiles[(index*6) + 2] = (vec3){ tiles[i].lb.x, y, tiles[i].lb.z };

	    //

	    deniedTiles[(index*6) + 3] = (vec3){ tiles[i].rt.x, y, tiles[i].lb.z };

	    deniedTiles[(index*6) + 4] = (vec3){ tiles[i].rt.x, y, tiles[i].rt.z };

	    deniedTiles[(index*6) + 5] = (vec3){ tiles[i].lb.x, y, tiles[i].rt.z };
	    
	    collisionLayersSize[deniedLayerT]++;
	}else{
	    int index = collisionLayersSize[acceptedLayerT];
	    
	    acceptedTiles[(index*6) + 0] = (vec3){ tiles[i].rt.x, y, tiles[i].lb.z };

	    acceptedTiles[(index*6) + 1] = (vec3){ tiles[i].lb.x, y, tiles[i].rt.z };

	    acceptedTiles[(index*6) + 2] = (vec3){ tiles[i].lb.x, y, tiles[i].lb.z };

	    //

	    acceptedTiles[(index*6) + 3] = (vec3){ tiles[i].rt.x, y, tiles[i].lb.z };

	    acceptedTiles[(index*6) + 4] = (vec3){ tiles[i].rt.x, y, tiles[i].rt.z };

	    acceptedTiles[(index*6) + 5] = (vec3){ tiles[i].lb.x, y, tiles[i].rt.z };
	    
	    acceptedCollisionTilesAABB[index].lb = (vec3){ tiles[i].lb.x, y, tiles[i].lb.z };
	    acceptedCollisionTilesAABB[index].rt = (vec3){ tiles[i].rt.x, y, tiles[i].rt.z };

	    collisionLayersSize[acceptedLayerT]++;
	}
    }
    

	printf("AfterL %d \n", collisionLayersSize[acceptedLayerT]);
    free(tiles);

    {
	glBindVertexArray(navigationTilesMesh[0].VAO); 
	glBindBuffer(GL_ARRAY_BUFFER, navigationTilesMesh[0].VBO);

	navigationTilesMesh[0].VBOsize = 6 * collisionLayersSize[acceptedLayerT];

	glBufferData(GL_ARRAY_BUFFER,
		sizeof(square) * collisionLayersSize[acceptedLayerT], acceptedTiles, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
		  
	free(acceptedTiles);
    }
	      
    {
	glBindVertexArray(navigationTilesMesh[1].VAO); 
	glBindBuffer(GL_ARRAY_BUFFER, navigationTilesMesh[1].VBO);

	navigationTilesMesh[1].VBOsize = 6 * collisionLayersSize[deniedLayerT];

	glBufferData(GL_ARRAY_BUFFER,
		     sizeof(square) * collisionLayersSize[deniedLayerT], deniedTiles, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	free(deniedTiles);
    }
}

int sortBonesByTime(BoneAction* a, BoneAction* b){
    if(a->time > b->time)  
    {  
        return 1;  
    }else if(a->time < b->time){  
        return -1;  
    }
    
    return 0;  
}

void traverseBones(int jointIndex){
    printf(" %s ", bones[jointIndex].name);

    for(int i=0; i<bones[jointIndex].childSize;i++){
	printf("->");
	traverseBones(bones[jointIndex].childIds[i]);
    }
    printf("\n");
}

void updateChildBonesMats(int jointIndex){
    if (bones[jointIndex].parentId != -1){
	bones[jointIndex].matrix =
	    multiplymat4(bones[bones[jointIndex].parentId].matrix, bones[jointIndex].matrix);
    }

    for (int i = 0; i < bones[jointIndex].childSize; ++i){
	updateChildBonesMats(bones[jointIndex].childIds[i]);
    }
}
