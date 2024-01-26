/* LIGHT SOURCE FRAGMENT SHADER
 * 
 * The light source fragment shader simply applies a uniform color to all objects irrespective of lighting.
 */

#version 330 core

out vec4 FragColor;

uniform vec3 color;

void main() {
    FragColor = vec4(color, 1.0f);
}