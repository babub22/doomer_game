#version 330

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform vec2 model2D;
uniform vec2 offset;	

out vec2 TexCoord;

void main(void)
{
TexCoord=aTexCoord;

vec2 point = model2D * (offset+aPos);
gl_Position = vec4(point,0.0f,1.0f);
}