#version 400
uniform vec3 cameraPos;

uniform sampler2D colorMap;
uniform sampler2DArray shadowMap;

uniform float radius;

uniform vec3 lightPoss;	

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace[8];

struct PointLight{
vec3  pos;
vec3 color;

float constant;
float linear;
float quadratic;

vec3 dir;
float rad;
float cutOff;	
};

#define MAX_LIGHTS 10

uniform int pointLightsSize;  
uniform PointLight pointLights[MAX_LIGHTS];

uniform int dirShadowLightsSize;  
uniform PointLight dirShadowLights[MAX_LIGHTS];

uniform int dirLightsSize;  
uniform PointLight dirLights[MAX_LIGHTS];

in vec3 vertexToPlayer;

float ambientC = .25f;
float specularC = .2f;

//uniform float far_plane;

float shadowCalc(int depthTxId,vec3 lightPos, vec3 lightDir, vec3 norm)
{
vec3 projCoords = FragPosLightSpace[depthTxId].xyz / FragPosLightSpace[depthTxId].w;
projCoords = projCoords * 0.5 + 0.5;
float closestDepth = texture(shadowMap, vec3(projCoords.xy, depthTxId)).r; 
float currentDepth = projCoords.z;

float bias = 0.0005f;//max(0.05 * (1.0 - dot(norm, lightDir)), 0.005);  
float shadow = 0.0f;//currentDepth - bias > closestDepth  ? 1.0 : 0.0;

//return currentDepth - bias > closestDepth  ? 1.0 : 0.0;

vec3 texelSize = 1.0 / textureSize(shadowMap, 0);

for(int x = -1; x <= 1; ++x)
{
for(int y = -1; y <= 1; ++y)
{
float pcfDepth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize.xy, depthTxId)).r; 
shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
}    
}
shadow /= 9.0;

if(projCoords.z > 1.0)
shadow = 0.0;

return shadow;
}	

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
vec3 ambient  = .05f * light.color;
vec3 diffuse  = diff * light.color;
vec3 specular = specularC * spec * light.color;

ambient  *= attenuation;
diffuse  *= attenuation;
specular *= attenuation;


return (ambient + (diffuse + specular));
}

vec3 dirShadowLightCalc(int lightId,PointLight light, vec3 norm, vec3 viewDir){
vec3 lightDir = normalize(light.pos - FragPos);
float theta = dot(lightDir, normalize(-light.dir));

//if(theta>light.cutOff){
float diff = max(dot(norm, lightDir), 0.0);

vec3 reflectDir = reflect(-lightDir, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

vec3 ambient  = ambientC * light.color;
vec3 diffuse  = diff * light.color;
vec3 specular = specularC * spec * light.color;

//float theta = dot(lightDir, normalize(-light.dir)); 
float epsilon = (light.rad - light.cutOff);
float intensity = clamp((theta - light.cutOff) / epsilon, 0.0, 1.0);
diffuse  *= intensity;
specular *= intensity;

float distance    = length(light.pos - FragPos);
float attenuation = 1.0 / (light.constant + light.linear * distance + 
light.quadratic * (distance * distance));

ambient *= attenuation;
diffuse *= attenuation;
specular *= attenuation;

float shadow = shadowCalc(lightId, light.pos, lightDir, norm);

return 	(ambient + (1.0f - shadow) * (diffuse + specular));
} 


vec3 dirLightCalc(PointLight light, vec3 norm, vec3 viewDir){
vec3 lightDir = normalize(light.pos - FragPos);
float theta = dot(lightDir, normalize(-light.dir));

//if(theta>light.cutOff){
float diff = max(dot(norm, lightDir), 0.0);

vec3 reflectDir = reflect(-lightDir, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

vec3 ambient  = ambientC * light.color;
vec3 diffuse  = diff * light.color;
vec3 specular = specularC * spec * light.color;

//float theta = dot(lightDir, normalize(-light.dir)); 
float epsilon = (light.rad - light.cutOff);
float intensity = clamp((theta - light.cutOff) / epsilon, 0.0, 1.0);
diffuse  *= intensity;
specular *= intensity;

float distance    = length(light.pos - FragPos);
float attenuation = 1.0 / (light.constant + light.linear * distance + 
light.quadratic * (distance * distance));

ambient *= attenuation;
diffuse *= attenuation;
specular *= attenuation;

return 	(ambient + (1.0f - 0.0f) * (diffuse + specular));
} 


void main(void){
vec4 tex = texture(colorMap, TexCoord);
vec3 color = tex.rgb;	

if(tex.a == 0.0){
discard;
}

//if(tex.a != 1.0f){
//discard;
//}

vec3 viewDir = normalize(cameraPos - FragPos);
vec3 norm = normalize(Normal);

vec3 res;

for(int i=0;i<dirShadowLightsSize;i++){
res+= dirShadowLightCalc(i, dirShadowLights[i], norm, viewDir);
}

for(int i=0;i<dirLightsSize;i++){
res+= dirLightCalc(dirLights[i], norm, viewDir);
}

for(int i=0;i<pointLightsSize;i++){
res+= pointLightCalc(pointLights[i], norm, viewDir);
}

float dist = length(vertexToPlayer);
float fogAttenuation = clamp((radius - dist) / radius, 0.0, 1.0);

//gl_FragColor = vec4(res * color,tex.a);

gl_FragColor = vec4(res * color * fogAttenuation + (.5 * (1.0-fogAttenuation)),tex.a);
}

/*


if(diffuseTexel.a == 0.0){
		  discard;
}

diffuseTerm.a = diffuseTexel.a; 

gl_FragColor = diffuseTerm;
*/