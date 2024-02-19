#include "deps.h"
#include "main.h"

Object** objsStore;
size_t objsStoreSize;

int main(int argc, char* argv[]) {
  SDL_Init(SDL_INIT_VIDEO);

  char windowTitle[] = game;

  const float windowW = 800.0f;
  const float windowH = 600.0f;
  
  SDL_Window* window = SDL_CreateWindow(windowTitle,
					SDL_WINDOWPOS_CENTERED,
					SDL_WINDOWPOS_CENTERED,
					windowW, windowH,
					SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

  //SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

  SDL_SetRelativeMouseMode(SDL_TRUE);

  
  SDL_GLContext context = SDL_GL_CreateContext(window);
  
  SDL_ShowCursor(SDL_DISABLE);

  glClearColor(0.2f, 0.3f, 0.4f, 1.0f);

  const float blockW =  0.1f;
  const float blockD =  0.1f;
  const float blockH =  0.2f;

  const float doorPad =  blockW / 4;
    
  const float doorW =  blockW - doorPad;

  const float doorFrameH = 0.2f;
  const float doorH =  doorFrameH * 0.85;

  float zoom = 0.0f;
  
  int floor = 0;

  bool testProsp = false;

  float INCREASER = 0;
  
  Mouse mouse = { .h = 0.005f, .w = 0.005f, .brush = 0, .end = {-1,-1}, .start = {-1,-1}  };

  float dist = sqrt(1 / 3.0);
  Camera camera = { .pos = { 1.0f, 1.0f, 1.0f }, .target={ 0.0f, 0.0f, 0.0f }, .pitch = -14.0f, .yaw = -130.0f };
  bool cameraMode = false;
  // set up camera
  {
    camera.dir = normalize((vec3){camera.pos.x - camera.target.x, camera.pos.y - camera.target.y , camera.pos.z - camera.target.z});

    camera.up = (vec3){ 0.0f, 1.0f, 0.0f };
    camera.front = (vec3){ -camera.pos.x, -camera.pos.y, -camera.pos.z };
  }
  
  Tile*** grid = malloc(sizeof(Tile*) * (floor+1));
  
  // init grid
  {
    for(int y=0;y<floor+1;y++){
      grid[y] = malloc(sizeof(Tile*) * (gridH));
    
      for(int z=0;z<gridH;z++){
	grid[y][z] = calloc(gridW,sizeof(Tile));
      
	for(int x=0;x<gridW;x++){
	  grid[y][z][x].center = 1;
	}
      }
    }
  }
  
  const float entityH = 0.17f;
  const float entityW = 0.1f / 2.0f;
  const float entityD = entityH / 6;

  vec3 initPos = { 0.3f + 0.1f/2, 0.0f, 0.3f + 0.1f/2 }; //+ 0.1f/2 - (entityD * 0.75f)/2 };
  
  Entity player = { initPos,initPos, (vec3){initPos.x + entityW, initPos.y + entityH, initPos.z + entityD}, 180.0f, 0, entityW, entityH, entityD };
  
  bool quit = false;

  clock_t lastFrame = clock();
  float deltaTime;

  float cameraSpeed = speed;
  
  while (!quit) {
    uint32_t starttime = GetTickCount();

    clock_t currentFrame = clock();
    deltaTime = (double)(currentFrame - lastFrame ) / CLOCKS_PER_SEC;
    lastFrame = currentFrame;

    cameraSpeed = 0.5f * deltaTime;

    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
	quit = true;
      }

      if(event.type == SDL_MOUSEBUTTONDOWN){
	if(event.button.button == SDL_BUTTON_LEFT){
	  mouse.clickL = true;
	}

	if(event.button.button == SDL_BUTTON_RIGHT){
	  mouse.clickR = true;
	}
      }

      mouse.wheel = event.wheel.y;
      

      if (event.type == SDL_MOUSEMOTION) {
	mouse.screenPos.x = event.motion.x;
	mouse.screenPos.y = event.motion.y;

	if(cameraMode){
	  mouse.screenPos.x = windowW / 2;
	  mouse.screenPos.y = windowH / 2;
	  
	  float xoffset = event.motion.xrel;
	  float yoffset = -event.motion.yrel;

	  const float sensitivity = 0.1f;
	  xoffset *= sensitivity;
	  yoffset *= sensitivity;

	  // calculate yaw, pitch
	  {
	    camera.yaw += xoffset;
	    camera.pitch += yoffset;

	    if(camera.pitch > 89.0f)
	      camera.pitch =  89.0f;
	    if(camera.pitch < -89.0f)
	      camera.pitch = -89.0f;

	    if(camera.yaw > 180.0f)
	      camera.yaw =  -180.0f;
	    if(camera.yaw < -180.0f)
	      camera.yaw = 180.0f;

	    camera.dir.x = cos(rad(camera.yaw)) * cos(rad(camera.pitch));
	    camera.dir.y = sin(rad(camera.pitch));
	    camera.dir.z = sin(rad(camera.yaw)) * cos(rad(camera.pitch));

	    camera.front = normalize(camera.dir);
	  }
	}
      }
    }

    const Uint8* currentKeyStates = SDL_GetKeyboardState( NULL );

    if( currentKeyStates[ SDL_SCANCODE_W ] )
      {
	if(cameraMode){
	  camera.pos.x += cameraSpeed * camera.front.x;
	  camera.pos.y += cameraSpeed * camera.front.y;
	  camera.pos.z += cameraSpeed * camera.front.z;
	}else{
	  float dx = speed * sin(rad(player.angle));  
	  float dz = speed * cos(rad(player.angle)); 

	  //	vec3 colBox = { player.max.x + player.w/2, player.max.y, player.max.z + 0.1f/2 - (player.d * 0.75f)/2 };
	
	  vec2 tile = { (player.max.x + dx)/blockW, (player.max.z+dz)/blockD };

	  bool isIntersect = false;	  

	  if (grid[floor][(int)tile.y][(int)tile.x].walls != 0) {	
	    for(int side=0;side<basicSideCounter;side++){
	      WallType type = (grid[floor][(int)tile.y][(int)tile.x].walls >> (side*8)) & 0xFF;
	      vec3 pos = { (float)(int)tile.x * blockW, 0.0f,  (float)(int)tile.y * blockD };
	  
	      if(type == wallT){
		vec3 rt = {0};
		vec3 lb = {0};

		const float wallD = 0.01f;

		switch(side){
		case(top):{
		  lb = (vec3){pos.x, pos.y, pos.z};
		  rt = (vec3){pos.x + blockW, pos.y + blockH, pos.z + wallD};

		  break;
		}
		case(bot):{
		  lb = (vec3){pos.x, pos.y, pos.z + blockD};
		  rt = (vec3){pos.x + blockW, pos.y + blockH, pos.z + blockD + wallD};

		  break;
		}
		case(left):{
		  lb = (vec3){pos.x, pos.y, pos.z};
		  rt = (vec3){ pos.x + wallD, pos.y + blockH, pos.z + blockD };
  
		  break;
		}
		case(right):{
		  lb = (vec3){ pos.x + blockW, pos.y, pos.z };
		  rt = (vec3){ pos.x + blockW,pos.y + blockH, pos.z + blockD }; 

		  break;
		}
		default: break;
		}
	    
		if(player.min.x + dx <= rt.x &&
		   player.max.x + dx >= lb.x &&
		   player.min.y <= rt.y &&
		   player.max.y >= lb.y &&
		   player.min.z + dz <= rt.z &&
		   player.max.z + dz >= lb.z){
		  isIntersect = true;
		  break;
		}

	      }
	    }
	  }
	
	  if(!isIntersect){
	    player.pos.x += dx;  
	    player.pos.z += dz;
	  }
    
	}
      }
    else if( currentKeyStates[ SDL_SCANCODE_S ] )
      {
	if(cameraMode){
	  camera.pos.x -= cameraSpeed * camera.front.x;
	  camera.pos.y -= cameraSpeed * camera.front.y;
	  camera.pos.z -= cameraSpeed * camera.front.z;
	}else{
	  player.pos.x -= speed * sin(rad(player.angle));  
	  player.pos.z -= speed * cos(rad(player.angle)); 
	}
      }
    else if(currentKeyStates[ SDL_SCANCODE_D ] )
      {
	if(cameraMode){
	  vec3 normFront = normalize(cross(camera.front, camera.up));
	  
	  camera.pos.x += cameraSpeed * normFront.x;
	  camera.pos.y += cameraSpeed * normFront.y;
	  camera.pos.z += cameraSpeed * normFront.z;
	}
	
	
	/*	float strafeAngle = player.angle;

		if(player.side == left || player.side == right || player.side == bot || player.side == top){
		strafeAngle += 90;
		}
	
		player.pos.x += speed * sin(strafeAngle * M_PI / 180);
		player.pos.z -= speed * cos(strafeAngle * M_PI / 180); */
      }
    else if( currentKeyStates[ SDL_SCANCODE_A ] )
      {
	if(cameraMode){
	  vec3 normFront = normalize(cross(camera.front, camera.up));
	  
	  camera.pos.x -= cameraSpeed * normFront.x;
	  camera.pos.y -= cameraSpeed * normFront.y;
	  camera.pos.z -= cameraSpeed * normFront.z;
	}
	
	/*	float strafeAngle = player.angle;

		if(player.side == left || player.side == right || player.side == bot || player.side == top){
		strafeAngle += 90;
		}

		player.pos.x -= speed * sin(strafeAngle * M_PI / 180);
		player.pos.z += speed * cos(strafeAngle * M_PI / 180); */

      }
    else if(currentKeyStates[ SDL_SCANCODE_SPACE ]){
      cameraMode = !cameraMode;
      
      if(cameraMode){
        SDL_SetRelativeMouseMode(SDL_TRUE);
      }else{
	SDL_SetRelativeMouseMode(SDL_FALSE);
      }
    }
    else if(currentKeyStates[ SDL_SCANCODE_0 ]){
      mouse.brush = 0;
    }
    else if(currentKeyStates[ SDL_SCANCODE_1 ]){
      mouse.brush = wallT;
    }
    else if(currentKeyStates[ SDL_SCANCODE_2 ]){
      mouse.brush = doorT;
    }
    else if(currentKeyStates[ SDL_SCANCODE_3 ]){
      mouse.brush = halfWallT;
    }
    else if(currentKeyStates[SDL_SCANCODE_P]){
		testProsp = !testProsp;
    }
    else if(currentKeyStates[SDL_SCANCODE_EQUALS]){
      //		zoom += 0.11f;
            INCREASER+= 0.1f;
    }
    else if(currentKeyStates[ SDL_SCANCODE_MINUS ]){
      //	zoom -= 0.11f;

      INCREASER-= 0.1f;
    }
    else if(currentKeyStates[ SDL_SCANCODE_4 ]){
      // 2.5fh = windowT;
    }
    else if(currentKeyStates[ SDL_SCANCODE_DELETE ]){
      if(mouse.wallSide != -1){
	WallType type = (mouse.selectedTile->walls >> (mouse.wallSide*8)) & 0xFF;
	mouse.selectedTile->walls &= ~(0xFF << (mouse.wallSide * 8));

	if(type == doorT){
	  int newSize = 0;
	    
	  for(int side=basicSideCounter;side!=0;side--){
	    if (((mouse.selectedTile->walls >> ((side-1) * 8)) & 0xFF) == doorT) {
	      newSize = side;
	      break;
	    };
	  }

	  if(newSize==0){
	    free(mouse.selectedTile->wallsData);
	    mouse.selectedTile->wallsData = NULL;
	  } else if(mouse.wallSide+1 > newSize) {
	    mouse.selectedTile->wallsData = realloc(mouse.selectedTile->wallsData, newSize * sizeof(Object*));
	  }
	}
      }
    }

    mouse.selectedTile = NULL;
    mouse.gridIntesec = (vec2){-1,-1};
    mouse.wallSide = -1;
    mouse.tileSide = -1;
    mouse.intersection = (vec3){-1,-1};
	    
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    


    
    float fov = cameraMode ? 30.0f : 20.0f;
    float dist = 5.5f;
    
    
    gluPerspective(fov, 1, 0.1f, 100.0f);

    //        gluPerspective(20.0f, 1, 0.1f, 100.0f);


    if(!cameraMode){
      //    gluLookAt(5.5f + zoom, 5.5f + zoom, 5.5f + zoom,  /* position of camera */
      //	  0.0f, 0.0f , 0.0f,   /* where camera is pointing at */
      //	  0.0f,  1.0f,  0.0f);  /* which direction is up */

      gluLookAt(3, 3, 3,  /* position of camera */
		0.0f, 0.0f , 0.0f,   /* where camera is pointing at */
		0.0f,  1.0f,  0.0f);  /* which direction is up */
	    
    }else{
      gluLookAt(camera.pos.x, camera.pos.y, camera.pos.z,
		camera.pos.x + camera.front.x, camera.pos.y + camera.front.y, camera.pos.z + camera.front.z,
		camera.up.x, camera.up.y, camera.up.z);
    }
    
    glMatrixMode(GL_MODELVIEW);
    
    // axises    
    if(true)
      {
	glBegin(GL_LINES);

	glColor3d(redColor);
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(1.0, 0.0, 0.0);

	glColor3d(greenColor);
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(0.0, 1.0, 0.0);

	glColor3d(blueColor);
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(0.0, 0.0, 1.0);

	glEnd();
      }

    for (int y = 0; y < floor+1; y++) {
      for (int z = 0; z < gridH; z++) {
	for (int x = 0; x < gridW; x++) {
	  vec3 tile = { (float)x / 10, (floor * blockH), (float)z / 10 };

	  if(!grid[y][z][x].center){
	    continue;
	  }

	  // tile things
	  {
	    const vec3 rt = { tile.x+ 0.1f + INCREASER, tile.y, tile.z +  0.1f + INCREASER };
	    const vec3 lb = { tile.x, tile.y, tile.z };

	    vec3 intersection = {0};
	
	    if (rayIntersectsTriangle(mouse.start, mouse.end, lb, rt, &intersection)) {
	      mouse.selectedTile = &grid[y][z][x];
	      mouse.gridIntesec = (vec2){x,z};
	      mouse.intersection = intersection;
	    
	      vec3 relIntersectionToTile = { intersection.x - tile.x, intersection.y - tile.y, intersection.z - tile.z };
	    
	      if(relIntersectionToTile.x < blockW/4){
		mouse.tileSide = left;
	      }else if(relIntersectionToTile.x > (blockW/4)*3){
		mouse.tileSide = right;
	      }else{
		if(relIntersectionToTile.z < blockD/4){
		  mouse.tileSide = top;
		}
		else if(relIntersectionToTile.z > (blockD/4) * 3){
		  mouse.tileSide = bot;
		}else{
		  mouse.tileSide = -1;
		}
	      }

	      switch(mouse.tileSide){
	      case(top):{
		renderTile((vec3){tile.x + blockW/4, tile.y, tile.z}, GL_TRIANGLE_STRIP, blockW/4 *2, blockD/4, darkPurple);

		break;
	      }
	      case(bot):{
		renderTile((vec3){tile.x + blockW/4, tile.y, tile.z + (blockD/4)*3}, GL_TRIANGLE_STRIP, blockW/4 *2, blockD/4, darkPurple);
	      
		break;
	      }
	      case(right):{
		renderTile((vec3){tile.x+(blockW/4) * 3, tile.y, tile.z + (blockD/4)}, GL_TRIANGLE_STRIP, blockW/4, blockD/4 * 2, darkPurple);
	      
		break;
	      }
	      case(left):{
		renderTile((vec3){tile.x, tile.y, tile.z + (blockD/4)},GL_TRIANGLE_STRIP , blockW/4, blockD/4 * 2, darkPurple);

		break;
	      }
	      default: break;
	      }

	      // render brush phantom
	      switch(mouse.brush){
	      case(wallT):{
		renderWall(tile, GL_LINES, blockW, blockD, mouse.tileSide, blockH,redColor);
	    
		break;
	      }
	      case(doorT):{
		vec3 doorPos = {tile.x + doorPad / 2,tile.y,tile.z };
	      
		if(mouse.tileSide == left || mouse.tileSide == right){
		  doorPos.z += doorPad/2;
		  renderWall(doorPos, GL_TRIANGLE_STRIP, blockD, doorW, mouse.tileSide, doorH, blueColor);
		}else{
		  renderWall(doorPos, GL_TRIANGLE_STRIP, doorW, blockD, mouse.tileSide, doorH, blueColor);
		}
	       
		break;
	      }
	      case(windowT):{
		break;
	      }
	      case(halfWallT):{
		renderWall(tile, GL_LINES, blockW, blockD, mouse.tileSide, blockH * 0.4f,redColor);

		break;
	      }
	      default: break;//assert(0 && "Unsuported brush type");
	      }

	      if(mouse.tileSide != -1){
		if(mouse.clickR){
		  // delete if something exist on this place
		  {
		    WallType type = (mouse.selectedTile->walls >> (mouse.tileSide*8)) & 0xFF;

		    if(type){
		      mouse.selectedTile->walls &= ~(0xFF << (mouse.tileSide * 8));

		      if(type == doorT){
			int newSize = 0;
	    
			for(int side=basicSideCounter;side!=0;side--){
			  if (((mouse.selectedTile->walls >> ((side-1) * 8)) & 0xFF) == doorT) {
			    newSize = side;
			    break;
			  };
			}

			if(newSize==0){
			  free(mouse.selectedTile->wallsData);
			  mouse.selectedTile->wallsData = NULL;
			} else if(mouse.tileSide+1 > newSize) {
			  mouse.selectedTile->wallsData = realloc(mouse.selectedTile->wallsData, newSize * sizeof(Object*));
			}
		      }
		    }
		  }
	      

		  if(mouse.brush == doorT){
		    int wallsSize = 0;

		    // to get wallsSize can be optimized
		    // to O(4) -> O(1) by using if's 
		    if(grid[y][z][x].walls !=0){
		      for(int side=basicSideCounter;side!=0;side--){
			if (((grid[y][z][x].walls >> ((side-1) * 8)) & 0xFF) == doorT) {
			  wallsSize = side;
			  break;
			};
		      }
		    }
		  
		    if(mouse.tileSide+1 > wallsSize) {
		      if (!grid[y][z][x].wallsData) {
			grid[y][z][x].wallsData = malloc((mouse.tileSide + 1) * sizeof(Object*));
		      }
		      else {
			grid[y][z][x].wallsData = realloc(grid[y][z][x].wallsData, (mouse.tileSide + 1) * sizeof(Object*));
		      }
		    }
	      	      
		    Object* newDoor = calloc(1,sizeof(Object));
		    newDoor->pos = (vec3){x,0,z};
		    newDoor->type = doorObj;
	    
		    DoorInfo* doorInfo = malloc(sizeof(DoorInfo));
		    doorInfo->opened = false;
		  
		    newDoor->objInfo = doorInfo;
		    grid[y][z][x].wallsData[mouse.tileSide] = newDoor;
		
		    addObjToStore(newDoor);
		  }

		  grid[y][z][x].walls |= (mouse.brush << mouse.tileSide*8);
		}
	      }
	    }
 	  
	    renderTile(tile, GL_LINES, blockW, 0.1f, darkPurple);
	  }

	  // walls rendering
	  if(grid[y][z][x].walls !=0)
	    for(int side=0;side<basicSideCounter;side++){
	      WallType type = (grid[y][z][x].walls >> (side*8)) & 0xFF;

	      switch(type){
	      case(doorT):{
		// should i use doorObj->pos insteadOf tile.x/y/z&
		Object* doorObj = grid[y][z][x].wallsData[side];
		DoorInfo* doorInfo = (DoorInfo*) doorObj->objInfo;

		// lb/rt only to intersection checking
		vec3 doorPos = {0};
		vec3 lb = {0};
		vec3 rt = {0};

		vec3 postRotationPos = { 0 };

		switch(side){
		case(top):{
		  if(doorInfo->opened){
		    lb = (vec3){tile.x + doorPad / 2 + doorW,tile.y,tile.z};
		    rt = (vec3){lb.x, tile.y+doorH, tile.z-doorW};
		  }else{
		    lb = (vec3){tile.x + doorPad / 2,tile.y,tile.z};
		    rt = (vec3){lb.x + doorW, tile.y+doorH, tile.z};
		  }

		  doorPos = (vec3){tile.x + doorPad / 2,tile.y,tile.z};

		  postRotationPos = (vec3){0, 0, 0.1f -doorPad};
		  break;
		}
		case(bot):{
		  if(doorInfo->opened){
		    lb = (vec3){tile.x + doorPad/2,tile.y, tile.z + blockW};
		    rt = (vec3){lb.x, tile.y+doorH, lb.z + doorW};
		  }else{
		    lb = (vec3){tile.x + doorPad/2,tile.y, tile.z + blockW};
		    rt = (vec3){lb.x+doorW, tile.y+doorH, tile.z + blockW};
		  }
	      
		  doorPos = (vec3){tile.x + doorPad / 2,tile.y,tile.z};
	      
		  postRotationPos = (vec3){-0.2f+doorPad, 0, -0.1f};
		  break;
		}
		case(left):{
		  if(doorInfo->opened){
		    lb = (vec3){tile.x, tile.y, tile.z + doorPad/2 + doorW};
		    rt = (vec3){tile.x-doorW, tile.y+doorH, lb.z};
		  }else{
		    lb = (vec3){tile.x, tile.y, tile.z + doorPad/2};
		    rt = (vec3){tile.x, tile.y+doorH, lb.z + doorW};
		  }

		  doorPos = (vec3){tile.x,tile.y,tile.z + doorPad/2};
	      
		  postRotationPos = (vec3){-0.1f+doorPad, 0, -0.1f+doorPad};
		  break;
		}
		case(right):{
		  if(doorInfo->opened){
		    lb = (vec3){tile.x + blockW,tile.y, tile.z + doorPad/2};
		    rt = (vec3){lb.x + doorW, tile.y+doorH, lb.z};
		  }else{
		    lb = (vec3){tile.x + blockW,tile.y, tile.z + doorPad/2};
		    rt = (vec3){tile.x + blockW, tile.y+doorH, lb.z + doorW};
		  }
	      
		  doorPos = (vec3){tile.x,tile.y,tile.z + doorPad/2};
	      
		  postRotationPos = (vec3){-0.1f, 0, 0.1f};
		  break;
		}
		default: break;
		}

		bool isIntersect = rayIntersectsTriangle(mouse.start,mouse.end,lb,rt, NULL);

		if(isIntersect){
		  mouse.wallSide = side;
		  mouse.selectedTile = &grid[y][z][x];
		  mouse.gridIntesec = (vec2){x,z};
	      
		  if(mouse.clickL && doorObj->anim.frames == 0){
		    doorInfo->opened = !doorInfo->opened;
		  }
		}

		GLenum mode = isIntersect ? GL_TRIANGLE_STRIP : GL_LINES;
		
		if(doorInfo->opened){
		  glPushMatrix();
		  glTranslatef(doorPos.x, doorPos.y, doorPos.z);
		  glRotatef(90, 0, 1, 0);
		  glTranslatef(-doorPos.x, -doorPos.y, -doorPos.z);
		  glTranslatef(postRotationPos.x,postRotationPos.y,postRotationPos.z);
		}

		if(side == left || side == right){
		  renderWall(doorPos, mode, blockD, doorW, side, doorH, blueColor);
		}else{
		  renderWall(doorPos, mode, doorW, blockD, side, doorH, blueColor);
		}

		if(doorInfo->opened){
		  glPopMatrix();
		  break;
		}

		break;
	      }
	      case(wallT):{
		vec3 lb = {0};
		vec3 rt = {0};

		switch(side){
		case(top):{
		  lb = tile;
		  rt = (vec3){tile.x+blockW, tile.y+blockH, tile.z};
		  break;
		}
		case(bot):{
		  lb = (vec3){tile.x,tile.y, tile.z + blockD};
		  rt = (vec3){tile.x+blockW, tile.y+blockH, tile.z + blockD};
		  break;
		}
		case(left):{
		  lb = (vec3){tile.x,tile.y, tile.z};
		  rt = (vec3){tile.x, tile.y+blockH, tile.z + blockD};
		  break;
		}
		case(right):{
		  lb = (vec3){tile.x + blockW,tile.y, tile.z};
		  rt = (vec3){tile.x+ blockW, tile.y+blockH, tile.z + blockD};
		  break;
		}
		default: break;
		}

		if(rayIntersectsTriangle(mouse.start,mouse.end,lb,rt, NULL)){
		  mouse.wallSide = side;
		  mouse.selectedTile = &grid[y][z][x];
		  mouse.gridIntesec = (vec2){x,z};
	      
		  renderWall(tile, GL_TRIANGLE_STRIP, blockW, blockD, side, blockH,redColor);
		}else{
		  renderWall(tile, GL_LINES, blockW, blockD, side, blockH,redColor);
		};

		break;
	      }
	      case(halfWallT):{
		vec3 lb = {0};
		vec3 rt = {0};

		switch(side){
		case(top):{
		  lb = tile;
		  rt = (vec3){tile.x+blockW, tile.y+blockH, tile.z};
		  break;
		}
		case(bot):{
		  lb = (vec3){tile.x,tile.y, tile.z + blockD};
		  rt = (vec3){tile.x+blockW, tile.y+blockH, tile.z + blockD};
		  break;
		}
		case(left):{
		  lb = (vec3){tile.x,tile.y, tile.z};
		  rt = (vec3){tile.x, tile.y+blockH, tile.z + blockD};
		  break;
		}
		case(right):{
		  lb = (vec3){tile.x + blockW,tile.y, tile.z};
		  rt = (vec3){tile.x+ blockW, tile.y+blockH, tile.z + blockD};
		  break;
		}
		default: break;
		}

		if(rayIntersectsTriangle(mouse.start,mouse.end,lb,rt, NULL)){
		  mouse.wallSide = side;
		  mouse.selectedTile = &grid[y][z][x];
		  mouse.gridIntesec = (vec2){x,z};
	      
		  renderWall(tile, GL_TRIANGLE_STRIP, blockW, blockD, side, blockH * 0.4f,redColor);
		}else{
		  renderWall(tile, GL_LINES, blockW, blockD, side, blockH * 0.4f,redColor);
		};

		break;
	      }

	      default: break;
	      }
	    }
	}
      }
    }
    // process logic of objs
    for(int i=0;i<objsStoreSize;i++){
      Object* obj = objsStore[i];

      if(obj->anim.frames != 0){
	obj->anim.frames--;
      }
    }
    
    // render player
    {
      if(mouse.intersection.x != -1 && mouse.intersection.z != -1){
	player.angle = atan2(mouse.intersection.x - (player.pos.x + 0.1f/2), mouse.intersection.z - (player.pos.z + 0.1f/2)) * 180 / M_PI;
      }

      const float headH = player.h / 6;
      const float bodyD = player.d * 0.75f;

      Side side = 0;
      bool harder = true;

#define middleAngle 45
	  
      if(player.angle<0){
	float angle = abs(player.angle);
	
	if(angle > 180.0f/2){
	  if(angle <= 120.0f){
	    side = left;
	  }else if(harder && angle > 90 + (90-middleAngle) / 2 && angle <= 90 + (90-middleAngle)/2 + middleAngle){
	    side = topLeft;
	  }else{
	    side = top;
	  }
	}else{
	  if(angle <= 30.0f){
	    side = bot;
	  }else if(harder && angle > 0 + (90-middleAngle) / 2 && angle <=  0 + (90-middleAngle)/2 + middleAngle){
	    side = botLeft;
	  }else{
	    side = left;
	  }
	}
      }else{
	if(player.angle > 180.0f/2){
	  if(player.angle <= 120.0f){
	    side = right;
	  }else if(harder && player.angle > 90 + (90-middleAngle) / 2 && player.angle <=  90 + (90-middleAngle)/2 + middleAngle){
	    side = rightTop;
	  }
	  else{
	    side = top;
	  }
	}else{
	  if(player.angle <= 30.0f){
	    side = bot;
	  }
	  else if(harder &&player.angle >  0 + (90-middleAngle) / 2 && player.angle <=  0 + (90-middleAngle)/2 + middleAngle){
	    side = botRight;
	  }
	  else{
	    side = right;
	  }
	}
      }

      player.side = side;

      float modelview[16];

      glPushMatrix();
      glTranslatef(player.pos.x,0.0f, player.pos.z + (entityD * 0.75f)/2);
      glRotatef(player.angle, 0.0f, 1.0f, 0.0f);
      glTranslatef(-player.pos.x, -player.pos.y, -player.pos.z - (entityD * 0.75f)/2);

      glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
      
      // draw humanoid
      {
	vec3 centrPos = { player.pos.x, player.pos.y, player.pos.z };

	// head
	renderCube((vec3){ player.pos.x - headH/2, centrPos.y + player.h - headH, centrPos.z }, headH, headH, headH, greenColor);

	// body
	renderCube((vec3){ player.pos.x - (player.w/3)/2, centrPos.y, centrPos.z }, player.w/3, player.h - headH, bodyD ,greenColor);

	const float armH = 0.08f;

	// r arm
	renderCube((vec3){ player.pos.x - ( (player.w/3)/2) * 3, player.h + centrPos.y - armH - headH - 0.01f, centrPos.z }, player.w/3, armH , bodyD ,greenColor);

	// l arm
	renderCube((vec3){ player.pos.x +  (player.w/3)/2, player.h + centrPos.y - armH - headH - 0.01f, centrPos.z }, player.w/3, armH, bodyD ,greenColor);

	glPopMatrix();
      }

      // recalculate AABB
      {
	vec3 minPoint = (vec3){player.pos.x - player.w/2, player.pos.y, player.pos.z };
	vec3 maxPoint = (vec3){player.pos.x - player.w/2 + player.w, player.pos.y + player.h, player.pos.z + player.d };

	vec3 vert[8] = { minPoint, {minPoint.x + player.w, minPoint.y, minPoint.z},
			 {minPoint.x, minPoint.y, minPoint.z + player.d},
			 {minPoint.x + player.w, minPoint.y, minPoint.z + player.d}, maxPoint,
			 {maxPoint.x - player.w, maxPoint.y, maxPoint.z},
			 {maxPoint.x, maxPoint.y, maxPoint.z - player.d},
			 {maxPoint.x - player.w, maxPoint.y, maxPoint.z - player.d}
	};

	player.min = vert[0];
	player.max = vert[0];
	
	for(int i=0; i<8;i++){
	  vert[i] = matrixMultPoint(modelview, vert[i]);
	    
	  player.min.x = min(player.min.x, vert[i].x);
	  player.min.y = min(player.min.y, vert[i].y);
	  player.min.z = min(player.min.z, vert[i].z);
	    
	  player.max.x = max(player.max.x, vert[i].x);
	  player.max.y = max(player.max.y, vert[i].y);
	  player.max.z = max(player.max.z, vert[i].z);
	}
	
	glBegin(GL_LINES);

	glVertex3d(vert[0].x,vert[0].y,vert[0].z);
	glVertex3d(vert[1].x,vert[1].y, vert[1].z);

	glVertex3d(vert[1].x,vert[1].y,vert[1].z);
	glVertex3d(vert[2].x,vert[2].y,vert[2].z);

	glVertex3d(vert[0].x,vert[0].y,vert[0].z);
	glVertex3d(vert[2].x,vert[2].y,vert[2].z);
	
	glVertex3d(vert[2].x,vert[2].y,vert[2].z);
	glVertex3d(vert[3].x,vert[3].y,vert[3].z);

	glColor3d(blueColor);
	glVertex3d(player.min.x,player.min.y,player.min.z );
	glVertex3d(player.min.x,player.max.y,player.min.z );

	glColor3d(greenColor);
	glVertex3d(player.max.x,player.min.y,player.max.z );
	glVertex3d(player.max.x,player.max.y,player.max.z );

	glEnd();

      }
    }


        // cursor world projection
    {
      vec3d start = {0};
      vec3d end = {0};

      double matModelView[16], matProjection[16];
      
      int viewport[4]; 
      glGetDoublev( GL_MODELVIEW_MATRIX, matModelView ); 
      glGetDoublev( GL_PROJECTION_MATRIX, matProjection ); 
      glGetIntegerv( GL_VIEWPORT, viewport );

      double winX = (double)mouse.screenPos.x; 
      double winY = viewport[3] - (double)mouse.screenPos.y; 

      gluUnProject(winX, winY, 0, matModelView, matProjection, 
		   viewport, &start.x, &start.y, &start.z);

      gluUnProject(winX, winY,  1, matModelView, matProjection, 
		   viewport, &end.x, &end.y, &end.z);


      mouse.start = vec3dToVec3(start);
      mouse.end = vec3dToVec3(end);

      glBegin(GL_LINES);
      glVertex3d(start.x, start.y, start.z);
      glVertex3d(start.x,start.y+0.2f, start.z);
      glEnd();      
    }
    printf("Start: %f %f %f \n End: %f %f %f \n",mouse.start.x, mouse.start.y, mouse.start.z,mouse.end.x, mouse.end.y, mouse.end.z);
		
    // renderCursor
    {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0, windowW, windowH, 0, -1, 1); // orthographic projection

      glBegin(GL_LINES);

      float relWindowX = mouse.w * windowW;
      float relWindowY = mouse.h * windowH;

      glVertex2f(mouse.screenPos.x - relWindowX, mouse.screenPos.y + relWindowY);
      glVertex2f(mouse.screenPos.x + relWindowX, mouse.screenPos.y - relWindowY);

      glVertex2f(mouse.screenPos.x + relWindowX, mouse.screenPos.y + relWindowY);
      glVertex2f(mouse.screenPos.x - relWindowX, mouse.screenPos.y - relWindowY);

      glEnd();
    }

    
    glFlush();

    mouse.clickL = false;
    mouse.clickR = false;
  
    SDL_GL_SwapWindow(window);

    uint32_t endtime = GetTickCount();
    uint32_t deltatime = endtime - starttime;

    if (!(deltatime > (1000 / FPS))) {
      Sleep((1000 / FPS) - deltatime);
    }
    
    if (deltatime != 0) {
      sprintf(windowTitle, game" FPS: %d", 1000 / deltatime);
      SDL_SetWindowTitle(window, windowTitle);
    }
  }

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

void renderWall(vec3 pos, GLenum mode , float blockW, float blockD, WallType wall, float wallH, float r, float g, float b){
  //  glPushMatrix();
  //glTranslated(0.5, 0.5, 0.5);
  glColor3d(r, g, b);

  glBegin(mode);

  switch(wall){
  case(top):{
    glVertex3d(pos.x, pos.y, pos.z);
    glVertex3d(pos.x, pos.y + wallH, pos.z);

    glVertex3d(pos.x + blockW,pos.y, pos.z);
    glVertex3d(pos.x + blockW,pos.y + wallH, pos.z);

    glVertex3d(pos.x ,pos.y + wallH, pos.z);
    glVertex3d(pos.x + blockW,pos.y + wallH, pos.z);

    glVertex3d(pos.x,pos.y, pos.z);
    glVertex3d(pos.x + blockW ,pos.y +wallH, pos.z);

    break;
  }
  case(bot):{
    glVertex3d(pos.x, pos.y, pos.z + blockD);
    glVertex3d(pos.x, pos.y + wallH, pos.z + blockD);

    glVertex3d(pos.x + blockW,pos.y, pos.z + blockD);
    glVertex3d(pos.x + blockW,pos.y + wallH, pos.z  + blockD);

    glVertex3d(pos.x + blockW,pos.y, pos.z  + blockD);
    glVertex3d(pos.x ,pos.y + wallH, pos.z + blockD);

    glVertex3d(pos.x ,pos.y + wallH, pos.z + blockD);
    glVertex3d(pos.x + blockW,pos.y + wallH, pos.z + blockD);

    break;
  }
  case(left):{
    glVertex3d(pos.x, pos.y + wallH, pos.z + blockD);
    glVertex3d(pos.x,pos.y + wallH, pos.z);
    
    glVertex3d(pos.x, pos.y, pos.z);
    glVertex3d(pos.x,pos.y + wallH, pos.z);


    //    glVertex3d(pos.x,pos.y, pos.z);
    glVertex3d(pos.x ,pos.y + wallH, pos.z+ blockD);
    glVertex3d(pos.x,pos.y, pos.z + blockD);

    glVertex3d(pos.x,pos.y + wallH, pos.z + blockD);
    glVertex3d(pos.x ,pos.y, pos.z);
  
    break;
  }
  case(right):{
    glVertex3d(pos.x + blockW, pos.y, pos.z);
    glVertex3d(pos.x + blockW, pos.y + wallH, pos.z);

    glVertex3d(pos.x + blockW,pos.y, pos.z + blockD);
    glVertex3d(pos.x + blockW,pos.y + wallH, pos.z + blockD);

    glVertex3d(pos.x + blockW,pos.y + wallH, pos.z);
    glVertex3d(pos.x + blockW,pos.y + wallH, pos.z + blockD);

    glVertex3d(pos.x + blockW,pos.y + wallH, pos.z);
    glVertex3d(pos.x + blockW,pos.y, pos.z + blockD);

    break;
  }
  default: break;
  }
  
  glEnd();

  //  glPopMatrix();
}

void renderCube(vec3 pos, float w, float h, float d, float r, float g, float b){
  //  glPushMatrix();
  //glTranslated(0.5, 0.5, 0.5);
  glColor3d(r, g, b);

  glBegin(GL_LINES);

  glVertex3d(pos.x, pos.y, pos.z);
  glVertex3d(pos.x, pos.y + h, pos.z);

  glVertex3d(pos.x ,pos.y, pos.z);
  glVertex3d(pos.x + w,pos.y, pos.z);

  glVertex3d(pos.x ,pos.y, pos.z);
  glVertex3d(pos.x ,pos.y, pos.z+d);

  glVertex3d(pos.x ,pos.y+ h, pos.z);
  glVertex3d(pos.x + w,pos.y+ h, pos.z);

  glVertex3d(pos.x ,pos.y+h, pos.z);
  glVertex3d(pos.x ,pos.y+h, pos.z+d);

  glVertex3d(pos.x + w,pos.y+h, pos.z);
  glVertex3d(pos.x + w,pos.y, pos.z);

  glVertex3d(pos.x ,pos.y+h, pos.z+d);
  glVertex3d(pos.x ,pos.y, pos.z+d);

  glVertex3d(pos.x ,pos.y+h, pos.z+d);
  glVertex3d(pos.x + w,pos.y+h, pos.z+d);

  glVertex3d(pos.x + w,pos.y+h, pos.z);
  glVertex3d(pos.x + w,pos.y+h, pos.z+d);

  glVertex3d(pos.x + w,pos.y+h, pos.z+d);
  glVertex3d(pos.x + w,pos.y, pos.z+d);
  
  glVertex3d(pos.x + w,pos.y+h, pos.z);
  glVertex3d(pos.x + w,pos.y+h, pos.z+d);

  glVertex3d(pos.x ,pos.y, pos.z+d);
  glVertex3d(pos.x + w,pos.y, pos.z+d);
  
  glVertex3d(pos.x + w,pos.y, pos.z);
  glVertex3d(pos.x + w,pos.y, pos.z+d);  

  glEnd();
  
  //glPopMatrix();
}

bool rayIntersectsTriangle(const vec3 start, const vec3 end, const vec3 lb, const vec3 rt, vec3* posOfIntersection) {
  vec3 dirfrac = {0};

  vec3 norm = normalize(end);
  
  dirfrac.x = 1.0f / norm.x;
  dirfrac.y = 1.0f / norm.y;
  dirfrac.z = 1.0f / norm.z;
  // lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
  // r.org is origin of ray

  //  vec3d rt = { point.x+0.1f, point.y, point.z+0.1f  };
  //  vec3d lb = { point.x, point.y, point.z };

  float t1 = (lb.x - start.x)*dirfrac.x;
  float t2 = (rt.x - start.x)*dirfrac.x;
  float t3 = (lb.y - start.y)*dirfrac.y;
  float t4 = (rt.y - start.y)*dirfrac.y;
  float t5 = (lb.z - start.z)*dirfrac.z;
  float t6 = (rt.z - start.z)*dirfrac.z;

  float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
  float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));
  
  bool res = false;
  float t;

  if (tmax < 0)
    {
      // if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
      t = tmax;
      res= false;//false;
    }
  else if (tmin > tmax)
    {
      // if tmin > tmax, ray doesn't intersect AABB
      t = tmax;
      res= false;
    }
  else{
    t = tmin;
    res= true;
  }

  
  if(res){
    if(posOfIntersection){
      vec3 displacement = {norm.x * t, norm.y * t, norm.z * t};

      vec3 intersection = {start.x + displacement.x, start.y + displacement.y, start.z + displacement.z};

      *posOfIntersection =  intersection;
    }
  }

  return res;
}

vec3 normalize(const vec3 vec) {
  float vecLen = sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
  vec3 norm = { 0 };

  if (vecLen != 0.0f) {
    norm.x = vec.x / vecLen;
    norm.y = vec.y / vecLen;
    norm.z = vec.z / vecLen;
  }

  return norm;
}

void renderTile(vec3 pos, GLenum mode, float w, float d, float r, float g, float b){
  glBegin(mode);
  glColor3d(r, g, b);

  if(mode == GL_LINES){
    glVertex3d(pos.x, pos.y, pos.z);
    glVertex3d(pos.x + w, pos.y, pos.z);

    glVertex3d(pos.x + w,pos.y, pos.z);
    glVertex3d(pos.x + w,pos.y, pos.z + d);

    glVertex3d(pos.x + w,pos.y, pos.z+d);
    glVertex3d(pos.x ,pos.y, pos.z +d);

    glVertex3d(pos.x ,pos.y, pos.z+d);
    glVertex3d(pos.x,pos.y, pos.z);
  }else{
    glVertex3d(pos.x, pos.y, pos.z);
    glVertex3d(pos.x + w, pos.y, pos.z);
    glVertex3d(pos.x + w,pos.y, pos.z + d);

    glVertex3d(pos.x ,pos.y, pos.z);
    glVertex3d(pos.x,pos.y, pos.z + d);
    glVertex3d(pos.x + w,pos.y, pos.z+d);
  }

  glEnd();
}

void addObjToStore(Object* obj){
  objsStoreSize++;
  
  if(objsStore == NULL){
    objsStore = malloc(sizeof(Object*));
  }else{
    objsStore = realloc(objsStore, objsStoreSize * sizeof(Object*));
  }

  // to O(1) lookuping
  obj->id = objsStoreSize-1;
  objsStore[objsStoreSize-1] = obj;
}

void addObjToTile(Tile* tile, Object* obj){
  tile->objsSize++;
	    
  if(!tile->objs){
    tile->objs = malloc(sizeof(Object*));
  }else{
    tile->objs = realloc(tile->objs,tile->objsSize * sizeof(Object*));
  }

  tile->objs[tile->objsSize-1] = obj;
}

vec3 matrixMultPoint(const float matrix[16], vec3 point){
  GLfloat p[4] = {point.x, point.y, point.z, 1.0f};
  GLfloat res[4] = {0};
  
  res[0] = matrix[0]*p[0]+matrix[4]*p[1]+matrix[8]*p[2]+matrix[12]*p[3];
  res[1] = matrix[1]*p[0]+matrix[5]*p[1]+matrix[9]*p[2]+matrix[13]*p[3];
  res[2] = matrix[2]*p[0]+matrix[6]*p[1]+matrix[10]*p[2]+matrix[14]*p[3];
  res[3] = matrix[3]*p[0]+matrix[7]*p[1]+matrix[11]*p[2]+matrix[15]*p[3];

  return (vec3){ res[0], res[1], res[2] };
}

vec3 cross(const vec3 v1, const vec3 v2){
  vec3 crossproduct = {0, 0, 0};

  crossproduct.x = v1.y * v2.z - v1.z * v2.y;
  crossproduct.y = v1.x * v2.z - v1.z * v2.x;
  crossproduct.z = v1.x * v2.y - v1.y * v2.x;

  return crossproduct;
}

int dot(const vec3 v1, const vec3 v2){
  int dproduct = 0;

  dproduct += v1.x * v2.x;
  dproduct += v1.y * v2.y;
  dproduct += v1.z * v2.z;

  return dproduct;
}

bool gluInvertMatrix(const double m[16], double invOut[16])
{
  double inv[16], det;
  int i;

  inv[0] = m[5]  * m[10] * m[15] - 
    m[5]  * m[11] * m[14] - 
    m[9]  * m[6]  * m[15] + 
    m[9]  * m[7]  * m[14] +
    m[13] * m[6]  * m[11] - 
    m[13] * m[7]  * m[10];

  inv[4] = -m[4]  * m[10] * m[15] + 
    m[4]  * m[11] * m[14] + 
    m[8]  * m[6]  * m[15] - 
    m[8]  * m[7]  * m[14] - 
    m[12] * m[6]  * m[11] + 
    m[12] * m[7]  * m[10];

  inv[8] = m[4]  * m[9] * m[15] - 
    m[4]  * m[11] * m[13] - 
    m[8]  * m[5] * m[15] + 
    m[8]  * m[7] * m[13] + 
    m[12] * m[5] * m[11] - 
    m[12] * m[7] * m[9];

  inv[12] = -m[4]  * m[9] * m[14] + 
    m[4]  * m[10] * m[13] +
    m[8]  * m[5] * m[14] - 
    m[8]  * m[6] * m[13] - 
    m[12] * m[5] * m[10] + 
    m[12] * m[6] * m[9];

  inv[1] = -m[1]  * m[10] * m[15] + 
    m[1]  * m[11] * m[14] + 
    m[9]  * m[2] * m[15] - 
    m[9]  * m[3] * m[14] - 
    m[13] * m[2] * m[11] + 
    m[13] * m[3] * m[10];

  inv[5] = m[0]  * m[10] * m[15] - 
    m[0]  * m[11] * m[14] - 
    m[8]  * m[2] * m[15] + 
    m[8]  * m[3] * m[14] + 
    m[12] * m[2] * m[11] - 
    m[12] * m[3] * m[10];

  inv[9] = -m[0]  * m[9] * m[15] + 
    m[0]  * m[11] * m[13] + 
    m[8]  * m[1] * m[15] - 
    m[8]  * m[3] * m[13] - 
    m[12] * m[1] * m[11] + 
    m[12] * m[3] * m[9];

  inv[13] = m[0]  * m[9] * m[14] - 
    m[0]  * m[10] * m[13] - 
    m[8]  * m[1] * m[14] + 
    m[8]  * m[2] * m[13] + 
    m[12] * m[1] * m[10] - 
    m[12] * m[2] * m[9];

  inv[2] = m[1]  * m[6] * m[15] - 
    m[1]  * m[7] * m[14] - 
    m[5]  * m[2] * m[15] + 
    m[5]  * m[3] * m[14] + 
    m[13] * m[2] * m[7] - 
    m[13] * m[3] * m[6];

  inv[6] = -m[0]  * m[6] * m[15] + 
    m[0]  * m[7] * m[14] + 
    m[4]  * m[2] * m[15] - 
    m[4]  * m[3] * m[14] - 
    m[12] * m[2] * m[7] + 
    m[12] * m[3] * m[6];

  inv[10] = m[0]  * m[5] * m[15] - 
    m[0]  * m[7] * m[13] - 
    m[4]  * m[1] * m[15] + 
    m[4]  * m[3] * m[13] + 
    m[12] * m[1] * m[7] - 
    m[12] * m[3] * m[5];

  inv[14] = -m[0]  * m[5] * m[14] + 
    m[0]  * m[6] * m[13] + 
    m[4]  * m[1] * m[14] - 
    m[4]  * m[2] * m[13] - 
    m[12] * m[1] * m[6] + 
    m[12] * m[2] * m[5];

  inv[3] = -m[1] * m[6] * m[11] + 
    m[1] * m[7] * m[10] + 
    m[5] * m[2] * m[11] - 
    m[5] * m[3] * m[10] - 
    m[9] * m[2] * m[7] + 
    m[9] * m[3] * m[6];

  inv[7] = m[0] * m[6] * m[11] - 
    m[0] * m[7] * m[10] - 
    m[4] * m[2] * m[11] + 
    m[4] * m[3] * m[10] + 
    m[8] * m[2] * m[7] - 
    m[8] * m[3] * m[6];

  inv[11] = -m[0] * m[5] * m[11] + 
    m[0] * m[7] * m[9] + 
    m[4] * m[1] * m[11] - 
    m[4] * m[3] * m[9] - 
    m[8] * m[1] * m[7] + 
    m[8] * m[3] * m[5];

  inv[15] = m[0] * m[5] * m[10] - 
    m[0] * m[6] * m[9] - 
    m[4] * m[1] * m[10] + 
    m[4] * m[2] * m[9] + 
    m[8] * m[1] * m[6] - 
    m[8] * m[2] * m[5];

  det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

  if (det == 0)
    return false;

  det = 1.0 / det;

  for (i = 0; i < 16; i++)
    invOut[i] = inv[i] * det;

  return true;
}

/*

// render door frame
{
glBegin(GL_LINE_LOOP);
	  
glColor3d(redColor);

// first plank of frame(left)
glVertex3d(tile.x, tile.y, tile.z + blockD);
glVertex3d(tile.x, tile.y + doorH, tile.z + blockD);
glVertex3d(tile.x + doorPad/2, tile.y, tile.z + blockD);

glVertex3d(tile.x, tile.y + doorH, tile.z + blockD);
glVertex3d(tile.x + doorPad/2, tile.y + doorH, tile.z + blockD);
glVertex3d(tile.x + doorPad/2, tile.y, tile.z + blockD);

// top plank of frame
glVertex3d(tile.x, tile.y + doorH, tile.z + blockD);
glVertex3d(tile.x + blockW + doorPad/2, tile.y + doorFrameH, tile.z + blockD);
glVertex3d(tile.x, tile.y + doorFrameH, tile.z + blockD);

glVertex3d(tile.x, tile.y + doorH, tile.z + blockD);
glVertex3d(tile.x + blockW, tile.y + doorFrameH, tile.z + blockD);
glVertex3d(tile.x + blockW, tile.y + doorH, tile.z + blockD);

// second plank of frame (right)
glVertex3d(tile.x + doorW + doorPad, tile.y + doorH, tile.z + blockD);
glVertex3d(tile.x + doorW + doorPad/2, tile.y, tile.z + blockD);
glVertex3d(tile.x + doorW + doorPad /2, tile.y + doorH, tile.z + blockD);

glVertex3d(tile.x + doorW + doorPad/2, tile.y, tile.z + blockD);
glVertex3d(tile.x + doorW + doorPad, tile.y, tile.z + blockD);
glVertex3d(tile.x + doorW + doorPad, tile.y + doorH, tile.z + blockD);
	      
glEnd();
}
*/

