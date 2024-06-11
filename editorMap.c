#include "deps.h"
#include "linearAlg.h"
#include "main.h"
#include "game.h"
#include "editorMap.h"

void editorMapOnSetInstance(){
  printf("Now editorMap\n");
  // curCamera->pos.y = cameraFloor;

  //  glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
}

typedef struct{
  float scale; // of circle
  vec2 mapPos; // with def map scale (1.0f)

  char* saveName;
} LocationCircle;

LocationCircle* locationCircles;
LocationCircle* selectedLocation;
int locationCirclesSize;

float zoomLevel = 1.0f;

MeshBuffer worldMapBuf;
MeshBuffer cursorBuf;

MeshBuffer circleBuf;

int mapTx;

//float mouse.cursor.x;
//float mouse.cursor.z;

float screenRatio;
 
vec2 offset;

bool mouseDown;

bool locationIsDragging;

float scaleY;

#define translateToGlobalCoords(x1,y1) (x1 / zoomLevel) - offset.x, (y1 / zoomLevel) - offset.z

void editorMap2dRender(){
  {
    renderText("WorldMap editor",-1.0f, 1.0f,1.0f);
  }
  
  // map
  {
    glUseProgram(shadersId[UITransfTx]);

    glBindTexture(GL_TEXTURE_2D,loadedTextures1D[mapTx].tx);

    // float translateX = mouse.cursor.x * (1.0f - 1.0f / zoomLevel);
    //    float translateY = mouse.cursor.z * (1.0f - 1.0f / zoomLevel);

    uniformVec2(UITransfTx, "model2D", (vec2){ zoomLevel, zoomLevel });
    uniformVec2(UITransfTx, "offset", offset);
    
    glBindVertexArray(worldMapBuf.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, worldMapBuf.VBO);

    glDrawArrays(GL_TRIANGLES, 0, worldMapBuf.VBOsize); 

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(shadersId[hudShader]);
  }


  // circle
  glUseProgram(shadersId[UITransfColor]);
  for(int i=0;i<locationCirclesSize;i++){
    float trasfCircleX = zoomLevel * (offset.x + locationCircles[i].mapPos.x);
    float trasfCircleZ = zoomLevel * (offset.z + locationCircles[i].mapPos.z);
    
    if(trasfCircleX > 1.0f || trasfCircleX < -1.0f){
      continue;
    }

    if(trasfCircleZ > 1.0f || trasfCircleZ < -1.0f){
      continue;
    }
    
    uniformVec2(UITransfColor, "offset", (vec2){
	trasfCircleX / zoomLevel,
	trasfCircleZ  / zoomLevel
      });
    
    uniformVec2(UITransfColor, "model2D", (vec2){ zoomLevel, zoomLevel });  

    vec2 circleCenter = { trasfCircleX, trasfCircleZ };

    float xRad = .025f / screenRatio * zoomLevel * locationCircles[i].scale;
    float yRad = .025f * zoomLevel * locationCircles[i].scale;

    float xFm = ((mouse.cursor.x - circleCenter.x) / xRad) * ((mouse.cursor.x - circleCenter.x) / xRad);
    float yFm = ((mouse.cursor.z - circleCenter.z) / yRad) * ((mouse.cursor.z - circleCenter.z) / yRad); 

    float distance = xFm + yFm;

    if(distance <= 1){
      if(!locationIsDragging && curUIBuf.rectsSize == 0){
	uniformVec3(UITransfColor, "color", (vec3){ greenColor });
	selectedLocation = &locationCircles[i];
      }
    }else{
      uniformVec3(UITransfColor, "color", (vec3){ blackColor });
    }
    
    glBindVertexArray(circleBuf.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, circleBuf.VBO);

    glDrawArrays(GL_TRIANGLES, 0, circleBuf.VBOsize);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    if(locationCircles[i].saveName){
      glUseProgram(shadersId[hudShader]);

      renderText(locationCircles[i].saveName,
		 zoomLevel * (trasfCircleX / zoomLevel) - ((strlen(locationCircles[i].saveName) + 1) * letterW) / 2.0f,
		 zoomLevel * ((trasfCircleZ  / zoomLevel) - .025f)
		 ,1.0f);
      
      glUseProgram(shadersId[UITransfColor]);
    }
  }
  glUseProgram(shadersId[hudShader]);
  
  if(mouseDown){
    glUseProgram(shadersId[UITransfTx]);
    uniformVec2(UITransfTx, "offset", (vec2){ mouse.cursor.x, mouse.cursor.z });
  }
};

void editorMap3dRender(){
    
};

void editorMapPreFrame(float deltaTime){
  if(!locationIsDragging && curUIBuf.rectsSize == 0){
    selectedLocation = NULL;
  }
};

void editorMapMatsSetup(int curShader){

};


void editorMapPreLoop() {
  {
    UIRect2* attachSaveWindow;
    int attachSaveWindowSize;
 
    float w = 0.3f;
    float h = 0.1f + letterH;

    // background
    {
      attachSaveWindow = calloc(4, sizeof(UIRect2));
      attachSaveWindowSize = 4;
        
      attachSaveWindow[0] = (UIRect2){ .pos = {
	  { -w, -h }, { -w, h }, { w, h },
	  { -w, -h }, { w, h }, { w, -h }
	},
	.c = { blackColor, 1.0f },
	.lb = {w, h}, .rt = {-w, -h}
      };

      attachSaveWindow[0].textPos = (vec2){ -w, h + letterH };
      attachSaveWindow[0].text = malloc(sizeof(char) * strlen("Attach map"));
      strcpy(attachSaveWindow[0].text, "Attach map");
    }

    // input
    {
      float inputLeftW = -w + (strlen("Save name:")+1) * letterW;
      float inputRightW = w - 0.03f;

      float inputTopH = h - 0.02f;
      float inputBotH = 0.0f + h/2;
    
      attachSaveWindow[1] = (UIRect2){ .pos = {
	  { inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
	  { inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
	},
	.lb = {inputLeftW, inputBotH}, .rt = {inputRightW, inputTopH},
	.c = { greenColor, 1.0f }
      };

      attachSaveWindow[1].input = calloc(1, sizeof(TextInput2));
      attachSaveWindow[1].input->limit = 15;
      attachSaveWindow[1].input->relatedUIRect = &attachSaveWindow[2];

      attachSaveWindow[1].textPos = (vec2){inputLeftW - (strlen("Save name:")+1) * letterW, inputBotH + letterH};
      attachSaveWindow[1].text = malloc(sizeof(char) * strlen("Save name:"));
      strcpy(attachSaveWindow[1].text, "Save name:");
    }

    // save button
    {
      float inputLeftW = -w + 0.1f;
      float inputRightW = -w + 0.1f + (strlen("attach")+1) * letterW;

      float inputTopH = -h + letterH + 0.02f;
      float inputBotH = -h + 0.02f;
    
      attachSaveWindow[2] = (UIRect2){ .pos = {
	  { inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
	  { inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
	},
	.lb = {inputLeftW, inputBotH}, .rt = {inputRightW, inputTopH},
	.c = { greenColor, 1.0f }
      };

      attachSaveWindow[2].onClick = attachSaveNameToLocation;
      
      attachSaveWindow[2].highlight = malloc(sizeof(MeshBuffer));
      bindUIQuad((vec2[6]) {
	  { inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
	  { inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
	},(uint8_t[4]) { redColor, 1.0f }, attachSaveWindow[2].highlight);

      attachSaveWindow[2].textPos = (vec2){inputLeftW, inputBotH + letterH };
      attachSaveWindow[2].text = malloc(sizeof(char) * strlen("attach"));
      strcpy(attachSaveWindow[2].text, "attach");
    }

    // cancel button
    {
      float inputLeftW = w - 0.1f - (strlen("cancel")+1) * letterW;
      float inputRightW = w - 0.1f;

      float inputTopH = -h + letterH + 0.02f;
      float inputBotH = -h + 0.02f;
    
      attachSaveWindow[3] = (UIRect2){ .pos = {
	  { inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
	  { inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
	},
	.lb = {inputLeftW, inputBotH}, .rt = {inputRightW, inputTopH},
	.c = { greenColor, 1.0f }
      };

      attachSaveWindow[3].onClick = clearCurrentUI;

      attachSaveWindow[3].highlight = malloc(sizeof(MeshBuffer));
      bindUIQuad((vec2[6]) {
	  { inputLeftW, inputBotH },{ inputLeftW, inputTopH }, { inputRightW, inputTopH },
	  { inputLeftW, inputBotH },{ inputRightW, inputTopH }, { inputRightW, inputBotH },
	},(uint8_t[4]) { redColor, 1.0f }, attachSaveWindow[3].highlight);

      attachSaveWindow[3].textPos = (vec2){inputLeftW, inputBotH + letterH };
      attachSaveWindow[3].text = malloc(sizeof(char) * strlen("cancel"));
      strcpy(attachSaveWindow[3].text, "cancel");
    }

    UIStructBufs[attachSaveWindowT] = batchUI(attachSaveWindow, attachSaveWindowSize);
  }

  screenRatio = windowW / windowH;
  
  // setup 2d circle
  {
    float angle = 360.0f / 8.0f;
    int triNum = 8 - 2;

    float r = .03f;

    vec2 points[8];

    for(int i=0;i<8;i++){
      float curAngle = angle * i;

      float x = (r/screenRatio) * cos(rad(curAngle));
      float y = (r) * sin(rad(curAngle));

      points[i] = (vec2){x, y};
    }

    float* circle2d = malloc((sizeof(float) * triNum * 18));

    int index = 0;
    for(int i=0;i<triNum;i++){
      {
	circle2d[index] = points[0].x;
	circle2d[index+1] = points[0].z;

	circle2d[index+2] = 0.0f;
	circle2d[index+3] = 0.0f;
	circle2d[index+4] = 0.0f;
	circle2d[index+5] = 0.5f;
      }

      {
	circle2d[index+6] = points[i+1].x;
	circle2d[index+7] = points[i+1].z;

	circle2d[index+8] = 0.0f;
	circle2d[index+9] = 0.0f;
	circle2d[index+10] = 0.0f;
	circle2d[index+11] = 0.5f;
      }

      {
	circle2d[index+12] = points[i+2].x;
	circle2d[index+13] = points[i+2].z;

	circle2d[index+14] = 0.0f;
	circle2d[index+15] = 0.0f;
	circle2d[index+16] = 0.0f;
	circle2d[index+17] = 0.5f;
      }
      
      index += 18;
    }

    glGenBuffers(1, &circleBuf.VBO);
    glGenVertexArrays(1, &circleBuf.VAO);
  
    glBindVertexArray(circleBuf.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, circleBuf.VBO);

    circleBuf.VBOsize = triNum * 3;
  
    glBufferData(GL_ARRAY_BUFFER, (sizeof(float) * triNum * 18), circle2d, GL_STATIC_DRAW);
  
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    free(circle2d);
  }

  mapTx = texture1DIndexByName("fallout2Map");

  float imgRatio = (float)loadedTextures1D[mapTx].w / (float)loadedTextures1D[mapTx].h;

  //    float scaleX, scaleY;
  float scaleX;

  if (screenRatio > imgRatio) {
    scaleY = 1.0f;
    scaleX = imgRatio / screenRatio;
  }
  else {
    scaleX = 1.0f;
    scaleY = screenRatio / imgRatio;
  }

  printf("scaleY %f \n", scaleY);

  bindUIQuadTx((vec4[6]) {
      { -scaleX, -scaleY, 0.0f, 0.0f },
      { -scaleX, scaleY, 0.0f, 1.0f },
      { scaleX, scaleY, 1.0f, 1.0f },

      { -scaleX, -scaleY, 0.0f, 0.0f },
      { scaleX, scaleY, 1.0f, 1.0f },
      { scaleX, -scaleY, 1.0f, 0.0f }
    }, & worldMapBuf);

  // cursor
  {
    bindUITri((vec2[3]) {
        { 0.0f, 0.0f },
        { cursorW * 0.05f, -cursorH },
        { cursorW, -cursorH + (cursorH * 0.5f) }
      },
      (uint8_t[4]) {
        whiteColor, 1.0f
      }, & cursorBuf);
  }
};

void editorMapEvents(SDL_Event event){
  if (event.type == SDL_KEYDOWN){
    if(!selectedTextInput2 && curUIBuf.rectsSize == 0) {
      if(selectedLocation && event.key.keysym.scancode == SDL_SCANCODE_A){
	if(curUIBuf.rects != UIStructBufs[attachSaveWindowT]->rects){
	  memcpy(&curUIBuf, UIStructBufs[attachSaveWindowT], sizeof(UIBuf));
	
	  selectedTextInput2 = curUIBuf.rects[1].input;
	  textInputCursorMat.x = curUIBuf.rects[1].pos[0].x;
	  textInputCursorMat.z = curUIBuf.rects[1].pos[0].z;
	}
      }
    
      if(event.key.keysym.scancode == SDL_SCANCODE_F) {
	locationCirclesSize++;
	
	if(!locationCircles){
	  locationCircles = malloc(sizeof(LocationCircle));
	}else{
	  locationCircles = realloc(locationCircles, sizeof(LocationCircle) * locationCirclesSize);
	}

	locationCircles[locationCirclesSize-1].saveName = NULL;
	locationCircles[locationCirclesSize-1].scale = 1.0f;
	locationCircles[locationCirclesSize-1].mapPos = (vec2){ translateToGlobalCoords(mouse.cursor.x, mouse.cursor.z) };
      }
    }
  }

  if (event.type == SDL_MOUSEMOTION) {
    float x = -1.0 + 2.0 * (event.motion.x / windowW);
    float y = -(-1.0 + 2.0 * (event.motion.y / windowH));

    //if (curMenu /*|| cursorMode */|| curUIBuf.rectsSize != 0) {
      mouse.lastCursor.x = mouse.cursor.x;
      mouse.lastCursor.z = mouse.cursor.z;

      mouse.cursor.x = x;
      mouse.cursor.z = y;
    //}
  }
  
  if (event.type == SDL_MOUSEWHEEL && curUIBuf.rectsSize == 0) {
    bool mouseVsCircle = false;
    
    if(selectedLocation){
      if (event.wheel.y > 0) {
	//	selectedLocation->scale += 0.01f;
      }else if (event.wheel.y < 0) {
	//	selectedLocation->scale -= 0.01f;
	

	
      }

    }else{
      if (event.wheel.y > 0) {
	zoomLevel += 0.05f; 
      }else if (event.wheel.y < 0 && zoomLevel >= 1.05f) {
	zoomLevel -= 0.05f;
      }

      glUseProgram(shadersId[UITransfTx]);
      uniformVec2(UITransfTx, "model2D", (vec2){ zoomLevel, zoomLevel });

      float topLimitY = (zoomLevel * ((offset.z)+scaleY));
      float botLimitY = (zoomLevel * ((offset.z)+(-scaleY)));

      if(botLimitY > -1.0f){
	offset.z -= 1.0f + botLimitY; 
      }

      if(topLimitY < 1.0f){
	offset.z += 1.0f - topLimitY; 
      }
    }
  }

  if(event.type == SDL_MOUSEMOTION && mouse.leftDown && curUIBuf.rectsSize == 0){
    float xoffset = event.motion.xrel;
    float yoffset = -event.motion.yrel;
    
    if(selectedLocation || locationIsDragging){
      vec2 gCursor = { translateToGlobalCoords(mouse.cursor.x, mouse.cursor.z) };
      
      selectedLocation->mapPos.x = gCursor.x;
      selectedLocation->mapPos.z = gCursor.z;
      
      locationIsDragging = true;
    }else{
      float dx = xoffset * (0.0001f * 1.5f);
      float dz = yoffset * (0.0001f * 1.5f) * screenRatio;
      
      float topLimitY = (zoomLevel * ((offset.z+dz)+scaleY));
      float botLimitY = (zoomLevel * ((offset.z+dz)+(-scaleY)));

      printf("dz: %f\n", dz);

      if(topLimitY > 1.0f && botLimitY < -1.0f){
	offset.z += dz;
      }
    
      offset.x += dx;
    }
  }

  if(event.type == SDL_MOUSEBUTTONDOWN){
    mouse.leftDown = event.button.button == SDL_BUTTON_LEFT;
  }

  if(event.type == SDL_MOUSEBUTTONUP){
    mouse.clickL = event.button.button == SDL_BUTTON_LEFT;
    mouse.leftDown = false;
    locationIsDragging = false;
  }
};

void editorMapMouseVS(){

}


void attachSaveNameToLocation(){
  if(UIStructBufs[attachSaveWindowT]->rects[1].input->buf && strlen(UIStructBufs[attachSaveWindowT]->rects[1].input->buf)){}
  else{
    if (UIStructBufs[attachSaveWindowT]->rects[2].onclickResText) {
      free(UIStructBufs[attachSaveWindowT]->rects[2].onclickResText);
    }
    
    UIStructBufs[attachSaveWindowT]->rects[2].onclickResText = malloc(sizeof(char) * strlen("Provide save name!"));
    strcpy(UIStructBufs[attachSaveWindowT]->rects[2].onclickResText, "Provide save name!");
  }
  
  char* save = calloc((strlen(UIStructBufs[attachSaveWindowT]->rects[1].input->buf) + strlen(".doomer")), sizeof(char));

  strcat(save, UIStructBufs[attachSaveWindowT]->rects[1].input->buf);
  strcat(save, ".doomer");

  FILE* map = fopen(save, "r");
  free(save);
  
  if(map){
    selectedLocation->saveName = malloc(sizeof(char) * strlen(UIStructBufs[attachSaveWindowT]->rects[1].input->buf));
    strcpy(selectedLocation->saveName, UIStructBufs[attachSaveWindowT]->rects[1].input->buf);
    
    clearCurrentUI();
    fclose(map);
  }else{
    if (UIStructBufs[attachSaveWindowT]->rects[2].onclickResText) {
      free(UIStructBufs[attachSaveWindowT]->rects[2].onclickResText);
    }
    
    UIStructBufs[attachSaveWindowT]->rects[2].onclickResText = malloc(sizeof(char) * strlen("Save doesnt exist!"));
    strcpy(UIStructBufs[attachSaveWindowT]->rects[2].onclickResText, "Save doesnt exist!");
  }
}

void editorMapRenderCursor(){
  // cursor 
  {
    glUseProgram(shadersId[hudShader]);
    
    char buf[64];
    sprintf(buf, "%f %f", offset.x - (mouse.cursor.x/ zoomLevel), offset.z - (mouse.cursor.z/zoomLevel));
    renderText(buf, mouse.cursor.x, mouse.cursor.z - 0.01f,1.0f);
    
    glUseProgram(shadersId[UITransfShader]);
    uniformVec2(UITransfShader, "model2D", (vec2){ mouse.cursor.x, mouse.cursor.z });
    
    glBindVertexArray(cursorBuf.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, cursorBuf.VBO);

    glDrawArrays(GL_TRIANGLES, 0, cursorBuf.VBOsize);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(shadersId[hudShader]);
  }
}
