#include "gui/texture.hpp"

std::string TEXTURE_PATH = "../res/textures/";
std::string BASIC_PATH = "basic/";
std::string CUBE_MAP_PATH = "cubemaps/";
std::string CUBE_MAP_PATHS[8] { "right", "left", "top", "bottom", "front", "back" };

// quick and dirty conversion from number of channels to image data format (probably doesn't always work, be careful with image types)
const unsigned int COLOR_FORMAT[] {0, GL_DEPTH_COMPONENT, 0, GL_RGB, GL_RGBA}; 
// for now, only handles 2D textures
const unsigned int DEFAULT_TYPE = GL_TEXTURE_2D;

Texture::Texture(const unsigned int textureFormat, const int width, const int height, const unsigned int component,
                 const unsigned int filter, const unsigned int wrapper, const unsigned int mipmap) 
            : textureFormat(textureFormat), slot(0), width(width), height(height) {
    // based on the component, the number of color channels can be determined
    switch(component) {
    case DEPTH_BUFFER: { nChannels = 1; } break;
    case COLOR_BUFFER: { nChannels = 3; } break;
    case ALPHA_BUFFER: { nChannels = 4; } break;
    }

    // generate a texture object in the OpenGL context save the id
    glGenTextures(1, &textureID);
    #if DEBUG_OPENGL_OBJECTS 
        std::cout << "Texture " << textureID << " was created." << std::endl;
    #endif

    // bind the texture to the context
    glBindTexture(textureFormat, textureID);

    // depending on the texture format, we need to call different openGL commands
    switch(textureFormat) {
    // in the case of multisampling, there is a special openGL function that takes the sample size
    case TEXTURE_2D_AA: { 
        glTexImage2DMultisample(textureFormat, ANTI_ALIASING_SAMPLE_SIZE, component, width, height, GL_TRUE); 
        textureType = T_BASIC_2D;
    } break;
    // otherwise, call the normal 2D texture function
    default: { 
        glTexImage2D(textureFormat, 0, component, width, height, 0, component, GL_UNSIGNED_BYTE, NULL); 
        textureType = T_CUBE;
    } break;
    }

    // set texture parameters
    if (mipmap) createMipmap(mipmap);
    if (filter) setFilter(filter);
    if (wrapper) setWrapper(wrapper);

    // unbind the texture
    glBindTexture(textureFormat, 0);
}
Texture::Texture(const unsigned int textureFormat, const std::string file_name, const std::string extn, 
                 const unsigned int filter, const unsigned int wrapper, const unsigned int mipmap) 
        : Texture(textureFormat, file_name, extn, filter, wrapper, mipmap, 0) {}
Texture::Texture(const unsigned int textureFormat, const std::string file_name, const std::string extn, 
                 const unsigned int filter, const unsigned int wrapper, const unsigned int mipmap, 
                 const int slot) {
    this->textureFormat = textureFormat;
    this->file_name = file_name;
    this->extension = extn;
    this->slot = slot;
    // add the texture directory path to the file name (each format has its own filepath)
    
    std::vector<std::string> filePaths;
    switch(textureFormat) {
    case TEXTURE_2D: {
        // ../res/textures/basic/(fileName).(extension)
        std::string file_path = TEXTURE_PATH + BASIC_PATH + file_name + extn;
        filePaths.push_back(file_path);
        textureType = T_BASIC_2D;
    } break;
    case TEXTURE_CUBE: {
        // cube maps require 6 image files (one for each face)
        for (int i = 0; i < 6; i++) {
            // ../res/textures/cubemaps/(fileName)/(front, back, right, etc.).(extension)
            std::string file_path = TEXTURE_PATH + CUBE_MAP_PATH + file_name + "/" + CUBE_MAP_PATHS[i] + extn;
            filePaths.push_back(file_path);
        }

        textureType = T_CUBE;
    } break;
    }

    // OpenGL considers (0, 0) to be the bottom left corner, but most image types considered (0, 0) to be the top left corner, so flip
    if (textureFormat == TEXTURE_CUBE) stbi_set_flip_vertically_on_load(false); // but for some reason OpenGL changes its mind for cube maps
    else stbi_set_flip_vertically_on_load(true);

    // generate a texture object in the OpenGL context save the id
    glGenTextures(1, &textureID);
    #if DEBUG_OPENGL_OBJECTS
        std::cout << "Texture " << textureID << " was created." << std::endl;
    #endif

    // bind the texture to the context
    glBindTexture(textureFormat, textureID);

    // generate a texture for each file, but all associated with the same openGL texture object
    for (int i = 0; i < filePaths.size(); i++) {
        // load image data from file
        unsigned char* data = stbi_load(filePaths[i].c_str(), &width, &height, &nChannels, 0);
        if (data != nullptr) {
            unsigned int format;
            // set the texture format value
            switch(textureFormat) {
            case TEXTURE_CUBE: { format = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i; } break;  // openGL uses 6 separate formats for each face
            default: { format = textureFormat; } break;
            }
            // load the image data and information about its format to the OpenGL texture object and then free the data
            glTexImage2D(format, 0, COLOR_FORMAT[nChannels], width, height, 0, COLOR_FORMAT[nChannels], GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {    
            // if data is null, then file was not read correctly, throw error
            std::cout << "\nError: Failed to load texture" << std::endl;
            std::cout << stbi_failure_reason() << std::endl;
        }
    }

    // set texture parameters
    if (mipmap) createMipmap(mipmap);
    if (filter) setFilter(filter);
    if (wrapper) setWrapper(wrapper);

    // unbind the texture
    glBindTexture(textureFormat, 0);
}
Texture::Texture(Serializer object) 
    : Texture(static_cast<unsigned int>(object["texture_format"]), 
              static_cast<std::string>(object["file_name"]), static_cast<std::string>(object["extension"]),
              static_cast<unsigned int>(object["filter"]), static_cast<unsigned int>(object["wrapper"]),
              static_cast<unsigned int>(object["mipmap"])) {}
Texture::~Texture() {
    // When the texture is deleted, make sure it is also deleted from the openGL context
    glDeleteTextures(1, &textureID);
    #if DEBUG_OPENGL_OBJECTS 
        std::cout << "Texture " << textureID << " was deleted." << std::endl;
    #endif
}

bool Texture::operator==(const Texture& other) const {
    return textureFormat == other.textureFormat && file_name == other.file_name && extension == other.extension &&
           width == other.width && height == other.height && nChannels == other.nChannels && mipmap == other.mipmap &&
           filter == other.filter && wrapper == other.wrapper;
}
bool Texture::operator!=(const Texture& other) const { return !operator==(other); }

void Texture::bind() const {
    // Rather than setting a texture uniform, textures are fed to shaders by making them "active" with a numerical slot identifier
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(textureFormat, textureID);
}
inline void Texture::unbind() const { glBindTexture(textureFormat, 0); } 

void Texture::createMipmap(const unsigned int value) {
    // generates a mipmap for a loaded texture and saves the parameter that should be used for it
    glGenerateMipmap(textureFormat);
    mipmap = value;
}
void Texture::setFilter(const unsigned int value) {
    ///NOTE: Uses the same parameter for both the min and mag filter, but doesn't necessarily have to
    // sets the minimization filter according to the input value and the existing mipmaptype value
    filter = value;

    switch(mipmap) {
    case MIPMAP_LINEAR:
        switch(value) {
        case GL_LINEAR:
            setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            break;
        case GL_NEAREST:
            setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            break;
        }
        break;
    case MIPMAP_NEAREST:
        switch(value) {
            case GL_LINEAR:
            setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            break;
        case GL_NEAREST:
            setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            break;
        }
        break;
    default:
        setParameter(GL_TEXTURE_MIN_FILTER, value);
        break;
    }
    // sets the magnification filter according to the input parameter
    setParameter(GL_TEXTURE_MAG_FILTER, value);
}
void Texture::setWrapper(const unsigned int value) {
    ///NOTE: Sets the S (horizontal) and T (vertical) wrappers to be the same, but they don't have to be
    // sets both wrapper directions with the input parameter
    wrapper = value;

    setParameter(GL_TEXTURE_WRAP_S, value);
    setParameter(GL_TEXTURE_WRAP_T, value);
    if (textureFormat == TEXTURE_CUBE) setParameter(GL_TEXTURE_WRAP_R, value);
}

Serializer Texture::getJSON() const {
    Serializer object;
    object["texture_format"] = textureFormat;
    object["file_name"] = file_name;
    object["extension"] = extension;
    object["filter"] = filter;
    object["wrapper"] = wrapper;
    object["mipmap"] = mipmap;

    return object;
}

void Texture::print() {
    std::cout << "Address = " << this << std::endl;
    std::cout << "ID = " << textureID << std::endl;
    std::cout << "Format: ";
    switch(textureFormat) {
    case GL_TEXTURE_2D : { std::cout << "2D"; } break;
    case GL_TEXTURE_2D_MULTISAMPLE : { std::cout << "2D Multisample"; } break;
    case GL_TEXTURE_CUBE_MAP : { std::cout << "Cube Map"; } break;
    default: { std::cout << "Unrecognized type"; } break;
    }
    std::cout << std::endl;
    std::cout << "Size: (" << width << " x " << height << ")" << std::endl;
    std::cout << "# of channels: " << nChannels << std::endl;
    std::cout << "Slot: " << slot << std::endl;
    std::cout << "Mipmapping: ";
    switch(mipmap) {
    case 0:
        std::cout << "None";
        break;
    case MIPMAP_LINEAR:
        std::cout << "Linear";
        break;
    case MIPMAP_NEAREST:
        std::cout << "Nearest";
        break;
    default:
        std::cout << "Unrecognized type" << std::endl;
        break;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}


TextureGroup::TextureGroup(Serializer& object) {
    nextSlot = object["first_slot"];
    for (int i = 0; i < object["textures"].size(); i++)
        addTexture(std::make_shared<Texture>(static_cast<Serializer&>(object["textures"][i])));
}

bool TextureGroup::operator==(TextureGroup& other) {
    if (textures.size() != other.textures.size()) return false;
    for (int t = 0; t < textures.size(); t++) if (*textures[t] != *(other.textures[t])) return false;
    return true;
}
bool TextureGroup::operator==(Texture& other) { return textures.size() == 1 && *textures[0] == other; }

Serializer TextureGroup::getJSON() {
    Serializer object;
    object["first_slot"] = getSlot(0);
    for (int i = 0; i < textures.size(); i++) object["textures"][i] = std::move(getTexture(i)->getJSON());

    return object;
}