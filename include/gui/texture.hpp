#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <iostream>
#include <memory>
#include <string.h>
#include <vector>

#include <glad/glad.h>

#include "stb_image/stb_image.h" // API used to read data from image files

#include "io/serializer.hpp"

#include "elements.hpp"

// texture formats encode for OpenGL what kind of texture structure to use
enum texture_formats {
    TEXTURE_2D = GL_TEXTURE_2D,                 // texture_2D refers to normal 2D textures
    TEXTURE_2D_AA = GL_TEXTURE_2D_MULTISAMPLE,  // texture_2D_aa refers to normal 2D textures that allow multisampling for anti aliasing
    TEXTURE_CUBE = GL_TEXTURE_CUBE_MAP          // texture_cube refers to a cube map texture (6 2D textures arranged in a cube)
};
// indicates what kind of buffer the texture is (depth or color)
enum buffer_component {
    COLOR_BUFFER = GL_RGB,
    ALPHA_BUFFER = GL_RGBA,
    DEPTH_BUFFER = GL_DEPTH_COMPONENT
};
// Texture type indicates how the texture will be used
enum texture_type {
    DIFFUSE = 0,    // diffuse maps are color buffers that carry diffuse lighting data
    SPECULAR = 1,   // specular maps are color buffers that carry specular lighting data
    EMISSION = 2,   // emission maps are color buffers that carry emssion lighting data
};
// Filters determine how OpenGL will fill space when the resolution of a texture is too low to be properly rendered
enum filter_type {
    FILTER_LINEAR = GL_LINEAR,
    FILTER_NEAREST = GL_NEAREST
};
// Wrappers determine how OpenGL will fill space when a texture coordinate falls outside of [0, 1]
enum wrapper_type {
    WRAPPER_REPEAT = GL_REPEAT,
    WRAPPER_MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
    WRAPPER_CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
    WRAPPER_CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER
};
// Mipmaps are smaller versions of textures that OpenGL will refer to when rendering small textures to avoid unintended visual artifacts
enum mipmap_type {
    MIPMAP_LINEAR = 1,
    MIPMAP_NEAREST = 2
};

/* TEXTURE CLASS
 * 
 * The texture class is used to interface with stored texture data in the openGL context and give OpenGL instructions on how to render
 * those textures. The texture class requires a path to a texture image file and several rendering parameters.
 * 
 * Like other interfaces with the openGL context, textures should be held in a strict 1 to 1 correspondence with OpenGL texture objects.
 * Textures should not be copied but instead passed by reference or pointer.
 */
class Texture {
public:
    /* Create an empty texture. Requires a format value, width, height, component (depth or color), 
     * filter, wrapper, and mipmap settings. 
     */
    Texture(const unsigned int textureFormat, 
            const int width, const int height, const unsigned int component, 
            const unsigned int filter, const unsigned int wrapper, const unsigned int mipmap);
    /* Create a texture with data from file. Requires a format value, file name and extension (.png, .jpg, etc.), a slot, 
     * and filter, wrapper, and mipmap settings 
     */
    Texture(const unsigned int textureFormat, const std::string fileName, const std::string extn,
            const unsigned int filter, const unsigned int wrapper, const unsigned int mipmap);
    Texture(const unsigned int textureFormat, const std::string fileName, const std::string extn,
            const unsigned int filter, const unsigned int wrapper, const unsigned int mipmap, 
            const int slot);
    Texture(Serializer object);
    // A destructor is required to delete the associated OpenGL texture object when this object is deleted
    ~Texture();

    Texture(const Texture&) = delete;
    void operator=(const Texture&) = delete;

    bool operator==(const Texture&) const;
    bool operator!=(const Texture&) const;

    // Bind/unbind the texture to/from the openGL context
    void bind() const;
    void unbind() const;

    // A series of functions that can be used to alter the rendering parameters of a texture
    void createMipmap(const unsigned int value);    // auto generate a mipmap for a texture that doesn't have one
    void setFilter(const unsigned int value);       // change the filter parameter
    void setWrapper(const unsigned int value);      // change the wrapper parameter

    // set the border wrapper's color
    void setBorderColor(const float r, const float g, const float b, const float a) { 
        float borderColor[] = { r, g, b, a };
        setParameter(GL_TEXTURE_BORDER_COLOR, borderColor);
    }

    // returns the texture slot of a texture. OpenGL can slot up to 16 textures at once.
    int getSlot() const { return slot; }
    unsigned int getID() const { return textureID; }
    unsigned int getType() const { return textureType; }

    // set the texture's slot
    void setSlot(const int slot) { this->slot = slot; }

    Serializer getJSON() const;

    void print();

private:
    unsigned int textureID;         // openGL id signifier
    unsigned int textureFormat;     // GL_TEXTURE_2D, 3D, etc.
    unsigned int textureType;       // T_BASIC_2D, T_CUBE, etc.

    std::string file_name, extension;

    int width, height, nChannels;   // width, height, and number of color channels of the texture file

    int slot;                       // indentifier that can be given to shaders to tell them to use the appropriate texture data

    unsigned int filter, wrapper, mipmap = 0;    // linear or nearest

    // used to generically set specific non-visible parameter types
    void setParameter(const unsigned int option, const int value) { glTexParameteri(textureFormat, option, value); }
    void setParameter(const unsigned int option, const float value[4]) { glTexParameterfv(textureFormat, option, value); }
};

//------------------------------------------------------------------------------------------------------------------------------------------

/* TEXTURE GROUP CLASS
 * 
 * The texture group class is used to properly slot and bind textures with little effort. The user simply needs to add textures to the
 * group, then when the group is bound, all the textures will be bound to sequential slots. For further manageability, the starting slot
 * can optionally be set manually.
 * 
 * Each texture should only be in a single texture group. A texture group should be considered as the natural grouping of given textures.
 * For example, a specular map is naturally paired with a diffuse map.
 */
class TextureGroup {
public:
    // can create an empty texture group or initialize it with a single member (as many texture groups will only have a single member)
    TextureGroup(const unsigned int firstSlot = 0) : nextSlot(firstSlot) {}
    TextureGroup(const std::shared_ptr<Texture> texture, const unsigned int firstSlot = 0) 
            : nextSlot(firstSlot) { addTexture(texture); }
    TextureGroup(Serializer& object);

    bool operator==(TextureGroup& other);
    bool operator==(Texture& other);

    // add an existing texture to the texture group, setting its slot accordingly
    int addTexture(const std::shared_ptr<Texture> texture) { 
        texture->setSlot(nextSlot++); 
        textures.emplace_back(texture); 
        return nextSlot - 1;
    }
    // create a new texture and add it to the texture group
    int addTexture(const std::string& fileName, const std::string& extension, 
                   const unsigned int filter, const unsigned int wrapper, const unsigned int mipmap) { 
        textures.emplace_back(std::make_shared<Texture>(TEXTURE_2D, fileName, extension, filter, wrapper, mipmap, nextSlot++)); 
        return nextSlot - 1;
    }

    // retrieve a point to the texture in the group
    const std::shared_ptr<const Texture> getTexture() const { return getTexture(0); }
    const std::shared_ptr<const Texture> getTexture(const unsigned int index) const { return textures[index]; }
    // retrieve the slot of a texture in the group
    int getSlot(const unsigned int index = 0) const { return textures[index]->getSlot(); }
    unsigned int getType() const { return (getTexture() != nullptr) ? getTexture()->getType() : T_DISABLED; }

    // retrieve the number of textures in the texture group
    unsigned int size() const { return textures.size(); }

    // bind all members of the texture group
    void bind() const { for (int t = 0; t < textures.size(); t++) textures[t]->bind(); }

    Serializer getJSON();

private:
    // need to tell the destructor whether to delete the textures or not, so pair each texture with a bool value
    std::vector<std::shared_ptr<const Texture>> textures = {};

    // value of the next slot, increments each time a new texture is added
    unsigned int nextSlot = 0;
};


#endif