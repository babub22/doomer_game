#version 400

layout(location = 0) in vec3 aPos;

uniform vec3 cameraPos;

out vec4 FragPosLightSpace[8];
out vec3 vertexToPlayer;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform mat4 lightSpaceMatrix[8];
uniform int dirShadowLightsSize;  

void main(void){
FragPos = vec3(model * vec4(aPos, 1.0));
gl_Position = proj * view  * model * vec4(aPos, 1.0);

for(int i=0;i<dirShadowLightsSize;i++){
FragPosLightSpace[i] = lightSpaceMatrix[i] * vec4(FragPos,1.0f);
}

vertexToPlayer = cameraPos - FragPos;
}