/* LINEARIZED DEPTH BUFFER FRAGMENT SHADER
 * 
 * The linearized depth buffer fragment shader renders depth values from a depth buffer texture as colors but applies a linearization to
 * to the depth values to make sure there are in a visible range first.
 */

#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D depthBuffer;

#define float near 0.1f;
#define float far 10.0f;

float linearizeDepth(float depth) {
    // depth values are normally non-linear, make them linear again
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{             
    float depthValue = texture(depthBuffer, TexCoords).r;
    FragColor = vec4(vec3(linearizeDepth(depthValue) / far), 1.0); // perspective
}  