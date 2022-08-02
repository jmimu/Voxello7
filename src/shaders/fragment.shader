#version 330 core
in vec3 ourColor;
in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D textureVox;

void main() {
    //FragColor = vec4(ourColor,1.0);
    //FragColor = vec4(0.0,1.0,0.0,1.0);
    FragColor = texture(textureVox, texCoord);
};
