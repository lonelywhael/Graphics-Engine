@@BASIC
@STRUCTS
struct Material {
    vec3 ambient, diffuse, specular;
    float shininess;
};
@

@UNIFORMS
uniform Material material;
@

@FUNCTIONS
&m_ambient
material.ambient&
&m_diffuse
material.diffuse&
&m_specular
material.specular&
&m_shininess
material.shininess&
@

@MAIN
&m_emission
&
@
@@


@@D_MAP
@STRUCTS
struct D_Map {
    &t_type& diffuse;
    vec3 specular;
    float shininess;
};
@

@UNIFORMS
uniform D_Map dMap;
@

@FUNCTIONS
&m_ambient
vec3(texture(dMap.diffuse, texCoord))&
&m_diffuse
vec3(texture(dMap.diffuse, texCoord))&
&m_specular
dMap.specular&
&m_shininess
dMap.shininess&
@

@MAIN
&m_emission
&
@
@@


@@DS_MAP
@STRUCTS
struct DS_Map {
    &t_type& diffuse, specular;
    float shininess;
};
@

@UNIFORMS
uniform DS_Map dsMap;
@

@FUNCTIONS
&m_ambient
vec3(texture(dsMap.diffuse, texCoord))&
&m_diffuse
vec3(texture(dsMap.diffuse, texCoord))&
&m_specular
vec3(texture(dsMap.specular, texCoord))&
&m_shininess
dsMap.shininess&
@

@MAIN
&m_emission
&
@
@@


@@DSE_MAP
@STRUCTS
struct DSE_Map {
    &t_type& diffuse, specular, emission;
    float shininess;
};
@

@UNIFORMS
uniform DSE_Map dseMap;
@

@FUNCTIONS
vec3 getEmission() {
    return (texture(dseMap.specular, texCoord).r == 0.0) ? vec3(texture(dseMap.emission, texCoord)) : vec3(0.0f);
}
&m_ambient
vec3(texture(dseMap.diffuse, texCoord))&
&m_diffuse
vec3(texture(dseMap.diffuse, texCoord))&
&m_specular
vec3(texture(dseMap.specular, texCoord))&
&m_shininess
dseMap.shininess&
@

@MAIN
&m_emission
result += getEmission();&
@
@@