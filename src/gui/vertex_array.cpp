#include "gui/vertex_array.hpp"

float NULL_FUNCTION(float, float) { return 0; }
float HILL_FUNCTION(float x, float y) {
    const float K = 1.0f;
    x *= K; y *= K;
    float c = 0.1 / K;
    return c * std::exp(100 * (-(x * x) - (y * y)));
}
float (*PLANE_FUNCTIONS[])(float, float) { NULL_FUNCTION, HILL_FUNCTION };

float NULL_FUNCTION(glm::vec3) { return 1; }
float (*SPHERE_FUNCTIONS[])(glm::vec3) { NULL_FUNCTION };

const char MESH_PATH[] = "../res/meshes/";
const size_t ATTRIB_OVERHEAD = 3 * sizeof(unsigned int), BUFFER_OVERHEAD = sizeof(unsigned int) + sizeof(size_t);

const unsigned int VERTEX_SIZE = 6;

size_t getSize(unsigned int dataType) {
    // function from data type id to sizeof(data type)
    size_t size;
    switch (dataType) {
    case GL_BYTE:
        size = sizeof(char);
        break;
    case GL_UNSIGNED_BYTE:
        size = sizeof(unsigned char);
        break;
    case GL_SHORT:
        size = sizeof(short);
        break;
    case GL_UNSIGNED_SHORT:
        size = sizeof(unsigned short);
        break;
    case GL_INT:
        size = sizeof(int);
        break;
    case GL_UNSIGNED_INT:
        size = sizeof(unsigned int);
        break;
    case GL_FLOAT:
        size = sizeof(float);
        break;
    case GL_DOUBLE:
        size = sizeof(double);
        break;
    default:
        std::cout << "ERROR::VERTEX_ARRAY::UNKNOWN_DATA_TYPE";
        size = -1;
        break;
    }
    return size;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void VertexAttribute::print() const {
    std::cout << "Dimension: " << dimension << std::endl;
    std::cout << "Type: ";
    switch (dataType) {
    case GL_BYTE: { std::cout << "Byte (8 bit)"; } break;
    case GL_UNSIGNED_BYTE: { std::cout << "Unsigned byte (8 bit)"; } break;
    case GL_SHORT: { std::cout << "Short (16 bit)"; } break;
    case GL_UNSIGNED_SHORT: { std::cout << "Unsigned short (16 bit)"; } break;
    case GL_INT: { std::cout << "Integer (32 bit)"; } break;
    case GL_UNSIGNED_INT: { std::cout << "Unsigned integer (32 bit)"; } break;
    case GL_HALF_FLOAT: { std::cout << "Half float (16 bit)"; } break;
    case GL_FLOAT: { std::cout << "Float (32 bit)"; } break;
    case GL_DOUBLE: { std::cout << "Double (64 bit)"; } break;
    default: { std::cout << "Unknown data type"; } break;
    }
    std::cout << std::endl;
    std::cout << "Normalized: ";
    if (normalized == GL_TRUE) std::cout << "True";
    else std::cout << "False";
    std::cout << std::endl;
    std::cout << offset << std::endl;
}

//------------------------------------------------------------------------------------------------------------------------------------------

VertexArray::VertexArray() { genOpenGL(); }
VertexArray::VertexArray(std::string file_name, unsigned int draw_type) 
        : file_name(file_name), draw_type(draw_type) {
    genOpenGL();
    load(file_name);
}
VertexArray::VertexArray(Format& object) 
        : draw_type(static_cast<unsigned int>(object["draw_type"])), geometry_type(static_cast<unsigned int>(object["geometry_type"])) {
    genOpenGL();

    switch(geometry_type) {
    case G_SAVED: { load(static_cast<std::string>(object["file_name"])); } break;
    case G_PANE: { makePane(static_cast<float>(object["pane_dims"][0]), static_cast<float>(object["pane_dims"][1]),
                            static_cast<float>(object["pane_dims"][2]), static_cast<float>(object["pane_dims"][3])); } break;
    case G_PLANE: { makeHeightMap(static_cast<unsigned int>(object["resolution"]), static_cast<unsigned int>(object["function_id"])); } break;
    case G_SPHERE: { makeSphereMap(static_cast<unsigned int>(object["resolution"]), static_cast<unsigned int>(object["function_id"])); } break;
    }
}

bool VertexArray::operator==(VertexArray& other) {
    if (draw_type == other.draw_type && geometry_type == other.geometry_type) {
        switch(geometry_type) {
        case G_SAVED: { return file_name == other.file_name; } break;
        case G_PANE: { 
            return pane_dims[0] == other.pane_dims[0] && pane_dims[1] == other.pane_dims[1] &&
                   pane_dims[2] == other.pane_dims[2] && pane_dims[3] == other.pane_dims[3]; 
        } break;
        case G_PLANE: case G_SPHERE: { return resolution == other.resolution && function_id == other.function_id; } break;
        }
    } 
    return false;
}

void VertexArray::genOpenGL() {
    // create a vertex array object in the openGL context
    glGenVertexArrays(1, &vertexArrayID);
    #if DEBUG_OPENGL_OBJECTS 
        std::cout << "VertexArray " << vertexArrayID << " was created." << std::endl;
    #endif
}
VertexArray::~VertexArray() {
    // delete the vertex array object in the openGl context
    glDeleteVertexArrays(1, &vertexArrayID); 
    #if DEBUG_OPENGL_OBJECTS 
        std::cout << "VertexArray " << vertexArrayID << " was deleted." << std::endl;
    #endif
}

void VertexArray::makePane(const float cornerX, const float cornerY, const float dimX, const float dimY) {
    draw_type = STATIC;
    geometry_type = G_PANE;
    pane_dims[0] = cornerX; pane_dims[1] = cornerY; pane_dims[2] = dimX; pane_dims[3] = dimY;
    // create a single rentangle to be covered by a texture given its lower lefthand corner and dimensions (in clip space)

    const unsigned int N_VERTICES = 4, N_INDICES = 6;   // 1 rectangle has 4 corners, 2 triangles have 6 corners
    // each vertex will store a 2D position value and a texture coordinate (all floats)
    const size_t VERTEX_SIZE = 4 * sizeof(float), INDEX_SIZE = sizeof(unsigned int);

    float vertices[] {
        cornerX,        cornerY,            0.0f, 0.0f, // lower left
        cornerX,        cornerY + dimY,     0.0f, 1.0f, // upper left
        cornerX + dimX, cornerY,            1.0f, 0.0f, // lower right
        cornerX + dimX, cornerY + dimY,     1.0f, 1.0f  // upper right
    };
    unsigned int indices[] {
        0, 1, 2,    1, 3, 2     // two triangles arranged CW
    };

    const void* p_vertices = &vertices, *p_indices = &indices;

    // bind the vertex array
    bind();
    // add the vertex and index arrays
    addBuffer(VERTEX_BUFFER, p_vertices, N_VERTICES * VERTEX_SIZE, N_VERTICES);
    addBuffer(INDEX_BUFFER, p_indices, N_INDICES * INDEX_SIZE, N_INDICES);
    // add the vertex attributes
    addAttribute(2, FLOAT, false);  // 2D position
    addAttribute(2, FLOAT, false);  // texture coordinate
    // activate vertex attributes
    activateAll();
    unbind();
}
void VertexArray::makeHeightMap(const unsigned int resolution, float (*heightFunction)(const float, const float)) {
    /* A height map can be thought of as a grid of patches, each of which is split into two triangles. The height value of each vertex is 
     * determined by the input heighFunction as a function of the x and z grid values. 
     */

    this->resolution = resolution;

    const unsigned int PATCH_CONST = 6;     // for each "patch" (tile) there are 2 triangles and 6 vertices

    // compute the number of vertices, which is the total number of patches * the number of vertices in a patch
    unsigned int vertexCount = (resolution - 1) * (resolution - 1) * PATCH_CONST;
    // compute the length of the vertex array, which is the number of vertices * (3 position values + 3 norm values) * sizeof(float)
    size_t vertexSize = vertexCount * VERTEX_SIZE * sizeof(float);

    // create a vertex array of the appropriate size
    void* vertices = malloc(vertexSize);

    // simple inline function for converting [0, resolution] to [-0.5, 0.5]
    auto norm = [resolution] (int x) -> float 
        { return ((float) x / (float) (resolution - 1)) - 0.5; };

    // iterate through each patch and add two triangles
    for (int x = 0; x < resolution - 1; x++) for (int z = 0; z < resolution - 1; z++) {
        // convert two dimensional patch array to one dimensional float array
        int i = x * (resolution - 1) * VERTEX_SIZE * PATCH_CONST + z * VERTEX_SIZE * PATCH_CONST;
        // compute 4 corners of the patch in clip coordinates
        glm::vec3 v[4] = { glm::vec3(norm(x    ), heightFunction(norm(x    ), norm(z    )), norm(z    )),   // bottom left
                           glm::vec3(norm(x + 1), heightFunction(norm(x + 1), norm(z    )), norm(z    )),   // bottom right
                           glm::vec3(norm(x    ), heightFunction(norm(x    ), norm(z + 1)), norm(z + 1)),   // top left
                           glm::vec3(norm(x + 1), heightFunction(norm(x + 1), norm(z + 1)), norm(z + 1)) }; // top right
        // add two triangles that cover this patch, both arranged in CW order
        addTriangle((float*) vertices + i, v[0], v[1], v[2]);
        addTriangle((float*) vertices + i + 3 * VERTEX_SIZE, v[3], v[2], v[1]);
    }

    bind();
    // add the array data as a buffer object
    addBuffer(VERTEX_BUFFER, std::move(vertices), vertexSize, vertexCount);
    // add vertex attributes
    addAttribute(3, FLOAT, false);  // 3D position vector
    addAttribute(3, FLOAT, false);  // 3D norm vector
    // activate all attributes
    activateAll();
    unbind();
}
void VertexArray::makeSphereMap(unsigned int resolution, float (*heightFunction)(glm::vec3)) {
    /* A sphere is generated by creating an octohedron and recursively dividing its faces into smaller and smaller triangles. All the
     * vertices of each face are then normed to produce a sphere. From that point, they can be further adjusted by the inputed height 
     * function.
     */

    /* The "depth" is the number of recursion that are performed. On the nth layer of recursion, there are 4^n trangles on each face. For
     * ease of user input, the user is supposed to enter a resolution value. In the analagous plane case, the user expects there to be 2 *
     * resolution^2 polygons. Therefore we want to solve for the smallest n s.t.: 8 * 4^n > 2 * resolution^2.
     * This gives us depth = floor(ln_4(0.25 * resolution^2)) + 1, where the number of polygons is 8 * 4^depth
     */

    this->resolution = resolution; 

    const unsigned int depth = (int) (std::log(0.25 * resolution * resolution) / std::log(4)) + 1;
    unsigned int vertexCount = 8 * (int) std::pow(4, depth) * 3;    // # of vertices is 3 * number of polygons
    size_t vertexSize = vertexCount * VERTEX_SIZE * sizeof(float);

    // allocate a vertex array of the appropriate size
    void* vertices = malloc(vertexSize);

    // hard coded values for the vertices of the octohedron
    glm::vec3 octoVertices[6] {
        glm::vec3{  0.5f,  0.0f,  0.0f }, // 0 right
        glm::vec3{  0.0f,  0.0f,  0.5f }, // 1 front
        glm::vec3{ -0.5f,  0.0f,  0.0f }, // 2 left
        glm::vec3{  0.0f,  0.0f, -0.5f }, // 3 back
        glm::vec3{  0.0f,  0.5f,  0.0f }, // 4 top
        glm::vec3{  0.0f, -0.5f,  0.0f }  // 5 bottom
    };

    // indexing the faces of the octohedron so that vertices are in CW order
    unsigned int indices[8 * 3] {
        0, 1, 4,    0, 5, 1,
        1, 2, 4,    1, 5, 2,
        2, 3, 4,    2, 5, 3,
        3, 0, 4,    3, 5, 0
    };

    // create a vertex variable that iterates through the vertex array and can be accessed by the recursion function
    float* vertex = (float*) vertices;

    // create the recursion function that splits a triangle into 4 subtriangles while maintaining parity
    std::function<void(unsigned int layer, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3)> split;
    split = [&] (unsigned int layer, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
        // if the layer reached is the depth layer, stop the recursion and add the triangles to the array
        if (layer == depth) {
            // normalize each vertex to a length of 0.5 then modify using the height function, writing the triangle to the current vertex location
            this->addTriangle(vertex, 
                              0.5f * heightFunction(v1) * glm::normalize(v1), 
                              0.5f * heightFunction(v3) * glm::normalize(v2), 
                              0.5f * heightFunction(v2) * glm::normalize(v3));
            // increment the vertex location to add the next triangle
            vertex += 3 * VERTEX_SIZE;
        } else {    // otherwise, continue the recursion
            // find the midpoint of each side of the triangle
            glm::vec3 m12 = 0.5f * (v1 + v2), m23 = 0.5f * (v2 + v3), m31 = 0.5f * (v3 + v1);
            // generate 4 new triangles using the given vertices and the mid points, maintaining the parity of the triangles, and calling the next layer of recursion
            split(layer + 1, v1, m12, m31);
            split(layer + 1, m12, v2, m23);
            split(layer + 1, m31, m23, v3);
            split(layer + 1, m12, m23, m31);
        }
    };

    // generate vertices for each of 8 faces of the octohedron
    for (int f = 0; f < 8; f++) {
        // get the vertices that correspond to the current face by using the index array (which orders the vertices CW)
        glm::vec3 v[] { octoVertices[indices[3 * f + 0]],
                        octoVertices[indices[3 * f + 1]], 
                        octoVertices[indices[3 * f + 2]] };
        // call the recursion function on these three vertices, starting at layer 0 (4^0 = 1 triangle). The recursion function maintains vertex parity.
        split(0, v[0], v[1], v[2]);
    }

    bind();
    // create a vertex buffer using the vertex array data
    addBuffer(VERTEX_BUFFER, std::move(vertices), vertexSize, vertexCount);
    // add attributes
    addAttribute(3, FLOAT, false);  // 3D position data
    addAttribute(3, FLOAT, false);  // 3D norm data
    // activate attributes
    activateAll();
    unbind();
}

void VertexArray::addVertex(float* vertex, const glm::vec3 pos, const glm::vec3 norm) const {
    // assign vertex data at the given location
    vertex[0] = pos.x; vertex[1] = pos.y; vertex[2] = pos.z;
    vertex[3] = norm.x; vertex[4] = norm.y; vertex[5] = norm.z;
}
void VertexArray::addTriangle(float* vertex, const glm::vec3 v1, const glm::vec3 v2, const glm::vec3 v3) const {
    // given three vertices, calculate the normal vector to the plane they form, then add each of them as vertices
    glm::vec3 norm = getNorm(v1, v2, v3);
    addVertex((float*) vertex + VERTEX_SIZE * 0, v1, norm);
    addVertex((float*) vertex + VERTEX_SIZE * 1, v2, norm);
    addVertex((float*) vertex + VERTEX_SIZE * 2, v3, norm);
}

void VertexArray::addBuffer(const unsigned int bufferType, const void*& data, const size_t size, const unsigned int count) {
    // create a new buffer object to be stored within the vertex array
    std::unique_ptr<Buffer> buffer = std::make_unique<Buffer>(bufferType, data, size, count, draw_type);
    // add an existing buffer object to be stored within the vertex array
    buffer->bind(); // bind the new buffer by default
    switch(buffer->type) { // update vertex/index count accordingly
    case VERTEX_BUFFER:
        activeVertexBuffer = buffers.size();
        break;
    case INDEX_BUFFER:
        activeIndexBuffer = buffers.size();
        break;
    }
    buffers.push_back(std::move(buffer));
}
void VertexArray::addBuffer(const unsigned int bufferType, void*&& data, const size_t size, const unsigned int count) {
    // create a new buffer object to be stored within the vertex array
    std::unique_ptr<Buffer> buffer = std::make_unique<Buffer>(bufferType, std::move(data), size, count, draw_type);
    // add an existing buffer object to be stored within the vertex array
    buffer->bind(); // bind the new buffer by default
    switch(buffer->type) { // update vertex/index count accordingly
    case VERTEX_BUFFER:
        activeVertexBuffer = buffers.size();
        break;
    case INDEX_BUFFER:
        activeIndexBuffer = buffers.size();
        break;
    }
    buffers.push_back(std::move(buffer));
}
void VertexArray::bindBuffer(const unsigned int index) {
    // bind a buffer to the openGL context and update the vertex/index count accordingly
    buffers[index]->bind();
    switch(buffers[index]->type) {
    case VERTEX_BUFFER:
        activeVertexBuffer = index;
        break;
    case INDEX_BUFFER:
        activeIndexBuffer = index;
        break;
    }
}
void VertexArray::unbindBuffer(const unsigned int index) {
    // unbind a buffer from the openGL context and update the vertex/index count accordingly
    buffers[index]->unbind();
    switch(buffers[index]->type) {
    case VERTEX_BUFFER:
        activeVertexBuffer = -1;
        break;
    case INDEX_BUFFER:
        activeIndexBuffer = -1;
        break;
    }
}

void VertexArray::addAttribute(const unsigned int dimension, const unsigned int dataType, const unsigned int normalized) {
    // set the offset of the attribute to the current stride (the size of all added attributes so far)
    void* offset = (void*) stride;
    // increase the stride by the size of the new attribute
    stride += dimension * getSize(dataType);

    // create a new vertex attribute and add it to the list
    std::unique_ptr<VertexAttribute> attribute = std::make_unique<VertexAttribute>(dimension, dataType, normalized, offset);
    vertexAttributes.push_back(std::move(attribute));
}
void VertexArray::activateAll() {
    // once all attributes have been added to the cpu object, the stride should be the correct value and all attributes can be added to the openGL context
    for (int i = 0; i < vertexAttributes.size(); i++) {
        glVertexAttribPointer(i, vertexAttributes[i]->dimension, vertexAttributes[i]->dataType, vertexAttributes[i]->normalized, stride, vertexAttributes[i]->offset);
        glEnableVertexAttribArray(i);
    }
}

unsigned int VertexArray::getVertexCount() const { return (activeVertexBuffer != -1) ? buffers[activeVertexBuffer]->count : 0; }
unsigned int VertexArray::getIndexCount() const { return (activeIndexBuffer != -1) ? buffers[activeIndexBuffer]->count : 0; }

Format VertexArray::getJSON() const {
    if (geometry_type == -1) {
        std::cout << "ERROR::VERTEX_ARRAY::SAVING_ERROR: Vertex array object cannot be saved since it is not in a saveable format." << std::endl;
        return (Format) nullptr;
    }
    Format object;
    object["draw_type"] = draw_type;
    object["geometry_type"] = geometry_type;
    switch(geometry_type) {
    case G_SAVED: { object["file_name"] = file_name; } break;
    case G_PANE: { object["pane_dims"] = { pane_dims[0], pane_dims[1], pane_dims[2], pane_dims[3] }; } break;
    case G_PLANE: case G_SPHERE: { object["resolution"] = resolution; object["function_id"] = function_id; } break; }

    return object;
}
void VertexArray::save(std::string fileName) {
    this->file_name = fileName;

    /* File data is stored as follows:
     * | # of attributes (u_int) |                                                                  sizeof(u_int)
     * | dimension (u_int)    | data type (u_int)       | normalize (u_int)   | x # of attributes   3 * sizeof(u_int) * (# of attributes)
     * | buffer type (u_int)  | size of buffer (size_t) | buffer data         | x # of buffers      sizeof(u_int) + sizeof(size_t) + sizeof(buffer data), for however many buffers
     */
    const unsigned int N_ATTRIBS = vertexAttributes.size(), N_BUFFERS = buffers.size();

    // compute the amount of space attribute data will take up 
    unsigned int sizeAttrib = sizeof(unsigned int) + ATTRIB_OVERHEAD * N_ATTRIBS, sizeBuffer = 0;
    // compute the amount of space buffer data will take up
    for (int i = 0; i < N_BUFFERS; i++) sizeBuffer += BUFFER_OVERHEAD + buffers[i]->size;

    // create a new data array with the necessary amount of space allocated
    char *data = new char[sizeAttrib + sizeBuffer];

    // save the total number of attributes in the first slot
    *(unsigned int*)data = (unsigned int) N_ATTRIBS;
    // iterate through the attribute and add the data from each
    for (int i = 0; i < N_ATTRIBS; i++) {
        // grab the necessary data from the attribute
        unsigned int dim = vertexAttributes[i]->dimension, type = vertexAttributes[i]->dataType, norm = vertexAttributes[i]->normalized;

        // copy the data to the array at the location of the current vertex attribute
        *(unsigned int*)(&data[sizeof(unsigned int) + i * ATTRIB_OVERHEAD] + 0 * sizeof(unsigned int)) = dim;
        *(unsigned int*)(&data[sizeof(unsigned int) + i * ATTRIB_OVERHEAD] + 1 * sizeof(unsigned int)) = type;
        *(unsigned int*)(&data[sizeof(unsigned int) + i * ATTRIB_OVERHEAD] + 2 * sizeof(unsigned int)) = norm;
    }

    // set an accumulator to 0 that can keep track of the position of the current buffer
    size_t accum = 0;
    // iterate through all the buffers and add the data from each
    for (int i = 0; i < N_BUFFERS; i++) {
        // grab the necessary data from the buffer
        unsigned int type = buffers[i]->type;
        size_t size = buffers[i]->size;

        // copy the buffer data over to the location of the current buffer
        *(unsigned int*)(&data[sizeAttrib + accum]) = type;
        *(size_t*)(&data[sizeAttrib + accum] + sizeof(unsigned int)) = size;
        memcpy(&data[sizeAttrib + accum] + BUFFER_OVERHEAD, buffers[i]->data, size);  // here, use memcpy since the buffer stores a pointer to data

        // increase the accumulator by the size of the buffer
        accum += BUFFER_OVERHEAD + size;
    }

    // create the correct file path based on the given file name: ../res/meshes/(fileName)
    std::string filePath = MESH_PATH + fileName;

    f_write(filePath, data, sizeAttrib + sizeBuffer, BIN);

    // dispose of the data since it has been saved to the hard drive
    std::cout << "Data associated with vertex array " << vertexArrayID << " was freed." << std::endl;
    delete[] data;
}
void VertexArray::load(std::string file_name) {
    // determine the correct file path - ../res/meshes/(filename)
    geometry_type = G_SAVED;
    std::string filePath = MESH_PATH + file_name;

    auto [data, length] = f_read(filePath, BIN);

    // bind the vertex array
    bind();

    /* File data is stored as follows:
     * | # of attributes (u_int) |                                                                  sizeof(u_int)
     * | dimension (u_int)    | data type (u_int)       | normalize (u_int)   | x # of attributes   3 * sizeof(u_int) * (# of attributes)
     * | buffer type (u_int)  | size of buffer (size_t) | buffer data         | x # of buffers      sizeof(u_int) + sizeof(size_t) + sizeof(buffer data), for however many buffers
     */

    // get the number of attributes from the first unsigned int in the file data
    unsigned int nAttributes = *reinterpret_cast<const unsigned int*>(data);

    // next, read through each attribute and create a vertex attribute struct
    for (int i = 0; i < nAttributes; i++) {
        unsigned int dim, type, norm;

        // read the next three values from the next attribute location
        dim  = *reinterpret_cast<const unsigned int*>(&data[sizeof(unsigned int) + i * ATTRIB_OVERHEAD] + 0 * sizeof(unsigned int));
        type = *reinterpret_cast<const unsigned int*>(&data[sizeof(unsigned int) + i * ATTRIB_OVERHEAD] + 1 * sizeof(unsigned int));
        norm = *reinterpret_cast<const unsigned int*>(&data[sizeof(unsigned int) + i * ATTRIB_OVERHEAD] + 2 * sizeof(unsigned int));

        // use this data to create an attribute struct
        addAttribute(dim, type, norm);
    }

    // calculate the total size of all the attribute data and start an accumulator at that value to keep track of how much data has been read
    unsigned int accum = sizeof(unsigned int) + nAttributes * ATTRIB_OVERHEAD;
    // keep reading buffer data until the accumulator reaches the end of the data array
    while (accum < length) {
        unsigned int type;
        size_t size;
        const void* bufferData;

        // read the buffer type and size data from the next buffer location, then copy the pointer to the data array
        type = *reinterpret_cast<const unsigned int*>(&data[accum]);
        size = *reinterpret_cast<const size_t*>(&data[accum] + sizeof(unsigned int));
        bufferData = reinterpret_cast<const void*>(&data[accum] + BUFFER_OVERHEAD);

        // depending on the buffer type above, create the correct type of buffer object
        switch(type) {
        // a vertex buffer will use the stride value that was implicitly computed while adding vertex attributes
        case VERTEX_BUFFER: { addBuffer(type, bufferData, size, size / stride); } break;
        // an index buffer will always have elements of the type u_int
        case INDEX_BUFFER: { addBuffer(type, bufferData, size, size / sizeof(unsigned int)); } break;
        }

        // increase the accumulator by the size of the buffer overhead plus the size of the buffer itself
        accum += BUFFER_OVERHEAD + size;
    }

    // with all vertex attributes and buffers added, the vertex attributes can now be added
    activateAll();
    unbind();

    // there is no need to keep the file data allocated since it has been encoded as a vertex array
    #if DEBUG_OPENGL_OBJECTS
        std::cout << "Data associated with vertex array " << vertexArrayID << " was freed." << std::endl;
    #endif
    delete[] data;
}

void VertexArray::print() const {
    std::cout << "OpenGL ID: " << vertexArrayID << std::endl;
    for (int va = 0; va < vertexAttributes.size(); va++) vertexAttributes[va]->print();
    for (int b = 0; b < buffers.size(); b++) buffers[b]->print();
    std::cout << "Active Vertex Buffer: " << ((activeVertexBuffer == -1) ? "None" 
                 : (std::to_string(activeVertexBuffer) + " (Count = " + std::to_string(getVertexCount()) + ")")) << std::endl;
    std::cout << "Active Index Buffer: " << ((activeIndexBuffer == -1) ? "None" 
                 : (std::to_string(activeIndexBuffer) + " (Count = " + std::to_string(getIndexCount()) + ")")) << std::endl;
}

//------------------------------------------------------------------------------------------------------------------------------------------

Buffer::Buffer(const unsigned int type, const void*& _data, const size_t size, const unsigned int count, const unsigned int draw_type) 
        : type(type), size(size), count(count), draw_type(draw_type) { 
    // creates a buffer object in the openGL context and retrieves an ID to reference it in future
    glGenBuffers(1, &bufferID); 
    #if DEBUG_OPENGL_OBJECTS 
        std::cout << "Buffer " << bufferID << " was created (copied)." << std::endl;
    #endif
    //deep copy the data given to the buffer so that the data given can be deleted
    data = malloc(size);
    memcpy(data, _data, size);
}
Buffer::Buffer(const unsigned int type, void*&& data, const size_t size, const unsigned int count, const unsigned int draw_type) 
        : type(type), size(size), count(count), draw_type(draw_type) {
    glGenBuffers(1, &bufferID); 
    #if DEBUG_OPENGL_OBJECTS 
        std::cout << "Buffer " << bufferID << " was created (moved)." << std::endl;
    #endif
    std::cout << ((data == nullptr) ? "nullptr" : data) << std::endl;
    this->data = data;
    data = nullptr;
}
Buffer::~Buffer() {
    // delete the openGL buffer object
    glDeleteBuffers(1, &bufferID);
    #if DEBUG_OPENGL_OBJECTS
        std::cout << "Buffer " << bufferID << " was deleted." << std::endl;
        std::cout << "Data associated with buffer " << bufferID << " was freed." << std::endl;
    #endif
    free(data);
}

void Buffer::print() const {
    switch(type) {
    case VERTEX_BUFFER: { std::cout << "Vertex "; } break;
    case INDEX_BUFFER: { std::cout << "Index "; } break;
    }
    std::cout << "Buffer ID: " << bufferID << std::endl;
    std::cout << "Count: " << count << std::endl;
    std::cout << "Size: " << size << std::endl;
    unsigned int stride = size / count;
    std::cout << "Stride: " << stride << std::endl;
    for (int i = 0; i < count; i++) {
        char* nextElement = (char*) data + i * stride;
        switch(type) {
        case VERTEX_BUFFER: { 
            for (int j = 0; j < stride / sizeof(float); j++) 
                std::cout << *((float*) nextElement + j) << "\t"; 
        } break;
        case INDEX_BUFFER: { 
            for (int j = 0; j < stride / sizeof(unsigned int); j++) 
                std::cout << *((unsigned int*) nextElement + j) << "\t"; 
        } break;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}