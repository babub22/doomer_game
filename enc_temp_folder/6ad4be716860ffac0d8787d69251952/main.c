#include "deps.h"
#include "main.h"

void renderRect(vec2 leftBot, float w, float h, float c1, float c2, float c3){
    glBegin(GL_LINE_LOOP);

    /*leftTop.x -= leftTop.x / 2;
    leftTop.y -= leftTop.y / 2;*/

    // 0.0, 0.0 - center of rect
    // top - positive coords
    // down - neg coords

    glColor3f(c1, c2, c3); // Red
    glVertex2f(leftBot.x + w, leftBot.y + h);
    glVertex2f(leftBot.x, leftBot.y + h);
    glVertex2f(leftBot.x + w, leftBot.y);
    glEnd();

    glBegin(GL_LINE_LOOP);

    //    glColor3f(0.0f, 1.0f, 0.0f); // Green
    glVertex2f(leftBot.x, leftBot.y + h);
    glVertex2f(leftBot.x, leftBot.y);
    glVertex2f(leftBot.x + w, leftBot.y);

    glEnd();
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("SDL2 OpenGL Example",
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
      for(int y=0;y<gridH;y++){
	grid[y] = malloc(sizeof(Tile) * gridW);
	for(int x=0;x<gridW;x++){
	  if(y==gridH-1 || x == gridW - 1){
	    grid[y][x].center = 2;
	    
	    continue;
	  }

	  grid[y][x].center = 1;
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
	    else if( currentKeyStates[ SDL_SCANCODE_Q ] )
	      {
		curSelectedEntity = &e2;
	      }

	    else if( currentKeyStates[ SDL_SCANCODE_E ] )
	      {
		curSelectedEntity = &e2;
	      }
        }

        glClear(GL_COLOR_BUFFER_BIT);

	// draw X-Y axis
	if(true)
	{
	  glBegin(GL_LINES);

	  glColor3f(redColor); // Red
	  glVertex2f(0,-5);
	  glVertex2f(0,5);

	  glVertex2f(5,0);
	  glVertex2f(-5,0);

	  glEnd();
	}

	
	//renderRect((vec2){0.1f,0.2f}, 0.1f, 0.1f, redColor);
	// 	
	for (int y = 0; y < gridH; y++) {
	  for (int x = 0; x < gridW; x++) {
	    if(grid[y][x].center){
	      vec2 tile = { (float)x / 10 , (float)y / 10 };
	      // cenralize grid
	      tile.x -= 0.7f;
	      tile.y -= 0.7f;

	      if(grid[y][x].center == 1){
		renderRect(tile, 0.1f, 0.1f, greenColor);
	      }else if(grid[y][x].center == 2){
		renderRect(tile, 0.1f, 0.1f, redColor);
	      }
	    }
	  }
        }
	
	for (int y = 0; y < gridH; y++) {
	  for (int x = 0; x < gridW; x++) {
	    vec2 tile = { (float)x / 10 , (float)y / 10 };
	      renderRect(toIsoVec2(tile), 0.1f, 0.1f, greenColor);
	      
	     if(grid[y][x].center == 2){
	       //tile.y+=0.1f;
	       renderRect(toIsoVec2(tile), 0.1f, 0.2f, redColor);
	       tile.y += 0.1f;
		 renderRect(toIsoVec2(tile), 0.1f, 0.1f, redColor);
	     }
	  }
        }

	  //	renderRect(toIsoVec2(e1.pos), e1.w, e1.h, blueColor);
	  //	renderRect(toIsoVec2(e2.pos), e2.w, e2.h, darkPurple);
		
                   //glEnd();
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
