#version 330

uniform sampler2D colorMap;

in vec2 TexCoord;

void main(void)
{
vec4 diffuseTexel = texture2D(colorMap, TexCoord);

//if(diffuseTexel.a == 0.0f){
//		  discard;
//}

gl_FragColor = diffuseTexel;
}