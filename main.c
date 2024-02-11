#include "deps.h"
#include "main.h"

void renderWall(vec3 pos, WallType wall ,float w, float h, float r, float g, float b){
  glPushMatrix();
  glTranslated(0.5, 0.5, 0.5);
  glColor3d(r, g, b);

  glBegin(GL_LINES);

  const float blockW = 0.1f;
  const float blockD = 0.1f;

  switch(wall){
  case(top):{
    glVertex3d(pos.x, pos.y, pos.z);
    glVertex3d(pos.x, pos.y + h, pos.z);

    glVertex3d(pos.x + blockW,pos.y, pos.z);
    glVertex3d(pos.x + blockW,pos.y + h, pos.z);

    glVertex3d(pos.x ,pos.y + h, pos.z);
    glVertex3d(pos.x + blockW,pos.y + h, pos.z);

    glVertex3d(pos.x,pos.y, pos.z);
    glVertex3d(pos.x + blockW ,pos.y +h, pos.z);

    break;
  }
  case(bot):{
    glVertex3d(pos.x, pos.y, pos.z + blockD);
    glVertex3d(pos.x, pos.y + h, pos.z + blockD);

    glVertex3d(pos.x + blockW,pos.y, pos.z + blockD);
    glVertex3d(pos.x + blockW,pos.y + h, pos.z  + blockD);

    glVertex3d(pos.x ,pos.y + h, pos.z + blockD);
    glVertex3d(pos.x + blockW,pos.y, pos.z  + blockD);

    glVertex3d(pos.x ,pos.y + h, pos.z + blockD);
    glVertex3d(pos.x + blockW,pos.y + h, pos.z + blockD);

    break;
  }
  case(left):{
    glVertex3d(pos.x, pos.y, pos.z);
    glVertex3d(pos.x, pos.y + h, pos.z + blockD);

    glVertex3d(pos.x,pos.y, pos.z);
    glVertex3d(pos.x,pos.y + h, pos.z);

    glVertex3d(pos.x ,pos.y + h, pos.z);
    glVertex3d(pos.x,pos.y + h, pos.z + blockD);

    glVertex3d(pos.x ,pos.y, pos.z  + blockD);
    glVertex3d(pos.x,pos.y + h, pos.z + blockD);
  
    break;
  }
  case(right):{
    glVertex3d(pos.x + blockW, pos.y, pos.z);
    glVertex3d(pos.x + blockW, pos.y + h, pos.z);

    glVertex3d(pos.x + blockW,pos.y, pos.z + blockD);
    glVertex3d(pos.x + blockW,pos.y + h, pos.z + blockD);

    glVertex3d(pos.x + blockW,pos.y + h, pos.z);
    glVertex3d(pos.x + blockW,pos.y + h, pos.z + blockD);

    glVertex3d(pos.x + blockW,pos.y + h, pos.z);
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

int main(int argc, char* argv[]) {
  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window* window = SDL_CreateWindow("Doomer game",
					SDL_WINDOWPOS_CENTERED,
					SDL_WINDOWPOS_CENTERED,
					800, 600,
					SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

  SDL_GLContext context = SDL_GL_CreateContext(window);

  glClearColor(0.2f, 0.3f, 0.4f, 1.0f);

  const int gridH = 5;
  const int gridW = 6;
    
  Tile** grid = malloc(sizeof(Tile*) * gridH);

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
    
  Entity e1 = { (vec2) { 0.1f - 0.25f, 0.1f - 0.25f }, 0.1f, 0.17f };
  Entity e2 = { (vec2) { 0.1f - 0.25f, 0.2f - 0.25f }, 0.1f, 0.17f };

  Entity* curSelectedEntity = &e1;
    
  bool quit = false;
  while (!quit) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
	quit = true;
      }

      const Uint8* currentKeyStates = SDL_GetKeyboardState( NULL );

      if( currentKeyStates[ SDL_SCANCODE_UP ] )
	{
	  float dx = curSelectedEntity->pos.x;
	  float dy = curSelectedEntity->pos.y + 0.1f;

	  //if(grid[(int)dx*10][(int)dy*10] == 1){
	  curSelectedEntity->pos.y = dy;
	  //}
	}
      else if( currentKeyStates[ SDL_SCANCODE_DOWN ] )
	{
	  float dx = curSelectedEntity->pos.x;
	  float dy = curSelectedEntity->pos.y - 0.1f;

	  //if(grid[(int)dx*10][(int)dy*10] == 1){
	  curSelectedEntity->pos.y = dy;
	  //}
	}
      else if( currentKeyStates[ SDL_SCANCODE_LEFT ] )
	{
	  float dx = curSelectedEntity->pos.x - 0.1f ;
	  float dy = curSelectedEntity->pos.y;

	  //		if(grid[(int)dx*10][(int)dy*10] == 1){
	  curSelectedEntity->pos.x = dx;
	  //}
	}
      else if( currentKeyStates[ SDL_SCANCODE_RIGHT ] )
	{
	  float dx = curSelectedEntity->pos.x + 0.1f;
	  float dy = curSelectedEntity->pos.y;

	  //		if(grid[(int)dx*10][(int)dy*10] == 1){
	  curSelectedEntity->pos.x = dx;
	  //}
	}
      else if( currentKeyStates[ SDL_SCANCODE_1 ] )
	{
	  curSelectedEntity = &e1;
	}
      else if( currentKeyStates[ SDL_SCANCODE_2 ] )
	{
	  curSelectedEntity = &e2;
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

	renderCube(tile, 0.1f, 0.1f, 0.1f,darkPurple);

	tile.y+=0.1f;
	for (int i = 0; i < sizeof(grid[z][x].walls); i++) {
	  if((grid[z][x].walls >> i) & 1){
	    renderWall(tile, i, 0.1f, 0.1f,redColor);
	  }
	}
      }
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
