#include "gui/shader.hpp"

const char SHADER_SAVE_PATH[] = "../res/shaders/saves/", SHADER_COMPONENTS_PATH[] = "../res/shaders/components/";
std::string GENERAL_KEY = "@@GENERAL",
            STRUCTURAL_KEYS[] = { "@GLOBAL\n", "@STRUCTS\n", "@IN\n", "@OUT\n", "@UNIFORMS\n", "@FUNCTIONS\n", "@MAIN\n" },
            RENDERING_KEYS[] = { "@@BASIC_2D", "@@BASIC_3D", "@@LIGHTING_3D", "@@SKYBOX"},
            OUTPUT_KEYS[] = { "@@COLOR_BUFFER", "@@DEPTH_BUFFER", "@@STENCIL_BUFFER"},
            LIGHTING_KEYS[] = { "", "@@DIR", "@@POINT", "@@SPOT", "@@DIR_POINT", "@@DIR_SPOT", "@@POINT_SPOT", "@@ALL_ENABLED" },
            SHADOW_KEYS[] = { "@@DISABLED", "@@SHADOW_MAPPING" },
            MATERIAL_KEYS[] = { "", "@@BASIC", "@@D_MAP", "@@DS_MAP", "@@DSE_MAP" },
            TEXTURE_KEYS[] = { "@@DISABLED", "@@BASIC_2D", "@@CUBE" },
            POSTPROCESSING_KEYS[] = { "@@DISABLED", "@@BLUR", "@@DEPTH_MAP", "@@LINEARIZED_DEPTH_MAP", "@@SHADOW_MAP"};
std::string RENDERING_FILE = "rendering.glsl", OUTPUT_FILE = "output.glsl", LIGHTING_FILE = "lighting.glsl", 
            MATERIAL_FILE = "material.glsl", SHADOW_FILE = "shadow.glsl", TEXTURE_FILE = "texture.glsl", 
            POSTPROCESSING_FILE = "postprocessing.glsl";

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
Shader::Shader(Format& object) {
    rendering_style = object["rendering_style"];
    output_buffer = object["output_buffer"];
    material_style = object["material_style"];
    lighting_style = object["lighting_style"];
    shadow_style = object["shadow_style"];
    texture_style = object["texture_style"];
    postprocessing = object["postprocessing"];

    programID = glCreateProgram();
    #if DEBUG_OPENGL_OBJECTS 
        std::cout << "Shader " << programID << " was created." << std::endl;
    #endif
}
void Shader::load() {
    std::string v_shader_name = genVertexShaderName(), f_shader_name = genFragmentShaderName();
    v_source = (f_exists(SHADER_SAVE_PATH + v_shader_name)) ? loadFile(v_shader_name, SHADER_SAVE_PATH) : generateVertexShader();
    f_source = (f_exists(SHADER_SAVE_PATH + f_shader_name)) ? loadFile(f_shader_name, SHADER_SAVE_PATH) : generateFragmentShader();

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
    return rendering_style == compare.rendering_style && output_buffer == compare.output_buffer &&
           material_style == compare.material_style && lighting_style == compare.lighting_style &&
           shadow_style == compare.shadow_style && texture_style == compare.texture_style && postprocessing == compare.postprocessing;
}


bool Shader::createProgram(const unsigned int& vertexShader, const unsigned int& fragmentShader) {
    bool success = true;
    // link individual shaders into a single program
    glAttachShader(programID, vertexShader);    //attach vertex shader to program
    glAttachShader(programID, fragmentShader);  //attach fragment shader to program
    glLinkProgram(programID);                   //link program
    if (!checkLinkingErrors()) {
        glDeleteProgram(programID);
        success = false;
        printSource(v_source);
        printSource(f_source);
    }
    // free memory allocated to the individual shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return success;
}
const unsigned int Shader::compileShader(const std::string& source, const unsigned int type) {
    // create a shader object in the openGL context and compile the shader, then return the shader id
    const unsigned int shaderID = glCreateShader(type);
    const char* const _source = source.c_str();
    glShaderSource(shaderID, 1, &_source, NULL);
    glCompileShader(shaderID);
    if (!checkCompilationErrors(shaderID)) {
        print();
        printSource(source);
    }

    return shaderID;
}
const unsigned int Shader::compileShader(const std::string& source, const unsigned int type, const std::string fileName) {
    // create a shader object in the openGL context and compile the shader, then return the shader id
    const unsigned int shaderID = glCreateShader(type);
    const char* const _source = source.c_str();
    glShaderSource(shaderID, 1, &_source, NULL);
    glCompileShader(shaderID);
    if (!checkCompilationErrors(shaderID, fileName)) {
        print();
        printSource(source);
    }

    return shaderID;
}
bool Shader::checkLinkingErrors() const {
    // OpenGL keeps track of linking errors and they can be printed.
    int success;
    char infoLog[512];
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(programID, 512, NULL, infoLog);
        std::cout << "ERROR::PROGRAM::LINKER_FAILED:\n" << infoLog << std::endl;
    }
    return success;
}
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
bool Shader::checkCompilationErrors(const unsigned int shader) const {
    // OpenGL keeps track of compilation errors as well.
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED: " << infoLog << std::endl;
    }
    return success;
}
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
    // Read from a material struct and copy data to the identical sub-struct in the shader program. Uniforms are dependent on material type.
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


Format Shader::getJSON() {
    Format object;
    object["rendering_style"] = rendering_style;
    object["output_buffer"] = output_buffer;
    object["material_style"] = material_style;
    object["lighting_style"] = lighting_style;
    object["shadow_style"] = shadow_style;
    object["texture_style"] = texture_style;
    object["postprocessing"] = postprocessing;

    std::string v_path = std::string(SHADER_SAVE_PATH) + genVertexShaderName(), 
                f_path = std::string(SHADER_SAVE_PATH) + genFragmentShaderName();

    if (!f_exists(v_path)) f_write(v_path, v_source.c_str(), v_source.length(), TXT);
    if (!f_exists(f_path)) f_write(f_path, f_source.c_str(), f_source.length(), TXT);

    return object;
}
std::string Shader::loadFile(std::string fileName, std::string path) {
    // add shader directory root to the fileName
    std::string filePath = path + fileName;

    auto [source, size] = f_read(filePath, TXT);
    std::string contents = std::string(source, size);
    delete[] source;

    return contents;
}
std::string Shader::genVertexShaderName() {
    std::string v_shader_name = "v";
    switch(rendering_style) {
    case R_BASIC_2D: { v_shader_name += "_2D"; } break;
    case R_BASIC_3D: { v_shader_name += "_3D"; } break;
    case R_LIGHTING_3D: { 
        v_shader_name += "_lighting_3D"; 
        switch(shadow_style) {
        case S_SHADOW_MAPPING: { v_shader_name += "_sm"; } break;
        }
    } break;
    case R_SKYBOX: { v_shader_name += "_skybox"; } break;
    }

    switch(texture_style) {
    case T_BASIC_2D: { v_shader_name += "_textured"; } break;
    case T_CUBE: { v_shader_name += "_textured_c"; } break;
    }
    return "v_shaders/" + v_shader_name + ".glsl";
}
std::string Shader::genFragmentShaderName() {
    std::string f_shader_name = "f";
    switch(rendering_style) {
    case R_BASIC_2D: { f_shader_name += "_2D"; } break;
    case R_BASIC_3D: { f_shader_name += "_3D"; } break;
    case R_LIGHTING_3D: { 
        f_shader_name += "_lighting_3D"; 
        switch(lighting_style) {
        case L_DIR: { f_shader_name += "_d"; } break;
        case L_POINT: { f_shader_name += "_p"; } break;
        case L_SPOT: { f_shader_name += "_s"; } break;
        case L_DIR_POINT: { f_shader_name += "_dp"; } break;
        case L_DIR_SPOT: { f_shader_name += "_ds"; } break;
        case L_POINT_SPOT: { f_shader_name += "_ps"; } break;
        case L_ALL_ENABLED: { f_shader_name += "_dps"; } break;
        }
        switch(material_style) {
        case M_BASIC: { f_shader_name += "_basic_mat"; } break;
        case M_D_MAP: { f_shader_name += "_d_map"; } break;
        case M_DS_MAP: { f_shader_name += "_ds_map"; } break;
        case M_DSE_MAP: { f_shader_name += "_dse_map"; } break;
        }
        switch(shadow_style) {
        case S_SHADOW_MAPPING: { f_shader_name += "_sm"; } break;
        }
    } break;
    case R_SKYBOX: { f_shader_name += "_skybox"; } break;
    }
    switch(output_buffer) {
    case B_DEPTH: { f_shader_name += "_depth"; } break;
    case B_STENCIL: { f_shader_name += "_stencil"; } break;
    }
    switch(texture_style) {
    case T_BASIC_2D: { f_shader_name += "_textured"; } break;
    case T_CUBE: { f_shader_name += "_textured_c"; } break;
    }
    switch(postprocessing) {
    case P_BLUR: { f_shader_name += "_blur"; } break;
    case P_SHADOW_MAP: { f_shader_name += "_s_map"; } break;
    case P_DEPTH_MAP: { f_shader_name += "_d_map"; } break;
    case P_LINEARIZED_DEPTH_MAP: { f_shader_name += "_ld_map"; } break;
    }
    return "f_shaders/" + f_shader_name + ".glsl";
}


std::string Shader::generateVertexShader() {
    std::string v_source = "#version " + std::to_string((int) (VERSION * 100)) + " ";
    switch(PROFILE) {
    case P_CORE: { v_source += "core"; } break;
    }
    v_source += "\n";

    std::vector<std::unique_ptr<std::string>> v_components;
    addComponent(v_components, RENDERING_FILE, RENDERING_KEYS[rendering_style], GL_VERTEX_SHADER);
    addComponent(v_components, TEXTURE_FILE, TEXTURE_KEYS[texture_style], GL_VERTEX_SHADER);
    if (rendering_style == R_LIGHTING_3D)
        addComponent(v_components, SHADOW_FILE, SHADOW_KEYS[shadow_style], GL_VERTEX_SHADER);

    for (std::string key : STRUCTURAL_KEYS) {
        std::string section = "";
        for (int c = 0; c < v_components.size(); c++) section += getKeyedSection(*v_components[c], '@', key);
        defineVars(section, "&&");
        defineVars(section, '&');
        if (section.length() > 0) v_source += '\n' + section;
    }
    size_t c = 0, pos = v_source.find('$');
    while(pos != std::string::npos) {
        v_source.replace(pos, 1, std::to_string(c++));
        pos = v_source.find('$');
    }

    return v_source;
}
std::string Shader::generateFragmentShader() {
    // version declaration
    f_source = "#version " + std::to_string((int) (VERSION * 100)) + " ";
    switch(PROFILE) {
    case P_CORE: { f_source += "core"; } break;
    }
    f_source += "\n";

    std::vector<std::unique_ptr<std::string>> f_components;

    addComponent(f_components, RENDERING_FILE, RENDERING_KEYS[rendering_style], GL_FRAGMENT_SHADER);
    addComponent(f_components, OUTPUT_FILE, OUTPUT_KEYS[output_buffer], GL_FRAGMENT_SHADER);
    addComponent(f_components, TEXTURE_FILE, TEXTURE_KEYS[texture_style], GL_FRAGMENT_SHADER);
    if (rendering_style == R_LIGHTING_3D) {
        addComponent(f_components, LIGHTING_FILE, LIGHTING_KEYS[lighting_style], GL_FRAGMENT_SHADER);
        addComponent(f_components, SHADOW_FILE, SHADOW_KEYS[shadow_style], GL_FRAGMENT_SHADER);
        addComponent(f_components, MATERIAL_FILE, MATERIAL_KEYS[material_style], GL_FRAGMENT_SHADER );
    } else 
        addComponent(f_components, POSTPROCESSING_FILE, POSTPROCESSING_KEYS[postprocessing], GL_FRAGMENT_SHADER);

    for (std::string key : STRUCTURAL_KEYS) {
        std::string section = "";
        for (int c = 0; c < f_components.size(); c++) section += getKeyedSection(*f_components[c], '@', key);
        defineVars(section, "&&");
        defineVars(section, '&');
        if (section.length() > 0) f_source += '\n' + section;
    }

    return f_source;
}
void Shader::addComponent(std::vector<std::unique_ptr<std::string>>& components, 
                          const std::string fileName, const std::string key, const unsigned int shaderType) {
    std::string _fileName;
    switch(shaderType) {
    case GL_VERTEX_SHADER: { _fileName = "v_" + fileName; } break;
    case GL_FRAGMENT_SHADER: { _fileName = "f_" + fileName; } break;
    }
    std::string source = loadFile(_fileName, SHADER_COMPONENTS_PATH);
    if (contains(source, GENERAL_KEY)) components.push_back(std::make_unique<std::string>(getKeyedSection(source, "@@", GENERAL_KEY)));
    components.push_back(std::make_unique<std::string>(getKeyedSection(source, "@@", key)));
}
void Shader::defineVars(std::string& section, char flag) {
    while(contains(section, flag)) {
        size_t var_start = section.find(flag), var_end = section.find(flag, var_start + 1), fill_start = 0, fill_end = 0;
        std::string var = section.substr(var_start, var_end - var_start), fill = "";
        if (contains(var, '\n')) {
            var = var.substr(0, var.find('\n')) + flag;
            fill = section.substr(var_start + var.length(), var_end - var_start - var.length());
            section.erase(var_start, var_end - var_start + 2);
            fill_start = section.find(var);
            if (fill_start != std::string::npos) {
                fill_end = section.find(flag, fill_start + 1);
                section.replace(fill_start, fill_end - fill_start + 1, fill);
            }
            #if DEBUG_SHADER_BUILDER_SHOW_UNUSED_VARS
                else std::cout << "ERROR::SHADER::UNUSED_VARIABLE: Variable \"" << var << "\" defined but not used." << std::endl;
            #endif
        } else {
            var += '\n';
            if (!contains(section, var)) {
                std::cout << "ERROR::SHADER::MISSING_DEFINITION: Variable \"" << var.substr(0, var.length() - 1) 
                    << flag << "\" undefined." << std::endl;
                break;
            }
            fill_start = section.find(var), fill_end = section.find(flag, fill_start + 1);
            fill = section.substr(fill_start + var.length(), fill_end - fill_start - var.length());
            section.replace(var_start, var_end - var_start + 1, fill);
            size_t l = fill_end - fill_start + 2;
            fill_start += (var_start < fill_start) ? fill.length() - var_end + var_start - 1 : 0;
            section.erase(fill_start, l);
        }
    }
}
void Shader::defineVars(std::string& section, std::string flag) {
    while(contains(section, flag)) {
        size_t var_start = section.find(flag), var_end = section.find(flag, var_start + 1), fill_start = 0, fill_end = 0;
        std::string var = section.substr(var_start, var_end - var_start), fill = "";
        if (contains(var, '\n')) {
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
            if (!contains(section, var)) {
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