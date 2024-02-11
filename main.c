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

  vec2 cursor = {0};
  
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
    
  bool quit = false;
  while (!quit) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
	quit = true;
      }

      if (event.type == SDL_MOUSEMOTION) {
	cursor.x = event.motion.x;
	cursor.y = event.motion.y;
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
    }

    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    /* use this length so that camera is 1 unit away from origin */
    double dist = sqrt(1 / 3.0);

    gluLookAt(dist, dist, dist,  /* position of camera */
	      0.0,  0.0,  0.0,   /* where camera is pointing at */
	      0.0,  1.0,  0.0);  /* which direction is up */
    glMatrixMode(GL_MODELVIEW);

    if(true)
      {
	glBegin(GL_LINES);

	glColor3d(1.0, 0.0, 0.0);
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(1.0, 0.0, 0.0);

	glColor3d(0.0, 1.0, 0.0);
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(0.0, 1.0, 0.0);

	glColor3d(0.0, 0.0, 1.0);
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(0.0, 0.0, 1.0);

	glEnd();
      }

    for (int z = 0; z < gridH; z++) {
      for (int x = 0; x < gridW; x++) {
	vec3 tile = { (float)x / 10 ,0, (float)z / 10 };
	const float blockW = 0.1f;
	const float blockD = 0.1f;

	renderCube(tile, 0.1f, blockW, 0.1f,darkPurple);

	tile.y+=0.1f;
	for (int wallType = 0; wallType < sizeof(grid[z][x].walls); wallType++) {
	  if((grid[z][x].walls >> wallType) & 1){
	    renderWall(tile, blockW, blockD, wallType, 0.1f,redColor);
	  }
	}
      }
    }

    renderCube(e1.pos, e1.w, e1.h, e1.d,greenColor);

    // renderCursor
    {
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      glOrtho(0, windowW, windowH, 0, -1, 1); // orthographic projection

      glBegin(GL_LINES);

      float relWindowX = cursorW * windowW;
      float relWindowY = cursorH * windowH;
      
      glVertex2f(cursor.x - relWindowX, cursor.y + relWindowY);
      glVertex2f(cursor.x + relWindowX, cursor.y - relWindowY);

      glVertex2f(cursor.x + relWindowX, cursor.y + relWindowY);
      glVertex2f(cursor.x - relWindowX, cursor.y - relWindowY);
      
      glEnd();
    }

    
    glFlush();
 
    SDL_GL_SwapWindow(window);
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
