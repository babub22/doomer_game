#version 330
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNorm;	

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform float thick;

out vec2 TexCoord;

vec3 FragPos;

void main(void)
{
TexCoord= aTexCoord;

vec3 scaledPosition = aPos + aNorm * thick;

FragPos = vec3(model * vec4(aPos, 1.0));

gl_Position = proj * view * model * vec4(scaledPosition, 1.0);

//gl_Position = proj * view * vec4(FragPos + aNorm*0.01, //1.0);	
}