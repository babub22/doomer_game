#include "deps.h"
#include "main.h"

Object** objsStore;
size_t objsStoreSize;
  
int gridX = 30;
int gridY = 15;
int gridZ = 30;

GLuint* mappedTextures;
SDL_Surface** textures;

Camera camera1 = { .pos = { 1.0f, 1.0f, 1.0f }, .target={ 0.0f, 0.0f, 0.0f }, .pitch = -14.0f, .yaw = -130.0f };
Camera camera2 = { .pos = { 1.0f, 1.0f, 1.0f }, .target={ 0.0f, 0.0f, 0.0f }, .pitch = -14.0f, .yaw = -130.0f };

Camera* curCamera = &camera1;

const float wallD = 0.0012f;

const Sizes wallsSizes[wallTypeCounter+1] = { {0}, {bBlockW,bBlockH,bBlockD}, {bBlockW,bBlockH * 0.4f,bBlockD}, {bBlockW,bBlockH,bBlockD}, {bBlockW,bBlockH,bBlockD}};

const float doorH = bBlockH * 0.85f;
const float doorPad =  bBlockW / 4;

const float doorTopPad = bBlockH - bBlockH * 0.85f;

float fieldOfView = 1.4f;

int main(int argc, char* argv[]) {
  SDL_Init(SDL_INIT_VIDEO);

  char windowTitle[] = game;

  const float windowW = 1280.0f;
  const float windowH = 720.0f;
  
  SDL_Window* window = SDL_CreateWindow(windowTitle,
					SDL_WINDOWPOS_CENTERED,
					SDL_WINDOWPOS_CENTERED,
					windowW, windowH,
					SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

  SDL_SetRelativeMouseMode(SDL_TRUE);
  //  SDL_SetRelativeMouseMode(SDL_TRUE);
  SDL_GLContext context = SDL_GL_CreateContext(window);
  
  SDL_ShowCursor(SDL_DISABLE);

  glClearColor(0.2f, 0.3f, 0.4f, 1.0f);

    
  const float doorW =  bBlockW - doorPad;

  const float doorFrameH = 0.2f;

  float zoom = 0.0f;
  
  int floor = 0;
  bool highlighting = 1;
  
  // init opengl
  {
    glEnable(GL_BLEND);
    //glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);  
    glDepthFunc(GL_LEQUAL);
  }
  
  // load textures
  {
    textures = malloc(texturesCounter * sizeof(SDL_Surface*));

    GLuint mappedTextures[texturesCounter];
    glGenTextures(texturesCounter, mappedTextures);
    
    for(int i=0;i<texturesCounter;i++){
      char path[60];
      sprintf(path, texturesFolder"%d.bmp",i);
    
      textures[i] = SDL_LoadBMP(path);

      if (!textures[i]) {
	printf("Loading of texture \"%d.bmp\" failed", i);
	exit(0);
      }

      glBindTexture(GL_TEXTURE_2D, mappedTextures[i]);
      
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
  }
  
  Mouse mouse = { .h = 0.005f, .w = 0.005f, .brush = 0, .end = {-1,-1}, .start = {-1,-1}, .interDist = 1000.0f  };

  float dist = sqrt(1 / 3.0);
  bool cameraMode = true;

  // set up camera
  {
    camera2.dir = normalize((vec3){curCamera->pos.x - curCamera->target.x, curCamera->pos.y - curCamera->target.y , curCamera->pos.z - curCamera->target.z});
    camera1.dir = normalize((vec3){curCamera->pos.x - curCamera->target.x, curCamera->pos.y - curCamera->target.y , curCamera->pos.z - curCamera->target.z});

    camera1.planes = malloc(sizeof(Plane) * 6);
    
    camera1.up = (vec3){ 0.0f, 1.0f, 0.0f };
    camera1.front = (vec3){ -camera1.pos.x, -camera1.pos.y, -camera1.pos.z };

    camera2.up = (vec3){ 0.0f, 1.0f, 0.0f };
    camera2.front = (vec3){ -camera2.pos.x, -camera2.pos.y, -camera2.pos.z };
  }

  float testFOV = editorFOV;
  
  Tile*** grid = NULL;

  float INCREASER = .0f;

  // load or init grid
  {
    FILE *map = fopen("map.doomer","r");

    if(map == NULL){
      grid = malloc(sizeof(Tile**) * (gridY));
  
      {
	for(int y=0;y<gridY;y++){
	  grid[y] = malloc(sizeof(Tile*) * (gridZ));
    
	  for(int z=0;z<gridZ;z++){
	    grid[y][z] = calloc(gridX,sizeof(Tile));
      
	    for(int x=0;x<gridX;x++){
	      setIn(&grid[y][z][x].ground, 0, netTile);
	    }
	  }
	}
      }

      printf("Map not found!\n");
    }else{
      fscanf(map,"%d %d %d \n", &gridY, &gridZ, &gridX);

      grid = malloc(sizeof(Tile**) * (gridY));
      
      for (int y = 0; y < gridY; y++) {
	grid[y] = malloc(sizeof(Tile*) * (gridZ));

	for (int z = 0; z < gridZ; z++) {
	  grid[y][z] = malloc(sizeof(Tile) * (gridX));
	  
	  for (int x = 0; x < gridX; x++) {
	    fscanf(map,"[Wls: %d, WlsTx %d, Grd: %d]",&grid[y][z][x].walls, &grid[y][z][x].wallsTx, &grid[y][z][x].ground);

	    if(grid[y][z][x].ground == 0 || valueIn(grid[y][z][x].ground, 0) == 0){
	      setIn(&grid[y][z][x].ground,0,netTile);
	    }

	    fgetc(map); // read ,
	  }
	}
	fgetc(map); // read \n
      }

      printf("Map loaded! \n");
    }

    fclose(map);
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

      if(event.type == SDL_KEYDOWN){
	switch(event.key.keysym.scancode){
	case(SDL_SCANCODE_UP):{
	  if(mouse.wallType != -1){
	    Texture nextTx = 0;
	      
	    if(mouse.wallTx != texturesCounter - 1){
	      nextTx = mouse.wallTx + 1;
	    }else{
	      nextTx = 0;
	    }

	    setIn(&grid[mouse.wallTile.y][mouse.wallTile.z][mouse.wallTile.x].wallsTx, mouse.wallSide, nextTx);
	  }else if(mouse.groundInter != -1){
	    GroundType type = valueIn(grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, 0);

	    if(type != texturedTile){
	      setIn(&grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, 0, texturedTile);
	      setIn(&grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, mouse.groundInter, 0);
	    }else{
	      Texture curTx = valueIn(grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, mouse.groundInter);
	      
	      if(curTx == texturesCounter-1){
		setIn(&grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, 0, netTile);
	      }else{
		setIn(&grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, mouse.groundInter, curTx+1);
	      }
	    }
	  }

	  break;
	}
	case(SDL_SCANCODE_DOWN):{
	  // TODO: if intersected tile + wall will work only tile changer 
	  if(mouse.wallType != -1){
	    Texture prevTx = 0;
	      
	    if(mouse.wallTx != 0){
	      prevTx = mouse.wallTx - 1;
	    }else{
	      prevTx = texturesCounter - 1;
	    }
	    
	    setIn(&grid[mouse.wallTile.y][mouse.wallTile.z][mouse.wallTile.x].wallsTx, mouse.wallSide, prevTx);
	  }else if(mouse.groundInter != -1){
	    GroundType type = valueIn(grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, 0);

	    if(type != texturedTile){
	      setIn(&grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, 0, texturedTile);
	      setIn(&grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, mouse.groundInter, texturesCounter - 1);
	    }else{
	      Texture curTx = valueIn(grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, mouse.groundInter);
	      
	      if(curTx == 0){
		setIn(&grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, 0, netTile);
	      }else{
		setIn(&grid[floor][mouse.gridIntersect.z][mouse.gridIntersect.x].ground, mouse.groundInter, curTx-1);
	      }
	    }
	  }

	  break;
	}
	case(SDL_SCANCODE_Z):{
		INCREASER += .01f;
	  
	  break;
	}
	case(SDL_SCANCODE_X):{
		INCREASER -= .01f;
	  
	  break;
	}
	case(SDL_SCANCODE_LEFT):{
	  if(mouse.wallType != -1){
	    WallType prevType = 0;
	      
	    if(mouse.wallType != 1){
	      prevType = mouse.wallType - 1;
	    }else{
	      prevType = wallTypeCounter - 1;
	    }

	    Side oppositeSide = 0;
	    vec2i oppositeTile = {0};

	    setIn(&grid[mouse.wallTile.y][mouse.wallTile.z][mouse.wallTile.x].walls, mouse.wallSide, prevType);
	    
	    if (oppositeTileTo((vec2i) { mouse.wallTile.x, mouse.wallTile.z }, mouse.wallSide, &oppositeTile, &oppositeSide)) {
	      setIn(&grid[mouse.wallTile.y][oppositeTile.z][oppositeTile.x].walls, oppositeSide, prevType);
	    }
	  }
	  
	  break;
	}
	case(SDL_SCANCODE_RIGHT):{
	  if(mouse.wallType != -1){
	    WallType nextType = 0;
	      
	    if(mouse.wallType != wallTypeCounter - 1){
	      nextType = mouse.wallType + 1;
	    }else{
	      nextType = 1;
	    }

	    Side oppositeSide = 0;
	    vec2i oppositeTile = {0};

	    setIn(&grid[mouse.wallTile.y][mouse.wallTile.z][mouse.wallTile.x].walls, mouse.wallSide, nextType);
	    
	    if (oppositeTileTo((vec2i) { mouse.wallTile.x, mouse.wallTile.z }, mouse.wallSide, &oppositeTile, &oppositeSide)) {
	      setIn(&grid[mouse.wallTile.y][oppositeTile.z][oppositeTile.x].walls, oppositeSide, nextType);
	    }
	  }

	  break;
	}

	  
	case(SDL_SCANCODE_SPACE):{
	  cameraMode = !cameraMode;
	  //	  curCamera->
	  curCamera = cameraMode ? &camera1 : &camera2;
      
	  break;
	}
	case(SDL_SCANCODE_EQUALS):{
	  if(floor < gridY-1){
	    floor++;
	  }else{
	    floor = 0;
	  }
	  
	  break;
	}
	case( SDL_SCANCODE_MINUS):{
	  if(floor != 0){
	    floor--;
	  }
	  
	  break;
	}
	case( SDL_SCANCODE_L):{
	  testFOV -= 1.1f;
	  break;
	}
	case( SDL_SCANCODE_O):{
	  testFOV += 1.1f;
	  break;
	}
	case(SDL_SCANCODE_M):{
	  fieldOfView += 0.05f;
	    break;
	}
	case(SDL_SCANCODE_N):{
	  fieldOfView -= 0.05f;
	  break;
	}
	case(SDL_SCANCODE_F5):{
	  FILE *map = fopen("map.doomer", "w");
      
	  fprintf(map,"%d %d %d \n", gridY, gridZ, gridX);
      
	  for (int y = 0; y < gridY; y++) {
	    for (int z = 0; z < gridZ; z++) {
	      for (int x = 0; x < gridX; x++) {
		fprintf(map,"[Wls: %d, WlsTx %d, Grd: %d],", grid[y][z][x].walls, grid[y][z][x].wallsTx, grid[y][z][x].ground);
	      }
	    }
	
	    fprintf(map,"\n");
	  }

	  printf("Map saved!\n");
      
	  fclose(map);
	  break;
	}
	case(SDL_SCANCODE_H):{
	  highlighting = !highlighting;
	  break;
	}
	case(SDL_SCANCODE_DELETE):{
	  if(mouse.wallSide != -1){
	    WallType type = (grid[mouse.wallTile.y][mouse.wallTile.z][mouse.wallTile.x].walls >> (mouse.wallSide*8)) & 0xFF;

	    /* if(type == doorT){
	       mouse.selectedTile->walls &= ~(0xFF << (mouse.wallSide * 8));
	      
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
	       }else{*/
	    Side oppositeSide = 0;
	    vec2i oppositeTile = {0};

	    grid[mouse.wallTile.y][mouse.wallTile.z][mouse.wallTile.x].walls &= ~(0xFF << (mouse.wallSide * 8));
	      
	    if(oppositeTileTo((vec2i){mouse.wallTile.x, mouse.wallTile.z}, mouse.wallSide,&oppositeTile,&oppositeSide)){
	      grid[mouse.wallTile.y][oppositeTile.z][oppositeTile.x].walls &= ~(0xFF << (oppositeSide * 8));
	    }
	    //}
	  }

	  break;
	}
	default: break;
	}
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
	mouse.screenPos.z = event.motion.y;

	if(curCamera){//cameraMode){
	  mouse.screenPos.x = windowW / 2;
	  mouse.screenPos.z = windowH / 2;
	  
	  float xoffset = event.motion.xrel;
	  float yoffset = -event.motion.yrel;

	  const float sensitivity = 0.1f;
	  xoffset *= sensitivity;
	  yoffset *= sensitivity;

	  // calculate yaw, pitch
	  {
	    curCamera->yaw += xoffset;
	    curCamera->pitch += yoffset;

	    if(curCamera->pitch > 89.0f)
	      curCamera->pitch =  89.0f;
	    if(curCamera->pitch < -89.0f)
	      curCamera->pitch = -89.0f;

	    if(curCamera->yaw > 180.0f)
	      curCamera->yaw =  -180.0f;
	    if(curCamera->yaw < -180.0f)
	      curCamera->yaw = 180.0f;

	    curCamera->dir.x = cos(rad(curCamera->yaw)) * cos(rad(curCamera->pitch));
	    curCamera->dir.y = sin(rad(curCamera->pitch));
	    curCamera->dir.z = sin(rad(curCamera->yaw)) * cos(rad(curCamera->pitch));

	    curCamera->front = normalize(curCamera->dir);

	   // vec3 right = normalize(cross(camera1.up, camera1.dir));

	    // Calculate up vector using cross product
	    //camera1.up = normalize(cross(right, camera1.dir));

	    //	    float rx = //(float)Math.sin(rad(getYaw()) * -1 * (1-Math.abs((float)Math.cos((double)getPitch() * (double)(Math.PI / 180) - 90 * (Math.PI / 180)) * -1));

	    /*	    float ry = cosf(rad(camera1.pitch) - rad(90.0));
	    camera1.up.y = camera1.up.y * camera1.front.y;
	    printf("UP: %f  Dir: %f \n", camera1.up.y, camera1.dir.y);

		vec3 right = normalize(cross(camera1.front, (vec3) { 0.0f, 1.0f, 0.0f }));
		vec3 up = normalize(cross(right, camera1.front));
		camera1.up = up;

		camera1.up.x = 0.0f;//-sin(rad(camera1.pitch));
		camera1.up.y = cos(rad(camera1.pitch));
		camera1.up.z = 0.0f;
*/	    
			    //	    float rz = (float)Math.cos((double)getYaw() * (double)(Math.PI / 180)) * -1 * (1- Math.abs((float)Math.cos((double)getPitch() * (double)(Math.PI / 180) - 90 * (Math.PI / 180)) * -1));
			    //	    return new Vector3f(rx, ry, rz);

			   // printf("UP Y: %f \n", ry);
			    /*
			      Matrix4 pitch = {
			      {
		{1.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, cosf(rad(curCamera->pitch)), -sinf(rad(curCamera->pitch)), 0.0f},
		{0.0f, sinf(rad(curCamera->pitch)), cosf(rad(curCamera->pitch)), 0.0f},
		{0.0f, 0.0f, 0.0f, 1.0f}
	      }
	    };

	    Matrix4 yaw = {
	      {
		{cosf(rad(curCamera->pitch)), 0.0f, sinf(rad(curCamera->pitch)), 0.0f},
		{0.0f, 1.0f, 0.0f, 0.0f},
		{-sinf(rad(curCamera->pitch)), 0.0f, cosf(rad(curCamera->pitch)), 0.0f},
		{0.0f, 0.0f, 0.0f, 1.0f}
	      }
	    };

	    Matrix4 orientation = Matrix4Multiply(pitch, yaw);

	    vec3 right = Matrix4MultiplyVector(orientation, (vec3){.1f, .0f, .0f});
	    vec3 forward = Matrix4MultiplyVector(orientation, (vec3){.0f, .0f, .1f});

	    Matrix4 position  = {
	      {
		{1, 0, 0, camera1.pos.x},
		{0, 1, 0, camera1.pos.y},
		{0, 0, 1, camera1.pos.z},
		{0, 0, 0, 1}
	      }
	    };
	    
	    Matrix4 cameraWorldPosition = Matrix4Multiply(position, orientation);
	    Matrix4 cameraViewMatrix = {0};*/
	   // gluInvertMatrix(cameraWorldPosition, cameraViewMatrix);

	    //GLfloat newY = camera1.pos.y * cos(rad(camera1.pitch)) - camera1.pos.z * sin(rad(camera1.pitch));
	    //	    GLfloat newZ = camera1.pos.y * sin(rad(camera1.pitch)) + camera1.pos.z * cos(rad(camera1.pitch));
	    //	    camera1.pos.y = newY;
	    //	    camera1.pos.z = newZ;

	    //	     vec3 newRight = normalize(cross(curCamera->dir, curCamera->up));
	     //	     curCamera->up = normalize(cross(newRight, curCamera->dir));
	    
	    //  curCamera->up = cross(normalize(cross(curCamera->front, (vec3){0, -1, 0})), curCamera->front);
	  }
	}
      }
    }


    const Uint8* currentKeyStates = SDL_GetKeyboardState( NULL );

    if( currentKeyStates[ SDL_SCANCODE_W ] )
      {
	if(curCamera){//cameraMode){
	  curCamera->pos.x += cameraSpeed * curCamera->front.x;
	  curCamera->pos.y += cameraSpeed * curCamera->front.y;
	  curCamera->pos.z += cameraSpeed * curCamera->front.z;
	}else{
	  float dx = speed * sin(rad(player.angle));  
	  float dz = speed * cos(rad(player.angle)); 

	  vec2 tile = { (player.max.x + dx)/bBlockW, (player.max.z+dz)/bBlockD };

	  bool isIntersect = false;	  

	  if (grid[floor][(int)tile.z][(int)tile.x].walls != 0) {	
	    for(int side=0;side<basicSideCounter;side++){
	      WallType type = (grid[floor][(int)tile.z][(int)tile.x].walls >> (side*8)) & 0xFF;
	      vec3 pos = { (float)(int)tile.x * bBlockW, 0.0f,  (float)(int)tile.z * bBlockD };
	  
	      if(type == wallT){
		vec3 rt = {0};
		vec3 lb = {0};

		switch(side){
		case(top):{
		  lb = (vec3){pos.x, pos.y, pos.z};
		  rt = (vec3){pos.x + bBlockW, pos.y + bBlockH, pos.z + wallD};

		  break;
		}
		case(bot):{
		  lb = (vec3){pos.x, pos.y, pos.z + bBlockD};
		  rt = (vec3){pos.x + bBlockW, pos.y + bBlockH, pos.z + bBlockD + wallD};

		  break;
		}
		case(left):{
		  lb = (vec3){pos.x, pos.y, pos.z};
		  rt = (vec3){ pos.x + wallD, pos.y + bBlockH, pos.z + bBlockD };
  
		  break;
		}
		case(right):{
		  lb = (vec3){ pos.x + bBlockW, pos.y, pos.z };
		  rt = (vec3){ pos.x + bBlockW,pos.y + bBlockH, pos.z + bBlockD }; 

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
	if(curCamera){//cameraMode){
	  curCamera->pos.x -= cameraSpeed * curCamera->front.x;
	  curCamera->pos.y -= cameraSpeed * curCamera->front.y;
	  curCamera->pos.z -= cameraSpeed * curCamera->front.z;
	}else{
	  player.pos.x -= speed * sin(rad(player.angle));  
	  player.pos.z -= speed * cos(rad(player.angle)); 
	}
      }
    else if(currentKeyStates[ SDL_SCANCODE_D ] )
      {
	if(curCamera){//cameraMode){
	  vec3 normFront = normalize(cross(curCamera->front, curCamera->up));
	  
	  curCamera->pos.x += cameraSpeed * normFront.x;
	  curCamera->pos.y += cameraSpeed * normFront.y;
	  curCamera->pos.z += cameraSpeed * normFront.z;
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
	if(curCamera){//cameraMode){
	  vec3 normFront = normalize(cross(curCamera->front, curCamera->up));
	  
	  curCamera->pos.x -= cameraSpeed * normFront.x;
	  curCamera->pos.y -= cameraSpeed * normFront.y;
	  curCamera->pos.z -= cameraSpeed * normFront.z;
	}
	
	/*	float strafeAngle = player.angle;

		if(player.side == left || player.side == right || player.side == bot || player.side == top){
		strafeAngle += 90;
		}

		player.pos.x -= speed * sin(strafeAngle * M_PI / 180);
		player.pos.z += speed * cos(strafeAngle * M_PI / 180); */

      }
    /*    else if(currentKeyStates[ SDL_SCANCODE_SPACE ]){
	  cameraMode = !cameraMode;
      
	  if(cameraMode){
	  SDL_SetRelativeMouseMode(SDL_TRUE);
	  }else{
	  SDL_SetRelativeMouseMode(SDL_FALSE);
	  }
	  }*/

    // brush settings
    {
      if(currentKeyStates[ SDL_SCANCODE_0 ]){
	mouse.brush = 0;
      }
    
      if(currentKeyStates[ SDL_SCANCODE_1 ]){
	mouse.brush = wallT;
      }
    
      if(currentKeyStates[ SDL_SCANCODE_2 ]){
	mouse.brush = halfWallT;
      }
    
      if(currentKeyStates[ SDL_SCANCODE_3 ]){
        mouse.brush = doorFrameT;
      }

      if(currentKeyStates[ SDL_SCANCODE_4 ]){
        mouse.brush = windowT;
      }
    }

    mouse.selectedTile = NULL;
    mouse.gridIntersect = (vec2i){-1,-1};
    mouse.wallSide = -1;
    mouse.wallTile = (vec3i){-1,-1,-1};
    mouse.wallType = -1;
    mouse.wallTx =   mouse.wallTx = -1;
    mouse.tileSide = -1;
    mouse.intersection = (vec3){-1,-1};
    mouse.groundInter = -1;
	    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    
    float fov = cameraMode ? testFOV : editorFOV;//30.0f;
    float dist = 5.5f;
    
    gluPerspective(fov, windowW/windowH, 0.1f, 100.0f);

    if(!cameraMode){
      //           gluLookAt(3, 3, 3,  /* position of camera */
      //	0.0f, 0.0f , 0.0f,   /* where camera is pointing at */
      //	0.0f,  1.0f,  0.0f);  /* which direction is up */
  
      //      		gluLookAt(3,5, 3,  /* position of camera */
      //      		(gridX / 2) * bBlockW, 0.0f, (gridZ / 2) * bBlockD,   /* where camera is pointing at */
      //      	0.0f,  1.0f,  0.0f);  /* which direction is up */

      gluLookAt(curCamera->pos.x, curCamera->pos.y, curCamera->pos.z,
		curCamera->pos.x + curCamera->front.x, curCamera->pos.y + curCamera->front.y, curCamera->pos.z + curCamera->front.z,
			  curCamera->up.x, curCamera->up.y, curCamera->up.z);

      
    }else{
      gluLookAt(curCamera->pos.x, curCamera->pos.y, curCamera->pos.z,
		curCamera->pos.x + curCamera->front.x, curCamera->pos.y + curCamera->front.y, curCamera->pos.z + curCamera->front.z,
		curCamera->up.x, curCamera->up.y, curCamera->up.z);
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

    // camera frustum
    {
      const float zFar = fieldOfView;
      const float zNear = 0.075f;
      
      #define ANG2RAD 3.14159265358979323846/180.0
      
      const float halfWFar = tanf((ANG2RAD * (editorFOV + 5.0f)) * .5f) * zFar;
      const float halfHFar = halfWFar * (windowW/windowH);
      
      const float halfWNear = tanf((ANG2RAD * (editorFOV - 5.0f)) * .5f) * zNear;
      const float halfHNear = halfWNear * (windowW/windowH);

      vec3 cameraZ = {camera1.front.x * -1.0f, camera1.front.y * -1.0f, camera1.front.z * -1.0f};
      vec3 cameraRight = normalize(cross((vec3) { 0.0f, 1.0f, 0.0f }, cameraZ));
      vec3 cameraUp = normalize(cross(cameraZ, cameraRight));

      vec3 fc = {
	camera1.pos.x - zFar * cameraZ.x,
	camera1.pos.y - zFar * cameraZ.y,
	camera1.pos.z - zFar * cameraZ.z 
      };

      vec3 nc = {
	camera1.pos.x - zNear * cameraZ.x,
	camera1.pos.y - zNear * cameraZ.y,
	camera1.pos.z - zNear * cameraZ.z };

      // vec3 right = normalize(cross(camera1.up, camera1.dir));

      vec3 ftl = {
	fc.x + (cameraUp.x * halfWFar) - (cameraRight.x * halfHFar),
	fc.y + (cameraUp.y * halfWFar) - (cameraRight.y * halfHFar),
	fc.z + (cameraUp.z * halfWFar) - (cameraRight.z * halfHFar)
      };
      
      vec3 ftr = {
	fc.x + (cameraUp.x * halfWFar) + (cameraRight.x * halfHFar),
	fc.y + (cameraUp.y * halfWFar) + (cameraRight.y * halfHFar),
	fc.z + (cameraUp.z * halfWFar) + (cameraRight.z * halfHFar)
      };

      vec3 fbl = {
	fc.x - (cameraUp.x * halfWFar) - (cameraRight.x * halfHFar),
	fc.y - (cameraUp.y * halfWFar) - (cameraRight.y * halfHFar),
	fc.z - (cameraUp.z * halfWFar) - (cameraRight.z * halfHFar)
      };

      vec3 fbr = {
	fc.x - (cameraUp.x * halfWFar) + (cameraRight.x * halfHFar),
	fc.y - (cameraUp.y * halfWFar) + (cameraRight.y * halfHFar),
	fc.z - (cameraUp.z * halfWFar) + (cameraRight.z * halfHFar)
      };
      // near
      vec3 ntl = {
	nc.x + (cameraUp.x * halfWNear) - (cameraRight.x * halfHNear),
	nc.y + (cameraUp.y * halfWNear) - (cameraRight.y * halfHNear),
	nc.z + (cameraUp.z * halfWNear) - (cameraRight.z * halfHNear)
      };
      
      vec3 ntr = {
	nc.x + (cameraUp.x * halfWNear) + (cameraRight.x * halfHNear),
	nc.y + (cameraUp.y * halfWNear) + (cameraRight.y * halfHNear),
	nc.z + (cameraUp.z * halfWNear) + (cameraRight.z * halfHNear)
      };

      vec3 nbl = {
	nc.x - (cameraUp.x * halfWNear) - (cameraRight.x * halfHNear),
	nc.y - (cameraUp.y * halfWNear) - (cameraRight.y * halfHNear),
	nc.z - (cameraUp.z * halfWNear) - (cameraRight.z * halfHNear)
      };

      vec3 nbr = {
	nc.x - (cameraUp.x * halfWNear) + (cameraRight.x * halfHNear),
	nc.y - (cameraUp.y * halfWNear) + (cameraRight.y * halfHNear),
	nc.z - (cameraUp.z * halfWNear) + (cameraRight.z * halfHNear)
      };
      
      /*
      vec3 fc = {
	camera1.pos.x + zFar * camera1.dir.x,
	camera1.pos.y + zFar * camera1.dir.y,
	camera1.pos.z + zFar * camera1.dir.z 
      };

      vec3 nc = {
	camera1.pos.x + zNear * camera1.dir.x,
	camera1.pos.y + zNear * camera1.dir.y,
	camera1.pos.z + zNear * camera1.dir.z };

      vec3 right = normalize(cross(cameraUp, camera1.dir));

cameraRight fbl = {
	fc.x + (cameraUp.x * halfWFar) - (cameraRight.x * halfHFar),
	fc.y + (cameraUp.y * halfWFar) - (cameraRight.y * halfHFar),
	fc.z + (cameraUp.z * halfWFar) - (cameraRight.z * halfHFar)
      };
      
      vec3 fbr = {
	fc.x + (cameraUp.x * halfWFar) + (cameraRight.x * halfHFar),
	fc.y + (cameraUp.y * halfWFar) + (cameraRight.y * halfHFar),
	fc.z + (camera1.up.z * halfWFar) + (right.z * halfHFar)
      };

      vec3 ftl = {
	fc.x - (camera1.up.x * halfWFar) - (right.x * halfHFar),
	fc.y - (camera1.up.y * halfWFar) - (right.y * halfHFar),
	fc.z - (camera1.up.z * halfWFar) - (right.z * halfHFar)
      };

      vec3 ftr = {
	fc.x - (camera1.up.x * halfWFar) + (right.x * halfHFar),
	fc.y - (camera1.up.y * halfWFar) + (right.y * halfHFar),
	fc.z - (camera1.up.z * halfWFar) + (right.z * halfHFar)
      };
      // near
      vec3 nbl = {
	nc.x + (camera1.up.x * halfWNear) - (right.x * halfHNear),
	nc.y + (camera1.up.y * halfWNear) - (right.y * halfHNear),
	nc.z + (camera1.up.z * halfWNear) - (right.z * halfHNear)
      };
      
      vec3 nbr = {
	nc.x + (camera1.up.x * halfWNear) + (right.x * halfHNear),
	nc.y + (camera1.up.y * halfWNear) + (right.y * halfHNear),
	nc.z + (camera1.up.z * halfWNear) + (right.z * halfHNear)
      };

      vec3 ntl = {
	nc.x - (camera1.up.x * halfWNear) - (right.x * halfHNear),
	nc.y - (camera1.up.y * halfWNear) - (right.y * halfHNear),
	nc.z - (camera1.up.z * halfWNear) - (right.z * halfHNear)
      };

      vec3 ntr = {
	nc.x - (camera1.up.x * halfWNear) + (right.x * halfHNear),
	nc.y - (camera1.up.y * halfWNear) + (right.y * halfHNear),
	nc.z - (camera1.up.z * halfWNear) + (right.z * halfHNear)
      };

*/
     //vec3 right = normalize(cross(camera1.up, camera1.dir));

      // Calculate the left point on the near plane (lc)
      vec3 lc = {
	(nbl.x + ntl.x + ftl.x + fbl.x) / 4.0f,
	(nbl.y + ntl.y + ftl.y + fbl.y) / 4.0f,
	(nbl.z + ntl.z + ftl.z + fbl.z) / 4.0f
      };

      // Calculate the right point on the near plane (rc)
      vec3 rc = {
	(nbr.x + ntr.x + ftr.x + fbr.x) / 4.0f,
	(nbr.y + ntr.y + ftr.y + fbr.y) / 4.0f,
	(nbr.z + ntr.z + ftr.z + fbr.z) / 4.0f
      };

      // Calculate the bottom point on the near plane (bc)
      vec3 bc = {
	(nbl.x + nbr.x + fbr.x + fbl.x) / 4.0f,
	(nbl.y + nbr.y + fbr.y + fbl.y) / 4.0f,
	(nbl.z + nbr.z + fbr.z + fbl.z) / 4.0f
      };

      // Calculate the top point on the near plane (tc)
      vec3 tc = {
	(ntl.x + ntr.x + ftr.x + ftl.x) / 4.0f,
	(ntl.y + ntr.y + ftr.y + ftl.y) / 4.0f,
	(ntl.z + ntr.z + ftr.z + ftl.z) / 4.0f
      };
      
      // near - works
      vec3 vec1_near = { ntr.x - ntl.x, ntr.y - ntl.y, ntr.z - ntl.z };
      vec3 vec2_near = { nbl.x - ntl.x, nbl.y - ntl.y, nbl.z - ntl.z };

      vec3 normal_near = normalize(cross(vec2_near, vec1_near));
      float distance_near = -dotf(ntl, normal_near);
	      camera1.planes[nearP] = (Plane){normal_near, distance_near};

      // Far Plane - works
      vec3 vec1_far = { ftr.x - ftl.x, ftr.y - ftl.y, ftr.z - ftl.z };
      vec3 vec2_far = { fbl.x - ftl.x, fbl.y - ftl.y, fbl.z - ftl.z };
      
      vec3 normal_far = normalize(cross(vec1_far, vec2_far));
      
      float distance_far = -dotf(ftl, normal_far);
	      camera1.planes[farP] = (Plane){normal_far, distance_far};

      // Right Plane
      vec3 vec1_right = { fbr.x - ftr.x, fbr.y - ftr.y, fbr.z - ftr.z }; 
      vec3 vec2_right = { ntr.x - ftr.x, ntr.y - ftr.y, ntr.z - ftr.z }; 

      vec3 normal_right = normalize(cross(vec1_right, vec2_right));

      float distance_right = -dotf(ftr, normal_right);
      camera1.planes[rightP] = (Plane){normal_right, distance_right};

      // Left Plane
      vec3 vec1_left = { fbl.x - ftl.x, fbl.y - ftl.y, fbl.z - ftl.z };
      vec3 vec2_left = { ntl.x - ftl.x, ntl.y - ftl.y, ntl.z - ftl.z };

      vec3 normal_left = normalize(cross(vec2_left, vec1_left));

      float distance_left = -dotf(ftl, normal_left);
      camera1.planes[leftP] = (Plane){normal_left, distance_left};

      // Top Plane
      vec3 vec1_top = { ftr.x - ftl.x, ftr.y - ftl.y, ftr.z - ftl.z }; 
      vec3 vec2_top = { ntl.x - ftl.x, ntl.y - ftl.y, ntl.z - ftl.z };
      
      vec3 normal_top = normalize(cross(vec1_top, vec2_top));
      
      float distance_top = dotf(ftl, normal_top);
      camera1.planes[topP] = (Plane){normal_top, distance_top};

      // Bottom Plane
      vec3 vec1_bottom = { nbr.x - fbl.x, nbr.y - fbl.y, nbr.z - fbl.z }; 
      vec3 vec2_bottom = { nbl.x - fbl.x, nbl.y - fbl.y, nbl.z - fbl.z };
      
      vec3 normal_bottom = normalize(cross(vec2_bottom, vec1_bottom));
      
      float distance_bottom = dotf(fbl, normal_bottom);
	//
      camera1.planes[botP] = (Plane){ normal_bottom,  distance_bottom };

      glBegin(GL_LINES);
      
      glColor3d(1.0f, 1.0f, 1.0f);

      glVertex3d(argVec3(lc));
      glVertex3d(lc.x + normal_left.x, lc.y + normal_left.y, lc.z + normal_left.z);

      glVertex3d(argVec3(rc));
      glVertex3d(rc.x + normal_right.x, rc.y + normal_right.y, rc.z + normal_right.z);
      
      glColor3d(cyan);
      
      glVertex3d(argVec3(camera1.pos));
      glVertex3d(argVec3(ntl));

      glVertex3d(argVec3(camera1.pos));
      glVertex3d(argVec3(ntr));
      
      glVertex3d(argVec3(camera1.pos));
      glVertex3d(argVec3(nbl));

      glVertex3d(argVec3(camera1.pos));
      glVertex3d(argVec3(nbr));

      //glVertex3d(argVec3(fc));
      //    glVertex3d(fc.x + normal_far.x, fc.y + normal_far.y, fc.z + normal_far.z);

      //    glVertex3d(argVec3(nc));
      //glVertex3d(nc.x + normal_near.x, nc.y + normal_near.y, nc.z + normal_near.z);


      
      glVertex3d(argVec3(fc));
      glVertex3d(argVec3(fc));
      //      glVertex3d(argVec3(bc));
      //      glVertex3d(bc.x + normal_bottom.x, bc.y + normal_bottom.y, bc.z + normal_bottom.z);

      //      glVertex3d(argVec3(tc));
      //      glVertex3d(tc.x + normal_top.x, tc.y + normal_top.y, tc.z + normal_top.z);


      
      //      glVertex3d(argVec3(tc));
      // glVertex3d(argVec3(normal_bottom));
      
      //      glVertex3d(argVec3(bc));
      //      glVertex3d(argVec3(normal_bottom));
      
      
      //      glVertex3d(argVec3(bc));
      //      glVertex3d(argVec3(tc));
      
      glColor3d(redColor);
      glVertex3d(argVec3(nbr));
      glVertex3d(argVec3(fbr));

      glVertex3d(argVec3(nbl));
      glVertex3d(argVec3(fbl));

      glVertex3d(argVec3(ntr));
      glVertex3d(argVec3(ftr));

      glVertex3d(argVec3(ntl));
      glVertex3d(argVec3(ftl));

      glVertex3d(argVec3(ntl));
      glVertex3d(argVec3(ntr));

      glVertex3d(argVec3(nbl));
      glVertex3d(argVec3(nbr));
      
      glVertex3d(argVec3(ftl));
      glVertex3d(argVec3(ftr));

      glVertex3d(argVec3(fbl));
      glVertex3d(argVec3(fbr));

      glVertex3d(argVec3(nbr));
      glVertex3d(argVec3(fbr));

      glVertex3d(argVec3(nbl));
      glVertex3d(argVec3(ntl));
      
      glVertex3d(argVec3(ftl));
      glVertex3d(argVec3(fbl));
      
      glVertex3d(argVec3(ftr));
      glVertex3d(argVec3(fbr));

      glVertex3d(argVec3(ntr));
      glVertex3d(argVec3(nbr));

      glVertex3d(argVec3(ntr));
      glVertex3d(argVec3(nbr));

      glEnd();
      
      vec3 cubeTile = { .5f ,.5f, .5f };
      
      float dFar = dotf(camera1.planes[farP].normal, cubeTile) + camera1.planes[farP].distance;
      float dNear = dotf(camera1.planes[nearP].normal, cubeTile) + camera1.planes[nearP].distance;

      float dRight = dotf(camera1.planes[rightP].normal, cubeTile) + camera1.planes[rightP].distance;
      float dLeft = dotf(camera1.planes[leftP].normal, cubeTile) + camera1.planes[leftP].distance;

      float dTop = dotf(camera1.planes[topP].normal, cubeTile) + camera1.planes[topP].distance;
      float dBot = dotf(camera1.planes[botP].normal, cubeTile) + camera1.planes[botP].distance;

      printf("[ ");
      
      if(dFar < 0){
	printf("far, ");
      }

      if(dNear < 0){
	printf("near ");
      }

      if(dRight < 0){
	printf("right, ");
      }

      if(dLeft < 0){
	printf("left ");
      }
      
      printf("]\n");
      
      if(dFar > 0 && dNear > 0
	 // && dRight > 0 && dLeft > 0
	 //	 && dTop < 0 && dBot < 0
	 ){
	glColor3d(greenColor);
	printf("Horit!!\n");
	renderCube(cubeTile, .01f, .01f, .01f, greenColor);
      }else{
	//	printf("No fucking way!!\n");
	renderCube(cubeTile, .01f, .01f, .01f, redColor);
      }
    }

    float minIntersectionDist = 1000.0f;

    for (int x = 0; x < gridX; x++) {
      for (int y = 0; y < gridY; y++) {
    	for (int z = 0; z < gridZ; z++) {
	  vec3 tile = { (float)x / 10, (float) y * bBlockH, (float)z / 10 };

	  // walls
	  if(grid[y][z][x].walls !=0){
	    for(int side=0;side<basicSideCounter;side++){
	      bool renderIt = true;
	      
	      WallType type = (grid[y][z][x].walls >> (side*8)) & 0xFF;

	      if(type == 0){
		continue;
	      }
	      
	      vec3* wallPos = wallPosBySide(tile, side, wallsSizes[type].h, wallD, bBlockD, bBlockW);

	      for(int i=0;i<6;i++){
		// wall in/out camera
		int out=0;
		int in=0;

		if(i == botP || i == topP || i == rightP || i == leftP) continue;

		for (int k = 0; k < 4 && (in==0 || out==0); k++) {
		  if (dotf(camera1.planes[i].normal, wallPos[k]) + camera1.planes[i].distance > 0)
		    in++;
		}
	    
		if(!in){
		  renderIt = false;
		  break;
		}
	      }

	      if(!renderIt){
		free(wallPos);
		// next wall side
		continue;
	      }

	      // wall drawing
	      vec3 lb = {0};
	      vec3 rt = {0};
		
	      lb.x = min(wallPos[3].x, wallPos[2].x);
	      lb.y = wallPos[3].y;
	      lb.z = min(wallPos[3].z, wallPos[2].z);

	      rt.x = max(wallPos[0].x, wallPos[1].x);
	      rt.y = wallPos[0].y;
	      rt.z = max(wallPos[0].z, wallPos[1].z);
		  
	      Texture tx = valueIn(grid[y][z][x].wallsTx, side);

	      if(y >= floor){
		float intersectionDistance = 0.0f;
		bool isIntersect = rayIntersectsTriangle(mouse.start,mouse.end,lb,rt, NULL, &intersectionDistance);

		if(isIntersect && minIntersectionDist > intersectionDistance){
		  mouse.wallSide = side;
		  mouse.wallTile = (vec3i){x,y,z};
		  mouse.wallType = type;
		  mouse.wallTx = tx;
		  
		  minIntersectionDist = intersectionDistance;
		}
	      }
		  
	      switch(type){
	      case(windowT):{
		renderWindow(wallPos,tx);
		break;
	      }
	      case(doorFrameT):{
		renderDoorFrame(wallPos,tx);
		break;
	      }
	      default: {
		renderWall(wallPos, tx);
		break;
	      };
	      }
	    }
	  }

	  // tile inter
	  {
	    const vec3 tile = { (float)x / 10, (float) y * bBlockH, (float)z / 10 };

	    const vec3 rt = { tile.x + bBlockW, floor * bBlockH, tile.z + bBlockD };
	    const vec3 lb = { tile.x, floor * bBlockH, tile.z };

	    const vec3 tileCornesrs[4] = { rt, lb, { lb.x, lb.y, lb.z + bBlockD }, { lb.x + bBlockW, lb.y, lb.z } };

	    bool renderIt = true;
	    
	    for(int i=0;i<6;i++){

	      if(i == botP || i == topP || i == rightP || i == leftP) continue;
	      
	      int out=0;
	      int in=0;

	      for (int k = 0; k < 4 && (in==0 || out==0); k++) {
		if (dotf(camera1.planes[i].normal, tileCornesrs[k]) + camera1.planes[i].distance > 0)
		  in++;
	      }
	    
	      if(!in){
		renderIt = false;
		break;
	      }
	    }

	    if(!renderIt){
	      // next iter of grid
	      continue;
	    }

	    float intersectionDistance = 0.0f;
	    vec3 intersection = {0};

	    bool isIntersect = rayIntersectsTriangle(mouse.start,mouse.end,lb,rt, &intersection, &intersectionDistance);
	
	    if(isIntersect && minIntersectionDist > intersectionDistance){
	      mouse.selectedTile = &grid[floor][z][x];
	      mouse.gridIntersect = (vec2i){x,z};
	      mouse.intersection = intersection;
	      mouse.groundInter = intersection.y <= curCamera->pos.y ? fromOver : fromUnder;
	    }
	    
	    GroundType type = valueIn(grid[floor][z][x].ground, 0);

	    if(y == floor && type == netTile){
	      renderTile((vec3){ tile.x, floor * bBlockH, tile.z }, GL_LINES, bBlockW, 0.1f, darkPurple);
	    }
	
	    // tile rendering
	    {
	      GroundType type = valueIn(grid[y][z][x].ground, 0);
	      
	      if(type == texturedTile){
		Texture underTx = valueIn(grid[y][z][x].ground, 1);
		Texture overTx = valueIn(grid[y][z][x].ground, 2);

		glEnable(GL_TEXTURE_2D);
		glColor3f(1.0f, 1.0f, 1.0f);

		int i = y == 0 ? 2 : 1;

		for(;i<=2;i++){
		  Texture tx = valueIn(grid[y][z][x].ground, i);

		  float yPos = tile.y;
		  
		  if(i == fromOver){
		    yPos += wallD;
		  }else if(i== fromUnder){
		    yPos -= wallD;
		  }
		  
		  glBindTexture(GL_TEXTURE_2D, tx+1);
		  glTexImage2D(GL_TEXTURE_2D, 0, 3, textures[tx]->w,
			       textures[tx]->h, 0, GL_RGB,
			       GL_UNSIGNED_BYTE, textures[tx]->pixels);
  
		  glBegin(GL_TRIANGLES);

		  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		  
		  glTexCoord2f(0.0f, 1.0f); glVertex3d(tile.x + wallsSizes[wallT].w, yPos, tile.z + wallsSizes[wallT].d);
		  glTexCoord2f(1.0f, 1.0f); glVertex3d(tile.x, yPos, tile.z + wallsSizes[wallT].d);
		  glTexCoord2f(0.0f, 0.0f); glVertex3d(tile.x + wallsSizes[wallT].w, yPos, tile.z);
  
		  glTexCoord2f(1.0f, 1.0f); glVertex3d(tile.x, yPos, tile.z + wallsSizes[wallT].d);
		  glTexCoord2f(0.0f, 0.0f); glVertex3d(tile.x + wallsSizes[wallT].w, yPos, tile.z);
		  glTexCoord2f(1.0f, 0.0f); glVertex3d(tile.x, yPos, tile.z);

		  glEnd();

		  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
		}
		
		glDisable(GL_TEXTURE_2D);
		
		glEnd();
	      }
	    }
	  }
    }
	        }
	  	}

	  
    //	      }
    
    // render mouse.brush
    if(mouse.selectedTile)
      {
	const float borderArea = bBlockW/8;
      
	const int x = mouse.gridIntersect.x;
	const int z = mouse.gridIntersect.z;

	const vec3 tile = {(float)x * bBlockW, (float)floor * bBlockH, (float)z * bBlockD};

	if(mouse.intersection.x < tile.x + borderArea){
	  mouse.tileSide = left;
	}else if(mouse.intersection.x > tile.x + bBlockW - borderArea){
	  mouse.tileSide = right;
	}else{
	  if(mouse.intersection.z < tile.z +borderArea){
	    mouse.tileSide = top;
	  }
	  else if(mouse.intersection.z > tile.z + bBlockD - borderArea){
	    mouse.tileSide = bot;
	  }else if(mouse.intersection.z > (tile.z + bBlockD/2) - borderArea && mouse.intersection.z < (tile.z + bBlockD/2) + borderArea && mouse.intersection.x >(tile.x + bBlockW/2) - borderArea && mouse.intersection.x < (tile.x +bBlockW/2) + borderArea){
	    mouse.tileSide = center;
	  }else{
	    mouse.tileSide = -1;
	  }
	}

	const float selectionW = borderArea * 3;

	if(mouse.tileSide == center){
	  renderTile((vec3){tile.x + bBlockD/2 - borderArea, tile.y, tile.z + bBlockD/2 - borderArea}, GL_TRIANGLE_STRIP, borderArea*2, borderArea*2, darkPurple);
	}else if(mouse.tileSide == top || mouse.tileSide == bot){
	  float zPos = tile.z;
	      
	  if(mouse.tileSide == top){
	    zPos -= borderArea / 2;
	  }else{
	    zPos += bBlockD - borderArea/2;
	  }
	      
	  renderTile((vec3){tile.x + borderArea + selectionW/2, tile.y, zPos}, GL_TRIANGLE_STRIP, borderArea * 3, borderArea, darkPurple);
	}else if(mouse.tileSide == left || mouse.tileSide == right){
	  float xPos = tile.x;
	      
	  if(mouse.tileSide == right){
	    xPos += bBlockW - borderArea/2;
	  }else{
	    xPos -= borderArea/2;
	  }
	      
	  renderTile((vec3){xPos, tile.y, tile.z + bBlockW/2 - selectionW / 2}, GL_TRIANGLE_STRIP, borderArea, borderArea * 3, darkPurple);
	}

	if(mouse.tileSide != -1 && mouse.tileSide != center){
	    
	  Side oppositeSide = 0;
	  vec2i oppositeTile = {0};

	  vec3* wallPos = wallPosBySide(tile, mouse.tileSide, wallsSizes[mouse.brush].h, wallD, bBlockD, bBlockW);


	  switch(mouse.brush){
	  case(windowT):{
	    renderWindow(wallPos,0);
	    break;
	  }
	  case(doorFrameT):{
	    renderDoorFrame(wallPos,0);
	    break;
	  }
	  default: {
	    renderWall(wallPos, 0);
	    break;
	  };
	  }
	      
	  if(oppositeTileTo((vec2i){x, z}, mouse.tileSide,&oppositeTile,&oppositeSide)){
	    vec3* wallPos = wallPosBySide((vec3){(float)oppositeTile.x / 10, tile.y, (float)oppositeTile.z / 10}, oppositeSide, wallsSizes[mouse.brush].h, wallD, bBlockD, bBlockW);
	    switch(mouse.brush){
	    case(windowT):{
	      renderWindow(wallPos,0);
	      break;
	    }
	    case(doorFrameT):{
	      renderDoorFrame(wallPos,0);
	      break;
	    }
	    default: {
	      renderWall(wallPos, 0);
	      break;
	    };
	    }
	  }


	  if(mouse.clickR){
	    // delete if something exist on this place
	    {
	      WallType type = (mouse.selectedTile->walls >> (mouse.tileSide*8)) & 0xFF;

	      if(type){
		mouse.selectedTile->walls &= ~(0xFF << (mouse.tileSide * 8));
	      }
	    }

	    Side oppositeSide = 0;
	    vec2i oppositeTile = {0};

	    setIn(&grid[floor][z][x].walls, mouse.tileSide, mouse.brush);
	    setIn(&grid[floor][z][x].wallsTx, mouse.tileSide, 0); // first texture
		  
	    if(oppositeTileTo((vec2i){x, z}, mouse.tileSide,&oppositeTile,&oppositeSide)){
	      grid[floor][(int)oppositeTile.z][(int)oppositeTile.x].walls |= (mouse.brush << oppositeSide*8);
	    }
	  }
	}
      }

    
    // higlight intersected wall with min dist
    if(highlighting && mouse.wallSide != -1)
      {
	mouse.interDist = minIntersectionDist;
      
	vec3 pos = { (float)mouse.wallTile.x * bBlockW, (float) mouse.wallTile.y * bBlockH, (float)mouse.wallTile.z * bBlockD };

	if(mouse.wallType == halfWallT){
	  vec3* wallPos = wallPosBySide(pos, mouse.wallSide, bBlockH * 0.4f, wallD, bBlockD, bBlockW);
	  renderWallBorder(wallPos,mouse.wallSide, selBorderT, redColor);
	}
	else {
	  vec3* wallPos = wallPosBySide(pos, mouse.wallSide, bBlockH, wallD, bBlockD, bBlockW);
	  renderWallBorder(wallPos,mouse.wallSide, selBorderT, redColor);
	} 
      }else{
      mouse.interDist = 0.0f;
    }

    //highlight intersected tile
    if(highlighting && mouse.groundInter != -1){
      GroundType type = valueIn(mouse.selectedTile->ground, 0);

      if(type != 0){
	vec3 tile = { (float)mouse.gridIntersect.x * bBlockW, floor * bBlockH, (float) mouse.gridIntersect.z * bBlockD };
	  
	float yPos = tile.y;
		  
	if(mouse.groundInter == fromOver){
	  yPos += wallD;
	}else if(mouse.groundInter == fromUnder){
	  yPos -= wallD;
	}
	
	glColor3d(redColor);
      
	glBegin(GL_TRIANGLES);

	// right
	glVertex3d(tile.x + wallsSizes[wallT].w, yPos, tile.z + wallsSizes[wallT].d - selBorderT);
	glVertex3d(tile.x + wallsSizes[wallT].w, yPos, tile.z + selBorderT);
	glVertex3d(tile.x + wallsSizes[wallT].w - selBorderT, yPos, tile.z + selBorderT);

	glVertex3d(tile.x + wallsSizes[wallT].w, yPos, tile.z + wallsSizes[wallT].d - selBorderT);
	glVertex3d(tile.x + wallsSizes[wallT].w - selBorderT, yPos, tile.z + wallsSizes[wallT].d - selBorderT);
	glVertex3d(tile.x + wallsSizes[wallT].w - selBorderT, yPos, tile.z + selBorderT);

	// left
	glVertex3d(tile.x + selBorderT, yPos, tile.z + wallsSizes[wallT].d - selBorderT);
	glVertex3d(tile.x + selBorderT, yPos, tile.z + selBorderT);
	glVertex3d(tile.x, yPos, tile.z + selBorderT);

	glVertex3d(tile.x + selBorderT, yPos, tile.z + wallsSizes[wallT].d - selBorderT);
	glVertex3d(tile.x, yPos, tile.z + wallsSizes[wallT].d - selBorderT);
	glVertex3d(tile.x, yPos, tile.z + selBorderT);
	
	// bot
	glVertex3d(tile.x + wallsSizes[wallT].w, yPos, tile.z + wallsSizes[wallT].d);
	glVertex3d(tile.x, yPos, tile.z + wallsSizes[wallT].d);
	glVertex3d(tile.x + wallsSizes[wallT].w, yPos, tile.z + wallsSizes[wallT].d - selBorderT);

	glVertex3d(tile.x, yPos, tile.z + wallsSizes[wallT].d);
	glVertex3d(tile.x, yPos, tile.z + wallsSizes[wallT].d - selBorderT);
	glVertex3d(tile.x + wallsSizes[wallT].w, yPos, tile.z + wallsSizes[wallT].d - selBorderT);
	
	// top
	glVertex3d(tile.x + wallsSizes[wallT].w, yPos, tile.z + selBorderT);
	glVertex3d(tile.x, yPos, tile.z + selBorderT);
	glVertex3d(tile.x, yPos, tile.z);

	glVertex3d(tile.x + wallsSizes[wallT].w, yPos, tile.z + selBorderT);
	glVertex3d(tile.x, yPos, tile.z);
	glVertex3d(tile.x + wallsSizes[wallT].w, yPos, tile.z);
	
	glEnd();
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

      float modelview[16];

      glPushMatrix();
      glTranslatef(player.pos.x,0.0f, player.pos.z + (entityD * 0.75f)/2);
      glRotatef(player.angle, 0.0f, 1.0f, 0.0f);
      glTranslatef(-player.pos.x, -player.pos.y, -player.pos.z - (entityD * 0.75f)/2);

      glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
      
      // draw humanoid
      {
	vec3 centrPos = { player.pos.x, player.pos.y + (floor * bBlockH), player.pos.z };

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
      double winY = viewport[3] - (double)mouse.screenPos.z; 

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
    //printf("Start: %f %f %f \n End: %f %f %f \n",mouse.start.x, mouse.start.y, mouse.start.z,mouse.end.x, mouse.end.y, mouse.end.z);
		
    // renderCursor
    {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0, windowW, windowH, 0, -1, 1); // orthographic projection

      glBegin(GL_LINES);

      float relWindowX = mouse.w * windowW;
      float relWindowY = mouse.h * windowH;

      glVertex2f(mouse.screenPos.x - relWindowX, mouse.screenPos.z + relWindowY);
      glVertex2f(mouse.screenPos.x + relWindowX, mouse.screenPos.z - relWindowY);

      glVertex2f(mouse.screenPos.x + relWindowX, mouse.screenPos.z + relWindowY);
      glVertex2f(mouse.screenPos.x - relWindowX, mouse.screenPos.z - relWindowY);

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

void renderWindow(vec3* pos, Texture tx){
  glEnable(GL_TEXTURE_2D);

  glColor3f(1.0f, 1.0f, 1.0f);
  
  glBindTexture(GL_TEXTURE_2D, tx+1);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, textures[tx]->w,
	       textures[tx]->h, 0, GL_RGB,
	       GL_UNSIGNED_BYTE, textures[tx]->pixels);
  
  glBegin(GL_TRIANGLES);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  bool rightOrLeftWall = !(pos[0].x != pos[1].x);

  const float windowBotH = bBlockH * 0.35f;
  
  // top
  glTexCoord2f(0.0f, 1.0f); glVertex3d(pos[0].x, pos[0].y, pos[0].z);
  glTexCoord2f(1.0f, 1.0f); glVertex3d(pos[1].x, pos[1].y, pos[1].z);
  glTexCoord2f(0.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h)); glVertex3d(pos[0].x, pos[0].y - doorTopPad, pos[0].z);

  glTexCoord2f(1.0f, 1.0f);
  glVertex3d(pos[1].x, pos[1].y, pos[1].z);
  glTexCoord2f(1.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h));
  glVertex3d(pos[1].x, pos[1].y - doorTopPad, pos[1].z);
  glTexCoord2f(0.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h));
  glVertex3d(pos[0].x, pos[0].y - doorTopPad, pos[0].z);

  if(rightOrLeftWall){
    // left
    glTexCoord2f(0.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h));
    glVertex3d(pos[0].x, pos[0].y - doorTopPad, pos[0].z);
    
    glTexCoord2f(0.0f + (doorPad/2 / wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h)); glVertex3d(pos[0].x, pos[0].y - doorTopPad, pos[0].z - doorPad/2);

    glTexCoord2f(0.0f, 0.0f + (windowBotH / wallsSizes[doorFrameT].h));
    glVertex3d(pos[3].x, pos[3].y + windowBotH, pos[3].z);

    //
    glTexCoord2f(0.0f + (doorPad/2 / wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h)); glVertex3d(pos[0].x, pos[0].y - doorTopPad, pos[0].z - doorPad/2);

    glTexCoord2f(0.0f + (doorPad/2 / wallsSizes[doorFrameT].w), 0.0f + (windowBotH / wallsSizes[doorFrameT].h));
    glVertex3d(pos[3].x, pos[3].y + windowBotH, pos[3].z - doorPad/2);

    glTexCoord2f(0.0f, 0.0f + (windowBotH / wallsSizes[doorFrameT].h));
    glVertex3d(pos[3].x, pos[3].y + windowBotH, pos[3].z);

    // right
    glTexCoord2f(1.0f - (doorPad/2 / wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h));
    glVertex3d(pos[1].x, pos[1].y - doorTopPad, pos[1].z + doorPad/2);

    glTexCoord2f(1.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h));
    glVertex3d(pos[1].x, pos[1].y - doorTopPad, pos[1].z);
      
    glTexCoord2f(1.0f - (doorPad/2 / wallsSizes[doorFrameT].w) , 0.0f + (windowBotH / wallsSizes[doorFrameT].h));
    glVertex3d(pos[2].x, pos[2].y + windowBotH, pos[2].z + doorPad/2);

    //
    glTexCoord2f(1.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h));
    glVertex3d(pos[1].x, pos[1].y - doorTopPad, pos[1].z);
      
    glTexCoord2f(1.0f, 0.0f + (windowBotH / wallsSizes[doorFrameT].h));
    glVertex3d(pos[2].x, pos[2].y + windowBotH, pos[2].z);
    
    glTexCoord2f(1.0f - (doorPad/2 / wallsSizes[doorFrameT].w) , 0.0f + (windowBotH / wallsSizes[doorFrameT].h));
    glVertex3d(pos[2].x, pos[2].y + windowBotH, pos[2].z + doorPad/2);
  }else{
    // left
    glTexCoord2f(0.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h));
    glVertex3d(pos[0].x, pos[0].y - doorTopPad, pos[0].z);
    
    glTexCoord2f(0.0f + (doorPad/2 / wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h)); glVertex3d(pos[0].x +  doorPad/2, pos[0].y - doorTopPad, pos[0].z);

    glTexCoord2f(0.0f, 0.0f + (windowBotH / wallsSizes[doorFrameT].h));
    glVertex3d(pos[3].x, pos[3].y + windowBotH, pos[3].z);

    //
    glTexCoord2f(0.0f + (doorPad/2 / wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h)); glVertex3d(pos[0].x + doorPad/2, pos[0].y - doorTopPad, pos[0].z);

    glTexCoord2f(0.0f + (doorPad/2 / wallsSizes[doorFrameT].w), 0.0f + (windowBotH / wallsSizes[doorFrameT].h));
    glVertex3d(pos[3].x + doorPad/2, pos[3].y + windowBotH, pos[3].z);

    glTexCoord2f(0.0f, 0.0f + (windowBotH / wallsSizes[doorFrameT].h));
    glVertex3d(pos[3].x, pos[3].y + windowBotH, pos[3].z);

    // right
    glTexCoord2f(1.0f - (doorPad/2 / wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h));
    glVertex3d(pos[1].x - doorPad/2, pos[1].y - doorTopPad, pos[0].z);

    glTexCoord2f(1.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h));
    glVertex3d(pos[1].x, pos[1].y - doorTopPad, pos[0].z);
      
    glTexCoord2f(1.0f - (doorPad/2 / wallsSizes[doorFrameT].w) , 0.0f + (windowBotH / wallsSizes[doorFrameT].h));
    glVertex3d(pos[2].x - doorPad/2, pos[2].y + windowBotH, pos[2].z);

    //
    glTexCoord2f(1.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h));
    glVertex3d(pos[1].x, pos[1].y - doorTopPad, pos[0].z);
      
    glTexCoord2f(1.0f, 0.0f + (windowBotH / wallsSizes[doorFrameT].h));
    glVertex3d(pos[2].x, pos[2].y + windowBotH, pos[2].z);
    
    glTexCoord2f(1.0f - (doorPad/2 / wallsSizes[doorFrameT].w) , 0.0f + (windowBotH / wallsSizes[doorFrameT].h));
    glVertex3d(pos[2].x - doorPad/2, pos[2].y + windowBotH, pos[2].z);
  }

  // bot
  glTexCoord2f(0.0f, 0.0f + (windowBotH / wallsSizes[doorFrameT].h));
  glVertex3d(pos[3].x, pos[3].y + windowBotH, pos[3].z);

  glTexCoord2f(1.0f, 0.0f + (windowBotH / wallsSizes[doorFrameT].h));
  glVertex3d(pos[2].x, pos[2].y + windowBotH, pos[2].z);

  glTexCoord2f(0.0f, 0.0f);
  glVertex3d(pos[3].x, pos[3].y, pos[3].z);
  
  glTexCoord2f(1.0f, 0.0f + (windowBotH / wallsSizes[doorFrameT].h));
  glVertex3d(pos[2].x, pos[2].y + windowBotH, pos[2].z);
  
  glTexCoord2f(1.0f, 0.0f);
  glVertex3d(pos[2].x, pos[2].y, pos[2].z);
  
  glTexCoord2f(0.0f, 0.0f);
  glVertex3d(pos[3].x, pos[3].y, pos[3].z);
  
  glEnd();
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);

  glDisable(GL_TEXTURE_2D);

  free(pos);
}

void renderDoorFrame(vec3* pos, Texture tx){
  glEnable(GL_TEXTURE_2D);

  glColor3f(1.0f, 1.0f, 1.0f);
  
  glBindTexture(GL_TEXTURE_2D, tx+1);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, textures[tx]->w,
	       textures[tx]->h, 0, GL_RGB,
	       GL_UNSIGNED_BYTE, textures[tx]->pixels);
  
  glBegin(GL_TRIANGLES);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);


  bool rightOrLeftWall = !(pos[0].x != pos[1].x);
  
  // top
  glTexCoord2f(0.0f, 1.0f); glVertex3d(pos[0].x, pos[0].y, pos[0].z);
  glTexCoord2f(1.0f, 1.0f); glVertex3d(pos[1].x, pos[1].y, pos[1].z);
  glTexCoord2f(0.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h)); glVertex3d(pos[0].x, pos[0].y - doorTopPad, pos[0].z);

  glTexCoord2f(1.0f, 1.0f);
  glVertex3d(pos[1].x, pos[1].y, pos[1].z);
  glTexCoord2f(1.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h));
  glVertex3d(pos[1].x, pos[1].y - doorTopPad, pos[1].z);
  glTexCoord2f(0.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h));
  glVertex3d(pos[0].x, pos[0].y - doorTopPad, pos[0].z);

  if(rightOrLeftWall){
    // left
    glTexCoord2f(0.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h)); glVertex3d(pos[0].x, pos[0].y - doorTopPad, pos[0].z);

    glTexCoord2f(0.0f + ((doorPad/2)/ wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h));
    glVertex3d(pos[0].x, pos[0].y - doorTopPad, pos[0].z - doorPad/2);

    glTexCoord2f(0.0f, 0.0f); glVertex3d(pos[3].x, pos[3].y, pos[3].z);

    glTexCoord2f(0.0f + ((doorPad/2)/ wallsSizes[doorFrameT].w) , 1.0f - (doorTopPad / wallsSizes[doorFrameT].h) );
    glVertex3d(pos[0].x, pos[0].y - doorTopPad, pos[0].z - doorPad/2);
    
    glTexCoord2f(0.0f + ((doorPad/2)/ wallsSizes[doorFrameT].w) , 0.0f );
    glVertex3d(pos[3].x, pos[3].y, pos[3].z - doorPad/2);
    
    glTexCoord2f(0.0f, 0.0f); glVertex3d(pos[3].x, pos[3].y, pos[3].z);
  
    // right
    glTexCoord2f(1.0f - ((doorPad/2)/ wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h));
    glVertex3d(pos[1].x, pos[1].y - doorTopPad, pos[1].z + doorPad/2);

    glTexCoord2f(1.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h));
    glVertex3d(pos[1].x, pos[1].y - doorTopPad, pos[1].z);
    
    glTexCoord2f(1.0f, 0.0f); glVertex3d(pos[2].x, pos[2].y, pos[2].z);

    glTexCoord2f(1.0f - ((doorPad/2)/ wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h));
    glVertex3d(pos[1].x, pos[1].y - doorTopPad, pos[1].z + doorPad/2);
    
    glTexCoord2f(1.0f, 0.0f); glVertex3d(pos[2].x, pos[2].y, pos[2].z);

    glTexCoord2f(1.0f - ((doorPad/2)/ wallsSizes[doorFrameT].w), 0.0f);
    glVertex3d(pos[2].x, pos[2].y, pos[2].z + doorPad/2);
  }else{
    glTexCoord2f(0.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h)); glVertex3d(pos[0].x, pos[0].y - doorTopPad, pos[0].z);
    glTexCoord2f(0.0f + ((doorPad/2)/ wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h)); glVertex3d(pos[0].x + doorPad/2, pos[0].y - doorTopPad, pos[0].z);
    glTexCoord2f(0.0f, 0.0f); glVertex3d(pos[3].x, pos[3].y, pos[3].z);

    glTexCoord2f(0.0f + ((doorPad/2)/ wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h)); glVertex3d(pos[0].x + doorPad/2, pos[0].y - doorTopPad, pos[0].z);

    glTexCoord2f(0.0f + ((doorPad/2)/ wallsSizes[doorFrameT].w), 0.0f - (doorTopPad / wallsSizes[doorFrameT].h)); glVertex3d(pos[3].x + doorPad/2, pos[3].y, pos[3].z);
    glTexCoord2f(0.0f, 0.0f); glVertex3d(pos[3].x, pos[3].y, pos[3].z);
  
    // right
    glTexCoord2f(1.0f - ((doorPad/2)/ wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h));
    glVertex3d(pos[1].x - doorPad/2, pos[1].y - doorTopPad, pos[1].z);

    glTexCoord2f(1.0f, 1.0f - (doorTopPad / wallsSizes[doorFrameT].h));
    glVertex3d(pos[1].x, pos[1].y - doorTopPad, pos[1].z);

    glTexCoord2f(1.0f, 0.0f); glVertex3d(pos[2].x, pos[2].y, pos[2].z);

    glTexCoord2f(1.0f - ((doorPad/2)/ wallsSizes[doorFrameT].w), 1.0f - (doorTopPad / wallsSizes[doorFrameT].h));
    glVertex3d(pos[1].x - doorPad/2, pos[1].y - doorTopPad, pos[1].z);
    glTexCoord2f(1.0f, 0.0f); glVertex3d(pos[2].x, pos[2].y, pos[2].z);

    glTexCoord2f(1.0f - ((doorPad/2)/ wallsSizes[doorFrameT].w), 0.0f);
    glVertex3d(pos[2].x - doorPad/2, pos[2].y, pos[2].z);
  }
  
  glEnd();
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);

  glDisable(GL_TEXTURE_2D);

  free(pos);
}

void renderWall(vec3* pos, Texture tx){
  glEnable(GL_TEXTURE_2D);

  glColor3f(1.0f, 1.0f, 1.0f);
  
  glBindTexture(GL_TEXTURE_2D, tx+1);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, textures[tx]->w,
	       textures[tx]->h, 0, GL_RGB,
	       GL_UNSIGNED_BYTE, textures[tx]->pixels);
  
  glBegin(GL_TRIANGLES);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  glTexCoord2f(0.0f, 1.0f); glVertex3d(pos[0].x, pos[0].y, pos[0].z);
  glTexCoord2f(1.0f, 1.0f); glVertex3d(pos[1].x, pos[1].y, pos[1].z);
  glTexCoord2f(0.0f, 0.0f); glVertex3d(pos[3].x, pos[3].y, pos[3].z);

  glTexCoord2f(1.0f, 1.0f); glVertex3d(pos[1].x, pos[1].y, pos[1].z);
  glTexCoord2f(1.0f, 0.0f); glVertex3d(pos[2].x, pos[2].y, pos[2].z);
  glTexCoord2f(0.0f, 0.0f); glVertex3d(pos[3].x, pos[3].y, pos[3].z);  
  
  glEnd();
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);

  glDisable(GL_TEXTURE_2D);

  free(pos);
}

void renderWallBorder(vec3* pos, Side side, float borderT, float r, float g, float b){
  glColor3f(r,g,b);
  
  glBegin(GL_TRIANGLES);

  switch(side){
  case(bot):{
    pos[0].z -= selBorderD;
    pos[1].z -= selBorderD;
    pos[2].z -= selBorderD;
    pos[3].z -= selBorderD;
    break;
  }
  case(top):{
    pos[0].z += selBorderD;
    pos[1].z += selBorderD;
    pos[2].z += selBorderD;
    pos[3].z += selBorderD;
    break;
  }
  case(left):{
    pos[0].x += selBorderD;
    pos[1].x += selBorderD;
    pos[2].x += selBorderD;
    pos[3].x += selBorderD;

    break;
  }
  case(right):{
    pos[0].x -= selBorderD;
    pos[1].x -= selBorderD;
    pos[2].x -= selBorderD;
    pos[3].x -= selBorderD;
    break;
  }
  default: break;
  }
  
  // top
  glVertex3d(pos[0].x, pos[0].y, pos[0].z);
  glVertex3d(pos[1].x, pos[1].y, pos[1].z);
  glVertex3d(pos[0].x, pos[0].y - borderT, pos[0].z);

  glVertex3d(pos[1].x, pos[1].y, pos[1].z);
  glVertex3d(pos[0].x, pos[0].y - borderT, pos[0].z);
  glVertex3d(pos[1].x, pos[1].y - borderT, pos[1].z);
  
  //bot
  glVertex3d(pos[3].x, pos[3].y + borderT, pos[3].z);
  glVertex3d(pos[2].x, pos[2].y + borderT, pos[2].z);
  glVertex3d(pos[3].x, pos[3].y, pos[3].z);

  glVertex3d(pos[2].x, pos[3].y + borderT, pos[2].z);
  glVertex3d(pos[2].x, pos[2].y, pos[2].z);
  glVertex3d(pos[3].x, pos[3].y, pos[3].z);
  
  if(side == top || side == bot){
    // left
    glVertex3d(pos[0].x, pos[0].y - borderT, pos[0].z);
    glVertex3d(pos[0].x + borderT, pos[0].y - borderT, pos[0].z);
    glVertex3d(pos[3].x, pos[3].y + borderT, pos[3].z);

    glVertex3d(pos[0].x + borderT, pos[0].y - borderT, pos[0].z);
    glVertex3d(pos[3].x, pos[3].y + borderT, pos[3].z);
    glVertex3d(pos[3].x + borderT, pos[3].y + borderT, pos[3].z);

    // right
    glVertex3d(pos[1].x - borderT, pos[1].y - borderT, pos[1].z);
    glVertex3d(pos[1].x, pos[1].y - borderT, pos[1].z);
    glVertex3d(pos[2].x - borderT, pos[2].y + borderT, pos[2].z);

    glVertex3d(pos[1].x, pos[1].y - borderT, pos[1].z);
    glVertex3d(pos[2].x, pos[2].y + borderT, pos[2].z);
    glVertex3d(pos[2].x - borderT, pos[2].y + borderT, pos[2].z);
  }else if(side == left || side == right){
    // left
    glVertex3d(pos[0].x, pos[0].y - borderT, pos[0].z);
    glVertex3d(pos[0].x, pos[0].y - borderT, pos[0].z - borderT);
    glVertex3d(pos[3].x, pos[3].y + borderT, pos[3].z);

    glVertex3d(pos[0].x, pos[0].y - borderT, pos[0].z - borderT);
    glVertex3d(pos[3].x, pos[3].y + borderT, pos[3].z);
    glVertex3d(pos[3].x, pos[3].y + borderT, pos[3].z - borderT);

    // right
    glVertex3d(pos[1].x, pos[1].y - borderT, pos[1].z + borderT);
    glVertex3d(pos[1].x, pos[1].y - borderT, pos[1].z);
    glVertex3d(pos[2].x, pos[2].y + borderT, pos[2].z + borderT);

    glVertex3d(pos[1].x, pos[1].y - borderT, pos[1].z);
    glVertex3d(pos[2].x, pos[2].y + borderT, pos[2].z);
    glVertex3d(pos[2].x, pos[2].y + borderT, pos[2].z + borderT);
  }
  
  glEnd();

  free(pos);
}


vec3* wallPosBySide(vec3 basePos, Side side, float wallH, float wallD, float tileD, float tileW){
  // should be free() after wall rendered
  vec3* wallPos = malloc(sizeof(vec3) * 4);
  
  switch(side){
  case(top):{
    wallPos[0] =(vec3){basePos.x, basePos.y + wallH, basePos.z + wallD};
    wallPos[1] =(vec3){basePos.x + tileW,basePos.y + wallH, basePos.z + wallD};
    wallPos[2] =(vec3){basePos.x + tileW,basePos.y, basePos.z + wallD};
    wallPos[3] =(vec3){basePos.x, basePos.y, basePos.z + wallD};

    break;
  }
  case(bot):{
    wallPos[0] =(vec3){basePos.x, basePos.y + wallH, basePos.z + tileD - wallD};
    wallPos[1] =(vec3){basePos.x + tileW,basePos.y + wallH, basePos.z + tileD - wallD};
    wallPos[2] =(vec3){basePos.x + tileW,basePos.y, basePos.z + tileD - wallD};
    wallPos[3] =(vec3){basePos.x, basePos.y, basePos.z + tileD - wallD};
    
    break;
  }
  case(left):{
    wallPos[0] =(vec3){basePos.x + wallD, basePos.y + wallH, basePos.z + tileD};
    wallPos[1] =(vec3){basePos.x + wallD, basePos.y + wallH, basePos.z};
    wallPos[2] =(vec3){basePos.x + wallD, basePos.y, basePos.z};
    wallPos[3] =(vec3){basePos.x + wallD,basePos.y, basePos.z + tileD};
  
    break;
  }
  case(right):{
    wallPos[0] =(vec3){basePos.x + tileW - wallD,basePos.y + wallH, basePos.z + tileD};
    wallPos[1] =(vec3){basePos.x + tileW - wallD, basePos.y + wallH, basePos.z};
    wallPos[2] =(vec3){basePos.x + tileW - wallD, basePos.y, basePos.z};
    wallPos[3] =(vec3){basePos.x + tileW - wallD,basePos.y, basePos.z + tileD};
  
    break;
  }
  default: break;
  }

  return wallPos;
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

bool rayIntersectsTriangle(const vec3 start, const vec3 end, const vec3 lb, const vec3 rt, vec3* posOfIntersection, float* dist) {
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

  if (dist) {
    *dist = t;
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

float dotf(const vec3 v1, const vec3 v2) {
	float dproduct = 0;

	dproduct += v1.x * v2.x;
	dproduct += v1.y * v2.y;
	dproduct += v1.z * v2.z;

	return dproduct;
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

Object* doorConstructor(vec3 pos, bool opened){
  Object* newDoor = calloc(1,sizeof(Object));
  newDoor->pos = pos;
  newDoor->type = doorObj;
	    
  DoorInfo* doorInfo = malloc(sizeof(DoorInfo));
  doorInfo->opened = opened;
		  
  newDoor->objInfo = doorInfo;

  return newDoor;
}

bool oppositeTileTo(vec2i XZ, Side side, vec2i* opTile, Side* opSide){
  Side oppositeSide = 0;
	      
  switch(side){
  case(top):{
    XZ.z--;
    oppositeSide = bot;
    break;
  }
  case(bot):{
    XZ.z++;
    oppositeSide = top;
    break;
  }
  case(left):{
    XZ.x--;
    oppositeSide = right;
    break;
  }
  case(right):{
    XZ.x++;
    oppositeSide = left;
    break;
  }
  default: return false;
  }

  if(XZ.x >= 0 && XZ.z >= 0 && XZ.x < gridX && XZ.z < gridZ){
    if(opTile){
      *opTile = XZ;
    }

    if(opSide){
      *opSide = oppositeSide;
    }
    
    return true;
  }
  
  return false;
}


bool deleteIn(int* num, int index, uint8_t newValue){
  if(index >= 0 && index < 4){
    *num &= ~(0xFF << (index * 8));
    
    return true;
  }

  return false;
}

bool setIn(int* num, int index, uint8_t newValue){
  if(index >= 0 && index < 4){
    *num &= ~(0xFF << (index * 8));
    *num |= (newValue << index * 8);

    return true;
  }

  return false;
}

uint8_t valueIn(int num, int index){
  if(index >= 0 && index < 4){
    return (num >> (index*8)) & 0xFF; 
  }

  return 0;
}

Matrix4 Matrix4Multiply(Matrix4 a, Matrix4 b) {
    Matrix4 result;
    int i, j, k;

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            result.m[i][j] = 0;
            for (k = 0; k < 4; ++k) {
                result.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }

    return result;
}

vec3 Matrix4MultiplyVector(Matrix4 matrix, vec3 vector) {
    vec3 result = {0};

    result.x = matrix.m[0][0] * vector.x + matrix.m[0][1] * vector.y + matrix.m[0][2] * vector.z + matrix.m[0][3];
    result.y = matrix.m[1][0] * vector.x + matrix.m[1][1] * vector.y + matrix.m[1][2] * vector.z + matrix.m[1][3];
    result.z = matrix.m[2][0] * vector.x + matrix.m[2][1] * vector.y + matrix.m[2][2] * vector.z + matrix.m[2][3];

    return result;
}

/*

// render door frame
{
glBegin(GL_LINE_LOOP);
	  
glColor3d(redColor);

// first plank of frame(left)
glVertex3d(tile.x, tile.y, tile.z + bBlockD);
glVertex3d(tile.x, tile.y + doorH, tile.z + bBlockD);
glVertex3d(tile.x + doorPad/2, tile.y, tile.z + bBlockD);

glVertex3d(tile.x, tile.y + doorH, tile.z + bBlockD);
glVertex3d(tile.x + doorPad/2, tile.y + doorH, tile.z + bBlockD);
glVertex3d(tile.x + doorPad/2, tile.y, tile.z + bBlockD);

// top plank of frame
glVertex3d(tile.x, tile.y + doorH, tile.z + bBlockD);
glVertex3d(tile.x + bBlockW + doorPad/2, tile.y + doorFrameH, tile.z + bBlockD);
glVertex3d(tile.x, tile.y + doorFrameH, tile.z + bBlockD);

glVertex3d(tile.x, tile.y + doorH, tile.z + bBlockD);
glVertex3d(tile.x + bBlockW, tile.y + doorFrameH, tile.z + bBlockD);
glVertex3d(tile.x + bBlockW, tile.y + doorH, tile.z + bBlockD);

// second plank of frame (right)
glVertex3d(tile.x + doorW + doorPad, tile.y + doorH, tile.z + bBlockD);
glVertex3d(tile.x + doorW + doorPad/2, tile.y, tile.z + bBlockD);
glVertex3d(tile.x + doorW + doorPad /2, tile.y + doorH, tile.z + bBlockD);

glVertex3d(tile.x + doorW + doorPad/2, tile.y, tile.z + bBlockD);
glVertex3d(tile.x + doorW + doorPad, tile.y, tile.z + bBlockD);
glVertex3d(tile.x + doorW + doorPad, tile.y + doorH, tile.z + bBlockD);
	      
glEnd();
}
*/


