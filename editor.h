void editor2dRender();
void editor3dRender();
void editorPreFrame(float deltaTime);
void editorPreLoop();
void editorEvents(SDL_Event event);
void editorOnSetInstance();
void editorMatsSetup(int curShader);
void editorMouseVS();

void createLight(vec3 pos, int type);
void createModel(int index, ModelType type);
void uniformLights();

//void addNewJointToStorage(WallJoint* newJoint);
void addNewWallStorage(Wall* newWall);
void addTileToStorage(Tile* newWall);

void deleteJointInStorage(int id);
void deleteWallInStorage(int id);

void initGizmosAABBFromSelected();

