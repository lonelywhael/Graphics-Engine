@@DISABLED
@STRUCTS
&s_map
&
@

@FUNCTIONS
&&s_shadow
&&
&s_frag_pos
&
&s_func
&
@

@MAIN
&s_frag_pos
&
@
@@


@@SHADOW_MAPPING
@STRUCTS
&s_map
sampler2D shadowMap;&
@

@IN
in vec3 fragPosLightSpace[N_LIGHTS];
@

@FUNCTIONS
&&s_shadow
float getShadow(Light light, vec3 fragPosLS) {
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(light.shadowMap, 0);
    for(int x = -1; x <= 1; ++x) for(int y = -1; y <= 1; ++y) {
        float pcfDepth = texture(light.shadowMap, fragPosLS.xy + vec2(x, y) * texelSize).r;
        shadow += fragPosLS.z > pcfDepth ? 1.0 : 0.0;
    }
    shadow /= 9.0;

    return shadow;
}&&
&s_frag_pos
, vec3 fragPosLS&
&s_func
* ((fragPosLS.z > 1.0f) ? 1.0f : 1.0f - getShadow(light, fragPosLS))&
@

@MAIN
&s_frag_pos
, fragPosLightSpace[i]&
@
@@