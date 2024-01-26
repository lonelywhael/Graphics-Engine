#version 330 core

#define EMPTY_LIGHT 0
#define SPOT_LIGHT 1
#define POINT_LIGHT 2
#define DIR_LIGHT 3

#define N_LIGHTS 4

struct Light {
    int type;
    vec3 dir;
    vec3 ambient, diffuse, specular;
    sampler2D shadowMap;
};
struct D_Map {
    sampler2D diffuse;
    vec3 specular;
    float shininess;
};

in vec3 norm;
in vec3 fragPos;
in vec2 texCoord;
in vec3 fragPosLightSpace[N_LIGHTS];

out vec4 FragColor;

uniform sampler2D value;
uniform Light lightList[N_LIGHTS];
uniform D_Map dMap;

vec3 getAmbient(vec3 lightAmbient) {
    vec3 ambientLight = vec3(texture(dMap.diffuse, texCoord)) * lightAmbient;
    return ambientLight;
}
vec3 getDiffuse(vec3 lightDiffuse, vec3 norm, vec3 lightDir) {
    float diffuse = max(dot(norm, lightDir), 0.0);
    vec3 diffuseLight = diffuse * vec3(texture(dMap.diffuse, texCoord)) * lightDiffuse;
    return diffuseLight;
}
vec3 getSpecular(vec3 lightSpecular, vec3 norm, vec3 lightDir) {
    vec3 viewDir = normalize(-fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float specular = pow(max(dot(viewDir, reflectDir), 0.0), dMap.shininess);
    vec3 specularLight = specular * dMap.specular * lightSpecular;
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
    vec3 lightDir = normalize(-light.dir);
    return getAmbient(light.ambient) +
           (getDiffuse(light.diffuse, norm, lightDir) + 
            getSpecular(light.specular, norm, lightDir)) * ((fragPosLS.z > 1.0f) ? 1.0f : 1.0f - getShadow(light, fragPosLS));
}

void main() {
    vec3 nNorm = normalize(norm);
vec3 result = vec3(0.0f);
for (int i = 0; i < N_LIGHTS; i++)
    result += (lightList[i].type != EMPTY_LIGHT) ? getLight(lightList[i], nNorm, fragPosLightSpace[i]) : vec3(0.0f);
 
    FragColor = vec4(result, 1.0f);
}
