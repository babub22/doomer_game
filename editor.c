#include "deps.h"
#include "linearAlg.h"
#include "main.h"
#include "editor.h"

int* dialogEditorHistory;
int dialogEditorHistoryLen;
int dialogEditorHistoryCursor;

bool lightView = false;

const float lightPresetTable[][2] = { {0.0014, 0.000007},
				      {0.007, 0.0002},
				      {0.014, 0.0007},
				      {0.022, 0.0019},
				      {0.027, 0.0028},
				      {0.045, 0.0075},
				      {0.07, 0.017},
				      {0.09, 0.032},
				      {0.14, 0.07},
				      {0.22, 0.20},
				      {0.35, 0.44},
				      {0.7, 1.8}
};

#define lightsPresetMax 12

bool hints = true;

typedef enum{
    moveMode = 1, rotationMode, cursorModeCunter
} EditorCursorMode;

typedef enum{
    XCircle = 1, YCircle, ZCircle, XYPlane, ZYPlane, XZPlane, axisCounter
} RotationCircles;

EditorCursorMode cursorMode;

float rotationGizmodegAcc;
bool isFreeGizmoRotation;

VPair* gizmosGeom[2];
const int gizmosNum[2] = { axisCounter-1, 3 }; 
vec3* gizmosAABB[2][2];
vec3* gizmosPaddings[2];

vec3 gizmoStartPos;
vec3 gizmoCurPos;
RotationCircles selectedGizmoAxis;

const vec3 gizmosColors[axisCounter - 1] = {
    [XCircle - 1] = {redColor},
    [YCircle - 1] = {greenColor},
    [ZCircle - 1] = {blueColor},

    [XZPlane - 1] = {redColor},
    [XYPlane - 1] = {greenColor},
    [ZYPlane - 1] = {blueColor},
};

const char* gizmosAxisStr[axisCounter] = {
    [0]= "None",
    [XCircle]= "X-axis",
    [YCircle]= "Y-axis",
    [ZCircle]= "Z-axis",
  
    [XYPlane]= "XY-Plane",
    [ZYPlane]= "ZY-Plane",
    [XZPlane]= "XZ-Plane",
};


int texturesMenuCurCategoryIndex = 0;
ModelType objectsMenuSelectedType = objectModelType;

Menu objectsMenu = { .type = objectsMenuT };
Menu blocksMenu = { .type = blocksMenuT };
Menu texturesMenu = { .type = texturesMenuT };
Menu lightMenu = { .type = lightMenuT };
Menu planeCreatorMenu = { .type = planeCreatorT };

// avaible/loaded models
ModelInfo* loadedModels1D;
ModelInfo** loadedModels2D;
size_t loadedModelsSize;

VPair cube;

char contextBelowText[500];
float contextBelowTextH;

float borderArea = (bBlockW / 8);

Matrix editorProj;
Matrix editorView;

VPair netTile;

//bool mouseMovedInThisFrame;

const int rotationBlock[4][2] = {
    {-bBlockW, 0.0f},
    { 0,        bBlockW }, // 12 14
    { bBlockW,  0 },
    { 0,-bBlockD }, 
};

vec2 mouseFrameDiff;

void editorOnSetInstance(){
    printf("Now editor");

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    renderCapYLayer = gridY;
    batchAllGeometry();
}


void editorPreLoop(){
    // setup mats for markers
    {
	// locationExitMarkerT
	markersMats[locationExitMarkerT] = IDENTITY_MATRIX;

	scale(&  markersMats[locationExitMarkerT], 9.0f, 1.0f, 9.0f);
	    
	markersMats[locationExitMarkerT].m[12] = bBlockD / 2;
	markersMats[locationExitMarkerT].m[13] = 0.1f;
	markersMats[locationExitMarkerT].m[14] = bBlockD / 2;

	// playerStartMarkerT
	//markersMats[playerStartMarkerT] = IDENTITY_MATRIX;

	//scale(&  markersMats[playerStartMarkerT], 3.0f, 5.0f, 3.0f);
	    
	//markersMats[playerStartMarkerT].m[12] = bBlockD / 2;
//	markersMats[playerStartMarkerT].m[13] = 5.0f / 2.0f;
//	markersMats[playerStartMarkerT].m[14] = bBlockD / 2;
    }

    // setup mats for entities
    {
	// playerEntity
	entitiesMats[playerEntityT] = IDENTITY_MATRIX;

//	scale(&entitiesMats[playerEntityT], 3.0f, 5.0f, 3.0f);
	    
//	entitiesMats[playerEntityT].m[12] = bBlockD / 2;
//	entitiesMats[playerEntityT].m[13] = 5.0f / 2.0f;
//	entitiesMats[playerEntityT].m[14] = bBlockD / 2;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // cursor in input
    {
	float textInputCursorW = 0.005;

	float cursorBotH = 0.002;
	float cursorTopH = letterH - textInputCursorW;
    
	bindUIQuad((vec2[6]){
		{ 0.0f, cursorBotH}, { 0.0f, cursorTopH}, { textInputCursorW, cursorTopH },
		{ 0.0f, cursorBotH }, { textInputCursorW, cursorTopH }, { textInputCursorW, cursorBotH}
	    },(uint8_t[4]){whiteColor, 1.0f},&textInputCursorBuf);
    }


   
    // markers Window
    {
	UIRect2* markersWindow;
	int markersWindowSize;

	markersWindowSize = 1 + markersCounter;
	markersWindow = calloc(markersWindowSize, sizeof(UIRect2));    
    
	float rW = -1000.0f;
	
	// background
	for(int i=0;i<markersCounter;i++){
	    float inputLeftW = -1.0f;
	    float inputRightW = -1.0f + (strlen(markersStr[i])+1) * letterW;

	    rW = fmaxf(rW, -1.0f + (strlen(markersStr[i])+1) * letterW);

	    float inputTopH = 1.0f - ((i+1) * letterH);
	    float inputBotH = inputTopH - letterH;
    
	    markersWindow[i+1] = (UIRect2){
		.pos = {
		    { inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
		    { inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
		},
            .lb = {inputLeftW, inputBotH}, .rt = {inputRightW, inputTopH},
					    .c = { blackColor, 1.0f }
	    };

	    /*
	    if(i == playerStartMarkerT){
		markersWindow[i].onClick = setPlayerStartMarkerBrush;
	    }else if(i == locationExitMarkerT){
		markersWindow[i].onClick = setExitMarkerMarkerBrush;
	    }*/

	    switch (i)
	    {
	    case(locationExitMarkerT): {
		markersWindow[i+1].onClick = setExitMarkerMarkerBrush;

		break;
	    }
	    default:
		break;
	    }

	    markersWindow[i+1].highlight = malloc(sizeof(MeshBuffer));
	    bindUIQuad((vec2[6]) {
		    { inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
		    { inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
		},(uint8_t[4]) { redColor, 1.0f }, markersWindow[i+1].highlight);

	    markersWindow[i+1].textPos = (vec2){inputLeftW, inputBotH + letterH };
	    markersWindow[i+1].text = malloc(sizeof(char) * strlen(markersStr[i]));
	    strcpy(markersWindow[i+1].text, markersStr[i]);
	}
    
	{
	    float lW = -1.0f;
	    float h = 1.0f - ((markersCounter+1) * letterH);
	    markersWindow[0] = (UIRect2){ .pos = {
		    { lW, h }, { lW, 1.0f }, { rW, 1.0f },
		    { lW, h }, { rW, 1.0f }, { rW, h }
		},
					  .c = { blackColor, 1.0f },
					  .lb = { lW, h}, .rt = {rW, 1.0f}
	    };

	    markersWindow[0].textPos = (vec2){ -1.0f, 1.0f };
	    markersWindow[0].text = malloc(sizeof(char) * strlen("Markers:"));
	    strcpy(markersWindow[0].text, "Markers:");
	}

/*
// load button

loadWindow[2].onClick = loadMapUI;

loadWindow[2].highlight = malloc(sizeof(MeshBuffer));
bindUIQuad((vec2[6]) {
{ inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
{ inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
},(uint8_t[4]) { redColor, 1.0f }, loadWindow[2].highlight);

loadWindow[2].textPos = (vec2){inputLeftW, inputBotH + letterH };
loadWindow[2].text = malloc(sizeof(char) * strlen("load"));
strcpy(loadWindow[2].text, "load");
}

// cancel button
{
float inputLeftW = w - 0.1f - (strlen("cancel")+1) * letterW;
float inputRightW = w - 0.1f;

float inputTopH = -h + letterH + 0.02f;
float inputBotH = -h + 0.02f;
    
loadWindow[3] = (UIRect2){ .pos = {
{ inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
{ inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
},
.lb = {inputLeftW, inputBotH}, .rt = {inputRightW, inputTopH},
.c = { greenColor, 1.0f }
};

loadWindow[3].onClick = clearCurrentUI;
      
loadWindow[3].highlight = malloc(sizeof(MeshBuffer));
bindUIQuad((vec2[6]) {
{ inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
{ inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
},(uint8_t[4]) { redColor, 1.0f }, loadWindow[3].highlight);

loadWindow[3].textPos = (vec2){inputLeftW, inputBotH + letterH };
loadWindow[3].text = malloc(sizeof(char) * strlen("cancel"));
strcpy(loadWindow[3].text, "cancel");
}
*/
	UIStructBufs[markersListWindowT] = batchUI(markersWindow, markersWindowSize);
    }

    // entity window
    {
	UIRect2* entitiesWindow;
	int entitiesWindowSize;

	entitiesWindowSize = 1 + entityTypesCounter;
	entitiesWindow = calloc(entitiesWindowSize, sizeof(UIRect2));    
    
	float rW = -1000.0f;
	
	// background
	for(int i=0;i<entityTypesCounter;i++){
	    float inputLeftW = -1.0f;
	    float inputRightW = -1.0f + (strlen(entityTypeStr[i])+1) * letterW;

	    rW = fmaxf(rW, -1.0f + (strlen(entityTypeStr[i])+1) * letterW);

	    float inputTopH = 1.0f - ((i+1) * letterH);
	    float inputBotH = inputTopH - letterH;
    
	    entitiesWindow[i+1] = (UIRect2){
		.pos = {
		    { inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
		    { inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
		},
            .lb = {inputLeftW, inputBotH}, .rt = {inputRightW, inputTopH},
					    .c = { blackColor, 1.0f }
	    };

	    switch (i)
	    {
	    case(playerEntityT): {
		entitiesWindow[i+1].onClick = setPlayerEntityBrush;

		break;
	    }
	    default:
		break;
	    }

	    entitiesWindow[i+1].highlight = malloc(sizeof(MeshBuffer));
	    bindUIQuad((vec2[6]) {
		    { inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
		    { inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
		},(uint8_t[4]) { redColor, 1.0f }, entitiesWindow[i+1].highlight);

	    entitiesWindow[i+1].textPos = (vec2){inputLeftW, inputBotH + letterH };
	    entitiesWindow[i+1].text = malloc(sizeof(char) * strlen(entityTypeStr[i]));
	    strcpy(entitiesWindow[i+1].text, entityTypeStr[i]);
	}
    
	{
	    float lW = -1.0f;
	    float h = 1.0f - ((entityTypesCounter+1) * letterH);
	    entitiesWindow[0] = (UIRect2){ .pos = {
		    { lW, h }, { lW, 1.0f }, { rW, 1.0f },
		    { lW, h }, { rW, 1.0f }, { rW, h }
		},
					  .c = { blackColor, 1.0f },
					  .lb = { lW, h}, .rt = {rW, 1.0f}
	    };

	    entitiesWindow[0].textPos = (vec2){ -1.0f, 1.0f };
	    entitiesWindow[0].text = malloc(sizeof(char) * strlen("Entities:"));
	    strcpy(entitiesWindow[0].text, "Entities:");
	}
	
	UIStructBufs[entityWindowT] = batchUI(entitiesWindow, entitiesWindowSize);
    }

    
    // loadWindow
    {
	UIRect2* loadWindow;
	int loadWindowSize;
    
	float w = 0.3f;
	float h = 0.1f + letterH;

	// background
	{
	    loadWindow = calloc(4, sizeof(UIRect2));
	    loadWindowSize = 4;
    
	    loadWindow[0] = (UIRect2){ .pos = {
		    { -w, -h }, { -w, h }, { w, h },
		    { -w, -h }, { w, h }, { w, -h }
		},
				       .c = { blackColor, 1.0f },
				       .lb = {w, h}, .rt = {-w, -h}
	    };

	    loadWindow[0].textPos = (vec2){ -w, h + letterH };
	    loadWindow[0].text = malloc(sizeof(char) * strlen("Map loading"));
	    strcpy(loadWindow[0].text, "Map loading");
	}

	// input
	{
	    float inputLeftW = -w + (strlen("Save name:")+1) * letterW;
	    float inputRightW = w - 0.03f;

	    float inputTopH = h - 0.02f;
	    float inputBotH = 0.0f + h/2;
    
	    loadWindow[1] = (UIRect2){ .pos = {
		    { inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
		    { inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
		},
				       .lb = {inputLeftW, inputBotH}, .rt = {inputRightW, inputTopH},
				       .c = { greenColor, 1.0f }
	    };
      
	    loadWindow[1].input = calloc(1, sizeof(TextInput2));
	    loadWindow[1].input->limit = 15;
	    loadWindow[1].input->relatedUIRect = &loadWindow[2];

	    loadWindow[1].textPos = (vec2){inputLeftW - (strlen("Save name:")+1) * letterW, inputBotH + letterH};
	    loadWindow[1].text = malloc(sizeof(char) * strlen("Save name:"));
	    strcpy(loadWindow[1].text, "Save name:");
	}

	// load button
	{
	    float inputLeftW = -w + 0.1f;
	    float inputRightW = -w + 0.1f + (strlen("load")+1) * letterW;

	    float inputTopH = -h + letterH + 0.02f;
	    float inputBotH = -h + 0.02f;
    
	    loadWindow[2] = (UIRect2){ .pos = {
		    { inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
		    { inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
		},
				       .lb = {inputLeftW, inputBotH}, .rt = {inputRightW, inputTopH},
				       .c = { greenColor, 1.0f }
	    };

	    loadWindow[2].onClick = loadMapUI;

	    loadWindow[2].highlight = malloc(sizeof(MeshBuffer));
	    bindUIQuad((vec2[6]) {
		    { inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
		    { inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
		},(uint8_t[4]) { redColor, 1.0f }, loadWindow[2].highlight);

	    loadWindow[2].textPos = (vec2){inputLeftW, inputBotH + letterH };
	    loadWindow[2].text = malloc(sizeof(char) * strlen("load"));
	    strcpy(loadWindow[2].text, "load");
	}

	// cancel button
	{
	    float inputLeftW = w - 0.1f - (strlen("cancel")+1) * letterW;
	    float inputRightW = w - 0.1f;

	    float inputTopH = -h + letterH + 0.02f;
	    float inputBotH = -h + 0.02f;
    
	    loadWindow[3] = (UIRect2){ .pos = {
		    { inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
		    { inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
		},
				       .lb = {inputLeftW, inputBotH}, .rt = {inputRightW, inputTopH},
				       .c = { greenColor, 1.0f }
	    };

	    loadWindow[3].onClick = clearCurrentUI;
      
	    loadWindow[3].highlight = malloc(sizeof(MeshBuffer));
	    bindUIQuad((vec2[6]) {
		    { inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
		    { inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
		},(uint8_t[4]) { redColor, 1.0f }, loadWindow[3].highlight);

	    loadWindow[3].textPos = (vec2){inputLeftW, inputBotH + letterH };
	    loadWindow[3].text = malloc(sizeof(char) * strlen("cancel"));
	    strcpy(loadWindow[3].text, "cancel");
	}

	UIStructBufs[loadWindowT] = batchUI(loadWindow, loadWindowSize);
    }

    // saveWindow
    {
	UIRect2* saveWindow;
	int saveWindowSize;
 
	float w = 0.3f;
	float h = 0.1f + letterH;

	// background
	{
	    saveWindow = calloc(4, sizeof(UIRect2));
	    saveWindowSize = 4;
        
	    saveWindow[0] = (UIRect2){ .pos = {
		    { -w, -h }, { -w, h }, { w, h },
		    { -w, -h }, { w, h }, { w, -h }
		},
				       .c = { blackColor, 1.0f },
				       .lb = {w, h}, .rt = {-w, -h}
	    };

	    saveWindow[0].textPos = (vec2){ -w, h + letterH };
	    saveWindow[0].text = malloc(sizeof(char) * strlen("Map saving"));
	    strcpy(saveWindow[0].text, "Map saving");
	}

	// input
	{
	    float inputLeftW = -w + (strlen("Save name:")+1) * letterW;
	    float inputRightW = w - 0.03f;

	    float inputTopH = h - 0.02f;
	    float inputBotH = 0.0f + h/2;
    
	    saveWindow[1] = (UIRect2){ .pos = {
		    { inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
		    { inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
		},
				       .lb = {inputLeftW, inputBotH}, .rt = {inputRightW, inputTopH},
				       .c = { greenColor, 1.0f }
	    };

	    saveWindow[1].input = calloc(1, sizeof(TextInput2));
	    saveWindow[1].input->limit = 15;
	    saveWindow[1].input->relatedUIRect = &saveWindow[2];

	    saveWindow[1].textPos = (vec2){inputLeftW - (strlen("Save name:")+1) * letterW, inputBotH + letterH};
	    saveWindow[1].text = malloc(sizeof(char) * strlen("Save name:"));
	    strcpy(saveWindow[1].text, "Save name:");
	}

	// save button
	{
	    float inputLeftW = -w + 0.1f;
	    float inputRightW = -w + 0.1f + (strlen("save")+1) * letterW;

	    float inputTopH = -h + letterH + 0.02f;
	    float inputBotH = -h + 0.02f;
    
	    saveWindow[2] = (UIRect2){ .pos = {
		    { inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
		    { inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
		},
				       .lb = {inputLeftW, inputBotH}, .rt = {inputRightW, inputTopH},
				       .c = { greenColor, 1.0f }
	    };

	    saveWindow[2].onClick = saveMapUI;
      
	    saveWindow[2].highlight = malloc(sizeof(MeshBuffer));
	    bindUIQuad((vec2[6]) {
		    { inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
		    { inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
		},(uint8_t[4]) { redColor, 1.0f }, saveWindow[2].highlight);

	    saveWindow[2].textPos = (vec2){inputLeftW, inputBotH + letterH };
	    saveWindow[2].text = malloc(sizeof(char) * strlen("save"));
	    strcpy(saveWindow[2].text, "save");
	}

	// cancel button
	{
	    float inputLeftW = w - 0.1f - (strlen("cancel")+1) * letterW;
	    float inputRightW = w - 0.1f;

	    float inputTopH = -h + letterH + 0.02f;
	    float inputBotH = -h + 0.02f;
    
	    saveWindow[3] = (UIRect2){ .pos = {
		    { inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
		    { inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
		},
				       .lb = {inputLeftW, inputBotH}, .rt = {inputRightW, inputTopH},
				       .c = { greenColor, 1.0f }
	    };

	    saveWindow[3].onClick = clearCurrentUI;

	    saveWindow[3].highlight = malloc(sizeof(MeshBuffer));
	    bindUIQuad((vec2[6]) {
		    { inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
		    { inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
		},(uint8_t[4]) { redColor, 1.0f }, saveWindow[3].highlight);

	    saveWindow[3].textPos = (vec2){inputLeftW, inputBotH + letterH };
	    saveWindow[3].text = malloc(sizeof(char) * strlen("cancel"));
	    strcpy(saveWindow[3].text, "cancel");
	}

	UIStructBufs[saveWindowT] = batchUI(saveWindow, saveWindowSize);
    }
  
    // circle buf
    {
	gizmosGeom[rotationMode -1] = calloc(gizmosNum[rotationMode -1], sizeof(VPair));
	gizmosAABB[rotationMode -1][0] = calloc(gizmosNum[rotationMode -1], sizeof(vec3));
	gizmosAABB[rotationMode -1][1] = calloc(gizmosNum[rotationMode -1], sizeof(vec3));

	gizmosPaddings[rotationMode -1] = calloc(gizmosNum[rotationMode -1], sizeof(VPair));
       
	for(int i=0;i<3;i++){
	    gizmosGeom[rotationMode -1][i].vertexNum = 6;
	    gizmosGeom[rotationMode -1][i].attrSize = 3;

	    float size = .15f;

	    if(i==0){ // x
		float plane[] = {
		    .0f,  -size,  -size,
		    .0f,  -size,  size,
		    .0f,  size,  size,

		    .0f,  -size,  -size,
		    .0f,  size,  size,
		    .0f,  size,  -size,
		};

		gizmosPaddings[rotationMode -1][i] = (vec3){ -.5f, .0f, .0f }; 

		gizmosGeom[rotationMode -1][i].vBuf = malloc(sizeof(plane));
		memcpy(gizmosGeom[rotationMode -1][i].vBuf, plane, sizeof(plane));
	    }else if(i == 1){ // y
		float plane[] = {
		    -size, .0f, -size,
		    -size, .0f, size,
		    size, .0f, size,

		    -size, .0f, -size,
		    size, .0f, size,
		    size, .0f, -size,
		};

		gizmosPaddings[rotationMode -1][i] = (vec3){ .0f, -.5f, .0f }; 
	
		gizmosGeom[rotationMode -1][i].vBuf = malloc(sizeof(plane));
		memcpy(gizmosGeom[rotationMode -1][i].vBuf, plane, sizeof(plane));
	    }else if(i == 2){ // z
		float plane[] = {
		    -size, -size, .0f,
		    size, size, .0f,
		    size, -size, .0f,
	  
		    -size, -size, .0f,
		    -size, size, .0f,
		    size, size, .0f,

		};

		gizmosPaddings[rotationMode -1][i] = (vec3){ .0f, .0f, .5f }; 

		gizmosGeom[rotationMode -1][i].vBuf = malloc(sizeof(plane));
		memcpy(gizmosGeom[rotationMode -1][i].vBuf, plane, sizeof(plane));
	    }
      
	    glGenVertexArrays(1, &gizmosGeom[rotationMode -1][i].VAO);
	    glBindVertexArray(gizmosGeom[rotationMode -1][i].VAO);

	    glGenBuffers(1, &gizmosGeom[rotationMode -1][i].VBO);
	    glBindBuffer(GL_ARRAY_BUFFER, gizmosGeom[rotationMode -1][i].VBO);

	    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 3, gizmosGeom[rotationMode -1][i].vBuf, GL_STATIC_DRAW);

	    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	    glEnableVertexAttribArray(0);
    
	    glBindBuffer(GL_ARRAY_BUFFER, 0);
	    glBindVertexArray(0);
	}

    }
  
    // translation gizmos
    {
	float h1 = 1.0f / 9.0f;

	gizmosGeom[moveMode -1] = calloc(gizmosNum[moveMode -1], sizeof(VPair));
	gizmosAABB[moveMode -1][0] = calloc(gizmosNum[moveMode -1], sizeof(vec3));
	gizmosAABB[moveMode -1][1] = calloc(gizmosNum[moveMode -1], sizeof(vec3));

	gizmosPaddings[moveMode -1] = calloc(gizmosNum[moveMode -1], sizeof(VPair));
      
	for(int i=0;i<gizmosNum[moveMode -1];i++){
	    //      printf("AAA %d \n", i);
      
	    gizmosGeom[moveMode -1][i].vertexNum = 6;
	    gizmosGeom[moveMode -1][i].attrSize = 3;

	    float sInterPlane =  -(0.5f / 4.0f);
	    float eInterPlane =  0.5f / 4.0f;

	    if(i+1==XCircle){
		float transl[] = {
		    // X axis
		    -0.5f, -(h1/2.0f), 0.0f,

		    0.5f, -(h1/2.0f), 0.0f,
		    0.5f, h1/2.0f, 0.0f,

		    -.5f, -(h1/2.0f), 0.0f,
		    .5f, h1/2.0f, 0.0f,
		    -.5f, h1/2.0f, 0.0f,
		};

		gizmosPaddings[moveMode -1][i] = (vec3){.5f,.0f,.0f};
	
		gizmosGeom[moveMode -1][i].vBuf = malloc(sizeof(transl));
		memcpy(gizmosGeom[moveMode -1][i].vBuf,transl,sizeof(transl));
	    }else if(i+1==YCircle){ // y
		float transl[] = {
		    // Y axis
		    -(h1/2.0f), -0.5f, 0.0f,
		    -(h1/2.0f), 0.5f,  0.0f,
		    h1/2.0f, 0.5f, 0.0f,

		    -(h1/2.0f), -.5f, 0.0f,
		    h1/2.0f, .5f, 0.0f,
		    h1/2.0f,-.5f, 0.0f,
		};

		gizmosPaddings[moveMode -1][i] = (vec3){.0f,.5f,.0f};

		gizmosGeom[moveMode -1][i].vBuf = malloc(sizeof(transl));
		memcpy(gizmosGeom[moveMode -1][i].vBuf,transl,sizeof(transl));
	    }else if(i+1==ZCircle){ // z
		float transl[] = {
		    // Z axis
		    0.0f, -(h1/2.0f), -.5f,
		    0.0f, -(h1/2.0f), .5f,
		    0.0f, h1/2.0f, .5f,

		    0.0f, -(h1/2.0f), -.5f,
		    0.0f, h1/2.0f, .5f,
		    0.0f, h1/2.0f, -.5f,
		};

		gizmosPaddings[moveMode -1][i] = (vec3){.0f,.0f,-.5f};
	
		gizmosGeom[moveMode -1][i].vBuf = malloc(sizeof(transl));
		memcpy(gizmosGeom[moveMode -1][i].vBuf,transl,sizeof(transl));
	    }else if(i+1==XYPlane){ // xy
		float transl[] = {
		    sInterPlane, sInterPlane, 0.0f,
		    sInterPlane, eInterPlane, 0.0f,
		    eInterPlane,  eInterPlane, 0.0f,

		    sInterPlane, sInterPlane, 0.0f,
		    eInterPlane, eInterPlane, 0.0f,
		    eInterPlane, sInterPlane, 0.0f,
		};

		gizmosPaddings[moveMode -1][i] = (vec3){.5f,.5f,.0f};
	
		gizmosGeom[moveMode -1][i].vBuf = malloc(sizeof(transl));
		memcpy(gizmosGeom[moveMode -1][i].vBuf,transl,sizeof(transl));
	    }else if(i+1==ZYPlane){ // zy
		float transl[] = {
		    0.0f, sInterPlane, sInterPlane,
		    0.0f, sInterPlane, eInterPlane,
		    0.0f, eInterPlane,  eInterPlane,

		    0.0f, sInterPlane, sInterPlane,
		    0.0f, eInterPlane, eInterPlane,
		    0.0f, eInterPlane, sInterPlane,
		};

		gizmosPaddings[moveMode -1][i] = (vec3){.0f,.5f,-.5f };

		gizmosGeom[moveMode -1][i].vBuf = malloc(sizeof(transl));
		memcpy(gizmosGeom[moveMode -1][i].vBuf,transl,sizeof(transl));
	    }else if(i+1==XZPlane){ // xz
		float transl[] = {
		    // XZ axis
		    sInterPlane, 0.0f, sInterPlane,
		    sInterPlane, 0.0f, eInterPlane,
		    eInterPlane, 0.0f, eInterPlane,

		    sInterPlane, 0.0f, sInterPlane,
		    eInterPlane, 0.0f, eInterPlane,
		    eInterPlane, 0.0f, sInterPlane,
		};

		gizmosPaddings[moveMode -1][i] = (vec3){.5f,.0f,-.5f};

		gizmosGeom[moveMode -1][i].vBuf = malloc(sizeof(transl));
		memcpy(gizmosGeom[moveMode -1][i].vBuf,transl,sizeof(transl));
	    }
      
	
	    //      for(int i2=0;i2<6*3;i2++){
	    //	printf("%f ", gizmosGeom[moveMode -1][i].vBuf[i2]);
	    //      }

	    //      printf("\n");

	    glGenVertexArrays(1, &gizmosGeom[moveMode -1][i].VAO);
	    glBindVertexArray(gizmosGeom[moveMode -1][i].VAO);

	    glGenBuffers(1, &gizmosGeom[moveMode -1][i].VBO);
	    glBindBuffer(GL_ARRAY_BUFFER, gizmosGeom[moveMode -1][i].VBO);

	    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 3, gizmosGeom[moveMode -1][i].vBuf, GL_STATIC_DRAW);
    
	    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	    glEnableVertexAttribArray(0);
      
	    glBindBuffer(GL_ARRAY_BUFFER, 0);
	    glBindVertexArray(0);
	}
    
    }

  
    editorProj = perspective(rad(fov), windowW / windowH, 0.01f, 1000.0f);

  
    // net tile
    {
	float netTileVerts[] = {
	    0.0f, 0.0f, 0.0f ,
	    bBlockW, 0.0f, 0.0f ,
      
	    0.0f, 0.0f, 0.0f ,
	    0.0f, 0.0f, bBlockD ,
      
	    0.0f, 0.0f, bBlockD ,
	    bBlockW, 0.0f, bBlockD ,
      
	    bBlockW, 0.0f, 0.0f ,
	    bBlockW, 0.0f, bBlockD 
	};

	float* buf = malloc(sizeof(float) * gridX * gridZ * 8 * 3);
	netTileAABB = malloc(sizeof(vec2) * gridX * gridZ * 2);
	//    netTileSize = 0;
    
	for (int z = 0; z < gridZ; z++) {
	    for (int x = 0; x < gridX; x++) {
		int index = (z * gridX + x) * 8 * 3;
		vec3 tile = xyz_indexesToCoords(x,0.0f,z);

		for(int i=0;i<8*3;i+=3){
		    if(i==0){
			// lb
			netTileAABB[netTileSize] = (vec2){ netTileVerts[i + 0] + tile.x, netTileVerts[i + 2] + tile.z };
			netTileSize++;
		    }
		    else if (i == 21) {
			// rt
			netTileAABB[netTileSize] = (vec2){ netTileVerts[i + 0] + tile.x, netTileVerts[i + 2] + tile.z };
			netTileSize++;
		    }

		    //	  printf("i = %d \n", i);
	    
		    buf[index + i] = netTileVerts[i + 0] + tile.x;
		    buf[index + i + 1] = netTileVerts[i + 1] + tile.y;
		    buf[index + i + 2] = netTileVerts[i + 2] + tile.z;
		}
	    }
	}
    
	glGenVertexArrays(1, &netTile.VAO);
	glBindVertexArray(netTile.VAO);

	glGenBuffers(1, &netTile.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, netTile.VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * gridX * gridZ * 8 * 3, buf, GL_STATIC_DRAW);
  
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	free(buf);
    }
  
    // obj menu    
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
}


void editorEvents(SDL_Event event){
    if (event.type == SDL_KEYDOWN) {
	if(event.key.keysym.scancode == SDL_SCANCODE_F5) {
	    if(curUIBuf.rects != UIStructBufs[saveWindowT]->rects){
		memcpy(&curUIBuf, UIStructBufs[saveWindowT], sizeof(UIBuf));
	
		selectedTextInput2 = curUIBuf.rects[1].input;
		textInputCursorMat.x = curUIBuf.rects[1].pos[0].x;
		textInputCursorMat.z = curUIBuf.rects[1].pos[0].z;
	    }else{
		clearCurrentUI();
	    }
	}else if(event.key.keysym.scancode == SDL_SCANCODE_L){
	    if(curUIBuf.rects != UIStructBufs[entityWindowT]->rects){
		memcpy(&curUIBuf, UIStructBufs[entityWindowT], sizeof(UIBuf));
	    }else{
		clearCurrentUI();
	    }
	}else if(event.key.keysym.scancode == SDL_SCANCODE_V){
	    if(curUIBuf.rects != UIStructBufs[markersListWindowT]->rects){
		memcpy(&curUIBuf, UIStructBufs[markersListWindowT], sizeof(UIBuf));
	    }else{
		clearCurrentUI();
	    }
	}else if(event.key.keysym.scancode == SDL_SCANCODE_F9) {
	    if(curUIBuf.rects != UIStructBufs[loadWindowT]->rects){
		memcpy(&curUIBuf, UIStructBufs[loadWindowT], sizeof(UIBuf));
	
		selectedTextInput2 = curUIBuf.rects[1].input;
		textInputCursorMat.x = curUIBuf.rects[1].pos[0].x;
		textInputCursorMat.z = curUIBuf.rects[1].pos[0].z;
	    }
	    else{
		clearCurrentUI();
	    }
	}else if(event.key.keysym.scancode == SDL_SCANCODE_RETURN && curUIBuf.rects){
	    if(curUIBuf.rects == UIStructBufs[loadWindowT]->rects || curUIBuf.rects == UIStructBufs[saveWindowT]->rects || curUIBuf.rects == UIStructBufs[attachSaveWindowT]->rects){
		printf("run onclick\n");
		curUIBuf.rects[2].onClick();
	    }
	}
	/* 
	   else if(dialogViewer.open){
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
	   }
	   else{
	   if(selectedTextInput2){

	   }
	
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
	   }*/
	//  }
	else if(curUIBuf.rectsSize == 0){       
	    switch (event.key.keysym.scancode) {
	    case(SDL_SCANCODE_F2):{
		hints = !hints;
	  
		break;
	    }
	    case(SDL_SCANCODE_Q): {
		curCamera->pos.y += .1f ;
	  
		break;
	    }case(SDL_SCANCODE_U):{
		 lightView = !lightView;
		 break;
	     }
	    case(SDL_SCANCODE_F): {
		// do i need focuse something without cursorMode
		if(false){
		    if(!mouse.focusedThing){
			if(mouse.selectedType == mouseLightT || mouse.selectedType == mousePlaneT || mouse.selectedType == mouseModelT){
			    mouse.focusedThing = mouse.selectedThing;
			    mouse.focusedType = mouse.selectedType;
			}
		    }else{
			mouse.focusedThing = NULL;
			mouse.focusedType = 0;
		    }
		}
	
		break;
	    }
	    case(SDL_SCANCODE_E): {
		curCamera->pos.y -= .1f;

		break;
	    }
	    case(SDL_SCANCODE_X): {
		const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

		if(cursorMode){
		    Model* model = NULL;
		    Matrix* mat = NULL;
		    Light* light = NULL;
    
		    if (mouse.selectedType == mouseModelT) { 
			model = (Model*)mouse.selectedThing; 
			mat = &model->mat; 
		    }
		    else if (mouse.selectedType == mousePlaneT) {
			Picture* plane = (Picture*)mouse.selectedThing;
			mat = &plane->mat;
		    }else if(mouse.selectedType == mouseLightT){
			light = (Light*)mouse.selectedThing;
			mat = &light->mat; 
		    }

		    // TODO: Make rotation reset
		    if(false && cursorMode == rotationMode){
			mat->m[0] = 0;
			mat->m[1] = 0;
			mat->m[2] = 0;

			mat->m[4] = 0;
			mat->m[5] = 0;
			mat->m[6] = 0;
	    
			mat->m[8] = 0;
			mat->m[9] = 0;
			mat->m[10] = 0;
	    
			if(model){
			    calculateModelAABB(model);
			}

			if(light){
			    uniformLights();
			    rerenderShadowsForAllLights();
			    //	      rerenderShadowForLight(light->id);
			    calculateAABB(light->mat, cube.vBuf, cube.vertexNum, cube.attrSize, &light->lb, &light->rt);
			}
		    }

		    // TODO: Make more clever way to alling object with net
		    if(false){
			if(light){
			    mat->m[13] = curFloor;
			    calculateAABB(light->mat, cube.vBuf, cube.vertexNum, cube.attrSize, &light->lb, &light->rt);

			    uniformLights();
	    
			    //	      if(light->type == shadowLightT){
			    rerenderShadowsForAllLights();
			    //		rerenderShadowForLight(light->id);
			    //	      }
			}

			if(model){
			    printf("%s %f \n", loadedModels1D[model->name].name ,loadedModels1D[model->name].modelSizes.y);
	    
			    mat->m[13] = curFloor + loadedModels1D[model->name].modelSizes.y;
			    calculateModelAABB(model);
			}
		    }
		}else{
		    if(mouse.brushThing || mouse.brushType){
			if (mouse.brushType == mouseBlockBrushT) {
			    TileBlock* block = (TileBlock*) mouse.brushThing;
			    free(block);
			}

			if (mouse.brushType == mouseWallBrushT && mouse.brushThing) {
			    free(mouse.brushThing);
			}

			mouse.brushThing = NULL;
			mouse.brushType = 0;
		    }
		}

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
		  // set focused to object under cursor in time pressed R
		  if(cursorMode &&
		     (mouse.selectedType == mouseModelT || mouse.selectedType == mouseLightT) && mouse.focusedThing != mouse.selectedThing){

		      initGizmosAABBFromSelected();
	    
		      cursorMode = rotationMode;
		  }else if(cursorMode == rotationMode){
		      cursorMode = moveMode;
		  }else if(cursorMode == moveMode){
		      cursorMode = rotationMode;
		  }
      
		  const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

		  if (mouse.selectedType == mouseBlockT || mouse.brushType == mouseBlockBrushT) {
		      if(!currentKeyStates[SDL_SCANCODE_X] && !currentKeyStates[SDL_SCANCODE_Y] && !currentKeyStates[SDL_SCANCODE_Z]){
			  TileBlock* block = NULL;
		  
			  if(mouse.selectedType == mouseBlockT){
			      block = (TileBlock*) mouse.selectedThing;
			  }else{
			      block = (TileBlock*) mouse.brushThing;
			  }
		  
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
	      
			  int index = block->rotateAngle / 90;
			  block->mat.m[12] += rotationBlock[index][0];
			  block->mat.m[14] += rotationBlock[index][1];
		
			  calculateAABB(block->mat, blocksVPairs[block->type].pairs[0].vBuf, blocksVPairs[block->type].pairs[0].vertexNum, blocksVPairs[block->type].pairs[0].attrSize, &block->lb, &block->rt);
		      }

		  }

		  break;
	      }case(SDL_SCANCODE_2):{
		   if(mouse.brushThing && mouse.brushType == mouseBlockBrushT){
		       free(mouse.brushThing);
		       mouse.brushThing = NULL;
		   }else if(mouse.brushType == mouseWallBrushT){
		       WallType* type = mouse.brushThing;

		       if(type == LRWallT){
			   mouse.brushType = 0;
			   free(mouse.brushThing);
			   mouse.brushThing = NULL;
		       }else{
			   *type = LRWallT;
		       }
		   }else{
		       mouse.brushType = mouseWallBrushT;

		       WallType* type = malloc(sizeof(WallType));
		       *type = LRWallT;
		 
		       mouse.brushThing = type;
		   }
	       
		   break;
	       }case(SDL_SCANCODE_1):{
		    if(mouse.brushThing && mouse.brushType == mouseBlockBrushT){
			free(mouse.brushThing);
			mouse.brushThing = NULL;
		    }else if(mouse.brushType == mouseWallBrushT){
			WallType* type = mouse.brushThing;

			if(type == normWallT){
			    mouse.brushType = 0;
			    free(mouse.brushThing);
			    mouse.brushThing = NULL;
			}else{
			    *type = normWallT;
			}
		    }else{
			mouse.brushType = mouseWallBrushT;

			WallType* type = malloc(sizeof(WallType));
			*type = normWallT;
		 
			mouse.brushThing = type;
		    }
	      
		    break;
		}case(SDL_SCANCODE_3):{
		     if(mouse.brushThing && mouse.brushType == mouseBlockBrushT){
			 free(mouse.brushThing);
			 mouse.brushThing = NULL;
		     }else if(mouse.brushType == mouseWallBrushT){
			 WallType* type = mouse.brushThing;

			 if(type == LWallT){
			     mouse.brushType = 0;
			     free(mouse.brushThing);
			     mouse.brushThing = NULL;
			 }else{
			     *type = LWallT;
			 }
		     }else{
			 mouse.brushType = mouseWallBrushT;

			 WallType* type = malloc(sizeof(WallType));
			 *type = LWallT;
		 
			 mouse.brushThing = type;
		     }
	      
		     break;
		 }case(SDL_SCANCODE_4):{
		      if(mouse.brushThing && mouse.brushType == mouseBlockBrushT){
			  free(mouse.brushThing);
			  mouse.brushThing = NULL;
		      }else if(mouse.brushType == mouseWallBrushT){
			  WallType* type = mouse.brushThing;

			  if(type == RWallT){
			      mouse.brushType = 0;
			      free(mouse.brushThing);
			      mouse.brushThing = NULL;
			  }else{
			      *type = RWallT;
			  }
		      }else{
			  mouse.brushType = mouseWallBrushT;

			  WallType* type = malloc(sizeof(WallType));
			  *type = RWallT;
		 
			  mouse.brushThing = type;
		      }
	      
		      break;
		  }case(SDL_SCANCODE_5):{
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
		   }case(SDL_SCANCODE_6):{
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
		    }case(SDL_SCANCODE_7):{
			 if(mouse.brushThing && mouse.brushType == mouseBlockBrushT){
			     free(mouse.brushThing);
			     mouse.brushThing = NULL;
			 }else if(mouse.brushType == mouseWallBrushT){
			     WallType* type = mouse.brushThing;

			     if(type == halfWallT){
				 mouse.brushType = 0;
				 free(mouse.brushThing);
				 mouse.brushThing = NULL;
			     }else{
				 *type = halfWallT;
			     }
			 }else{
			     mouse.brushType = mouseWallBrushT;

			     WallType* type = malloc(sizeof(WallType));
			     *type = halfWallT;
		 
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
		 const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
		 if(currentKeyStates[SDL_SCANCODE_LCTRL] && dofPercent < 1.0f){
		     dofPercent += 0.01f;
		 }else{
		     if (curFloor < gridY - 1) {
			 curFloor++;
		     }
		 }

		 printf("%d \n", curFloor);
		 

		 break; 
	     }
	    case(SDL_SCANCODE_MINUS): {
		const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
		if(currentKeyStates[SDL_SCANCODE_LCTRL] && dofPercent > 0.0f){
		    dofPercent -= 0.01f;
		}else{

		    if (curFloor != 0) {
			curFloor--;
		    }
		}

		break;
	    }
		case(SDL_SCANCODE_LSHIFT):{
			if (mouse.clickL) {
				if (selectedCollisionTileIndex != -1 &&
					entityStorage[playerEntityT] != NULL) {

					float div = 1.0f / 3.0f;
					int h = acceptedCollisionTilesAABB[selectedCollisionTileIndex].lb.y - 0.1f;

					vec2i dist = { acceptedCollisionTilesAABB[selectedCollisionTileIndex].lb.x / div,
						   acceptedCollisionTilesAABB[selectedCollisionTileIndex].lb.z / div };

					vec2i start = { (entityStorage[playerEntityT]->mat.m[12] - div / 2.0f) / div,
							(entityStorage[playerEntityT]->mat.m[14] - div / 2.0f) / div };

					// astar
					{
					    //bool closedList[gridZ*3][gridX*3] = {{ 0 }};
//					    AstarCell cellsDetails[gridZ][gridX] = {{ 0 }};

					    bool** closedList = malloc(sizeof(bool*) * gridZ*3);

					    for(int z=0;z<gridZ*3;z++){
						closedList[z] = malloc(sizeof(bool) * gridX*3);
					    }
					    
					    AstarCell** cellsDetails = malloc(sizeof(AstarCell*) * gridZ*3);

					    for(int z=0;z<gridZ*3;z++){
						cellsDetails[z] = malloc(sizeof(AstarCell) * gridX*3);
					    }

					    AstarOpenCell* openCells = malloc(sizeof(AstarOpenCell) * (gridZ*3) * (gridX * 3));
					    int openCellsTop = 0;

						bool pathFound = false;

					    for (int z = 0; z < gridZ*3; z++) {
						for (int x = 0; x < gridX*3; x++) {
						    cellsDetails[z][x].g = FLT_MAX;
						    cellsDetails[z][x].g = FLT_MAX;
						    cellsDetails[z][x].f = FLT_MAX;
						    cellsDetails[z][x].parX = -1;
						    cellsDetails[z][x].parZ = -1;
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
						openCells[curIndex] = openCells[--openCellsTop];
						
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

								printf("pre dest: %d %d dest: %d %d \n", cur.z, cur.x, dist.z, dist.x);
							    
							    cellsDetails[mCur.z][mCur.x].parX = cur.x;
							    cellsDetails[mCur.z][mCur.x].parZ = cur.z;
							    
							    printf("Destination found!: ");
							    pathFound = true;

							    int diZ = dist.z;
							    int diX = dist.x;

							    int pathSize = 0;

							    /*while(cellsDetails[diZ][diX].parZ != dist.z &&
							      cellsDetails[diZ][diX].parX != dist.x){*/

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

							    printf("start: %d %d -> ", start.z, start.x);

							    for(int i=pathSize-1;i>=0;i--){
								printf("%d %d -> ", path[i].z, path[i].x);
							    }

//							    printf("end: %d %d", dist.z, dist.x);

							    vec3* buf = malloc(sizeof(vec3) * ((pathSize+1)*2));

							    buf[0] = (vec3){ start.x * div + div/2.0f, (float)h + 0.2f, start.z * div + div/2.0f };
							    buf[1] = (vec3){ path[pathSize-1].x * div + div/2.0f, (float)h + 0.2f, path[pathSize-1].z * div + div/2.0f };

							    int index = 2;
							    for(int i=pathSize-1;i>0;i--){
								buf[index] = (vec3){ path[i].x * div + div/2.0f, (float)h + 0.2f, path[i].z * div + div/2.0f };
								index++;
								
								buf[index] = (vec3){ path[i-1].x * div + div/2.0f, (float)h + 0.2f, path[i-1].z * div + div/2.0f };
								index++;
							    }

							    for(int i=0;i<(pathSize+1)*2;i++){
								printf("%f %f %f\n", argVec3(buf[i]));
							    }

							    glBindVertexArray(lastFindedPath.VAO); 
							    glBindBuffer(GL_ARRAY_BUFFER, lastFindedPath.VBO);

							    lastFindedPath.VBOsize = (pathSize+1)*2;

							    glBufferData(GL_ARRAY_BUFFER,
									 sizeof(vec3) * ((pathSize+1)*2), buf, GL_STATIC_DRAW);

							    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
							    glEnableVertexAttribArray(0);

							    glBindBuffer(GL_ARRAY_BUFFER, 0);
							    glBindVertexArray(0);
		  
							    free(buf);
							    free(path);
							    
							    break;
							}else if(!closedList[mCur.z][mCur.x]
								 && !collisionGrid[h][mCur.z][mCur.x]){
							    float gNew = cellsDetails[cur.z][cur.x].g + 1.0f;
							    float hNew = sqrtf((mCur.z - dist.z) * (mCur.z - dist.z) + (mCur.x - dist.x) * (mCur.x - dist.x));
							    float fNew = gNew + hNew;

							    if(cellsDetails[mCur.z][mCur.x].f == FLT_MAX ||
							       cellsDetails[mCur.z][mCur.x].f > fNew){

								openCells[openCellsTop] = (AstarOpenCell){
								    fNew, .z = mCur.z, .x = mCur.x
								};
								openCellsTop++;
								
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

						if (pathFound) break;

					for(int z=0;z<gridZ*3;z++){
					    free(closedList[z]);
					}

					free(closedList);

					for(int z=0;z<gridZ*3;z++){
					    free(cellsDetails[z]);
					}

					free(cellsDetails);
					free(openCells);
					}


					printf("start: %d %d dist: %d %d \n", argVec2(start), argVec2(dist));

				}
			}

		break;
	    }
	    case(SDL_SCANCODE_V): {
	  
		break;
	    }
	    case(SDL_SCANCODE_N): {
		Light* light = NULL;
	    
		if(mouse.focusedType == mouseLightT){
		    light = (Light*) mouse.focusedThing;
		}else if(mouse.selectedType == mouseLightT){
		    light = (Light*) mouse.selectedThing;
		}

		if(light){
		    light->off = !light->off;
		    uniformLights();

		    if(light->type == dirLightShadowT){
			rerenderShadowForLight(light->id);
		    }
		}
				  
		break;
	    }      /*case(SDL_SCANCODE_M): {
		     Light* light = NULL;
	    
		     if(mouse.focusedType == mouseLightT){
		     light = (Light*) mouse.focusedThing;
		     }else if(mouse.selectedType == mouseLightT){
		     light = (Light*) mouse.selectedThing;
		     }

		     if(light){
		     light->r = rand() % 255;
		     light->g = rand() % 255;
		     light->b = rand() % 255;

		     uniformLights();
		     }
				  
		     break;
		     }*/
	    case(SDL_SCANCODE_P): {
		const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

		picturesStorageSize++;
		
		if(picturesStorage){
		    picturesStorage = realloc(picturesStorage, sizeof(Picture) * picturesStorageSize);
		}else{
		    picturesStorage = malloc(sizeof(Picture));
		}

		picturesStorage[picturesStorageSize-1].txIndex = 0;

		geomentyByTxCounter[0] += 6 * 8 * sizeof(float);
		
		picturesStorage[picturesStorageSize-1].id = picturesStorageSize-1;
		
		//		picturesStorage[picturesStorageSize-1].w = picturesStorageSize-1;
		//		picturesStorage[picturesStorageSize-1].h = picturesStorageSize-1;
		
		picturesStorage[picturesStorageSize-1].mat = IDENTITY_MATRIX;
		picturesStorage[picturesStorageSize-1].mat.m[12] = curCamera->pos.x;
		picturesStorage[picturesStorageSize-1].mat.m[13] = curCamera->pos.y;
		picturesStorage[picturesStorageSize-1].mat.m[14] = curCamera->pos.z;

		calculateAABB(picturesStorage[picturesStorageSize-1].mat,
			      planePairs.vBuf, planePairs.vertexNum, planePairs.attrSize,
			      &picturesStorage[picturesStorageSize-1].lb, &picturesStorage[picturesStorageSize-1].rt);

		/*		printf("RT: %f %f %f LB: %f %f %f \n",
				argVec3(picturesStorage[picturesStorageSize-1].rt),
				argVec3(picturesStorage[picturesStorageSize-1].lb));*/
		
		batchAllGeometry();
		
		//}
	     
		break;
	    }case(SDL_SCANCODE_T):{
		 const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
		 
		 if((!curMenu || curMenu->type == texturesMenuT) && !currentKeyStates[SDL_SCANCODE_LCTRL]){
		     texturesMenu.open = !texturesMenu.open;
		     curMenu = texturesMenu.open ? &texturesMenu : NULL;
		 }
	     
		 break;
	     }case(SDL_SCANCODE_L):{
		  const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
			      
		  if((!curMenu || curMenu->type == lightMenuT) && !currentKeyStates[SDL_SCANCODE_LCTRL]){
		      lightMenu.open = !lightMenu.open;
		      curMenu = lightMenu.open ? &lightMenu : NULL;
		  }
	     
		  break;
	      }
	    case(SDL_SCANCODE_H): {
		const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

		if(mouse.selectedType == mouseWallT){
		    WallMouseData* data = (WallMouseData*)mouse.selectedThing;

		    if(data->wall){
			if(data->wall->type == LRWallT){
			    data->wall->prevType = data->wall->type;
			    data->wall->type = hiddenLRWallT;
			}else if(data->wall->prevType == LRWallT){
			    data->wall->type = data->wall->prevType;
			}

			if(data->wall->type == LWallT){
			    data->wall->prevType = data->wall->type;
			    data->wall->type = hiddenLWallT;
			}else if(data->wall->prevType == LWallT){
			    data->wall->type = data->wall->prevType;
			}

			if(data->wall->type == RWallT){
			    data->wall->prevType = data->wall->type;
			    data->wall->type = hiddenRWallT;
			}else if(data->wall->prevType == RWallT){
			    data->wall->type = data->wall->prevType;
			}

			if(data->wall->type == doorT){
			    data->wall->prevType = data->wall->type;
			    data->wall->type = hiddenDoorT;
			}else if(data->wall->prevType == doorT){
			    data->wall->type = data->wall->prevType;
			}

			if(data->wall->type == normWallT || data->wall->type == windowT){
			    data->wall->prevType = data->wall->type;
			    data->wall->type = hiddenWallT;
			}else if(data->wall->prevType == normWallT || data->wall->prevType == windowT){
			    data->wall->type = data->wall->prevType;
			}
		    }

		    if(showHiddenWalls){
			batchAllGeometry();
		    }else{
			batchAllGeometryNoHidden();
		    }
		}

		break;
	    }
	    case(SDL_SCANCODE_DELETE): {
		const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
	    
		//	if(!currentKeyStates[SDL_SCANCODE_LCTRL]){

		if (mouse.selectedType == mouseModelT) {
		    Model* model = (Model*)mouse.selectedThing;
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
		    mouse.selectedThing = 0;

		    batchModels();
	    
		    if(navPointsDraw){
			batchGeometry();
		    }
	    
		    rerenderShadowsForAllLights();
		}else if (mouse.selectedType == mouseLightT) {
		    Light* light = (Light*) mouse.selectedThing;

		    int index = 0;
		    for(int i=0;i<lightStorageSizeByType[light->type];i++){
			if (lightStorage[light->type][i].id == light->id) {
			    continue;
			}

			lightStorage[light->type][index] = lightStorage[light->type][i];
			lightStorage[light->type][index].id = index;
			index++;
		    }

		    lightStorageSizeByType[light->type] = index;

		    uniformLights();

		    if(light->type == dirLightShadowT){
			rerenderShadowsForAllLights();
		    }
		}else if (mouse.selectedType == mousePlaneT) {
		    /* 
		       Picture* panel = (Picture*)mouse.selectedThing;
		       int index = 0;

		       // clear dialogs
		       int charId = panel->characterId;

		       destroyCharacter(charId);

		       for (int i = 0; i < curModelsSize; i++) {
		       if (picturesStorage[i].id == panel->id){
		       continue;
		       }

		       picturesStorage[index] = picturesStorage[i];
		       index++;
		       }
    		  
		       picturesStorageSize--;
		       picturesStorage = realloc(picturesStorage, picturesStorageSize * sizeof(Model));
		    */
	    
		    mouse.focusedThing = NULL;
		    mouse.focusedType = 0;

		    batchGeometry();
		}
		else if (mouse.selectedType == mouseWallT) {
		    WallMouseData* data = (WallMouseData*)mouse.selectedThing;

		    for (int i = 0; i < wallsVPairs[data->wall->type].planesNum; i++) {
			geomentyByTxCounter[data->wall->planes[i].txIndex] -= wallsVPairs[data->wall->type].pairs[i].vertexNum * sizeof(float) * wallsVPairs[data->wall->type].pairs[i].attrSize;
		    }

		    deleteWallInStorage(data->wall->id);

		    free(data->wall->planes);
		    tilesStorage[data->tileId].wall[data->side] = NULL;
		    free(data->wall);
	  
		    batchAllGeometry();
		    rerenderShadowsForAllLights();
		}
		else if (mouse.selectedType == mouseTileT) {
		    TileMouseData* data = (TileMouseData*)mouse.selectedThing;

		    if (data->tx != -1) {
			int tx = tilesStorage[data->tileId].tx;

			printf("tx: %d \n", tx);
	    
			geomentyByTxCounter[tx] -= sizeof(float) * 8 * 6;
	    
			tilesStorage[data->tileId].tx = -1;

			if(!tilesStorage[data->tileId].block && !tilesStorage[data->tileId].wall[0] && !tilesStorage[data->tileId].wall[1]){
			    int tileId = data->tileId;
			    // free(tilesStorage[data->tileId]);
	      
			    deleteTileInStorage(tileId);

			    grid[(int)data->pos.y][(int)data->pos.z][(int)data->pos.x] = NULL;

			    mouse.selectedType = 0;
			    free(mouse.selectedThing);
			    mouse.selectedThing = NULL;
			}

			batchAllGeometry();
			rerenderShadowsForAllLights();
		    }
		}
		else if (mouse.selectedType == mouseBlockT) {
		    TileBlock* data = (TileBlock*)mouse.selectedThing;

		    tilesStorage[data->tileId].block = NULL;
		    free(data);

		    batchGeometry();
		}
		//	}

		break;
	    }
	    default: break;
	    }
	}
    }

    if (event.type == SDL_MOUSEBUTTONDOWN) {
	mouse.leftDown = event.button.button == SDL_BUTTON_LEFT;
	mouse.rightDown = event.button.button == SDL_BUTTON_RIGHT;

	// set focused to object under click
	if (mouse.leftDown &&
	    cursorMode &&
	    (mouse.selectedType == mouseModelT || mouse.selectedType == mouseLightT || mouse.selectedType == mousePlaneT) && mouse.focusedThing != mouse.selectedThing) {

	    initGizmosAABBFromSelected();
	}

	if (mouse.focusedThing != mouse.selectedThing && selectedGizmoAxis == 0) {
	    cursorMode = moveMode;

	    mouse.focusedThing = NULL;
	    mouse.focusedType = 0;
	}
    }
    else if (event.type == SDL_MOUSEBUTTONUP) {
	mouse.leftDown = false;
	mouse.rightDown = false;

	rotationGizmodegAcc = 0.0f;

	mouse.clickL = event.button.button == SDL_BUTTON_LEFT;
	mouse.clickR = event.button.button == SDL_BUTTON_RIGHT;

	if (selectedGizmoAxis) {
	    Matrix* mat = NULL;
	    Matrix out;

	    if (mouse.focusedType == mouseModelT) {
		Model* model = (Model*)mouse.focusedThing;
		mat = &model->mat;
	    }
	    else if (mouse.focusedType == mousePlaneT) {
		Picture* plane = (Picture*)mouse.focusedThing;
		mat = &plane->mat;
	    }
	    else if (mouse.focusedType == mouseLightT) {
		Light* light = (Light*)mouse.focusedThing;
		mat = &light->mat;
	    }

	    // return from scaledAABB back to normal
	    // unselected axis
	    if (mat) {
		out = IDENTITY_MATRIX;

		out.m[12] = mat->m[12] + gizmosPaddings[cursorMode - 1][selectedGizmoAxis - 1].x;
		out.m[13] = mat->m[13] + gizmosPaddings[cursorMode - 1][selectedGizmoAxis - 1].y;
		out.m[14] = mat->m[14] + gizmosPaddings[cursorMode - 1][selectedGizmoAxis - 1].z;

		calculateAABB(out, gizmosGeom[cursorMode - 1][selectedGizmoAxis - 1].vBuf, gizmosGeom[cursorMode - 1][selectedGizmoAxis - 1].vertexNum, gizmosGeom[cursorMode - 1][selectedGizmoAxis - 1].attrSize,
			      &gizmosAABB[cursorMode - 1][0][selectedGizmoAxis - 1], &gizmosAABB[cursorMode - 1][1][selectedGizmoAxis - 1]);
	    }
	}
    }


    if (event.type == SDL_MOUSEWHEEL) {
	mouse.wheel = event.wheel.y;

	if (cursorMode) {
	    Model* model = NULL;
	    Matrix* mat = NULL;
	    Light* light = NULL;

	    if (mouse.selectedType == mouseModelT) {
		model = (Model*)mouse.selectedThing;
		mat = &model->mat;
	    }
	    else if (mouse.selectedType == mousePlaneT) {
		Picture* plane = (Picture*)mouse.selectedThing;
		mat = &plane->mat;
	    }
	    else if (mouse.selectedType == mouseLightT) {
		light = (Light*)mouse.selectedThing;
		mat = &light->mat;
	    }

	    float scaleStep = 0.01f;

	    if (light) {
		if(light->type == pointLightT){
		    if (event.wheel.y > 0) {
			if(light->curLightPresetIndex > 0){
			    light->curLightPresetIndex--;
			}
		    }
		    else if (event.wheel.y < 0) {
			if(light->curLightPresetIndex < 11){
			    light->curLightPresetIndex++;
			}
		    }

		    printf("light %d \n", light->curLightPresetIndex);
		}else if(light->type == dirLightShadowT || light->type == dirLightT){
		    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

	  
		    if (event.wheel.y > 0) {
			if(currentKeyStates[SDL_SCANCODE_LALT]){
			    light->rad -= 0.01f;
			}else if(currentKeyStates[SDL_SCANCODE_LSHIFT]){
			    if(light->curLightPresetIndex > 0)
				light->curLightPresetIndex--;
			}else{
			    light->cutOff -= 0.001f;
			}
		    }
		    else if (event.wheel.y < 0) {
			if(currentKeyStates[SDL_SCANCODE_LALT]){
			    light->rad += 0.01f;
			}else if(currentKeyStates[SDL_SCANCODE_LSHIFT]){
			    if(light->curLightPresetIndex < 11)
				light->curLightPresetIndex++;
			}else{
			    light->cutOff += 0.001f;
			}
		    }
		}

		uniformLights();

		//	if (light->type == shadowLightT) {
		rerenderShadowsForAllLights();
		//	  rerenderShadowForLight(light->id);
		//	}
	    }

	    if (model) {
		float xTemp = mat->m[12];
		float yTemp = mat->m[13];
		float zTemp = mat->m[14];

		mat->m[12] = 0;
		mat->m[13] = 0;
		mat->m[14] = -zTemp;

		if (event.wheel.y > 0) {
		    scale(mat, 1.0f + scaleStep, 1.0f + scaleStep, 1.0f + scaleStep);
		}
		else if (event.wheel.y < 0) {
		    scale(mat, 1.0f / (1.0f + scaleStep),
			  1.0f / (1.0f + scaleStep),
			  1.0f / (1.0f + scaleStep));
		}

		mat->m[12] = xTemp;
		mat->m[13] = yTemp;
		mat->m[14] = zTemp;

		calculateModelAABB(model);

		if (navPointsDraw) {
		    batchGeometry();
		}

		batchModels();
		rerenderShadowsForAllLights();
	    }
	}
    }

    if (event.type == SDL_MOUSEMOTION) {
	float x = -1.0 + 2.0 * (event.motion.x / windowW);
	float y = -(-1.0 + 2.0 * (event.motion.y / windowH));

	if (curMenu || cursorMode || curUIBuf.rectsSize != 0) {
	    mouse.lastCursor.x = mouse.cursor.x;
	    mouse.lastCursor.z = mouse.cursor.z;

	    mouse.cursor.x = x;
	    mouse.cursor.z = y;
	}

	if (curCamera && !curMenu && !cursorMode && curUIBuf.rectsSize == 0) {
	    mouse.lastCursor.x = mouse.cursor.x;
	    mouse.lastCursor.z = mouse.cursor.z;

	    mouse.cursor.x = 0.0f;//-1.0 + 2.0 * (windowW / 2);
	    mouse.cursor.z = 0.0f;//-(-1.0 + 2.0 * (windowH));      

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

		vec3 dir;
		dir.x = cosf(rad(curCamera->yaw)) * cosf(rad(curCamera->pitch));
		dir.y = sinf(rad(curCamera->pitch));
		dir.z = (sinf(rad(curCamera->yaw)) * cosf(rad(curCamera->pitch)));

		curCamera->front = normalize3(dir);

		curCamera->right = normalize3(cross3(curCamera->front, (vec3){0.0f,1.0f,0.0f}));
		curCamera->up = normalize3(cross3(curCamera->right, curCamera->front));
	    }
	}
    }
}

void editorMatsSetup(int curShader) {
    {
	//editorView = IDENTITY_MATRIX;
	vec3 negPos = { curCamera->pos.x, -curCamera->pos.y, -curCamera->pos.z };

	vec3 normFront = normalize3(cross3(curCamera->front, curCamera->up));
	///*
	if(lightView){
	    //      editorProj = orthogonal(-10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 100.0f);
	    editorProj = perspective(rad(90.0f), 1.0f, 0.01f, 1000.0f);
      
	    vec3 lightPos = { lightStorage[dirLightShadowT][0].mat.m[12],
			      -lightStorage[dirLightShadowT][0].mat.m[13],
			      -lightStorage[dirLightShadowT][0].mat.m[14]
	    };
	    vec3 lightDir = { lightStorage[dirLightShadowT][0].mat.m[0] + 0.001f, lightStorage[dirLightShadowT][0].mat.m[1], lightStorage[dirLightShadowT][0].mat.m[2] }; 

	    // printf("d %f %f %f \n", argVec3(lightDir));
	    // printf("p %f %f %f \n", argVec3(lightPos));

	    editorView = lookAt(lightPos,
				(vec3) {
				    lightPos.x + lightDir.x,
				    lightPos.y + lightDir.y,
				    lightPos.z + lightDir.z
				},
				(vec3){0.0f, 1.0f, 0.0f});

	}else{
	    //   printf("d %f %f %f \n", argVec3(negPos));
	    //   printf("p %f %f %f \n", argVec3(curCamera->front));
					
	    // editorView = fpsView(curCamera->pos, curCamera->pitch, curCamera->yaw);/*lookAt(curCamera->pos,
	    editorProj = perspective(rad(fov), windowW / windowH, 0.01f, 1000.0f);
	
	    editorView = lookAt(negPos,
				(vec3) {
				    negPos.x + curCamera->front.x,
				    negPos.y + curCamera->front.y,
				    negPos.z + curCamera->front.z
				},
				(vec3){0.0f, 1.0f, 0.0f});
	}

	/*    editorView = lookAt((vec3){0.01f, -1.0f, .0f},
	      (vec3) {
	      0.0f + 0.0f,
	      -1.0f -1.0f,
	      0.0f + 0.0f
	      },
	      (vec3){0.0f, 1.0f, 0.0f});*/
    
	// */
	//}
	//    }
	//(vec3){0.0f, 1.0f, 0.0f});

	/*
	  editorView = lookAt((vec3){0.0f, 0.0f, 0.0f},
	  (vec3){ curCamera->front.x,
	  curCamera->front.y,
	  curCamera->front.z},
	  (vec3){0.0f, 1.0f, 0.0f});

	*/
	//    translate(&editorView, argVec3(negPos));

	// rotateY(&editorView, rad(curCamera->yaw));
	// rotateX(&editorView, rad(curCamera->pitch));

	for (int i = 0; i < shadersCounter; i++) {
	    glUseProgram(shadersId[i]);
	    uniformMat4(i, "proj", editorProj.m);
	    uniformMat4(i, "view", editorView.m);
	}

	glUseProgram(shadersId[curShader]);

	//    vec3 front = (vec3){ editorView.m[8], editorView.m[9], editorView.m[10] };

	//    curCamera->Z = normalize3((vec3) { front.x * -1.0f, front.y * 1.0f, front.z * 1.0f });
	//    curCamera->X = normalize3(cross3(curCamera->Z, curCamera->up));
	//    curCamera->Y = (vec3){ 0,dotf3(curCamera->X, curCamera->Z),0 };

	// cursor things
	{
	    float x = 0.0f;
	    float y = 0.0f;

	    if (!curMenu) {
		int xx, yy;
		SDL_GetMouseState(&xx, &yy);

		//	x = -1.0 + 2.0 * (xx / windowW);
		//	y = -(-1.0 + 2.0 * (yy / windowH));

		x = mouse.cursor.x;
		y = mouse.cursor.z;
	    }

	    float z = 1.0f;
	    vec4 rayClip = { x, y, -1.0f, 1.0f };

	    Matrix inversedProj = IDENTITY_MATRIX;

	    inverse(editorProj.m, inversedProj.m);
	    vec4 ray_eye = mulmatvec4(inversedProj, rayClip);

	    ray_eye.z = -1.0f;
	    ray_eye.w = 0.0f;

	    mouse.rayView = (vec3){ argVec3(ray_eye) };

	    Matrix inversedView = IDENTITY_MATRIX;

	    inverse(editorView.m, inversedView.m);

	    vec4 ray_wor = mulmatvec4(inversedView, ray_eye);

	    //      mouse.worldRayPos = (vec3){ argVec3(ray_wor) };
	    mouse.rayDir = normalize3((vec3) { argVec3(ray_wor) });

	    //normalize4(&ray_wor);

	}
    }
}

void editorPreFrame(float deltaTime) {
    if (cursorMode && mouse.leftDown) {
	Model* model = NULL;
	Matrix* mat = NULL;
	Light* light = NULL;
	Picture* picture = NULL;

	if (mouse.focusedType == mouseModelT) {
	    model = (Model*)mouse.focusedThing;
	    mat = &model->mat;
	}
	else if (mouse.focusedType == mousePlaneT) {
	    picture = (Picture*)mouse.focusedThing;
	    mat = &picture->mat;
	}
	else if (mouse.focusedType == mouseLightT) {
	    light = (Light*)mouse.focusedThing;
	    mat = &light->mat;
	}

	if (mat && (mouse.leftDown || mouse.rightDown)) {
	    vec3 diffDrag = { gizmoCurPos.x - gizmoStartPos.x, gizmoCurPos.y - gizmoStartPos.y, gizmoCurPos.z - gizmoStartPos.z };

	    //   printf("diff %f %f %f\n", argVec3(diffDrag));

	    if (cursorMode == moveMode) {
		if(picture && currentKeyStates[SDL_SCANCODE_LSHIFT]){
		    float xTemp = mat->m[12];
		    float yTemp = mat->m[13];
		    float zTemp = mat->m[14];

		    mat->m[12] = 0;
		    mat->m[13] = 0;
		    mat->m[14] = 0;

		    printf("dragX: %f dragY: %f \n", diffDrag.x, diffDrag.y);
	  
		    if (selectedGizmoAxis == XCircle || selectedGizmoAxis == XYPlane || selectedGizmoAxis == XZPlane) {
			if(diffDrag.x > 0){
			    scale(mat, 1.0f + diffDrag.x, 1.0f, 1.0f);
			}else{
			    scale(mat, 1.0f / (1.0f + fabs(diffDrag.x)), 1.0f, 1.0f);
			}
		    }

		    if (selectedGizmoAxis == YCircle || selectedGizmoAxis == XYPlane || selectedGizmoAxis == ZYPlane) {
			if(diffDrag.y > 0){
			    scale(mat, 1.0f, 1.0f + diffDrag.y, 1.0f);
			}else{
			    scale(mat, 1.0f, 1.0f / (1.0f + fabs(diffDrag.y)), 1.0f);
			}
		    }

		    if (selectedGizmoAxis == ZCircle || selectedGizmoAxis == ZYPlane || selectedGizmoAxis == XZPlane) {
			if(diffDrag.z > 0){
			    scale(mat, 1.0f, 1.0f, 1.0f + diffDrag.z);
			}else{
			    scale(mat, 1.0f, 1.0f, 1.0f / (1.0f + fabs(diffDrag.z)));
			}
		    }

		    /*
		      else if (selectedGizmoAxis == XYPlane) {
		      mat->m[12] += diffDrag.x;
		      mat->m[13] += diffDrag.y;
		      }
		      else if (selectedGizmoAxis == ZYPlane) {
		      mat->m[14] += diffDrag.z;
		      mat->m[13] += diffDrag.y;
		      }
		      else if (selectedGizmoAxis == XZPlane) {
		      mat->m[14] += diffDrag.z;
		      mat->m[12] += diffDrag.x;
		      }*/
	  
		    mat->m[12] = xTemp;
		    mat->m[13] = yTemp;
		    mat->m[14] = zTemp;

		    calculateAABB(picture->mat, planePairs.vBuf, planePairs.vertexNum, planePairs.attrSize, &picture->lb, &picture->rt);
		    batchAllGeometry();
		}else{
		    if (selectedGizmoAxis == XCircle) {
			mat->m[12] += diffDrag.x;
		    }
		    else if (selectedGizmoAxis == YCircle) {
			mat->m[13] += diffDrag.y;
		    }
		    else if (selectedGizmoAxis == ZCircle) {
			mat->m[14] += diffDrag.z;
		    }
		    else if (selectedGizmoAxis == XYPlane) {
			mat->m[12] += diffDrag.x;
			mat->m[13] += diffDrag.y;
		    }
		    else if (selectedGizmoAxis == ZYPlane) {
			mat->m[14] += diffDrag.z;
			mat->m[13] += diffDrag.y;
		    }
		    else if (selectedGizmoAxis == XZPlane) {
			mat->m[14] += diffDrag.z;
			mat->m[12] += diffDrag.x;
		    }
		}
	    }
	    else if (cursorMode == rotationMode) {
		const vec3 axis[3] = { {1.0f,0.0f,0.0f},
				       {0.0f,1.0f,0.0f},
				       {0.0f,0.0f,1.0f} };

		if (selectedGizmoAxis) {
		    // static const vec3 padding[3] = { { -.5f, .0f, .0f }, { .0f, -.5f, .0f}, { .0f, .0f, .5f} }; // XZ
		    //	  static const vec3 padding[3] = { { .0f, .0f, .0f }, { .0f, .0f, .0f}, { .0f, .0f,.0f} }; // XZ

		    vec3 planePos = {

			mat->m[12] + gizmosPaddings[cursorMode - 1][selectedGizmoAxis - 1].x,
			mat->m[13] + gizmosPaddings[cursorMode - 1][selectedGizmoAxis - 1].y,
			mat->m[14] + gizmosPaddings[cursorMode - 1][selectedGizmoAxis - 1].z,

		    };

		    vec3 prev_vector = {
			gizmoStartPos.x - planePos.x,
			gizmoStartPos.y - planePos.y,
			gizmoStartPos.z - planePos.z
		    };

		    vec3 cur_vector = {
			gizmoCurPos.x - planePos.x,
			gizmoCurPos.y - planePos.y,
			gizmoCurPos.z - planePos.z
		    };

		    cur_vector = normalize3(cur_vector);
		    prev_vector = normalize3(prev_vector);

		    float scDot = dotf3(prev_vector, cur_vector);

		    if (scDot < -1.0f) {
			scDot = -1.0f;
		    }
		    else if (scDot > 1.0f) {
			scDot = 1.0f;
		    }

		    float angle = acosf(scDot);

		    vec3 posDir = cross3(axis[selectedGizmoAxis - 1], prev_vector);

		    if (dotf3(cur_vector, posDir) >= 0.0f) {
			angle *= -1.0f;
		    }

		    float tempX = mat->m[12];
		    float tempY = mat->m[13];
		    float tempZ = mat->m[14];

		    mat->m[12] = 0.0f;
		    mat->m[13] = 0.0f;
		    mat->m[14] = 0.0f;

		    if (!isFreeGizmoRotation) {
			rotationGizmodegAcc += angle;

			if (rotationGizmodegAcc >= rad(15.0f)) {
			    rotate(mat, rad(15.0f), argVec3(axis[selectedGizmoAxis - 1]));
			    rotationGizmodegAcc = 0.0f;
			}
			else if (rotationGizmodegAcc <= -rad(15.0f)) {
			    rotate(mat, -rad(15.0f), argVec3(axis[selectedGizmoAxis - 1]));
			    rotationGizmodegAcc = 0.0f;
			}
		    }
		    else {
			rotate(mat, angle, argVec3(axis[selectedGizmoAxis - 1]));
		    }

		    mat->m[12] = tempX;
		    mat->m[13] = tempY;
		    mat->m[14] = tempZ;
		}
	    }

	    Matrix out2;

	    if (mat) {
		for (int i = 0; i < 2; i++) {
		    for (int i2 = 0; i2 < gizmosNum[i]; i2++) {
			out2 = IDENTITY_MATRIX;

			out2.m[12] = mat->m[12] + gizmosPaddings[i][i2].x;
			out2.m[13] = mat->m[13] + gizmosPaddings[i][i2].y;
			out2.m[14] = mat->m[14] + gizmosPaddings[i][i2].z;

			calculateAABB(out2, gizmosGeom[i][i2].vBuf, gizmosGeom[i][i2].vertexNum, gizmosGeom[i][i2].attrSize,
				      &gizmosAABB[i][0][i2], &gizmosAABB[i][1][i2]);
		    }
		}

		if (selectedGizmoAxis) {
		    out2 = IDENTITY_MATRIX;
		    scale(&out2, 1000.0f, 1000.0f, 1000.0f);

		    out2.m[12] = mat->m[12] + gizmosPaddings[cursorMode - 1][selectedGizmoAxis - 1].x;
		    out2.m[13] = mat->m[13] + gizmosPaddings[cursorMode - 1][selectedGizmoAxis - 1].y;
		    out2.m[14] = mat->m[14] + gizmosPaddings[cursorMode - 1][selectedGizmoAxis - 1].z;

		    calculateAABB(out2, gizmosGeom[cursorMode - 1][selectedGizmoAxis - 1].vBuf, gizmosGeom[cursorMode - 1][selectedGizmoAxis - 1].vertexNum, gizmosGeom[cursorMode - 1][selectedGizmoAxis - 1].attrSize,
				  &gizmosAABB[cursorMode - 1][0][selectedGizmoAxis - 1], &gizmosAABB[cursorMode - 1][1][selectedGizmoAxis - 1]);
		}
	    }

	    gizmoStartPos = gizmoCurPos;

	}

	if (light) {
	    //      printf("%f %f %f %f \n%f %f %f %f \n%f %f %f %f\n %f %f %f %f\n", spreadMat4(light->mat.m));
	    calculateAABB(light->mat, cube.vBuf, cube.vertexNum, cube.attrSize, &light->lb, &light->rt);
	    uniformLights();

	    //      if (light->type == shadowLightT) {
	    rerenderShadowsForAllLights();
	    //	rerenderShadowForLight(light->id);
	    //      }
	}

	if (picture) {
	    calculateAABB(picture->mat, planePairs.vBuf, planePairs.vertexNum, planePairs.attrSize, &picture->lb, &picture->rt);
	    batchAllGeometry();
	}

	if (model) {
	    //		  if (navPointsDraw) {
	    //		    batchAllGeometry();
	    //		  }

	    batchModels();
	    calculateModelAABB(model);
	    rerenderShadowsForAllLights();
	}
    }

    if (!console.open && !curMenu && curUIBuf.rectsSize == 0) {
	float cameraSpeed = 10.0f * deltaTime;
	const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

	isFreeGizmoRotation = currentKeyStates[SDL_SCANCODE_LALT];

	// cursorMode will be started but we have focusedType
	if (!cursorMode && currentKeyStates[SDL_SCANCODE_LCTRL]) {
	    cursorMode = moveMode;
	    mouse.focusedThing = NULL;
	    mouse.focusedType = 0;
	}

	if (cursorMode && !currentKeyStates[SDL_SCANCODE_LCTRL]) {
	    cursorMode = 0;
	    selectedGizmoAxis = 0;

	    gizmoCurPos = (vec3){ 0 };
	    gizmoStartPos = (vec3){ 0 };

	    mouse.focusedThing = NULL;
	    mouse.focusedType = 0;
	}

	if(currentKeyStates[SDL_SCANCODE_LSHIFT]){
			if (mouse.clickL) {
				if (selectedCollisionTileIndex != -1 &&
					entityStorage[playerEntityT] != NULL) {

					float div = 1.0f / 3.0f;
					int h = acceptedCollisionTilesAABB[selectedCollisionTileIndex].lb.y - 0.1f;

					vec2i dist = { acceptedCollisionTilesAABB[selectedCollisionTileIndex].lb.x / div,
						   acceptedCollisionTilesAABB[selectedCollisionTileIndex].lb.z / div };

					vec2i start = { (entityStorage[playerEntityT]->mat.m[12] - div / 2.0f) / div,
							(entityStorage[playerEntityT]->mat.m[14] - div / 2.0f) / div };

					// astar
					{
					    //bool closedList[gridZ*3][gridX*3] = {{ 0 }};
//					    AstarCell cellsDetails[gridZ][gridX] = {{ 0 }};

					    bool** closedList = malloc(sizeof(bool*) * gridZ*3);

					    for(int z=0;z<gridZ*3;z++){
						closedList[z] = malloc(sizeof(bool) * gridX*3);
					    }
					    
					    AstarCell** cellsDetails = malloc(sizeof(AstarCell*) * gridZ*3);

					    for(int z=0;z<gridZ*3;z++){
						cellsDetails[z] = malloc(sizeof(AstarCell) * gridX*3);
					    }

					    AstarOpenCell* openCells = malloc(sizeof(AstarOpenCell) * (gridZ*3) * (gridX * 3));
					    int openCellsTop = 0;

						bool pathFound = false;

					    for (int z = 0; z < gridZ*3; z++) {
						for (int x = 0; x < gridX*3; x++) {
						    cellsDetails[z][x].g = FLT_MAX;
						    cellsDetails[z][x].g = FLT_MAX;
						    cellsDetails[z][x].f = FLT_MAX;
						    cellsDetails[z][x].parX = -1;
						    cellsDetails[z][x].parZ = -1;
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
						openCells[curIndex] = openCells[--openCellsTop];
						
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

								printf("pre dest: %d %d dest: %d %d \n", cur.z, cur.x, dist.z, dist.x);
							    
							    cellsDetails[mCur.z][mCur.x].parX = cur.x;
							    cellsDetails[mCur.z][mCur.x].parZ = cur.z;
							    
							    printf("Destination found!: ");
							    pathFound = true;

							    int diZ = dist.z;
							    int diX = dist.x;

							    int pathSize = 0;

							    /*while(cellsDetails[diZ][diX].parZ != dist.z &&
							      cellsDetails[diZ][diX].parX != dist.x){*/

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

							    printf("start: %d %d -> ", start.z, start.x);

							    for(int i=pathSize-1;i>=0;i--){
								printf("%d %d -> ", path[i].z, path[i].x);
							    }

//							    printf("end: %d %d", dist.z, dist.x);

							    vec3* buf = malloc(sizeof(vec3) * ((pathSize+1)*2));

							    buf[0] = (vec3){ start.x * div + div/2.0f, (float)h + 0.2f, start.z * div + div/2.0f };
							    buf[1] = (vec3){ path[pathSize-1].x * div + div/2.0f, (float)h + 0.2f, path[pathSize-1].z * div + div/2.0f };

							    int index = 2;
							    for(int i=pathSize-1;i>0;i--){
								buf[index] = (vec3){ path[i].x * div + div/2.0f, (float)h + 0.2f, path[i].z * div + div/2.0f };
								index++;
								
								buf[index] = (vec3){ path[i-1].x * div + div/2.0f, (float)h + 0.2f, path[i-1].z * div + div/2.0f };
								index++;
							    }

							    for(int i=0;i<(pathSize+1)*2;i++){
								printf("%f %f %f\n", argVec3(buf[i]));
							    }

							    glBindVertexArray(lastFindedPath.VAO); 
							    glBindBuffer(GL_ARRAY_BUFFER, lastFindedPath.VBO);

							    lastFindedPath.VBOsize = (pathSize+1)*2;

							    glBufferData(GL_ARRAY_BUFFER,
									 sizeof(vec3) * ((pathSize+1)*2), buf, GL_STATIC_DRAW);

							    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
							    glEnableVertexAttribArray(0);

							    glBindBuffer(GL_ARRAY_BUFFER, 0);
							    glBindVertexArray(0);
		  
							    free(buf);
							    free(path);
							    
							    break;
							}else if(!closedList[mCur.z][mCur.x]
								 && !collisionGrid[h][mCur.z][mCur.x]){
							    float gNew = cellsDetails[cur.z][cur.x].g + 1.0f;
							    float hNew = sqrtf((mCur.z - dist.z) * (mCur.z - dist.z) + (mCur.x - dist.x) * (mCur.x - dist.x));
							    float fNew = gNew + hNew;

							    if(cellsDetails[mCur.z][mCur.x].f == FLT_MAX ||
							       cellsDetails[mCur.z][mCur.x].f > fNew){

								openCells[openCellsTop] = (AstarOpenCell){
								    fNew, .z = mCur.z, .x = mCur.x
								};
								openCellsTop++;
								
								cellsDetails[mCur.z][mCur.x].g = gNew;
								cellsDetails[mCur.z][mCur.x].h = hNew;
								cellsDetails[mCur.z][mCur.x].f = fNew;
								cellsDetails[mCur.z][mCur.x].parX = cur.x;
								cellsDetails[mCur.z][mCur.x].parZ = cur.z;
							    }
							}
							
						    }
						if (pathFound) break;
						}

						if (pathFound) break;

					    }


					for(int z=0;z<gridZ*3;z++){
					    free(closedList[z]);
					}

					free(closedList);

					for(int z=0;z<gridZ*3;z++){
					    free(cellsDetails[z]);
					}

					free(cellsDetails);
					free(openCells);
					}


					printf("start: %d %d dist: %d %d \n", argVec2(start), argVec2(dist));

				}
			}

			    }

	if (currentKeyStates[SDL_SCANCODE_W]) {
	    if (curCamera) {//cameraMode){
		//	vec3 normFront = normalize3(cross3(curCamera->front, curCamera->up));

		curCamera->pos.x += cameraSpeed * curCamera->front.x;
		curCamera->pos.z += cameraSpeed * curCamera->front.z;
	    }
	}
	else if (currentKeyStates[SDL_SCANCODE_S])
	{
	    if (curCamera) {//cameraMode){

		//	  vec3 normFront = normalize3(cross3(curCamera->front, curCamera->up));

		curCamera->pos.x -= cameraSpeed * curCamera->front.x;
		curCamera->pos.z -= cameraSpeed * curCamera->front.z;
	  
		//	  curCamera->pos.y -= cameraSpeed * curCamera->front.y;

		//	    glUniform3f(cameraPos, argVec3(curCamera->pos));
	    }
	    //else {
	    //player.pos.x -= speed * sin(rad(player.angle));
	    //player.pos.z -= speed * cos(rad(player.angle));
	    //}
	}
	else if (currentKeyStates[SDL_SCANCODE_D])
	{
	    if (curCamera) {//cameraMode){
		vec3 right = normalize3(cross3(curCamera->front, curCamera->up));
	  
		curCamera->pos.x += cameraSpeed * curCamera->right.x;
		curCamera->pos.z += cameraSpeed * curCamera->right.z;

		//	    glUniform3f(cameraPos, argVec3(curCamera->pos));
	    }
	}
	else if (currentKeyStates[SDL_SCANCODE_A])
	{
	    if (curCamera) {//cameraMode){
		//	  vec3 curCamera->right = normalize3(cross3(curCamera->front, curCamera->up));

		curCamera->pos.x -= cameraSpeed * curCamera->right.x;
		curCamera->pos.z -= cameraSpeed * curCamera->right.z;
		//glUniform3f(cameraPos, argVec3(curCamera->pos));
	    }
	}

    }

}

// 3d specific for editor mode 
void editor3dRender() {
    {
	stancilHighlight[mouse.selectedType]();

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, depthMaps);

	glActiveTexture(GL_TEXTURE0);
	renderScene(mainShader);
    }

    for (int i = 0;false &&  i < picturesStorageSize; i++) {
	uniformMat4(mainShader, "model", picturesStorage[i].mat.m);

	glBindBuffer(GL_ARRAY_BUFFER, planePairs.VBO);
	glBindVertexArray(planePairs.VAO);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, loadedTextures1D[picturesStorage[i].txIndex].tx); 

	glDrawArrays(GL_TRIANGLES, 0, planePairs.vertexNum); 
		 
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
    }

    glUseProgram(shadersId[lightSourceShader]);

    // markers
    for(int i=0;i<markersCounter;i++){
	uniformVec3(lightSourceShader, "color", (vec3) { darkPurple });
	
	Matrix mat = IDENTITY_MATRIX;

	uniformMat4(lightSourceShader, "model", mat.m);

	glBindBuffer(GL_ARRAY_BUFFER, markersBufs[i].VBO);
	glBindVertexArray(markersBufs[i].VAO);

	glDrawArrays(GL_TRIANGLES, 0, markersBufs[i].vertexNum);

	glBindTexture(GL_TEXTURE_2D, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
    }

    // markers
    for(int i=0;i<markersCounter;i++){
	uniformVec3(lightSourceShader, "color", (vec3) { darkPurple });
	
	Matrix mat = IDENTITY_MATRIX;

	uniformMat4(lightSourceShader, "model", mat.m);

	glBindBuffer(GL_ARRAY_BUFFER, markersBufs[i].VBO);
	glBindVertexArray(markersBufs[i].VAO);

	glDrawArrays(GL_TRIANGLES, 0, markersBufs[i].vertexNum);

	glBindTexture(GL_TEXTURE_2D, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
    }

// entity box
    glUseProgram(shadersId[animShader]);
    
    for(int i=0;i<entityTypesCounter;i++){
	if(entityStorageSize[i] == 0){
	    continue;
	}
	
	setSolidColorTx(blackColor, 1.0f);
	glBindTexture(GL_TEXTURE_2D, solidColorTx);
//	uniformVec3(lightSourceShader, "color", (vec3) { cyan });
	
	Matrix mat = entityStorage[i][0].mat;
	//IDENTITY_MATRIX;
	
	uniformMat4(animShader, "model", mat.m);

	glBindBuffer(GL_ARRAY_BUFFER, modelInfo2->mesh.VBO);
	glBindVertexArray(modelInfo2->mesh.VAO);

//	glBindBuffer(GL_ARRAY_BUFFER, entitiesBatch[i].VBO);
//	glBindVertexArray(entitiesBatch[i].VAO);

	glDrawArrays(GL_TRIANGLES, 0, modelInfo2->mesh.VBOsize);

	glBindTexture(GL_TEXTURE_2D, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

    }
    
    glUseProgram(shadersId[lightSourceShader]);

    // net tile draw
    if (curFloor != 0) {
//    if (true) {
	uniformVec3(lightSourceShader, "color", (vec3) { darkPurple });
	Matrix out2 = IDENTITY_MATRIX;
	out2.m[13] = curFloor;
	uniformMat4(lightSourceShader, "model", out2.m);

	glBindBuffer(GL_ARRAY_BUFFER, netTile.VBO);
	glBindVertexArray(netTile.VAO);

	glDrawArrays(GL_LINES, 0, 8 * gridX * gridZ);

	glBindTexture(GL_TEXTURE_2D, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
    }

    // gizmos draw
    if (mouse.focusedThing && cursorMode) {
	Matrix* mat = NULL;

	if (mouse.focusedType == mouseModelT) {
	    Model* model = (Model*)mouse.focusedThing;
	    mat = &model->mat;
	}
	else if (mouse.focusedType == mousePlaneT) {
	    Picture* plane = (Picture*)mouse.focusedThing;
	    mat = &plane->mat;
	}
	else if (mouse.focusedType == mouseLightT) {
	    Light* light = (Light*)mouse.focusedThing;
	    mat = &light->mat;
	}

	if (mat) {
	    for (int i = 0; i < gizmosNum[cursorMode - 1]; i++) {
		uniformVec3(lightSourceShader, "color", gizmosColors[i]);

		Matrix out2 = IDENTITY_MATRIX;

		out2.m[12] = mat->m[12] + gizmosPaddings[cursorMode - 1][i].x;
		out2.m[13] = mat->m[13] + gizmosPaddings[cursorMode - 1][i].y;
		out2.m[14] = mat->m[14] + gizmosPaddings[cursorMode - 1][i].z;

		uniformMat4(lightSourceShader, "model", out2.m);

		glBindBuffer(GL_ARRAY_BUFFER, gizmosGeom[cursorMode - 1][i].VBO);
		glBindVertexArray(gizmosGeom[cursorMode - 1][i].VAO);

		glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_TRIANGLES, 0, gizmosGeom[cursorMode - 1][i].vertexNum);
		glEnable(GL_DEPTH_TEST);
	    }
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
    }

    // lights render
    {
	glBindBuffer(GL_ARRAY_BUFFER, cube.VBO);
	glBindVertexArray(cube.VAO);

	for (int i2 = 0; i2 < lightsTypeCounter; i2++) {
	    for (int i = 0; i < lightStorageSizeByType[i2]; i++) {
		if(lightStorage[i2][i].off){
		    uniformVec3(lightSourceShader, "color", (vec3){ rgbToGl(128, 128, 128) });
		}else{
		    vec3 color = { rgbToGl(lightStorage[i2][i].r, lightStorage[i2][i].g, lightStorage[i2][i].b) };
		    uniformVec3(lightSourceShader, "color", color);
		}

		uniformMat4(lightSourceShader, "model", lightStorage[i2][i].mat.m);

		glDrawArrays(GL_TRIANGLES, 0, cube.vertexNum);
	    }
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
    }

    glUseProgram(shadersId[mainShader]);

    // render mouse.brush
    if (mouse.selectedType == mouseTileT) {
	TileMouseData* tileData = (TileMouseData*)mouse.selectedThing;

	const vec3 tile = tileData->pos;//xyz_indexesToCoords(tileData->grid.x,curFloor, tileData->grid.z);

	if (tileData->intersection.x < tile.x + borderArea && tileData->intersection.x >= tile.x) {
	    mouse.tileSide = left;
	}else if(tileData->intersection.x < tile.x + 1.0f && tileData->intersection.x >= tile.x + 1.0f - borderArea) {
	    mouse.tileSide = right;
	}
	else {
	    if (tileData->intersection.z < tile.z + borderArea && tileData->intersection.z >= tile.z) {
		mouse.tileSide = top;
	    }else if(tileData->intersection.z < tile.z + 1.0f && tileData->intersection.z >= tile.z + 1.0f - borderArea) {
		mouse.tileSide = bot;
	    }else if (tileData->intersection.z > (tile.z + bBlockD / 2) - borderArea && tileData->intersection.z < (tile.z + bBlockD / 2) + borderArea && tileData->intersection.x >(tile.x + bBlockW / 2) - borderArea && tileData->intersection.x < (tile.x + bBlockW / 2) + borderArea) {
		mouse.tileSide = center;
	    }
	    else {
		mouse.tileSide = -1;
	    }
	}

	/*   if(mouse.tileSide != -1 && mouse.tileSide != center){
	     printf("%s \n", sidesToStr[mouse.tileSide]);
	     }*/

	const float selectionW = borderArea * 3;

	/*if (mouse.tileSide != -1) {
	  glBindVertexArray(selectionRectVAO);
	  glBindBuffer(GL_ARRAY_BUFFER, selectionRectVBO);

	  Matrix out = IDENTITY_MATRIX;

	  out.m[12] = tile.x;
	  out.m[13] = tile.y;
	  out.m[14] = tile.z;

	  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out.m);
	  }*/

	//	printf("%d \n", mouse.tileSide);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);

	static vec3 prevTile = { -1,-1,-1 };

	if (mouse.brushType == mouseBlockBrushT) {
	    TileBlock* block = (TileBlock*)mouse.brushThing;

	    if ((prevTile.x != -1 && prevTile.y != -1 && prevTile.z != -1)
		&& (prevTile.x != tile.x || prevTile.y != tile.y || prevTile.z != tile.z)) {
		block->mat = IDENTITY_MATRIX;

		rotateY(block->mat.m, rad(block->rotateAngle));

		block->mat.m[12] = tile.x;
		block->mat.m[13] = tile.y;
		block->mat.m[14] = tile.z;

		//block->tile = tileData->tile;

		int index = (block->rotateAngle) / 90;

		//	    block->mat.m[12] += rotationBlock[index][0];
		//	    block->mat.m[14] += rotationBlock[index][1];

		//	    
		if (block->rotateAngle == 90) {
		    block->mat.m[14] += bBlockW;
		}
		else if (block->rotateAngle == 180) {
		    block->mat.m[12] += bBlockW;
		    block->mat.m[14] += bBlockW;
		}
		else if (block->rotateAngle == 270) {
		    block->mat.m[12] += bBlockD;
		}
	    }

	    prevTile.x = tile.x;
	    prevTile.y = tile.y;
	    prevTile.z = tile.z;

	    glBindVertexArray(blocksVPairs[block->type].pairs[0].VAO);
	    glBindBuffer(GL_ARRAY_BUFFER, blocksVPairs[block->type].pairs[0].VBO);

	    glBindTexture(GL_TEXTURE_2D, loadedTextures1D[block->txIndex].tx);
	    uniformMat4(mainShader, "model", block->mat.m);

	    glDrawArrays(GL_TRIANGLES, 0, blocksVPairs[block->type].pairs[0].vertexNum);

	    glBindBuffer(GL_ARRAY_BUFFER, 0);
	    glBindVertexArray(0);
	}else if(mouse.brushType == mouseEntityBrushT){
	    Entity* entity = (Entity*) mouse.brushThing;
	    
	    glUseProgram(shadersId[lightSourceShader]);
		
	    glBindBuffer(GL_ARRAY_BUFFER, cube.VBO);
	    glBindVertexArray(cube.VAO);

	    Matrix mat;
	    memcpy(mat.m, entitiesMats[entity->type].m, sizeof(float) * 16);

	    if(selectedCollisionTileIndex != -1){
		float div = 1.0f / 3.0f / 2.0f;
		mat.m[12] = acceptedCollisionTilesAABB[selectedCollisionTileIndex].lb.x + div;
		mat.m[13] = acceptedCollisionTilesAABB[selectedCollisionTileIndex].lb.y;
		mat.m[14] = acceptedCollisionTilesAABB[selectedCollisionTileIndex].lb.z + div;
	    }else{
		mat.m[12] = tile.x + bBlockD / 2;
		mat.m[13] = tile.y;
		mat.m[14] = tile.z + bBlockD / 2;
	    }

	    entity->mat.m[12] = mat.m[12];
	    entity->mat.m[13] = mat.m[13];
	    entity->mat.m[14] = mat.m[14];

	    uniformMat4(lightSourceShader, "model", mat.m);

	    if(mouse.brushThing == locationExitMarkerT){
		uniformVec3(lightSourceShader, "color", (vec3){ blackColor } );
	    }
	    
	    glDrawArrays(GL_TRIANGLES, 0, cube.vertexNum);
	    
	    glUseProgram(shadersId[mainShader]);
	    
	    glBindBuffer(GL_ARRAY_BUFFER, 0);
	    glBindVertexArray(0);
	}else if(mouse.brushType == mouseMarkerBrushT){
	    glUseProgram(shadersId[lightSourceShader]);
		
	    glBindBuffer(GL_ARRAY_BUFFER, cube.VBO);
	    glBindVertexArray(cube.VAO);

	    Matrix mat;
	    memcpy(mat.m, markersMats[(int)mouse.brushThing].m, sizeof(float) * 16);

	    mat.m[12] = tile.x + bBlockD / 2;
	    mat.m[13] = tile.y;
	    mat.m[14] = tile.z + bBlockD / 2;

	    uniformMat4(lightSourceShader, "model", mat.m);

	    if(mouse.brushThing == locationExitMarkerT){
		uniformVec3(lightSourceShader, "color", (vec3){ blackColor } );
	    }
	    
	    glDrawArrays(GL_TRIANGLES, 0, cube.vertexNum);
	    
	    glUseProgram(shadersId[mainShader]);
	    
	    glBindBuffer(GL_ARRAY_BUFFER, 0);
	    glBindVertexArray(0);
	}else if (mouse.brushType == mouseWallBrushT && mouse.tileSide != -1 && mouse.tileSide != center) {
	    // vec2i curTile = tileData->grid;
	    Side selectedSide = mouse.tileSide;

	    WallType* type = mouse.brushThing;

	    Wall wal = { 0 };
	    Matrix wallMat;

	    // setup wall 
	    {
		memset(&wal, 0, sizeof(Wall));

		wal.type = *type;
		wal.prevType = *type;
		//wal.mat = IDENTITY_MATRIX;
	    }

	    // brush phantom
	    {
		  
		memcpy(wallMat.m, hardWallMatrices[selectedSide].m, sizeof(float) * 16);

		wallMat.m[12] += tile.x;
		wallMat.m[13] = curFloor;
		wallMat.m[14] += tile.z;

		uniformMat4(mainShader, "model", wallMat.m);

		glBindTexture(GL_TEXTURE_2D, loadedTextures1D[0].tx);

		for (int i = 0; i < wallsVPairs[wal.type].planesNum; i++) {
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

	    vec3i gTile = { tileData->pos.x, curFloor, tileData->pos.z };// xyz_indexesToCoords(tileData->grid.x, curFloor, tileData->grid.z);

	    if (mouse.clickR) {
		wal.sideForMat = selectedSide;
		wal.planes = calloc(wallsVPairs[wal.type].planesNum, sizeof(Plane));
		memcpy(wal.mat.m, wallMat.m ,sizeof(float) * 16);
	
		//	  printf("Calllllllll\n");
		//grid[curFloor][tileData->grid.z][tileData->grid.x] = calloc(1,sizeof(Tile));
		// tileData->tile = grid[curFloor][tileData->grid.z][tileData->grid.x];

		int tileId = tileData->tileId;

		//printf("%s: ", sidesToStr[selectedSide]);
		//printf(spreadMat4(wal.mat.m));

		//printf("isTile exist under wall: %d \n", grid[curFloor][(int)tile.z][(int)tile.x]);

		if (tileData->tileId == -1) {
		    //	if(grid[curFloor][(int)tile.z][(int)tile.x] == 0){
		    tilesStorage = realloc(tilesStorage, sizeof(Tile) * (tilesStorageSize + 1)); 

		    //tilesStorage[tilesStorageSize] = calloc(1, sizeof(Tile));
		    tilesStorage[tilesStorageSize].pos = tileData->pos;
		    tilesStorage[tilesStorageSize].id = tilesStorageSize;
		    tilesStorage[tilesStorageSize].tx = -1;

		    tilesStorage[tilesStorageSize].block = NULL;
		    tilesStorage[tilesStorageSize].wall[0] = NULL;
		    tilesStorage[tilesStorageSize].wall[1] = NULL;

		    tileId = tilesStorageSize;
	  
		    grid[curFloor][(int)tileData->pos.z][(int)tileData->pos.x] = &tilesStorage[tilesStorageSize]; 
		    tilesStorageSize++;
		}

		for (int i = 0; i < wallsVPairs[wal.type].planesNum; i++) {
		    calculateAABB(wallMat, wallsVPairs[wal.type].pairs[i].vBuf, wallsVPairs[wal.type].pairs[i].vertexNum, wallsVPairs[wal.type].pairs[i].attrSize, &wal.planes[i].lb, &wal.planes[i].rt);
	  
		    geomentyByTxCounter[wal.planes[i].txIndex] += wallsVPairs[wal.type].pairs[i].vertexNum * sizeof(float) * wallsVPairs[wal.type].pairs[i].attrSize;
		}

		vec3 tile = tilesStorage[tileId].pos;
	
		if(selectedSide == right){
		    selectedSide = left;
		}else if(selectedSide == bot){
		    selectedSide = top;
		}

		tilesStorage[tileId].wall[selectedSide] = malloc(sizeof(Wall));
		memcpy(tilesStorage[tileId].wall[selectedSide], &wal, sizeof(Wall));

		tilesStorage[tileId].wall[selectedSide]->tileId = tileId;
		tilesStorage[tileId].wall[selectedSide]->id = wallsStorageSize;
	
		wallsStorageSize++;
		if (!wallsStorage) {
		    wallsStorage = malloc(sizeof(Wall*));
		}else {
		    wallsStorage = realloc(wallsStorage, sizeof(Wall*) * wallsStorageSize);
		}

		printf("%s \n", wallTypeStr[tilesStorage[tileId].wall[selectedSide]->type]);

		wallsStorage[wallsStorageSize - 1] = tilesStorage[tileId].wall[selectedSide];
		//grid[curFloor][(int)curTile.z][(int)curTile.x]->wall[selectedSide] = malloc(sizeof(Wall));
	
		if(showHiddenWalls){
		    batchAllGeometry();
		}else{
		    batchAllGeometryNoHidden();
		}
	    }
	}
    }
}
//}

void editor2dRender() {
  
    if(mouse.tileSide != -1 && mouse.tileSide != center){
	char buf[64];
	sprintf(buf, "%s", sidesToStr[mouse.tileSide]);
	renderText(buf, .0f, .0f, 1.0f);
    }
  
    // setup context text
    {
	if (mouse.focusedThing) {
	    switch (mouse.focusedType) {
	    case(mouseModelT): {
		sprintf(contextBelowText, modelContextText);
		contextBelowTextH = 2;

		break;
	    } case(mousePlaneT): {
		  sprintf(contextBelowText, planeContextText);
		  contextBelowTextH = 2;

		  break;
	      }
	    default: break;
	    }
	}
	else if (mouse.selectedThing) {
	    switch (mouse.selectedType) {
	    case(mouseTileT): {
		sprintf(contextBelowText, tileContextText);
		contextBelowTextH = 1;

		break;
	    }case(mouseBlockT): {
		 sprintf(contextBelowText, blockContextText);
		 contextBelowTextH = 1;

		 break;
	     }
	    default: break;
	    }
	}
	else {
	    sprintf(contextBelowText, generalContextText);
	    contextBelowTextH = 1;
	}
    }

    // aim render
    if (!curMenu && hints && curUIBuf.rectsSize == 0) {
	glBindVertexArray(hudRect.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, hudRect.VBO);

	glActiveTexture(GL_TEXTURE0);
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
    if ((mouse.brushType || mouse.brushThing) && !curMenu && hints && curUIBuf.rectsSize == 0) {
	baseYPad = letterH;

	char buf[130];

	sprintf(buf, "On brush [%s] [X] to discard", mouseBrushTypeStr[mouse.brushType]);

	renderText(buf, -1.0f, 1.0f, 1.0f);

	switch (mouse.brushType) {
	case(mouseTextureBrushT): {
	    if (mouse.clickL) {
		Texture* texture = (Texture*)mouse.brushThing;

		if (mouse.selectedType == mouseWallT) {
		    WallMouseData* wallData = (WallMouseData*)mouse.selectedThing;

		    int prevTx;

		    if (wallData->wall) {
			if (wallData->wall->type == windowT && wallData->plane <= winCenterBackPlane) {
			    static const int winPlanePairs[12] = {
				[winFrontCapPlane] = winFrontBotPlane,
				[winFrontBotPlane] = winFrontCapPlane,

				[winBackBotPlane] = winBackCapPlane,
				[winBackCapPlane] = winBackBotPlane,

				[winInnerTopPlane] = winInnerBotPlane,
				[winInnerBotPlane] = winInnerTopPlane,

				[winCenterBackPlane] = winCenterFrontPlane,
				[winCenterFrontPlane] = winCenterBackPlane,
			    };

			    geomentyByTxCounter[texture->index1D] += wallsVPairs[wallData->wall->type].pairs[wallData->plane].vertexNum * sizeof(float) * wallsVPairs[wallData->wall->type].pairs[wallData->plane].attrSize;

			    prevTx = wallData->wall->planes[winPlanePairs[wallData->plane]].txIndex;

			    geomentyByTxCounter[prevTx] -= wallsVPairs[wallData->wall->type].pairs[wallData->plane].vertexNum * sizeof(float) * wallsVPairs[wallData->wall->type].pairs[wallData->plane].attrSize;
	      
			    wallData->wall->planes[winPlanePairs[wallData->plane]].txIndex = texture->index1D;
			}

			/*
			  if (wallData->wall->type == doorT && wallData->plane <= doorCenterBackPlane) {
			  static const int doorPlanePairs[2] = {
			  [doorCenterFrontPlane] = doorCenterBackPlane,
			  [doorCenterBackPlane] = doorCenterFrontPlane,
			  };

			  geomentyByTxCounter[texture->index1D] += wallsVPairs[wallData->wall->type].pairs[wallData->plane].vertexNum * sizeof(float) * wallsVPairs[wallData->wall->type].pairs[wallData->plane].attrSize;

			  prevTx = wallData->wall->planes[doorPlanePairs[wallData->plane]].txIndex;

			  geomentyByTxCounter[prevTx] -= wallsVPairs[wallData->wall->type].pairs[wallData->plane].vertexNum * sizeof(float) * wallsVPairs[wallData->wall->type].pairs[wallData->plane].attrSize;
	      
			  wallData->wall->planes[doorPlanePairs[wallData->plane]].txIndex = texture->index1D;
			  }*/

			geomentyByTxCounter[texture->index1D] += wallsVPairs[wallData->wall->type].pairs[wallData->plane].vertexNum * sizeof(float) * wallsVPairs[wallData->wall->type].pairs[wallData->plane].attrSize;

			prevTx = wallData->wall->planes[wallData->plane].txIndex;

			geomentyByTxCounter[prevTx] -= wallsVPairs[wallData->wall->type].pairs[wallData->plane].vertexNum * sizeof(float) * wallsVPairs[wallData->wall->type].pairs[wallData->plane].attrSize;
	    
			wallData->wall->planes[wallData->plane].txIndex = texture->index1D;
			printf("Texturize: %d \n", wallData->plane);
		    }

		    //	  for (int i = 0; i < wallsVPairs[wallData->wall->type].planesNum; i++) {
		    //	    geomentyByTxCounter[prevTx] -= wallsVPairs[wallData->wall->type].pairs[i].vertexNum * sizeof(float) * wallsVPairs[wallData->wall->type].pairs[i].attrSize;
	    
		    //geomentyByTxCounter[texture->index1D] += wallsVPairs[wallData->wall->type].pairs[i].vertexNum * sizeof(float) * wallsVPairs[wallData->wall->type].pairs[i].attrSize;
		    // }
	  
		    batchAllGeometry();
		}
		else if (mouse.selectedType == mouseTileT) {
		    TileMouseData* tileData = (TileMouseData*)mouse.selectedThing;

		    int tileId = tileData->tileId;

		    if (tileData->tileId == -1) {
			tilesStorage = realloc(tilesStorage, sizeof(Tile) * (tilesStorageSize + 1));

			// tilesStorage[tilesStorageSize] = calloc(1, sizeof(Tile));
			tilesStorage[tilesStorageSize].pos = tileData->pos;
			tilesStorage[tilesStorageSize].id = tilesStorageSize;

			tilesStorage[tilesStorageSize].block = NULL;
			tilesStorage[tilesStorageSize].wall[0] = NULL;
			tilesStorage[tilesStorageSize].wall[1] = NULL;

			grid[(int)tileData->pos.y][(int)tileData->pos.z][(int)tileData->pos.x] = &tilesStorage[tilesStorageSize];
	    
			tileId = tilesStorageSize;
	    
			tilesStorageSize++;
		    }else{
			geomentyByTxCounter[tilesStorage[tileId].tx] -= sizeof(float) * 8 * 6;
		    }

		    tilesStorage[tileId].tx = texture->index1D;
		    geomentyByTxCounter[texture->index1D] += sizeof(float) * 8 * 6;
	  
		    batchAllGeometry();
		}
		else if (mouse.selectedType == mouseBlockT) {
		    TileBlock* block = (TileBlock*)mouse.selectedThing;

		    block->txIndex = texture->index1D;
		    batchGeometry();
		}
		else if (mouse.selectedType == mousePlaneT) {
		    Picture* plane = (Picture*)mouse.selectedThing;

		    geomentyByTxCounter[plane->txIndex] -= 6 * 8 * sizeof(float);
		    plane->txIndex = texture->index1D;
		    geomentyByTxCounter[plane->txIndex] += 6 * 8 * sizeof(float);

	  
		    batchAllGeometry();
		}
	    }

	    break;
	}case(mouseBlockBrushT): {
	     if (mouse.clickR && mouse.selectedType == mouseTileT) {
		 TileMouseData* tileData = (TileMouseData*)mouse.selectedThing;
		 TileBlock* block = (TileBlock*)mouse.brushThing;

		 if (tileData->tileId == -1) {
		     tilesStorageSize++;
		     tilesStorage = realloc(tilesStorage, tilesStorageSize * sizeof(Tile));
		     tilesStorage[tilesStorageSize-1].pos = tileData->pos;
		 }

		 tilesStorage[tilesStorageSize-1].block = malloc(sizeof(TileBlock));
		 memcpy(tilesStorage[tilesStorageSize-1].block, block, sizeof(TileBlock));

		 TileBlock* newBlock = tilesStorage[tilesStorageSize-1].block;

		 vec3 tile = tileData->pos;// xyz_indexesToCoords(tileData->grid.x, curFloor, tileData->grid.z);
		 tile.y = curFloor;

		 int index = (block->rotateAngle) / 90;

		 newBlock->mat = IDENTITY_MATRIX;
		 memcpy(newBlock->mat.m, block->mat.m, sizeof(float) * 16);

		 printf("new tile %f %f %f \n", tile.x, tile.y, tile.z);

		 calculateAABB(newBlock->mat, blocksVPairs[newBlock->type].pairs[0].vBuf, blocksVPairs[newBlock->type].pairs[0].vertexNum, blocksVPairs[newBlock->type].pairs[0].attrSize, &newBlock->lb, &newBlock->rt);

		 printf("new lb %f %f %f \n", argVec3(newBlock->lb));
		 printf("new rt %f %f %f \n\n", argVec3(newBlock->rt));

		 //mouse.brushThing = constructNewBlock(block->type, block->rotateAngle);
		 batchGeometry();
	     }

	     break;
	 }case(mouseEntityBrushT): {
	     if (mouse.clickR && mouse.selectedType == mouseTileT) {
		 Entity* entity = (Entity*) mouse.brushThing;

		 if(entity->type != playerEntityT || (entity->type == playerEntityT && entityStorageSize[playerEntityT] == 0)){
		     entityStorageSize[entity->type]++;

		     if(!entityStorage[entity->type]){
			 entityStorage[entity->type] = malloc(sizeof(Entity));
		     }else{
			 entityStorage[entity->type] = realloc(entityStorage[entity->type], sizeof(Entity) * entityStorageSize[entity->type]);
		     }
		 }
		 
		 memcpy(&entityStorage[entity->type][entityStorageSize[entity->type]-1], entity, sizeof(Entity));
		 
		 free(mouse.brushThing);
		 mouse.brushThing = NULL;
		 mouse.brushType = 0;

		 batchEntitiesBoxes();
	     }

	     break;
	 }
	case(mouseMarkerBrushT): {
	     if (mouse.clickR && mouse.selectedType == mouseTileT) {
		 TileMouseData* tileData = (TileMouseData*)mouse.selectedThing;
		 int marker = mouse.brushThing;

		 int tileId = tileData->tileId;

		 if (tileData->tileId == -1) {
		     tilesStorageSize++;
		     tilesStorage = realloc(tilesStorage, tilesStorageSize * sizeof(Tile));
		     tilesStorage[tilesStorageSize-1].pos = tileData->pos;

		     tileId = tilesStorageSize-1;
		 }
		 
		 tilesStorage[tileId].marker = marker;

		 markersCounterByType[marker]++;
		 markersStorageSize++;

		 if(!markersStorage){
		     markersStorage = malloc(sizeof(Tile*));
		 }else{
		     markersStorage = realloc(markersStorage, sizeof(Tile*) * markersStorageSize);
		 }

		 markersStorage[markersStorageSize - 1] = &tilesStorage[tileId];

		 batchMarkers();
	     }

	     break;
	 }
	default: break;
	}
    }

    // render selected or focused thing
    if (mouse.selectedThing && !curMenu && hints && curUIBuf.rectsSize == 0) {
	char buf[164];
	 
	switch (mouse.selectedType) {
	case(mouseLightT): {
	    Light* light = (Light*)mouse.selectedThing;

	    sprintf(buf, "Selected light [%s:%d]", lightTypesStr[light->type], light->id);

	    break;
	}
	case(mouseModelT): { 
	    Model* data = (Model*)mouse.selectedThing; 

	    sprintf(buf, "Selected model: %s", loadedModels1D[data->name].name); 

	    break;
	}case(mouseWallT): {
	     WallMouseData* data = (WallMouseData*)mouse.selectedThing;

	     char* plane = "NULL";

	     if (data->wall) {
		 if (data->wall->type == doorT) {
		     plane = doorPlanesStr[data->plane];
		 }
		 else if (data->wall->type == normWallT) {
		     plane = wallPlanesStr[data->plane];
		 }
		 else if (data->wall->type == windowT) { 
		     plane = windowPlanesStr[data->plane];
		 }
	     }

	     int id;
	     int type;

	     if (data->wall) {
		 id = data->wall->id;
		 type = data->wall->type;

	     }


	     sprintf(buf, "Selected wall[%d]: [%s] type: [%s] plane: [%s] with tx: [%s]",  
		     id, 
		     sidesToStr[data->side],
		     wallTypeStr[type],
		     plane,
		     loadedTexturesNames[data->txIndex]);

	     break;
	 }case(mouseBlockT): {
	      TileBlock* data = (TileBlock*)mouse.selectedThing;

	      sprintf(buf, "Selected block: [%s]", tileBlocksStr[data->type]);

	      break;
	  }case(mousePlaneT): {
	       Picture* data = (Picture*)mouse.selectedThing;

	       sprintf(buf, "Selected plane: [ID: %d] tx: [%s]", data->id, loadedTexturesNames[data->txIndex]);

	       break;
	   }case(mouseTileT): {
		TileMouseData* data = (TileMouseData*)mouse.selectedThing;

		if (data->tileId != -1) {
		    int tileTx = tilesStorage[data->tileId].tx;

		    if (tileTx != -1) {
			sprintf(buf, "Selected tile[%d] tx: [%s] grid:[%f %f %f]", data->tileId, loadedTexturesNames[tileTx], argVec3(data->intersection));
		    }
		    else {
			sprintf(buf, "Selected tile[%d] tx: [No tile tx] grid:[%f %f %f]", data->tileId, argVec3(data->intersection));
		    }
		}
		else {
		    sprintf(buf, "Selected empty tile");
		}

		break;
	    }
	default: break;
	}

	renderText(buf, -1.0f, 1.0f - baseYPad, 1.0f);
    }

    // render objects menu
    if (curMenu && curMenu->type == objectsMenuT) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, solidColorTx);
	setSolidColorTx(blackColor, 1.0f);

	glBindVertexArray(objectsMenu.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, objectsMenu.VBO);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	float mouseY = mouse.cursor.z;

	int selectedIndex = (1.0f - ((mouseY + 1.0f) / 2.0f)) / (letterH / 2);
	selectedIndex++;

	if (mouse.cursor.x <= objectsMenuWidth && selectedIndex <= modelsTypesInfo[objectsMenuSelectedType].counter) {
	    if (mouse.clickL) {
		mouse.focusedThing = NULL;
		mouse.focusedType = 0;

		createModel(selectedIndex - 1, objectsMenuSelectedType);

		batchModels();

		if (navPointsDraw) {
		    batchGeometry();
		}

		rerenderShadowsForAllLights();

		objectsMenu.open = false;
		curMenu = NULL;

		//  }

	    };

	    setSolidColorTx(redColor, 1.0f);

	    glBindVertexArray(hudRect.VAO);
	    glBindBuffer(GL_ARRAY_BUFFER, hudRect.VBO);

	    float  selectionRect[] = {
		-1.0f, 1.0f - (selectedIndex - 1) * letterH, 1.0f, 0.0f,
		objectsMenuWidth, 1.0f - (selectedIndex - 1) * letterH, 1.0f, 1.0f,
		-1.0f, 1.0f - (selectedIndex)*letterH, 0.0f, 0.0f,

		objectsMenuWidth, 1.0f - (selectedIndex - 1) * letterH, 1.0f, 1.0f,
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
	    glActiveTexture(GL_TEXTURE0);
	    glBindTexture(GL_TEXTURE_2D, solidColorTx);

	    float typeButtonW = (1.0f + objectsMenuWidth) / 2;

	    float baseX = objectsMenuWidth;

	    // change selection
	    //{
	    if ((mouse.cursor.x >= baseX && mouse.cursor.x <= baseX + typeButtonW) && selectedIndex <= modelTypeCounter) {
		if (mouse.clickL) {
		    objectsMenuSelectedType = selectedIndex - 1;
		}
	    }
	    //}

	    glBindVertexArray(hudRect.VAO);
	    glBindBuffer(GL_ARRAY_BUFFER, hudRect.VBO);

	    for (int i = 0; i < modelTypeCounter; i++) {
		if (i == objectsMenuSelectedType) {
		    setSolidColorTx(redColor, 1.0f);
		}
		else {
		    setSolidColorTx(blackColor, 1.0f);
		}

		float typeRect[] = {
		    baseX, 1.0f - (i)*letterH, 1.0f, 0.0f,
		    baseX + typeButtonW, 1.0f - (i)*letterH, 1.0f, 1.0f,
		    baseX, 1.0f - (i + 1) * letterH, 0.0f, 0.0f,

		    baseX + typeButtonW, 1.0f - (i)*letterH, 1.0f, 1.0f,
		    baseX, 1.0f - (i + 1) * letterH, 0.0f, 0.0f,
		    baseX + typeButtonW, 1.0f - (i + 1) * letterH, 0.0f, 1.0f
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
		renderText(modelsTypesInfo[i].str, baseX, 1.0f - ((i)*letterH), 1.0f);
	    }
	}

	for (int i = 0; i < modelsTypesInfo[objectsMenuSelectedType].counter; i++) {
	    renderText(loadedModels2D[objectsMenuSelectedType][i].name, -1.0f, 1.0f - (i * letterH), 1);
	}
    }
    else if (curMenu && curMenu->type == blocksMenuT) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, solidColorTx);
	setSolidColorTx(blackColor, 1.0f);

	glBindVertexArray(objectsMenu.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, objectsMenu.VBO);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	float mouseY = mouse.cursor.z;

	int selectedIndex = (1.0f - ((mouseY + 1.0f) / 2.0f)) / (letterH / 2.0f);
	selectedIndex++;

	if (mouse.cursor.x <= objectsMenuWidth && selectedIndex <= tileBlocksCounter) {
	    if (mouse.clickL) {
		if (mouse.brushType == mouseBlockBrushT) {
		    TileBlock* block = (TileBlock*)mouse.brushThing;
		    free(block);

		    mouse.brushType = 0;
		    mouse.brushThing = NULL;
		}

		//	if(mouse.brushType == mouseBlockBrushT){
		//	  free(mouse.brushThing);
		//	}

		mouse.brushThing = constructNewBlock(selectedIndex - 1, 0);
		mouse.brushType = mouseBlockBrushT;

		curMenu->open = false;
		curMenu = NULL;
	    };

	    setSolidColorTx(redColor, 1.0f);

	    glBindVertexArray(hudRect.VAO);
	    glBindBuffer(GL_ARRAY_BUFFER, hudRect.VBO);

	    float  selectionRect[] = {
		-1.0f, 1.0f - (selectedIndex - 1) * letterH, 1.0f, 0.0f,
		objectsMenuWidth, 1.0f - (selectedIndex - 1) * letterH, 1.0f, 1.0f,
		-1.0f, 1.0f - (selectedIndex)*letterH, 0.0f, 0.0f,

		objectsMenuWidth, 1.0f - (selectedIndex - 1) * letterH, 1.0f, 1.0f,
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

	for (int i = 0; i < tileBlocksCounter; i++) {
	    renderText(tileBlocksStr[i], -1.0f, 1.0f - (i * letterH), 1);
	}
    }
    else if (curMenu && curMenu->type == lightMenuT) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, solidColorTx);
	setSolidColorTx(blackColor, 1.0f);

	glBindVertexArray(objectsMenu.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, objectsMenu.VBO);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	float mouseY = mouse.cursor.z;

	int selectedIndex = (1.0f - ((mouseY + 1.0f) / 2.0f)) / (letterH / 2);
	selectedIndex++;

	if (mouse.cursor.x <= objectsMenuWidth && selectedIndex <= lightsTypeCounter) {
	    if (mouse.clickL) {
		createLight(curCamera->pos, selectedIndex - 1);

		curMenu->open = false;
		curMenu = NULL;
	    };

	    setSolidColorTx(redColor, 1.0f);

	    glBindVertexArray(hudRect.VAO);
	    glBindBuffer(GL_ARRAY_BUFFER, hudRect.VBO);

	    float  selectionRect[] = {
		-1.0f, 1.0f - (selectedIndex - 1) * letterH, 1.0f, 0.0f,
		objectsMenuWidth, 1.0f - (selectedIndex - 1) * letterH, 1.0f, 1.0f,
		-1.0f, 1.0f - (selectedIndex)*letterH, 0.0f, 0.0f,

		objectsMenuWidth, 1.0f - (selectedIndex - 1) * letterH, 1.0f, 1.0f,
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

	for (int i = 0; i < lightsTypeCounter; i++) {
	    renderText(lightTypesStr[i], -1.0f, 1.0f - (i * letterH), 1);
	}
    }
    else if (curMenu && curMenu->type == texturesMenuT) {
	glActiveTexture(GL_TEXTURE0);;
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

	float mouseY = mouse.cursor.z;
	int selectedIndex = (1.0f - ((mouseY + 1.0f) / 2.0f)) / (letterH / 2);

	// 1 - textureSide
	// 2 - textureCategorySide
	int selectedPart = 0;

	float xStart = -1.0f;
	float xEnd = -1.0f + textureSideW;

	if (mouse.cursor.x <= -1.0f + textureSideW) {
	    selectedPart = 1;
	}
	else if (mouse.cursor.x > -1.0f + textureSideW && mouse.cursor.x <= -1.0f + textureSideW + categorySideW) {
	    xStart = -1.0f + textureSideW;
	    xEnd = -1.0f + textureSideW + categorySideW;

	    selectedPart = 2;
	}

	int curCategoryTexturesSize = loadedTexturesCategories[texturesMenuCurCategoryIndex].txWithThisCategorySize;

	if ((selectedPart == 1 && selectedIndex < curCategoryTexturesSize) || (selectedPart == 2 && selectedIndex < loadedTexturesCategoryCounter)) {

	    // selection draw
	    {
		setSolidColorTx(redColor, 1.0f);

		glBindVertexArray(hudRect.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, hudRect.VBO);

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

	    if (mouse.clickL) {
		if (selectedPart == 2) {
		    texturesMenuCurCategoryIndex = selectedIndex;
		}
		else if (selectedPart == 1) {

		    if (mouse.brushType == mouseBlockBrushT) {
			free(mouse.brushThing);
		    }
		    mouse.brushType = mouseTextureBrushT;
		    mouse.brushThing = &loadedTextures2D[texturesMenuCurCategoryIndex][selectedIndex];

		    curMenu->open = false;
		    curMenu = NULL;
		}
	    }
	}

	for (int i = 0; i < loadedTexturesCategories[texturesMenuCurCategoryIndex].txWithThisCategorySize; i++) {
	    int index = loadedTextures2D[texturesMenuCurCategoryIndex][i].index1D;
	    renderText(loadedTexturesNames[index], -1.0f, 1.0f - (i * letterH), 1.0f);
	}

	for (int i = 0; i < loadedTexturesCategoryCounter; i++) {
	    renderText(loadedTexturesCategories[i].name, -1.0f + textureSideW, 1.0f - (i * letterH), 1.0f);
	}

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

	// dialog editorViewer background
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

	float mouseY = mouse.cursor.z;
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
      
	glActiveTexture(GL_TEXTURE0);;
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
		float mouseY = mouse.cursor.z;
	  
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
		    // vs buttons
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
	if(editedCharacter->curDialogIndex != 0){
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
    /*  if(!curMenu && hints){
    // black backGround drawins
    {
    glActiveTexture(GL_TEXTURE0);;
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
    */

    // render curs

    if(console.open){
	glActiveTexture(GL_TEXTURE0);;
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
}


void createLight(vec3 pos, int type){
    static int lastUsedCubemap = 0;
    
    lightStorageSize[type]++;
    lightStorageSizeByType[type]++;

    if(!lightStorage[type]){
	lightStorage[type] = malloc(sizeof(Light));
    }else{
	lightStorage[type] = realloc(lightStorage[type], sizeof(Light) * lightStorageSizeByType[type]);
    }

    int indexOfNew = lightStorageSizeByType[type]-1;
	    
    memcpy(&lightStorage[type][indexOfNew], &lightDef[type], sizeof(Light));
  
    //  lightStorage[type][indexOfNew].pos = pos;
    lightStorage[type][indexOfNew].id = lightStorageSizeByType[type] - 1;
    lightStorage[type][indexOfNew].type = type;

    //if(type == shadowLightT){
    //  lightStorage[type][indexOfNew].cubemapIndex = lastUsedCubemap;
    //  lastUsedCubemap++; 
    //}

    lightStorage[type][indexOfNew].mat = IDENTITY_MATRIX;

    //  lightStorage[type][indexOfNew].mat.m[0] = .0f;
    //  lightStorage[type][indexOfNew].mat.m[10] = -1.0f;

    //  rotateZ(&lightStorage[type][indexOfNew].mat, rad(-90.0f));

    lightStorage[type][indexOfNew].mat.m[0] = .0f;
    lightStorage[type][indexOfNew].mat.m[1] = -1.0f;

    lightStorage[type][indexOfNew].mat.m[4] = 1.0f;
    lightStorage[type][indexOfNew].mat.m[5] = .0f;
  
    lightStorage[type][indexOfNew].mat.m[12] = pos.x;
    lightStorage[type][indexOfNew].mat.m[13] = pos.y;
    lightStorage[type][indexOfNew].mat.m[14] = pos.z;

    calculateAABB(lightStorage[type][indexOfNew].mat, cube.vBuf, cube.vertexNum, cube.attrSize,
		  &lightStorage[type][indexOfNew].lb, &lightStorage[type][indexOfNew].rt);

    //  mouse.focusedType = mouseLightT; 
    //  mouse.focusedThing = &lightStorage[type][indexOfNew];

    // update light info in main shader
    uniformLights();

    if(type == dirLightShadowT){
	rerenderShadowsForAllLights();
    }

    //  if (type == shadowLightT) {
    //rerenderShadowForLight(lightStorage[type][indexOfNew].id);
    //  }
}

void uniformLights(){
    GLint curShader = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &curShader);

    glUseProgram(shadersId[mainShader]);

    char buf[64];

    static const char* shaderVarSufixStr[] = {
	[pointLightT] = "point",
	[dirLightT] = "dir",
	[dirLightShadowT] = "dirShadow"
    };

    uniformFloat(mainShader, "radius", max(gridX / 2.0f, gridZ / 2.0f)); 

    int localLightsCounter[lightsTypeCounter] = { 0 };
    int* onLightsIndexes;

    for (int i2 = 0; i2 < lightsTypeCounter; i2++) {
	int onCounter = 0;
    
	for (int i = 0; i < lightStorageSizeByType[i2]; i++) {
	    if (!lightStorage[i2][i].off) {
		onCounter++;
	    }
	}

	onLightsIndexes = malloc(sizeof(int) * onCounter);
	onCounter = 0;

	for (int i = 0; i < lightStorageSizeByType[i2]; i++) {
	    if (!lightStorage[i2][i].off) {
		onLightsIndexes[onCounter] = i;
		onCounter++;
	    }
	}

	for (int i = 0; i < onCounter; i++) {
	    int indx = onLightsIndexes[i];
      
	    sprintf(buf, "%sLights[%i].pos",
		    shaderVarSufixStr[i2], i);
	    uniformVec3(mainShader, buf, (vec3) { lightStorage[i2][indx].mat.m[12], lightStorage[i2][indx].mat.m[13], lightStorage[i2][indx].mat.m[14], });

	    sprintf(buf, "%sLights[%i].color",
		    shaderVarSufixStr[i2], i);

	    vec3 color = { rgbToGl(lightStorage[i2][indx].r, lightStorage[i2][indx].g, lightStorage[i2][indx].b) };
	    uniformVec3(mainShader, buf, color);
		    
	    sprintf(buf, "%sLights[%i].constant",
		    shaderVarSufixStr[i2], i); 
	    uniformFloat(mainShader, buf, 1.0f);

	    sprintf(buf, "%sLights[%i].linear",
		    shaderVarSufixStr[i2], i);
	    uniformFloat(mainShader, buf, lightPresetTable[lightStorage[i2][indx].curLightPresetIndex][0]);

	    sprintf(buf, "%sLights[%i].qaudratic",
		    shaderVarSufixStr[i2], i);
	    uniformFloat(mainShader, buf, lightPresetTable[lightStorage[i2][indx].curLightPresetIndex][1]);

	    sprintf(buf, "%sLights[%i].depthTxIndex",
		    shaderVarSufixStr[i2], i);
	    uniformInt(mainShader, buf, lightStorage[i2][indx].id);
      
	    sprintf(buf, "%sLights[%i].dir",
		    shaderVarSufixStr[i2], i);
	    uniformVec3(mainShader, buf, (vec3){lightStorage[i2][indx].mat.m[0], lightStorage[i2][indx].mat.m[1], lightStorage[i2][indx].mat.m[2]});
      
	    sprintf(buf, "%sLights[%i].rad",
		    shaderVarSufixStr[i2], i);
	    uniformFloat(mainShader, buf, lightStorage[i2][indx].rad);
      
	    sprintf(buf, "%sLights[%i].cutOff",
		    shaderVarSufixStr[i2], i);
	    uniformFloat(mainShader, buf, lightStorage[i2][indx].cutOff);
	}

	sprintf(buf, "%sLightsSize",
		shaderVarSufixStr[i2]);
	uniformInt(mainShader, buf, onCounter);

	free(onLightsIndexes);
    }

    glUseProgram(curShader);
}

void editorMouseVS(){
    if (mouse.selectedType == mouseWallT || mouse.selectedType == mouseTileT){
	free(mouse.selectedThing);
    }

    mouse.selectedThing = NULL;
    mouse.selectedType = 0;
    
    float minDistToCamera = 1000.0f;

    bool atLeastOneGizmoInter = false;

    // check gizmos inter
    if(mouse.focusedThing && cursorMode){
	if(!mouse.leftDown){
	    selectedGizmoAxis = 0;
    
	    float minDistToCamera = 1000.0f;
	    float intersectionDistance;
    
	    for(int i=0;i<gizmosNum[cursorMode-1];i++){
		bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, gizmosAABB[cursorMode-1][0][i], gizmosAABB[cursorMode-1][1][i], &gizmoStartPos, &intersectionDistance);
	
		if(isIntersect && minDistToCamera > intersectionDistance){
		    selectedGizmoAxis = i+1;
		    minDistToCamera = intersectionDistance;

		    atLeastOneGizmoInter = true;
		}
	    }
	}else{
	    vec3 inter;
	    bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, gizmosAABB[cursorMode-1][0][selectedGizmoAxis-1], gizmosAABB[cursorMode - 1][1][selectedGizmoAxis-1], &inter, NULL);

	    if(isIntersect){
		atLeastOneGizmoInter = true;
		//	printf("%f %f %f \n", argVec3(inter));
		gizmoCurPos = inter;
	    }
	}
    }

    if(atLeastOneGizmoInter){
	return;
    }
  
    // check geom inter
    WallMouseData* intersWallData = malloc(sizeof(WallMouseData));
    TileMouseData* intersTileData = malloc(sizeof(TileMouseData));

    for(int i=0;i<blocksStorageSize;i++){
	TileBlock* block = blocksStorage[i];
    
	float intersectionDistance;
	bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, block->lb, block->rt, &mouse.gizmoPosOfInter, &intersectionDistance);

	if (isIntersect && minDistToCamera > intersectionDistance) {
	    mouse.selectedThing = block;
	    mouse.selectedType = mouseBlockT;

	    mouse.interDist = intersectionDistance; 
	    minDistToCamera = intersectionDistance;
	}
    }


    // pictures
    for(int i=0;i<picturesStorageSize;i++){
	float intersectionDistance;
	bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, picturesStorage[i].lb, picturesStorage[i].rt, NULL, &intersectionDistance);   

	if (isIntersect && minDistToCamera > intersectionDistance) {
	    mouse.selectedThing = &picturesStorage[i];
	    mouse.selectedType = mousePlaneT;

	    mouse.interDist = intersectionDistance; 
	    minDistToCamera = intersectionDistance; 
	}
    }
  
    // walls
    {
	for(int i=0;i<wallsStorageSize;i++){
	    WallType type = wallsStorage[i]->type;
      
	    float intersectionDistance;

	    if(tilesStorage[wallsStorage[i]->tileId].pos.y < curFloor){
		continue;
	    }

	    for (int i3 = 0; i3 < wallsVPairs[type].planesNum; i3++) {
		bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, wallsStorage[i]->planes[i3].lb, wallsStorage[i]->planes[i3].rt, NULL, &intersectionDistance); 

		if (isIntersect && minDistToCamera > intersectionDistance) {
		    int tx = wallsStorage[i]->planes[i3].txIndex;

		    intersWallData->wall = wallsStorage[i];
	  
		    intersWallData->txIndex = tx;
		    intersWallData->tileId = wallsStorage[i]->tileId;
		    //	  printf("tileId %d \n", wallsStorage[i]->tileId);

		    //	  intersWallData->pos = ;

		    intersWallData->side = wallsStorage[i]->side;
  
		    intersWallData->plane = i3;

		    mouse.selectedType = mouseWallT;
		    mouse.selectedThing = intersWallData;
	      
		    minDistToCamera = intersectionDistance;
		}
	    }
	}
    }
  
    for(int i=0;i<tilesStorageSize;i++){
	Tile tl = tilesStorage[i];

	//    vec3i ind = coords

	// printf("%d %d\n",tl->pos.y == curFloor,(int)tl->pos.y, curFloor);
    
	//    if(tl->pos.y == curFloor){
	if(true){
	    const vec3 rt = { tl.pos.x + bBlockW, tl.pos.y, tl.pos.z + bBlockD };
	    const vec3 lb = { tl.pos.x, tl.pos.y, tl.pos.z };

	    //  printf("Rt: [%f %f %f] Lb [%f %f %f]\n", argVec3(rt), argVec3(lb));

	    float intersectionDistance;
	    vec3 intersection;

	    bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, lb, rt, &intersection, &intersectionDistance);

	    if (isIntersect && minDistToCamera > intersectionDistance) {
		//	intersTileData->tile = tl;
		intersTileData->tileId = i;

		intersTileData->pos = tl.pos;

		//intersTileData->grid = (vec2i){ tl->pos.x, tl->pos.z };
		intersTileData->intersection = intersection;

		mouse.selectedType = mouseTileT;
		mouse.selectedThing = intersTileData;

		minDistToCamera = intersectionDistance;
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
    if(true){ //curFloor != 0){
	for(int i=0;i<netTileSize;i+=2){
	    const vec3 rt = { netTileAABB[i+1].x, curFloor, netTileAABB[i+1].z };
	    const vec3 lb = { netTileAABB[i].x, curFloor, netTileAABB[i].z };

	    float intersectionDistance;
	    vec3 intersection;

	    bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, lb, rt, &intersection, &intersectionDistance);

	    if (isIntersect && minDistToCamera > intersectionDistance) {
		vec3i gridInd = xyz_coordsToIndexes(netTileAABB[i].x, curFloor, netTileAABB[i].z);
		//intersTileData->tile = grid[curFloor][gridInd.z][gridInd.x];
	
		intersTileData->tileId = -1;
		intersTileData->pos = lb;
	
		intersTileData->intersection = intersection;

		mouse.selectedType = mouseTileT;
		mouse.selectedThing = intersTileData;

		minDistToCamera = intersectionDistance;
	    }
	}
    }
    
    selectedCollisionTileIndex = -1;
    if(navPointsDraw && mouse.selectedType == mouseTileT){
	for(int i=0;i<collisionLayersSize[acceptedLayerT];i++){
	    if((acceptedCollisionTilesAABB[i].lb.y - 0.1f) == intersTileData->pos.y){
		if(intersTileData->intersection.x >= acceptedCollisionTilesAABB[i].lb.x &&
		   intersTileData->intersection.x <= acceptedCollisionTilesAABB[i].rt.x &&
		   intersTileData->intersection.z >= acceptedCollisionTilesAABB[i].lb.z &&
		   intersTileData->intersection.z <= acceptedCollisionTilesAABB[i].rt.z){
		    selectedCollisionTileIndex = i;
		    break;
		}
	    }
	}
	
	if(selectedCollisionTileIndex != -1){
		int index = selectedCollisionTileIndex;
		float h = acceptedCollisionTilesAABB[index].lb.y + 0.02f;
		
		float square[] = {
		    acceptedCollisionTilesAABB[index].rt.x,  h, acceptedCollisionTilesAABB[index].lb.z,   
		    acceptedCollisionTilesAABB[index].lb.x,  h, acceptedCollisionTilesAABB[index].rt.z,  
		    acceptedCollisionTilesAABB[index].lb.x,  h, acceptedCollisionTilesAABB[index].lb.z,

		    acceptedCollisionTilesAABB[index].rt.x,  h,  acceptedCollisionTilesAABB[index].lb.z,   
		    acceptedCollisionTilesAABB[index].rt.x,  h,  acceptedCollisionTilesAABB[index].rt.z,      
		    acceptedCollisionTilesAABB[index].lb.x,  h,  acceptedCollisionTilesAABB[index].rt.z,
		};

		glBindVertexArray(selectedCollisionTileBuf.VAO); 
		glBindBuffer(GL_ARRAY_BUFFER, selectedCollisionTileBuf.VBO);

		selectedCollisionTileBuf.VBOsize = 6;

		glBufferData(GL_ARRAY_BUFFER,
			     sizeof(square), square, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	    }
	
    }
    
    if(mouse.selectedType != mouseTileT){
	free(intersTileData);
    }

    if(mouse.selectedType != mouseWallT){
	free(intersWallData);
    }
}

void initGizmosAABBFromSelected(){
    Matrix out2;
    Matrix* mat = NULL;

    if (mouse.selectedType == mouseModelT) {
	Model* model  = (Model*)mouse.selectedThing;
	mat = &model->mat;
    }
    else if (mouse.selectedType == mousePlaneT) {
	Picture* plane = (Picture*)mouse.selectedThing;
	mat = &plane->mat;
    }
    else if (mouse.selectedType == mouseLightT) {
	Light* light = (Light*)mouse.selectedThing;
	mat = &light->mat;
    }
      
    if(mat){
	for(int i=0;i<2;i++){
	    for(int i2=0;i2<gizmosNum[i];i2++){
		out2 = IDENTITY_MATRIX;

		out2.m[12] = mat->m[12] + gizmosPaddings[i][i2].x;
		out2.m[13] = mat->m[13] + gizmosPaddings[i][i2].y;
		out2.m[14] = mat->m[14] + gizmosPaddings[i][i2].z;

		calculateAABB(out2, gizmosGeom[i][i2].vBuf, gizmosGeom[i][i2].vertexNum,gizmosGeom[i][i2].attrSize,
			      &gizmosAABB[i][0][i2], &gizmosAABB[i][1][i2]);
	    }
	}

	if(selectedGizmoAxis){
	    out2 = IDENTITY_MATRIX;
	    scale(&out2, 1000.0f, 1000.0f, 1000.0f);

	    out2.m[12] = mat->m[12] + gizmosPaddings[cursorMode-1][selectedGizmoAxis-1].x;
	    out2.m[13] = mat->m[13] + gizmosPaddings[cursorMode-1][selectedGizmoAxis-1].y;
	    out2.m[14] = mat->m[14] + gizmosPaddings[cursorMode-1][selectedGizmoAxis-1].z;

	    calculateAABB(out2, gizmosGeom[cursorMode-1][selectedGizmoAxis-1].vBuf, gizmosGeom[cursorMode-1][selectedGizmoAxis-1].vertexNum,gizmosGeom[cursorMode-1][selectedGizmoAxis-1].attrSize,
			  &gizmosAABB[cursorMode-1][0][selectedGizmoAxis-1], &gizmosAABB[cursorMode-1][1][selectedGizmoAxis-1]);
	}
	      
	mouse.focusedThing = mouse.selectedThing;
	mouse.focusedType = mouse.selectedType;
    }
}

void createModel(int index, ModelType type){
    if(objectsMenuSelectedType != playerModelT || (objectsMenuSelectedType == playerModelT && !playerModel)){
	curModelsSize++;

	if(curModels){ 
	    curModels = realloc(curModels, curModelsSize * sizeof(Model));
	    printf("Calloc moder\n");
	}else{
	    curModels = malloc(sizeof(Model));
	    printf("Calloc moder\n");
	}

  
	curModels[curModelsSize-1].id = curModelsSize-1;

	int index1D = loadedModels2D[type][index].index1D;

	curModels[curModelsSize-1].name = index1D;
	curModels[curModelsSize-1].characterId = -1; 
	  
	curModels[curModelsSize-1].mat = IDENTITY_MATRIX;
    }

    if(objectsMenuSelectedType == playerModelT){
	if(!playerModel){
	    playerModel = &curModels[curModelsSize-1];
	}
    
	playerModel->mat.m[12] = (int)curCamera->pos.x;
	playerModel->mat.m[13] = (int)curCamera->pos.y;
	playerModel->mat.m[14] = (int)curCamera->pos.z;
	    
    }else{
	curModels[curModelsSize-1].mat.m[12] = curCamera->pos.x;
	curModels[curModelsSize-1].mat.m[13] = curCamera->pos.y;
	curModels[curModelsSize-1].mat.m[14] = curCamera->pos.z;
    }
	
    calculateModelAABB(&curModels[curModelsSize-1]);

}

void deleteTileInStorage(int id){
    int index = 0;
    for(int i=0;i<tilesStorageSize;i++){
	if(tilesStorage[i].id == id){
	    continue;
	}
    
	tilesStorage[index] = tilesStorage[i];
	tilesStorage[index].id = index;
    
	index++;
    }
    tilesStorageSize--;
    tilesStorage = realloc(tilesStorage, tilesStorageSize * sizeof(Tile));
}

void deleteWallInStorage(int id){
    int index = 0;
    for(int i=0;i<wallsStorageSize;i++){
	if(wallsStorage[i]->id == id){
	    continue;
	}
    
	wallsStorage[index] = wallsStorage[i];
	wallsStorage[index]->id = index;
	index++;
    }
    wallsStorageSize--;
}

void addNewWallStorage(Wall* newWall){
    wallsStorageSize++;
  
    if(!wallsStorage){
	wallsStorage = malloc(sizeof(Wall*));
    }else{
	wallsStorage = realloc(wallsStorage, sizeof(Wall*) * wallsStorageSize);
    }

    wallsStorage[wallsStorageSize-1] = newWall;
}

void saveMapUI() {
    if(UIStructBufs[saveWindowT]->rects[1].input->buf && strlen(UIStructBufs[saveWindowT]->rects[1].input->buf)){
	saveMap(UIStructBufs[saveWindowT]->rects[1].input->buf);
	clearCurrentUI();
    }
    else {
	if (UIStructBufs[saveWindowT]->rects[2].onclickResText) {
	    free(UIStructBufs[saveWindowT]->rects[2].onclickResText);
	}

	UIStructBufs[saveWindowT]->rects[2].onclickResText = malloc(sizeof(char) * strlen("Provide save name!"));
	strcpy(UIStructBufs[saveWindowT]->rects[2].onclickResText, "Provide save name!");
    }
}

void loadMapUI() {
    if(UIStructBufs[loadWindowT]->rects[1].input->buf) {
	bool loaded = loadSave(UIStructBufs[loadWindowT]->rects[1].input->buf);

	if (loaded) {
	    clearCurrentUI();
	}
	else {
	    if (UIStructBufs[loadWindowT]->rects[2].onclickResText) {
		free(UIStructBufs[loadWindowT]->rects[2].onclickResText);
	    }

	    UIStructBufs[loadWindowT]->rects[2].onclickResText = malloc(sizeof(char) * strlen("Save doesnt exist!"));
	    strcpy(UIStructBufs[loadWindowT]->rects[2].onclickResText, "Save doesnt exist!");
	    // error on load
	}
    }
    else {
	if (UIStructBufs[loadWindowT]->rects[2].onclickResText) {
	    free(UIStructBufs[loadWindowT]->rects[2].onclickResText);
	}

	UIStructBufs[loadWindowT]->rects[2].onclickResText = malloc(sizeof(char) * strlen("Provide save name!"));
	strcpy(UIStructBufs[loadWindowT]->rects[2].onclickResText, "Provide save name!");
	// name wasnt provided
    }
}

void editorRenderCursor(){
    if(curMenu || cursorMode || curUIBuf.rectsSize != 0)
    {
	glActiveTexture(GL_TEXTURE0);;
	glBindTexture(GL_TEXTURE_2D, solidColorTx);
	setSolidColorTx(whiteColor, 1.0f);
      
	glBindVertexArray(cursor.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, cursor.VBO);

	int xx, yy;
	SDL_GetMouseState(&xx, &yy);

	float x = -1.0 + 2.0 * (xx / windowW);
	float z = -(-1.0 + 2.0 * (yy / windowH));

	float cursorPoint[] = {
	    x, z,                                                   0.0f, 0.0f,
	    x + cursorW * 0.05f, z - cursorH,                       0.0f, 0.0f,
	    x + cursorW, z - cursorH + (cursorH * 0.5f),            0.0f, 0.0f,
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
}

void setPlayerEntityBrush(){
    Entity* entity = malloc(sizeof(Entity));
    entity->type = playerEntityT;

    memcpy(entity->mat.m, &entitiesMats[entity->type], sizeof(float)*16);
    
    mouse.brushType = mouseEntityBrushT;
    mouse.brushThing = entity;

    clearCurrentUI();
}

void setExitMarkerMarkerBrush(){
    mouse.brushType = mouseMarkerBrushT;
    mouse.brushThing = locationExitMarkerT;

    clearCurrentUI();
}

void batchMarkers(){
    for(int i=0;i<markersCounter;i++){
	markersBufs[i].vertexNum = cube.vertexNum * markersCounterByType[i];
	markersBufs[i].vBuf = malloc(markersBufs[i].vertexNum * sizeof(float) * 3);
	markersCounterByType[i] = 0;
    }
    
    for(int i=0;i<markersStorageSize;i++){
	vec3 markerPos = markersStorage[i]->pos;
	MarkersTypes type = markersStorage[i]->marker;
    
	int cubeIndx = 0;
	for(int i2=markersCounterByType[type] * cube.vertexNum;
	    i2 < (markersCounterByType[type]+1) * cube.vertexNum;i2++){
	    
	    int index = i2*3;

	    vec4 cubeVert = { cube.vBuf[cubeIndx+0],
			      cube.vBuf[cubeIndx+1],
			      cube.vBuf[cubeIndx+2], 1.0f };

	    vec4 scaledCube = mulmatvec4(markersMats[type], cubeVert);

	    vec4 pos = { markerPos.x + scaledCube.x,
			 markerPos.y + scaledCube.y,
			 markerPos.z + scaledCube.z, 1.0f };
	    
/*	    vec4 pos = { markerPos.x + scaledCube.x + bBlockD / 2,
			 markerPos.y + scaledCube.y,
			 markerPos.z + scaledCube.z + bBlockD / 2, 1.0f };*/
		     
	    markersBufs[type].vBuf[index+0] = pos.x;
	    markersBufs[type].vBuf[index+1] = pos.y;
	    markersBufs[type].vBuf[index+2] = pos.z;

	    cubeIndx += 3;
	}
	
	markersCounterByType[type]++;
    }	      
           
    for(int i=0;i<markersStorageSize;i++){
	MarkersTypes type = markersStorage[i]->marker;

	for(int i2=i * cube.vertexNum;
	    i2 < (i+1) * cube.vertexNum;i2++){
	    
	    int index = i2*3;
	    
	    printf("%f %f %f \n", markersBufs[type].vBuf[index+0], markersBufs[type].vBuf[index+1], markersBufs[type].vBuf[index+2]);
	}
    }

    for(int i=0;i<markersCounter;i++){
	glBindVertexArray(markersBufs[i].VAO); 
	glBindBuffer(GL_ARRAY_BUFFER, markersBufs[i].VBO);

	glBufferData(GL_ARRAY_BUFFER,
		     markersBufs[i].vertexNum * sizeof(float) * 3, markersBufs[i].vBuf, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	free(markersBufs[i].vBuf);
	//markersBufs[i].vBuf = NULL;
    }
}


void batchEntitiesBoxes(){
    float* buf[entityTypesCounter];
    int counter[entityTypesCounter] = { 0 };
    
    for(int i=0;i<entityTypesCounter;i++){
	entitiesBatch[i].VBOsize = cube.vertexNum * entityStorageSize[i];
	buf[i] = malloc(entitiesBatch[i].VBOsize * sizeof(float) * 3);

	for(int i1=0;i1<entityStorageSize[i];i1++){
	    int cubeIndx = 0;
	    for(int i2=counter[i] * cube.vertexNum;
		i2 < (counter[i]+1) * cube.vertexNum;i2++){
		int index = i2*3;

		vec4 cubeVert = { cube.vBuf[cubeIndx+0],
				  cube.vBuf[cubeIndx+1],
				  cube.vBuf[cubeIndx+2], 1.0f };

		vec4 pos = mulmatvec4(entityStorage[i][i1].mat, cubeVert);
		/*
		vec4 pos = { markerPos.x + scaledCube.x,
				 markerPos.y + scaledCube.y,
				 markerPos.z + scaledCube.z, 1.0f };
		*/

	    
		buf[i][index+0] = pos.x;
		buf[i][index+1] = pos.y;
		buf[i][index+2] = pos.z;

		cubeIndx += 3;
	    }
	    counter[i]++;
	}

	glBindVertexArray(entitiesBatch[i].VAO); 
	glBindBuffer(GL_ARRAY_BUFFER, entitiesBatch[i].VBO);

	glBufferData(GL_ARRAY_BUFFER,
		     entitiesBatch[i].VBOsize * sizeof(float) * 3,
		     buf[i], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	free(buf[i]);	
    }
}

