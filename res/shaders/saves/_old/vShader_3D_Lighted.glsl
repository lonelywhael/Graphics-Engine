/* 3D LIGHTED VERTEX SHADER
 * 
 * This vertex shader is used to render lighted objects in 3D. It requires a matrix that transforms from model space to clip space for the
 * vertices, and two matrices that transform from model to view space for the lighting calculations. The view matrix for normal vectors is
 * simply the inverse transpose of the normal view matrix.
 * 
 * Side note: the reason it makes sense to do the conversion to view space here is that the fragment shader is run significantly more times
 * than the vertex shader (per pixel vs. per vertex). There for, it is beneficial to do something slightly harder here to make something 
 * slightly easier in the fragment shader.
 */

#version 330 core

#define N_LIGHTS 4

// receive information about the position and normals from the GPU memory
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;

// give the transformed position (one for camera and N for lights) and normal for the lighting calculation
out vec3 norm;
out vec3 fragPos;
out vec3 fragPosLightSpace[N_LIGHTS];

// matrices for calculations; best to compute these matrices on the CPU since they only need to be computed once per frame
uniform mat4 clipMat;   // model to clip space
uniform mat4 viewMat;   // model to view space
uniform mat3 normalMat; // inverse transpose of view matrix
uniform mat4 lightMat[N_LIGHTS];

void main() {
    // transform give coordinates to clip space
    gl_Position = clipMat * vec4(aPos, 1.0);
    // also transform them to view space for the sake of the lighting calculation
    fragPos = vec3(viewMat * vec4(aPos, 1.0)); // 3D translations !do not! work in 3 dimensions bc they are not linear (move origin)
    // and the normal
    norm = normalMat * aNorm;

    // need to pass the transformed light positions from the perspective of each light in order to perform the shadow calculation
    for (int i = 0; i < N_LIGHTS; i++) {
        vec4 fpls = lightMat[i] * vec4(aPos, 1.0);
        fragPosLightSpace[i] = 0.5f * (fpls.xyz / fpls.w) + 0.5f;   // normalize before sending to the fragment shader
    }
}