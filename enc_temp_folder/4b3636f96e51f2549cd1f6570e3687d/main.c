#include "deps.h"
#include "main.h"

Object** objsStore;
size_t objsStoreSize;

int main(int argc, char* argv[]) {
  SDL_Init(SDL_INIT_VIDEO);

  char windowTitle[] = game;

  SDL_Window* window = SDL_CreateWindow(windowTitle,
					SDL_WINDOWPOS_CENTERED,
					SDL_WINDOWPOS_CENTERED,
					windowW, windowH,
					SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

  SDL_GLContext context = SDL_GL_CreateContext(window);
  
  SDL_ShowCursor(SDL_DISABLE);

  glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
  
  Tile** grid = malloc(sizeof(Tile*) * (gridH + balkonH));
  
  Mouse mouse = { .h = 0.005f, .w = 0.005f, .brush = 0 };
  
  // init grid
  {
    for(int z=0;z<gridH+balkonH;z++){
      grid[z] = calloc(gridW,sizeof(Tile));
      
      for(int x=0;x<gridW;x++){
	grid[z][x].center = 1;
      }
    }
  }

  const float entityW = 0.1f / 2.0f;
  const float entityD = 0.05f;
    
  Entity e1 = { (vec3) { 0.1f + entityW / 2.0f, 0.0f + 0.1f, 0.1f + entityD / 2.0f }, 0.05f, 0.17f, 0.05f };

  float zoom = 1;
    
  bool quit = false;
  while (!quit) {
    uint32_t starttime = GetTickCount();

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

      if (event.type == SDL_MOUSEMOTION) {
	mouse.screenPos.x = event.motion.x;
	mouse.screenPos.y = event.motion.y;
      }

      const Uint8* currentKeyStates = SDL_GetKeyboardState( NULL );

      if( currentKeyStates[ SDL_SCANCODE_UP ] )
	{
	  float dx = e1.pos.x;
	  float dz = e1.pos.z - 0.1f;

	  e1.pos.z = dz;
	}
      else if( currentKeyStates[ SDL_SCANCODE_DOWN ] )
	{
	  float dx = e1.pos.x;
	  float dz = e1.pos.z + 0.1f;

	  e1.pos.z = dz;
	}
      else if( currentKeyStates[ SDL_SCANCODE_LEFT ] )
	{
	  float dx = e1.pos.x - 0.1f ;
	  float dz = e1.pos.z;

	  e1.pos.x = dx;
	}
      else if( currentKeyStates[ SDL_SCANCODE_RIGHT ] )
	{
	  float dx = e1.pos.x + 0.1f;
	  float dz = e1.pos.z;

	  e1.pos.x = dx;
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
	// brush = windowT;
      }
      else if(currentKeyStates[ SDL_SCANCODE_4 ]){
	//	brush = halfWallT;
      }
      else if(currentKeyStates[ SDL_SCANCODE_DELETE ]){
	if(mouse.wallSide != -1){
	  WallType type = (mouse.selectedTile->walls >> (mouse.wallSide*8)) & 0xFF;
	  mouse.selectedTile->walls &= ~(0xFF << (mouse.wallSide * 8));

	  if(type == doorT){
	    int newSize = 0;
	    
	    for(int side=sideCounter;side!=0;side--){
	      if ((mouse.selectedTile->walls >> ((side-1) * 8)) & 0xFF) {
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
	    //}
	  }
	}
      }
    }

    mouse.selectedTile = NULL;
    
    mouse.wallSide = -1;
    mouse.tileSide = -1;
	
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    /* use this length so that camera is 1 unit away from origin */
    double dist = sqrt(zoom / 3.0);

    gluLookAt(dist, dist, dist,  /* position of camera */
	      0.0f, 0.0f, 0.0f,   /* where camera is pointing at */
	      0.0f,  1.0f,  0.0f);  /* which direction is up */
    glMatrixMode(GL_MODELVIEW);
    
    const float blockW =  0.1f;
    const float blockD =  0.1f;
    const float blockH =  0.025f;

    const float doorXPad =  blockW / 4;
    
    const float doorW =  blockW - doorXPad;

    const float blockHForDoor = 0.2f;
    //const float doorH =  blockH * 0.85;
    const float doorH =  blockHForDoor * 0.85;

    for (int z = 0; z < gridH + balkonH; z++) {
      for (int x = 0; x < gridW; x++) {
	vec3 tile = { (double)x / 10 , 0, (double)z / 10 };

	if(!grid[z][x].center){
	  continue;
	}

	// tile things
	{
	  const vec3 rt = { tile.x+0.1f, tile.y, tile.z + 0.1f };
	  const vec3 lb = tile;

	  vec3 intersection = {0};
	
	  if (rayIntersectsTriangle(mouse.start, mouse.end, lb, rt, &intersection)) {
	    mouse.selectedTile = &grid[z][x];
	    
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
	      vec3 doorPos = {tile.x + doorXPad / 2,tile.y,tile.z };
	      
	      if(mouse.tileSide == left || mouse.tileSide == right){
		doorPos.z += doorXPad/2;
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
	      break;
	    }
	    default: break;//assert(0 && "Unsuported brush type");
	    }

	    if(mouse.tileSide != -1){
	      if(mouse.clickR){

		if(mouse.brush == doorT){
		  int wallsSize = 0;

		  // to get wallsSize can be optimized
		  // to O(4) -> O(1) by using if's 
		  for(int side=sideCounter;side!=0;side--){
		    if ((grid[z][x].walls >> ((side-1) * 8)) & 0xFF) {
		      wallsSize = side;
		      break;
		    };
		  }
		  
		  
		  if(mouse.tileSide+1 > wallsSize) {
			  if (!grid[z][x].wallsData) {
				  grid[z][x].wallsData = malloc((mouse.tileSide + 1) * sizeof(Object*));
			  }
			  else {
				  grid[z][x].wallsData = realloc(grid[z][x].wallsData, (mouse.tileSide + 1) * sizeof(Object*));
			  }
		  }
	      	      
		  Object* newDoor = calloc(1,sizeof(Object));
		  newDoor->pos = (vec3){x,0,z};
		  newDoor->type = doorObj;
	    
		  DoorInfo* doorInfo = malloc(sizeof(DoorInfo));
		  doorInfo->opened = false;
		  
		  newDoor->objInfo = doorInfo;
		  grid[z][x].wallsData[mouse.tileSide] = newDoor;
		
		  addObjToStore(newDoor);
		}

		
		grid[z][x].walls |= (mouse.brush << mouse.tileSide*8);
	      }
	    }
	  }
	  
	  renderTile(tile, GL_LINES, blockW, 0.1f, darkPurple);
	}

	// walls rendering
	for(int side=0;side<sideCounter;side++){
	  WallType type = (grid[z][x].walls >> (side*8)) & 0xFF;

	  switch(type){
	  case(doorT):{
	    Object* doorObj = grid[z][x].wallsData[side];
	    DoorInfo* doorInfo = (DoorInfo*) doorObj->objInfo;

	    vec3 doorPos = {tile.x + doorXPad / 2,tile.y,tile.z};

	    vec3 lb = {0};
	    vec3 rt = {0};

	    vec3 postRotationPos = { 0 };
		
	    switch(side){
	    case(top):{
	      lb = doorPos;
	      rt = (vec3){tile.x+doorW, tile.y+doorH, tile.z};

	      postRotationPos = (vec3){0, 0, 0.1f -doorXPad};
	      break;
	    }
	    case(bot):{
	      lb = (vec3){tile.x,tile.y, tile.z + blockD};
	      rt = (vec3){tile.x+doorW, tile.y+doorH, tile.z + blockD};

	      postRotationPos = (vec3){-0.2f+doorXPad, 0, -0.1f};
	      break;
	    }
	    case(left):{
	      lb = (vec3){tile.x,tile.y, tile.z};
	      rt = (vec3){tile.x, tile.y+doorH, tile.z + blockD};
		  
	      postRotationPos = (vec3){-0.1f+doorXPad, 0, -0.1f};
	      break;
	    }
	    case(right):{
	      lb = (vec3){tile.x + doorW,tile.y, tile.z};
	      rt = (vec3){tile.x+ doorW, tile.y+doorH, tile.z + blockD};

	      postRotationPos = (vec3){-0.1f, 0, 0.1f - doorXPad};
	      break;
	    }
	    default: break;
	    }

	    bool isIntersect = rayIntersectsTriangle(mouse.start,mouse.end,lb,rt, NULL);

	    if(isIntersect){
			mouse.wallSide = side;
			mouse.selectedTile = &grid[z][x];

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
	      doorPos.z += doorXPad/2;
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
	      mouse.selectedTile = &grid[z][x];
		
	      renderWall(tile, GL_TRIANGLE_STRIP, blockW, blockD, side, blockH,redColor);
	    }else{
	      renderWall(tile, GL_LINES, blockW, blockD, side, blockH,redColor);
	    };

	    break;
	  }
	  default: break;
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
    
    renderCube(e1.pos, e1.w, e1.h, e1.d,greenColor);

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

      gluUnProject(winX, winY, 0.0, matModelView, matProjection, 
		   viewport, &start.x, &start.y, &start.z);

      gluUnProject(winX, winY, 10.0, matModelView, matProjection, 
		   viewport, &end.x, &end.y, &end.z);

      mouse.start = vec3dToVec3(start);
      mouse.end = vec3dToVec3(end);
      
      glBegin(GL_LINES);
      glVertex3d(start.x, start.y, start.z);
      glVertex3d(start.x,start.y+0.2f, start.z);
      glEnd();      
    }

    // renderCursor
    {
      glMatrixMode(GL_PROJECTION);
      //glPushMatrix();
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

    glVertex3d(pos.x ,pos.y + wallH, pos.z + blockD);
    glVertex3d(pos.x + blockW,pos.y, pos.z  + blockD);

    glVertex3d(pos.x ,pos.y + wallH, pos.z + blockD);
    glVertex3d(pos.x + blockW,pos.y + wallH, pos.z + blockD);

    break;
  }
  case(left):{
    glVertex3d(pos.x,pos.y + wallH, pos.z);
    glVertex3d(pos.x, pos.y, pos.z);

    //    glVertex3d(pos.x,pos.y, pos.z);
    glVertex3d(pos.x, pos.y + wallH, pos.z + blockD);
    glVertex3d(pos.x,pos.y + wallH, pos.z);

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



/*

// render door frame
{
glBegin(GL_LINE_LOOP);
	  
glColor3d(redColor);

// first plank of frame(left)
glVertex3d(tile.x, tile.y, tile.z + blockD);
glVertex3d(tile.x, tile.y + doorH, tile.z + blockD);
glVertex3d(tile.x + doorXPad/2, tile.y, tile.z + blockD);

glVertex3d(tile.x, tile.y + doorH, tile.z + blockD);
glVertex3d(tile.x + doorXPad/2, tile.y + doorH, tile.z + blockD);
glVertex3d(tile.x + doorXPad/2, tile.y, tile.z + blockD);

// top plank of frame
glVertex3d(tile.x, tile.y + doorH, tile.z + blockD);
glVertex3d(tile.x + blockW + doorXPad/2, tile.y + blockHForDoor, tile.z + blockD);
glVertex3d(tile.x, tile.y + blockHForDoor, tile.z + blockD);

glVertex3d(tile.x, tile.y + doorH, tile.z + blockD);
glVertex3d(tile.x + blockW, tile.y + blockHForDoor, tile.z + blockD);
glVertex3d(tile.x + blockW, tile.y + doorH, tile.z + blockD);

// second plank of frame (right)
glVertex3d(tile.x + doorW + doorXPad, tile.y + doorH, tile.z + blockD);
glVertex3d(tile.x + doorW + doorXPad/2, tile.y, tile.z + blockD);
glVertex3d(tile.x + doorW + doorXPad /2, tile.y + doorH, tile.z + blockD);

glVertex3d(tile.x + doorW + doorXPad/2, tile.y, tile.z + blockD);
glVertex3d(tile.x + doorW + doorXPad, tile.y, tile.z + blockD);
glVertex3d(tile.x + doorW + doorXPad, tile.y + doorH, tile.z + blockD);
	      
glEnd();
}
*/

