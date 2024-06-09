#version 330

uniform sampler2D colorMap;

in vec2 TexCoord;

void main(void)
{
vec4 tex = texture(colorMap, TexCoord);

gl_FragColor = tex;
}