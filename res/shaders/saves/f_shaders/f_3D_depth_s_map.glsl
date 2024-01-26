#version 330 core

#define BIAS 0.05

void main() {
    gl_FragDepth = gl_FragCoord.z + (gl_FrontFacing ? BIAS : 0.0);
}
