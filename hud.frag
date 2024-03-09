#version 330

uniform sampler2D colorMap;

in vec2 TexCoord;
//uniform vec3 u_Color;

void main(void)
{
vec4 diffuseTexel = texture2D(colorMap, TexCoord);
gl_FragColor = diffuseTexel;
}