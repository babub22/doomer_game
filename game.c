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

  glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
}

Model* player;


void game2dRender(){
  //  renderText(instancesStr[gameInstance], .0f, .0f, 1.0f);

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
};

void game3dRender(){
    
};

int frameCounter;
vec3 targetPos;

void gamePreFrame(float deltaTime){
  printf("%d %d %d \n", frameCounter == 20, targetPos.x != 0, frameCounter);
  
  if(frameCounter == 20){
    if(targetPos.x != 0){
      //   targetPos.x;
      //    targetPos.y;

      vec3 playerPos = { 
	player->mat.m[12],
	0.0f,
	player->mat.m[14]
      };

      printf("%f %f %f \n", argVec3(playerPos));

      if(playerPos.x > targetPos.x){
	playerPos.x -= 0.1f;
    }else if(playerPos.x < targetPos.x){
      playerPos.x += 0.1f;
    }

      if(playerPos.z > targetPos.z){
	playerPos.z -= 0.1f;
      }else if(playerPos.z < targetPos.z){
	playerPos.z += 0.1f;
      }

      if(playerPos.z == targetPos.z && playerPos.x == targetPos.x){
	targetPos.x = 0;
      }

      player->mat.m[12] = playerPos.x;
      player->mat.m[13] = 0.0f;
      player->mat.m[14] = playerPos.z;

      calculateModelAABB(player);
      batchModels();
    
    }
    
    frameCounter=0;
  }

  
  frameCounter++;
};

void gameMatsSetup(int curShader){
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

    glUseProgram(shadersId[curShader]);

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
      //      cursorMode = moveMode;
      
      mouse.focusedThing = NULL;
      mouse.focusedType = 0;
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
    }
  }
};

void gameMouseVS(){
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
	intersTileData->groundInter = intersection.y <= curCamera->pos.y ? fromOver : fromUnder;

	mouse.selectedType = mouseTileT;
	mouse.selectedThing = intersTileData;

	minDistToCamera = intersectionDistance;
      }
    }

    if (ind.y >= curFloor) {
      // walls
      {
	for(int i2=0;i2<batchedGeometryIndexes[i].wallsSize;i2++){
	  int wallSide = batchedGeometryIndexes[i].wallsIndexes[i2];
	  
	  WallType type = bBlock->wall[wallSide]->type;
	  
	  float intersectionDistance;

	  for (int i3 = 0; i3 < wallsVPairs[type].planesNum; i3++) {
	    bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, bBlock->wall[wallSide]->planes[i3].lb, bBlock->wall[wallSide]->planes[i3].rt, NULL, &intersectionDistance);

	    if (isIntersect && minDistToCamera > intersectionDistance) {
	      intersWallData->side = wallSide;

	      int tx = bBlock->wall[wallSide]->planes[i3].txIndex;

	      intersWallData->txIndex = tx;
	      //intersWallData->tile = bBlock;

	      //intersWallData->type = type;
	      intersWallData->plane = i3;

	      mouse.selectedType = mouseWallT;
	      mouse.selectedThing = intersWallData;
	      
	      minDistToCamera = intersectionDistance;
	    }
	  }
	}
      }

      // joints
      {
	for (int i2 = 0; i2 < batchedGeometryIndexes[i].jointsSize; i2++) {
	  int jointIndex = batchedGeometryIndexes[i].jointsIndexes[i2];
	  float intersectionDistance;
	    
	  for (int i3 = 0; i3 < jointPlaneCounter; i3++) {
	    bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, bBlock->joint[jointIndex]->plane[i3].lb, bBlock->joint[jointIndex]->plane[i3].rt, NULL, &intersectionDistance);

	    if (isIntersect && minDistToCamera > intersectionDistance) {

	      intersWallData->side = jointIndex;
	      intersWallData->txIndex = bBlock->joint[jointIndex]->plane[i3].txIndex;
	      //intersWallData->tile = bBlock;

	     // intersWallData->type = wallJointT;
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
	intersTileData->groundInter = intersection.y <= curCamera->pos.y ? fromOver : fromUnder;

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
