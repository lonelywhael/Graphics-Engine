#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 texCoord;

uniform mat4 clipMat;

void main() {
    texCoord = aPos;
    vec4 pos = clipMat * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}
