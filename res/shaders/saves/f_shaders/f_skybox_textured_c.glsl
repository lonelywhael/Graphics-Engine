#version 330 core

in vec3 texCoord;

out vec4 FragColor;

uniform samplerCube value;

void main() {
    
    FragColor = vec4(vec3(texture(value, texCoord)), 1.0f);
}
