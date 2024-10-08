#include "deps.h"
#include "linearAlg.h"
#include "main.h"
#include "game.h"

float gameFov = 45.0f;
int gameCameraFloor = 1;

#define cameraFloor (bBlockH * ((float)gameCameraFloor+1.0f) + bBlockH)

void gameOnSetInstance(){
  printf("Now game\n");
//  curCamera->pos.y = cameraFloor;

  glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
}

Model* player;


void game2dRender(){
  //  renderText(instancesStr[gameInstance], .0f, .0f, 1.0f);

};

void game3dRender(){
  
};

int frameCounter;
vec3 targetPos;

void gamePreFrame(float deltaTime){
    currentKeyStates = SDL_GetKeyboardState(NULL);
    
    float step = 0.04f;

    if(currentKeyStates[SDL_SCANCODE_LSHIFT]){
      step = 0.08f;
    }
    
    vec2 nextPos = {0};
    bool move = false;
    int preferedAnim = -1;
    

    vec3 forward = entityStorage[playerEntityT][0].dir;
    forward.y = 0.0f;
    forward = normalize3(forward);
    
    vec3 right = normalize3(cross3((vec3) { .0f, 1.0f, .0f }, forward));

    if(currentKeyStates[SDL_SCANCODE_W]){
	nextPos.x += entityStorage[playerEntityT][0].mat.m[12] + forward.x * step;
	nextPos.z += entityStorage[playerEntityT][0].mat.m[14] + forward.z * step;
	
	move = true;
	entityStorage[playerEntityT][0].model->mirrored = false;
	preferedAnim = walkAnim;
    }else if(currentKeyStates[SDL_SCANCODE_S]){
	nextPos.x += entityStorage[playerEntityT][0].mat.m[12] - forward.x * step;
	nextPos.z += entityStorage[playerEntityT][0].mat.m[14] - forward.z * step;
	
	move = true;
	entityStorage[playerEntityT][0].model->mirrored = true;
	preferedAnim = walkAnim;
    }else if(currentKeyStates[SDL_SCANCODE_A]){
	nextPos.x += entityStorage[playerEntityT][0].mat.m[12] + right.x * step;
	nextPos.z += entityStorage[playerEntityT][0].mat.m[14] + right.z * step;

	move = true;
	entityStorage[playerEntityT][0].model->mirrored = false;
	preferedAnim = strafeAnim;
    }else if(currentKeyStates[SDL_SCANCODE_D]){
	nextPos.x += entityStorage[playerEntityT][0].mat.m[12] - right.x * step;
	nextPos.z += entityStorage[playerEntityT][0].mat.m[14] - right.z * step;
	
	move = true;
	entityStorage[playerEntityT][0].model->mirrored = true;
	preferedAnim = strafeAnim;
    }

    bool isMoveAnims = entityStorage[playerEntityT][0].model->curAnim == walkAnim
	|| entityStorage[playerEntityT][0].model->curAnim == strafeAnim;
    
    if(move){
      vec3 pos;
      float entityR = max(entityStorage[0][0].model->data->size[0], entityStorage[0][0].model->data->size[2])/2.0f;
      float entityH = entityStorage[0][0].model->data->size[1];
    
      bool validPos = entityVsMeshes((vec3){ nextPos.x, entityStorage[playerEntityT][0].mat.m[13], nextPos.z }, entityStorage[playerEntityT][0].model->data->size[1]*0.3f, entityR,entityH, &pos);

	if(validPos){
	    entityStorage[playerEntityT][0].mat.m[12] = pos.x;
	    entityStorage[playerEntityT][0].mat.m[13] = pos.y;
	    entityStorage[playerEntityT][0].mat.m[14] = pos.z;
	    bindCylindersAroundEntities();
	}

	    if((!isMoveAnims && entityStorage[playerEntityT][0].model->nextAnim == entityStorage[playerEntityT][0].model->curAnim) || entityStorage[playerEntityT][0].model->curAnim != preferedAnim){
		entityStorage[playerEntityT][0].model->nextAnim = preferedAnim;
		entityStorage[playerEntityT][0].model->action = playAnimInLoopT;
		entityStorage[playerEntityT][0].model->time = 0;
//		printf("Hit\n");
	    }
	}else{
	    if(isMoveAnims && entityStorage[playerEntityT][0].model->nextAnim == entityStorage[playerEntityT][0].model->curAnim){
		entityStorage[playerEntityT][0].model->nextAnim = idleAnim;
		entityStorage[playerEntityT][0].model->action = playAnimInLoopT;
		entityStorage[playerEntityT][0].model->time = 0;
	    }
	}
};

void gameMatsSetup(int curShader){
    Matrix proj = perspective(rad(gameFov), (windowW) / (windowH), 1.0f, 1000.0f);
//    Matrix proj = perspective(rad(gameFov), fakeWinW / fakeWinH, 1.0f, 1000.0f);

    Matrix view;// = IDENTITY_MATRIX;

    {
	vec3 dir = entityStorage[playerEntityT][0].dir;

	float dist = 0.015f;
         
        vec4 pos = { maxZVertex[0],
		     maxZVertex[1],
		     maxZVertex[2] + dist, 1.0f };

	Matrix skinMat = {.m={0}};
	for(int i=0;i<4;i++){
	    if(maxZVertex[12+i]==0.0f) continue;

	    int joint = maxZVertex[8+i];	    
	    skinMat = addMats(skinMat, mulMatNum(entityStorage[playerEntityT][0].model->jointsMats[joint], maxZVertex[12+i]));
	}
	
	pos = mulmatvec4(skinMat, pos);
	pos = mulmatvec4(entityStorage[playerEntityT][0].mat, pos);
	
	glUseProgram(shadersId[mainShader]);
	
	uniformVec3(mainShader, "cameraPos", (vec3){argVec3(pos)});
	uniformVec3(waterShader, "cameraPos", (vec3){argVec3(pos)});
	uniformVec3(animShader, "cameraPos", (vec3){argVec3(pos)});
	uniformVec3(skyboxShader, "cameraPos", (vec3){argVec3(pos)});

	
	pos.y *= -1.0f;
	pos.z *= -1.0f;
	
	vec3 target =  {pos.x + dir.x,
			pos.y + dir.y,
			pos.z + dir.z
	};
	
	view = lookAt((vec3){argVec3(pos)}, target, (vec3) { .0f, 1.0f, .0f });
	
    }
    
/*    vec3 negPos = { -curCamera->pos.x, -curCamera->pos.y, -curCamera->pos.z };

    translate(&view, argVec3(negPos));

    rotateY(&view, rad(-135.0f));
    rotateX(&view, rad(-45.0f)); */     

    for (int i = 0; i < shadersCounter; i++) {
      glUseProgram(shadersId[i]);
      uniformMat4(i, "proj", proj.m);
      uniformMat4(i, "view", view.m);
    }


    glUseProgram(shadersId[curShader]);

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
	float xoffset = event.motion.xrel;
	float yoffset = -event.motion.yrel;

	const float sensitivity = 0.03f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	// calculate yaw, pitch
	{
	    curCamera->yaw += xoffset;
	    curCamera->pitch += yoffset;

	    if (curCamera->pitch > 70.0f)
		curCamera->pitch = 70.0f;
	    if (curCamera->pitch <= -75.0f)
		curCamera->pitch = -75.0f;

	    if (curCamera->yaw > 180.0f)
		curCamera->yaw = -180.0f;
	    if (curCamera->yaw < -180.0f)
		curCamera->yaw = 180.0f;

	    vec3 dir;
	    dir.x = cosf(rad(curCamera->yaw)) * cosf(rad(curCamera->pitch));
	    dir.y = sinf(rad(curCamera->pitch));
	    dir.z = (sinf(rad(curCamera->yaw)) * cosf(rad(curCamera->pitch)));

	    float angle = angle2Vec(
		(vec2){entityStorage[playerEntityT][0].dir.x,
		       entityStorage[playerEntityT][0].dir.z},
		(vec2){dir.x, dir.z});

	    /*entityStorage[playerEntityT][0].model->nextAnim = turnAnim;
	    entityStorage[playerEntityT][0].model->action = playAnimOnceT;
	    entityStorage[playerEntityT][0].model->blendFactor = 0;
	    
	    if(angle>0){
		entityStorage[playerEntityT][0].model->mirrored = false;
	    }else{
		entityStorage[playerEntityT][0].model->mirrored = true;
	    }*/
	    
	    float tempX = entityStorage[playerEntityT][0].mat.m[12];
	    float tempY = entityStorage[playerEntityT][0].mat.m[13];
	    float tempZ = entityStorage[playerEntityT][0].mat.m[14];

	    entityStorage[playerEntityT][0].mat.m[12]=0;
	    entityStorage[playerEntityT][0].mat.m[13]=0;
	    entityStorage[playerEntityT][0].mat.m[14]=0;
	    
	    rotate(&entityStorage[playerEntityT][0].mat, angle, .0f, 1.0f, .0f);
		
	    entityStorage[playerEntityT][0].mat.m[12] = tempX;
	    entityStorage[playerEntityT][0].mat.m[13] = tempY;
	    entityStorage[playerEntityT][0].mat.m[14] = tempZ;
	    
	    entityStorage[playerEntityT][0].dir = dir;

	    bindCylindersAroundEntities();
	}
	    
	if(false && entityStorageSize[playerEntityT] && mouse.selectedType == mouseTileT){
	    TileMouseData* tileData = (TileMouseData*)mouse.selectedThing;
	    
	    vec3 v2 = (vec3){
		    tileData->intersection.x - entityStorage[playerEntityT][0].mat.m[12],
		    tileData->intersection.y - entityStorage[playerEntityT][0].mat.m[13],
		    tileData->intersection.z - entityStorage[playerEntityT][0].mat.m[14]
		};

	    float r = .4f;
	    if((v2.x*v2.x) + (v2.z*v2.z) > r * r){
		v2 = normalize3(v2);
		vec3 v1 = entityStorage[playerEntityT][0].dir;

		float angle = angle2Vec((vec2) { v1.x, v1.z }, (vec2) { v2.x, v2.z });

		float cosTheta = cosf(angle);
		float sinTheta = sinf(angle);

		entityStorage[playerEntityT][0].dir.x = v1.x * cosTheta - v1.z * sinTheta;
		entityStorage[playerEntityT][0].dir.z = v1.x * sinTheta + v1.z * cosTheta;

		float tempX = entityStorage[playerEntityT][0].mat.m[12];
		float tempY = entityStorage[playerEntityT][0].mat.m[13];
		float tempZ = entityStorage[playerEntityT][0].mat.m[14];

		entityStorage[playerEntityT][0].mat.m[12]=0;
		entityStorage[playerEntityT][0].mat.m[13]=0;
		entityStorage[playerEntityT][0].mat.m[14]=0;
	    
		rotate(&entityStorage[playerEntityT][0].mat, angle, .0f, 1.0f, .0f);
		
		entityStorage[playerEntityT][0].mat.m[12] = tempX;
		entityStorage[playerEntityT][0].mat.m[13] = tempY;
		entityStorage[playerEntityT][0].mat.m[14] = tempZ;
	    }
	}
      
	float x = -1.0 + 2.0 * (event.motion.x / windowW);
	float y = -(-1.0 + 2.0 * (event.motion.y / windowH));

	mouse.cursor.x = x;
	mouse.cursor.z = y;

	float cameraSpeed = 0.015f;

	/*
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
    }*/
  }
        
  if (event.type == SDL_MOUSEBUTTONUP) {
    mouse.leftDown = false;
    mouse.rightDown = false;

    mouse.clickL = event.button.button == SDL_BUTTON_LEFT;
    mouse.clickR = event.button.button == SDL_BUTTON_RIGHT;

    /*if (mouse.clickL && mouse.selectedType == mouseTileT) {
      TileMouseData* tileData = (TileMouseData*) mouse.selectedThing;
      
      targetPos.x = tileData->grid.x;
      targetPos.y = 0.0f;
      targetPos.z = tileData->grid.z;

      printf("%f %f %f \n", argVec3(targetPos));
    }*/

    // if click missing foucedThing reset focus
    if(mouse.focusedThing != mouse.selectedThing){
      mouse.focusedThing = NULL;
      mouse.focusedType = 0;
    }
  }


  if(event.type == SDL_KEYDOWN){
//      float step = 0.1f;
      if(event.key.keysym.scancode == SDL_SCANCODE_MINUS){
	  gameFov--;
      }else if(event.key.keysym.scancode == SDL_SCANCODE_EQUALS){
	  gameFov++;
      }else if(event.key.keysym.scancode == SDL_SCANCODE_E){
	  gameCameraFloor--;
	  curCamera->pos.y = cameraFloor;
      }else if(event.key.keysym.scancode == SDL_SCANCODE_F){
	  if(entityStorage[playerEntityT][0].model->curAnim != pickAnim){
	      entityStorage[playerEntityT][0].model->nextAnim = pickAnim;
	      entityStorage[playerEntityT][0].model->action = playAnimOnceT;
	      entityStorage[playerEntityT][0].model->mirrored = false;
	      entityStorage[playerEntityT][0].model->blendFactor = 0;
	      entityStorage[playerEntityT][0].model->time = 0;
	  }
      }else if(event.key.keysym.scancode == SDL_SCANCODE_C){
	  if(entityStorage[playerEntityT][0].model->curAnim != sitAnim){
	      entityStorage[playerEntityT][0].model->nextAnim = sitAnim;
	      entityStorage[playerEntityT][0].model->action = playAnimAndPauseT;
	      
	      entityStorage[playerEntityT][0].model->mirrored = false;
	      entityStorage[playerEntityT][0].model->blendFactor = 0;
	      entityStorage[playerEntityT][0].model->time = 0;
	  }
      }


/*else if(event.key.keysym.scancode == SDL_SCANCODE_Q){
	 gameCameraFloor++;
	 curCamera->pos.y = cameraFloor;
	 }else if(event.key.keysym.scancode == SDL_SCANCODE_F){
      if(mouse.selectedType == mouseModelT){
	Model* model = (Model*)mouse.selectedThing;

	if(loadedModels1D[model->name].type == playerModelT){
	  printf("Dada\n");
	  player = model;
	}
      }else{
	  player = NULL;
      }

      frameCounter=0;
    }*/
  }
};

void gameMouseVS(){
    mouse.selectedThing = NULL;
    mouse.selectedType = 0;
}

void gameRenderCursor(){
    return;
  glBindTexture(GL_TEXTURE_2D, solidColorTx);
  setSolidColorTx(whiteColor, 1.0f);
      
  glBindVertexArray(cursor.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, cursor.VBO);

  float cursorPoint[] = {
    mouse.cursor.x, mouse.cursor.z,            0.0f, 0.0f,
    mouse.cursor.x + cursorW * 0.05f, mouse.cursor.z - cursorH,            0.0f, 0.0f,
    mouse.cursor.x + cursorW, mouse.cursor.z - cursorH + (cursorH * 0.5f),            0.0f, 0.0f,
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
