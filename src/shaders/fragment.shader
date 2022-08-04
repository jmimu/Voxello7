#version 330 core
in vec3 ourColor;
in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D textureCol;
uniform sampler2D textureZbuf;
uniform sampler2D textureNorm;

void main() {
    //FragColor = vec4(ourColor,1.0);
    //FragColor = vec4(0.0,1.0,0.0,1.0);
	vec4 col = texture(textureCol, texCoord);
	//vec4 zbuf = texture(textureZbuf, texCoord);
	float z = texture(textureZbuf, texCoord).r*10.0;
	vec4 n = texture(textureNorm, texCoord);

	FragColor = vec4(vec3(1/z),1.0);
	//FragColor = col;
	//FragColor = n;
};
