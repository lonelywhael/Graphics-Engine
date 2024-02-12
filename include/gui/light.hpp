#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <iostream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../io/serializer.hpp"

#include "elements.hpp"
#include "texture.hpp"

/* LIGHT STRUCT
 * 
 * Lights store data used for lighting calculations within a shader. Lights should be not be thought of as actual light models that appear 
 * on screen. Rather, these are abstract light objects strictly used to make calculations. 
 * 
 * The light object can be easily written to or animated and the shader class can recieve it as a format to be easily update uniforms in
 * the shader program itself. Even though there are multiple types of lights, all lights are stored as a single uniform array within the shader program. The shader 
 * knows how to differentiate between light types and can, to some degree, mix and match light types where possible.
 * 
 * Light objects also store shadow maps that are used to buffer shadow data which is later sent to a second pass rendering pipeline. The
 * light object is also responsable for making calculations related to shadow geometry, and as such also stores certain data related to 
 */

// type list for lights
extern const int MAX_LIGHTS;    // shaders cannot store dynamic arrays so the max number of lights in a scene must be specified
extern const std::string LIGHT_NAME;    // the name of the uniform used to store light data in a shader

struct Light {
    /* The light class contains all possible lighting parameters so that lights are, to some degree, not rigidly constrained to the kind of
     * indicated by their light flag. As such, any values not called in the constructor recieve default values so that they will not 
     * interfere with lighting calculations.
     */

    // Type serves as a flag to the shader to indicate which parts of the shader algorithm should be used
    int type = L_DISABLED;
    // Spatial data: pos gives the light's position (point and spot) and dir gives its direction (directional and spot)
    glm::vec3 pos = glm::vec3(0.0f), dir = glm::vec3(0.0f);
    // Color data: (all) lights typically have differential ambient, diffuse, and specular values, and can also be colored
    glm::vec3 ambient = glm::vec3(0.0f), diffuse = glm::vec3(0.0f), specular = glm::vec3(0.0f);
    // Attenuation: some lights (point and spot) can attenuate with distance away from their source, with the formula 1/(c + ld + qd^2)
    float constant = 1.0f, linear = 0.0f, quadratic = 0.0f;
    // Spot: spot lights are constrained to light an area a certain angle from the source direction. 
    //       They can also have a soft edge, starting to fade at inner and becoming completely dark at outer.
    float inner = 0.0f, outer = 0.0f;
    int shadowMap = FRAME_SLOT;

    // the default constructor is used to create "empty lights" which fill slots in the light array without changing anything in the shader
    Light() {}
    // Directional lights need a direction vector and color data
    Light(const glm::vec3 dir, 
          const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular)
            : type(L_DIR), 
              dir(dir),
              ambient(ambient), diffuse(diffuse), specular(specular) {}
    // Point lights need a position vector, color data, and attenuation values. For no attenutation, set c = 1, l = q = 0
    Light(const glm::vec3 pos, 
          const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, 
          const float constant, const float linear, const float quadratic)
            : type(L_POINT), 
              pos(pos), 
              ambient(ambient), diffuse(diffuse), specular(specular), 
              constant(constant), linear(linear), quadratic(quadratic) {}
    // Spot lights need position and direction, color and attenuation, and spot angles. 
    Light(const glm::vec3 pos, const glm::vec3 dir, const glm::vec3 ambient, 
          const glm::vec3 diffuse, const glm::vec3 specular, 
          const float constant, const float linear, const float quadratic, 
          const float inner, const float outer)
            : type(L_SPOT), 
              pos(pos), dir(dir), 
              ambient(ambient), diffuse(diffuse), specular(specular), 
              constant(constant), linear(linear), quadratic(quadratic), 
              // spot angles are given as degrees, but shader needs a cosine value since std::cos() is an expensive call
              inner(std::cos(glm::radians(inner))), outer(std::cos(glm::radians(outer))) {}
    Light(Serializer& object);

    bool operator==(Light& other);

    // set the spatial parameters of a light
    void setSpatial(const glm::vec3 pos, const glm::vec3 dir) { this->pos = pos; this->dir = dir; }
    // set the color parameters of a light
    void setColor(const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular) 
        { this->ambient = ambient; this->diffuse = diffuse; this->specular = specular; }
    // set the attenuation parameters of a light
    void setAttenuation(const float constant, const float linear, const float quadratic) 
        { this->constant = constant; this->linear = linear; this->quadratic = quadratic; }
    // set the spot parameters of a light
    void setSpotlight(const float inner, const float outer) { this->inner = std::cos(inner); this->outer = std::cos(outer); }

    // transform an empty light into a directional light
    void dirLight(const glm::vec3 dir, 
                  const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular) 
        { type = L_DIR; this->dir = dir; setColor(ambient, diffuse, specular); }
    // transform an empty light into a point light
    void pointLight(const glm::vec3 pos, 
                    const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, 
                    const float constant, const float linear, const float quadratic) 
        { type = L_POINT; this->pos = pos; setColor(ambient, diffuse, specular); setAttenuation(constant, linear, quadratic); }
    // transform an empty light into a spot light
    void spotLight(const glm::vec3 pos, const glm::vec3 dir, 
                   const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, 
                   const float constant, const float linear, const float quadratic, 
                   const float inner, const float outer) {
        type = L_SPOT; 
        setSpatial(pos, dir); setColor(ambient, diffuse, specular); setAttenuation(constant, linear, quadratic); setSpotlight(inner, outer);
    }

    // for shadow calculations, we need a transformation matrix that converts world space into the clip space from the light's perspective
    void setLightTransform(const glm::vec3 target);
    glm::mat4 getLightTransform() const { return lightTransform; };

    void setShadowMapSlot(const int slot) { shadowMap = slot; }
    // modify and retrieve the slot value of shadow maps (necessary for properly slotting them when rendering a complete scene)
    int getShadowMapSlot() const { return shadowMap; }

    Serializer getJSON();
    void print() const;
private:
    // transformation from world space to light's clip space
    glm::mat4 lightTransform;
};
// instead of spending time to create empty lights, can also just reference this existing empty light value
extern const Light NULL_LIGHT;

#endif