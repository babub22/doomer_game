#version 400
uniform vec3 cameraPos;

uniform sampler2D colorMap;
uniform sampler2D shadowMap;
//uniform samplerCubeArray depthMapsArray;

uniform float radius;

uniform vec3 lightPoss;	

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

struct PointLight{
vec3  pos;
vec3 color;

float constant;
float linear;
float quadratic;

int cubemapIndex;

vec3 dir;
float rad;
float cutOff;	
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

uniform int shadowPointLightsSize;  
uniform PointLight shadowPointLights[MAX_LIGHTS];

uniform int dirLightsSize;  
uniform PointLight dirLights[MAX_LIGHTS];

in vec3 vertexToPlayer;

float ambientC = .25f;
float specularC = .2f;

uniform float far_plane;

vec3 gridSamplingDisk[20] = vec3[]
(
vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);


float shadowCalc(vec3 lightPos, vec3 norm)
{
    // perform perspective divide
    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;

    vec3 lightDir = normalize(lightPos - FragPos);
    float bias = max(0.05 * (1.0 - dot(norm, lightDir)), 0.005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}	

/*
float shadowCalc(vec3 lightDir, vec3 lightPos, int cubemapIndex){

vec3 fragToLight = FragPos - lightPos;
float currentDepth = length(fragToLight);

float shadow = 0.0;
float bias = 0.15; // .15
int samples = 20;
float viewDistance = length(cameraPos - FragPos);
float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
for(int i = 0; i < samples; ++i)
{
float closestDepth = texture(depthMapsArray, vec4(fragToLight + gridSamplingDisk[i] * diskRadius, cubemapIndex)).r;
closestDepth *= far_plane;   // undo mapping [0;1]
if(currentDepth - bias > closestDepth)
shadow += 1.0;
}
shadow /= float(samples);



//gl_FragColor = vec4(res * color,tex.a);

return shadow;
}*/


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
vec3 ambient  = ambientC * light.color;
vec3 diffuse  = diff * light.color;
vec3 specular = specularC * spec * light.color;

ambient  *= attenuation;
diffuse  *= attenuation;
specular *= attenuation;

//ambient += (1.0 - shadow);

return (ambient + (diffuse + specular));
}

vec3 pointLightCalcShadow(PointLight light, vec3 norm, vec3 viewDir){
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
vec3 ambient  = ambientC * light.color;
vec3 diffuse  = diff * light.color;
vec3 specular = specularC * spec * light.color;

ambient  *= attenuation;
diffuse  *= attenuation;
specular *= attenuation;

//float shadow = shadowCalc(light.pos, light.cubemapIndex);

//ambient += (1.0 - shadow);

return (ambient + (diffuse + specular));// * (1.0 - shadow));
}


vec3 dirLightCalc(PointLight light, vec3 norm, vec3 viewDir){
vec3 lightDir = normalize(light.pos - FragPos);
// diffuse shading
float diff = max(dot(norm, lightDir), 0.0);
// specular shading
vec3 reflectDir = reflect(-lightDir, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

// combine results
vec3 ambient  = ambientC * light.color;
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

float shadow = shadowCalc(light.pos, norm);

return (ambient + (1.0f - shadow) * (diffuse + specular));
} 

void main(void){
vec4 tex = texture2D(colorMap, TexCoord);
vec3 color = tex.rgb;	

if(tex.a == 0.0){
discard;
}

vec3 viewDir = normalize(cameraPos - FragPos);
vec3 norm = normalize(Normal);

vec3 res;
//res = vec3(ambientC * vec3(112.0f / 255.f, 117.0f, 113.0f) );

for(int i=0;i<dirLightsSize;i++){
res+= dirLightCalc(dirLights[i], norm, viewDir);
}

// shadows;
/*for(int i=0;i<shadowPointLightsSize;i++){
res+= pointLightCalcShadow(shadowPointLights[i], norm, viewDir);
}

for(int i=0;i<pointLightsSize;i++){
res+= pointLightCalc(pointLights[i], norm, viewDir);
}*/

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