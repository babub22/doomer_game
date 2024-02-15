#include "deps.h"
#include "main.h"

Object** objsStore;
size_t objsStoreSize;

int main(int argc, char* argv[]) {
  SDL_Init(SDL_INIT_VIDEO);


  SDL_Window* window = SDL_CreateWindow("Doomer game",
					SDL_WINDOWPOS_CENTERED,
					SDL_WINDOWPOS_CENTERED,
					windowW, windowH,
					SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

  SDL_GLContext context = SDL_GL_CreateContext(window);
  
  SDL_ShowCursor(SDL_DISABLE);

  glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
  
  Tile** grid = malloc(sizeof(Tile*) * (gridH + balkonH));
  
  Mouse mouse = { .h = 0.005f, .w = 0.005f};
  
  // init grid
  {
    for(int z=0;z<gridH+balkonH;z++){
      grid[z] = calloc(gridW,sizeof(Tile));
      const bool isBalkonZ = z == gridH+balkonH - 1;
      
      for(int x=0;x<gridW;x++){
	const int centerX = (gridW-1)/2;

	if(isBalkonZ){
	  if(x<centerX || x>centerX+2){
	    continue;
	  }
	}
	
	if(z==0){
	  grid[z][x].walls |= (wallT << top*8); 
	}

	if(x==0 || (isBalkonZ && x == centerX)){
	  grid[z][x].walls |= (wallT << left*8);
	}

	if(x==gridW-1 || (isBalkonZ && x == centerX+2)){
	  grid[z][x].walls |= (wallT << right*8);
	}

	if(z==gridH-1 || isBalkonZ){
	  if(z==gridH-1 && x==(gridW-1)/2){
	    grid[z][x].walls |= (doorT << bot*8);

	    Object* newDoor = calloc(1,sizeof(Object));
	    newDoor->pos = (vec3){x,0,z};
	    newDoor->type = doorObj;
	    
	    DoorInfo* doorInf = malloc(sizeof(DoorInfo));
	    doorInf->opened = false;

	    newDoor->objInfo = doorInf;

	    addObjToTile(&grid[z][x], newDoor);
	    addObjToStore(newDoor);
	  }else{
	    grid[z][x].walls |= (wallT << bot*8);
	  }
	}

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
    Uint32 starttime = GetTickCount();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
	quit = true;
      }

      if(event.type == SDL_MOUSEBUTTONDOWN){
	if(event.button.button == SDL_BUTTON_LEFT){
	  mouse.click = true;
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
      else if(currentKeyStates[ SDL_SCANCODE_W ]){
	zoom+=0.1f;
      }
      else if(currentKeyStates[ SDL_SCANCODE_S ]){
	zoom-=0.1f;
      }
    }

    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    /* use this length so that camera is 1 unit away from origin */
    double dist = sqrt(zoom / 3.0);

    gluLookAt(dist, dist, dist,  /* position of camera */
	      0.0,  0.0,  0.0,   /* where camera is pointing at */
	      0.0,  1.0,  0.0);  /* which direction is up */
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

	const vec3 rt = { tile.x+0.1f, tile.y, tile.z + 0.1f };
	const vec3 lb = tile;
	
	if (rayIntersectsTriangle(mouse.start, mouse.end, lb, rt)) {
	  glBegin(GL_TRIANGLES);
	  
	  glColor3d(darkPurple);

	  glVertex3d(tile.x, tile.y, tile.z);
	  glVertex3d(tile.x + 0.1f, tile.y, tile.z);
	  glVertex3d(tile.x + 0.1f, tile.y, tile.z + 0.1f);

	  glVertex3d(tile.x, tile.y, tile.z);
	  glVertex3d(tile.x, tile.y, tile.z + 0.1f);
	  glVertex3d(tile.x + 0.1f, tile.y, tile.z + 0.1f);

	  glEnd();
	}


	renderTile(tile, blockW, 0.1f, darkPurple);

	for(int side=0;side<sideCounter;side++){
	  WallType type = (grid[z][x].walls >> (side*8)) & 0xFF;

	  switch(type){
	  case(doorT):{
	    for(int i=0;i<grid[z][x].objsSize;i++){
	      if(grid[z][x].objs[i]->type == doorObj){
		Object* doorObj = grid[z][x].objs[i];
		DoorInfo* doorInfo = (DoorInfo*) grid[z][x].objs[i]->objInfo;

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
		
		vec3 doorPos = {tile.x + doorXPad / 2,tile.y,tile.z};

		vec3 lb = {0};
		vec3 rt = {0};

		switch(side){
		case(top):{
		  lb = doorPos;
		  rt = (vec3){tile.x+doorW, tile.y+doorH, tile.z};
		  break;
		}
		case(bot):{
		  lb = (vec3){tile.x,tile.y, tile.z + blockD};
		  rt = (vec3){tile.x+doorW, tile.y+doorH, tile.z + blockD};
		  break;
		}
		case(left):{
		  lb = (vec3){tile.x,tile.y, tile.z};
		  rt = (vec3){tile.x, tile.y+doorH, tile.z + blockD};
		  break;
		}
		case(right):{
		  lb = (vec3){tile.x + doorW,tile.y, tile.z};
		  rt = (vec3){tile.x+ doorW, tile.y+doorH, tile.z + blockD};
		  break;
		}
		default: break;
		}

		if(rayIntersectsTriangle(mouse.start,mouse.end,lb,rt)){
		  if(mouse.click && doorObj->anim.frames == 0){
		    doorInfo->opened = !doorInfo->opened;
		  }

		  if(doorInfo->opened){
		    glPushMatrix();
		    glTranslatef(doorPos.x, doorPos.y, doorPos.z);
		    glRotatef(90, 0, 1, 0);
		    glTranslatef(-doorPos.x-0.2f+doorXPad, -doorPos.y, -doorPos.z -0.1f );
		  }		    //   doorObj->anim.frames = 90; // 60 * 3;
		  
		  renderWall(doorPos, GL_TRIANGLE_STRIP, doorW, blockD, side, doorH, blueColor);

		  if(doorInfo->opened){
		    glPopMatrix();
		  }
		}else{
		  if(doorInfo->opened){
		    glPushMatrix();
		    glTranslatef(doorPos.x, doorPos.y, doorPos.z);
		    glRotatef(90, 0, 1, 0);
		    glTranslatef(-doorPos.x-0.2f+doorXPad, -doorPos.y, -doorPos.z -0.1f );
		  }		    //   doorObj->anim.frames = 90; // 60 * 3;
		  
		  renderWall(doorPos, GL_LINES, doorW, blockD, side, doorH, blueColor);

		  if(doorInfo->opened){
		    glPopMatrix();
		  }
		};
	      }
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

	    if(rayIntersectsTriangle(mouse.start,mouse.end,lb,rt)){
	      renderWall(tile, GL_TRIANGLE_STRIP, blockW, blockD, side, blockH,redColor);
	    }else{
	      renderWall(tile, GL_LINES, blockW, blockD, side, blockH,redColor);
	    };

	    break;
	  }
	  default: break;
	  }
	  //	}
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

  mouse.click = false;
  SDL_GL_SwapWindow(window);

  Uint32 endtime = GetTickCount();
  Uint32 deltatime = endtime - starttime;

  if (deltatime > (1000 / FPS)) {
  } else {
    Sleep((1000 / FPS) - deltatime);
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

bool rayIntersectsTriangle(const vec3 start, const vec3 end, const vec3 lb, const vec3 rt) {
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
  
  if(res)
    printf("%f \n",t);

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

void renderTile(vec3 pos, float w, float d, float r, float g, float b){
  glBegin(GL_LINES);
  glColor3d(r, g, b);
  
  glVertex3d(pos.x, pos.y, pos.z);
  glVertex3d(pos.x + w, pos.y, pos.z);

  glVertex3d(pos.x + w,pos.y, pos.z);
  glVertex3d(pos.x + w,pos.y, pos.z + d);

  glVertex3d(pos.x + w,pos.y, pos.z+d);
  glVertex3d(pos.x ,pos.y, pos.z +d);

  glVertex3d(pos.x ,pos.y, pos.z+d);
  glVertex3d(pos.x,pos.y, pos.z);

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
