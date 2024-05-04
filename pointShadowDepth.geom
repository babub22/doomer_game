#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadowMatrices[6];

out vec4 FragPos;


/*
void emitFace(mat4 m) {
for(int i = 0; i < 3; ++i)
{
FragPos = gl_in[i].gl_Position;
gl_Position = m * FragPos;
EmitVertex();
}
EndPrimitive();
}

void main()
{
gl_Layer = 0;
emitFace(shadowMatrices[0]);

gl_Layer = 1;
emitFace(shadowMatrices[1]);

gl_Layer = 2;
emitFace(shadowMatrices[2]);

gl_Layer = 3;
emitFace(shadowMatrices[3]);

gl_Layer = 4;
emitFace(shadowMatrices[4]);

gl_Layer = 5;
emitFace(shadowMatrices[5]);
}*/

void main()
{

for(int face = 0; face < 6; ++face)
{
gl_Layer = face; // built-in variable that specifies to which face we render.
for(int i = 0; i < 3; ++i) // for each triangle's vertices
{
FragPos = gl_in[i].gl_Position;

//FragPos.y = -FragPos.y;

gl_Position = shadowMatrices[face] * FragPos;

EmitVertex();
}


EndPrimitive();
}

}