#version 330 core
in vec3 ourColor;
in vec2 texCoord;

out vec4 FragColor;

uniform vec3 sunDir;
uniform sampler2D textureCol;
uniform sampler2D textureZbuf;
uniform sampler2D textureNorm;

const float ambiant = 0.2;

void main() {
	vec4 col = texture(textureCol, texCoord);
	float z = 1/(10*(texture(textureZbuf, texCoord).r));
	z = clamp(z,0,1);
	vec4 n = 2*texture(textureNorm, texCoord)-1;

	//FragColor = vec4(ourColor,1.0);
    //FragColor = vec4(0.0,1.0,0.0,1.0);
	//vec4 zbuf = texture(textureZbuf, texCoord);
	//float z = 1/(10*sqrt(texture(textureZbuf, texCoord).r));
	//vec4 n = texture(textureNorm, texCoord);
	float light = clamp(dot(vec3(n),sunDir)*2,0.5,2);
	FragColor = vec4(col.rgb*z,1.0)*light;
	//FragColor = col+sunDir+n+z;
	//FragColor = n;
	//FragColor = vec4(sunDir,1,1,1);
	//FragColor = vec4(sunDir.b,0.0,texCoord.x,1.0);
};
