/* SKYBOX FRAGMENT SHADER
 * 
 * The skybox fragment shader reads texture data from a cube texture.
 */

#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube cubeMap;

void main() {
    FragColor = texture(cubeMap, TexCoords);
}