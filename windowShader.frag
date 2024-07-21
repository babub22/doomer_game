#version 400

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D colorMap;

void main(void){
     vec4 tex;// = texture(colorMap, TexCoord);	
     //vec3 color = vec3(0,0,0);// / 255.0f;

     //gl_FragColor = vec4(color, 0.3f);

  //   if(tex.a == 0){
	vec3 color = vec3(0,0,0);
	tex.rgb = color;
	tex.a = 0.3f;
//     }
     
     gl_FragColor = tex;
}
