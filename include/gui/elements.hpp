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
    R_BASIC_2D = 0,
    R_BASIC_3D = 1,
    R_LIGHTING_3D = 2,
    R_SKYBOX = 3
};
enum material_styles {
    M_DISABLED = 0,
    M_BASIC = 1,
    M_D_MAP = 2,
    M_DS_MAP = 3,
    M_DSE_MAP = 4
};
enum lighting_styles {
    L_DISABLED = 0,
    L_DIR = 1,
    L_POINT = 2,
    L_SPOT = 3,
    L_DIR_POINT = 4,
    L_DIR_SPOT = 5,
    L_POINT_SPOT = 6,
    L_ALL_ENABLED = 7
};
enum shadow_styles {
    S_DISABLED = 0,
    S_SHADOW_MAPPING = 1
};
enum texture_styles {
    T_DISABLED = 0,
    T_BASIC_2D = 1,
    T_CUBE = 2
};
enum output_buffers {
    B_COLOR = 0,
    B_DEPTH = 1,
    B_STENCIL = 2
};
enum postprocessing {
    P_DISABLED = 0,
    P_BLUR = 1,
    P_DEPTH_MAP = 2,
    P_LINEARIZED_DEPTH_MAP = 3,
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