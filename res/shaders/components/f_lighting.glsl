@@GENERAL
@GLOBAL
#define EMPTY_LIGHT 0
#define DIR_LIGHT 1
#define POINT_LIGHT 2
#define SPOT_LIGHT 3

#define N_LIGHTS 4
@

@UNIFORMS
uniform Light lightList[N_LIGHTS];
@

@FUNCTIONS
&&l_ambient
vec3 getAmbient(vec3 lightAmbient) {
    vec3 ambientLight = &m_ambient& * lightAmbient;
    return ambientLight;
}&&
&&l_diffuse
vec3 getDiffuse(vec3 lightDiffuse, vec3 norm, vec3 lightDir) {
    float diffuse = max(dot(norm, lightDir), 0.0);
    vec3 diffuseLight = diffuse * &m_diffuse& * lightDiffuse;
    return diffuseLight;
}&&
&&l_specular
vec3 getSpecular(vec3 lightSpecular, vec3 norm, vec3 lightDir) {
    vec3 viewDir = normalize(-fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float specular = pow(max(dot(viewDir, reflectDir), 0.0), &m_shininess&);
    vec3 specularLight = specular * &m_specular& * lightSpecular;
    return specularLight;
}&&
&&l_ambient_c
vec3 getAmbient(float c, vec3 lightAmbient) {
    vec3 ambientLight = c * &m_ambient& * lightAmbient;
    return ambientLight;
}&&
&&l_diffuse_c
vec3 getDiffuse(float c, vec3 lightDiffuse, vec3 norm, vec3 lightDir) {
    float diffuse = max(dot(norm, lightDir), 0.0);
    vec3 diffuseLight = c * diffuse * &m_diffuse& * lightDiffuse;
    return diffuseLight;
}&&
&&l_specular_c
vec3 getSpecular(float c, vec3 lightSpecular, vec3 norm, vec3 lightDir) {
    vec3 viewDir = normalize(-fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float specular = pow(max(dot(viewDir, reflectDir), 0.0), &m_shininess&);
    vec3 specularLight = c * specular * &m_specular& * lightSpecular;
    return specularLight;
}&&
&l_dist
vec3 lightDir = normalize(light.pos - fragPos);&
&l_dist_d
vec3 lightDir = normalize(-light.dir);&
&l_dist_m
vec3 lightDir = (light.type == DIR_LIGHT) ? 
                        normalize(-light.dir) :
                        normalize(light.pos - fragPos);&
&l_attenuation
float distance = length(light.pos - fragPos);
    float attenuation = (light.linear + light.quadratic > 0) ?
                            1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)) : 
                            1.0f;&
&l_spot
float theta = dot(lightDir, normalize(-light.dir));
    float intensity = clamp((theta - light.outer) / (light.inner - light.outer), 0.0, 1.0);&
&l_spot_m
float theta = (light.type == SPOT_LIGHT) ? 
                        dot(lightDir, normalize(-light.dir)) :
                        1.0f;
    float intensity = (light.type == SPOT_LIGHT) ?
                            clamp((theta - light.outer) / (light.inner - light.outer), 0.0, 1.0) : 
                            1.0f;&
&l_func
getAmbient(light.ambient) +
           (getDiffuse(light.diffuse, norm, lightDir) + 
            getSpecular(light.specular, norm, lightDir))&
&l_func_c
getAmbient(attenuation, light.ambient) +
           (getDiffuse(attenuation, light.diffuse, norm, lightDir) + 
            getSpecular(attenuation, light.specular, norm, lightDir))&
&l_func_m
getAmbient(attenuation, light.ambient) +
           ((theta >= light.outer) ?
                getDiffuse(intensity * attenuation, light.diffuse, norm, lightDir) + 
                getSpecular(intensity * attenuation, light.specular, norm, lightDir) : 
                vec3(0.0f))&
@

@MAIN
&&gen_func
vec3 nNorm = normalize(norm);
vec3 result = vec3(0.0f);
for (int i = 0; i < N_LIGHTS; i++)
    result += (lightList[i].type != EMPTY_LIGHT) ? getLight(lightList[i], nNorm&s_frag_pos&) : vec3(0.0f);
&m_emission& &&
&gen_color
vec4(result, 1.0f)&
@
@@

@@POINT
@STRUCTS
struct Light {
    int type;
    vec3 pos;
    vec3 ambient, diffuse, specular;
    float constant, linear, quadratic;
    &s_map&
};
@

@FUNCTIONS
&&l_ambient_c&&
&&l_diffuse_c&&
&&l_specular_c&&
&&s_shadow&&
vec3 getLight(Light light, vec3 norm&s_frag_pos&) {
    &l_dist&
    &l_attenuation&
    return &l_func_c& &s_func&;
}
@
@@

@@SPOT
@STRUCTS
struct Light {
    int type;
    vec3 pos, dir;
    vec3 ambient, diffuse, specular;
    float constant, linear, quadratic;
    float inner, outer;
    &s_map&
};
@

@FUNCTIONS
&&l_ambient_c&&
&&l_diffuse_c&&
&&l_specular_c&&
&&s_shadow&&
vec3 getLight(Light light, vec3 norm&s_frag_pos&) {
    &l_dist&
    &l_attenuation&
    &l_spot&
    return &l_func_m& &s_func&;
}
@
@@

@@DIR
@STRUCTS
struct Light {
    int type;
    vec3 dir;
    vec3 ambient, diffuse, specular;
    &s_map&
};
@

@FUNCTIONS
&&l_ambient&&
&&l_diffuse&&
&&l_specular&&
&&s_shadow&&
vec3 getLight(Light light, vec3 norm&s_frag_pos&) {
    &l_dist_d&
    return &l_func& &s_func&;
}
@
@@

@@POINT_SPOT
@STRUCTS
struct Light {
    int type;
    vec3 pos, dir;
    vec3 ambient, diffuse, specular;
    float constant, linear, quadratic;
    float inner, outer;
    &s_map&
};
@

@FUNCTIONS
&&l_ambient_c&&
&&l_diffuse_c&&
&&l_specular_c&&
&&s_shadow&&
vec3 getLight(Light light, vec3 norm&s_frag_pos&) {
    &l_dist&
    &l_attenuation&
    &l_spot_m&
    return &l_func_m& &s_func&;
}
@
@@

@@DIR_POINT
@STRUCTS
struct Light {
    int type;
    vec3 pos, dir;
    vec3 ambient, diffuse, specular;
    float constant, linear, quadratic;
    &s_map&
};
@

@FUNCTIONS
&&l_ambient_c&&
&&l_diffuse_c&&
&&l_specular_c&&
&&s_shadow&&
vec3 getLight(Light light, vec3 norm&s_frag_pos&) {
    &l_dist_m&
    &l_attenuation&
    return &l_func_c& &s_func&;
}
@
@@

@@DIR_SPOT
@STRUCTS
struct Light {
    int type;
    vec3 pos, dir;
    vec3 ambient, diffuse, specular;
    float constant, linear, quadratic;
    float inner, outer;
    &s_map&
};
@

@FUNCTIONS
&&l_ambient_c&&
&&l_diffuse_c&&
&&l_specular_c&&
&&s_shadow&&
vec3 getLight(Light light, vec3 norm&s_frag_pos&) {
    &l_dist_m&
    &l_attenuation&
    &l_spot_m&
    return &l_func_m& &s_func&;
}
@
@@

@@ALL_ENABLED
@STRUCTS
struct Light {
    int type;
    vec3 pos, dir;
    vec3 ambient, diffuse, specular;
    float constant, linear, quadratic;
    float inner, outer;
    &s_map&
};
@

@FUNCTIONS
&&l_ambient_c&&
&&l_diffuse_c&&
&&l_specular_c&&
&&s_shadow&&
vec3 getLight(Light light, vec3 norm&s_frag_pos&) {
    &l_dist_m&
    &l_attenuation&
    &l_spot_m&
    return &l_func_m& &s_func&;
}
@
@@