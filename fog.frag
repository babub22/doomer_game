uniform vec3 cameraPos;
uniform sampler2D colorMap;
uniform float radius;

varying vec3 vertexToPlayer;

void main(void)
{
    float dist = length(vertexToPlayer);

    float attenuation = clamp((radius - dist) / radius, 0.0, 1.0);

    vec4 diffuseTexel = texture2D(colorMap, gl_TexCoord[0].st);
    
    vec4 diffuseTerm  = diffuseTexel * attenuation + (.5 * (1.0-attenuation));
    
    diffuseTerm.a = diffuseTexel.a; /* preserve alpha */

    gl_FragColor = diffuseTerm;
}