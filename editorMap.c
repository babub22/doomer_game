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

float zoomLevel = 1.0f;

MeshBuffer worldMapBuf;
MeshBuffer cursorBuf;
int mapTx;

float cursorX;
float cursorZ;

vec2 offset;

bool mouseDown;

void editorMap2dRender(){
  
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
    glUseProgram(shadersId[UITransfShader]);

    uniformVec2(UITransfShader, "model2D", (vec2){ cursorX, cursorZ });
    
    glBindVertexArray(cursorBuf.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, cursorBuf.VBO);

    glDrawArrays(GL_TRIANGLES, 0, cursorBuf.VBOsize);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(shadersId[hudShader]);
  }
  
  if(mouseDown){
    glUseProgram(shadersId[UITransfTx]);
    uniformVec2(UITransfTx, "offset", (vec2){ cursorX, cursorZ });
  }
};

void editorMap3dRender(){
    
};

void editorMapPreFrame(float deltaTime){
  //  mouseDown = false;
};

void editorMapMatsSetup(int curShader){

};


void editorMapPreLoop() {
  //  glUseProgram(shadersId[UITransfShader]);
  //  uniformVec2(UITransfTx, "model2D", (vec2){ zoomLevel, zoomLevel });
  
    mapTx = texture1DIndexByName("fallout2Map");

    float screenRatio = windowW / windowH;
    float imgRatio = (float)loadedTextures1D[mapTx].w / (float)loadedTextures1D[mapTx].h;

    float scaleX, scaleY;

    if (screenRatio > imgRatio) {
        scaleY = 1.0f;
        scaleX = imgRatio / screenRatio;
    }
    else {
        scaleX = 1.0f;
        scaleY = screenRatio / imgRatio;
    }

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
  if (event.type == SDL_MOUSEWHEEL) {
    if (event.wheel.y > 0) {
      zoomLevel += 0.05f; 
    }else if (event.wheel.y < 0 && zoomLevel > 1.0f) {
      zoomLevel -= 0.05f;
    }

    glUseProgram(shadersId[UITransfTx]);
    uniformVec2(UITransfTx, "model2D", (vec2){ zoomLevel, zoomLevel });
  }

  if(event.type == SDL_MOUSEMOTION && mouse.leftDown){
    float xoffset = event.motion.xrel;
    float yoffset = -event.motion.yrel;

    offset.x += xoffset * 0.001f / 2.0f;
    offset.z += yoffset * 0.001f / 2.0f;
  }

  if(event.type == SDL_MOUSEBUTTONDOWN){
    mouse.leftDown = event.button.button == SDL_BUTTON_LEFT;
  }

  if(event.type == SDL_MOUSEBUTTONUP){
    mouse.leftDown = false;
  }
};

void editorMapMouseVS(){

}
