/* DEPTH BUFFER FRAGMENT SHADER
 * 
 * A simple fragment shader that prints out a depth buffer.
 */

#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D depthBuffer;

void main()
{
    float depthValue = texture(depthBuffer, TexCoords).r;
    FragColor = vec4(vec3(depthValue), 1.0);    // translates 1D depth data to black/white spectrum
} 