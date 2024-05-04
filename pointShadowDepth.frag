#version 330 core

uniform vec3 lightPos;
uniform float far_plane;

in vec4 FragPos;

void main()
{

gl_FragDepth = length(FragPos.xyz - lightPos) / far_plane;	
}