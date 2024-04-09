#version 330
uniform vec3 cameraPos;
uniform vec4 u_Color;	
uniform sampler2D colorMap;
uniform float radius;

varying vec3 vertexToPlayer;

uniform vec3 lightColor;
uniform vec3 lightPos;	

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;  

void main(void)
{
vec3 norm = normalize(Normal);
vec3 lightDir = normalize(lightPos - FragPos);

float diff = max(dot(norm, lightDir), 0.0);

vec3 diffuse = diff * lightColor;
vec3 ambient = 0.1 * lightColor;

vec4 diffuseTexel = texture2D(colorMap, TexCoord);
vec3 result = (ambient + diffuse) * diffuseTexel.xyz;

gl_FragColor = vec4(result, 1.0);

/*
float dist = length(vertexToPlayer);
float attenuation = clamp((radius - dist) / radius, 0.0, 1.0);


if(diffuseTexel.a == 0.0){
		  discard;
}

vec4 diffuseTerm  = diffuseTexel * attenuation + (.5 * (1.0-attenuation));
diffuseTerm.a = diffuseTexel.a; 

gl_FragColor = diffuseTerm;
*/
}