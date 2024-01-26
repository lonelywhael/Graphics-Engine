@@BASIC_2D
@IN
layout (location = $) in vec2 aPos;
&t_in&
@

@MAIN
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    &t_func& 
}
@
@@


@@BASIC_3D
@IN
layout (location = $) in vec3 aPos;
&t_in&
@

@UNIFORMS
uniform mat4 clipMat;
@

@MAIN
void main() {
    gl_Position = clipMat * vec4(aPos, 1.0);
    &t_func&
}
@
@@


@@LIGHTING_3D
@IN
layout (location = $) in vec3 aPos;
layout (location = $) in vec3 aNorm;
&t_in&
@

@OUT
out vec3 norm;
out vec3 fragPos;
@

@UNIFORMS
uniform mat4 clipMat;
uniform mat4 viewMat;
uniform mat3 normalMat;
@

@MAIN
void main() {
    gl_Position = clipMat * vec4(aPos, 1.0);
    fragPos = vec3(viewMat * vec4(aPos, 1.0));
    norm = normalMat * aNorm;
    &s_func&
    &t_func&
}
@
@@


@@SKYBOX
@IN
layout (location = 0) in vec3 aPos;
@

@UNIFORMS
uniform mat4 clipMat;
@

@MAIN
void main() {
    texCoord = aPos;
    vec4 pos = clipMat * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}
@
@@