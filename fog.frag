#version 330
uniform vec3 cameraPos;
uniform sampler2D colorMap;
uniform float radius;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

struct PointLight{
vec3  pos;
vec3 color;

float constant;
float linear;
float quadratic;
};

struct DirLight{
vec3  pos;
vec3 color;
vec3 dir;

float constant;
float linear;
float quadratic;

float rad;
float cutOff;	
};

#define MAX_LIGHTS 50
uniform int pointLightsSize;  
uniform PointLight pointLights[MAX_LIGHTS];

uniform int dirLightsSize;  
uniform DirLight dirLights[MAX_LIGHTS];

in vec3 vertexToPlayer;

float ambientC = .05f;
float specularC = .2f;	

vec3 pointLightCalc(PointLight light, vec3 norm, vec3 viewDir){
vec3 lightDir = normalize(light.pos - FragPos);
// diffuse shading
float diff = max(dot(norm, lightDir), 0.0);
// specular shading

vec3 reflectDir = reflect(-lightDir, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

// attenuation
float distance    = length(light.pos - FragPos);
float attenuation = 1.0 / (light.constant + light.linear * distance + 
light.quadratic * (distance * distance));    

// combine results
vec3 ambient  = ambientC  * light.color;
vec3 diffuse  = diff * light.color;
vec3 specular = specularC * spec * light.color;

ambient  *= attenuation;
diffuse  *= attenuation;
specular *= attenuation;

return (ambient + diffuse + specular);
}

vec3 dirLightCalc(DirLight light, vec3 norm, vec3 viewDir){
vec3 lightDir = normalize(light.pos - FragPos);
// diffuse shading
float diff = max(dot(norm, lightDir), 0.0);
// specular shading
vec3 reflectDir = reflect(-lightDir, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

// combine results
vec3 ambient  = ambientC  * light.color;
vec3 diffuse  = diff * light.color;
vec3 specular = specularC * spec * light.color;


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

return (ambient + diffuse + specular);

}  

void main(void){
vec3 viewDir = normalize(cameraPos - FragPos);
vec3 norm = normalize(Normal);

vec3 res;
vec3 tex = texture2D(colorMap, TexCoord).rgb;

for(int i=0;i<dirLightsSize;i++){
res+= dirLightCalc(dirLights[i], norm, viewDir);
}

for(int i=0;i<pointLightsSize;i++){
res+= pointLightCalc(pointLights[i], norm, viewDir);
}

//gl_FragColor = vec4(res * tex,1.0f);

float dist = length(vertexToPlayer);
float fogAttenuation = clamp((radius - dist) / radius, 0.0, 1.0);

gl_FragColor = vec4(res * tex * fogAttenuation + (.5 * (1.0-fogAttenuation)),1.0f);
}
/*


if(diffuseTexel.a == 0.0){
		  discard;
}

diffuseTerm.a = diffuseTexel.a; 

gl_FragColor = diffuseTerm;
*/