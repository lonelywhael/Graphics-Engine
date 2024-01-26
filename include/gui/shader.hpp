#ifndef SHADER_HPP
#define SHADER_HPP

#include <iostream>
#include <memory>
#include <string>

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

#include "../io/file_io.hpp"
#include "../io/format.hpp"

#include "elements.hpp"
#include "light.hpp"
#include "material.hpp"

/* SHADER CLASS
 * 
 * The shader class is used for loading shader programs onto the openGL context and facilitating interaction between the shader program
 * and the rest of the program.
 * 
 * When a shader is created, the constructor reads from shader files and compiles them into a program. Once created, the class can be used
 * to set uniforms on the program.
 * 
 * Like other classes that interface with the OpenGL context, shader should be held in a strict 1 to 1 correspondence with OpenGL program
 * objects. Shaders should not be copied, but instead passed only by reference or pointer.
 */
class Shader {
public:
    // Shader requires a path to both the vertex shader and the fragment shader
    Shader(const unsigned int RENDERING_STYLE, const unsigned int OUTPUT_BUFFER,
           const unsigned int MATERIAL_STYLE, const unsigned int LIGHTING_STYLE, const unsigned int SHADOW_STYLE,
           const unsigned int TEXTURE_STYLE, const unsigned int POSTPROCESSING);
    Shader(Format& object);
    void load();
    // Destructor is required because the program needs to remove itself from the OpenGL context
    ~Shader();

    Shader(const Shader&) = delete;
    void operator=(const Shader&) = delete;

    bool operator==(const Shader& compare);

    // bind the program in the OpenGL context (required before setting any uniforms or rendering)
    void use() const { glUseProgram(programID); }

    // Given the name as a string of a uniform and a value of some type, the set uniform command sends that information to the program
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

    const unsigned int getRenderingStyle() const { return rendering_style; }
    const unsigned int getOutputBuffer() const { return output_buffer; }
    const unsigned int getMaterialStyle() const { return material_style; }
    const unsigned int getLightingStyle() const { return lighting_style; }
    const unsigned int getShadowStyle() const { return shadow_style; }
    const unsigned int getTextureStyle() const { return texture_style; }
    const unsigned int getPostprocessing() const { return postprocessing; }

    Format getJSON();

    void print() const;
private:
    // the OpenGL context returns an unsigned int which can be used to reference the program in the context
    unsigned int programID;

    unsigned int rendering_style = -1, output_buffer = -1;
    unsigned int material_style = M_DISABLED, lighting_style = L_DISABLED, shadow_style = S_DISABLED, 
                 texture_style = T_DISABLED, postprocessing = P_DISABLED;

    std::string v_source, f_source;

    bool createProgram(const unsigned int& vertexShader, const unsigned int& fragmentShader);
    // a function that reads code from a shader file and creates a shader object in the OpenGl context, giving us an int to reference it
    const unsigned int compileShader(const std::string& source, const unsigned int type);
    const unsigned int compileShader(const std::string& source, const unsigned int type, const std::string fileName);
    // a function that checks if there were linking errors when linking the shaders into a single program
    bool checkLinkingErrors() const;
    bool checkLinkingErrors(std::string vFilePath, std::string fFilePath) const;
    // a function that checks if there were compilation errors in compiling shader code
    bool checkCompilationErrors(const unsigned int shader) const;
    bool checkCompilationErrors(const unsigned int shader, std::string filePath) const;

    std::string loadFile(std::string fileName, std::string path);
    std::string genVertexShaderName();
    std::string genFragmentShaderName();

    std::string generateVertexShader();
    std::string generateFragmentShader();
    void addComponent(std::vector<std::unique_ptr<std::string>>& components, 
                      const std::string fileName, const std::string key, const unsigned int shaderType);
    std::string getKeyedSection(const std::string string, const char flag, const std::string key) 
        { return (contains(string, key) ? substr(string, flag, key) : ""); }
    std::string getKeyedSection(std::string string, std::string flag, std::string key) 
        { return (contains(string, key) ? substr(string, flag, key) : ""); }
    void defineVars(std::string& section, char flag);
    void defineVars(std::string& section, std::string flag);
    bool contains(std::string string, char key) { return (string.find(key) != std::string::npos); }
    bool contains(std::string string, std::string key) { return (string.find(key) != std::string::npos); }
    std::string substr(std::string string, char flag, char key) {
        size_t start = string.find(key) + 1, end = string.find(flag, start);
        return string.substr(start, end - start);
    }
    std::string substr(std::string string, char flag, std::string key) {
        size_t start = string.find(key) + key.length(), end = string.find(flag, start);
        return string.substr(start, end - start);
    }
    std::string substr(std::string string, std::string flag, std::string key) {
        size_t start = string.find(key) + key.length(), end = string.find(flag, start);
        return string.substr(start, end - start);
    }


    void printSource(std::string source);
};

#endif