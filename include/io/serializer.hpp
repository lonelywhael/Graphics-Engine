#ifndef SERIALIZER_HPP
#define SERIALIZER_HPP

#define DEBUG_CONSTRUCTOR_CALLS false

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "file_io.hpp"
#include "parser.hpp"

class Element;
class Serializer;

// A list of file format ids
enum file_format : size_t {
    JSON = 1
};
// A list of data type ids, "J" indicates use by JSON format
enum data_types : size_t {
    NO_TYPE = 0,
    J_BOOL = 1,
    J_CHAR = 2,
    J_NUMBER = 3,
    J_STRING = 4,
    J_ARRAY = 5,
    J_OBJECT = 6,
    J_NULL = 7
};

/* The Element class is a container class that stores a raw void pointer and the type that should be used to access it. This class
 * provides the syntax for accessing the data stored in this pointer using JSON-like syntax. Casting an element will automatically 
 * check if the element is being casted to the correct type and, if so, will convert the void pointer to a reference of the requested
 * type. The Element class also handles setting the value of the void pointer and accessing child elements.
 * 
 * Example 1:
 * Serializer serializer = Serializer();
 * serializer["bool_value"] = true;
 * bool value = serializer["bool_value"];
 * 
 * In this example, the second line first calls Serializer::operator[](std::string) and creates an uninstantiated element. Then 
 * Element::operator=(bool) is called, which sets the newly created element to the rvalue of true. In the third line, we call
 * Serializer::operator[](std::string) again which will return the above element. Then Element::operator bool&() is implicitly called 
 * which converts the void pointer in the element to a bool and returns it as an lvalue reference. 
 * 
 * Example 2:
 * Serializer serializer = Serializer();
 * serializer["array"] = { 0, 1, 2, 3, 4 };
 * int x = serializer["array"][2];
 * 
 * In this example, the second line again calls Serializer::operator[](std::string), creating an uninstantiated element, and then 
 * Element::operator=(std::initializer_list<int>). This in turn calls the setArray() method which creates an element for each element
 * of the list and instantiates it with the corresponding value. In the third line, we call Serializer::operator[](std::string) which
 * returns the array element, but this time Element::operator[](int) is called which in turn calls Serializer::operator[](int) for the
 * subelement. This, in turn, returns an element reference, which is returned again by Element::operator[](int). Then, as before, 
 * Element::operator int&() is implicitly called which converts the subelement void pointer to an int and returns a reference.
 */
class Element {
    friend Serializer;
public:
    // Create an element that has no key (sometimes elements need not be mapped))
    Element(void* value, size_t type = NO_TYPE) : value(value), type(type) {}
    // Create an element with type and key
    Element(void* value, size_t type, std::string key) : value(value), type(type), key(key) {}

    // Provide casting overrides so that Elements can be used as if they were actual datatypes. Whenever being casted to a specific
    // data type, the Element will check that this is the same data type that used to set the value.
    operator bool&() { return *static_cast<bool*>(e_typeMismatch(J_BOOL)? NULL_VALUE : value); }
    operator char&() { return *static_cast<char*>(e_typeMismatch(J_CHAR)? NULL_VALUE : value); }
    operator int() { return static_cast<int>(*static_cast<double*>(e_typeMismatch(J_NUMBER)? NULL_VALUE : value)); }
    operator unsigned int() { return static_cast<unsigned int>(*static_cast<double*>(e_typeMismatch(J_NUMBER)? NULL_VALUE : value)); }
    operator float() { return static_cast<float>(*static_cast<double*>(e_typeMismatch(J_NUMBER)? NULL_VALUE : value)); }
    operator double&() { return *static_cast<double*>(e_typeMismatch(J_NUMBER)? NULL_VALUE : value); }
    operator std::string&() { return *static_cast<std::string*>(e_typeMismatch(J_STRING)? NULL_VALUE : value); }
    operator Serializer&() { return *static_cast<Serializer*>(e_serializerTypeMismatch()? NULL_VALUE : value); }
    operator void*() { return value; }

    operator glm::vec3();

    // Provide setting overrides to take values and convert them to void pointers, saving the type so they can be reaccessed correctly.
    Element& operator=(bool _value) { return set(_value, J_BOOL); }
    Element& operator=(char _value) { return set(_value, J_CHAR); }
    Element& operator=(int _value) { return set(static_cast<double>(_value), J_NUMBER); }
    Element& operator=(unsigned int _value) { return set(static_cast<double>(_value), J_NUMBER); }
    Element& operator=(float _value) { return set(static_cast<double>(_value), J_NUMBER); }
    Element& operator=(const std::string& _value) { return set(_value, J_STRING); }
    Element& operator=(const char* _value) { return set(std::string(_value), J_STRING); }
    Element& operator=(const Serializer& _value);
    Element& operator=(Serializer&& _value);        // move constructor is special case
    Element& operator=(std::nullptr_t _value) { return set(_value, J_NULL); }

    // Also provide the ability to initialize arrays via initializer lists.
    Element& operator=(std::initializer_list<double> _values) { return setArray(_values, J_NUMBER); }
    Element& operator=(std::initializer_list<std::string> _values) { return setArray(_values, J_STRING); }
    Element& operator=(glm::vec3 _value) { return setArray({_value.x, _value.y, _value.z}, J_NUMBER); }

    // Provide accession overrides to access subelements of map and array elements
    Element& operator[](std::string key);
    Element& operator[](const char* key) { return (*this)[std::string(key)]; }
    Element& operator[](size_t index);
    Element& operator[](int index) { return (*this)[static_cast<size_t>(index)]; }

    // Return the number of subelements a map or array element has.
    size_t size() const;

    // Print information about the element.
    void print() { if (e_typeMismatch(J_OBJECT)) return; printObject(); }
private:
    // Void pointer value used to return a reference to null data after throwing error.
    static void* NULL_VALUE;
    // Unassigend null element used to return an element reference afting throwing error.
    static Element NULL_ELEMENT;

    // Void pointer used to store data in generic form that can be current 
    void* value;

    // Indicates the data type of the element, which is set when value is set and is used to check that the memory is been accessed 
    // correctly.
    size_t type;
    // The key is saved to return error data.
    std::string key;

    // The set function can take any data type and encode it as a void pointer, recording the type it is set with.
    template <typename T>
    Element& set(const T& _value, size_t _type) {
        // if type has not been set, set it to the given type and encode the value.
        if (type == NO_TYPE) {
            type = _type; 
            value = new T(_value);
        // otherwise, value is being changed from a different value, check that it is the same type and change the value
        } else if (!e_typeMismatch(_type)) *static_cast<T*>(value) = _value;
        // if not the same type, set value to nullptr to avoid undefined behavior
        else value = nullptr;
        return *this;
    }
    // The set array function can take an initializer list and encode it into an array element that has subelements
    template <typename T>
    Element& setArray(std::initializer_list<T> _values, size_t _type) {
        // can only take an initializer list if the array has not already been initialized
        if (!e_alreadyInitialized()) {
            // set the type to array
            type = J_ARRAY;
            // call getSerializer() instead of "new" since the Serializer class has not been defined
            value = getSerializer();
            // iterate through each element of the list and create a new sub element
            for (T _value : _values) addArrayElement(new T(_value), _type);
        }
        return *this;
    }

    // Returns a dynamically allowed Serializer which can be used by function templates that are declared in the header before
    // Serializer is defined
    Serializer* getSerializer();
    // Adds a subelement to as an array element
    void addArrayElement(void* value, size_t type);

    // Print object element
    void printObject();

    // error handling

    // Checks if the data type being used to read from an element is the same as what was used to write to it. Returns true if they are
    // not the same.
    bool e_typeMismatch(size_t compare) const;
    // Checks if the data type used to write to an element was a Serializer type (array or object). Returns true if it was not.
    bool e_serializerTypeMismatch() const;
    // Checks if the index being used to access an element of an array is in bounds. Return true if out of bounds.
    bool e_outOfBounds(size_t index) const;
    // Checks if the element has already been initialized. Returns true if it has.
    bool e_alreadyInitialized() const;
};

/* The Serializer class is a recursive data structure that is used to convert between specific serialized file data and c++ objects. The
 * Serializer class is intended to be used to save class data in a way that creates human-editable save files that can be reopened on
 * subsequent program runs. The user can create a "save()" method in a class of their choosing that creates a Serializer object and
 * encodes it with information necessary for constructing the object in the same state. The serializer can be called upon to save this
 * serialized data to a universal serializing format (e.g., JSON). The user should then create a constructor that takes a Serializer
 * object, then reads the data from that object in the same way that it was previously encoded so that the object can be reconstructed.
 *
 * The Serializer uses the same data structure for storing data regardless of what file type is used to save the serialized data. The
 * Serializer has separate writing and parsing methods for each file format. As such, the object which calls upon the Serializer to
 * save and load object data does not need to know what file format the Serializer is reading from.
 *
 * The Serializer can take the form of an array, in which case it can be access just as an array would be using the following:
 *      serializer[0];
 * The Serializer can also take the form of a map to simulate the behavior of a class, where variable names are accessed via keys, e.g.:
 *      serializer["var_name"];
 * Since serializers are recursive, we can also call them as multiple arrays, using array and object/map as long as this correctly
 * corresponds to the structure encoded in the serializer. For example:
 *      serializer["array_of_objects"][2]["list_of_attributes"][3];
 * The serializer should return variables as the data type they were saved. However, since this procedure is ambiguous, it is sometimes
 * necessary to explicitly cast the result if there is no implicit cast, as follows:
 *      static_cast<double>(serializer["decimal_value"]);
 * Note that the data type of the data must match the data type it was encoded as. If this is not the case, the Serializer will throw
 * an error and return a default value. 
 */
class Serializer {
    friend Element;
public:
    // elementMap is used to retrieve elements based on their string keys to mimick object functionality.
    // Use shared pointers so that Serializer objects can be freely copied and elements can be shared between the map and array.
    std::map<std::string, std::shared_ptr<Element>> elementMap;
    // elements is an ordered array of elements that can be accessed by index to mimick array functionality.
    // Use shared pointers so that Serializer objects can be freely copied and elements can be shared between the map and array.
    std::vector<std::shared_ptr<Element>> elements;

    // Create an empty Serializer object.
    Serializer() = default;
    // Load a Serializer object from a file.
    Serializer(std::string path);
    // Copy a Serializer object from another Serializer object.
    Serializer(const Serializer& serializer);
    // Move a Serializer object from one object to another.
    Serializer(Serializer&& serializer);
    // Destroy a Serializer object.
    ~Serializer();

    // Set a Serializer object by copying another Serializer object.
    Serializer& operator=(const Serializer& serializer);
    // Set a Serializer object by moving another Serializer object's data to the new Serializer object.
    Serializer& operator=(Serializer&& serializer);

    // The [] operator can take a string key value and output an Element to simulate the Serializer's object functionality. Since this 
    // operator returns an lvalue, it can be used to either read from or write to the Serializer object. Inputting a key that does not 
    // already exist will create a new element.
    // NOTE: The Element struct must be resolved to a type listed above. This can be done explicitly or implicitly.
    Element& operator[](std::string key);
    // The [] operator can take an index and output and element to simulate the Serializer's array functionality. Since this operator 
    // returns an lvalue, it can be used to either read from or write to the Serializer object. The user can also add a new element to the 
    // Serializer by inputting an index that does not yet have an associated value. However, this value should follow immediately the 
    // previous existing value in the array. For example, if n elements are defined, using n+1 is okay but n+2 will give an error.
    // NOTE: The Element struct must be resolved to a type listed above. This can be done explicitly or implicitly.
    Element& operator[](size_t index);

    // These functions allow the used to add an element to the Serializer by specifying the type explicitly so as to avoid relying on
    // type casting. All functions convert the value to a void* and handle conversion to element in a separate method.
    // Add a boolean element to object Serializer.
    void addBool(std::string key, bool value) { addElement(key, new bool(value), J_BOOL); }
    // Add a boolean element to array Serializer.
    void addBool(bool value) { addElement(new bool(value), J_BOOL); }
    // Add a char element to object Serializer.
    void addChar(std::string key, char value) { addElement(key, new char(value), J_CHAR); }
    // Add a char element to array Serializer.
    void addChar(char value) { addElement(new char(value), J_CHAR); }
    // Add a double element to object Serializer.
    void addNumber(std::string key, double value) { addElement(key, new double(value), J_NUMBER); }
    // Add a double element to array Serializer.
    void addNumber(double value) { addElement(new double(value), J_NUMBER); }
    // Add a string element to object Serializer.
    void addString(std::string key, std::string value) { addElement(key, new std::string(value), J_STRING); }
    // Add a string element to array Serializer.
    void addString(std::string value) { addElement(new std::string(value), J_STRING); }
    // Add a array element to object Serializer. This will add a recursion step.
    void addArray(std::string key) { addElement(key, new Serializer, J_ARRAY); }
    // Add a array element to array Serializer. This will add a recursion step.
    void addArray() { addElement(new Serializer, J_ARRAY); }
    // Add a object element to object Serializer. This will add a recusion step.
    void addObject(std::string key) { addElement(key, new Serializer, J_OBJECT); }
    // Add a object element to array Serializer. This will add a recusion step.
    void addObject() { addElement(new Serializer, J_OBJECT); }

    // Check if the Serializer has a certain key mapped.
    bool has(std::string key) const { return elementMap.contains(key); }
    // Check if the Serializer has child serializers. 
    bool hasChildren() const {
        for (std::shared_ptr<Element> element : elements) if (element->type == J_ARRAY || element->type == J_OBJECT) return true;
        return false;
    }
    // Check if the Serializer is behaving as an array (arrays do not place elements in elementMap).
    bool isArray() const { return (elementMap.size() != elements.size()); }
    bool isMultiArray() const {
        if (!isArray()) return false;
        for (std::shared_ptr<Element> element : elements) 
            if (element->type != J_ARRAY || static_cast<Serializer*>(element->value)->hasChildren()) return false;
        return true;
    }
    bool isObject() const { return !isArray() && elements.size() > 0; }
    // Get the number of elements contained in the serializer.
    unsigned int size() const { return elements.size(); }

    // Print the elements in JSON format.
    void print() const { std::cout << getJSON() << std::endl; }
    // Save the serializer to a specific location. The file extension will determine the format.
    void save(std::string path);
private:
    // Construct a serializer from source script of a given file format and serializer type. This is typically used when a
    // Serializer object creates a child Serializer object. As such it can only be called by class methods.
    Serializer(std::string source, size_t file_format);

    // Add an object element of any specified type.
    void addElement(std::string key, void* value, size_t type);
    // Add an array element of any specified type.
    void addElement(void* value, size_t type);

    // Parse a JSON file and convert it to a Serializer object.
    void parseJSON(std::string path);
    // Create a JSON script from a Serializer object. This method is called externally.
    std::string getJSON() const;
    // Create a JSON script from a Serializer object. This method is called internally by this or other getJSON methods recursively.
    std::string getJSON(int tab) const;

    // TODO - Add bin parsing implementation
    void parseBin(std::string path, std::string scheme_path, size_t type) 
        { Serializer* scheme = new Serializer(scheme_path); parseBin(path, scheme, type); delete scheme; }
    void parseBin(std::string path, Serializer* scheme, size_t type);

    // Error handling

    // This error test is for arrays that are being treated of a single type. Returns true if array has elements of the wrong type.
    bool e_typeMismatch(size_t compare) const;
    // This error test is to check if a Serializer has been accessed as the proper data structure (array or object/map). Returns true
    // if it has been accessed incorrectly.
    bool e_improperAccession(size_t type) const;
};

#endif
