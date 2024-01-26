#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <iostream>
#include <string>

#include <glm/glm.hpp>

#include "../io/format.hpp"

#include "elements.hpp"

/* MATERIAL STRUCT
 * 
 * Materials are containers for data that is fed directly into the shader. They provide information to the shader about how to access
 * textures and what color values to use.
 * 
 * There are multiple material formats but are all stored in the same union for ease of abstraction and save processing time (at some
 * cost in space). The type variable can be used to determine which type a material is an proceed using a switch statement.
 */
///TODO: Maybe figure out how to do this a bit more efficiently? Do all float values need such high precision? _Float16?
///TODO: Functions for returning various material types instead of having to fill out the array manually

extern const unsigned int N_TEXTURES[]; // each material requires a specific number of textures, e.g. N_TEXTURES[D_MAP] = 1
extern const std::string MAT_NAME[];    // each material uses a specific variable name, e.g. MAT_NAME[D_MAP] = "dMap"

struct Material {
    unsigned int type;
    union {
        struct {                                    // Basic Material – 40 bytes
            glm::vec3 ambient, diffuse, specular;   // All lighting types differentiated with separate rgb values
            float shininess;                        // Shininess determines how diffuse reflected light is (independently from color)
        } basicMat;
        struct {                                    // D Map – 20 bytes
            int diffuse;                            // int value used to reference the texture slot of a diffuse map
            glm::vec3 specular;                     // rgb values used for specular
            float shininess;
        } dMap;
        struct {                                    // DS Map - 12 bytes
            int diffuse, specular;                  // int values used to reference the texture slots of diffuse and specular maps
            float shininess;
        } dsMap;
        struct {                                    // DSE Map - 16 bytes
            int diffuse, specular, emission;        // int values used to reference the texture slots of diffuse, specular, and emission maps
            float shininess;
        } dseMap;
    };

    Material(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float shininess);
    Material(int diffuse, glm::vec3 specular, float shininess);
    Material(int diffuse, int specular, float shininess);
    Material(int diffuse, int specular, int emission, float shininess);
    Material(Format& object);

    bool operator==(Material& other);

    Format getJSON();
};

#endif