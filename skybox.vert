#version 400
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;
out vec3 vertexToPlayer;

uniform mat4 proj;
uniform mat4 view;
uniform vec3 cameraPos;	

void main()
{
TexCoords = aPos;

vertexToPlayer = cameraPos;// - aPos/30.f;

vec4 vertex = proj * view * vec4(aPos, 1.0);

vec4 snappedPos = vertex;
snappedPos.xyz = vertex.xyz / vertex.w;
vec2 resolution = { 200, 200 };	
snappedPos.xyz *= vertex.w;
gl_Position = snappedPos;
}  