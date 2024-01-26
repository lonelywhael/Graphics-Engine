#ifndef SCENE_HPP
#define SCENE_HPP

#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "../io/format.hpp"

#include "elements.hpp"
#include "frame.hpp"
#include "shader.hpp"
#include "render_group.hpp"
#include "texture.hpp"

/* SCENE CLASS
 * 
 * The Scene class is a large container class used to manage all scene elements in a single place, specifically shaders, models, meshes,
 * lights, materials, and textures. The purpose of the scene class is to be able to add all these elements to a single scene, then load
 * and draw that scene in a single function call.
 * 
 * Since many elements are "reusable," i.e., a single mesh or texture can be used across many different objects, memory and computation time
 * to be saved by reusing the same elements repeatedly instead of recreating those elements for each object they might be used in.
 * 
 * In order to achieve the single function load and draw calls, the Scene class uses the Shader Group container. Each shader group links
 * a set of models and lights to a shader, then performs the shader specific routine on all objects within the shader group. For example,
 * since normal objects require a different shader than light source objects, they would be place into two different shader groups. The 
 * shader group struct itself can be found in shader.hpp.
 * 
 * Elements can be stored in the scene itself by storing dynamic pointers, or they can be created else where in the program and simply
 * linked to the scene. The scene will only delete pointers to objects that are stored in the scene itself and will not delete pointers
 * declared elsewhere.
 */

class Scene {
public:
    // There is no constructor since elements should be added to the scene individually.
    Scene(std::shared_ptr<Camera> camera, const std::string& file_name);
    Scene(const int width, const int height, const int pixelWidth, std::shared_ptr<Camera> camera)
            : viewportWidth(width), viewportHeight(height), pixelWidth(pixelWidth), camera(camera) {}
    Scene(const int width, const int height, std::shared_ptr<Camera> camera) 
            : viewportWidth(width), viewportHeight(height), camera(camera) {}

    // Add a shader to the scene by reference to an existing shader
    const std::shared_ptr<Shader> addShader(std::shared_ptr<Shader> shader);
    // Create a new shader in the scene
    const std::shared_ptr<Shader> addShader(const unsigned int RENDERING_STYLE, const unsigned int OUTPUT_BUFFER,
                                            const unsigned int MATERIAL_STYLE, const unsigned int LIGHTING_STYLE, 
                                            const unsigned int SHADOW_STYLE, const unsigned int TEXTURE_STYLE, 
                                            const unsigned int POSTPROCESSING);
    // Retrieve a shader from the scene by index
    Shader& getShader(const unsigned int index) const { return *shaders[index]; }

    // Add a model to the scene by reference to an existing model
    const std::shared_ptr<Model> addModel(std::shared_ptr<Model> model) { return addModel(model, true); }
    // Create a new 3D light source model by referencing existing mesh and light objects
    const std::shared_ptr<Model> addPane(std::shared_ptr<VertexArray> vertexArray, std::shared_ptr<Texture> texture);
    const std::shared_ptr<Model> addLightModel(std::shared_ptr<VertexArray> vertexArray, const std::shared_ptr<const Light> light,
                                               const glm::vec3 scale = glm::vec3(0.0f), 
                                               const glm::vec3 aos = glm::vec3(0.0, 1.0, 0.0), const float rotation = 0.0f);
    // create a 3D model with material lighting data
    const std::shared_ptr<Model> addBasicModel(std::shared_ptr<VertexArray> vertexArray, std::shared_ptr<Material> material,
                                               const glm::vec3 pos = glm::vec3(0.0f), 
                                               const glm::vec3 scale = glm::vec3(0.0f), 
                                               const glm::vec3 aos = glm::vec3(0.0, 1.0, 0.0), const float rotation = 0.0f);
    // created a 3D model with texture lighting data
    const std::shared_ptr<Model> addTexturedModel(std::shared_ptr<VertexArray> vertexArray, std::shared_ptr<Material> material, 
                                                  std::shared_ptr<Texture> texture,
                                                  const glm::vec3 pos = glm::vec3(0.0f), 
                                                  const glm::vec3 scale = glm::vec3(0.0f), 
                                                  const glm::vec3 aos = glm::vec3(0.0, 1.0, 0.0), const float rotation = 0.0f);
    const std::shared_ptr<Model> addTexturedModel(std::shared_ptr<VertexArray> vertexArray, std::shared_ptr<Material> material, 
                                                  std::shared_ptr<TextureGroup> textureGroup,
                                                  const glm::vec3 pos = glm::vec3(0.0f), 
                                                  const glm::vec3 scale = glm::vec3(0.0f), 
                                                  const glm::vec3 aos = glm::vec3(0.0, 1.0, 0.0), const float rotation = 0.0f);
    // Create a new light source model by referencing scene-based mesh and light objects
    const std::shared_ptr<Model> addLightModel(const unsigned int vaID, const unsigned int lightID,
                                               const glm::vec3 scale = glm::vec3(1.0f), 
                                               const glm::vec3 aos = glm::vec3(0.0, 1.0, 0.0), const float rotation = 0.0f);
    // Create a new object model by referencing scene-based mesh, material, and texture objects
    const std::shared_ptr<Model> addBasicModel(const unsigned int vaID, const unsigned int materialID,
                                               const glm::vec3 pos = glm::vec3(0.0f), 
                                               const glm::vec3 scale = glm::vec3(1.0f), 
                                               const glm::vec3 aos = glm::vec3(0.0, 1.0, 0.0), const float rotation = 0.0f);
    // created a new object model by referencing scene-based textures
    const std::shared_ptr<Model> addTexturedModel(const unsigned int vaID, const unsigned int materialID, const unsigned int textureGroupID,
                                                  const glm::vec3 pos = glm::vec3(0.0f), 
                                                  const glm::vec3 scale = glm::vec3(1.0f), 
                                                  const glm::vec3 aos = glm::vec3(0.0, 1.0, 0.0), const float rotation = 0.0f);
    // retrieve a scene based model by index
    Model& getModel(const unsigned int index) const { return *models[index]; }

    // add an existing mesh object to the scene
    const std::shared_ptr<VertexArray> addVertexArray(std::shared_ptr<VertexArray> vertexArray);
    // create a new mesh object in the scene
    const std::shared_ptr<VertexArray> addPane(const float cornerX = -1.0f, const float cornerY = -1.0f, 
                                               const float dimX = 2.0f, const float dimY = 2.0f);
    const std::shared_ptr<VertexArray> addHeightMap(const unsigned int resolution, const unsigned int function, 
                                                    const unsigned int draw_type);
    const std::shared_ptr<VertexArray> addSphereMap(const unsigned int resolution, const unsigned int function, 
                                                    const unsigned int draw_type);
    const std::shared_ptr<VertexArray> addVertexArray(const std::string& fileName, const unsigned int renderStrategy);
    // retrieve a mesh object from the scene by index
    VertexArray& getVertexArray(const unsigned int index) const { return *vertexArrays[index]; }

    // add an existing light object to the scene
    const std::shared_ptr<Light> addLight(std::shared_ptr<Light> light);
    // create a directional light in the scene
    const std::shared_ptr<Light> addDirLight(const glm::vec3 dir, 
                                             const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular);
    // create a point light in the scene
    const std::shared_ptr<Light> addPointLight(const glm::vec3 pos, 
                                               const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, 
                                               const float constant, const float linear, const float quadratic);
    // create a spot light in the scene
    const std::shared_ptr<Light> addSpotLight(const glm::vec3 pos, const glm::vec3 dir, 
                                              const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, 
                                              const float constant, const float linear, const float quadratic, 
                                              const float inner, const float outer);
    // retrieve a light from the scene by index
    Light& getLight(const unsigned int index) const { return *lights[index]; }

    // add an existing material object to the scene
    const std::shared_ptr<Material> addMaterial(std::shared_ptr<Material> material);
    // create a basic material in the scene
    const std::shared_ptr<Material> addBasicMat(const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, const float shininess);
    // create a dMap in the scene by referencing existing textures
    const std::shared_ptr<Material> addDMap(const std::shared_ptr<const TextureGroup> textureGroup, const glm::vec3 specular, const float shininess);
    // create a dMap in the scene by referencing scene-based textures
    const std::shared_ptr<Material> addDMap(const unsigned int texGroupID, const glm::vec3 specular, const float shininess);
    // create a dsMap in the scene by referencing existing textures
    const std::shared_ptr<Material> addDSMap(const std::shared_ptr<const TextureGroup> textureGroup, const float shininess);
    // create a dsMap in the scene by referencing scene-based textures
    const std::shared_ptr<Material> addDSMap(const unsigned int texGroupID, const float shininess);
    // create a dseMap in the scene by referencing existing textures
    const std::shared_ptr<Material> addDSEMap(const std::shared_ptr<const TextureGroup> textureGroup, const float shininess);
    // create a dseMap in the scene by referenceing scene-based textures
    const std::shared_ptr<Material> addDSEMap(const unsigned int texGroupID, const float shininess);
    // retrieve a material from the scene by index
    Material& getMaterial(const unsigned int index) const { return *materials[index]; }

    // add an existing texture to the scene
    const std::shared_ptr<TextureGroup> addTextureGroup(std::shared_ptr<TextureGroup> textureGroup);
    // create texture group in the scene
    const std::shared_ptr<TextureGroup> addTextureGroup(unsigned int firstSlot = 0);
    // create new texture group with single member
    const std::shared_ptr<TextureGroup> addTextureGroup(const std::shared_ptr<Texture> texture);
    // create a next texture and add it to an existing group
    void addTextureToGroup(const unsigned int textureGroupID, 
                           const std::string& fileName, const std::string& extension, 
                           const unsigned int filter, const unsigned int wrapper, const unsigned int mipmap)
        { getTextureGroup(textureGroupID).addTexture(fileName, extension, filter, wrapper, mipmap); }
    // add an existing texture to a group
    void addTextureToGroup(const unsigned int textureGroupID, const std::shared_ptr<Texture> texture)
        { getTextureGroup(textureGroupID).addTexture(texture); }
    // retrieve a texture from the scene by reference
    TextureGroup& getTextureGroup(const unsigned int index) const { return *textureGroups[index]; }

    void enableAntiAliasing() { aa_enabled = true; }
    void disableAntiAliasin() { aa_enabled = false; }
    void enableBlur() { blur = true; }
    void disableBlur() { blur = false; }
    void setPixelWidth(const int pixelWidth) { this->pixelWidth = pixelWidth; }
    void setShadowStyle(const unsigned int shadowStyle) { this->shadowStyle = shadowStyle; }

    // load the scene (occurs before render loop)
    void load();
    // draw the scene (draw the scene to the framebuffer each frame)
    void draw();

    void save(const std::string& path);
    void print() const;
private:
    // a list of shader groups, each of which will be drawn every frame
    std::shared_ptr<Camera> camera;
    std::unique_ptr<Frame> frame;
    std::vector<std::shared_ptr<RenderGroup>> renderGroups;

    // Lists of elements
    std::vector<std::shared_ptr<Shader>> shaders;
    std::vector<std::shared_ptr<Model>> models;
    std::vector<std::shared_ptr<VertexArray>> vertexArrays;
    std::vector<std::shared_ptr<Light>> lights;
    std::vector<std::shared_ptr<Material>> materials;
    std::vector<std::shared_ptr<TextureGroup>> textureGroups;

    std::map<std::shared_ptr<RenderGroup>, int> renderGroup_lookup;
    std::map<std::shared_ptr<Shader>, int> shader_lookup;
    std::map<std::shared_ptr<Model>, int> model_lookup;
    std::map<std::shared_ptr<VertexArray>, int> vertexArray_lookup;
    std::map<std::shared_ptr<Light>, int> light_lookup;
    std::map<std::shared_ptr<Material>, int> material_lookup;
    std::map<std::shared_ptr<TextureGroup>, int> textureGroup_lookup;

    int viewportWidth, viewportHeight, pixelWidth = 1;

    // post processing settings
    bool aa_enabled = false;
    bool blur = false;

    // 3D rendering settings
    unsigned int shadowStyle = S_DISABLED;

    // Add shader group by linking a shader, a list of models, and a list of lights.
    const std::shared_ptr<RenderGroup> addRenderGroup(std::shared_ptr<Shader> shader) { return addRenderGroup(-1, shader); }
    const std::shared_ptr<RenderGroup> addRenderGroup(unsigned int index, std::shared_ptr<Shader> shader);
    // Add shader group by listing the indices of already stored shader, models, and lights
    const std::shared_ptr<RenderGroup> addRenderGroup(const unsigned int shaderID) { return addRenderGroup(-1, shaders[shaderID]); }
    // Add a model (by pointer) to an existing shader group (by index)
    void addModelToGroup(const std::shared_ptr<RenderGroup> renderGroup, std::shared_ptr<Model> model) 
        { addModel(model); renderGroup->addModel(model); }
    void addModelToGroup(const unsigned int index, std::shared_ptr<Model> model) 
        { addModel(model); getRenderGroup(index).addModel(model); }
    // Add a model to an existing shader group (both by index)
    void addModelToGroup(const std::shared_ptr<RenderGroup> renderGroup, const unsigned int modelID) 
        { addModelToGroup(renderGroup, models[modelID]); }
    void addModelToGroup(const unsigned int index, const unsigned int modelID) 
        { addModelToGroup(index, models[modelID]); }
    // Add a light (by pointer) to an existing shader group (by index)
    void addLightToGroup(const std::shared_ptr<RenderGroup> renderGroup, std::shared_ptr<Light> light) 
        { addLight(light); renderGroup->addLight(light); }
    void addLightToGroup(const unsigned int index, std::shared_ptr<Light> light) 
        { addLight(light); getRenderGroup(index).addLight(light); }
    // Add a light to an existing shader group (both by index)
    void addLightToGroup(const std::shared_ptr<RenderGroup> renderGroup, const unsigned int lightID) 
        { addLightToGroup(renderGroup, lights[lightID]); }
    void addLightToGroup(const unsigned int index, const unsigned int lightID) 
        { addLightToGroup(index, lights[lightID]); }
    // Add an existing camera to an existing shader group by index
    void addLightToGroup(const std::shared_ptr<RenderGroup> renderGroup, std::shared_ptr<Camera> camera) 
        { renderGroup->addCamera(camera); }
    void addCameraToGroup(const std::shared_ptr<RenderGroup> renderGroup, std::shared_ptr<Camera> camera) 
        { renderGroup->addCamera(camera); }
    void addCameraToGroup(const unsigned int index, std::shared_ptr<Camera> camera) 
        { getRenderGroup(index).addCamera(camera); }
    RenderGroup& getRenderGroup(const unsigned int index) { return *renderGroups[index]; }

    const std::shared_ptr<Model> addModel(std::shared_ptr<Model> model, bool add_all);
};

#endif