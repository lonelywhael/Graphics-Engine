#ifndef ELEMENTS_HPP
#define ELEMENTS_HPP

#include <iostream>

#include <glm/glm.hpp>

// information about the version and profile of OpenGL being used
enum profiles {
    P_CORE = 0
};
extern const float VERSION;
extern const unsigned int PROFILE;

// SHADER FORMATTING RULES
enum rendering_styles {
    // Renders in 2D
    R_BASIC_2D = 0,
    // Renders in 3D with no lighting
    R_BASIC_3D = 1,
    // Renders in 3D with lighting
    R_LIGHTING_3D = 2,
    // Renders a 3D cubemap that has orientation but no spatial position, just surrouds the viewer
    R_SKYBOX = 3
};
enum material_styles {
    // No materials style, default setting when lighting is not enabled
    M_DISABLED = 0,
    // Materials are a constant color regardless of lighting (recommended for light textures)
    M_BASIC = 1,
    // Materials have a specified diffusive lighting value (have much they appear lit by a light)
    M_D_MAP = 2,
    // Mateirals have both diffusive and specular lighting values (can also reflect light)
    M_DS_MAP = 3,
    // Materials have diffusive, specular, and emmissive lighting values (can emit light)
    M_DSE_MAP = 4
};
enum lighting_styles {
    // No lighting
    L_DISABLED = 0,
    // Only directional lighting (light source is infinitely far away in a specified direction)
    L_DIR = 1,
    // Only point lighting (light source has a spatial orientation and decays with distance)
    L_POINT = 2,
    // Only spot lighting (like point lighting, but only shines in a specified direction or range of directions)
    L_SPOT = 3,
    // Combinations of multiple kinds of lighting (more expensive)
    L_DIR_POINT = 4,
    L_DIR_SPOT = 5,
    L_POINT_SPOT = 6,
    L_ALL_ENABLED = 7
};
enum shadow_styles {
    // No shadows
    S_DISABLED = 0,
    // Shadow mapping, where shadows are mapped by considering the viewpoint of each light source
    S_SHADOW_MAPPING = 1
};
enum texture_styles {
    // No textures
    T_DISABLED = 0,
    // 2D textures
    T_BASIC_2D = 1,
    // 6 2D textures stitched together into a cube shape
    T_CUBE = 2
};
enum output_buffers {
    // Shader outputs a 3 dimensional color buffer
    B_COLOR = 0,
    // Shader outputs a 1 dimensional depth/z-buffer
    B_DEPTH = 1,
    // Shader ouputs a 1 dimensional stencil buffer
    B_STENCIL = 2
};
enum postprocessing {
    // No postprocessing
    P_DISABLED = 0,
    // Adds a blur postprocessing effect
    P_BLUR = 1,
    // Converts a depth buffer to a black and white color buffer for visualization purposes or fog
    P_DEPTH_MAP = 2,
    // An alternate depth mapping strategy that adjust z-values to a better visual scale
    P_LINEARIZED_DEPTH_MAP = 3,
    // Generates a shadow map for each light source that is used when rendering shadows in a lit scene
    P_SHADOW_MAP = 4
};

// DEBUG RULES

// adds messages whenever OpenGL context objects are created or destroyed, useful for determining whether accidental copies are being made
#define DEBUG_OPENGL_OBJECTS false
#define DEBUG_SHADER_BUILDER_SHOW_UNUSED_VARS false
#define DEBUG_RENDER_FUNCTIONS false

extern void printMat(glm::mat4 mat);        // prints a metrix to the screen

// DEFAULT VALUES
#define FRAME_SLOT 3

#define ANTI_ALIASING_SAMPLE_SIZE 4

/* ELEMENTS
 *
 * Several container structs arrange several pieces of data in different locations and thus need to reference to the existance of certain
 * objects without needing to know what is in them. This header allows those classes to be declared in one place to avoid linking issues.
 */

class Scene;

class RenderGroup;
class Frame;

class FrameBuffer;

class Camera;

class VertexArray;
class Model;            // Stores all data needed to render a single object

struct Material;        // stores material data in format used by shaders (how is the object lighted? what color is it?)

struct Light;           // stores light data in format used by shaders (what color is the light? how does it behave?)

class Shader;           // compiles shader and facilitates loading and rendering

class Texture;          // stores texture data and facilitates loading and rendering

#endif