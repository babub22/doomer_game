#version 330
uniform vec3 u_Color;

void main(void)
{
    gl_FragColor = vec4(u_Color[0], u_Color[1], u_Color[2], 1.0f);
}