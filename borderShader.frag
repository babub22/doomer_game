#version 330 core

uniform vec3 borderColor;

uniform sampler2D colorMap;
in vec2 TexCoord; 

void main()
{
vec4 tex = texture2D(colorMap, TexCoord);

if(tex.a == 0.0){
	 discard;
}

gl_FragColor = vec4(borderColor.xyz, 1.0);
}