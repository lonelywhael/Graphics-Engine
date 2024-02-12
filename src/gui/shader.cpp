#include "gui/shader.hpp"

// these paths are the locations of the shader save files and shader component files
const char SHADER_SAVE_PATH[] = "../res/shaders/saves/", SHADER_COMPONENTS_PATH[] = "../res/shaders/components/";
// these are keys that are used to parse component files so that specific code snippets can be found
// structural keys are used for structural elements of the shader while all others are based on parameter specifications
std::string GENERAL_KEY = "@@GENERAL",
            STRUCTURAL_KEYS[] = { "@GLOBAL\n", "@STRUCTS\n", "@IN\n", "@OUT\n", "@UNIFORMS\n", "@FUNCTIONS\n", "@MAIN\n" },
            RENDERING_KEYS[] = { "@@BASIC_2D", "@@BASIC_3D", "@@LIGHTING_3D", "@@SKYBOX"},
            OUTPUT_KEYS[] = { "@@COLOR_BUFFER", "@@DEPTH_BUFFER", "@@STENCIL_BUFFER"},
            LIGHTING_KEYS[] = { "", "@@DIR", "@@POINT", "@@SPOT", "@@DIR_POINT", "@@DIR_SPOT", "@@POINT_SPOT", "@@ALL_ENABLED" },
            SHADOW_KEYS[] = { "@@DISABLED", "@@SHADOW_MAPPING" },
            MATERIAL_KEYS[] = { "", "@@BASIC", "@@D_MAP", "@@DS_MAP", "@@DSE_MAP" },
            TEXTURE_KEYS[] = { "@@DISABLED", "@@BASIC_2D", "@@CUBE" },
            POSTPROCESSING_KEYS[] = { "@@DISABLED", "@@BLUR", "@@DEPTH_MAP", "@@LINEARIZED_DEPTH_MAP", "@@SHADOW_MAP"};
// these are the names of shader component files (each corresponds to a shader parameter)
std::string RENDERING_FILE = "rendering.glsl", OUTPUT_FILE = "output.glsl", LIGHTING_FILE = "lighting.glsl", 
            MATERIAL_FILE = "material.glsl", SHADOW_FILE = "shadow.glsl", TEXTURE_FILE = "texture.glsl", 
            POSTPROCESSING_FILE = "postprocessing.glsl";

// this constructor takes shader parameters as inputs
Shader::Shader(const unsigned int RENDERING_STYLE, const unsigned int OUTPUT_BUFFER,
               const unsigned int MATERIAL_STYLE, const unsigned int LIGHTING_STYLE, const unsigned int SHADOW_STYLE,
               const unsigned int TEXTURE_STYLE, const unsigned int POSTPROCESSING)
        : rendering_style(RENDERING_STYLE), output_buffer(OUTPUT_BUFFER), material_style(MATERIAL_STYLE), lighting_style(LIGHTING_STYLE),
          shadow_style(SHADOW_STYLE), texture_style(TEXTURE_STYLE), postprocessing(POSTPROCESSING) {
    
    // create shader obeject in the openGL context and retrieve integer id with which to reference it
    programID = glCreateProgram();
    #if DEBUG_OPENGL_OBJECTS 
        std::cout << "Shader " << programID << " was created." << std::endl;
    #endif
}
Shader::Shader(Serializer& object) {
    // read shader parameters from the format object (which allows for communication with json files)
    rendering_style = object["rendering_style"];
    output_buffer = object["output_buffer"];
    material_style = object["material_style"];
    lighting_style = object["lighting_style"];
    shadow_style = object["shadow_style"];
    texture_style = object["texture_style"];
    postprocessing = object["postprocessing"]; 

    // create shader obeject in the openGL context and retrieve integer id with which to reference it
    programID = glCreateProgram();
    #if DEBUG_OPENGL_OBJECTS 
        std::cout << "Shader " << programID << " was created." << std::endl;
    #endif
}
void Shader::load() {
    // generate unique shader file names based on the shader parameters
    std::string v_shader_name = genVertexShaderName(), f_shader_name = genFragmentShaderName();
    // if there are already shader files that have this name in the save directory, load those files, otherwise create new shaders
    v_source = (f_exists(SHADER_SAVE_PATH + v_shader_name)) ? loadFile(v_shader_name, SHADER_SAVE_PATH) : generateVertexShader();
    f_source = (f_exists(SHADER_SAVE_PATH + f_shader_name)) ? loadFile(f_shader_name, SHADER_SAVE_PATH) : generateFragmentShader();

    // compile the shaders and then link them together
    unsigned int vertexShader = compileShader(v_source, GL_VERTEX_SHADER), fragmentShader = compileShader(f_source, GL_FRAGMENT_SHADER);
    createProgram(vertexShader, fragmentShader); 
}
Shader::~Shader() {
    // When shader object is deleted, also make sure openGL context program object is deleted. 
    glDeleteProgram(programID);
    #if DEBUG_OPENGL_OBJECTS 
        std::cout << "Shader " << programID << " was deleted." << std::endl;
    #endif
}


bool Shader::operator==(const Shader& compare) {
    // Shaders are unique based on their parameter settings. Check for equality between those.
    return rendering_style == compare.rendering_style && output_buffer == compare.output_buffer &&
           material_style == compare.material_style && lighting_style == compare.lighting_style &&
           shadow_style == compare.shadow_style && texture_style == compare.texture_style && postprocessing == compare.postprocessing;
}


bool Shader::createProgram(const unsigned int& vertexShader, const unsigned int& fragmentShader) {
    // Checks if shaders were successfully linked
    bool success = true;
    // link individual shaders into a single program
    glAttachShader(programID, vertexShader);    //attach vertex shader to program
    glAttachShader(programID, fragmentShader);  //attach fragment shader to program
    glLinkProgram(programID);                   //link program
    // if there are errors, print for debugging
    if (!checkLinkingErrors()) {
        glDeleteProgram(programID);
        success = false;
        printSource(v_source);
        printSource(f_source);
    }
    // remove compiled shaders from the OpenGL objects, we only need the linked binary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return success;
}
const unsigned int Shader::compileShader(const std::string& source, const unsigned int type) { return compileShader(source, type, ""); }
const unsigned int Shader::compileShader(const std::string& source, const unsigned int type, const std::string fileName) {
    // create a shader object in the openGL context and compile the shader, then return the shader id
    const unsigned int shaderID = glCreateShader(type);
    const char* const _source = source.c_str();
    glShaderSource(shaderID, 1, &_source, NULL);
    glCompileShader(shaderID);
    // check for compilation errors, but don't stop the program. This will cause the error to cascade to the linking step.
    if (!checkCompilationErrors(shaderID, fileName)) {
        print();
        printSource(source);
    }

    return shaderID;
}
bool Shader::checkLinkingErrors() const { return checkLinkingErrors("(not recorded)", "(not recorded)"); }
bool Shader::checkLinkingErrors(const std::string vFilePath, const std::string fFilePath) const {
    // OpenGL keeps track of linking errors and they can be printed.
    int success;
    char infoLog[512];
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(programID, 512, NULL, infoLog);
        std::cout << "ERROR::PROGRAM::LINKER_FAILED:\n" << 
                     "Vertex Shader: " << vFilePath << "\n" <<
                     "Fragment Shader: " << fFilePath << "\n" << 
                     infoLog << std::endl;
    }
    return success;
}
bool Shader::checkCompilationErrors(const unsigned int shader) const { return checkCompilationErrors(shader, "(path not recorded)"); }
bool Shader::checkCompilationErrors(const unsigned int shader, const std::string filePath) const {
    // OpenGL keeps track of compilation errors as well.
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED: " << filePath << "\n" << infoLog << std::endl;
    }
    return success;
}

void Shader::setUniform(std::shared_ptr<Material> material) const {
    // Read from a material struct and copy data to the identical struct in the shader program. Uniforms are dependent on material type.
    switch(material->type) {
    case M_BASIC: {
        setUniform(MAT_NAME[M_BASIC] + ".ambient", material->basicMat.ambient);
        setUniform(MAT_NAME[M_BASIC] + ".diffuse", material->basicMat.diffuse);
        setUniform(MAT_NAME[M_BASIC] + ".specular", material->basicMat.specular);
        setUniform(MAT_NAME[M_BASIC] + ".shininess", material->basicMat.shininess);
    } break;
    case M_D_MAP: {
        setUniform(MAT_NAME[M_D_MAP] + ".diffuse", material->dMap.diffuse);
        setUniform(MAT_NAME[M_D_MAP] + ".specular", material->dMap.specular);
        setUniform(MAT_NAME[M_D_MAP] + ".shininess", material->dMap.shininess);
    } break;
    case M_DS_MAP: {
        setUniform(MAT_NAME[M_DS_MAP] + ".diffuse", material->dsMap.diffuse);
        setUniform(MAT_NAME[M_DS_MAP] + ".specular", material->dsMap.specular);
        setUniform(MAT_NAME[M_DS_MAP] + ".shininess", material->dsMap.shininess);
    } break;
    case M_DSE_MAP: {
        setUniform(MAT_NAME[M_DSE_MAP] + ".diffuse", material->dseMap.diffuse);
        setUniform(MAT_NAME[M_DSE_MAP] + ".specular", material->dseMap.specular);
        setUniform(MAT_NAME[M_DSE_MAP] + ".emission", material->dseMap.emission);
        setUniform(MAT_NAME[M_DSE_MAP] + ".shininess", material->dseMap.shininess);
    } break;
    }
}
void Shader::setUniform(const std::string &name, const Light& light, const glm::mat4 view) const {
    // Copy data from a light object to an equivalent light struct in the shader program.
    setUniform(name + ".type", light.type);
    // need to convert spatial vectors to view space
    setUniform(name + ".pos", glm::vec3(view * glm::vec4(light.pos, 1.0f))); 
    setUniform(name + ".dir", glm::mat3(view) * light.dir);
    setUniform(name + ".ambient", light.ambient);
    setUniform(name + ".diffuse", light.diffuse);
    setUniform(name + ".specular", light.specular);
    setUniform(name + ".constant", light.constant);
    setUniform(name + ".linear", light.linear);
    setUniform(name + ".quadratic", light.quadratic);
    setUniform(name + ".inner", light.inner);
    setUniform(name + ".outer", light.outer);
    setUniform(name + ".shadowMap", light.getShadowMapSlot());
}

// TODO fix naming convention for "getJSON" methods
Serializer Shader::getJSON() {
    // save shader parameters to a format object, which will convert them to a json or can be added to a scene object
    Serializer object;
    object["rendering_style"] = rendering_style;
    object["output_buffer"] = output_buffer;
    object["material_style"] = material_style;
    object["lighting_style"] = lighting_style;
    object["shadow_style"] = shadow_style;
    object["texture_style"] = texture_style;
    object["postprocessing"] = postprocessing;

    // TODO - offload to file saving method
    // create unique file names based on parameters
    std::string v_path = std::string(SHADER_SAVE_PATH) + genVertexShaderName(), 
                f_path = std::string(SHADER_SAVE_PATH) + genFragmentShaderName();

    // check if there are files that have those names already, if not make ones
    if (!f_exists(v_path)) f_writeText(v_path, v_source);
    if (!f_exists(f_path)) f_writeText(f_path, f_source);

    return object;
}
// TODO - this should be in io/file_io.hpp
std::string Shader::loadFile(std::string fileName, std::string path) {
    // add shader directory root to the fileName
    std::string filePath = path + fileName;

    // read text from the file at the file path to a c_string 
    std::string contents;
    f_readText(filePath, contents);

    return contents;
}
std::string Shader::genVertexShaderName() {
    // note: not all parameters are needed to fully specify a vertex shader

    // concatenate a string with various components that indicate what kind of vertex shader it is
    // "v" marks that this is a vertex shader
    std::string v_shader_name = "v";
    // rendering strategy determines whether other parameters will be used
    switch(rendering_style) {
    case R_BASIC_2D: { v_shader_name += "_2D"; } break;
    case R_BASIC_3D: { v_shader_name += "_3D"; } break;
    case R_LIGHTING_3D: { 
        v_shader_name += "_l3D";
        // if there is lighting, there might also be shadows (if nothing, then shadows are disabled)
        switch(shadow_style) {
        case S_SHADOW_MAPPING: { v_shader_name += "_ssmap"; } break;
        }
    } break;
    case R_SKYBOX: { v_shader_name += "_box"; } break;
    }

    // textures can appear in a number of rendering strategies (if nothing, then textures are disabled)
    switch(texture_style) {
    case T_BASIC_2D: { v_shader_name += "_t2D"; } break;
    case T_CUBE: { v_shader_name += "_tcube"; } break;
    }
    // make sure to place in vertex shader folder and add the correct file type
    return "v_shaders/" + v_shader_name + ".glsl";
}
std::string Shader::genFragmentShaderName() {
    // concatenate a string with various components that indicate what kind of fragment shader it is
    // "f" marks that this is a vertex shader
    std::string f_shader_name = "f";
    // rendering strategy determines whether other parameters will be used
    switch(rendering_style) {
    case R_BASIC_2D: { f_shader_name += "_2D"; } break;
    case R_BASIC_3D: { f_shader_name += "_3D"; } break;
    case R_LIGHTING_3D: { 
        // if there is lighting, we will also need a lighting style, material style, and shadow style
        f_shader_name += "_l3D"; 
        switch(lighting_style) {
        case L_DIR: { f_shader_name += "_ld"; } break;
        case L_POINT: { f_shader_name += "_lp"; } break;
        case L_SPOT: { f_shader_name += "_ls"; } break;
        case L_DIR_POINT: { f_shader_name += "_ldp"; } break;
        case L_DIR_SPOT: { f_shader_name += "_lds"; } break;
        case L_POINT_SPOT: { f_shader_name += "_lps"; } break;
        case L_ALL_ENABLED: { f_shader_name += "_ldps"; } break;
        }
        switch(material_style) {
        case M_BASIC: { f_shader_name += "_mbas"; } break;
        case M_D_MAP: { f_shader_name += "_mdmap"; } break;
        case M_DS_MAP: { f_shader_name += "_mdsmap"; } break;
        case M_DSE_MAP: { f_shader_name += "_mdsemap"; } break;
        }
        switch(shadow_style) {
        case S_SHADOW_MAPPING: { f_shader_name += "_ssmap"; } break;
        }
    } break;
    case R_SKYBOX: { f_shader_name += "_box"; } break;
    }
    // next, specify output buffer (if there is nothing, that means it is a color buffer)
    switch(output_buffer) {
    case B_DEPTH: { f_shader_name += "_dbuf"; } break;
    case B_STENCIL: { f_shader_name += "_sbuf"; } break;
    }
    // then texture styles (if nothing, then textures are disabled)
    switch(texture_style) {
    case T_BASIC_2D: { f_shader_name += "_t2D"; } break;
    case T_CUBE: { f_shader_name += "_tcube"; } break;
    }
    // then postprocessing strategy (if nothing, then postprocessing is disabled)
    switch(postprocessing) {
    case P_BLUR: { f_shader_name += "_pblur"; } break;
    case P_SHADOW_MAP: { f_shader_name += "_psmap"; } break;
    case P_DEPTH_MAP: { f_shader_name += "_pdmap"; } break;
    case P_LINEARIZED_DEPTH_MAP: { f_shader_name += "_pldmap"; } break;
    }
    return "f_shaders/" + f_shader_name + ".glsl";
}

std::string& Shader::generateVertexShader() {
    // Version declaration
    v_source = getVersion();

    // iterate through various component files, each of which corresponds to a shader parameter. Each element in the array corresponds 
    // a section, which is indicated by a structural key.
    std::string v_components = "";
    //std::vector<std::unique_ptr<std::string>> v_components;
    addComponent(v_components, RENDERING_FILE, RENDERING_KEYS[rendering_style], GL_VERTEX_SHADER);
    addComponent(v_components, TEXTURE_FILE, TEXTURE_KEYS[texture_style], GL_VERTEX_SHADER);
    if (rendering_style == R_LIGHTING_3D)
        addComponent(v_components, SHADOW_FILE, SHADOW_KEYS[shadow_style], GL_VERTEX_SHADER);

    // once all components are added, they will need to be arranged into the proper order and placeholder sections will need to be with
    // the correct code snippets.
    // the code with be rearranged by section such that each section has the appropriate code snippets from each component in the order
    // the components were added above.
    assembleSource(v_source, v_components);

    // some placeholders are indicated by a '$'. These are used to specify layout positions. Layout positions must occur in
    // in incrementing order, so we simply set them to an incrementing value for each one we find.
    size_t c = 0, pos = v_source.find('$');
    while(pos != std::string::npos) {
        v_source.replace(pos, 1, std::to_string(c++));
        pos = v_source.find('$');
    }

    return v_source;
}
std::string& Shader::generateFragmentShader() {
    // version declaration
    f_source = getVersion();

    std::string f_components = "";
    //std::vector<std::unique_ptr<std::string>> f_components;

    // similar to above, though there are more parameters that must be specified
    addComponent(f_components, RENDERING_FILE, RENDERING_KEYS[rendering_style], GL_FRAGMENT_SHADER);
    addComponent(f_components, OUTPUT_FILE, OUTPUT_KEYS[output_buffer], GL_FRAGMENT_SHADER);
    addComponent(f_components, TEXTURE_FILE, TEXTURE_KEYS[texture_style], GL_FRAGMENT_SHADER);
    if (rendering_style == R_LIGHTING_3D) {
        addComponent(f_components, LIGHTING_FILE, LIGHTING_KEYS[lighting_style], GL_FRAGMENT_SHADER);
        addComponent(f_components, SHADOW_FILE, SHADOW_KEYS[shadow_style], GL_FRAGMENT_SHADER);
        addComponent(f_components, MATERIAL_FILE, MATERIAL_KEYS[material_style], GL_FRAGMENT_SHADER );
    } else 
        addComponent(f_components, POSTPROCESSING_FILE, POSTPROCESSING_KEYS[postprocessing], GL_FRAGMENT_SHADER);

    // as above
    assembleSource(f_source, f_components);

    return f_source;
}
std::string Shader::getVersion() {
    std::string version;
    // GLSL version parameters are set at the top of shader code (e.g., "#version 330 core"). These values are saved in elements.cpp
    version = "#version " + std::to_string((int) (VERSION * 100)) + " ";
    switch(PROFILE) {
    case P_CORE: { version += "core"; } break;
    }
    version += "\n";

    return version;
}
void Shader::addComponent(std::string& components, const std::string fileName, const std::string key, const unsigned int shaderType) {
    // specify between vertex and fragment components
    std::string _fileName;
    switch(shaderType) {
    case GL_VERTEX_SHADER: { _fileName = "v_" + fileName; } break;
    case GL_FRAGMENT_SHADER: { _fileName = "f_" + fileName; } break;
    }
    // load file
    std::string source = loadFile(_fileName, SHADER_COMPONENTS_PATH);
    // some component files have snippets that are needed regardless of shader parameters, which are accessed with the general key
    if (p_hasKey(source, GENERAL_KEY)) components += p_getKeyedSubstr(source, "@@", GENERAL_KEY);
    // otherwise, only copy the code associated with the specified key
    components += p_getKeyedSubstr(source, "@@", key);
}
void Shader::assembleSource(std::string& source, const std::string components) {
    for (std::string key : STRUCTURAL_KEYS) {
        std::string section = "";
        int c = components.find(key);
        while(c != std::string::npos) {
            section += p_getKeyedSubstr(components.substr(c), '@', key);
            c = components.find(key, c + 1);
        }
        fillPlaceholders(section, "&&");
        fillPlaceholders(section, '&');
        if (section.length() > 0) source += '\n' + section;
    }
}
void Shader::fillPlaceholders(std::string& section, char flag) {
    // keeps running until there are no more of the specified flags
    while(p_hasKey(section, flag)) {
        // look for the first and second appearances of the flag. These are the opening and closing flags of the first placeholder.
        size_t var_start = section.find(flag), var_end = section.find(flag, var_start + 1), fill_start = 0, fill_end = 0;
        // the text contained within the flags is either the placeholder or the snippet that should replace the placeholder
        std::string var = section.substr(var_start, var_end - var_start), fill = "";
        // if var has a line break in it, it must be the snippet that replaces the placeholder
        if (p_hasKey(var, '\n')) {
            // the placeholder is the text that occurs before the line break plus a closing flag
            var = var.substr(0, var.find('\n')) + flag;
            // the replacement text is everything after the line break until the closing character
            fill = section.substr(var_start + var.length(), var_end - var_start - var.length());
            // erase the replacement text
            section.erase(var_start, var_end - var_start + 2);
            // search for the key and replace it with the replacement text
            fill_start = section.find(var);
            if (fill_start != std::string::npos) {
                fill_end = section.find(flag, fill_start + 1);
                section.replace(fill_start, fill_end - fill_start + 1, fill);
            }
            #if DEBUG_SHADER_BUILDER_SHOW_UNUSED_VARS
                // if could not find the key, then there was a designated replacement text but it was unused
                // this is not necessarily an error (because the replacement text still gets deleted), but may not be intended
                else std::cout << "ERROR::SHADER::UNUSED_VARIABLE: Variable \"" << var << "\" defined but not used." << std::endl;
            #endif
        // otherwise, var is already a placeholder
        } else {
            // add a line break and look for the replacement text
            var += '\n';
            if (!p_hasKey(section, var)) {
                // if the flag-key-line break combo doesn't exist, then there is no replacement section
                // this is critical because the shader code is unlikely to compile if placeholders are not replaced
                std::cout << "ERROR::SHADER::MISSING_DEFINITION: Variable \"" << var.substr(0, var.length() - 1) 
                    << flag << "\" undefined." << std::endl;
                break;
            }
            // locating replacement text so that it can be erased
            fill_start = section.find(var), fill_end = section.find(flag, fill_start + 1);
            // replacement code starts after the line break
            fill = section.substr(fill_start + var.length(), fill_end - fill_start - var.length());
            // replace the original placeholder with replacement code
            section.replace(var_start, var_end - var_start + 1, fill);
            // account for the change in text positions given the above replacement, then erase the replacement code
            size_t l = fill_end - fill_start + 2;
            fill_start += (var_start < fill_start) ? fill.length() - var_end + var_start - 1 : 0;
            section.erase(fill_start, l);
        }
    }
}
void Shader::fillPlaceholders(std::string& section, std::string flag) {
    while(p_hasKey(section, flag)) {
        size_t var_start = section.find(flag), var_end = section.find(flag, var_start + 1), fill_start = 0, fill_end = 0;
        std::string var = section.substr(var_start, var_end - var_start), fill = "";
        if (p_hasKey(var, '\n')) {
            var = var.substr(0, var.find('\n')) + flag;
            fill = section.substr(var_start + var.length() - flag.length() + 1, 
                                  var_end - var_start - var.length() + flag.length() - 1);
            section.erase(var_start, var_end - var_start + flag.length() + 1);
            fill_start = section.find(var);
            if (fill_start != std::string::npos) {
                fill_end = section.find(flag, fill_start + 1);
                section.replace(fill_start, fill_end - fill_start + flag.length(), fill);
            } 
            #if DEBUG_SHADER_BUILDER_SHOW_UNUSED_VARS
                else std::cout << "ERROR::SHADER::UNUSED_VARIABLE: Variable \"" << var << "\" defined but not used." << std::endl;
            #endif
        } else {
            var += '\n';
            if (!p_hasKey(section, var)) {
                std::cout << "ERROR::SHADER::MISSING_DEFINITION: Variable \"" << var.substr(0, var.length() - flag.length()) 
                    << flag << "\" undefined." << std::endl;
                break;
            }
            fill_start = section.find(var), fill_end = section.find(flag, fill_start + 1);
            fill = section.substr(fill_start + var.length(), fill_end - fill_start - var.length());
            section.replace(var_start, var_end - var_start + flag.length(), fill);
            size_t l = fill_end - fill_start + flag.length() + 1;
            fill_start += (var_start < fill_start) ? fill.length() - var_end + var_start - flag.length() : 0;
            section.erase(fill_start, l);
        }
    }
}


void Shader::print() const {
    std::cout << "Render Style (" << rendering_style << "):\t";
    switch(rendering_style) {
    case R_BASIC_2D: { std::cout << "Basic 2D"; } break;
    case R_BASIC_3D: { std::cout << "Basic 3D"; } break;
    case R_LIGHTING_3D: { std::cout << "Lighting 3D"; } break;
    case R_SKYBOX: { std::cout << "Skybox"; } break;
    }
    std::cout << std::endl;
    std::cout << "Texture Style (" << texture_style << "):\t";
    switch(texture_style) {
    case T_DISABLED: { std::cout << "Texture Disabled"; } break;
    case T_BASIC_2D: { std::cout << "Basic 2D"; } break;
    case T_CUBE: { std::cout << "Cube"; } break;
    }
    std::cout << std::endl;
    std::cout << "Material Style (" << material_style << "):\t";
    switch(material_style) {
    case M_DISABLED: { std::cout << "Material Disabled"; } break;
    case M_BASIC: { std::cout << "Basic Material"; } break;
    case M_D_MAP: { std::cout << "D Map"; } break;
    case M_DS_MAP: { std::cout << "DS Map"; } break;
    case M_DSE_MAP: { std::cout << "DSE Map"; } break;
    }
    std::cout << std::endl;
    std::cout << "Lighting Style (" << lighting_style << "):\t";
    switch(lighting_style) {
    case L_DISABLED: { std::cout << "Lighting Disabled"; } break;
    case L_DIR: { std::cout << "Directional Lighting"; } break;
    case L_POINT: { std::cout << "Point Lighting"; } break;
    case L_SPOT: { std::cout << "Spot Lighting"; } break;
    case L_DIR_POINT: { std::cout << "Directional and Point Lighting"; } break;
    case L_DIR_SPOT: { std::cout << "Directional and Spot Lighting"; } break;
    case L_POINT_SPOT: { std::cout << "Point and Spot Lighting"; } break;
    case L_ALL_ENABLED: { std::cout << "All Lighting Enabled"; } break;
    }
    std::cout << std::endl;
    std::cout << "Shadow Style (" << shadow_style << "):\t";
    switch(shadow_style) {
    case S_DISABLED: { std::cout << "Shadows Disabled"; } break;
    case S_SHADOW_MAPPING: { std::cout << "Shadows Enabled"; } break;
    }
    std::cout << std::endl;
    std::cout << "Output Buffer (" << output_buffer << "):\t";
    switch(output_buffer) {
    case B_COLOR: { std::cout << "Color Buffer"; } break;
    case B_DEPTH: { std::cout << "Depth Buffer"; } break;
    case B_STENCIL: { std::cout << "Stencil Buffer"; } break;
    }
    std::cout << std::endl;
    std::cout << "Postprocessing (" << postprocessing << "):\t";
    switch(postprocessing) {
    case P_DISABLED: { std::cout << "Postprocessing Disabled"; } break;
    case P_BLUR: { std::cout << "Blur"; } break;
    case P_DEPTH_MAP: { std::cout << "Depth Map"; } break;
    case P_LINEARIZED_DEPTH_MAP: { std::cout << "Linearized Depth Map"; } break;
    case P_SHADOW_MAP: { std::cout << "Shadow Map"; } break;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}
void Shader::printSource(std::string source) {
    int s = 0, e = 0, i = 1;
    while (e != std::string::npos) {
        e = source.find('\n', s);
        std::cout << i << ":\t" << source.substr(s, e - s) << std::endl;
        s = e + 1;
        i++;
    }
}