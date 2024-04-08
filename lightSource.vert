#version 330
//uniform vec3 cameraPos;
//uniform sampler2D colorMap;
//uniform float radius;

layout(location = 0) in vec3 aPos;
//layout(location = 1) in vec2 aTexCoord;

//out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main(void)
{
//TexCoord= aTexCoord;

gl_Position = proj * view  * model * vec4(aPos, 1.0);
}