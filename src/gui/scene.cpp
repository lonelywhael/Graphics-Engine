#include "gui/scene.hpp"

const std::string SCENE_PATH = "../res/scenes/";

Scene::Scene(std::shared_ptr<Camera> camera, const std::string& file_name) : camera(camera) {
    Serializer object = Serializer(SCENE_PATH + file_name);
    frame = std::make_unique<Frame>(static_cast<Serializer&>(object["frame"]));
}

const std::shared_ptr<RenderGroup> Scene::addRenderGroup(unsigned int index, std::shared_ptr<Shader> shader) {
    std::shared_ptr<RenderGroup> renderGroup = std::make_shared<RenderGroup>(shader);
    if (index == -1) {
        renderGroup_lookup.emplace(renderGroup, renderGroups.size());
        renderGroups.push_back(renderGroup);
    }
    else {
        renderGroups.insert(renderGroups.begin() + index, renderGroup);
        for (int rg = index; rg < renderGroups.size(); rg++) renderGroup_lookup[renderGroups[rg]] = rg;
    }
    return renderGroup;
}

const std::shared_ptr<Shader> Scene::addShader(std::shared_ptr<Shader> shader) {
    for (int i = 0; i < shaders.size(); i++) if (shaders[i].get() == shader.get() || shaders[i] == shader) return shaders[i]; 
    shader->load();
    shader_lookup.emplace(shader, shaders.size());
    shaders.push_back(shader); 
    return shader; 
}
const std::shared_ptr<Shader> Scene::addShader(const unsigned int RENDERING_STYLE, const unsigned int OUTPUT_BUFFER,
                                               const unsigned int MATERIAL_STYLE, const unsigned int LIGHTING_STYLE, 
                                               const unsigned int SHADOW_STYLE, const unsigned int TEXTURE_STYLE, 
                                               const unsigned int POSTPROCESSING) {
    std::shared_ptr<Shader> shader = std::make_shared<Shader>(RENDERING_STYLE, OUTPUT_BUFFER, MATERIAL_STYLE, LIGHTING_STYLE,
                                                              SHADOW_STYLE, TEXTURE_STYLE, POSTPROCESSING);
    return addShader(shader);
}

const std::shared_ptr<Model> Scene::addModel(std::shared_ptr<Model> model, bool add_all) {
    for (int i = 0; i < models.size(); i++) if (models[i].get() == model.get() || models[i] == model) return models[i];
    if (add_all) {
        if (model->vertexArray != nullptr) addVertexArray(model->vertexArray);
        if (model->material != nullptr) addMaterial(model->material);
        if (model->textureGroup != nullptr) addTextureGroup(model->textureGroup);
    }
    model_lookup.emplace(model, models.size());
    models.push_back(model); 
    return model; 
}
const std::shared_ptr<Model> Scene::addPane(std::shared_ptr<VertexArray> vertexArray, std::shared_ptr<Texture> texture) {
    std::shared_ptr<Model> model = std::make_shared<Model>(vertexArray, texture);
    return addModel(model, true);
}
// Create a new 3D light source model by referencing existing mesh and light objects
const std::shared_ptr<Model> Scene::addLightModel(std::shared_ptr<VertexArray> vertexArray, const std::shared_ptr<const Light> light,
                                                  const glm::vec3 scale, 
                                                  const glm::vec3 aos, const float rotation) { 
    std::shared_ptr<Model> model = std::make_shared<Model>(vertexArray, light, scale, aos, rotation);
    return addModel(model, true);
}
// create a 3D model with material lighting data
const std::shared_ptr<Model> Scene::addBasicModel(std::shared_ptr<VertexArray> vertexArray, std::shared_ptr<Material> material,
                                                  const glm::vec3 pos, 
                                                  const glm::vec3 scale, 
                                                  const glm::vec3 aos, const float rotation) {                  
    std::shared_ptr<Model> model = std::make_shared<Model>( vertexArray, material, pos, scale, aos, rotation );
    return addModel(model, true);
}
// created a 3D model with texture lighting data
const std::shared_ptr<Model> Scene::addTexturedModel(std::shared_ptr<VertexArray> vertexArray, std::shared_ptr<Material> material, 
                                                     std::shared_ptr<Texture> texture,
                                                     const glm::vec3 pos, 
                                                     const glm::vec3 scale, 
                                                     const glm::vec3 aos, const float rotation) { 
    std::shared_ptr<Model> model = std::make_shared<Model>( vertexArray, material, texture, pos, scale, aos, rotation );
    return addModel(model, true);
}
const std::shared_ptr<Model> Scene::addTexturedModel(std::shared_ptr<VertexArray> vertexArray, std::shared_ptr<Material> material, 
                                                     std::shared_ptr<TextureGroup> textureGroup,
                                                     const glm::vec3 pos, 
                                                     const glm::vec3 scale, 
                                                     const glm::vec3 aos, const float rotation) { 
    std::shared_ptr<Model> model = std::make_shared<Model>( vertexArray, material, textureGroup, pos, scale, aos, rotation );
    return addModel(model, true);
}
// Create a new light source model by referencing scene-based mesh and light objects
const std::shared_ptr<Model> Scene::addLightModel(const unsigned int vaID, const unsigned int lightID,
                                                  const glm::vec3 scale, const glm::vec3 aos, const float rotation) {
    std::shared_ptr<Model> model = std::make_shared<Model>(vertexArrays[vaID], lights[lightID], scale, aos, rotation); 
    return addModel(model, false);
}
// Create a new object model by referencing scene-based mesh, material, and texture objects
const std::shared_ptr<Model> Scene::addBasicModel(const unsigned int vaID, const unsigned int materialID,
                                                  const glm::vec3 pos, const glm::vec3 scale, 
                                                  const glm::vec3 aos, const float rotation) {
    std::shared_ptr<Model> model = std::make_shared<Model>(vertexArrays[vaID], materials[materialID], pos, scale, aos, rotation); 
    return addModel(model, false);
}
// created a new object model by referencing scene-based textures
const std::shared_ptr<Model> Scene::addTexturedModel(const unsigned int vaID, const unsigned int materialID, 
                                                     const unsigned int textureGroupID,
                                                     const glm::vec3 pos, const glm::vec3 scale, 
                                                     const glm::vec3 aos, const float rotation) {
    std::shared_ptr<Model> model = std::make_shared<Model>(vertexArrays[vaID], materials[materialID], textureGroups[textureGroupID], 
                                                           pos, scale, aos, rotation); 
    return addModel(model, false);
}

const std::shared_ptr<VertexArray> Scene::addVertexArray(std::shared_ptr<VertexArray> vertexArray) {
    for (int i = 0; i < vertexArrays.size(); i++) if (vertexArrays[i].get() == vertexArray.get() || vertexArrays[i] == vertexArray)
        return vertexArrays[i];
    vertexArray_lookup.emplace(vertexArray, vertexArrays.size());
    vertexArrays.push_back(vertexArray); 
    return vertexArray;
}
const std::shared_ptr<VertexArray> Scene::addPane(const float cornerX, const float cornerY, const float dimX, const float dimY) {
    std::shared_ptr<VertexArray> vertexArray = std::make_shared<VertexArray>();
    vertexArray->makePane(cornerX, cornerY, dimX, dimY);
    return addVertexArray(vertexArray);
}
const std::shared_ptr<VertexArray> Scene::addHeightMap(const unsigned int resolution, const unsigned int function, 
                                                       const unsigned int draw_type) {
    std::shared_ptr<VertexArray> vertexArray = std::make_shared<VertexArray>();
    vertexArray->makeHeightMap(resolution, function, draw_type);
    return addVertexArray(vertexArray);
}
const std::shared_ptr<VertexArray> Scene::addSphereMap(const unsigned int resolution, const unsigned int function, 
                                                const unsigned int draw_type) {
    std::shared_ptr<VertexArray> vertexArray = std::make_shared<VertexArray>();
    vertexArray->makeSphereMap(resolution, function, draw_type);
    return addVertexArray(vertexArray);
}
const std::shared_ptr<VertexArray> Scene::addVertexArray(const std::string& fileName, const unsigned int renderStrategy) { 
    std::shared_ptr<VertexArray> vertexArray = std::make_shared<VertexArray>(fileName, renderStrategy);
    return addVertexArray(vertexArray);
}

const std::shared_ptr<Light> Scene::addLight(std::shared_ptr<Light> light) { 
    for (int i = 0; i < lights.size(); i++) if (lights[i].get() == light.get() || lights[i] == light) return lights[i];
    light_lookup.emplace(light, lights.size());
    lights.push_back(light); 
    return light; 
}
const std::shared_ptr<Light> Scene::addDirLight(const glm::vec3 dir, 
                                                const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular) { 
    std::shared_ptr<Light> light = std::make_shared<Light>(dir, ambient, diffuse, specular);
    return addLight(light);
}
// create a point light in the scene
const std::shared_ptr<Light> Scene::addPointLight(const glm::vec3 pos, 
                                                  const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, 
                                                  const float constant, const float linear, const float quadratic) { 
    std::shared_ptr<Light> light = std::make_shared<Light>(pos, ambient, diffuse, specular, constant, linear, quadratic);
    return addLight(light);
}
// create a spot light in the scene
const std::shared_ptr<Light> Scene::addSpotLight(const glm::vec3 pos, const glm::vec3 dir, 
                                                 const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, 
                                                 const float constant, const float linear, const float quadratic, 
                                                 const float inner, const float outer) { 
    std::shared_ptr<Light> light = std::make_shared<Light>(pos, dir, ambient, diffuse, specular, constant, linear, quadratic, inner, outer);
    return addLight(light);
}

const std::shared_ptr<Material> Scene::addMaterial(std::shared_ptr<Material> material) {
    for (int i = 0; i < materials.size(); i++) if (materials[i].get() == material.get() || materials[i] == material) return materials[i];
    material_lookup.emplace(material, materials.size());
    materials.push_back(material); 
    return material;
}
const std::shared_ptr<Material> Scene::addBasicMat(const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, const float shininess) { 
    std::shared_ptr<Material> material = std::make_shared<Material>(ambient, diffuse, specular, shininess);
    return addMaterial(material);
}
// create a dMap in the scene by referencing existing textures
const std::shared_ptr<Material> Scene::addDMap(const std::shared_ptr<const TextureGroup> textureGroup, const glm::vec3 specular, const float shininess) {
    std::shared_ptr<Material> material = std::make_shared<Material>(textureGroup->getSlot(DIFFUSE), specular, shininess);
    return addMaterial(material);
}
// create a dMap in the scene by referencing scene-based textures
const std::shared_ptr<Material> Scene::addDMap(const unsigned int texGroupID, const glm::vec3 specular, const float shininess) { 
    std::shared_ptr<Material> material = std::make_shared<Material>(getTextureGroup(texGroupID).getSlot(DIFFUSE), specular, shininess);
    return addMaterial(material);
}
// create a dsMap in the scene by referencing existing textures
const std::shared_ptr<Material> Scene::addDSMap(const std::shared_ptr<const TextureGroup> textureGroup, const float shininess) {
    std::shared_ptr<Material> material = std::make_shared<Material>(textureGroup->getSlot(DIFFUSE), 
                                                                    textureGroup->getSlot(SPECULAR), shininess);
    return addMaterial(material);
}
// create a dsMap in the scene by referencing scene-based textures
const std::shared_ptr<Material> Scene::addDSMap(const unsigned int texGroupID, const float shininess) { 
    std::shared_ptr<Material> material = std::make_shared<Material>(getTextureGroup(texGroupID).getSlot(DIFFUSE), 
                                                                    getTextureGroup(texGroupID).getSlot(SPECULAR), shininess);
    return addMaterial(material);
}
// create a dseMap in the scene by referencing existing textures
const std::shared_ptr<Material> Scene::addDSEMap(const std::shared_ptr<const TextureGroup> textureGroup, const float shininess) { 
    std::shared_ptr<Material> material = std::make_shared<Material>(textureGroup->getSlot(DIFFUSE), 
                                                                    textureGroup->getSlot(SPECULAR), 
                                                                    textureGroup->getSlot(EMISSION), shininess);
    return addMaterial(material);
}
// create a dseMap in the scene by referenceing scene-based textures
const std::shared_ptr<Material> Scene::addDSEMap(const unsigned int texGroupID, const float shininess) {
    std::shared_ptr<Material> material = std::make_shared<Material>(getTextureGroup(texGroupID).getSlot(DIFFUSE), 
                                                                    getTextureGroup(texGroupID).getSlot(SPECULAR), 
                                                                    getTextureGroup(texGroupID).getSlot(EMISSION), shininess);
    return addMaterial(material);
}

const std::shared_ptr<TextureGroup> Scene::addTextureGroup(std::shared_ptr<TextureGroup> textureGroup) { 
    for (int i = 0; i < textureGroups.size(); i++) if (textureGroups[i].get() == textureGroup.get() || textureGroups[i] == textureGroup)
        return textureGroups[i];
    textureGroup_lookup.emplace(textureGroup, textureGroups.size());
    textureGroups.push_back(textureGroup); 
    return textureGroup; 
}
// create texture group in the scene
const std::shared_ptr<TextureGroup> Scene::addTextureGroup(unsigned int firstSlot) {
    std::shared_ptr<TextureGroup> textureGroup = std::make_shared<TextureGroup>(firstSlot);
    return addTextureGroup(textureGroup); 
}
// create new texture group with single member
const std::shared_ptr<TextureGroup> Scene::addTextureGroup(const std::shared_ptr<Texture> texture) { 
    std::shared_ptr<TextureGroup> textureGroup = std::make_shared<TextureGroup>(texture);
    return addTextureGroup(textureGroup);
}

void Scene::load() {
    bool createShadowMaps = false;

    unsigned int l_style;
    bool dir = false, point = false, spot = false;
    for (int l = 0; l < lights.size(); l++)
        switch(getLight(l).type) {
        case L_DIR: { dir = true; } break;
        case L_POINT: { point = true; } break;
        case L_SPOT: { spot = true; } break;
        }
    if (dir) {
        if (point) {
            if (spot) l_style = L_ALL_ENABLED;
            else l_style = L_DIR_POINT;
        } else if (spot) l_style = L_DIR_SPOT;
        else l_style = L_DIR;
    } else if (point) {
        if (spot) l_style = L_POINT_SPOT;
        else l_style = L_POINT;
    } else if (spot) l_style = L_SPOT;
    else l_style = L_DISABLED;

    std::unique_ptr<FrameBuffer> _frameBuffer = nullptr;
    if (aa_enabled)
        _frameBuffer = std::make_unique<FrameBuffer>(FB_ANTI_ALIASING, FRAME_BUFFER_RW, 
                                                     viewportWidth / pixelWidth, viewportHeight / pixelWidth, 
                                                     3, true);
    else if (blur || pixelWidth > 1)
        _frameBuffer = std::make_unique<FrameBuffer>(FB_BASIC, FRAME_BUFFER_RW, 
                                                     viewportWidth / pixelWidth, viewportHeight / pixelWidth, 
                                                     3, true);
    std::unique_ptr<Frame> _frame = (_frameBuffer == nullptr) ? std::make_unique<Frame>() : 
                                                                std::make_unique<Pane>(std::move(_frameBuffer));

    for (int m = 0; m < models.size(); m++) {
        std::shared_ptr<Model> model = models[m];

        unsigned int r_style = model->getType();
        std::shared_ptr<Shader> shader = addShader(r_style, B_COLOR, model->getMaterialType(), 
                                                   (r_style == R_LIGHTING_3D) ? l_style : L_DISABLED, 
                                                   (r_style == R_LIGHTING_3D) ? shadowStyle : S_DISABLED, 
                                                   model->getTextureType(), P_DISABLED);

        std::shared_ptr<RenderGroup> renderGroup = nullptr;
        for (int rg = 0; rg < renderGroups.size(); rg++) if (renderGroups[rg]->getShader() == shader) 
            { renderGroup = renderGroups[rg]; break; }
        if (renderGroup == nullptr) {
            renderGroup = addRenderGroup(shader);
            if (shader->getRenderingStyle() != R_BASIC_2D) {
                addCameraToGroup(renderGroup, camera);
                if (shader->getRenderingStyle() == R_LIGHTING_3D) {
                    for (int l = 0; l < lights.size(); l++) addLightToGroup(renderGroup, lights[l]);
                    if (shader->getShadowStyle() == S_SHADOW_MAPPING) createShadowMaps = true;
                }
            }
            _frame->addRenderGroup(renderGroup);
        }
        addModelToGroup(renderGroup, model);
    }

    if (lights.size() > 0 && createShadowMaps) {
        std::shared_ptr<Shader> sm_shader = addShader(R_BASIC_3D, B_DEPTH, M_DISABLED, L_DISABLED, S_DISABLED, T_DISABLED, P_SHADOW_MAP);

        for (int l = 0; l < lights.size(); l++) {
            std::shared_ptr<Light> light = lights[l];
            int dim = (viewportHeight + viewportWidth) / pixelWidth;
            std::unique_ptr<FrameBuffer> sm_frameBuffer = std::make_unique<FrameBuffer>(FB_DEPTH_MAP, FRAME_BUFFER_RW, dim, dim, 1, true);
            std::unique_ptr<Frame> sm_frame = std::make_unique<Frame>(std::move(sm_frameBuffer));

            std::shared_ptr<RenderGroup> sm_renderGroup = addRenderGroup(l, sm_shader);
            addLightToGroup(sm_renderGroup, light);
            addCameraToGroup(sm_renderGroup, camera);
            for (int m = 0; m < models.size(); m++) if (getModel(m).getType() == R_BASIC_3D || getModel(m).getType() == R_LIGHTING_3D)
                addModelToGroup(sm_renderGroup, m);
            sm_frame->addRenderGroup(sm_renderGroup);

            light->setShadowMapSlot(_frame->addFrame(std::move(sm_frame)));
        }
    }

    if (!_frame->isDefault()) {
        std::unique_ptr<Frame> p_frame = std::make_unique<Frame>();
        DEPTH_TESTING_ENABLED = false;

        unsigned int p_style = P_DISABLED;
        if (blur) p_style = P_BLUR;

        std::shared_ptr<Shader> p_shader = addShader(R_BASIC_2D, B_COLOR, M_DISABLED, L_DISABLED, S_DISABLED, T_BASIC_2D, p_style);

        std::shared_ptr<RenderGroup> p_rg = addRenderGroup(p_shader);
        p_frame->addPane(p_rg, std::move(_frame));
        frame = std::move(p_frame);
    } else {
        DEPTH_TESTING_ENABLED = true;
        frame = std::move(_frame);
    }

    // for each shader group, call the shader's load function
    for (int rg = 0; rg < renderGroups.size(); rg++) { getRenderGroup(rg).load(); }
}

void Scene::draw() {
    for (int l = 0; l < lights.size(); l++) getLight(l).setLightTransform(glm::vec3(0.0f));
    frame->render();
}

void Scene::save(const std::string& file_name) {
    Serializer object;

    for (int rg = 0; rg < renderGroups.size(); rg++) {
        bool add_new = true;
        /*for (int rg_m = 0; rg_m < renderGroups[rg]->nModels(); rg_m++) {
            for (int m = 0; m < models.size(); m++) if (renderGroups[rg]->getModel(rg_m) == models[m]) { add_new = false; break; }
            (add_new) ? addModel(renderGroups[rg]->getModel(rg_m)) : add_new = true;
            for (int m = 0; m < materials.size(); m++) if (renderGroups[rg]->getModel(rg_m)->getMaterial() == materials[m])
                { add_new = false; break; }
            (add_new) ? addMaterial(renderGroups[rg]->getModel(rg_m)->getMaterial()) : add_new = true;
            for (int tg = 0; tg < textureGroups.size(); tg++) if (renderGroups[rg]->getModel(rg_m)->getTextureGroup() == textureGroups[tg])
                { add_new = false; break; }
            (add_new) ? addTextureGroup(renderGroups[rg]->getModel(rg_m)->getTextureGroup()) : add_new = true; 
            for (int va = 0; va < vertexArrays.size(); va++) if (renderGroups[rg]->getModel(rg_m)->getVertexArray() == vertexArrays[va])
                { add_new = false; break; }
            (add_new) ? addVertexArray(renderGroups[rg]->getModel(rg_m)->getVertexArray()) : add_new = true;
        }
        for (int rg_l = 0; rg_l < renderGroups[rg]->nLights(); rg_l++) {
            for (int l = 0; l < lights.size(); l++) if (renderGroups[rg]->getLight(rg_l) == lights[l]) { add_new = false; break; }
            (add_new) ? addLight(renderGroups[rg]->getLight[l]) : add_new = true;
        }*/
    }

    for (int s = 0; s < shaders.size(); s++) object["shaders"][s] = std::move(getShader(s).getJSON());
    for (int m = 0; m < models.size(); m++) object["models"][m] = std::move(getModel(m).getJSON());
    for (int va = 0; va < vertexArrays.size(); va++) object["vertex_arrays"][va] = std::move(getVertexArray(va).getJSON());
    for (int l = 0; l < lights.size(); l++) object["lights"][l] = std::move(getLight(l).getJSON());
    for (int m = 0; m < materials.size(); m++) object["materials"][m] = std::move(getMaterial(m).getJSON());

    //object["frame"] = std::move(frame->getJSON(), *this);
    object.save(SCENE_PATH + file_name);
}
void Scene::print() const {
    std::cout << "Frame: " << &frame << std::endl;
    frame->print(1);
    for (int i = 0; i < shaders.size(); i++) std::cout << "Shader[" << i << "]: " << &getShader(i) << std::endl;
    for (int i = 0; i < models.size(); i++) {
        std::cout << "Model[" << i << "]: " << &getModel(i) << std::endl;
        //models[i].model->print();
    }
    for (int i = 0; i < lights.size(); i++) {
        std::cout << "Light[" << i << "]: " << &getLight(i) << std::endl;
        //lights[i].light->print();
    }
    for (int i = 0; i < vertexArrays.size(); i++) {
        std::cout << "VertexArray[" << i << "]: " << &getVertexArray(i) << std::endl;
        //meshes[i].mesh->print();
    }
    for (int i = 0; i < textureGroups.size(); i++) {
        std::cout << "TextureGroup[" << i << "]: " << &getTextureGroup(i) << std::endl;
        for(int j = 0; j < getTextureGroup(i).size(); j++) 
            std::cout << "\tTexture[" << j << "]: " << getTextureGroup(i).getTexture(j) << std::endl;
    }
    for (int i = 0; i < materials.size(); i++) std::cout << "Material[" << i << "]: " << &getMaterial(i) << std::endl;
}