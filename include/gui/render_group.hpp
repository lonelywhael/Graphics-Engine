#ifndef SHADER_GROUP_HPP
#define SHADER_GROUP_HPP

#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "../io/format.hpp"

#include "camera.hpp"
#include "frame_buffer.hpp"
#include "model.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "elements.hpp"

/* RENDER_GROUP CLASS
 * 
 * The render group class is a container class that groups together shader and various elements that render with the same shader and frame. 
 * Within a single scene, there is typically a 1 to 1 correspondence between shaders and render groups. The render group connects associated 
 * objects (models, lights, a camera) to a shader so that they can efficiently feed information to the uniforms and GPU memory. It also
 * renders to a specified frame buffer, but multiple render groups may render to the same frame buffer.
 * 
 * The render group is created by selecting a shader and frame object, then binding a number of other elements to the object that will be 
 * treated by the shader when it is loaded and rendered to the frame. The render group also needs to be given specific implementations of 
 * the load and render methods, which are highly specific to the chosen shader program. 
 * 
 * Once created, the render group can be easily loaded and rendered by calling the load() and render() methods. 
 * 
 * An example of a render group would be all the light sources in a scene. The shader group would be created with the appropriate shader 
 * designed to render light source objects, and then light models would be added to the group along with a camera object. This particular 
 * group would require the LOAD_DEFAULT() and RENDER_SOURCE() callback functions. With all this information stored in the render group, 
 * calling render() would draw all the light source models to the selected frame.
 */
class RenderGroup {
public:
    RenderGroup(std::shared_ptr<Shader> shader) : shader(shader)
        { for (int l = 0; l < MAX_LIGHTS; l++) l_view.push_back(glm::mat4(1.0f)); }
    RenderGroup(Format& object);

    // models, lights and cameras are specified later
    void addModel(std::shared_ptr<Model> model);
    ///TODO: light slotting should be done by the scene, not the render group 
    void addLight(std::shared_ptr<Light> light);
    void addCamera(std::shared_ptr<Camera> camera) { this->camera = camera; }

    // calling load() or render() simply calls the specified callback function taking the shadergroup object as an argument
    void load();
    void render();

    // retrieve pointers to render group elements
    ///NOTE: all of these should be const pointer consts but temporarily need to keep them available for outside use
    const std::shared_ptr<const Shader> getShader() const { return shader; }
    const std::shared_ptr<const Model> getModel(const unsigned int index = 0) const { return models[index]; }
    unsigned int nModels() const { return models.size(); }
    const std::shared_ptr<const Light> getLight(const unsigned int index = 0) const { return lights[index]; }
    unsigned int nLights() const { return lights.size(); }
    const std::shared_ptr<const Camera> getCamera() const { return camera; }

    glm::mat4 getCamView() const { return c_view; }
    glm::mat4 getCamProj() const { return c_proj; }
    glm::mat4 getLightView(const int l) const { return l_view.at(l); }
    void setCamView(glm::mat4 c_view) { this->c_view = c_view; }
    void setCamProj(glm::mat4 c_proj) { this->c_proj = c_proj; }
    void setLightView(glm::mat4 l_view, const int l) { this->l_view.at(l) = l_view; }

    Format getJSON();

    void print(int tab) const;
    void printRenderSequence() const;
    void printFunc(void*) const;
private:
    unsigned int type;
    // The shader group always needs a shader and at least 1 model. It can also have a framebuffer, lights and a camera.
    ///TODO: divert framebuffer operations to scene
    std::shared_ptr<Shader> shader;
    std::vector<std::shared_ptr<Model>> models;
    std::vector<std::shared_ptr<Light>> lights;
    std::shared_ptr<Camera> camera;

    glm::mat4 c_view, c_proj;
    std::vector<glm::mat4> l_view;

    std::vector<void (*)(RenderGroup&)> renderSequence, postRenderSequence;
    std::vector<void (*)(RenderGroup&, int)> modelSequence, lightSequence;
};

/* RENDER FUNCTIONS
 * 
 * Each shader requires slightly different loading and rendering procedures to properly set up the required uniforms. Listed below are some
 * standard implementations of those functions, which serve as callback functions for the shader group's load and render functions.
 */

extern void BIND_SHADER(RenderGroup& rg);

extern void SET_DEPTH_TEST_LE(RenderGroup& rg);
extern void SET_DEPTH_TEST_L(RenderGroup& rg);
extern void TOGGLE_CULLING(RenderGroup& rg);

extern void CALC_TRANS_V(RenderGroup& rg);
extern void CALC_TRANS_VP(RenderGroup& rg);
extern void CALC_TRANS_S(RenderGroup& rg, int l);

extern void SET_LIGHT(RenderGroup& rg, int l);
extern void SET_MATERIAL(RenderGroup& rg, int m);
extern void SET_TRANS(RenderGroup& rg, int m);
extern void SET_TRANS_L(RenderGroup& rg, int m);
extern void SET_TRANS_S(RenderGroup& rg, int m);
extern void SET_TRANS_SM(RenderGroup& rg, int m); 
extern void SET_TRANS_SKYBOX(RenderGroup& rg, int m);
extern void SET_VALUE(RenderGroup& rg, int m);
extern void SET_VALUE_T(RenderGroup& rg, int m);

extern void RENDER_MODEL(RenderGroup& rg, int m);

#endif