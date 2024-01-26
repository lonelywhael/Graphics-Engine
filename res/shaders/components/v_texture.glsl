@@DISABLED
@IN
&t_in
&
@

@MAIN
&t_func
&
@
@@


@@BASIC_2D
@IN
&t_in
layout (location = $) in vec2 aTexCoord;&
@

@OUT
out vec2 texCoord;
@

@MAIN
&t_func
texCoord = aTexCoord;&
@
@@


@@CUBE
@IN
&t_in
layout (location = $) in vec3 aTexCoord;&
@

@OUT
out vec3 texCoord;
@

@MAIN
&t_func
texCoord = aTexCoord;&
@
@@