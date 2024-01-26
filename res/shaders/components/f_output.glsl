@@GENERAL
@MAIN
void main() {
    &&gen_func&&
    &&o_output&&
}
@
@@

@@COLOR_BUFFER
@OUT
out vec4 FragColor;
@

@UNIFORMS
uniform &t_type& value;
@

@MAIN
&&o_output
FragColor = &gen_color&;&&
@
@@


@@DEPTH_BUFFER
@MAIN
&&o_output
gl_FragDepth = &gen_color&;&&
@
@@