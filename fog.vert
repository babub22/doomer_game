uniform vec3 cameraPos;
uniform sampler2D colorMap;
uniform float radius;

varying vec3 vertexToPlayer;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main(void)
{
//    vec3 vertexPos = vec3(aPos[0], aPos[1], aPos[2]);
   // gl_TexCoord[0] = gl_MultiTexCoord0;



   TexCoord = vec2(aTexCoord.x, aTexCoord.y);

    
   // gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

 gl_Position = proj * view  * model * vec4(aPos, 1.0);
    vertexToPlayer = cameraPos - vec3(gl_Position[0], gl_Position[1], gl_Position[2]);
}