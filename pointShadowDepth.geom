#version 330 core

layout (triangles) in;
//layout (triangle_strip, max_vertices=18) out;
layout (triangle_strip, max_vertices=108) out;	

uniform mat4 shadowMatrices[6*6];

//out vec4 FragPos;
//out vec3 lightPos;

out float len;

uniform int lightsSize;
uniform vec3 lightsPos[6];

in vec2 TexCoords[];
out vec2 geo_TexCoords;	

void main()
{

for(int indx = 0; indx < lightsSize; ++indx){
for(int face = 0; face < 6; ++face){
gl_Layer = indx * 6 + face;
for(int i = 0; i < 3; ++i){

geo_TexCoords = TexCoords[i];
vec4 pos = gl_in[i].gl_Position;
len = length(pos.xyz - lightsPos[indx]);

gl_Position = shadowMatrices[gl_Layer] * gl_in[i].gl_Position;

EmitVertex();
}

//geo_TexCoords = TexCoords[0];
//EmitVertex();

EndPrimitive();
}
}

}