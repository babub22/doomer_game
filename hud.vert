#version 330

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTex;	

out vec2 TexCoord;

uniform mat4 ortho;
uniform vec2 viewport;	

void main(void)
{
 TexCoord=aTex;	
 gl_Position = vec4(aPos,0.0f,1.0f);
}