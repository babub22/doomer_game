#version 330

layout(location = 0) in vec2 aPos;
//layout(location = 1) in vec4 aColor;	

out vec4 ColorSpec;

uniform vec2 model2D;
uniform vec2 offset;
uniform vec3 color;		

void main(void)
{
ColorSpec = vec4(color, 1.0f);

vec2 point = model2D * (offset + aPos);
gl_Position = vec4(point,0.0f,1.0f);
}