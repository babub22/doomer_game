uniform vec3 cameraPos;
uniform sampler2D colorMap;
uniform float radius;

varying vec3 vertexToPlayer;

void main(void)
{
    vec3 vertexPos = vec3(gl_Vertex[0], gl_Vertex[1], gl_Vertex[2]);

    vertexToPlayer = cameraPos - vertexPos;

    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}