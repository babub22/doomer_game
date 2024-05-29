#version 330 core

uniform sampler2D colorMap;
in vec2 TexCoord;

void main()
{
vec4 tex = texture(colorMap, TexCoord);

if(tex.a != 1.0f){
discard;
}
    // gl_FragDepth = gl_FragCoord.z;
}