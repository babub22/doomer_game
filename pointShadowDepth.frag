#version 330 core

//uniform vec3 lightPos;
uniform float far_plane;
uniform sampler2D colorMap;



in vec2 geo_TexCoords;
//in vec3 lightPos;

in float len;

void main()
{
vec4 tex = texture2D(colorMap, geo_TexCoords);

if(tex.a == .0f){
 discard;
}

gl_FragDepth = len / far_plane;	
}