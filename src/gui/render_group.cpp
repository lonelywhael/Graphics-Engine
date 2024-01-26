#include "gui/render_group.hpp"

RenderGroup::RenderGroup(Format& object) {
    shader = std::make_shared<Shader>(static_cast<Format&>(object["shader"]));
    for (int i = 0; i < object["models"].size(); i++)
        addModel(std::make_shared<Model>(static_cast<Format&>(object["models"][i])));
    for (int i = 0; i < object["lights"].size(); i++)
        addLight(std::make_shared<Light>(static_cast<Format&>(object["lights"][i])));
}

void RenderGroup::addModel(std::shared_ptr<Model> model) {
    // add a pointer to index map and then add the model to the model array
    models.push_back(model);
}
void RenderGroup::addLight(std::shared_ptr<Light> light) {
    // as above
    lights.push_back(light);
}

void RenderGroup::load() {
    renderSequence.push_back(BIND_SHADER);
    switch(getShader()->getRenderingStyle()) {
    case R_BASIC_2D: {
        (getShader()->getTextureStyle() == T_DISABLED) ? modelSequence.push_back(SET_VALUE) : modelSequence.push_back(SET_VALUE_T);
        modelSequence.push_back(RENDER_MODEL);
    } break;
    case R_BASIC_3D: {
        if (getShader()->getPostprocessing() == P_SHADOW_MAP) {
            renderSequence.push_back(TOGGLE_CULLING);
            postRenderSequence.push_back(TOGGLE_CULLING);
        }
        renderSequence.push_back(CALC_TRANS_VP);
        (getShader()->getTextureStyle() == T_DISABLED) ? modelSequence.push_back(SET_VALUE) : modelSequence.push_back(SET_VALUE_T);
        (getShader()->getPostprocessing() == P_SHADOW_MAP) ? modelSequence.push_back(SET_TRANS_SM) : modelSequence.push_back(SET_TRANS);
        modelSequence.push_back(RENDER_MODEL);
    } break;
    case R_LIGHTING_3D: {
        renderSequence.push_back(CALC_TRANS_VP);
        lightSequence.push_back(SET_LIGHT);
        modelSequence.push_back(SET_MATERIAL);
        modelSequence.push_back(SET_TRANS_L);
        if (getShader()->getShadowStyle() == S_SHADOW_MAPPING) {
            lightSequence.push_back(CALC_TRANS_S);
            modelSequence.push_back(SET_TRANS_S);
        }
        modelSequence.push_back(RENDER_MODEL);
    } break;
    case R_SKYBOX: {
        renderSequence.push_back(SET_DEPTH_TEST_LE);
        renderSequence.push_back(CALC_TRANS_VP);
        modelSequence.push_back(SET_TRANS_SKYBOX);
        modelSequence.push_back(RENDER_MODEL);
        postRenderSequence.push_back(SET_DEPTH_TEST_L);
    } break;
    }
}
void RenderGroup::render() {
    // call the render function
    for (void (*r_func)(RenderGroup&) : renderSequence) {
        #if DEBUG_RENDER_FUNCTIONS 
            printFunc((void*) r_func); 
        #endif
        r_func(*this);
    }
    for (int l = 0; l < MAX_LIGHTS; l++) for (void (*l_func)(RenderGroup&, int) : lightSequence) {
        #if DEBUG_RENDER_FUNCTIONS 
            printFunc((void*) l_func); 
        #endif
        l_func(*this, l);
    }
    for (int m = 0; m < models.size(); m++) for (void (*m_func)(RenderGroup&, int) : modelSequence) {
        #if DEBUG_RENDER_FUNCTIONS 
            printFunc((void*) m_func); 
        #endif
        m_func(*this, m);
    }
    for (void (*pr_func)(RenderGroup&) : postRenderSequence) {
        #if DEBUG_RENDER_FUNCTIONS 
            printFunc((void*) pr_func); 
        #endif
        pr_func(*this);
    }
}

Format RenderGroup::getJSON() {
    Format object;
    object["shader"] = std::move(shader->getJSON());
    for (int i = 0; i < models.size(); i++) object["models"][i] = std::move(models[i]->getJSON());
    for (int i = 0; i < lights.size(); i++) object["lights"][i] = std::move(lights[i]->getJSON());

    return object;
}

void RenderGroup::print(int tab) const {
    std::string tabs(tab, '\t');
    std::cout << tabs;
    std::cout << "Shader: " << shader << std::endl;
    for (int i = 0; i < lights.size(); i++) std::cout << tabs << "Light[" << i << "]: " << lights[i] << std::endl;
    for (int i = 0; i < models.size(); i++) std::cout << tabs << "Model[" << i << "]: " << models[i] << std::endl;
}
void RenderGroup::printRenderSequence() const {
    for (void (*r_func)(RenderGroup&) : renderSequence) printFunc((void*) r_func);
    if (lightSequence.size() > 0) {
        std::cout << "for (int l = 0; l < MAX_LIGHTS; l++) {" << std::endl;
        for (void (*l_func)(RenderGroup&, int) : lightSequence) {
            std::cout << "\t";
            printFunc((void*) l_func);
        }
        std::cout << "}" << std::endl;
    }
    if (modelSequence.size() > 0) {
        std::cout << "for (int m = 0; m < models.size(); m++) {" << std::endl;
        for (void (*m_func)(RenderGroup&, int) : modelSequence) {
            std::cout << "\t";
            printFunc((void*) m_func);
        }
        std::cout << "}" << std::endl;
    }
    for (void (*pr_func)(RenderGroup&) : postRenderSequence) printFunc((void*) pr_func);
    std::cout << "------------" << std::endl;
}
void RenderGroup::printFunc(void* func) const {
    if (func == (void*) BIND_SHADER) std::cout << "rg.getShader()->use();" << std::endl;
    else if (func == (void*) SET_DEPTH_TEST_LE) std::cout << "r_SetDepthTest(D_LEQUAL);" << std::endl;
    else if (func == (void*) SET_DEPTH_TEST_L) std::cout << "r_SetDepthTest(D_LESS);" << std::endl;
    else if (func == (void*) TOGGLE_CULLING) std::cout << "r_ToggleFaceCulling();" << std::endl;
    else if (func == (void*) CALC_TRANS_V) std::cout << "rg.setCamView(rg.getCamera()->getView());" << std::endl;
    else if (func == (void*) CALC_TRANS_VP) 
        std::cout << "rg.setCamView(rg.getCamera()->getView());\nrg.setCamProj(rg.getCamera()->getProj());" << std::endl; 
    else if (func == (void*) CALC_TRANS_S) 
        std::cout << "rg.setLightView((l < rg.nLights()) ? rg.getLight(l)->getLightTransform() : glm::mat4(1.0f), l);" << std::endl;
    else if (func == (void*) SET_LIGHT) 
        std::cout << "rg.getShader()->setUniform(\"lightList[\" + std::to_string(l) + \"]\"," << 
                     "(l < rg.nLights()) ? *(rg.getLight(l)) : NULL_LIGHT, rg.getCamView());" << std::endl; 
    else if (func == (void*) SET_MATERIAL) 
        std::cout << "const Material* material = rg.getModel(m)->getMaterial();\n" <<
                     "\tif (material != nullptr) rg.getShader()->setUniform(material);" << std::endl;
    else if (func == (void*) SET_TRANS) 
        std::cout << "rg.getShader()->setUniform(\"clipMat\", rg.getCamProj() * rg.getCamView());" << std::endl;
    else if (func == (void*) SET_TRANS_L)
        std::cout << "glm::mat4 mv = rg.getCamView() * rg.getModel(m)->getModel();\n" <<
                     "\trg.getShader()->setUniform(\"clipMat\", rg.getCamProj() * mv);\n" <<
                     "\trg.getShader()->setUniform(\"viewMat\", mv);\n" <<
                     "\trg.getShader()->setUniform(\"normalMat\", glm::mat3(transpose(inverse(mv))));" << std::endl;
    else if (func == (void*) SET_TRANS_S)
        std::cout << "for (int l = 0; l < MAX_LIGHTS; l++)\n" <<
                     "\t\trg.getShader()->setUniform(\"lightMat[\" + std::to_string(l) + \"]\"," << 
                     "rg.getLightView(l) * rg.getModel(m)->getModel());" << std::endl;
    else if (func == (void*) SET_TRANS_SM)
        std::cout << "rg.getShader()->setUniform(\"clipMat\"," << 
                     "rg.getLight()->getLightTransform() * rg.getModel(m)->getModel());" << std::endl;
    else if (func == (void*) SET_TRANS_SKYBOX)
        std::cout << "rg.getShader()->setUniform(\"clipMat\", rg.getCamProj() * glm::mat4(glm::mat3(rg.getCamView())));" << std::endl;
    else if (func == (void*) SET_VALUE) std::cout << "rg.getShader()->setUniform(\"value\", rg.getModel(m)->getColor());" << std::endl;
    else if (func == (void*) SET_VALUE_T) 
        std::cout << "rg.getShader()->setUniform(\"value\", rg.getModel(m)->getTextureGroup()->getSlot());" << std::endl;
    else if (func == (void*) RENDER_MODEL)
        std::cout << "if (rg.getModel(m)->getVertexArray()->getIndexCount() > 0)\n" << 
                     "\t\tr_DrawIndices(*(rg.getModel(m)->getVertexArray()), *(rg.getShader()), rg.getModel(m)->getTextureGroup());\n" <<                     
                     "\telse r_DrawVertices(*(rg.getModel(m)->getVertexArray()), *(rg.getShader()), rg.getModel(m)->getTextureGroup());" << std::endl;
}


void BIND_SHADER(RenderGroup& rg) { rg.getShader()->use(); }

void SET_DEPTH_TEST_LE(RenderGroup& rg) { r_SetDepthTest(D_LEQUAL); }
void SET_DEPTH_TEST_L(RenderGroup& rg) { r_SetDepthTest(D_LESS); }
void TOGGLE_CULLING(RenderGroup& rg) { r_ToggleFaceCulling(); }

void CALC_TRANS_V(RenderGroup& rg) { rg.setCamView(rg.getCamera()->getView()); }
void CALC_TRANS_VP(RenderGroup& rg) { rg.setCamView(rg.getCamera()->getView()); rg.setCamProj(rg.getCamera()->getProj()); }
void CALC_TRANS_S(RenderGroup& rg, int l)
    { rg.setLightView((l < rg.nLights()) ? rg.getLight(l)->getLightTransform() : glm::mat4(1.0f), l); }

void SET_LIGHT(RenderGroup& rg, int l) 
    { rg.getShader()->setUniform("lightList[" + std::to_string(l) + "]", (l < rg.nLights()) ? *(rg.getLight(l)) : NULL_LIGHT, rg.getCamView()); }
void SET_MATERIAL(RenderGroup&rg, int m) { 
    const std::shared_ptr<Material> material = rg.getModel(m)->getMaterial();
    if (material != nullptr) rg.getShader()->setUniform(material); 
}
void SET_TRANS(RenderGroup& rg, int m) { rg.getShader()->setUniform("clipMat", rg.getCamProj() * rg.getCamView()); }
void SET_TRANS_L(RenderGroup& rg, int m) {
    glm::mat4 mv = rg.getCamView() * rg.getModel(m)->getModel();
    rg.getShader()->setUniform("clipMat", rg.getCamProj() * mv);                 // clipMat goes from mesh to clip space
    rg.getShader()->setUniform("viewMat", mv);                                   // viewMat goes from mesh to view space (for lighting)
    rg.getShader()->setUniform("normalMat", glm::mat3(transpose(inverse(mv))));  // normalMat is used to transform normals (for lighting)
}
void SET_TRANS_S(RenderGroup& rg, int m) {
    // for each light, get the transformation from model to that light's clip space and send it to the shader
    for (int l = 0; l < MAX_LIGHTS; l++)
        rg.getShader()->setUniform("lightMat[" + std::to_string(l) + "]", rg.getLightView(l) * rg.getModel(m)->getModel());
}
void SET_TRANS_SM(RenderGroup& rg, int m) 
    { rg.getShader()->setUniform("clipMat", rg.getLight()->getLightTransform() * rg.getModel(m)->getModel()); }
void SET_TRANS_SKYBOX(RenderGroup& rg, int m) 
    { rg.getShader()->setUniform("clipMat", rg.getCamProj() * glm::mat4(glm::mat3(rg.getCamView()))); }
void SET_VALUE(RenderGroup& rg, int m) { rg.getShader()->setUniform("value", rg.getModel(m)->getColor()); }
void SET_VALUE_T(RenderGroup& rg, int m) { rg.getShader()->setUniform("value", rg.getModel(m)->getTextureGroup()->getSlot()); }

void RENDER_MODEL(RenderGroup& rg, int m) {
    if (rg.getModel(m)->getVertexArray()->getIndexCount() > 0) 
        r_DrawIndices(*(rg.getModel(m)->getVertexArray()), *(rg.getShader()), rg.getModel(m)->getTextureGroup());
    else r_DrawVertices(*(rg.getModel(m)->getVertexArray()), *(rg.getShader()), rg.getModel(m)->getTextureGroup());
}