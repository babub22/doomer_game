#include "deps.h"
#include "linearAlg.h"
#include "main.h"
#include "game.h"

float gameFov = 35.0f;
int gameCameraFloor = 1;

#define cameraFloor (bBlockH * ((float)gameCameraFloor+1.0f) + bBlockH)

void gameOnSetInstance(){
  printf("Now game\n");
  curCamera->pos.y = cameraFloor;
}


void game2dRender(){
  renderText(instancesStr[gameInstance], .0f, .0f, 1.0f);

  glActiveTexture(solidColorTx);
  glBindTexture(GL_TEXTURE_2D, solidColorTx);
  setSolidColorTx(whiteColor, 1.0f);
      
  glBindVertexArray(cursor.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, cursor.VBO);
	
  float cursorPoint[] = {
    mouse.cursor.x + cursorW * 0.05f, mouse.cursor.z + cursorH, 0.0f, 0.0f,
    mouse.cursor.x + cursorW, mouse.cursor.z + cursorH/2.0f, 0.0f, 0.0f,
    mouse.cursor.x, mouse.cursor.z + cursorH / 4.0f, 0.0f, 0.0f, 
  };

  glBufferData(GL_ARRAY_BUFFER, sizeof(cursorPoint), cursorPoint, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glDrawArrays(GL_TRIANGLES, 0, 3);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
};

void game3dRender(){
    
};

void gamePreFrame(float deltaTime){
    Matrix proj = perspective(rad(gameFov), windowW / windowH, 0.01f, 1000.0f);

    Matrix view = IDENTITY_MATRIX;
    vec3 negPos = { -curCamera->pos.x, -curCamera->pos.y, -curCamera->pos.z };

    translate(&view, argVec3(negPos));

    rotateY(&view, rad(-135.0f));
    rotateX(&view, rad(-45.0f));      

    for (int i = 0; i < shadersCounter; i++) {
      glUseProgram(shadersId[i]);
      uniformMat4(i, "proj", proj.m);
      uniformMat4(i, "view", view.m);
    }

    vec3 front  = ((vec3){ view.m[8], view.m[9], view.m[10] });

    curCamera->Z = normalize3((vec3) { front.x * -1.0f, front.y * 1.0f, front.z * 1.0f });
    curCamera->X = normalize3(cross3(curCamera->Z, curCamera->up));
    curCamera->Y = (vec3){ 0,dotf3(curCamera->X, curCamera->Z),0 };

    // cursor things
    {
      float x = mouse.cursor.x;//(2.0f * mouse.cursor.x) / windowW - 1.0f;
	float y = mouse.cursor.z;//1.0f - (2.0f * mouse.cursor.z) / windowH;
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
};

void gamePreLoop(){

};

void gameEvents(SDL_Event event){
  if (event.type == SDL_MOUSEMOTION) {
    float x = -1.0 + 2.0 * (event.motion.x / windowW);
    float y = -(-1.0 + 2.0 * (event.motion.y / windowH));

    mouse.cursor.x = x;
    mouse.cursor.z = y;

    float cameraSpeed = 0.015f;

    // UP
    if (mouse.cursor.z >= 1.0f - cursorH) {
      curCamera->pos.x -= cameraSpeed;
      curCamera->pos.z += cameraSpeed;

      mouse.cursor.z -= cursorH;
    }
    // DOWN
    else if (mouse.cursor.z <= -1.0f + cursorH) {
      curCamera->pos.x += cameraSpeed;
      curCamera->pos.z -= cameraSpeed;
      
      mouse.cursor.z += cursorH;
    }

    // RIGHT
    if (mouse.cursor.x >= 1.0f - cursorW) {
      curCamera->pos.x -= cameraSpeed;
      curCamera->pos.z -= cameraSpeed;

      mouse.cursor.x -= cursorW;
    }
    // LEFT
    else if (mouse.cursor.x <= -1.0f + cursorW) {
      curCamera->pos.z += cameraSpeed;
      curCamera->pos.x += cameraSpeed;
      
      mouse.cursor.x += cursorW;
    }
  }

  if(event.type == SDL_KEYDOWN){
    if(event.key.keysym.scancode == SDL_SCANCODE_MINUS){
      gameFov--;
    }else if(event.key.keysym.scancode == SDL_SCANCODE_EQUALS){
      gameFov++;
    }else if(event.key.keysym.scancode == SDL_SCANCODE_E){
      gameCameraFloor--;
      curCamera->pos.y = cameraFloor;
    }else if(event.key.keysym.scancode == SDL_SCANCODE_Q){
      gameCameraFloor++;
      curCamera->pos.y = cameraFloor;
    }
  }
};
