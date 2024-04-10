#version 330
uniform vec3 cameraPos;
uniform sampler2D colorMap;
uniform float radius;

//varying vec3 vertexToPlayer;

//uniform vec3 lightColor;
//uniform vec3 lightPos;	

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

/*struct Material {
vec3 ambient;
vec3 diffuse;
vec3 specular;
float shininess;
};*/

struct Light {
vec3  pos;
vec3  dir;
vec3 color;

float rad;
float cutOff;

float constant;
float linear;
float quadratic;
};

uniform Light light;

//uniform Material material;

void main(void)
{

vec4 diffuseTexel = texture2D(colorMap, TexCoord);

vec3 lightDir = normalize(light.pos - FragPos);

// ambient 
vec3 ambient = 0.10 * light.color;

// diffuse
vec3 norm = normalize(Normal);
float diff = max(dot(norm, lightDir), 0.0);
vec3 diffuse = diff * light.color;

// specular
float specularStrength = 0.2;
vec3 viewDir = normalize(cameraPos - FragPos);
vec3 reflectDir = reflect(-lightDir, norm);

float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
vec3 specular = specularStrength * spec * light.color;  

float theta = dot(lightDir, normalize(-light.dir));
float epsilon   = light.rad - light.cutOff;
float intensity = clamp((theta - light.cutOff) / epsilon, 0.0, 1.0);

diffuse  *= intensity;
specular *= intensity;

float distance    = length(light.pos - FragPos);
float attenuation = 1.0 / (light.constant + light.linear * distance + 
light.quadratic * (distance * distance));

ambient  *= attenuation; 
diffuse  *= attenuation;
specular *= attenuation;

    
    	 gl_FragColor = vec4((ambient + diffuse + specular) * diffuseTexel.xyz, 1.0);

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