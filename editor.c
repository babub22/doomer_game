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
	    markersWindow[i+1].text = malloc(sizeof(char) * (strlen(markersStr[i]) + 1));
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
	    markersWindow[0].text = malloc(sizeof(char) * (strlen("Markers:") + 1));
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
	    entitiesWindow[i+1].text = malloc(sizeof(char) * (strlen(entityTypeStr[i])+1));
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
	    entitiesWindow[0].text = malloc(sizeof(char) * (strlen("Entities:")+1));
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
	    loadWindow[0].text = malloc(sizeof(char) * (strlen("Map loading")+1));
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
	    loadWindow[1].text = malloc(sizeof(char) * (strlen("Save name:")+1));
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
	    loadWindow[2].text = malloc(sizeof(char) * (strlen("load")+1));
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
	    loadWindow[3].text = malloc(sizeof(char) * (strlen("cancel")+1));
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
	    saveWindow[0].text = malloc(sizeof(char) * (strlen("Map saving")+1));
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
	    saveWindow[1].text = malloc(sizeof(char) * (strlen("Save name:")+1));
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
	    saveWindow[2].text = malloc(sizeof(char) * (strlen("save")+1));
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
	    saveWindow[3].text = malloc(sizeof(char) * (strlen("cancel")+1));
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
	}else if(event.key.keysym.scancode == SDL_SCANCODE_K){
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
		    else if (mouse.selectedType == mouseMirrorT) {
			Mirror* mirror = (Mirror*)mouse.selectedThing;
			mat = &mirror->mat;
		    }
		    else if (mouse.selectedType == mousePlaneT) {
			Picture* plane = (Picture*)mouse.selectedThing;
			mat = &plane->mat;
		    }else if(mouse.selectedType == mouseLightT){
			light = (Light*)mouse.selectedThing;
			mat = &light->mat; 
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
		
//			  calculateAABB(block->mat, blocksVPairs[block->type].pairs[0].vBuf, blocksVPairs[block->type].pairs[0].vertexNum, blocksVPairs[block->type].pairs[0].attrSize, &block->lb, &block->rt);
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
		 }
		 

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
	    }case(SDL_SCANCODE_V): {
	  
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
	    }      case(SDL_SCANCODE_M): {
		       mirrorsStorageSize++;
		
		       if(mirrorsStorage){
			   mirrorsStorage = realloc(mirrorsStorage, sizeof(Mirror) * mirrorsStorageSize);
		       }else{
			   mirrorsStorage = malloc(sizeof(Mirror));
		       }

		       glGenFramebuffers(1, &mirrorsStorage[mirrorsStorageSize-1].writeFbo);
		       glBindFramebuffer(GL_FRAMEBUFFER, mirrorsStorage[mirrorsStorageSize-1].writeFbo);
/*
		       glGenFramebuffers(1, &fbo);
		       glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		       glGenTextures(1, &textureColorBufferMultiSampled);
		       glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
		       glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, windowW, windowH, GL_TRUE);
		       glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		       glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);*/

		       glGenTextures(1, &mirrorsStorage[mirrorsStorageSize-1].tx);
		       glBindTexture(GL_TEXTURE_2D, mirrorsStorage[mirrorsStorageSize-1].tx);
		       glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowW, windowH, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		       glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					      GL_TEXTURE_2D, mirrorsStorage[mirrorsStorageSize-1].tx, 0);

		       // attach depth buffer
		       {
			   unsigned int rbo;
			   glGenRenderbuffers(1, &rbo);
			   glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			   glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowW, windowH);
			   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
		       }

		       
		       if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			   printf("intermediateFBO creation failed! With %d \n", glCheckFramebufferStatus(GL_FRAMEBUFFER));
			   exit(0);
		       }

		       glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		       mirrorsStorage[mirrorsStorageSize-1].id = mirrorsStorageSize-1;

		       mirrorsStorage[mirrorsStorageSize-1].mat = IDENTITY_MATRIX;
		       mirrorsStorage[mirrorsStorageSize-1].mat.m[12] = curCamera->pos.x;
		       mirrorsStorage[mirrorsStorageSize-1].mat.m[13] = curCamera->pos.y;
		       mirrorsStorage[mirrorsStorageSize-1].mat.m[14] = curCamera->pos.z;
		       mirrorsStorage[mirrorsStorageSize - 1].dir = (vec3){ .0f,.0f,1.0f };

		       break;
		   }
	    case(SDL_SCANCODE_T):{
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
		}
		else if (mouse.selectedType == mouseWallT) {
		    WallMouseData* data = (WallMouseData*)mouse.selectedThing;

		    for (int i = 0; i < wallsVPairs[data->wall->type].planesNum; i++) {
			//geomentyByTxCounter[data->wall->planes[i].txIndex] -= wallsVPairs[data->wall->type].pairs[i].vertexNum * sizeof(float) * wallsVPairs[data->wall->type].pairs[i].attrSize;
		    }

			placedWallCounter[data->wall->type]--;

		    free(data->wall->planes);
		    //tilesStorage[data->tileId].wall[data->side] = NULL;
		    free(data->wall);
	  
		    
		    rerenderShadowsForAllLights();
		}

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
	    (mouse.selectedType == mouseModelT || mouse.selectedType == mouseMirrorT || mouse.selectedType == mouseLightT || mouse.selectedType == mousePlaneT) && mouse.focusedThing != mouse.selectedThing) {

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
	    else if (mouse.focusedType == mouseMirrorT) {
		Mirror* mirror = (Mirror*)mouse.focusedThing;
		mat = &mirror->mat;
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

//		calculateAABB(out, gizmosGeom[cursorMode - 1][selectedGizmoAxis - 1].vBuf, gizmosGeom[cursorMode - 1][selectedGizmoAxis - 1].vertexNum, gizmosGeom[cursorMode - 1][selectedGizmoAxis - 1].attrSize,
//			      &gizmosAABB[cursorMode - 1][0][selectedGizmoAxis - 1], &gizmosAABB[cursorMode - 1][1][selectedGizmoAxis - 1]);
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
	    else if (mouse.selectedType == mouseMirrorT) {
		Mirror* mirror = (Mirror*)mouse.selectedThing;
		mat = &mirror->mat;
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
	    
	    glUseProgram(shadersId[mainShader]);
	    
	    uniformVec3(mainShader, "cameraPos", curCamera->pos);
	    uniformVec3(waterShader, "cameraPos", curCamera->pos);
	    uniformVec3(animShader, "cameraPos", curCamera->pos);
	
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
    /*for(int i=0;i<entityTypesCounter;i++){
	for(int i2=0;i2<entityStorageSize[i];i2++){
	    if(entityStorage[i][i2].path){
		entityStorage[i][i2].frame++;

		if(entityStorage[i][i2].frame == 20){		    
		    entityStorage[i][i2].mat.m[12] = entityStorage[i][i2].path[entityStorage[i][i2].curPath].x + entitiesMats[i].m[12];
		    entityStorage[i][i2].mat.m[13] = entityStorage[i][i2].path[entityStorage[i][i2].curPath].y + entitiesMats[i].m[13];
		    entityStorage[i][i2].mat.m[14] = entityStorage[i][i2].path[entityStorage[i][i2].curPath].z + entitiesMats[i].m[14];
		    
		    entityStorage[i][i2].curPath++;
		    entityStorage[i][i2].frame = 0;

			batchEntitiesBoxes();
		}

		if(entityStorage[i][i2].curPath == entityStorage[i][i2].pathSize){
		    free(entityStorage[i][i2].path);
			entityStorage[i][i2].path = NULL;
			entityStorage[i][i2].pathSize = 0;
		}
	    }
	}
    }*/
    

    
    if (cursorMode && mouse.leftDown) {
	Model* model = NULL;
	Matrix* mat = NULL;
	Light* light = NULL;
	Picture* picture = NULL;
	Mirror* mirror = NULL;

	if (mouse.focusedType == mouseModelT) {
	    model = (Model*)mouse.focusedThing;
	    mat = &model->mat;
	}
	else if (mouse.focusedType == mousePlaneT) {
	    picture = (Picture*)mouse.focusedThing;
	    mat = &picture->mat;
	}
	else if (mouse.focusedType == mouseMirrorT) {
	    mirror = (Mirror*)mouse.focusedThing;
	    mat = &mirror->mat;
	}
	else if (mouse.focusedType == mouseLightT) {
	    light = (Light*)mouse.focusedThing;
	    mat = &light->mat;
	}

	if (mat && (mouse.leftDown || mouse.rightDown)) {
	    vec3 diffDrag = { gizmoCurPos.x - gizmoStartPos.x, gizmoCurPos.y - gizmoStartPos.y, gizmoCurPos.z - gizmoStartPos.z };

	    //   printf("diff %f %f %f\n", argVec3(diffDrag));

	    if (cursorMode == moveMode) {
		if((picture || mirror) && currentKeyStates[SDL_SCANCODE_LSHIFT]){
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

			/*
		    if(picture){
			calculateAABB(picture->mat, planePairs.vBuf, planePairs.vertexNum, planePairs.attrSize, &picture->lb, &picture->rt);
			
		    }else{
			calculateAABB(mirror->mat, planePairs.vBuf, planePairs.vertexNum, planePairs.attrSize, &mirror->lb, &mirror->rt);
		    }*/
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

			    if(mirror){
				vec4 newDir = mulmatvec4(*mat,(vec4){ .0f, .0f, 1.0f, 1.0f });
				mirror->dir = ((vec3){argVec3(newDir)});
			    }

			}
			else if (rotationGizmodegAcc <= -rad(15.0f)) {
			    rotate(mat, -rad(15.0f), argVec3(axis[selectedGizmoAxis - 1]));
			    rotationGizmodegAcc = 0.0f;

			    if(mirror){
				vec4 newDir = mulmatvec4(*mat,(vec4){ .0f, .0f, 1.0f , 1.0f });
				mirror->dir = ((vec3){argVec3(newDir)});
			    }

			}
		    }
		    else {			
			rotate(mat, angle, argVec3(axis[selectedGizmoAxis - 1]));
			if(mirror){
			    vec4 newDir = mulmatvec4(*mat,(vec4){ .0f, .0f, 1.0f , 1.0f });
			    mirror->dir = ((vec3){argVec3(newDir)});
			}
			
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
//			calculateAABB(out2, gizmosGeom[i][i2].vBuf, gizmosGeom[i][i2].vertexNum, gizmosGeom[i][i2].attrSize,
//				      &gizmosAABB[i][0][i2], &gizmosAABB[i][1][i2]);
		    }
		}

		if (selectedGizmoAxis) {
		    out2 = IDENTITY_MATRIX;
		    scale(&out2, 1000.0f, 1000.0f, 1000.0f);

		    out2.m[12] = mat->m[12] + gizmosPaddings[cursorMode - 1][selectedGizmoAxis - 1].x;
		    out2.m[13] = mat->m[13] + gizmosPaddings[cursorMode - 1][selectedGizmoAxis - 1].y;
		    out2.m[14] = mat->m[14] + gizmosPaddings[cursorMode - 1][selectedGizmoAxis - 1].z;

//		    calculateAABB(out2, gizmosGeom[cursorMode - 1][selectedGizmoAxis - 1].vBuf, gizmosGeom[cursorMode - 1][selectedGizmoAxis - 1].vertexNum, gizmosGeom[cursorMode - 1][selectedGizmoAxis - 1].attrSize,
//				  &gizmosAABB[cursorMode - 1][0][selectedGizmoAxis - 1], &gizmosAABB[cursorMode - 1][1][selectedGizmoAxis - 1]);
		}
	    }

	    gizmoStartPos = gizmoCurPos;

	}

	if (mirror) {
	  //  calculateAABB(mirror->mat, planePairs.vBuf, planePairs.vertexNum, planePairs.attrSize, &mirror->lb, &mirror->rt);
	}
    }

    if (!console.open && !curMenu && curUIBuf.rectsSize == 0) {
	static uint32_t lastTime = 0.0f;
	uint32_t time = SDL_GetTicks();
	float dt = (time - lastTime) / 1000.0f;
	lastTime = time;

	float cameraSpeed = 10.0f * dt;
	const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

	if(currentKeyStates[SDL_SCANCODE_LSHIFT]){
	    cameraSpeed = 60.0f * dt;
	}
	
	isFreeGizmoRotation = currentKeyStates[SDL_SCANCODE_LALT];

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

	vec3 forward = curCamera->front;
	forward.y = 0.0f;
	forward = normalize3(forward);
    
	vec3 right = normalize3(cross3((vec3) { .0f, 1.0f, .0f }, forward));

	if (currentKeyStates[SDL_SCANCODE_W]) {
	  curCamera->pos.x += cameraSpeed * forward.x;
	  curCamera->pos.z += cameraSpeed * forward.z;
	}
	else if (currentKeyStates[SDL_SCANCODE_S]){
	  curCamera->pos.x -= cameraSpeed * forward.x;
	  curCamera->pos.z -= cameraSpeed * forward.z;
	}
	else if (currentKeyStates[SDL_SCANCODE_D]){
	  curCamera->pos.x -= cameraSpeed * right.x;
	  curCamera->pos.z -= cameraSpeed * right.z;
	}
	else if (currentKeyStates[SDL_SCANCODE_A]){
	  curCamera->pos.x += cameraSpeed * right.x;
	  curCamera->pos.z += cameraSpeed * right.z;
	}

    }

}

// 3d specific for editor mode 
void editor3dRender() {

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
    
    glUseProgram(shadersId[lightSourceShader]);

    // gizmos draw
    if (mouse.focusedThing && cursorMode) {
	Matrix* mat = NULL;

	if (mouse.focusedType == mouseMirrorT) {
	    Mirror* mirror = (Mirror*)mouse.focusedThing;
	    mat = &mirror->mat;
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
}
//}

void editor2dRender() {
    float baseYPad = 0.0f;

    // brush handle usage
    if ((mouse.brushType || mouse.brushThing) && !curMenu && hints && curUIBuf.rectsSize == 0) {
	baseYPad = letterH;

	char buf[130];

	sprintf(buf, "On brush [%s] [X] to discard", mouseBrushTypeStr[mouse.brushType]);

	renderText(buf, -1.0f, 1.0f, 1.0f);

	switch (mouse.brushType) {
	case(mouseEntityBrushT): {
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

		 //batchEntitiesBoxes();
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
	}case(mouseMirrorT): {
	       Mirror* data = (Mirror*)mouse.selectedThing;

	       sprintf(buf, "Selected mirror: [ID: %d]", data->id);

	       break;
	   }case(mousePlaneT): {
	       Picture* data = (Picture*)mouse.selectedThing;

	       sprintf(buf, "Selected plane: [ID: %d] tx: [%s]", data->id);

	       break;
	   }case(mouseTileT): {

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

//		mouse.brushThing = constructNewBlock(selectedIndex - 1, 0);
//		mouse.brushType = mouseBlockBrushT;

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

//	for (int i = 0; i < tileBlocksCounter; i++) {
//	    renderText(tileBlocksStr[i], -1.0f, 1.0f - (i * letterH), 1);
//	}
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
		//createLight(curCamera->pos, selectedIndex - 1);

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
	else if (curMenu && curMenu->type == dialogViewerT) {
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

void uniformLights(){
    int shaderTable[] = { mainShader, snowShader, animShader, waterShader };
    int shaderTableSize = sizeof(shaderTable) / sizeof(shaderTable[0]);
    int lightsCounter[5] = { 0 };

    for(int s=0;s<shaderTableSize;s++){
	char buf[64];

	for(int i=0;i<lightsStorageSize;i++){
	    sprintf(buf, "%sLights[%i].pos",
		    shaderVarSufixStr[lightsStorage[i].type], i);
	    uniformVec3(shaderTable[s], buf, (vec3) { lightsStorage[i].mat.m[12], lightsStorage[i].mat.m[13], lightsStorage[i].mat.m[14], });

	    sprintf(buf, "%sLights[%i].color",
		    shaderVarSufixStr[lightsStorage[i].type], i);
	    uniformVec3(shaderTable[s], buf, lightsStorage[i].color);

	    sprintf(buf, "%sLights[%i].power",
	    shaderVarSufixStr[lightsStorage[i].type], i);
	    uniformFloat(shaderTable[s], buf, lightsStorage[i].power);

	    lightsCounter[lightsStorage[i].type]++;
	}

	for(int i=0;i<5;i++){
	    if(lightsCounter[i] != 0 ){
		sprintf(buf, "%sLightsSize",
			shaderVarSufixStr[i]);
		uniformInt(shaderTable[s], buf, lightsCounter[i]);
	    }
	}
	

/*
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
		uniformVec3(shaderTable[s], buf, (vec3) { lightStorage[i2][indx].mat.m[12], lightStorage[i2][indx].mat.m[13], lightStorage[i2][indx].mat.m[14], });

		sprintf(buf, "%sLights[%i].color",
			shaderVarSufixStr[i2], i);

		vec3 color = { rgbToGl(lightStorage[i2][indx].r, lightStorage[i2][indx].g, lightStorage[i2][indx].b) };
		uniformVec3(shaderTable[s], buf, color);
		    
		sprintf(buf, "%sLights[%i].constant",
			shaderVarSufixStr[i2], i); 
		uniformFloat(shaderTable[s], buf, 1.0f);

		sprintf(buf, "%sLights[%i].linear",
			shaderVarSufixStr[i2], i);
		uniformFloat(shaderTable[s], buf, lightPresetTable[lightStorage[i2][indx].curLightPresetIndex][0]);

		sprintf(buf, "%sLights[%i].qaudratic",
			shaderVarSufixStr[i2], i);
		uniformFloat(shaderTable[s], buf, lightPresetTable[lightStorage[i2][indx].curLightPresetIndex][1]);

		sprintf(buf, "%sLights[%i].depthTxIndex",
			shaderVarSufixStr[i2], i);
		uniformInt(shaderTable[s], buf, lightStorage[i2][indx].id);
      
		sprintf(buf, "%sLights[%i].dir",
			shaderVarSufixStr[i2], i);
		uniformVec3(shaderTable[s], buf, (vec3){lightStorage[i2][indx].mat.m[0], lightStorage[i2][indx].mat.m[1], lightStorage[i2][indx].mat.m[2]});
      
		sprintf(buf, "%sLights[%i].rad",
			shaderVarSufixStr[i2], i);
		uniformFloat(shaderTable[s], buf, lightStorage[i2][indx].rad);
      
		sprintf(buf, "%sLights[%i].cutOff",
			shaderVarSufixStr[i2], i);
		uniformFloat(shaderTable[s], buf, lightStorage[i2][indx].cutOff);
	    }

	    sprintf(buf, "%sLightsSize",
		    shaderVarSufixStr[i2]);
	    uniformInt(shaderTable[s], buf, onCounter);

	    free(onLightsIndexes);
	    }*/
    }
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

    // mirrors
    for(int i=0;i<mirrorsStorageSize;i++){
	float intersectionDistance;
	bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, mirrorsStorage[i].lb, mirrorsStorage[i].rt, NULL, &intersectionDistance);   

	if (isIntersect && minDistToCamera > intersectionDistance) {
	    mouse.selectedThing = &mirrorsStorage[i];
	    mouse.selectedType = mouseMirrorT;

	    mouse.interDist = intersectionDistance; 
	    minDistToCamera = intersectionDistance; 
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
    else if (mouse.selectedType == mouseMirrorT) {
	Mirror* mirror = (Mirror*)mouse.selectedThing;
	mat = &mirror->mat;
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

//		calculateAABB(out2, gizmosGeom[i][i2].vBuf, gizmosGeom[i][i2].vertexNum,gizmosGeom[i][i2].attrSize,
//			      &gizmosAABB[i][0][i2], &gizmosAABB[i][1][i2]);
	    }
	}

	if(selectedGizmoAxis){
	    out2 = IDENTITY_MATRIX;
	    scale(&out2, 1000.0f, 1000.0f, 1000.0f);

	    out2.m[12] = mat->m[12] + gizmosPaddings[cursorMode-1][selectedGizmoAxis-1].x;
	    out2.m[13] = mat->m[13] + gizmosPaddings[cursorMode-1][selectedGizmoAxis-1].y;
	    out2.m[14] = mat->m[14] + gizmosPaddings[cursorMode-1][selectedGizmoAxis-1].z;

//	    calculateAABB(out2, gizmosGeom[cursorMode-1][selectedGizmoAxis-1].vBuf, gizmosGeom[cursorMode-1][selectedGizmoAxis-1].vertexNum,gizmosGeom[cursorMode-1][selectedGizmoAxis-1].attrSize,
//			  &gizmosAABB[cursorMode-1][0][selectedGizmoAxis-1], &gizmosAABB[cursorMode-1][1][selectedGizmoAxis-1])//;
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
	    
    }else{
	curModels[curModelsSize-1].mat.m[12] = curCamera->pos.x;
	curModels[curModelsSize-1].mat.m[13] = curCamera->pos.y;
	curModels[curModelsSize-1].mat.m[14] = curCamera->pos.z;
    }
	
//    calculateModelAABB(&curModels[curModelsSize-1]);

}

void saveMapUI() {
    /*
    if(UIStructBufs[saveWindowT]->rects[1].input->buf && strlen(UIStructBufs[saveWindowT]->rects[1].input->buf)){
	saveMap(UIStructBufs[saveWindowT]->rects[1].input->buf);
	clearCurrentUI();
    }
    else {
	if (UIStructBufs[saveWindowT]->rects[2].onclickResText) {
	    free(UIStructBufs[saveWindowT]->rects[2].onclickResText);
	}

	UIStructBufs[saveWindowT]->rects[2].onclickResText = malloc(sizeof(char) * (strlen("Provide save name!") + 1));
	strcpy(UIStructBufs[saveWindowT]->rects[2].onclickResText, "Provide save name!");
    }
    */
}

void loadMapUI() {
    /*
    if(UIStructBufs[loadWindowT]->rects[1].input->buf) {
	bool loaded = loadSave(UIStructBufs[loadWindowT]->rects[1].input->buf);

	if (loaded) {
	    clearCurrentUI();
	}
	else {
	    if (UIStructBufs[loadWindowT]->rects[2].onclickResText) {
		free(UIStructBufs[loadWindowT]->rects[2].onclickResText);
	    }

	    UIStructBufs[loadWindowT]->rects[2].onclickResText = malloc(sizeof(char) * (strlen("Save doesnt exist!") + 1));
	    strcpy(UIStructBufs[loadWindowT]->rects[2].onclickResText, "Save doesnt exist!");
	    // error on load
	}
    }
    else {
	if (UIStructBufs[loadWindowT]->rects[2].onclickResText) {
	    free(UIStructBufs[loadWindowT]->rects[2].onclickResText);
	}

	UIStructBufs[loadWindowT]->rects[2].onclickResText = malloc(sizeof(char) * (strlen("Provide save name!") + 1));
	strcpy(UIStructBufs[loadWindowT]->rects[2].onclickResText, "Provide save name!");
	// name wasnt provided
    }
    */
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
    Entity* entity = calloc(1,sizeof(Entity));
    entity->type = playerEntityT;

    // redo with specs
    entity->model = calloc(1,sizeof(ModelData));
    entity->model->data = &modelsData[0];

    entity->model->nodes = malloc(sizeof(GLTFNode)* entity->model->data->nodesSize);
    entity->model->tempTransforms = malloc(sizeof(float)* 10 * entity->model->data->nodesSize);
    
    printf("transf: %d nodes: %d \n",
	   sizeof(float)* 10 * entity->model->data->nodesSize ,
	   sizeof(GLTFNode)* entity->model->data->nodesSize);
    
    memcpy(entity->model->nodes,
	   entity->model->data->nodes,
	   sizeof(GLTFNode)* entity->model->data->nodesSize);

    entity->model->jointsMats = malloc(sizeof(Matrix)*entity->model->data->jointsIdxsSize);

    /*
    entity->model->prevAnim = 0;
    entity->model->curAnim = 0;
    entity->model->curStage = 0;
    entity->model->mirrored = false;*/
    
    entity->mat = IDENTITY_MATRIX;
    
    entity->dir = (vec3){ .0f, .0f, 1.0f};
    
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

	printf("%d batched \n", entitiesBatch[i].VBOsize * sizeof(float) * 3);
	
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

