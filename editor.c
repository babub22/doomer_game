#include "deps.h"
#include "linearAlg.h"
#include "main.h"
#include "editor.h"

TextInput* selectedTextInput;

int* dialogEditorHistory;
int dialogEditorHistoryLen;
int dialogEditorHistoryCursor;

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
				   {0.7, 1.8} };

#define lightsPresetMax 12

bool cursorMode;

bool cursorShouldBeCentered;

int texturesMenuCurCategoryIndex = 0;
ModelType objectsMenuSelectedType = objectModelType;

Menu objectsMenu = { .type = objectsMenuT };
Menu blocksMenu = { .type = blocksMenuT };
Menu texturesMenu = { .type = texturesMenuT };
Menu lightMenu = { .type = lightMenuT };
Menu planeCreatorMenu = { .type = planeCreatorT };

ManipulationMode manipulationMode = 0;
float manipulationStep = 0.01f;
float manipulationScaleStep = 5 * 0.01f + 1;

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
  batchGeometry();
}

void editorPreLoop(){
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

    // for(int i=0;i< netTileSize;i++){
    //      printf("%f %f \n", argVec2(netTileAABB[i]));
    //    }

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
    if(event.key.keysym.scancode == SDL_SCANCODE_F5 && !selectedTextInput) {
      saveMap(curSaveName);
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
      }
      case(SDL_SCANCODE_F2):{
	hints = !hints;
	  
	break;
      }	  case(SDL_SCANCODE_F3):{
	    //enviromental.snow = !enviromental.snow;
	  
	    break;
	  }
      case(SDL_SCANCODE_UP): {
	if(manipulationMode != 0 && (mouse.focusedType == mouseLightT || mouse.focusedType == mouseModelT || mouse.focusedType == mousePlaneT)){
	  Matrix* mat = -1;
	  bool itPlane = false;
		   
	  Model* model = NULL;
	  Light* light = NULL;
	      
	  if (mouse.focusedType == mouseModelT) { 
	    model = (Model*)mouse.focusedThing; 
	    mat = &model->mat; 
	  }
	  else if (mouse.focusedType == mousePlaneT) {
	    Picture* plane = (Picture*)mouse.focusedThing;
	    mat = &plane->mat;
	    itPlane = true;
	  }else if(mouse.focusedType == mouseLightT){
	    light = (Light*)mouse.focusedThing;
	    mat = &light->mat; 
	  }

	  switch(manipulationMode){
	  case(TRANSFORM_Z):{
	    mat->m[13] = mat->m[13] + manipulationStep;

	    if(model){
	      calculateModelAABB(model);
	    }else if(light){
	      calculateAABB(light->mat, cube.vBuf, cube.vertexNum, cube.attrSize, &light->lb, &light->rt);
	      uniformLights();

	      if(light->type == shadowPointLightT){
		rerenderShadowForLight(light->id);
	      }
	    }
		
	    break;
	  }
	  case(TRANSFORM_XY): {
	    mat->m[12] = mat->m[12] - manipulationStep;

	    if(model){
	      calculateModelAABB(model);
	    }else if(light){
	      calculateAABB(light->mat, cube.vBuf, cube.vertexNum, cube.attrSize, &light->lb, &light->rt);

	      uniformLights();
	      
	      if(light->type == shadowPointLightT){
		rerenderShadowForLight(light->id);
	      }
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
	    }else if(mouse.focusedType != mouseLightT){
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
	else if(false && (mouse.brushType == mouseBlockBrushT || mouse.selectedType == mouseBlockT)){
	  TileBlock* block = NULL;

	  if(mouse.brushType == mouseBlockBrushT){
	    block = (TileBlock*) mouse.brushThing;
	  }else if(mouse.selectedType == mouseBlockT){
	    block = (TileBlock*) mouse.selectedThing;
	  }

	  if(block->type == roofBlockT) { 
	    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL); 

	    /*if (currentKeyStates[SDL_SCANCODE_LCTRL] && block->vertexes[11] + manipulationStep <= floorH) {
	      block->vertexes[11] += manipulationStep;
	      block->vertexes[21] += manipulationStep;
	      block->vertexes[26] += manipulationStep;  
	      }
	      else if (!currentKeyStates[SDL_SCANCODE_LCTRL] && block->vertexes[1] + manipulationStep <= floorH) { 
	      block->vertexes[1] += manipulationStep;
	      block->vertexes[6] += manipulationStep;
	      block->vertexes[16] += manipulationStep;
	      }*/
		
	  }
	}
	else if (mouse.selectedType == mouseWallT) {
	  WallMouseData* data = (WallMouseData*) mouse.selectedThing;

	  const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

	  Matrix* mat = &data->tile->walls[data->side].mat;

	  if(currentKeyStates[SDL_SCANCODE_LCTRL]){
	    scale(mat, 1.0f, 1.05f, 1.0f);		
	  }/*else{
	     if(data->side == bot || data->side == top){
	     mat->m[14] += .05f;
	     }else{
	     mat->m[12] += .05f;
	     }
	     }*/

	  for(int i=0;i<wallsVPairs[data->type].planesNum;i++){
	    calculateAABB(*mat, wallsVPairs[data->type].pairs[i].vBuf, wallsVPairs[data->type].pairs[i].vertexNum, wallsVPairs[data->type].pairs[i].attrSize, &data->tile->walls[data->side].planes[i].lb, &data->tile->walls[data->side].planes[i].rt);
	  }

	  batchGeometry();
	}

	break;
      }
      case(SDL_SCANCODE_DOWN): {
	// TODO: if intersected tile + wall will work only tile changer
	if(manipulationMode != 0 && (mouse.focusedType == mouseLightT || mouse.focusedType == mouseModelT || mouse.focusedType == mousePlaneT)){
	  Matrix* mat = -1;
	  bool isPlane = false;
	  Model* model = NULL;
	  Light* light = NULL;
	      
	  if (mouse.focusedType == mouseModelT) {
	    model = (Model*)mouse.focusedThing;
	    mat = &model->mat;
	  }
	  else if (mouse.focusedType == mousePlaneT) {
	    Picture* plane = (Picture*)mouse.focusedThing;
	    mat = &plane->mat;
	    isPlane = true;
	  }
	  else if(mouse.focusedType == mouseLightT){
	    light = (Light*)mouse.focusedThing;
	    mat = &light->mat; 
	  }

	  switch(manipulationMode){ 
	  case(TRANSFORM_Z):{
	    mat->m[13] = mat->m[13] - manipulationStep;

	    if(model){
	      calculateModelAABB(model);
	    }else if(light){
	      calculateAABB(light->mat, cube.vBuf, cube.vertexNum, cube.attrSize, &light->lb, &light->rt);
	      uniformLights();

	      if(light->type == shadowPointLightT){
		rerenderShadowForLight(light->id);
	      }
	    }
		
	    break;
	  }
	  case(TRANSFORM_XY): {
	    mat->m[12] = mat->m[12] + manipulationStep;

	    if(model){
	      calculateModelAABB(model);
	    }else if(light){
	      calculateAABB(light->mat, cube.vBuf, cube.vertexNum, cube.attrSize, &light->lb, &light->rt);
	      uniformLights();

	      if(light->type == shadowPointLightT){
		rerenderShadowForLight(light->id);
	      }
	    }
		
	    break;
	  }
	  case(SCALE): {
	    if(isPlane){
	      Picture* plane = (Picture*)mouse.focusedThing;
	      plane->h -= 0.01f;
	    }else if(mouse.focusedType != mouseLightT){
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

	}else if(false && mouse.brushType == mouseBlockBrushT){
	  TileBlock* block = (TileBlock*) mouse.brushThing;

	  if(block->type == roofBlockT) {
	    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

	    /*if (currentKeyStates[SDL_SCANCODE_LCTRL] && block->vertexes[11] - manipulationStep >= 0) {
	      block->vertexes[11] -= manipulationStep;
	      block->vertexes[21] -= manipulationStep;
	      block->vertexes[26] -= manipulationStep;
	      }
	      else if (!currentKeyStates[SDL_SCANCODE_LCTRL] && block->vertexes[1] - manipulationStep >= 0) {
	      block->vertexes[1] -= manipulationStep;
	      block->vertexes[6] -= manipulationStep;
	      block->vertexes[16] -= manipulationStep;
	      }*/
		
	  }
	}else if (mouse.selectedType == mouseWallT) {
	  WallMouseData* data = (WallMouseData*)mouse.selectedThing;
	      
	  const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

	  Matrix* mat = &data->tile->walls[data->side].mat;

	  if(currentKeyStates[SDL_SCANCODE_LCTRL]){
	    scale(mat, 1.0f, 1/1.05f, 1.0f);
	  }/*else{
	     if(data->side == bot || data->side == top){
	     mat->m[14] -= .05f;
	     }else{
	     mat->m[12] -= .05f;
	     }
	     }*/
	      
	  for(int i=0;i<wallsVPairs[data->type].planesNum;i++){
	    calculateAABB(*mat, wallsVPairs[data->type].pairs[i].vBuf, wallsVPairs[data->type].pairs[i].vertexNum, wallsVPairs[data->type].pairs[i].attrSize, &data->tile->walls[data->side].planes[i].lb, &data->tile->walls[data->side].planes[i].rt);
	  }

	  batchGeometry();
	}

	break;
      }
      case(SDL_SCANCODE_Q): {
	curCamera->pos.y += .1f ;
	  
	break;
      }
      case(SDL_SCANCODE_F): {
	if(!mouse.focusedThing){
	  if(mouse.selectedType == mouseLightT || mouse.selectedType == mousePlaneT || mouse.selectedType == mouseModelT){
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


	  if(light){
	    mat->m[13] = curFloor;
	    calculateAABB(light->mat, cube.vBuf, cube.vertexNum, cube.attrSize, &light->lb, &light->rt);

	    uniformLights();
	    
	    if(light->type == shadowPointLightT){
		rerenderShadowForLight(light->id);
	      }
	  }

	  if(model){
	    printf("%s %f \n", loadedModels1D[model->name].name ,loadedModels1D[model->name].modelSizes.y);
	    
	    mat->m[13] = curFloor + loadedModels1D[model->name].modelSizes.y;
	    calculateModelAABB(model);
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
      case(SDL_SCANCODE_LEFT): {
	const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

	Light* light = (Light*)mouse.focusedThing;
	    
	/*	    if(mouse.focusedType == mouseLightT){
		    light->pos.x -= .1f;
		    }else*/
	if(manipulationMode != 0 && (mouse.focusedType == mouseModelT || mouse.focusedType == mousePlaneT || mouse.focusedType == mouseLightT)){  
	  Matrix* mat = -1; 
	  bool isPlane = false;
	      
	  Model* model = NULL;
	  Light* light = NULL;

	  if (mouse.focusedType == mouseModelT) {
	    model = (Model*)mouse.focusedThing;
	    mat = &model->mat;

	  }
	  else if (mouse.focusedType == mousePlaneT) {
	    Picture* plane = (Picture*)mouse.focusedThing;
	    mat = &plane->mat;
	    isPlane = true;
	  }else if(mouse.focusedType == mouseLightT){
	    light = (Light*)mouse.focusedThing;
	    mat = &light->mat;
	    //		light->dir = (vec3){0.0f, -1.0f, 0.0f};
	  }

	  float xTemp = mat->m[12];
	  float yTemp = mat->m[13];
	  float zTemp = mat->m[14];

	  if (manipulationMode == ROTATE_Y || manipulationMode == ROTATE_X || manipulationMode == ROTATE_Z) {
	    mat->m[12] = 0;
	    mat->m[13] = 0;
	    mat->m[14] = 0;

	    float step = 1.0f;

	    if (manipulationMode == ROTATE_Y) {
	      rotateY(mat->m, rad(-step));
	    }
	    else if (manipulationMode == ROTATE_X) {
	      rotateX(mat->m, rad(-step));
	    }
	    else if (manipulationMode == ROTATE_Z) {
	      rotateZ(mat->m, rad(-step));
	    }

	    if(light){
	      light->dir = (vec3){0.0f, -1.0f, 0.0f};
		  
	      vec4 trasfDir = mulmatvec4(light->mat, (vec4){light->dir.x, light->dir.y, light->dir.z, 0.0f});

	      light->dir.x = trasfDir.x;
	      light->dir.y = trasfDir.y;
	      light->dir.z = trasfDir.z;

	      calculateAABB(light->mat, cube.vBuf, cube.vertexNum, cube.attrSize, &light->lb, &light->rt);
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
	    }else if(light){
	      calculateAABB(light->mat, cube.vBuf, cube.vertexNum, cube.attrSize, &light->lb, &light->rt);
	      uniformLights();

	      if(light->type == shadowPointLightT){
		rerenderShadowForLight(light->id);
	      }
	    }
	  }
	}
	    
	break;
      }case(SDL_SCANCODE_RIGHT): {  
	 const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL); 

	 Light* light = (Light*)mouse.focusedThing;  
		   
	 /*if(mouse.focusedType == mouseLightT){ 
	   light->pos.x += .1f;  
	   }else*/
	 if(manipulationMode != 0 && (mouse.focusedType == mouseModelT || mouse.focusedType == mousePlaneT || mouse.focusedType == mouseLightT)){  
	   Matrix* mat = -1;
	   bool isPlane = false;

	   Light* light = NULL;
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
	   else if(mouse.focusedType == mouseLightT){
	     light = (Light*)mouse.focusedThing;
	     mat = &light->mat;
	     //		light->dir = (vec3){0.0f, -1.0f, 0.0f};
	   }

	   float xTemp = mat->m[12];
	   float yTemp = mat->m[13];
	   float zTemp = mat->m[14];

	   if (manipulationMode == ROTATE_Y || manipulationMode == ROTATE_X || manipulationMode == ROTATE_Z) {
	     mat->m[12] = 0;
	     mat->m[13] = 0;
	     mat->m[14] = -zTemp;

	     float step = 1.0f;

	     if (manipulationMode == ROTATE_Y) {
	       rotateY(mat->m, rad(step));
	     }
	     else if (manipulationMode == ROTATE_X) {
	       rotateX(mat->m, rad(step));
	     }
	     else if (manipulationMode == ROTATE_Z) {
	       rotateZ(mat->m, rad(step));
	     }

	     if(light){
	       light->dir = (vec3){0.0f, -1.0f, 0.0f};
		  
	       vec4 trasfDir = mulmatvec4(light->mat, (vec4){light->dir.x, light->dir.y, light->dir.z, 0.0f});

	       light->dir.x = trasfDir.x;
	       light->dir.y = trasfDir.y;
	       light->dir.z = trasfDir.z;

	       calculateAABB(light->mat, cube.vBuf, cube.vertexNum, cube.attrSize, &light->lb, &light->rt);
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
	     }else if(light){
	       calculateAABB(light->mat, cube.vBuf, cube.vertexNum, cube.attrSize, &light->lb, &light->rt);
	       uniformLights();

	       if(light->type == shadowPointLightT){
		rerenderShadowForLight(light->id);
	       }
	     }
		 
	     break;
	   }

	   default: break;
	   }
	 }

	 break;
       }case(SDL_SCANCODE_SPACE): {
	  //cameraMode = !cameraMode;

	  //curCamera = cameraMode ? &camera1 : &camera2;

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

	    if(mat){
	      float xTemp = mat->m[12];
	      float yTemp = mat->m[13];
	      float zTemp = mat->m[14];
      
	      mat->m[12] = 0;
	      mat->m[13] = 0;
	      mat->m[14] = 0;

	      rotateY(mat->m, rad(45.0f));

	      if(light){
		/*		light->dir = (vec3){0.0f, -1.0f, 0.0f};
		  
		vec4 trasfDir = mulmatvec4(light->mat, (vec4){light->dir.x, light->dir.y, light->dir.z, 0.0f});

		light->dir.x = trasfDir.x;
		light->dir.y = trasfDir.y;
		light->dir.z = trasfDir.z;*/

		calculateAABB(light->mat, cube.vBuf, cube.vertexNum, cube.attrSize, &light->lb, &light->rt);
	      }

	      mat->m[12] = xTemp;
	      mat->m[13] = yTemp;
	      mat->m[14] = zTemp;

	      if(model){
		calculateModelAABB(model);
	      }
	    }
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

	      /*	      static const float increaseData[4][2] = {
		{ -bBlockW, 0.0f }, // [12] [14]
		{ 0.0f, bBlockW },
		{ bBlockW, 0.0f },
		{ 0.0f, -bBlockD }
	      };*/

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
	 const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
	 if(currentKeyStates[SDL_SCANCODE_LCTRL] && dofPercent < 1.0f){
	   dofPercent += 0.01f;
	 }else{
	   if (curFloor < gridY - 1) {
	     curFloor++;
	   }
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
	Light* light = NULL;
	    
	if(mouse.focusedType == mouseLightT){
	  light = (Light*) mouse.focusedThing;
	}else if(mouse.selectedType == mouseLightT){
	  light = (Light*) mouse.selectedThing;
	}

	if(light){
	  light->off = !light->off;
	  uniformLights();
	  
	}
				  
	break;
      }      case(SDL_SCANCODE_M): {
	Light* light = NULL;
	    
	if(mouse.focusedType == mouseLightT){
	  light = (Light*) mouse.focusedThing;
	}else if(mouse.selectedType == mouseLightT){
	  light = (Light*) mouse.selectedThing;
	}

	if(light){
	  light->color = (vec3){ (float)(rand() % 1000 + 1.0f) / 1000.0f, (float)(rand() % 1000 + 1) / 1000.0f, (float)(rand() % 1000 + 1) / 1000.0f};
	  uniformLights();
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

	  if(data->type == wallJointT){
	    data->tile->jointExist[data->side] = !data->tile->jointExist[data->side];
	  }else{
	    data->tile->walls[data->side].planes[data->plane].hide = !data->tile->walls[data->side].planes[data->plane].hide;
	  }

	  batchGeometry();
	}
	    
	//highlighting = !highlighting;
	    
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

	      batchGeometry();
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

	      batchGeometry();
	    }
	  }else if (mouse.selectedType == mouseWallT) {
	    // WallType type = (grid[mouse.wallTile.y][mouse.wallTile.z][mouse.wallTile.x]->walls >> (mouse.wallSide * 8)) & 0xFF;
	    WallMouseData* data = (WallMouseData*)mouse.selectedThing;

	    free(data->tile->walls[data->side].planes);
	    data->tile->walls[data->side].planes = NULL;

	    batchGeometry();
	  }else if (mouse.selectedType == mouseTileT) {
	    // WallType type = (grid[mouse.wallTile.y][mouse.wallTile.z][mouse.wallTile.x]->walls >> (mouse.wallSide * 8)) & 0xFF;
	    TileMouseData* data = (TileMouseData*)mouse.selectedThing; 

	    if(data->tile){
	      setIn(data->tile->ground, data->groundInter, 0);
	      setIn(data->tile->ground, 0, netTileT);

	      batchGeometry();
	    }
	  }else if (mouse.selectedType == mouseBlockT) {
	    TileBlock* data = (TileBlock*)mouse.selectedThing; 

	    data->tile->block = NULL;   
	
	    free(data);

	    batchGeometry();
	  }
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
  }

  if (event.type == SDL_MOUSEBUTTONUP) {
    mouse.leftDown = false;
    mouse.rightDown = false;

    mouse.clickL = event.button.button == SDL_BUTTON_LEFT;
    mouse.clickR = event.button.button == SDL_BUTTON_RIGHT;

    if(cursorMode && mouse.focusedThing){
      if(mouse.focusedType == mouseTileT || mouse.focusedType == mouseBlockT){
	free(mouse.focusedThing);
      }

      mouse.focusedThing = NULL;
      mouse.focusedType = 0;
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

      float scaleStep = 0.01f;

      if(light){
	int temp = light->curLightPresetIndex;
	
	if(event.wheel.y > 0){
	  light->curLightPresetIndex--;
	}else if(event.wheel.y < 0){
	  light->curLightPresetIndex++;
	}

	if(light->curLightPresetIndex < 0 || light->curLightPresetIndex >= lightsPresetMax){
	  light->curLightPresetIndex = temp;
	}

	uniformLights();
	if(light->type == shadowPointLightT){
	  rerenderShadowForLight(light->id);
	}
      }

      if(model){
	float xTemp = mat->m[12];
	float yTemp = mat->m[13];
	float zTemp = mat->m[14];

	mat->m[12] = 0;
	mat->m[13] = 0;
	mat->m[14] = -zTemp;

	if(event.wheel.y > 0){
	  scale(mat, 1.0f+ scaleStep,1.0f+ scaleStep, 1.0f + scaleStep);
	}else if(event.wheel.y < 0){
	  scale(mat, 1.0f/(1.0f+ scaleStep),
		1.0f / (1.0f + scaleStep),
		1.0f / (1.0f + scaleStep));
	}
	
	mat->m[12] = xTemp;
	mat->m[13] = yTemp;
	mat->m[14] = zTemp;

	calculateModelAABB(model);
      }
    }
  }
      
  if (event.type == SDL_MOUSEMOTION) {
    float x = -1.0 + 2.0 * (event.motion.x / windowW);
    float y = -(-1.0 + 2.0 * (event.motion.y / windowH));
    
    if(curMenu || cursorMode){
      mouse.cursor.x = x;
      mouse.cursor.z = y;  
    }

    if (curCamera && !curMenu && !cursorMode) { 
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
	    
	curCamera->dir.x = cos(rad(curCamera->yaw)) * cos(rad(curCamera->pitch));
	curCamera->dir.y = sin(rad(curCamera->pitch));
	curCamera->dir.z = sin(rad(curCamera->yaw)) * cos(rad(curCamera->pitch));
	    
	curCamera->front = normalize3(curCamera->dir);
      }
    }
}
}

void editorMatsSetup(int curShader){
    {
    editorView = IDENTITY_MATRIX;
    vec3 negPos = { -curCamera->pos.x, -curCamera->pos.y, -curCamera->pos.z };

    translate(&editorView, argVec3(negPos));
      
    rotateY(&editorView, rad(curCamera->yaw));
    rotateX(&editorView, rad(curCamera->pitch));
    
    for (int i = 0; i < shadersCounter; i++) {
      glUseProgram(shadersId[i]);
      uniformMat4(i, "proj", editorProj.m);
      uniformMat4(i, "view", editorView.m);
    }

    glUseProgram(shadersId[curShader]);

    vec3 front  = (vec3){ editorView.m[8], editorView.m[9], editorView.m[10] };

    curCamera->Z = normalize3((vec3) { front.x * -1.0f, front.y * 1.0f, front.z * 1.0f });
    curCamera->X = normalize3(cross3(curCamera->Z, curCamera->up));
    curCamera->Y = (vec3){ 0,dotf3(curCamera->X, curCamera->Z),0 };

    // cursor things
    {
      float x = 0.0f;
      float y = 0.0f;

      if(!curMenu){
	int xx, yy;
	SDL_GetMouseState(&xx, &yy);

	//	x = -1.0 + 2.0 * (xx / windowW);
	//	y = -(-1.0 + 2.0 * (yy / windowH));

	x= mouse.cursor.x;
	y= mouse.cursor.z;
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

void editorPreFrame(float deltaTime){
  if((cursorMode) && (mouse.leftDown || mouse.rightDown)){
    if(!mouse.focusedThing){
      if(mouse.focusedType == mouseTileT || mouse.focusedType == mouseBlockT){
	free(mouse.focusedThing);
      }
      
      mouse.focusedThing = mouse.selectedThing;
      mouse.focusedType = mouse.selectedType;

      mouse.selectedThing = NULL;
      mouse.selectedType = 0;
    }
  
    Model* model = NULL;
    Matrix* mat = NULL;
    Light* light = NULL;
    
    if (mouse.focusedType == mouseModelT) { 
      model = (Model*)mouse.focusedThing; 
      mat = &model->mat; 
    }
    else if (mouse.focusedType == mousePlaneT) {
      Picture* plane = (Picture*)mouse.focusedThing;
      mat = &plane->mat;
    }else if(mouse.focusedType == mouseLightT){
      light = (Light*)mouse.focusedThing;
      mat = &light->mat; 
    }

    if(mat && (mouse.leftDown || mouse.rightDown)){
      vec3 ray = (vec3) { mouse.rayDir.x, 0, mouse.rayDir.z };
      vec3 modelPos = (vec3) { mat->m[12], mat->m[13], mat->m[14] };

      vec4 modelViewPos = mulmatvec4(editorView, (vec4) { argVec3(modelPos), 1.0f });
    
      vec4 viewIntersect = (vec4) { -mouse.rayView.x * modelViewPos.z, -mouse.rayView.y * modelViewPos.z, 1.0f * modelViewPos.z, 1.0f  };

      Matrix inversedView = IDENTITY_MATRIX;
      inverse(editorView.m, inversedView.m);

      vec4 worldPoint = mulmatvec4(inversedView, viewIntersect);

      if(mouse.leftDown){
	mat->m[12] = worldPoint.x;
	mat->m[14] = worldPoint.z;
      }else if(mouse.rightDown){
	mat->m[13] = worldPoint.y;
      }

      //      vec3 centroid = {0};

      /*if(light){
	calculateAABB(light->mat, cube.vBuf, cube.vertexNum, cube.attrSize, &light->lb, &light->rt);
	
	//	centroid.x = (light->rt.x+light->lb.x)/2.0f;
	//	centroid.y = (light->rt.y+light->lb.y)/2.0f;
	//	centroid.z = (light->rt.z+light->lb.z)/2.0f;
      }*/

      if(light){
	calculateAABB(light->mat, cube.vBuf, cube.vertexNum, cube.attrSize, &light->lb, &light->rt);
	uniformLights();
	if(light->type == shadowPointLightT){
	  rerenderShadowForLight(light->id);
	}
      }

      if(model){
	calculateModelAABB(model);
	/*
	centroid.x = model->rt.x / 2.0f;
	centroid.y = model->rt.y / 2.0f;
	centroid.z = model->rt.z / 2.0f;
	
	vec4 worldCentroid = mulmatvec4(*mat, (vec4){ argVec3(centroid), 1.0f });
	vec4 eyeCentroid = mulmatvec4(editorView, worldPoint);
	vec4 clip = mulmatvec4(editorProj, eyeCentroid);

	//	clip.w = -1.0f;

	vec3 nds = { clip.x / clip.w, clip.y / clip.w, clip.z / clip.w };
	vec2 viewPos = { ((nds.x + 1.0f) / 2.0f) * windowW, ((nds.y + 1.0f) / 2.0f) * windowH };

	mouse.cursor.x = nds.x;
	mouse.cursor.z = nds.y;
	SDL_WarpMouseInWindow(window, viewPos.x, viewPos.z);

	printf("Screen %f %f NDC: %f %f %f \n", argVec2(viewPos),argVec3(nds));*/
      }

      
    }
  }
  
  if(!console.open && !curMenu){
    float cameraSpeed = 10.0f * deltaTime;
    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

    if(!cursorMode && currentKeyStates[SDL_SCANCODE_LCTRL]){
      cursorShouldBeCentered = true;

      if(mouse.focusedType == mouseTileT || mouse.focusedType == mouseBlockT){
	free(mouse.focusedThing);
      }

      mouse.focusedThing = NULL;
      mouse.focusedType = 0;
    }
    
    cursorMode = currentKeyStates[SDL_SCANCODE_LCTRL];

    if (currentKeyStates[SDL_SCANCODE_W]){
	if (curCamera) {//cameraMode){
	  vec3 normFront = normalize3(cross3(curCamera->front, curCamera->up));
		    
	  curCamera->pos.x -= cameraSpeed * normFront.x;
	  curCamera->pos.y -= cameraSpeed * normFront.y;
	  curCamera->pos.z -= cameraSpeed * normFront.z;
	}
      }
    else if (currentKeyStates[SDL_SCANCODE_S])
      {
	if (curCamera) {//cameraMode){
		    
	  vec3 normFront = normalize3(cross3(curCamera->front, curCamera->up));

	  curCamera->pos.x += cameraSpeed * normFront.x;
	  curCamera->pos.y += cameraSpeed * normFront.y;
	  curCamera->pos.z += cameraSpeed * normFront.z;

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
	  curCamera->pos.x += cameraSpeed * curCamera->front.x;
	  curCamera->pos.z += cameraSpeed * curCamera->front.z;

	  //	    glUniform3f(cameraPos, argVec3(curCamera->pos));
	}
      }
    else if (currentKeyStates[SDL_SCANCODE_A])
      {
	if (curCamera) {//cameraMode){
	  vec3 normFront = normalize3(cross3(curCamera->front, curCamera->up));

	  curCamera->pos.x -= cameraSpeed * curCamera->front.x;
	  //			  curCamera->pos.y -= cameraSpeed * curCamera->front.y;
	  curCamera->pos.z -= cameraSpeed * curCamera->front.z;
	  //glUniform3f(cameraPos, argVec3(curCamera->pos));
	}
      }

  }

  if ((mouse.focusedType == mouseLightT || mouse.focusedType == mouseModelT || mouse.focusedType == mousePlaneT) && currentKeyStates[SDL_SCANCODE_LCTRL]) {
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
	else if (mouse.focusedType == mouseLightT) {
	  Light* light = (Light*)mouse.focusedThing;
	  mat = &light->mat;
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
      }else if (false && currentKeyStates[SDL_SCANCODE_Z]){
	manipulationMode = ROTATE_Z;
      } 
    }else if(currentKeyStates[SDL_SCANCODE_T]){
      manipulationMode = TRANSFORM_XY;
    }else if (false && currentKeyStates[SDL_SCANCODE_Z]) {
      manipulationMode = TRANSFORM_Z;
    }
    else if(currentKeyStates[SDL_SCANCODE_G]){
      // Scale
      manipulationMode = SCALE;
    }
  }
}

// 3d specific for editor mode 
void editor3dRender() {
  glUseProgram(shadersId[lightSourceShader]);

  // net tile draw
  if(curFloor != 0){
    uniformVec3(lightSourceShader, "color", (vec3){ darkPurple });
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
  
  // lights render
  {
    glBindBuffer(GL_ARRAY_BUFFER, cube.VBO);
    glBindVertexArray(cube.VAO);

    for (int i2 = 0; i2 < lightsTypeCounter; i2++) {
      for (int i = 0; i < lightsStoreSizeByType[i2]; i++) {
	if(&lightsStore[i] == mouse.selectedThing){
	  //   continue;
	}
    
	uniformVec3(lightSourceShader, "color", lightsStore[i2][i].color);

	uniformMat4(lightSourceShader, "model", lightsStore[i2][i].mat.m);

	glDrawArrays(GL_TRIANGLES, 0, cube.vertexNum);
      }
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
      
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 
  }

  glUseProgram(shadersId[mainShader]); 
  
// render mouse.brush
    if(mouse.selectedType == mouseTileT){
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

  	static vec3 prevTile = {-1,-1,-1};

	if(mouse.brushType == mouseBlockBrushT){
	  TileBlock* block = (TileBlock*) mouse.brushThing;
	  
	  if((prevTile.x != -1 && prevTile.y != -1 && prevTile.z != -1)
	     && (prevTile.x != tile.x || prevTile.y != tile.y || prevTile.z != tile.z)){   
	    block->mat = IDENTITY_MATRIX; 

	    rotateY(block->mat.m, rad(block->rotateAngle)); 
			
	    block->mat.m[12] = tile.x;
	    block->mat.m[13] = tile.y;
	    block->mat.m[14] = tile.z;

	    block->tile = tileData->tile;
	    
	    int index = (block->rotateAngle) / 90;

	    //	    block->mat.m[12] += rotationBlock[index][0];
	    //	    block->mat.m[14] += rotationBlock[index][1];
	    
	    //	    
	    if(block->rotateAngle == 90){
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
	}else if(mouse.brushType == mouseWallBrushT && mouse.tileSide != -1 && mouse.tileSide != center){
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
	    [bot] = { 180, 1, 1 },
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
	    uniformMat4(mainShader, "model", wal.mat.m);
	    
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

	  if(mouse.clickR){ 
	    wal.planes = calloc(wallsVPairs[wal.type].planesNum, sizeof(Plane));

	    if(!tileData->tile){
	      printf("Calllllllll\n");
	      grid[curFloor][tileData->grid.z][tileData->grid.x] = calloc(1,sizeof(Tile));
	      tileData->tile = grid[curFloor][tileData->grid.z][tileData->grid.x];
	    }
	    
	    for(int i=0;i<wallsVPairs[wal.type].planesNum;i++){
	      calculateAABB(wal.mat, wallsVPairs[wal.type].pairs[i].vBuf, wallsVPairs[wal.type].pairs[i].vertexNum, wallsVPairs[wal.type].pairs[i].attrSize, &wal.planes[i].lb, &wal.planes[i].rt);
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

	    batchGeometry();
	  }
	}
      }
}
//}

void editor2dRender(){
    
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
      case(mouseTileT):{
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

  // aim render
  if(!curMenu && hints){
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
  if((mouse.brushType || mouse.brushThing) && !curMenu && hints){
    baseYPad = letterH;
      
    char buf[130];
      
    sprintf(buf, "On brush [%s] [X] to discard", mouseBrushTypeStr[mouse.brushType]);

    renderText(buf, -1.0f, 1.0f, 1.0f);

    switch(mouse.brushType){
    case(mouseTextureBrushT):{
      if(mouse.clickL){ 
	Texture* texture = (Texture*)mouse.brushThing;
	  
	if(mouse.selectedType == mouseWallT){
	  WallMouseData* wallData = (WallMouseData*)mouse.selectedThing;

	  if(wallData->type == windowT && wallData->plane <= winCenterBackPlane){
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
		   
	    wallData->tile->walls[wallData->side].planes[winPlanePairs[wallData->plane]].txIndex = texture->index1D;  
	  }

	  if(wallData->type == doorT && wallData->plane <= doorCenterBackPlane){
	    static const int doorPlanePairs[2] = {
	      [doorCenterFrontPlane] = doorCenterBackPlane,
	      [doorCenterBackPlane] = doorCenterFrontPlane,
	    };		   
	    wallData->tile->walls[wallData->side].planes[doorPlanePairs[wallData->plane]].txIndex = texture->index1D;  
	  }
	    
	  if(wallData->type == wallJointT){
	    wallData->tile->joint[wallData->side][wallData->plane].txIndex = texture->index1D;
	  }else{
	    wallData->tile->walls[wallData->side].planes[wallData->plane].txIndex = texture->index1D;
	  }

	  batchGeometry();
	}else if(mouse.selectedType == mouseTileT){
	  TileMouseData* tileData = (TileMouseData*)mouse.selectedThing; 

	  if(!tileData->tile){
	    grid[curFloor][tileData->grid.z][tileData->grid.x] = calloc(1, sizeof(Tile));
	    printf("Calloc TIle\n");
	  }
	    
	  setIn(grid[curFloor][tileData->grid.z][tileData->grid.x]->ground, 0, texturedTile);
	  setIn(grid[curFloor][tileData->grid.z][tileData->grid.x]->ground, tileData->groundInter, texture->index1D);
	  batchGeometry();
	}else if(mouse.selectedType == mouseBlockT){
	  TileBlock* block = (TileBlock*)mouse.selectedThing; 

	  block->txIndex = texture->index1D;
	  batchGeometry();
	}else if(mouse.selectedType == mousePlaneT){ 
	  Picture* plane = (Picture*)mouse.selectedThing;

	  plane->txIndex = texture->index1D; 
	  batchGeometry();
	}
      }
	  
      break;
    }case(mouseBlockBrushT):{
       if (mouse.clickR && mouse.selectedType == mouseTileT) {
	 TileMouseData* tileData = (TileMouseData*) mouse.selectedThing;
	 TileBlock* block = (TileBlock*) mouse.brushThing;

	 if (!tileData->tile) {
	   grid[curFloor][tileData->grid.z][tileData->grid.x] = calloc(1, sizeof(Tile));
	   printf("Calloc TIle Block\n");
	 }else{
	   printf("%d \n", tileData->tile->block);
	 }

	 grid[curFloor][tileData->grid.z][tileData->grid.x]->block = malloc(sizeof(TileBlock));
	 memcpy(grid[curFloor][tileData->grid.z][tileData->grid.x]->block, block, sizeof(TileBlock));

	 TileBlock* newBlock = grid[curFloor][tileData->grid.z][tileData->grid.x]->block;
	 //	 newBlock->mat = IDENTITY_MATRIX;

	 vec3 tile = xyz_indexesToCoords(tileData->grid.x, curFloor, tileData->grid.z);

	 int index = (block->rotateAngle) / 90;

	 newBlock->mat = IDENTITY_MATRIX;
	 memcpy(newBlock->mat.m, block->mat.m, sizeof(float) * 16);
	 //	 memcpy(newBlock->mat.m,)

	 //	 newBlock->mat.m[12] = tile.x + rotationBlock[index][0];
	 //	 newBlock->mat.m[13] = tile.y;
	 //	 newBlock->mat.m[14] = tile.z + rotationBlock[index][1];

	 newBlock->tile = tileData->tile;

	 printf("new tile %f %f %f \n", tile.x, tile.y, tile.z);

	 calculateAABB(newBlock->mat, blocksVPairs[newBlock->type].pairs[0].vBuf, blocksVPairs[newBlock->type].pairs[0].vertexNum, blocksVPairs[newBlock->type].pairs[0].attrSize, &newBlock->lb, &newBlock->rt);

	 printf("new lb %f %f %f \n", argVec3(newBlock->lb));
	 printf("new rt %f %f %f \n\n", argVec3(newBlock->rt));
	 
	 //mouse.brushThing = constructNewBlock(block->type, block->rotateAngle);
	 batchGeometry(); 
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
     }case(mouseLightT):{
	Light* light = (Light*)mouse.focusedThing;
	sprintf(buf, "Focused light [%s:%d] Mode: [%s] Step: [%f]", lightTypesStr[light->type],light->id, manipulationModeStr[manipulationMode], manipulationStep);
	
	break;
      }
    default: break;
    }

    renderText(buf, -1.0f, 1.0f - baseYPad, 1.0f);
  }else if(mouse.selectedThing && !curMenu && hints){ 
    char buf[164]; 

    switch(mouse.selectedType){
    case(mouseLightT):{
      Light* light = (Light*)mouse.selectedThing;  

      sprintf(buf, "Selected light [%s:%d]",lightTypesStr[light->type],light->id);
	
      break;
    }
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
	    sprintf(buf, "Selected empty tile");  
	  }
	
	  break; 
	}
    default: break;
    }
       
    renderText(buf, -1.0f, 1.0f - baseYPad, 1.0f);    
  }
    
  // render objects menu
  if(curMenu && curMenu->type == objectsMenuT){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, solidColorTx);
    setSolidColorTx(blackColor, 1.0f);
      
    glBindVertexArray(objectsMenu.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, objectsMenu.VBO);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float mouseY = mouse.cursor.z;
      
    int selectedIndex = (1.0f - ((mouseY +1.0f)/2.0f)) / (letterH/2) ;
    selectedIndex++;
      
    if(mouse.cursor.x <= objectsMenuWidth && selectedIndex <= modelsTypesInfo[objectsMenuSelectedType].counter){
      if(mouse.clickL){
	mouse.focusedThing = NULL;
	mouse.focusedType = 0;
	  
	curModelsSize++;

	if(curModels){ 
	  curModels = realloc(curModels, curModelsSize * sizeof(Model));
	  printf("Calloc moder\n");
	}else{
	  curModels = malloc(sizeof(Model));
	  printf("Calloc moder\n");
	}

	curModels[curModelsSize-1].id = curModelsSize-1;

	int index1D = loadedModels2D[objectsMenuSelectedType][selectedIndex - 1].index1D;

	curModels[curModelsSize-1].name = index1D;
	curModels[curModelsSize-1].characterId = -1; 

	// if type == char add new character
	  
	  
	curModels[curModelsSize-1].mat = IDENTITY_MATRIX;  

	//scale(&curModels[curModelsSize-1].mat, 0.25f, 0.25f, 0.25f); 

	if(mouse.selectedType == mouseTileT){
	  TileMouseData* tileData =  (TileMouseData*) mouse.selectedThing;
	  vec3 tile = xyz_indexesToCoords(tileData->grid.x, curFloor, tileData->grid.z);
	    
	  curModels[curModelsSize-1].mat.m[12] = tile.x;
	  curModels[curModelsSize-1].mat.m[13] = tile.y;
	  curModels[curModelsSize-1].mat.m[14] = tile.z;
	  
	  curModels[curModelsSize-1].mat.m[13] += loadedModels2D[objectsMenuSelectedType][selectedIndex - 1].modelSizes.y;
	}else{
	  vec3 modelSize = loadedModels2D[objectsMenuSelectedType][selectedIndex - 1].modelSizes;
	  
	  curModels[curModelsSize-1].mat.m[12] = (mouse.rayDir.x*modelSize.x*4.0f) +curCamera->pos.x;
	  curModels[curModelsSize-1].mat.m[13] = mouse.rayDir.y +curCamera->pos.y;
	  curModels[curModelsSize-1].mat.m[14] = (mouse.rayDir.z*modelSize.z*4.0f) +curCamera->pos.z;
	}
	  calculateModelAABB(&curModels[curModelsSize-1]);
	  
	  objectsMenu.open = false;
	  curMenu = NULL;
	};

	setSolidColorTx(redColor, 1.0f);
      
      glBindVertexArray(hudRect.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, hudRect.VBO);
	  
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
    glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, solidColorTx);

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
	
      glBindVertexArray(hudRect.VAO);
	  glBindBuffer(GL_ARRAY_BUFFER, hudRect.VBO);
	  
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
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, solidColorTx);
    setSolidColorTx(blackColor, 1.0f);
      
    glBindVertexArray(objectsMenu.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, objectsMenu.VBO);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float mouseY = mouse.cursor.z;
      
    int selectedIndex = (1.0f-((mouseY +1.0f)/2.0f)) / (letterH/2.0f);
    selectedIndex++;
      
    if(mouse.cursor.x <= objectsMenuWidth && selectedIndex <= tileBlocksCounter){
      if(mouse.clickL){
	if(mouse.brushType == mouseBlockBrushT){
	  TileBlock* block = (TileBlock*)mouse.brushThing;
	  free(block);
		
	  mouse.brushType = 0;
	  mouse.brushThing = NULL;
	}

	//	if(mouse.brushType == mouseBlockBrushT){
	//	  free(mouse.brushThing);
	//	}

	mouse.brushThing = constructNewBlock(selectedIndex-1, 0);
	mouse.brushType = mouseBlockBrushT;
	  
	curMenu->open = false; 
	curMenu = NULL;
      };

      setSolidColorTx(redColor, 1.0f);
      
      glBindVertexArray(hudRect.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, hudRect.VBO);

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
  }else if(curMenu && curMenu->type == lightMenuT){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, solidColorTx);
    setSolidColorTx(blackColor, 1.0f);
      
    glBindVertexArray(objectsMenu.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, objectsMenu.VBO);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float mouseY = mouse.cursor.z;
      
    int selectedIndex = (1.0f-((mouseY +1.0f)/2.0f)) / (letterH/2);
    selectedIndex++;
      
    if(mouse.cursor.x <= objectsMenuWidth && selectedIndex <= lightsTypeCounter){
      if(mouse.clickL){
	createLight(curCamera->pos, selectedIndex-1);
	  
	curMenu->open = false; 
	curMenu = NULL;
      };

      setSolidColorTx(redColor, 1.0f);
      
      glBindVertexArray(hudRect.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, hudRect.VBO);
	  
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

    for(int i=0;i<lightsTypeCounter;i++){
      renderText(lightTypesStr[i], -1.0f, 1.0f - (i * letterH), 1);
    }
  }else if(curMenu && curMenu->type == texturesMenuT){
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

    glActiveTexture(GL_TEXTURE0);;
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

    float mouseY = mouse.cursor.z;
      
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
      
      glBindVertexArray(hudRect.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, hudRect.VBO);

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

  // render cursor
  if(curMenu || cursorMode)
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

	
      /*      float cursorPoint[] = {
	mouse.cursor.x + cursorW * 0.05f, mouse.cursor.z + cursorH, 0.0f, 0.0f,
	mouse.cursor.x + cursorW, mouse.cursor.z + cursorH/2.0f, 0.0f, 0.0f,
	mouse.cursor.x, mouse.cursor.z + cursorH / 4.0f, 0.0f, 0.0f, 
	};

	float cursorPoint[] = {
	x + cursorW * 0.05f, z - cursorH, 0.0f, 0.0f,
	x + cursorW, z - cursorH/2.0f,    0.0f, 0.0f,
	x, z - cursorH / 4.0f,            0.0f, 0.0f, 
	};
      */

      float cursorPoint[] = {
	x, z,            0.0f, 0.0f,
	x + cursorW * 0.05f, z - cursorH,            0.0f, 0.0f,
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
    
  lightsStoreSize++;
  lightsStoreSizeByType[type]++;

  if(!lightsStore[type]){
    lightsStore[type] = malloc(sizeof(Light));
  }else{
    lightsStore[type] = realloc(lightsStore[type], sizeof(Light) * lightsStoreSizeByType[type]);
  }

  int indexOfNew = lightsStoreSizeByType[type]-1;
	    
  memcpy(&lightsStore[type][indexOfNew], &lightDef, sizeof(Light));
  
  lightsStore[type][indexOfNew].pos = pos;
  lightsStore[type][indexOfNew].id = lightsStoreSizeByType[type] - 1;
  lightsStore[type][indexOfNew].type = type;

  if(type == shadowPointLightT){
    lightsStore[type][indexOfNew].cubemapIndex = lastUsedCubemap;
    lastUsedCubemap++; 
  }
	  
  lightsStore[type][indexOfNew].mat = IDENTITY_MATRIX;
  lightsStore[type][indexOfNew].mat.m[12] = pos.x;
  lightsStore[type][indexOfNew].mat.m[13] = pos.y;
  lightsStore[type][indexOfNew].mat.m[14] = pos.z;

  calculateAABB(lightsStore[type][indexOfNew].mat, cube.vBuf, cube.vertexNum, cube.attrSize,
		&lightsStore[type][indexOfNew].lb, &lightsStore[type][indexOfNew].rt);

  mouse.focusedType = mouseLightT; 
  mouse.focusedThing = &lightsStore[type][indexOfNew];

  // update light info in main shader
  uniformLights();
  rerenderShadowForLight(lightsStore[type][indexOfNew].id);
}

void uniformLights(){
  GLint curShader = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &curShader);

  glUseProgram(shadersId[mainShader]);

    char buf[64];

    static const char* shaderVarSufixStr[] = {
      [pointLightT] = "point",
      [shadowPointLightT] = "shadowPoint"
    };

    uniformFloat(mainShader, "radius", max(gridX / 2.0f, gridZ / 2.0f));

    int localLightsCounter[lightsTypeCounter] = { 0 };
    
    for (int i2 = 0; i2 < lightsTypeCounter; i2++) {
      for (int i = 0; i < lightsStoreSizeByType[i2]; i++) {
	if (lightsStore[i2][i].off) {
	  continue;
	}

	sprintf(buf, "%sLights[%i].pos",
		shaderVarSufixStr[i2], i);
	uniformVec3(mainShader, buf, (vec3) { lightsStore[i2][i].mat.m[12], lightsStore[i2][i].mat.m[13], lightsStore[i2][i].mat.m[14], });

	sprintf(buf, "%sLights[%i].color",
		shaderVarSufixStr[i2], i);
	uniformVec3(mainShader, buf, lightsStore[i2][i].color);
		    

	sprintf(buf, "%sLights[%i].constant",
		shaderVarSufixStr[i2], i);
	uniformFloat(mainShader, buf, 1.0f);

	sprintf(buf, "%sLights[%i].linear",
		shaderVarSufixStr[i2], i);
	uniformFloat(mainShader, buf, lightPresetTable[lightsStore[i2][i].curLightPresetIndex][0]);

	sprintf(buf, "%sLights[%i].qaudratic",
		shaderVarSufixStr[i2], i);
	uniformFloat(mainShader, buf, lightPresetTable[lightsStore[i2][i].curLightPresetIndex][1]);

	sprintf(buf, "%sLights[%i].cubemapIndex",
		shaderVarSufixStr[i2], i);
	uniformInt(mainShader, buf, lightsStore[i2][i].cubemapIndex);
	printf("cubemap: %d \n",  lightsStore[i2][i].cubemapIndex);


	localLightsCounter[lightsStore[i2][i].type]++;
      }
      
      sprintf(buf, "%sLightsSize",
	      shaderVarSufixStr[i2]);
      uniformInt(mainShader, buf, lightsStoreSizeByType[i2]);
    }

    glUseProgram(shadersId[pointShadowShader]);
    uniformInt(pointShadowShader, "lightsSize", lightsStoreSizeByType[shadowPointLightT]);

    for(int i=0;i<lightsStoreSizeByType[shadowPointLightT];i++){
      vec3 pos = { lightsStore[shadowPointLightT][i].mat.m[12], lightsStore[shadowPointLightT][i].mat.m[13], lightsStore[shadowPointLightT][i].mat.m[14] };
      
      sprintf(buf, "lightsPos[%d]", i);
      uniformVec3(pointShadowShader, buf, pos);
    }

  glUseProgram(curShader);
}

