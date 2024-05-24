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
out vec4 FragPosLightSpace;  

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform mat4 lightSpaceMatrix;

//uniform vec3 mapCenter;

void main(void)
{
TexCoord= aTexCoord;
Normal = aNormal;

FragPos = vec3(model * vec4(aPos, 1.0));
gl_Position = proj * view  * model * vec4(aPos, 1.0);

FragPosLightSpace = lightSpaceMatrix * vec4(FragPos,1.0f);

vertexToPlayer = cameraPos - FragPos;
}