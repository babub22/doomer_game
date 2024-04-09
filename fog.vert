#version 330
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

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main(void)
{
TexCoord= aTexCoord;
Normal = aNormal;

FragPos = vec3(model * vec4(aPos, 1.0));
gl_Position = proj * view  * model * vec4(aPos, 1.0);

vertexToPlayer = cameraPos - vec3(gl_Position[0], gl_Position[1], gl_Position[2]);
}