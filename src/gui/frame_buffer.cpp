#include "gui/frame_buffer.hpp"

// by default, depth testing and anti aliasing are disabled
bool DEPTH_TESTING_ENABLED = false;
bool ANTI_ALIASING_ENABLED = false;
void BIND_DEFAULT_FRAME() {
    // change the openGL viewport size to the size of the currently bound window
    glViewport(0, 0, boundWindow->getWidth(), boundWindow->getHeight());
    // if anti-aliasing is enabled, tell GLFW to render frames with the proper number of samples, otherwise tell it not to do that
    if (ANTI_ALIASING_ENABLED) glfwWindowHint(GLFW_SAMPLES, ANTI_ALIASING_SAMPLE_SIZE);
    else glfwWindowHint(GLFW_SAMPLES, 0);
    // bind the default frame buffer (id = 0) to the openGL context
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void CLEAR_DEFAULT_FRAME() {
    BIND_DEFAULT_FRAME();
    r_ClearColorBuffer();                               // always clear the default screen's color buffer
    if (DEPTH_TESTING_ENABLED) r_ClearDepthBuffer();    // only clear the default screen's depth buffer if it is enabled
}

FrameBuffer::FrameBuffer(const unsigned int type, const unsigned int call_format, 
                         const int width, const int height, const unsigned int n_channels, 
                         const bool depth_enabled) 
        : type(type), call_format(call_format), 
          width(width), height(height), n_channels(n_channels), 
          depth_enabled(depth_enabled) 
    { initialize(); }
FrameBuffer::FrameBuffer(Format& object) 
        : type(static_cast<unsigned int>(object["type"])), call_format(static_cast<unsigned int>(object["call_format"])),
          width(static_cast<int>(object["width"])), height(static_cast<int>(object["height"])), n_channels(static_cast<unsigned int>(object["n_channels"])),
          depth_enabled(static_cast<bool>(object["depth_enabled"])) 
    { initialize(); }
void FrameBuffer::initialize() {
    // generate the openGL context frame buffer object
    glGenFramebuffers(1, &frameBufferID);
    #if DEBUG_OPENGL_OBJECTS 
        std::cout << "Frame Buffer " << frameBufferID << " was created." << std::endl;
    #endif

    // based on the frame buffer type, call a different method to finish constructing the frame buffer
    switch(type) {
    case FB_ANTI_ALIASING: { makeAntiAliasing(); } break;
    case FB_DEPTH_MAP: { makeDepthMap(); } break;
    default: { makeBasic(); } break;
    }
}
void FrameBuffer::makeBasic() {
    /* For a basic frame buffer, create a color buffer texture and attach it to the frame to be written to, and if depth testing is
     * enabled, then create a render buffer for depth values that is not intended to be used after the the current pass.
     */
    // bind the frame
    bind();
    // create a texture for color values and attach it to the frame
    unsigned int component = (n_channels == 3) ? COLOR_BUFFER : ALPHA_BUFFER;
    t_buffer = std::make_shared<Texture>(TEXTURE_2D, width, height, component, FILTER_NEAREST, WRAPPER_CLAMP_TO_EDGE, 0);
    glFramebufferTexture2D(call_format, GL_COLOR_ATTACHMENT0, TEXTURE_2D, t_buffer->getID(), 0);

    // if depth testing is enabled, create a render buffer and attach it to the frame
    if (depth_enabled) {
        r_buffer = std::make_unique<RenderBuffer>(width, height, false);
        glFramebufferRenderbuffer(call_format, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, r_buffer->getID());
    }
    // here, the frame buffer should have all the necessary attachments, so check for errors
    checkStatus();
}
void FrameBuffer::makeAntiAliasing() {
    /* For anti aliasing frame buffers, the process is more complicated, with two openGL frames actually built into a single frame buffer
     * object. The way anti aliasing works is that a texture (referred to by "frameBufferID") is rendered using a special "multisample" 
     * setting. OpenGL has a built in function for performing the anti aliasing post processing on this texture and sends the output to a 
     * second texture (referred to by "antiAliasedID"). 
     * 
     * When the bind() function is called externally, it will bind the OpenGL frame associated with frameBufferID. This frame is attached
     * to the texture called intermediateBuffer. When the applyAntiAliasing() function is called, openGL will perform the anti aliasing
     * and copy the output to the colorBuffer. When the getColorBuffer() function is called externally, the colorBuffer is returned.
     */
    // bind the frame that will be the target of all rendering operations
    bind(frameBufferID);
    // create a texture, separate from the final output texture, that will store samples for anti aliasing
    unsigned int component = (n_channels == 3) ? COLOR_BUFFER : ALPHA_BUFFER;
    intermediateBuffer = std::make_unique<Texture>(TEXTURE_2D_AA, width, height, component, FILTER_NEAREST, WRAPPER_CLAMP_TO_EDGE, 0);
    glFramebufferTexture2D(call_format, GL_COLOR_ATTACHMENT0, TEXTURE_2D_AA, intermediateBuffer->getID(), 0);
    // if depth testing is enabled, create a depth render buffer and bind it to the frame
    if (depth_enabled) {
        r_buffer = std::make_unique<RenderBuffer>(width, height, true);
        glFramebufferRenderbuffer(call_format, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, r_buffer->getID());
    }
    // the first frame should be complete, check for errors
    checkStatus();

    // create a second frame that will perform post processing on the output of the first frame
    glGenFramebuffers(1, &antiAliasedID);
    #if DEBUG_OPENGL_OBJECTS 
        std::cout << "Frame Buffer " << antiAliasedID << " was created." << std::endl;
    #endif

    // bind the second frame
    bind(antiAliasedID);
    // create a texture that will the target of post processing functions and will be read from externally
    t_buffer = std::make_shared<Texture>(TEXTURE_2D, width, height, component, FILTER_NEAREST, WRAPPER_CLAMP_TO_EDGE, 0);
    glFramebufferTexture2D(call_format, GL_COLOR_ATTACHMENT0, TEXTURE_2D, t_buffer->getID(), 0);
    // this frame doesn't need a depth buffer because it is simply processing data that has already been fully rendered
    // should be complete, check for errors
    checkStatus();
}
void FrameBuffer::makeDepthMap() {
    /* For depth maps, there is no need for a color buffer. Instead, a single channel texture is created to store depth data. This texture
     * can be accessed by calling the getDepthBuffer() function.
     */
    // bind the frame
    bind();
    // create a texture that will store depth data
    t_buffer = std::make_shared<Texture>(TEXTURE_2D, width, height, DEPTH_BUFFER, FILTER_NEAREST, WRAPPER_CLAMP_TO_BORDER, 0);
    // by using a border and setting its "color" like so, we can provide an easy test for shaders to determine if we have left the
    // the map's fustrum
    t_buffer->setBorderColor(1.0f, 1.0f, 1.0f, 1.0f);
    glFramebufferTexture2D(call_format, GL_DEPTH_ATTACHMENT, TEXTURE_2D, getBuffer()->getID(), 0);
    // since openGL's frames explicitly require color buffers to be attached, we need to tell it that we are not going to be using one
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    // the texture should be complete, check for errors
    checkStatus();
}

FrameBuffer::~FrameBuffer() {
    //need to delete the openGL frame object and the color buffer
    glDeleteFramebuffers(1, &frameBufferID);
    #if DEBUG_OPENGL_OBJECTS 
        std::cout << "Frame Buffer " << frameBufferID << " was deleted." << std::endl;
    #endif

    // other dynamic objects may have been created depending on the type of frame created
    switch(type) {
    // anti aliasing frames create an extra openGL frame and an extra texture, and may have a depth render buffer
    case FB_ANTI_ALIASING: {
        glDeleteFramebuffers(1, &antiAliasedID);
        #if DEBUG_OPENGL_OBJECTS
            std::cout << "Frame Buffer " << antiAliasedID << " was deleted." << std::endl;
        #endif
    } break;
    }
}

bool FrameBuffer::operator==(FrameBuffer& other) { return frameBufferID == other.frameBufferID; }

void FrameBuffer::bind() const {
    // set the openGL viewport to have the width and height of the frame in use
    glViewport(0, 0, width, height);
    // by default, bind the frame attached to frameBufferID
    glBindFramebuffer(call_format, frameBufferID);
}
void FrameBuffer::bind(unsigned int id) const {
    // identicle to the above, except the id of the frame can be specified
    glViewport(0, 0, width, height);
    glBindFramebuffer(call_format, id);
}

void FrameBuffer::clear() {
    // clearing buffers is an operation specific to the type of frame in use
    switch(type) {
    case FB_ANTI_ALIASING: {
        // anti aliasing frames need to clear two frames, the first has a color buffer and may have a depth buffer, second has a color buffer
        bind(frameBufferID);
        r_ClearColorBuffer();
        if (depth_enabled) r_ClearDepthBuffer();
        bind(antiAliasedID);
        r_ClearColorBuffer();
    } break;
    case FB_DEPTH_MAP: {
        // depth maps only have a depth buffer
        bind();
        r_ClearDepthBuffer();
    } break;
    default: {
        // basic frams have color buffers and may have depth buffers
        bind();
        r_ClearColorBuffer();
        if (depth_enabled) r_ClearDepthBuffer();
    } break;
    }
}

void FrameBuffer::applyAntiAliasing() {
    // apply anti aliasing post processing to intermediateBuffer and send the data to colorBuffer
    if (type == FB_ANTI_ALIASING) {
        glBindFramebuffer(FRAME_BUFFER_R, frameBufferID);  // will read from frameBufferID frame (attached to intermediateBuffer)
        glBindFramebuffer(FRAME_BUFFER_W, antiAliasedID);  // will write to antiAliasedID frame (attached to colorBuffer)
        // perform the openGL post processing function, known as "blitting"
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }
}

bool FrameBuffer::checkStatus() const { 
    // check the frame buffer for errors
    auto fboError = glCheckFramebufferStatus(call_format);
    // The error codes below are from https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCheckFramebufferStatus.xhtml
    if (fboError != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAME_BUFFER::FRAME_BUFFER_NOT_COMPLETE::";
        switch(fboError) {
        case GL_FRAMEBUFFER_UNDEFINED: { 
            std::cout << "GL_FRAMEBUFFER_UNDEFINED" << std::endl;
            std::cout << "The specified framebuffer is the default read or draw framebuffer, but the default framebuffer does not exist."; 
        } break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: {
            std::cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" << std::endl;
            std::cout << "One of the framebuffer attachment points are framebuffer incomplete.";
        } break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: {
            std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" << std::endl;
            std::cout << "The framebuffer does not have at least one image attached to it.";
        } break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: {
            std::cout << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER" << std::endl;
            std::cout << "The value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment point(s) named by " << 
                         "GL_DRAW_BUFFERi.";
        } break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: {
            std::cout << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER" << std::endl;
            std::cout << "GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color " << 
                         "attachment point named by GL_READ_BUFFER.";
        } break;
        case GL_FRAMEBUFFER_UNSUPPORTED: {
            std::cout << "GL_FRAMEBUFFER_UNSUPPORTED" << std::endl;
            std::cout << "The combination of internal formats of the attached images violates an implementation-dependent set of " << 
                         "restrictions.";
        } break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: {
            std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE" << std::endl;
            std::cout << "The value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers; if the value of " << 
                         "GL_TEXTURE_SAMPLES is the not same for all attached textures; or, if the attached images are a mix of " <<
                         "renderbuffers and textures, the value of GL_RENDERBUFFER_SAMPLES does not match the value of GL_TEXTURE_SAMPLES; " << 
                         "of the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures; or, if the attached " << 
                         "images are a mix of renderbuffers and textures, the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not GL_TRUE for " <<
                         "all attached textures";
        } break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: {
            std::cout << "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS" << std::endl;
            std::cout << "At least one framebuffer attachment is layered, and any populated attachment is not layered, or if all populated " << 
                         "color attachments are not from textures of the same target.";
        } break;
        }
    }
    return fboError;
}

Format FrameBuffer::getJSON() {
    Format object = Format();
    object["type"] = type;
    object["call_format"] = call_format;
    object["width"] = width;
    object["height"] = height;
    object["n_channels"] = n_channels;
    object["depth_enabled"] = depth_enabled;

    return object;
}

//-------------------------------------------------------------------------------------------------------

RenderBuffer::RenderBuffer(const int width, const int height, const bool antiAliasing) { 
    // create an openGL render buffer object
    glGenRenderbuffers(1, &renderBufferID); 
    #if DEBUG_OPENGL_OBJECTS 
        std::cout << "Render Buffer " << renderBufferID << " was created." << std::endl;
    #endif

    // bind the render buffer to the openGL context
    bind();
    // if anti aliasing, generate a renderbuffer that can handle multisampling
    if (antiAliasing) glRenderbufferStorageMultisample(GL_RENDERBUFFER, ANTI_ALIASING_SAMPLE_SIZE, GL_DEPTH24_STENCIL8, width, height);
    // otherwise, just create a normal render buffer
    else glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
}