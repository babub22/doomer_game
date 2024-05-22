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

void initGizmosAABBFromSelected();

