@@DISABLED
@MAIN
&&gen_func
&&
&gen_color
vec4(&&t_func&&, 1.0f)&
@
@@


@@BLUR
@GLOBAL
#define offset 1.0 / 300.0
vec2 offsets[9] = vec2[](
    vec2(-offset,  offset),
    vec2( 0.0f,    offset),
    vec2( offset,  offset),
    vec2(-offset,  0.0f),
    vec2( 0.0f,    0.0f),
    vec2( offset,  0.0f),
    vec2(-offset, -offset),
    vec2( 0.0f,   -offset),
    vec2( offset, -offset)   
);
float kernel[9] = float[](
    1.0 / 16, 2.0 / 16, 1.0 / 16,
    2.0 / 16, 4.0 / 16, 2.0 / 16,
    1.0 / 16, 2.0 / 16, 1.0 / 16  
);
@

@MAIN
&&gen_func
vec3 texels[9];
for(int i = 0; i < 9; i++) texels[i] = vec3(texture(value, texCoord.st + offsets[i]));
vec3 color = vec3(0.0);
for(int i = 0; i < 9; i++) color += texels[i] * kernel[i];&&
&gen_color
vec4(color, 1.0f)&
@
@@


@@DEPTH_MAP
@MAIN
&&gen_func
float depth = texture(value, texCoord).r;&&
&gen_color
vec4(vec3(depth), 1.0)&
@
@@


@@LINEARIZED_DEPTH_MAP
@GLOBAL
#define float near 0.1f;
#define float far 10.0f;
@

@FUNCTIONS
float linearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));
}
@

@MAIN
&&gen_func
float depth = texture(value, texCoord).r;&&
&gen_color
vec4(vec3(linearizeDepth(depth)), 1.0)&
@
@@


@@SHADOW_MAP
@GLOBAL
#define BIAS 0.05
@

@MAIN
&&gen_func
&&
&gen_color
gl_FragCoord.z + (gl_FrontFacing ? BIAS : 0.0)&
@
@@