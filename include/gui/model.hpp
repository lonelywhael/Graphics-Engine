#ifndef MODEL_HPP
#define MODEL_HPP

#include <iostream>
#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../io/serializer.hpp"

#include "elements.hpp"
#include "light.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "vertex_array.hpp"

/* MODEL STRUCT
 * The Model struct is a container for storing data necessary to render an object with a high degree of flexibility. The model generically
 * can store any kind information about an object necessary to a shader relating to geometry, lighting, and texturing.
 * 
 * GEOMTETRY: All models must have a vertex array. This does not necessarily need to store 3D data, but it must be present for the model
 * to be able to send vertex data to the shader, without which nothing would be processed. In the case of 3D geometry, the model can also
 * store 3D transformations that can be applied to vertex arrays (which are typically centered at the origin and normalized). 
 * 
 * LIGHTING: Models can optionally be given data to instruct the shader how to light an object (give it color, textures, etc.). This is done
 * by attaching a material struct (see the shader class).
 * 
 * TEXTURES: Models can also optionally recieve texture data, either as a single texture or a texture group (e.g. in the case of a DSE map).
 * Note that textures need to be attached to both the model and the material. 
 * 
 * A model is unique to a single object that will appear on screen. Consider a model as a single container created using a cube mesh, a 
 * DS Map, and 2 texture objects. It has a unique position, size, and orientation in world coordinates. As such, there is a one to one 
 * correspondence between model objects and objects that appear on screen, by contrast to meshes, materials, and textures, which may have
 * multiple corresponding objects on screen.
 */
struct Model {
    friend Scene;
public:
    // Models must be given 1 mesh, 1 material, 0-3 textures, and transformation vectors to transform from mesh space to world space
    Model(std::shared_ptr<VertexArray> vertexArray, std::shared_ptr<Texture> texture) 
            : type(R_BASIC_2D), vertexArray(vertexArray), textureGroup( std::make_shared<TextureGroup>(texture) ) {}
    Model(std::shared_ptr<VertexArray> vertexArray, std::shared_ptr<TextureGroup> textureGroup)
            : type(R_BASIC_2D), vertexArray(vertexArray), textureGroup(textureGroup) {}
    Model(std::shared_ptr<VertexArray> vertexArray, std::shared_ptr<Texture> texture, const unsigned int type)
            : type(type), vertexArray(vertexArray), textureGroup( std::make_shared<TextureGroup>(texture) ) {}
    Model(std::shared_ptr<VertexArray> vertexArray, std::shared_ptr<TextureGroup> textureGroup, const unsigned int type)
            : type(type), vertexArray(vertexArray), textureGroup(textureGroup) {}
    Model(std::shared_ptr<VertexArray> vertexArray, std::shared_ptr<Material> material,
          const glm::vec3 pos = glm::vec3(0.0f), 
          const glm::vec3 scale = glm::vec3(1.0f), 
          const glm::vec3 aos = glm::vec3(0.0f, 1.0f, 0.0f), const float angle = 0)
            : type(R_LIGHTING_3D),
              vertexArray(vertexArray), material(material), 
              pos(pos), scale(scale), aos(aos), angle(angle) { setModel(); }
    Model(std::shared_ptr<VertexArray> vertexArray, const std::shared_ptr<const Light> light,
          const glm::vec3 scale = glm::vec3(1.0f), 
          const glm::vec3 aos = glm::vec3(0.0f, 1.0f, 0.0f), const float angle = 0)
            : type(R_BASIC_3D),
              vertexArray(vertexArray), 
              pos(light->pos), scale(scale), aos(aos), angle(angle),
              color(light->specular) { setModel(); }
    Model(std::shared_ptr<VertexArray> vertexArray, std::shared_ptr<Material> material, std::shared_ptr<Texture> texture,
          const glm::vec3 pos = glm::vec3(0.0f), 
          const glm::vec3 scale = glm::vec3(1.0f), 
          const glm::vec3 aos = glm::vec3(0.0f, 1.0f, 0.0f), const float angle = 0)
            : type(R_LIGHTING_3D),
              vertexArray(vertexArray), material(material), textureGroup( std::make_shared<TextureGroup>(texture) ), 
              pos(pos), scale(scale), aos(aos), angle(angle) { setModel(); }
    Model(std::shared_ptr<VertexArray> vertexArray, std::shared_ptr<Material> material, std::shared_ptr<TextureGroup> textureGroup, 
          const glm::vec3 pos = glm::vec3(0.0f), 
          const glm::vec3 scale = glm::vec3(1.0f), 
          const glm::vec3 aos = glm::vec3(0.0f, 1.0f, 0.0f), const float angle = 0)
            : type(R_LIGHTING_3D),
              vertexArray(vertexArray), material(material), textureGroup(textureGroup), 
              pos(pos), scale(scale), aos(aos), angle(angle) { setModel(); }
    Model(Serializer& object);

    bool operator==(Model& other) {
        return type == other.type && vertexArray == other.vertexArray && textureGroup == other.textureGroup && 
               material == other.material && pos == other.pos && scale == other.scale && aos == other.aos && angle == other.angle && 
               color == other.color;
    }

    // change the position, size, or orientation of the model
    void move(const glm::vec3 pos) { this->pos = pos; setModel(); }
    void grow(const glm::vec3 scale) { this->scale = scale; setModel(); }
    void rotate(const glm::vec3 aos, const float angle) { this->aos = aos; this->angle = angle; setModel(); }

    // get/set model transformation that transforms from mesh space to world space
    void setModel();
    glm::mat4 getModel() const { return model; }

    // retrieve the current position of the model in world space
    unsigned int getType() const { return type; }
    unsigned int getMaterialType() const { return (material != nullptr) ? material->type : M_DISABLED; }
    unsigned int getTextureType() const { return (textureGroup != nullptr) ? textureGroup->getType() : T_DISABLED; }
    glm::vec3 getPos() const { return pos; }
    glm::vec3 getColor() const { return color; }

    // retrieve non-modifiable copies of the constituent parts of the model
    std::shared_ptr<VertexArray> getVertexArray() const { return vertexArray; }
    std::shared_ptr<TextureGroup> getTextureGroup() const { return textureGroup; }
    std::shared_ptr<Material> getMaterial() const { return material; }

    // based on existing texture data, create a material for a model
    void generateMaterial(const glm::vec3 specular, const float shininess);
    void generateMaterial(const float shininess = 0.0f);

    Serializer getJSON();

    void print() const;
private:
    // model type tells us how specific models should be rendered (are they 3D models? Do they have lighting information? etc.)
    unsigned int type;

    // As a container, the model's references objects are stored publicly. Models contain 1 mesh, 1 material, and 0-3 textures.
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<TextureGroup> textureGroup = nullptr;
    std::shared_ptr<Material> material = nullptr;

    // transformation information for translating from mesh to world space
    // pos - translates to center on pos vector; scale – scales by coefficients in scale vector; 
    // aos and angle – rotates by angle around aos vector (axis of symmetry)
    glm::vec3 pos, scale, aos;
    float angle;
    // transformation matrix from mesh to world space
    glm::mat4 model;

    // non-lighted models sometimes need a color
    glm::vec3 color;
};


#endif