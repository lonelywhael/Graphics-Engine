#ifndef FRAME_BUFFER_HPP
#define FRAME_BUFFER_HPP

#include <iostream>
#include <memory>

#include <glad/glad.h>

//implements OpenGL functionality to the specific context of Apple in order to draw windows and such
#include <GLFW/glfw3.h>

#include "../io/format.hpp"

#include "elements.hpp"
#include "renderer.hpp"
#include "texture.hpp"
#include "vertex_array.hpp"
#include "window.hpp"

class RenderBuffer;

/* DEFAULT FRAME
 * 
 * GLFW automatically creates a frame along with a window. In order to render anything to the screen, that frame must be bound. Since that
 * frame does not work light any other frames, it is instead modified through a separate bind function and modify rendering strategies.
 */
extern bool DEPTH_TESTING_ENABLED;          // turns on the depth buffer whenever the default frame is bound
extern bool ANTI_ALIASING_ENABLED;  // currently does nothing, but would turn on anti-aliasing for the default frame
extern void BIND_DEFAULT_FRAME();          // binds the default frame so that any render functions subsequently called will render to the screen
extern void CLEAR_DEFAULT_FRAME();

// Buffer call formats allow frames to be specifically targeted for read and write functions separately
enum frame_buffer_call_formats {
    FRAME_BUFFER_RW = GL_FRAMEBUFFER,       // R/W framebuffers will be targeted by all framebuffer functions
    FRAME_BUFFER_R = GL_READ_FRAMEBUFFER,   // R only framebuffers are targeted by functions that read out data from a frame
    FRAME_BUFFER_W = GL_DRAW_FRAMEBUFFER    // W only framebuffers are targeted by functions that render to a frame
};
// Buffer types specify how the frame buffer should fulfill certain operations
enum frame_buffer_types {
    FB_BASIC = 0,           // basic frame buffers simply render to an output texture
    FB_ANTI_ALIASING = 1,   // anti aliasing buffers apply anti aliasing to the output texture
    FB_DEPTH_MAP = 2        // depth maps output a depth buffer (still in the format of a texture) instead of a color buffer
};

/* FRAME BUFFER CLASS
 * 
 * The frame buffer class allows users to render images to textures and later feed those textures through another rendering pipeline. This 
 * is useful for 2 main reasons:
 *      1. It allows the user to design multiple "passes" at rendering data. This is necessary for all post-processing effects and allows
 *         for the creation and use of maps that might be later used by other rendering pipelines (e.g. a shadowMap is created on the first
 *         pass and then sent to the next frame to be rendered as shadows).
 *      2. It allows the user to easily manage multiple scenes at once, for example to render a mirror or a virtual camera.
 * 
 * Render Groups:
 * Each render group will need one (and only one) framebuffer that it knows it should render to. If not supplied a frame buffer pointer, it 
 * will assume that it should render to the default frame buffer (i.e. appear on screen). However, unlike shaders, multiple render groups
 * can reasonably be assigned to a single frame buffer. 
 * 
 * Textures:
 * One must also take care to consider the texture objects created by the frame buffer. Most frame buffers will create a color buffer
 * texture that can be later used by another render pipeline and can be accessed by the getColorBuffer() method, but other frame buffers
 * may instead generate depth buffers or stencil buffers.
 * 
 * OpenGL:
 * Frame buffer objects are created in parallel to one (or more) OpenGl frame buffer objects. As such, take care not to accidentally copy
 * frame buffer objects because upon deletion they will elminate the parallel OpenGL object.
 */
class FrameBuffer {
public:
    // A frame buffer requires a type, a call format, width, height, number of channels, and whether or not depth buffering is enabled
    FrameBuffer(const unsigned int type, const unsigned int call_format, 
                const int width, const int height, const unsigned int n_channels, 
                const bool depth_enabled);
    FrameBuffer(Format& object);
    // Frame buffers handle several dynamic pointers and have parallel openGL objects that must be disposed of properly on deletion
    ~FrameBuffer();

    // framebuffer objects cannot be copied
    FrameBuffer(const FrameBuffer&) = delete;
    void operator=(const FrameBuffer&) = delete;

    bool operator==(FrameBuffer&);

    // bind the frame buffer to the OpenGL context; must be done in order to render anything to the frame
    void bind() const;

    // clear the frame buffer
    void clear();

    // retrieve informatino about the frame buffer 
    unsigned int getID() const { return frameBufferID; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }

    // retrieve a pointer to the frame buffer's output texture
    std::shared_ptr<Texture> getBuffer() { return t_buffer; }

    // determining which rendering options are enabled for the frame buffer
    bool isDepthEnabled() const { return depth_enabled; }
    bool isAntiAliasingEnabled() const { return (type == FB_ANTI_ALIASING); }

    // apply anti aliasing post-processing to the frame
    void applyAntiAliasing();

    Format getJSON();
private:
    // id used to refer to the parallel openGL frame buffer object
    unsigned int frameBufferID;
    // call format and buffer type values
    unsigned int call_format, type;

    // information about the output texture
    int width, height;
    unsigned int n_channels;
    std::shared_ptr<Texture> t_buffer = nullptr;

    // parameters related to the depth buffer
    bool depth_enabled;
    std::unique_ptr<RenderBuffer> r_buffer;

    // additional objects are needed for anti aliasing frame buffers, see .cpp file for more information
    unsigned int antiAliasedID;
    std::unique_ptr<Texture> intermediateBuffer;

    // specific methods used by the constructor to create the correct frame buffer type
    void initialize();
    void makeBasic();           // sets up a basic frame buffer
    void makeAntiAliasing();    // sets up an anti aliasing frame buffer
    void makeDepthMap();        // sets up a depth map

    // a bind function that allows the openGL object's id to be specified (instead of assuming that it is frameBufferID)
    // used when there are multiple OpenGL frame buffer objects
    void bind(unsigned int id) const;

    // checks the status of a frame buffer object and returns an explanation of the error when it is missing attachments
    bool checkStatus() const;
};

//----------------------------------------------------------------------------------------------------------------------------------------

/* RENDER BUFFER CLASS
 * 
 * The render buffer class is basically a write-only version of the texture class that is slightly more optimized for writing than textures.
 * A render buffer should be used in place of a texture when there is no need to refer back to data produced in later render passes. For
 * example, depth data is needed in order to properly render a 3D frame, but the depth data (normally) does not need to be passed on to the 
 * next rendering pipeline. Therefore, the depth data should be stored in a render buffer rather than a texture.
 * 
 * OpenGL:
 * Like a texture, render buffers are held in a one to one correspondence with an openGL context object and therefore should not be copied,
 * as stated above for frame buffers.
 */
class RenderBuffer {
public:
    // need to tell the render buffer the width and height of the window and tell it whether or not to allow anti aliasing
    RenderBuffer(const int width, const int height, const bool antiAliasing);
    // need to delete the associated openGL context object along with the render buffer object
    ~RenderBuffer() {
        glDeleteRenderbuffers(1, &renderBufferID);
        #if DEBUG_OPENGL_OBJECTS
            std::cout << "Render Buffer " << renderBufferID << " was deleted." << std::endl;
        #endif
    }

    // bind and unbind the render buffer from the openGL context
    void bind() { glBindRenderbuffer(GL_RENDERBUFFER, renderBufferID); }
    void unbind() const { glBindRenderbuffer(GL_RENDERBUFFER, 0); }

    // return the id of the render buffer
    unsigned int getID() const { return renderBufferID; }
private:
    // the id associated with the openGL render buffer object
    unsigned int renderBufferID;
};

#endif