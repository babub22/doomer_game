#version 330

layout(location = 0) in vec3 aPos;

uniform mat4 ortho;  

void main(void)
{
//vec2 screenPos = aPos * //vec2(2.0/1280.0f,2.0/720.0f) - //vec2(1,1);

 gl_Position = ortho * vec4(aPos, 1);
}