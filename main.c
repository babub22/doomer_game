#include "deps.h"
#include "main.h"

int main(int argc, char* argv[]) {
  SDL_Init(SDL_INIT_VIDEO);

  const int windowW = 800;
  const int windowH = 600;

  SDL_Window* window = SDL_CreateWindow("Doomer game",
					SDL_WINDOWPOS_CENTERED,
					SDL_WINDOWPOS_CENTERED,
					windowW, windowH,
					SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

  SDL_GLContext context = SDL_GL_CreateContext(window);

  SDL_ShowCursor(SDL_DISABLE);

  glClearColor(0.2f, 0.3f, 0.4f, 1.0f);

  const int gridH = 5;
  const int gridW = 6;
    
  Tile** grid = malloc(sizeof(Tile*) * gridH);

  Mouse mouse = { .h = 0.005f, .w = 0.005f};
  
  // init grid
  {
    for(int z=0;z<gridH;z++){
      grid[z] = calloc(gridW,sizeof(Tile));
      for(int x=0;x<gridW;x++){
	if(z==0){
	  grid[z][x].walls |= (1 << top); 
	}

	if(x==0){
	  grid[z][x].walls |= (1 << left);
	}

	if(x==gridW-1){
	  grid[z][x].walls |= (1 << right);
	}

	if(z==gridH-1){
	  grid[z][x].walls |= (1 << bot);
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

    for (int z = 0; z < gridH; z++) {
      for (int x = 0; x < gridW; x++) {
	vec3 tile = { (float)x / 10 , 0, (float)z / 10 };

	if (rayIntersectsTriangle(mouse.start, mouse.end, (vec3d) {tile.x,tile.y,tile.z})) {
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

	const float blockW = 0.1f;
	const float blockD = 0.1f;

	renderTile(tile, blockW, 0.1f, darkPurple);
	//	renderCube(tile, 0.1f, blockW, 0.1f,darkPurple);

	for (int wallType = 0; wallType < sizeof(grid[z][x].walls); wallType++) {
	  if((grid[z][x].walls >> wallType) & 1){
	    renderWall(tile, blockW, blockD, wallType, 0.1f,redColor);
	  }
	}
      }
    }

    printf("nest\n\n");
    
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

      mouse.start = start;
      mouse.end = end;
      
      glBegin(GL_LINES);
      glVertex3d(start.x, start.y, start.z);
      glVertex3d(start.x,start.y+0.2f, start.z);
      glEnd();      
    }

    // renderCursor
    {
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
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

vec2 toIsoVec2(vec2 point){
  return (vec2){ point.x - point.y, (point.x + point.y)/2 };
}

void renderWall(vec3 pos, float blockW, float blockD, WallType wall, float wallH, float r, float g, float b){
  glPushMatrix();
  glTranslated(0.5, 0.5, 0.5);
  glColor3d(r, g, b);

  glBegin(GL_LINES);

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
    glVertex3d(pos.x, pos.y, pos.z);
    glVertex3d(pos.x, pos.y + wallH, pos.z + blockD);

    glVertex3d(pos.x,pos.y, pos.z);
    glVertex3d(pos.x,pos.y + wallH, pos.z);

    glVertex3d(pos.x ,pos.y + wallH, pos.z);
    glVertex3d(pos.x,pos.y + wallH, pos.z + blockD);

    glVertex3d(pos.x ,pos.y, pos.z  + blockD);
    glVertex3d(pos.x,pos.y + wallH, pos.z + blockD);
  
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

  glPopMatrix();
}

void renderCube(vec3 pos, float w, float h, float d, float r, float g, float b){
  glPushMatrix();
  glTranslated(0.5, 0.5, 0.5);
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
  
  glPopMatrix();
}

bool rayIntersectsTriangle(const vec3d start, const vec3d end, const vec3d point) {
  vec3d dirfrac = {0};

  vec3d norm = normalize(end);
  
  dirfrac.x = 1.0f / norm.x;
  dirfrac.y = 1.0f / norm.y;
  dirfrac.z = 1.0f / norm.z;
  // lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
  // r.org is origin of ray

  vec3d rt = { point.x+0.1f, point.y, point.z+0.1f  };
  vec3d lb = { point.x, point.y, point.z };

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

vec3d normalize(vec3d vec) {
    float vecLen = sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    vec3d norm = { 0 };

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
