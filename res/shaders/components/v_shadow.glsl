@@DISABLED
@MAIN
&s_func
&
@
@@

@@SHADOW_MAPPING
@GLOBAL
#define N_LIGHTS 4
@

@OUT
out vec3 fragPosLightSpace[N_LIGHTS];
@

@UNIFORMS
uniform mat4 lightMat[N_LIGHTS];
@

@MAIN
&s_func
for (int i = 0; i < N_LIGHTS; i++) {
    vec4 fpls = lightMat[i] * vec4(aPos, 1.0);
    fragPosLightSpace[i] = 0.5f * (fpls.xyz / fpls.w) + 0.5f;
}&
@
@@