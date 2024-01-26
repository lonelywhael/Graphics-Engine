#version 330 core

in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D value;

void main() {
    
    FragColor = vec4(vec3(texture(value, texCoord)), 1.0f);
}
