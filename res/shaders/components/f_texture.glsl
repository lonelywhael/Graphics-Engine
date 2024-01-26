@@DISABLED

@UNIFORMS
&t_type
vec3&
@

@MAIN
&&t_func
value&&
@
@@

@@BASIC_2D
@STRUCTS
&t_type
sampler2D&
@

@IN
in vec2 texCoord;
@

@UNIFORMS
&t_type
sampler2D&
@

@MAIN
&&t_func
vec3(texture(value, texCoord))&&
@
@@

@@CUBE
@STRUCTS
&t_type
samplerCube&
@

@IN
in vec3 texCoord;
@

@UNIFORMS
&t_type
samplerCube&
@

@MAIN
&&t_func
vec3(texture(value, texCoord))&&
@
@@