#version 330 core
in vec3 ourColor;
in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D textureCol;
//uniform sampler2D textureZbuf;
uniform sampler2D textureNorm;

void main() {
    //FragColor = vec4(ourColor,1.0);
    //FragColor = vec4(0.0,1.0,0.0,1.0);
	vec4 col = texture(textureCol, texCoord);
	//vec4 zbuf = texture(textureZbuf, texCoord);
	//float z = texture(textureZbuf, texCoord).x;
	vec4 n_tmp = texture(textureNorm, texCoord);
	vec3 n = vec3(n_tmp.x-0.5,n_tmp.y-0.5,n_tmp.z-0.5);
	//FragColor = vec4(vec3((n_tmp.y+0.5)*10.0-5.0),1.0);
	FragColor = col*n_tmp.z;
};
