#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

uniform float dof;
uniform float time;	

float random(vec2 co) {
return fract( sin( co.x * 3433.8 + co.y * 3843.98 ) * 45933.8 );
}

int dithering_pattern(ivec2 fragcoord) {
	const int pattern[] = {
		-4, +0, -3, +1, 
		+2, -2, +3, -1, 
		-3, +1, -4, +0, 
		+3, -1, +2, -2
	};
	
	int x = fragcoord.x % 4;
	int y = fragcoord.y % 4;
	
	return pattern[y * 4 + x];
}


const int color_depth = 5;

void main()
{
/*
ivec2 uv = ivec2(gl_FragCoord.xy / vec2(textureSize(screenTexture, 0)));
vec3 color = texture(screenTexture, TexCoords).rgb; */

int resolution_scale = 3;

ivec2 uv = ivec2(gl_FragCoord.xy / float(resolution_scale));
vec3 color = texelFetch(screenTexture, uv * resolution_scale, 0).rgb;

ivec3 c = ivec3(round(color * 255.0));
c += ivec3(dithering_pattern(uv));

c >>= (8 - color_depth);	

gl_FragColor = vec4(vec3(c) / float(1 << color_depth), 1.0f);

/*
float n = random(uv * time);
float gray = dot(c, vec3(0.2126, 0.7152, 0.0722));
vec3 grayscale = mix(c, vec3(gray),dof);
gl_FragColor = vec4(mix(grayscale,vec3(n), dof/2.0f),1.0f);
*/
//gl_FragColor = texture(screenTexture, TexCoords);

//vec3 grayscale = mix(gl_FragColor.rgb, vec3(gray),dof);

//gl_FragColor = vec4(mix(grayscale, vec3(n), dof/2.0f),1.0f);
}