/* SKYBOX VERTEX SHADER
 * 
 * The skybox vertex shader prepares special position vectors that can be easily used to render a skybox.
 */

#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 clipMat;

void main()
{
    // pass along texture coordinate
    TexCoords = aPos;
    // get position in clip space
    vec4 pos = clipMat * vec4(aPos, 1.0);
    // make sure the depth value is set to the maximum possible value (1.0)
    gl_Position = pos.xyww;
}  