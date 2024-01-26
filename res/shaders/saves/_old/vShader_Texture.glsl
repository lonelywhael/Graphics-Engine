/* TEXTURE VERTEX SHADER
 * 
 * The texture vertex shader is almost identicle to the 3D vertex shader, but it must pass a texture coordinate to the fragment shader.
 */

#version 330 core

#define N_LIGHTS 4

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoord; // read texture coordinate from the GPU memory

out vec2 TexCoord;  // output the same texture coordinate
out vec3 norm;
out vec3 fragPos;
out vec3 fragPosLightSpace[N_LIGHTS];

uniform mat4 clipMat;
uniform mat4 viewMat;
uniform mat3 normalMat;
uniform mat4 lightMat[N_LIGHTS];

void main()
{
    gl_Position = clipMat * vec4(aPos, 1.0);
    fragPos = vec3(viewMat * vec4(aPos, 1.0));
    norm = normalMat * aNorm;

    for (int i = 0; i < N_LIGHTS; i++) {
        vec4 fpls = lightMat[i] * vec4(aPos, 1.0);
        fragPosLightSpace[i] = 0.5f * (fpls.xyz / fpls.w) + 0.5f;
    }

    TexCoord = aTexCoord;   // outputs texture coordinate without changing anything
}