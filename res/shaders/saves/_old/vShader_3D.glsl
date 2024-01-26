/* 3D VERTEX SHADER
 * 
 * A very simple vertex shader for rendering 3D models without lighting. As such, the only transformation performed is that from model to 
 * clip space.
 */

#version 330 core
// only need a position, normals are for lighting
layout (location = 0) in vec3 aPos;

// need a matrix to transform from model to clip space
uniform mat4 clipMat;

void main()
{
    // transfrom position to clip space, and that's the only step
    gl_Position = clipMat * vec4(aPos, 1.0);
}