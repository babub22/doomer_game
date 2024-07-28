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

};

void game3dRender(){
  {
    stancilHighlight[mouse.selectedType](); 

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthMaps);

    glActiveTexture(GL_TEXTURE0);
    renderScene(mainShader);
  }

  
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

   // vec3 front  = ((vec3){ view.m[8], view.m[9], view.m[10] });

   // curCamera->Z = normalize3((vec3) { front.x * -1.0f, front.y * 1.0f, front.z * 1.0f });
   // curCamera->X = normalize3(cross3(curCamera->Z, curCamera->up));
   // curCamera->Y = (vec3){ 0,dotf3(curCamera->X, curCamera->Z),0 };

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
  
  if(mouse.clickR
     && selectedCollisionTileIndex != -1 
     && entityStorage[playerEntityT] != NULL){
      float div = 1.0f / 3.0f;

      int h = acceptedCollisionTilesAABB[selectedCollisionTileIndex].lb.y - 0.1f;

      vec2i dist = { acceptedCollisionTilesAABB[selectedCollisionTileIndex].lb.x / div,
		     acceptedCollisionTilesAABB[selectedCollisionTileIndex].lb.z / div };

      vec2i start = { (entityStorage[playerEntityT]->mat.m[12] - div / 2.0f) / div,
		      (entityStorage[playerEntityT]->mat.m[14] - div / 2.0f) / div };

      int pathSize;
      findPath(start, dist, h, &pathSize);
  }
};

void gameMouseVS(){
    if (mouse.selectedType == mouseWallT || mouse.selectedType == mouseTileT){
	free(mouse.selectedThing);
    }

    mouse.selectedThing = NULL;
    mouse.selectedType = 0;
    
    float minDistToCamera = 1000.0f;

    // check geom inter
    WallMouseData* intersWallData = malloc(sizeof(WallMouseData));
    TileMouseData* intersTileData = malloc(sizeof(TileMouseData));

     // walls
    {
	for(int i=0;i<wallsStorageSize;i++){
	    WallType type = wallsStorage[i]->type;

	    if(type >= hiddenDoorT){
		continue;
	    }
      
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
  

  selectedCollisionTileIndex = -1;
  if(mouse.selectedType == mouseTileT){
      for(int i=0;i<collisionLayersSize[acceptedLayerT];i++){
	//  printf("%f ", acceptedCollisionTilesAABB[i].lb.y - 0.1f);
	  
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

void gameRenderCursor(){

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
