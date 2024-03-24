#version 330
uniform vec3 cameraPos;
uniform vec4 u_Color;	
uniform sampler2D colorMap;
uniform float radius;

varying vec3 vertexToPlayer;

in vec2 TexCoord;

void main(void)
{
vec4 diffuseTexel = texture2D(colorMap, TexCoord);
float dist = length(vertexToPlayer);
float attenuation = clamp((radius - dist) / radius, 0.0, 1.0);


if(diffuseTexel.a == 0.0){
		  discard;
}

vec4 diffuseTerm  = diffuseTexel * attenuation + (.5 * (1.0-attenuation));
diffuseTerm.a = diffuseTexel.a; 

gl_FragColor = diffuseTerm;
//gl_FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
}