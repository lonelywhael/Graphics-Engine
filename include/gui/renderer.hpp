#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <iostream>
#include <memory>

#include "model.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "vertex_array.hpp"

/* RENDERER
 * A set of functions that can be used to interface with the openGL rendering pipeline. The functions are used 1) to instruct the openGL
 * to draw something to the frame buffer and 2) to set alter certain rendering parameters used by OpenGL.
 * 
 * The renderer is not a class but rather a mechanism for interfacing with the openGL context. Since there is only one openGL context, it
 * would not make sense to instantiate multiple renderers.
 */

// Instructs openGL to fill the screen with a solid block of a single color (black by default)
extern void r_ClearColorBuffer();
extern void r_ClearDepthBuffer();

// instructs openGL to render vertex data using a specified shader and textures
extern void r_DrawVertices(const VertexArray &vao, const Shader &shader, 
                           const std::shared_ptr<const Texture>* texture = nullptr, const unsigned int nTextures = 0);
extern void r_DrawVertices(const VertexArray &vao, const Shader &shader, 
                           const std::shared_ptr<const TextureGroup> textureGroup);
extern void r_DrawVertices(const VertexArray &vao, const Shader &shader,
                           std::vector<std::shared_ptr<TextureGroup>> textureGroups);
// as above, but uses index data instead of vertex data
extern void r_DrawIndices(const VertexArray &vao, const Shader &shader, 
                          const std::shared_ptr<const Texture>* texture = nullptr, const unsigned int nTextures = 0);
extern void r_DrawIndices(const VertexArray &vao, const Shader &shader, 
                          const std::shared_ptr<const TextureGroup> textureGroup);
extern void r_DrawIndices(const VertexArray &vao, const Shader &shader,
                          std::vector<std::shared_ptr<const TextureGroup>> textureGroups);

// tells openGL context to draw using a depth buffer (for 3D only)
enum depth_tests {          // there are different kinds of depth test rules openGL can use
    D_LESS = GL_LESS,       // check if the current z value is less than the buffered z value
    D_LEQUAL = GL_LEQUAL    // check if the current z value is less than or equal to the buffered z value
};
extern void r_EnableDepthBuffer();
extern void r_DisableDepthBuffer();
extern void r_SetDepthTest(unsigned int depthTest); // set a depth test rule

// tells openGL to allow multisampling (used for anti aliasing)
extern void r_EnableMultisample();
extern void r_DisableMultisample();

// tell openGL to ignore polygons that face in the wrong direction
extern void r_EnableFaceCulling();
extern void r_DisableFaceCulling();
extern void r_ToggleFaceCulling();
extern void r_CullFront();  // cull front facing polygons
extern void r_CullBack();   // cull back facing polygons

// tell openGL not to fill polygons, only render the edges
extern void r_EnableWireframe();

#endif