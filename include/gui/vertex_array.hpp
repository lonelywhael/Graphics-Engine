#ifndef VERTEX_ARRAY_HPP
#define VERTEX_ARRAY_HPP

#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <string.h>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "io/file_io.hpp"
#include "io/format.hpp"

#include "elements.hpp"

struct VertexAttribute;

class Buffer;

// different types of buffers for storing vertex data vs. storing indices
enum buffer_type {
    INDEX_BUFFER = GL_ELEMENT_ARRAY_BUFFER,
    VERTEX_BUFFER = GL_ARRAY_BUFFER
};
// different types for loading strategies
enum draw_type {
    STREAM = GL_STREAM_DRAW,    // use once
    STATIC = GL_STATIC_DRAW,    // use many times, but don't change often
    DYNAMIC = GL_DYNAMIC_DRAW   // use many times and change a lot
};
// different data types
enum data_type {
    BYTE = GL_BYTE,
    UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
    INT = GL_INT,
    UNSIGNED_INT = GL_UNSIGNED_INT,
    FLOAT = GL_FLOAT,
    DOUBLE = GL_DOUBLE
};
enum geometry_type {
    G_SAVED = 0,
    G_PANE = 1,
    G_PLANE = 2,
    G_SPHERE = 3
};
extern size_t getSize(unsigned int dataType);

/* VERTEX ATTRIBUTE STRUCT
 * 
 * Vertex attributes are organizational units that stores instructions about how to read data from a vertex buffer. An "attribute" is a
 * variable associated with each vertex. It could be something like a position vector, a normal vector, or an rgb color value. 
 */
struct VertexAttribute {
    unsigned int dimension, dataType, normalized;
    void* offset;

    VertexAttribute(unsigned int dimension, unsigned int dataType, unsigned int normalized, void* offset)
            : dimension(dimension), dataType(dataType), normalized(normalized), offset(offset) {}

    void print() const;
};

/* VERTEX ARRAY CLASS
 * 
 * A vertex array is an object used to encode data to the openGL context that can be accessed by programs during rendering. The vertex
 * array stores raw data (vertices) as well as instructions for how to read off those vertices and it instructs the openGL context how to
 * use this data. 
 * 
 * The vertex array needs buffers to handle data and it needs vertex attributes to feed instructions to the openGL about the location of
 * those attributes. Once this is done, the program can simply pull attributes by their index, e.g., the GLSL keyword "layout (location = 0)" 
 * refers to the information stored at the 0th vertex attribute, which would typically be a position vector.
 * 
 * Vertex arrays can be created either by reading vertex data from a file or by running a routine that generates a preset geometric pattern.
 *  - A frame cover is a rectangular pane that covers part of the viewport
 *  - A height map is a 2D manifold in 3D space that allows the user to define a function for height in cartesian coordinates
 *  - A sphere map is a 2D manifold in 3D space that allows the user to define a function for radius in 3D cartesian coordinates
 * 
 * Vertex arrays can be saved and loaded from save files. These saves are stored as binary files. See the saveVertexArray() method in the 
 * .cpp file for further details.
 * 
 * Like other interfaces with the openGL context, vertex arrays should be held in a strict 1 to 1 correspondence with OpenGL vertex array
 * objects. Vertex arrays should not be copied but instead passed by reference or pointer.
 */

// Null functions can be passed to the height map and sphere map functions if a "flat" surface is desired
extern float (*PLANE_FUNCTIONS[])(float, float);
enum plane_functions {
    PF_NULL = 0,
    PF_HILL = 1
};
extern float (*SPHERE_FUNCTIONS[])(glm::vec3);
enum sphere_functions {
    SF_NULL = 0
};

class VertexArray {
public:
    // Need non-default constructor and destructor for creating/destroying the parallel vertex array object in the openGL context
    VertexArray();
    // read a vertex array from a save file, using the specified render strategy
    VertexArray(std::string file_name, const unsigned int draw_type);
    VertexArray(Format& object);
    ~VertexArray();

    VertexArray(const VertexArray&) = delete;
    void operator=(const VertexArray&) = delete;

    bool operator==(VertexArray& other);

    // functions for creating vertex data for specific kinds of geometric structures (see above)
    void makePane(const float cornerX = -1.0f, const float cornerY = -1.0f, const float dimX = 2.0f, const float dimY = 2.0f);
    void makeHeightMap(const unsigned int resolution, const unsigned int function)
        { geometry_type = G_PLANE; function_id = function; makeHeightMap(resolution, PLANE_FUNCTIONS[function]); }
    void makeHeightMap(const unsigned int resolution, const unsigned int function, const unsigned int draw_type)
        { geometry_type = G_PLANE; function_id = function; makeHeightMap(resolution, PLANE_FUNCTIONS[function], draw_type); }
    void makeHeightMap(const unsigned int resolution, float (*heightFunction)(float, float), const unsigned int draw_type)
        { this->draw_type = draw_type; makeHeightMap(resolution, heightFunction); }
    void makeHeightMap(const unsigned int resolution, float (*heightFunction)(float, float));
    void makeSphereMap(const unsigned int resolution, const unsigned int function)
        { geometry_type = G_SPHERE; function_id = function; makeSphereMap(resolution, SPHERE_FUNCTIONS[function]); }
    void makeSphereMap(const unsigned int resolution, const unsigned int function, const unsigned int draw_type)
        { geometry_type = G_SPHERE; function_id = function; makeSphereMap(resolution, SPHERE_FUNCTIONS[function], draw_type); }
    void makeSphereMap(const unsigned int resolution, float (*heightFunction)(glm::vec3), const unsigned int draw_type)
        { this->draw_type = draw_type; makeSphereMap(resolution, heightFunction); }
    void makeSphereMap(const unsigned int resolution, float (*heightFunction)(glm::vec3));

    // bind/unbind the vertex array to/from the openGl context
    void bind() const { glBindVertexArray(vertexArrayID); }
    void unbind() const { glBindVertexArray(0); }

    // add a buffer to the vertex array
    void addBuffer(const unsigned int bufferType, const void*& data, const size_t size, const unsigned int count);
    void addBuffer(const unsigned int bufferType, void*&& data, const size_t size, const unsigned int count);
    // bind or unbind a buffer that is attached to the vertex array by index
    void bindBuffer(const unsigned int index);
    void unbindBuffer(const unsigned int index);

    // add an attribute to the vertex array
    void addAttribute(const unsigned int dimension, const unsigned int draw_type, const unsigned int normalized);
    // after adding all attributes needed to read the buffer, must call the activateAll() function to write them to the openGL context
    void activateAll();

    // enable/disable certain attributes
    void enableAttribute(const unsigned int index) const { glEnableVertexAttribArray(index); }
    void disableAttribute(const unsigned int index) const { glDisableVertexAttribArray(index); }

    // retrieve the length of the currently active buffers
    unsigned int getVertexCount() const;
    unsigned int getIndexCount() const;

    // save the vertex array to a binary file
    Format getJSON() const;
    void save(std::string fileName);
    void load(std::string fileName);

    void print() const;
private:
    // the id used to reference the vertex array object created in the openGL context
    unsigned int vertexArrayID;
    // list of attributes and buffers
    std::vector<std::unique_ptr<VertexAttribute>> vertexAttributes = {};
    std::vector<std::unique_ptr<Buffer>> buffers = {};

    // total size of each individual vertex
    size_t stride = 0;

    unsigned int draw_type;
    unsigned int geometry_type = -1;
    std::string file_name;
    std::array<float, 4> pane_dims;
    unsigned int function_id, resolution;

    // total number of vertices and indices in the currently bound buffers
    unsigned int activeVertexBuffer = -1, activeIndexBuffer = -1;

    void genOpenGL();

    // functions for adding simple structures to and making geometric calculations
    // adds a (non-textured) vertex using the given data at the specified location
    void addVertex(float* vertex, const glm::vec3 pos, const glm::vec3 norm) const;
    // adds a (non-textured) triangle using the given data at the specied location
    void addTriangle(float* vertex, const glm::vec3 v1, const glm::vec3 v2, const glm::vec3 v3) const;
    // returns the normal vector to the plane specified by three points
    glm::vec3 getNorm(const glm::vec3 v1, const glm::vec3 v2, const glm::vec3 v3) const
        { return glm::normalize(glm::cross(v1 - v2, v3 - v1)); }

    // activate an individual vertex attribute in the openGL context
    void activateAttribute(unsigned int index, const std::unique_ptr<const VertexAttribute> attribute) const;
};

/* BUFFER CLASS
 *
 * The buffer class stores data and loads it to eh openGL context. It is essentially an array of any kind of data type that is linked to
 * the openGL context.
 * 
 * Like other interfaces with the openGL context, buffers should be held in a strict 1 to 1 correspondence with OpenGL buffer objects. 
 * Buffers should not be copied but instead passed by reference or pointer.
 */

class Buffer {
    friend VertexArray;
public:
    // Constructor needs the type of array (vertex or index), the pointer to data, the size and number of elements, and rendering strategy
    Buffer(const unsigned int bufferType, const void*& data, const size_t size, const unsigned int count, const unsigned int draw_type);
    Buffer(const unsigned int bufferType, void*&& data, const size_t size, const unsigned int count, const unsigned int draw_type);
    // Non-default destructor needed in order to delete the parallel buffer object in the openGL context
    ~Buffer();

    Buffer(const Buffer&) = delete;
    void operator=(const Buffer&) = delete;

    // bind/unbind the buffer object to/from the openGL context
    void bind() const {
        // bind buffer. Only one buffer of a given buffer type can be bound at any time.
        glBindBuffer(type, bufferID);
        // send buffer data to the openGL context (takes a bit of time, limited by latency)
        glBufferData(type, size, data, draw_type);
    }
    void unbind() { glBindBuffer(type, 0); };

    void print() const;
private:
    // id used to reference the parallel buffer object in the openGL context
    unsigned int bufferID;

    // parameters describing the buffer
    unsigned int type;    // either an index or vertex buffer
    unsigned int count;         // number of elements in the buffer
    size_t size;                // size of buffer
    void* data;           // pointer to buffer data
    /* how will this data be rendered? (stored in renderType)
     * GL_STREAM_DRAW: set once and drawn a few times
     * GL_STATIC_DRAW: set once and drawn many times
     * GL_DYNAMIC_DRAW: set many times and drawn many times
     */
    unsigned int draw_type;
};

#endif