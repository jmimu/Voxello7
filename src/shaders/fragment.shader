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
	//float z = 1/(10*sqrt(texture(textureZbuf, texCoord).r));
	float z = 1/(10*(texture(textureZbuf, texCoord).r));
	z = clamp(z,0,1);
	vec4 n = 2*texture(textureNorm, texCoord)-1;


	FragColor = vec4(col.rgb*z,1.0)*-n.y;
	//FragColor = col;
	//FragColor = n;
};
