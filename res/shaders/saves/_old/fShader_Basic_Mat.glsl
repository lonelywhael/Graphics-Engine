/* BASIC MATERIAL FRAGMENT SHADER
 * 
 * This shader renders objects colored using basic materials with lighting in 3 dimensions. The shader can also handle a number of lights,
 * which can be of any type. 
 */

#version 330 core // version 3.3 with core profile

// makeshift enum for light types
#define EMPTY_LIGHT 0
#define SPOT_LIGHT 1
#define POINT_LIGHT 2
#define DIR_LIGHT 3

#define N_LIGHTS 4

struct Material {
    vec3 ambient, diffuse, specular;    // rgb values for ambient, diffuse, and specular lighting
    float shininess;                    // shininess variable for specular lighting
};
struct Light {
    int type;                           // type of light
    vec3 pos, dir;                      // spatial parameters; defaults to (0, 0, 0) and (0, 0, 0)
    vec3 ambient, diffuse, specular;    // color parameters; defaults to black
    float constant, linear, quadratic;  // attenuation parameters; defaults to constant function (1, 0, 0)
    float inner, outer;                 // spotlight parameters; default to 0, 0
    sampler2D shadowMap;                // need a shadow map texture in order to do shadow calculations
};

// takes in a normal vector and a position vector
in vec3 norm;

// need the position of the fragment in view space, as well as the position of the fragment in the view space of each of the lights (for shadows)
in vec3 fragPos;
in vec3 fragPosLightSpace[N_LIGHTS];

// outputs a color (RGBA)
out vec4 FragColor;

// needs a material from the CPU and a list of lights
uniform Material material;
///TODO: In theory the following line could be left incomplete and dynamically specified number could be inserted by the CPU program.
uniform Light lightList[N_LIGHTS];

vec3 getAmbient(float c, vec3 lightAmbient) {
    // ambient light value is the attenuation parameter * the material ambient color * the light ambient color
    vec3 ambientLight = c * material.ambient * lightAmbient;
    return ambientLight;
}
vec3 getDiffuse(float c, vec3 lightDiffuse, vec3 norm, vec3 lightDir) {
    // calculate the dot of the normal vector and the direction of the light. if negative, the the face is occluded
    float diffuse = max(dot(norm, lightDir), 0.0);
    // attenuation * spot intensity * material diffuse color * light diffuse color
    vec3 diffuseLight = c * diffuse * material.diffuse * lightDiffuse;
    return diffuseLight;
}
vec3 getSpecular(float c, vec3 lightSpecular, vec3 norm, vec3 lightDir) {
    // calculating how close the angle of reflection is to the vector from the light to the object
    vec3 viewDir = normalize(-fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    // raise to the power of shininess to affect how concentrated the shine is
    float specular = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation * intensity * material specular color * light specular color
    vec3 specularLight = c * specular * material.specular * lightSpecular;  
    return specularLight;
}
float getShadow(Light light, vec3 fragPosLS) {
    // treat shadow value as an accumulator and perform kernal operation to blend shadow
    float shadow = 0.0;
    // set texelSize to be one pixel of the shadow map texture
    vec2 texelSize = 1.0 / textureSize(light.shadowMap, 0);
    // iterate through surrounding pixels
    for(int x = -1; x <= 1; ++x) for(int y = -1; y <= 1; ++y) {
        float pcfDepth = texture(light.shadowMap, fragPosLS.xy + vec2(x, y) * texelSize).r;
        // if the depth at the current pixel is less than the object's depth, then the object is in shadow and should not be lighted
        shadow += fragPosLS.z > pcfDepth ? 1.0 : 0.0;        
    }
    // normalize
    shadow /= 9.0;

    return shadow;
}

vec3 getLight(Light light, vec3 norm, vec3 fragPosLS) {
    // calculate attenuation amount based on distance from source (a = 1 / (c + ld + qd^2))
    float distance = length(light.pos - fragPos);
    float attenuation = (light.linear + light.quadratic > 0) ? // if l = q = 0, don't bother
                            1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)) : 
                            1.0f;

    // calculate direction of light to object
    vec3 lightDir = (light.type == DIR_LIGHT) ? 
                        normalize(-light.dir) :         // if the light is a directional light, the direction is always fixed
                        normalize(light.pos - fragPos); // otherwise, the direction is the vector pointing from the light to the object

    // calculate spotlighting
    float theta = (light.type == SPOT_LIGHT) ? 
                        dot(lightDir, normalize(-light.dir)) :  // how close is the direction of the light to the relative position vec?
                        1.0f;                                   // if its not a spot light, it doesn't matter
    float intensity = (light.type == SPOT_LIGHT) ?              // want a smooth transition from in the spot to out of the spot
                            clamp((theta - light.outer) / (light.inner - light.outer), 0.0, 1.0) : 
                            1.0f;                               // not a spot? don't bother

    return getAmbient(attenuation, light.ambient) +             // always have ambient light
           ((theta >= light.outer) ?                            // no ambient or specular light if the light is outside of the spot (default outer = 0, theta = 1)
                getDiffuse(intensity * attenuation, light.diffuse, norm, lightDir) + 
                getSpecular(intensity * attenuation, light.specular, norm, lightDir) : 
                vec3(0.0f)) 
            * ((fragPosLS.z > 1.0f) ? 1.0f : 1.0f - getShadow(light, fragPosLS));   //check to see whether a spot is within the shadow's range and run the shadow algorithm
}

void main() {
    // make sure that the norm is normalized
    vec3 nNorm = normalize(norm);

    // start the result vector at 0 and add values from each light source, skipping empty light slots
    vec3 result = vec3(0.0f);
    for (int i = 0; i < N_LIGHTS; i++) 
        result += (lightList[i].type != EMPTY_LIGHT) ? getLight(lightList[i], nNorm, fragPosLightSpace[i]) : vec3(0.0f);

    FragColor = vec4(result, 1.0f);
}