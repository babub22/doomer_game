#version 400

uniform vec3 cameraPos;
uniform sampler2D colorMap;
uniform float radius;

varying vec3 vertexToPlayer;

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace[8];  

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform mat4 lightSpaceMatrix[8];
uniform int dirShadowLightsSize;

out vec2 texCoords;
out float affline;

uniform float screenW;
uniform float screenH;

const float PI = 3.1415926535897932384626433832795;
/*
const float waveLength = .3f;
const float waveAmplitude = .35f;
const float waveSpeed  = 1.0f;

const float waveLength = .2f;
const float waveAmplitude = .15f;
const float waveSpeed  = 10.0f;
*/

const float waveLength = .2f;
const float waveAmplitude = .01f;
const float waveSpeed  = 1.0f;

uniform float dTime;

float generateDistortion(float x, float z) {
float radX = (x / waveLength) * 2.0f * PI;
float radZ = (z / waveLength) * 2.0f * PI;
return waveAmplitude * sin(radX + radZ + (dTime * waveSpeed));
}

vec3 applyDistortion(vec3 vertex) {
float newY = generateDistortion(vertex.x, vertex.z);
return vec3(vertex.x, vertex.y + newY, vertex.z);  // Only modify y (height)
}


void main(void)
{
TexCoord= aTexCoord;
Normal = aNormal;

FragPos = vec3(model * vec4(aPos, 1.0));

vec3 vertex3 = applyDistortion(vec3(aPos));
vec4 vertex = proj * view  * model * vec4(vertex3, 1.0);

// make PS1 textures distortion
{
vec4 snappedPos = vertex;
snappedPos.xyz = vertex.xyz / vertex.w; // convert to normalised device coordinates (NDC)
//vec2 resolution = { screenW, screenH };
//vec2 resolution = { 256, 224 };
vec2 resolution = { 200, 200 };	
snappedPos.xy = floor(resolution * snappedPos.xy) / resolution; // snap the vertex to the lower-resolution grid
snappedPos.xyz *= vertex.w;
//snappedPos
gl_Position = snappedPos;
}

for(int i=0;i<dirShadowLightsSize;i++){
FragPosLightSpace[i] = lightSpaceMatrix[i] * vec4(FragPos,1.0f);
}

vertexToPlayer = cameraPos - FragPos;
}