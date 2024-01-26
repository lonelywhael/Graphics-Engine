#include "gui/renderer.hpp"

void r_ClearColorBuffer() { glClear(GL_COLOR_BUFFER_BIT); }
void r_ClearDepthBuffer() { glClear(GL_DEPTH_BUFFER_BIT); }

void r_DrawVertices(const VertexArray &vao, const Shader &shader, const std::shared_ptr<const Texture>* texture, const unsigned int nTextures) {
    // bind textures, shader, and vertex array
    for (int i = 0; i < nTextures; i++) texture[i]->bind();
    shader.use();
    vao.bind();
    // instruct openGL to draw vertices as triangles
    glDrawArrays(GL_TRIANGLES, 0, vao.getVertexCount());
}
void r_DrawVertices(const VertexArray &vao, const Shader &shader, const std::shared_ptr<const TextureGroup> textureGroup) {
    if (textureGroup != nullptr) textureGroup->bind();
    shader.use();
    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, vao.getVertexCount());
}
void r_DrawVertices(const VertexArray &vao, const Shader &shader, std::vector<std::shared_ptr<const TextureGroup>> textureGroups) {
    for (int i = 0; i < textureGroups.size(); i++) if (textureGroups[i] != nullptr) textureGroups[i]->bind();
    shader.use();
    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, vao.getVertexCount());
}

void r_DrawIndices(const VertexArray &vao, const Shader &shader, const std::shared_ptr<const Texture>* texture, const unsigned int nTextures) {
    // bind textures, shader, and vertex array
    for (int i = 0; i < nTextures; i++) texture[i]->bind();
    shader.use();
    vao.bind();
    // instruct openGL to read off index array to access vertices (still as triangles) rather than accessing vertices directly
    glDrawElements(GL_TRIANGLES, vao.getIndexCount(), GL_UNSIGNED_INT, 0);
}
void r_DrawIndices(const VertexArray &vao, const Shader &shader, const std::shared_ptr<const TextureGroup> textureGroup) {
    if (textureGroup != nullptr) textureGroup->bind();
    shader.use();
    vao.bind();
    glDrawElements(GL_TRIANGLES, vao.getIndexCount(), GL_UNSIGNED_INT, 0);
}
void r_DrawIndices(const VertexArray &vao, const Shader &shader, std::vector<std::shared_ptr<const TextureGroup>> textureGroups) {
    for (int i = 0; i < textureGroups.size(); i++) if (textureGroups[i] != nullptr) textureGroups[i]->bind();
    shader.use();
    vao.bind();
    glDrawElements(GL_TRIANGLES, vao.getIndexCount(), GL_UNSIGNED_INT, 0);
}


void r_EnableDepthBuffer() { glEnable(GL_DEPTH_TEST); }
void r_DisableDepthBuffer() { glDisable(GL_DEPTH_TEST); }
void r_SetDepthTest(unsigned int depthTest) { glDepthFunc(depthTest); }

void r_EnableMultisample() { glEnable(GL_MULTISAMPLE); }
void r_DisableMultisample() { glDisable(GL_MULTISAMPLE); }

bool faceCulling = false;
void r_EnableFaceCulling() {
    glEnable(GL_CULL_FACE);
    // cull back faces by default
    glCullFace(GL_BACK);
    // openGL determines that a polygon is front facing if its vertices are arrange in clockwise order
    ///NOTE: need to be careful to assign vertex values such that their vertices are written clockwise
    glFrontFace(GL_CW);
    faceCulling = true;
}
void r_DisableFaceCulling() {
    glDisable(GL_CULL_FACE);
    faceCulling = false;
}
void r_ToggleFaceCulling() { (faceCulling) ? r_DisableFaceCulling() : r_EnableFaceCulling(); }
void r_CullFront() { glCullFace(GL_FRONT); }
void r_CullBack() { glCullFace(GL_BACK); }

void r_EnableWireframe() { glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); }