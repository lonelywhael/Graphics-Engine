#include "gui/material.hpp"

const unsigned int N_TEXTURES[] = { 0, 0, 1, 2, 3 };
const std::string MAT_NAME[] = { "", "material", "dMap", "dsMap", "dseMap" };

Material::Material(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float shininess)
        : type(M_BASIC), basicMat({ambient, diffuse, specular, shininess}) {}
Material::Material(int diffuse, glm::vec3 specular, float shininess)
        : type(M_D_MAP), dMap({diffuse, specular, shininess}) {}
Material::Material(int diffuse, int specular, float shininess)
        : type(M_DS_MAP), dsMap({diffuse, specular, shininess}) {}
Material::Material(int diffuse, int specular, int emission, float shininess)
        : type(M_DSE_MAP), dseMap({diffuse, specular, emission, shininess}) {}
Material::Material(Format& object) {
    type = object["type"];
    switch(type) {
    case M_BASIC: {
        basicMat.ambient = object["ambient"];
        basicMat.diffuse = object["diffuse"];
        basicMat.specular = object["specular"];
        basicMat.shininess = object["shininess"];
    } break;
    case M_D_MAP: {
        dMap.diffuse = object["diffuse"];
        dMap.specular = object["specular"];
        dMap.shininess = object["shininess"];
    } break;
    case M_DS_MAP: {
        dsMap.diffuse = object["diffuse"];
        dsMap.specular = object["specular"];
        dsMap.shininess = object["shininess"];
    } break;
    case M_DSE_MAP: {
        dseMap.diffuse = object["diffuse"];
        dseMap.specular = object["specular"];
        dseMap.emission = object["emission"];
        dseMap.shininess = object["shininess"];
    } break;
    }
}

bool Material::operator==(Material& other) {
    if (type == other.type) {
        switch(type) {
        case M_BASIC: {
            return basicMat.ambient == other.basicMat.ambient && basicMat.diffuse == other.basicMat.diffuse && 
                   basicMat.shininess == other.basicMat.shininess && basicMat.specular == other.basicMat.specular;
        } break;
        case M_D_MAP: {
            return dMap.diffuse == other.dMap.diffuse && dMap.shininess == other.dMap.shininess && dMap.specular == other.dMap.specular;
        } break;
        case M_DS_MAP: {
            return dsMap.diffuse == other.dsMap.diffuse && dsMap.shininess == other.dsMap.shininess && 
                   dsMap.specular == other.dsMap.specular;
        } break;
        case M_DSE_MAP: {
            return dseMap.diffuse == other.dseMap.diffuse && dseMap.shininess == other.dseMap.shininess && 
                   dseMap.specular == other.dseMap.specular && dseMap.emission == other.dseMap.emission;
        } break;
        }
    } return false;
}

Format Material::getJSON() {
    Format object;
    object["type"] = type;
    switch(type) {
    case M_BASIC: {
        object["ambient"] = basicMat.ambient;
        object["diffuse"] = basicMat.diffuse;
        object["specular"] = basicMat.specular;
        object["shininess"] = basicMat.shininess;
    } break;
    case M_D_MAP: {
        object["diffuse"] = dMap.diffuse;
        object["specular"] = dMap.specular;
        object["shininess"] = dMap.shininess;
    } break;
    case M_DS_MAP: {
        object["diffuse"] = dsMap.diffuse;
        object["specular"] = dsMap.specular;
        object["shininess"] = dsMap.shininess;
    } break;
    case M_DSE_MAP: {
        object["diffuse"] = dseMap.diffuse;
        object["specular"] = dseMap.specular;
        object["emission"] = dseMap.emission;
        object["shininess"] = dseMap.shininess;
    }
    }

    return object;
}