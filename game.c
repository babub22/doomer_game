#include "deps.h"
#include "linearAlg.h"
#include "main.h"
#include "game.h"

void game2dRender(){
  renderText(instancesStr[gameInstance], .0f, .0f, 1.0f);
};

void game3dRender(){
    
};

void gamePreFrame(float deltaTime){
    Matrix proj = perspective(rad(fov), windowW / windowH, 0.01f, 1000.0f);

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
      float x = (2.0f * mouse.screenPos.x) / windowW - 1.0f;
      float y = 1.0f - (2.0f * mouse.screenPos.z) / windowH;
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

};
