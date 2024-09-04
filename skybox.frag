#version 400
out vec4 FragColor;

in vec3 TexCoords;
in vec3 vertexToPlayer;

uniform samplerCube skybox;
uniform float radius;

void main()
{

float dist = length(vertexToPlayer);
float fogAttenuation = clamp((radius - dist) / radius, 0.0, 1.0);
fogAttenuation = 1.0f;

    vec4 tex = texture(skybox, TexCoords);
gl_FragColor = vec4(tex.rgb * fogAttenuation + (.5 * (1.0-fogAttenuation)), tex.a);
}