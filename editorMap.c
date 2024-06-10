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
} LocationCircle;

LocationCircle* locationCircles;
int locationCirclesSize;

float zoomLevel = 1.0f;

MeshBuffer worldMapBuf;
MeshBuffer cursorBuf;

MeshBuffer circleBuf;

int mapTx;

float cursorX;
float cursorZ;

float screenRatio;
 
vec2 offset;

bool mouseDown;

float scaleY;

#define translateToGlobalCoords(x1,y1) offset.x - (x1 / zoomLevel), offset.z - (y1 / zoomLevel)

void editorMap2dRender(){
  {
    renderText("WorldMap editor",-1.0f, 1.0f,1.0f);
  }
  
  int xx, yy;
  SDL_GetMouseState(&xx, &yy);

  cursorX = -1.0 + 2.0 * (xx / windowW);
  cursorZ = -(-1.0 + 2.0 * (yy / windowH));
  
  // map
  {
    glUseProgram(shadersId[UITransfTx]);

    glBindTexture(GL_TEXTURE_2D,loadedTextures1D[mapTx].tx);

    // float translateX = cursorX * (1.0f - 1.0f / zoomLevel);
    //    float translateY = cursorZ * (1.0f - 1.0f / zoomLevel);

    uniformVec2(UITransfTx, "model2D", (vec2){ zoomLevel, zoomLevel });
    uniformVec2(UITransfTx, "offset", offset);
    
    glBindVertexArray(worldMapBuf.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, worldMapBuf.VBO);

    glDrawArrays(GL_TRIANGLES, 0, worldMapBuf.VBOsize); 

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(shadersId[hudShader]);
  }

  // cursor 
  {
    glUseProgram(shadersId[hudShader]);
    
    char buf[64];
    sprintf(buf, "%f %f", offset.x - (cursorX/ zoomLevel), offset.z - (cursorZ/zoomLevel));
    renderText(buf, cursorX, cursorZ - 0.01f,1.0f);
    
    glUseProgram(shadersId[UITransfShader]);
    uniformVec2(UITransfShader, "model2D", (vec2){ cursorX, cursorZ });
    
    glBindVertexArray(cursorBuf.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, cursorBuf.VBO);

    glDrawArrays(GL_TRIANGLES, 0, cursorBuf.VBOsize);

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
    
    //    uniformVec2(UITransfColor, "offset", (vec2){ locationCircles[i].mapPos.x, locationCircles[i].mapPos.z });
    uniformVec2(UITransfColor, "offset", (vec2){
	zoomLevel * (offset.x + locationCircles[i].mapPos.x),
	zoomLevel * (offset.z + locationCircles[i].mapPos.z)
      });
    
    uniformVec2(UITransfColor, "model2D", (vec2){ locationCircles[i].scale, locationCircles[i].scale });
    
    glBindVertexArray(circleBuf.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, circleBuf.VBO);

    glDrawArrays(GL_TRIANGLES, 0, circleBuf.VBOsize);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }
  glUseProgram(shadersId[hudShader]);
  
  if(mouseDown){
    glUseProgram(shadersId[UITransfTx]);
    uniformVec2(UITransfTx, "offset", (vec2){ cursorX, cursorZ });
  }
};

void editorMap3dRender(){
    
};

void editorMapPreFrame(float deltaTime){
  mouse.clickL = false;
};

void editorMapMatsSetup(int curShader){

};


void editorMapPreLoop() {
  screenRatio = windowW / windowH;
  
  // setup 2d circle
  {
    float angle = 360.0f / 8.0f;
    int triNum = 8 - 2;

    float r = .025f;

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
  if (event.type == SDL_KEYDOWN) {
    if(event.key.keysym.scancode == SDL_SCANCODE_F) {
      locationCirclesSize++;
	
      if(!locationCircles){
	locationCircles = malloc(sizeof(LocationCircle));
      }else{
	locationCircles = realloc(locationCircles, sizeof(LocationCircle) * locationCirclesSize);
      }

      locationCircles[locationCirclesSize-1].scale = 1.0f;
      //      locationCircles[locationCirclesSize-1].mapPos = (vec2){ translateToGlobalCoords(cursorX, cursorZ) };
      //      locationCircles[locationCirclesSize-1].mapPos = (vec2){ cursorX, cursorZ };

      float trasfCircleX = zoomLevel * (offset.x + cursorX);
      float trasfCircleZ = zoomLevel * (offset.z + cursorZ);
      locationCircles[locationCirclesSize-1].mapPos = (vec2){ trasfCircleX, trasfCircleZ };

    }
  }

  
  if (event.type == SDL_MOUSEWHEEL) {
    if (event.wheel.y > 0) {
      zoomLevel += 0.05f; 
    }else if (event.wheel.y < 0 && zoomLevel > 1.0f) {
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

  if(event.type == SDL_MOUSEMOTION && mouse.leftDown){ 
    float xoffset = event.motion.xrel;
    float yoffset = -event.motion.yrel;

    float dx = xoffset * (0.0001f * 1.5f);
    float dz = yoffset * (0.0001f * 1.5f) * screenRatio;

    printf("trasfY %f \n", zoomLevel * (offset.z+(-scaleY)));
    printf("zoomedY %f \n", zoomLevel * (-scaleY));

    //    if( offset.x + dx)

    float topLimitY = (zoomLevel * ((offset.z+dz)+scaleY));
    float botLimitY = (zoomLevel * ((offset.z+dz)+(-scaleY)));

    printf("dz: %f\n", dz);

    if(topLimitY > 1.0f && botLimitY < -1.0f){
      offset.z += dz;
    }
    
    offset.x += dx;
  }

  if(event.type == SDL_MOUSEBUTTONDOWN){
    mouse.leftDown = event.button.button == SDL_BUTTON_LEFT;
  }

  if(event.type == SDL_MOUSEBUTTONUP){
    mouse.clickL = event.button.button == SDL_BUTTON_LEFT;
    mouse.leftDown = false;
  }
};

void editorMapMouseVS(){

}
