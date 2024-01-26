/* COLOR BUFFER FRAGMENT SHADER
 * 
 * A simple fragment shader that simply prints out a color buffer.
 */

#version 330 core

out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D colorBuffer;

void main() { 
    FragColor = texture(colorBuffer, TexCoords);
}