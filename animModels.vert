#version 400

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;
layout(location = 5) in ivec4 boneIds; 
layout(location = 6) in vec4 weights;
	
uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
	
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

uniform mat4 lightSpaceMatrix[8];
uniform int dirShadowLightsSize;  
	
out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace[8];

out vec3 vertexToPlayer;

uniform vec3 cameraPos;
uniform sampler2D colorMap;
uniform float radius;

void main()
{
{
TexCoord= aTexCoord;
Normal = aNormal;

FragPos = vec3(model * vec4(aPos, 1.0));

for(int i=0;i<dirShadowLightsSize;i++){
FragPosLightSpace[i] = lightSpaceMatrix[i] * vec4(FragPos,1.0f);
}

vertexToPlayer = cameraPos - FragPos;
}

vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] == -1) 
            continue;
        if(boneIds[i] >=MAX_BONES) 
        {
            totalPosition = vec4(aPos,1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos,1.0f);
        totalPosition += localPosition * weights[i];
        vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * aNormal;
    }
		
    mat4 viewModel = view * model;
    gl_Position =  proj * viewModel * totalPosition;
}