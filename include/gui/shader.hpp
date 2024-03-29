#ifndef SHADER_HPP
#define SHADER_HPP

#include <iostream>
#include <memory>
#include <string>

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

#include "../io/file_io.hpp"
#include "../io/parser.hpp"
#include "../io/serializer.hpp"

#include "elements.hpp"
#include "light.hpp"
#include "material.hpp"

/* TODO - specific uniforms are needed by specific snippets of glsl files. Can create uniform objects (strings and values) that
 * need to be set or there is an error.
 */


/* Shaders are scripts that run on the GPU kernal. They are meant to be highly parallelized and simple. The shader class makes handling
 * shaders easy by generating shader script based on a set of shader parameters and handling all interaction with shaders in the OpenGL
 * context.
 * 
 * Shaders are constructed by specifying the values of these parameters. Enumerated lists of possible values can be found in
 * elements.hpp. The values of parameters can also be read from a format object that was created by the "getJSON()." The constructor
 * will also create a shader program object in the OpenGL context which will only die once the shader is destroyed.
 * 
 * Shader parameters uniquely define the shader object. If two shaders have the same parameters, they will have the same shader script 
 * because the parsing function is deterministic. As such, before running the rather expensive parsing method, it may be beneficial to
 * compare shader parameters across multiple shaders to make sure there are no duplicates using the "==" operator.
 * 
 * If no duplicate shaders are found, one can proceed to call the "load()" method, which will retrieve or assemble the shader script
 * for all sub-shader programs (e.g., vertex and fragment shaders), then compile and link them together and bind them to the OpenGL
 * context.
 * 
 * For each shader, load() will first attempt to find shaders that already exist that match the shader parameters in 
 * /res/shaders/saves/. Saved shaders have file names that are also completely determined by their shader parameters. Loading from these
 * files is faster than creating the shader scripts from scratch. If this fails, then load() will call the appropriate genShader()
 * method which will piece together snippets of code from /res/shaders/components/. Once the shader source code is obtained, then
 * load() will attempt to compile the code and link the various shaders together.
 * 
 * Once the shader program is successfully compiled and bound to the openGL context, the shader object can also be used to interact
 * with the shader program in the OpenGL context. The "use()" method binds the shader program in the OpenGL context so it can be used
 * for rendering. The "setUniform()" method is used to update shader uniform variable values.
 */
class Shader {
public:
    // Shader can be constructed by specifying what it does. The follow inputs correspond to specific rendering modes.
    Shader(const unsigned int RENDERING_STYLE, const unsigned int OUTPUT_BUFFER,
           const unsigned int MATERIAL_STYLE, const unsigned int LIGHTING_STYLE, const unsigned int SHADOW_STYLE,
           const unsigned int TEXTURE_STYLE, const unsigned int POSTPROCESSING);
    // Shader can also recieve a format object that converts from the json file format to an object.
    Shader(Serializer& object);
    // The load function completes the construction of the shader after it has been determined that there are no duplicates already.
    void load();
    // Need to make sure the shader is removed from the OpenGL context when it is destroyed
    ~Shader();

    // Shaders should not be copied as they have a one-to-one correspondence with shaders in the OpenGL context
    Shader(const Shader&) = delete;
    void operator=(const Shader&) = delete;

    // It is pointless to create multiple shaders that have the same parameters, so we can check two shaders for this kind of equality
    bool operator==(const Shader& compare);

    // Bind the program in the OpenGL context (required before setting any uniforms or rendering)
    void use() const { glUseProgram(programID); }

    // Set uniforms in the OpenGL context. The uniforms are determined based on the program parameters.
    void setUniform(const std::string &name, const bool value) const 
        { glUniform1i(glGetUniformLocation(programID, name.c_str()), (int) value); }
    void setUniform(const std::string &name, const int value) const 
        { glUniform1i(glGetUniformLocation(programID, name.c_str()), value); };
    void setUniform(const std::string &name, const float value) const 
        { glUniform1f(glGetUniformLocation(programID, name.c_str()), value); };
    void setUniform(const std::string &name, const float v1, const float v2, const float v3) const 
        { glUniform3f(glGetUniformLocation(programID, name.c_str()), v1, v2, v3); }
    void setUniform(const std::string &name, const float v1, const float v2, const float v3, const float v4) const 
        { glUniform4f(glGetUniformLocation(programID, name.c_str()), v1, v2, v3, v4); }
    void setUniform(const std::string &name, const glm::vec3 value) const 
        { setUniform(name, value.x, value.y, value.z); }
    void setUniform(const std::string &name, const glm::vec4 value) const 
        { setUniform(name, value.x, value.y, value.z, value.w); }
    void setUniform(const std::string &name, const glm::mat3 mat) const 
        { glUniformMatrix3fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat)); }
    void setUniform(const std::string &name, const glm::mat4 mat) const 
        { glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat)); }
    // structs have fixed names within the shader, so a name is not required
    void setUniform(const std::shared_ptr<Material> material) const;
    void setUniform(const unsigned int index, const Light& light, const glm::mat4 view) const 
        { setUniform(LIGHT_NAME + "[" + std::to_string(index) + "]", light, view); }
    void setUniform(const std::string &name, const Light& light, const glm::mat4 view) const;

    // Return shader parameters
    const unsigned int getRenderingStyle() const { return rendering_style; }
    const unsigned int getOutputBuffer() const { return output_buffer; }
    const unsigned int getMaterialStyle() const { return material_style; }
    const unsigned int getLightingStyle() const { return lighting_style; }
    const unsigned int getShadowStyle() const { return shadow_style; }
    const unsigned int getTextureStyle() const { return texture_style; }
    const unsigned int getPostprocessing() const { return postprocessing; }

    // Encode a format object with data that can be used to recreate the shader. The format object can save object data in a json file.
    Serializer getJSON();

    // Print information about shader parameters
    void print() const;
private:
    // the OpenGL context returns an unsigned int which can be used to reference the program in the context
    unsigned int programID;

    /* Shader parameters are used to generate shader programs that can be run in the OpenGL context. See elements.hpp for explanations
     * of individual parameter settings.
     */
    // Rendering Style: The rendering style tells the shader whether it is rendering in 2D, basic 3D, or 3D with lighting
    unsigned int rendering_style = -1,
    // Output Buffer: The output buffer tells the shader what kind of data of its output buffer will store: color, depth, or stencil
                 output_buffer = -1;
    // Material Style: The material style tells the shader how to light objects' surfaces: disabled, basic, D map, DS map, or DSE map
    unsigned int material_style = M_DISABLED, 
    // Lighting Style: The lighting style tells the shader what kinds of lights are present in the scene: any combo of point lights, 
    //                 directional lights, and spot lights.
                 lighting_style = L_DISABLED,
    // Shadow Style: The shadow style tells the shader how/whether to render shadows: disabled, shadow mapping
                 shadow_style = S_DISABLED, 
    // Texture Style: The texture style tells the shader how/whether to render textures: disabled, 2D, 3D cube map
                 texture_style = T_DISABLED,
    // Postprocessing: The postprocessing parameter tells the shader to a number of different possible postprocessing effects 
                 postprocessing = P_DISABLED;

    // complete vertex and fragment shader code generated by stitching together code snippets from ./res/shaders/components
    std::string v_source, f_source;

    // creates a shader program in the OpenGL context by linking vertex and fragment shaders
    bool createProgram(const unsigned int& vertexShader, const unsigned int& fragmentShader);
    // a function that compiles glsl code and creates a shader object in the OpenGl context, giving us an int to reference it
    const unsigned int compileShader(const std::string& source, const unsigned int type);
    const unsigned int compileShader(const std::string& source, const unsigned int type, const std::string fileName);
    // a function that checks if there were linking errors when linking the shaders into a single program
    bool checkLinkingErrors() const;
    bool checkLinkingErrors(std::string vFilePath, std::string fFilePath) const;
    // a function that checks if there were compilation errors in compiling shader code
    bool checkCompilationErrors(const unsigned int shader) const;
    bool checkCompilationErrors(const unsigned int shader, std::string filePath) const;

    // a function that loads shader source code from a glsl file
    std::string loadFile(std::string fileName, std::string path);
    // This function will generate a unique vertex shader file name based on the shader parameters defined by the constructor.
    std::string genVertexShaderName();
    // This function will generate a unique fragment shader file name based on the shader parameters defined by the constructor.
    std::string genFragmentShaderName();

    // This function will generate vertex shader source code based on the shader parameters defined by the constructor. It will 
    // compose this code by taking snippets from the component files located at ./res/shaders/components.
    // Certain layout variables can be defined during runtime.
    std::string& generateVertexShader();
    // This function will generate fragment shader source code based on the shader parameters defined by the constructor. It will 
    // compose this code by taking snippets from the component files located at ./res/shaders/components.
    std::string& generateFragmentShader();
    // This function will generate a GLSL version declaration to be used in shader source code based on parameters set in elements.cpp.
    std::string getVersion();
    // This function copies keyed sections from code component files and appends them to the input string.
    void addComponent(std::string& components, const std::string fileName, const std::string key, const unsigned int shaderType);
    // This function orders the list of input components then replaces placeholder values with actual code. The result is (almost) 
    // source code.
    void assembleSource(std::string& source, const std::string components);
    // A number of placeholder terms will exist in section code that is not compilable. This function will find placeholders then
    // replace them with the code snippets that actually belongs there.
    void fillPlaceholders(std::string& section, char flag);
    // A number of placeholder terms will exist in section code that is not compilable. This function will find placeholders then
    // replace them with the code snippets that actually belongs there.
    void fillPlaceholders(std::string& section, std::string flag);

    // this function prints source code in a user friendly way (for debugging)
    void printSource(std::string source);
};

#endif