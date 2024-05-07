#version 330 core

//uniform vec3 lightPos;
uniform float far_plane;

//in vec4 FragPos;
//in vec3 lightPos;

in float len;

void main()
{

gl_FragDepth = len / far_plane;	
}