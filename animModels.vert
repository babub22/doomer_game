#version 400

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec4 boneIds; 
layout(location = 4) in vec4 weights;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

uniform vec3 discardVert;

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

out vec4 modelPos;

uniform vec3 cameraPos;
uniform sampler2D colorMap; 
uniform float radius; // This is also not used in the vertex shader

void main()
{
	TexCoord = aTexCoord;
    Normal = aNormal;

    FragPos = vec3(model * vec4(aPos, 1.0));

    for (int i = 0; i < dirShadowLightsSize; i++) {
        FragPosLightSpace[i] = lightSpaceMatrix[i] * vec4(FragPos, 1.0f);
    }

    vertexToPlayer = cameraPos - FragPos;

    vec4 animatedPos = vec4(0.0f);

    ivec4 ids = ivec4(boneIds);
    
    mat4 skinMat = weights.x * finalBonesMatrices[ids.x] +
    weights.y * finalBonesMatrices[ids.y] +
    weights.z * finalBonesMatrices[ids.z] +
    weights.w * finalBonesMatrices[ids.w];

    modelPos =  model *
    skinMat * vec4(aPos, 1.0f);
    gl_Position = proj * view * modelPos;
}
