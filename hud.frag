#version 330

uniform sampler2D colorMap;

in vec2 TexCoord;

void main(void)
{
vec4 diffuseTexel = texture2D(colorMap, TexCoord);
gl_FragColor = diffuseTexel;
}