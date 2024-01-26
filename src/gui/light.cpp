#include "gui/light.hpp"

// set the maximum number of lights that can be rendered with a single shader
const int MAX_LIGHTS = 4;
// the uniform name for the light variable within shaders
const std::string LIGHT_NAME = "lightList";

Light::Light(Format& object) {
    type = object["type"];
    pos = object["pos"];
    dir = object["dir"];
    ambient = object["ambient"];
    diffuse = object["diffuse"];
    specular = object["specular"];
    constant = object["constant"];
    linear = object["linear"];
    quadratic = object["quadratic"];
    inner = object["inner"];
    outer = object["outer"];
}

bool Light::operator==(Light& other) {
    return type == other.type && pos == other.pos && dir == other.dir && ambient == other.ambient && diffuse == other.diffuse &&
           specular == other.specular && constant == other.constant && linear == other.linear && quadratic == other.quadratic &&
           inner == other.inner && outer == other.outer;
}

const float NEAR = 0.1f, DIR_FAR = 10.0f, DEFAULT_FAR = 20.0f;
void Light::setLightTransform(const glm::vec3 target) {
    glm::mat4 lightProjection, lightView;

    // first, define the light's fustrum
    const float CUTOFF = 0.05f; // for attenuated lights, cut off when the light diminishes to 5% of its original brightness
    float near = NEAR, far; 
    switch(type) {
    // since directional lights use an orthographic projection, the fustrum needs to be smaller to avoid introducing weird effects
    case L_DIR: { far = DIR_FAR; } break;
    default: {
        far = (quadratic > 0) ? 
                    // if the light is attenuated, we want to solve the equation qx^2 + lx + c = 1 / CUTOFF
                    0.5 * (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - 1.0f / CUTOFF))) / quadratic :
                    // otherwise use some default large value
                    DEFAULT_FAR;
    } break;
    }

    // next, we need to set the light projection and view vectors according to the light type
    glm::vec3 pos, dir, up;

    if (type == L_DIR) {
        // directional lights will use orthographic projections because light rays are assumed to be parallel
        lightProjection = glm::ortho(-far, far, -far, far, near, far);
        // directional lights don't have a position, but we need to pretend they do so just find a point that looks in the right direction at the target
        pos = target - 0.5f * far * this->dir;
    } else {
        // point and spot lights will spread out over an angle and so will have similar calculation to our camera view calculation
        lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, near, far);
        // point and spot lights already have a position
        pos = this->pos;
    }
    // the direction vector is the vector from the position to the target
    dir = glm::normalize(target - pos);
    // up will be in the dir - y plane, unless dir is already in the y direction, in which case it will be in the dir-x plane
    up = (!(dir.x == 0.0f && dir.z == 0.0f)) ? 
                glm::normalize(glm::cross(glm::cross(dir, glm::vec3(0.0f, 1.0f, 0.0f)), dir)) :
                glm::vec3(1.0f, 0.0f, 0.0f);

    // set the view matrix based on the above calculations
    lightView = glm::lookAt(pos, target, up);

    // multiply the projection and view matrices to get the transformation from world to clip space
    lightTransform = lightProjection * lightView;
}

Format Light::getJSON() {
    Format object;
    object["type"] = type;
    object["pos"] = pos;
    object["dir"] = dir;
    object["ambient"] = ambient;
    object["diffuse"] = diffuse;
    object["specular"] = specular;
    object["constant"] = constant;
    object["linear"] = linear;
    object["quadratic"] = quadratic;
    object["inner"] = inner;
    object["outer"] = outer;
    object["shadow_map"] = shadowMap;

    return object;
}
void Light::print() const {
    std::cout << "Type: ";
    switch(type) {
    case L_DISABLED:
        std::cout << "Empty";
        break;
    case L_DIR:
        std::cout << "Directional Light";
        break;
    case L_POINT:
        std::cout << "Point Light";
        break;
    case L_SPOT:
        std::cout << "Spot Light";
        break;
    }
    std::cout << std::endl;
    std::cout << "Spatial Parameters -\t\tpos(" << pos.x << ", " << pos.y << ", " << pos.z << "), " <<
                                      "dir(" << dir.x << ", " << dir.y << ", " << dir.z << ")" << std::endl;
    std::cout << "Color Parameters –\t\tambient(" << ambient.x << ", " << ambient.y << ", " << ambient.z << "), " <<
                                       "diffuse(" << diffuse.x << ", " << diffuse.y << ", " << diffuse.z << "), " <<
                                       "specular(" << specular.x << ", " << specular.y << ", " << specular.z << "), " << std::endl;
    std::cout << "Attenuation Parameters –\t1/(" << constant << " + " << linear << "d + " << quadratic << "d^2)" << std::endl;
    std::cout << "Spotlight Parameters –\t\tInner = " << inner << ", Outer = " << outer << std::endl;
    std::cout << "Shadow Map: " << shadowMap << std::endl;
    std::cout << std::endl;
}

const Light NULL_LIGHT = Light();