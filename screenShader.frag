#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

uniform float dof;
uniform float time;	

float random(vec2 co) {
return fract( sin( co.x * 3433.8 + co.y * 3843.98 ) * 45933.8 );
}

void main()
{
vec2 uv = gl_FragCoord.xy / vec2(textureSize(screenTexture, 0));

float n = random(uv * time);

gl_FragColor = texture(screenTexture, TexCoords);
float gray = dot(gl_FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));

vec3 grayscale = mix(gl_FragColor.rgb, vec3(gray),dof);

gl_FragColor = vec4(mix(grayscale, vec3(n), dof/2.0f),1.0f);
}