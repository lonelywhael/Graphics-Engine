/* DS MAP FRAGMENT SHADER
 * 
 * Almost identical to the basic material shader, this shader only uses texture values instead of color maps.
 */

#version 330 core

#define EMPTY_LIGHT 0
#define SPOT_LIGHT 1
#define POINT_LIGHT 2
#define DIR_LIGHT 3

#define N_LIGHTS 4

struct DS_Map {
    sampler2D diffuse, specular;
    float shininess;
};
struct Light {
    int type;
    vec3 pos, dir;
    vec3 ambient, diffuse, specular;
    float constant, linear, quadratic;
    float inner, outer;
    sampler2D shadowMap;
};

in vec2 TexCoord;
in vec3 norm;

in vec3 fragPos;
in vec3 fragPosLightSpace[N_LIGHTS];

out vec4 FragColor;

uniform DS_Map dsMap;
uniform Light lightList[N_LIGHTS];

vec3 getAmbient(float c, vec3 lightAmbient) {
    vec3 ambientLight = c * vec3(texture(dsMap.diffuse, TexCoord)) * lightAmbient;                  // from texture
    return ambientLight;
}
vec3 getDiffuse(float c, vec3 lightDiffuse, vec3 nNorm, vec3 lightDir) {
    float diffuse = max(dot(nNorm, lightDir), 0.0);
    vec3 diffuseLight = c * diffuse * vec3(texture(dsMap.diffuse, TexCoord)) * lightDiffuse;        // from texture
    return diffuseLight;
}
vec3 getSpecular(float c, vec3 lightSpecular, vec3 nNorm, vec3 lightDir) {
    vec3 viewDir = normalize(-fragPos);
    vec3 reflectDir = reflect(-lightDir, nNorm);
    float specular = pow(max(dot(viewDir, reflectDir), 0.0), dsMap.shininess);
    vec3 specularLight = c * specular * vec3(texture(dsMap.specular, TexCoord)) * lightSpecular;    // from texture
    return specularLight;
}
float getShadow(Light light, vec3 fragPosLS) {
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(light.shadowMap, 0);
    for(int x = -1; x <= 1; ++x) for(int y = -1; y <= 1; ++y) {
        float pcfDepth = texture(light.shadowMap, fragPosLS.xy + vec2(x, y) * texelSize).r; 
        shadow += fragPosLS.z > pcfDepth ? 1.0 : 0.0;        
    }
    shadow /= 9.0;

    return shadow;
}

vec3 getLight(Light light, vec3 norm, vec3 fragPosLS) {
    float distance = length(light.pos - fragPos);
    float attenuation = (light.linear + light.quadratic > 0) ? 
                            1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)) : 
                            1.0f;

    vec3 lightDir = (light.type == DIR_LIGHT) ? 
                        normalize(-light.dir) : 
                        normalize(light.pos - fragPos);

    float theta = (light.type == SPOT_LIGHT) ? 
                        dot(lightDir, normalize(-light.dir)) : 
                        1.0f;
    float intensity = (light.type == SPOT_LIGHT) ? 
                            clamp((theta - light.outer) / (light.inner- light.outer), 0.0, 1.0) : 
                            1.0f;

    return getAmbient(attenuation, light.ambient) +
           ((theta >= light.outer) ?
                getDiffuse(intensity * attenuation, light.diffuse, norm, lightDir) + 
                    getSpecular(intensity * attenuation, light.specular, norm, lightDir) : 
                vec3(0.0f))
            * ((fragPosLS.z > 1.0f) ? 1.0f : 1.0f - getShadow(light, fragPosLS));
}

void main() {
    vec3 nNorm = normalize(norm);

    vec3 result = vec3(0.0f);
    for (int i = 0; i < N_LIGHTS; i++) 
        result += (lightList[i].type != EMPTY_LIGHT) ? getLight(lightList[i], nNorm, fragPosLightSpace[i]) : vec3(0.0f);

    FragColor = vec4(result, 1.0f);
}