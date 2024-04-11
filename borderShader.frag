#version 330 core

uniform vec3 borderColor;

void main()
{
    gl_FragColor = vec4(borderColor.xyz, 1.0);
}