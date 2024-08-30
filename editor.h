void editor2dRender();
void editor3dRender();
void editorPreFrame(float deltaTime);
void editorPreLoop();
void editorEvents(SDL_Event event);
void editorOnSetInstance();
void editorMatsSetup(int curShader);
void editorMouseVS();

void createModel(int index, ModelType type);
void uniformLights();

//void addNewJointToStorage(WallJoint* newJoint);
void addNewWallStorage(Wall* newWall);
void addTileToStorage(Tile* newWall);

void deleteJointInStorage(int id);
void deleteWallInStorage(int id);

void initGizmosAABBFromSelected();

void deleteTileInStorage(int id);

UIBuf* batchUI(UIRect2* rects, int rectsSize);

void editorRenderCursor();

//UIBuf curUIBuf;

//void bindUIQuad(vec2 pos[6], uint8_t c[4], MeshBuffer* buf);

void setExitMarkerMarkerBrush();
void setPlayerEntityBrush();

void batchMarkers();
