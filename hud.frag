#version 330

uniform sampler2D colorMap;

in vec2 TexCoord;
uniform vec3 u_Color;

void main(void)
{
vec4 diffuseTexel = texture2D(colorMap, TexCoord);
gl_FragColor = diffuseTexel;
//gl_FragColor = vec4(u_Color[0], u_Color[1], //u_Color[2], 1.0f);
}