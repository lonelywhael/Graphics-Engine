#include "io/serializer.hpp"

// Allocate memory for the default null void point value to be derefenced and passed back when throwing errors. Use size_t because it
// is the largest possible amount of memory for a single value.
void* Element::NULL_VALUE = malloc(sizeof(size_t));
// Instantiate an empty element to be returned as a reference when throwing errors
Element Element::NULL_ELEMENT = Element(NULL_VALUE, NO_TYPE);

// A list of type names that corresponds to the data type enum
const std::string TYPE_NAMES[] = { "N/A (no type)", "\"bool\"", "\"char\"", "\"number\"", "\"string\"", "\"array\"", 
                                   "\"object\"", "\"null\"" };

Element::operator glm::vec3() { 
    // casting to glm::vec3 requires that the element is an array with a size of 3 and that all subelements are numbers
    if (e_typeMismatch(J_ARRAY) || e_outOfBounds(3) || static_cast<Serializer*>(value)->e_typeMismatch(J_NUMBER)) return glm::vec3(0);

    // cast all elements to floats and return as a glm::vec3
    return glm::vec3 { *static_cast<float*>((*static_cast<Serializer*>(value))[0].value), 
                       *static_cast<float*>((*static_cast<Serializer*>(value))[1].value), 
                       *static_cast<float*>((*static_cast<Serializer*>(value))[2].value) };
}

// as with other casting operators, but isArray() cannot be called in the header as the Serializer class is uninstantiated
Element& Element::operator=(const Serializer& _value) { return set(_value, (_value.isArray()) ? J_ARRAY : J_OBJECT); }
Element& Element::operator=(Serializer&& _value) { 
    // similar to set() function, but use move semantics with the Serializer
    if (type == NO_TYPE) {
        type = (_value.isArray()) ? J_ARRAY : J_OBJECT; 
        value = new Serializer(std::move(_value));
    } else if (!e_typeMismatch(_value.isArray() ? J_ARRAY : J_OBJECT))
        *static_cast<Serializer*>(value) = std::move(_value);
    else value = nullptr;
    return *this;
}

Element& Element::operator[](std::string key) {
    // if element is uninstantiated, instantiate as object
    if (type == NO_TYPE) {
        type = J_OBJECT;
        value = new Serializer();
    }
    // check instantiation type is object, if so cast to serializer and call the subelement's accession operator
    return e_typeMismatch(J_OBJECT)? NULL_ELEMENT : (*static_cast<Serializer*>(value))[key]; 
}
Element& Element::operator[](size_t index) {
    // as above
    if (type == NO_TYPE) {
        type = J_ARRAY;
        value = getSerializer();
    } 
    return e_typeMismatch(J_ARRAY)? NULL_ELEMENT : (*static_cast<Serializer*>(value))[index];
}

// check to make sure element is a serializer, then call the serializer size() method
size_t Element::size() const { return e_serializerTypeMismatch()? 0 : static_cast<Serializer*>(value)->size(); }

Serializer* Element::getSerializer() { return new Serializer(); }
void Element::addArrayElement(void* _value, size_t _type) { static_cast<Serializer*>(value)->addElement(_value, _type); }

void Element::printObject() { static_cast<Serializer*>(value)->print(); }

bool Element::e_typeMismatch(size_t compare) const {
    *static_cast<size_t*>(NULL_VALUE) = 0;
    if (compare != type) {
        std::cout << "ERROR::ELEMENT::TYPE_MISMATCH: Incorrect type used for variable \"" << key << "\"." 
                  << " Used " << TYPE_NAMES[compare] << " and should be " << TYPE_NAMES[type] << "." << std::endl;
        return true;
    }
    return false;
}
bool Element::e_serializerTypeMismatch() const {
    *static_cast<size_t*>(NULL_VALUE) = 0;
    if (type != J_ARRAY && type != J_OBJECT) {
        std::cout << "ERROR::SERIALIZER::TYPE_MISMATCH: Incorrect type used for variable \"" << key << "\"." << 
                        " Used \"object\" or \"array\" and should be " << TYPE_NAMES[type] << "." << std::endl;
        return true;
    }
    return false;
}
bool Element::e_outOfBounds(size_t index) const { 
    *static_cast<size_t*>(NULL_VALUE) = 0;
    if (index > size()) {
        std::cout << "ERROR::ELEMENT::ARRAY_OUT_OF_BOUNDS: Serializer array accessed at index " << index << " but has a size of "
                    << size() << "." << std::endl;
        return true;
    } 
    return false;
}
bool Element::e_alreadyInitialized() const { 
    *static_cast<size_t*>(NULL_VALUE) = 0;
    if (type != NO_TYPE) {
        std::cout << "ERROR::ELEMENT::ARRAY_ALREADY_INITIALIZED: Attempted to reinitialize and already initialized array." 
                    << std::endl;
        return true;
    }
    return false;
}



//-----------------------------------------------------------------------------------------------------------------------------------



Serializer::Serializer(std::string path) {
    // check for the file extension and call the appropriate parsing reading and parsing function
    if (path.find(".json") != std::string::npos) {
        // copy file at path location to string, do not suppress comments as JSON does not allow comments
        std::string source;
        f_readText(path, source, false);
        // remove spaces and line breaks (except those within quotation marks) to avoid ambiguity over expected substr lengths
        source = p_removeAllExceptFlagged(source, {' ', '\n'}, '\"');
        // pass source contain within the outermost braces, and use those braces to determine whether contents should be parsed as an
        // object or an array
        parseJSON(source);
    }
}
Serializer::Serializer(std::string source, size_t file_format) {
    // private constructor
    // pass a script snippet along to the appropriate parser
    switch(file_format) {
    case JSON: { parseJSON(source); } break;
    }
}
Serializer::Serializer(const Serializer& serializer) {
    #if (DEBUG_CONSTRUCTOR_CALLS)
        std::cout << "Serializer copied" << std::endl;
    #endif

    // copy all elements over to the new Serializer.
    for (auto [key, element] : serializer.elementMap)
        this->elementMap.emplace(key, element);
    for (std::shared_ptr<Element> element : serializer.elements)
        this->elements.push_back(element);
}
Serializer::Serializer(Serializer&& Serializer) {
    #if (DEBUG_CONSTRUCTOR_CALLS)
        std::cout << "Serializer moved" << std::endl;
    #endif

    // move all elements over to the new Serializer, then clear the containers of elements.
    this->elementMap = std::move(Serializer.elementMap);
    this->elements = std::move(Serializer.elements);
    Serializer.elementMap.clear();
    Serializer.elements.clear();
}
Serializer::~Serializer() {
    #if (DEBUG_CONSTRUCTOR_CALLS)
        std::cout << "serializer deleted" << std::endl;
    #endif

    // convert element data to their appropriate type and delete them
    for (std::shared_ptr<Element> element : elements) if (element->value != nullptr)
        switch(element->type) {
        case J_BOOL: { delete static_cast<bool*>(element->value); } break;
        case J_CHAR: { delete static_cast<char*>(element->value); } break;
        case J_NUMBER: { delete static_cast<double*>(element->value); } break;
        case J_STRING: { delete static_cast<std::string*>(element->value); } break;
        case J_ARRAY: case J_OBJECT: { delete static_cast<Serializer*>(element->value); } break;
        }
}


Serializer& Serializer::operator=(const Serializer& object) {
    // essentially identicle to the copy constructor
    for (auto [key, element] : object.elementMap)
        this->elementMap.emplace(key, element);
    for (std::shared_ptr<Element> element : object.elements)
        this->elements.push_back(element);

    return *this;
}
Serializer& Serializer::operator=(Serializer&& object) {
    // essentially identicle to the move constructor
    this->elementMap = std::move(object.elementMap);
    this->elements = std::move(object.elements);
    object.elementMap.clear();
    object.elements.clear();

    return *this;
}

Element& Serializer::operator[](std::string key) {
    std::shared_ptr<Element> e;
    if (e_improperAccession(J_OBJECT)) return *e;
    // If the key already exists, then simply read from or write to the corresponding element.
    if (elementMap.contains(key)) e = elementMap[key];
    // Otherwise, create a new element.
    else {
        // Create a new element with no value or type, but uses the input key.
        // Typically, this operator is called along with the "=" operator so that these values are set immediately.
        e = std::make_shared<Element>(nullptr, NO_TYPE, key);
        elementMap.emplace(key, e);
        elements.push_back(e);
    }
    return *e; 
}
Element& Serializer::operator[](size_t index) {
    std::shared_ptr<Element> e;
    // Check if serializer is a map, if so this method should not be used
    if (e_improperAccession(J_ARRAY)) return *e;
    // Check if index is already defined, if so return associated value
    else if (index < elements.size()) e = elements[index];
    // If the index is one more than the existing array size, add a new element
    else if (index == elements.size()) {
        // As above, create a new element with no value or type, us the index as a key
        // TODO - can we get rid of this key?
        e = std::make_shared<Element>(nullptr, NO_TYPE, std::to_string(index));
        elements.push_back(e);
    // If the index exceeds the array size by more than one, throw an error and do nothing.
    } else { e->e_outOfBounds(index); return Element::NULL_ELEMENT; }
    return *e;
}

void Serializer::addElement(std::string key, void* value, size_t type) {
    // check proper accession
    if (e_improperAccession(J_OBJECT)) return;
    // a null value overrides whatever type was given
    if (value == nullptr) type = J_NULL;
    // create new Element
    std::shared_ptr<Element> e = std::make_shared<Element>(value, type, key);
    elementMap.emplace(key, e);
    elements.push_back(e);
}
void Serializer::addElement(void* value, size_t type) {
    // as above
    if (e_improperAccession(J_ARRAY)) return;
    if (value == nullptr) type = J_NULL;
    std::shared_ptr<Element> e = std::make_shared<Element>(value, type, std::to_string(elements.size()));
    elements.push_back(e);
}


void Serializer::save(std::string path) {
    // use the file extension from the path to determine the file format, then convert the Serializer to that format and send to file
    if (path.find(".json") != std::string::npos) {
        std::string source = getJSON();
        f_writeText(path, source);
    }
}

void Serializer::parseJSON(std::string source) {
    // use outermost flag characters to determine object type
    // it is assumed that the first char is an open brace and the last char is a close brace
    size_t type = (source[0] == '[') ? J_ARRAY : J_OBJECT;

    size_t p = 1;
    std::string var, key;
    // in general, all parsing operations will need to enable escape char because it is legal for strings to contain flag characters

    // keep parsing until the parsing index reaches the end of the source (excluding the final close brace)
    while(p < source.length() - 1) {
        // if parsing an object, look for keys
        // "key" : ...
        if (type == J_OBJECT) {
            key = p_getFlaggedSubstr(source, p, {'\"'}, P_ENABLE_ESCAPE_CHAR);  // take next string as key
            p = p_getNext(source, p, ':', P_ENABLE_ESCAPE_CHAR) + 1;            // parser jumps past next ':'
        }
    
        // There are three possibilities:
        // {...}, -- variable that follows key is an object
        // [...], -- variable that follows key is an array
        // ..., -- variable that follows key is something else
        // We can see that by looking for braces and comma and seeing which comes first determines which of these options is used.
        // There is also the possibility that there is no comma in which case our search will return npos.
        size_t next = p_getFirstIndexOf(source, p, {'{', '[', ','}, P_ENABLE_ESCAPE_CHAR);
        std::shared_ptr<Element> element;
        // if a brace comes first, we know the variable is either an object or an array
        if (source[next] == '[' || source[next] == '{') {
            // parse to the closing brace and return the (inclusive) substring within
            std::string source_substr = p_getFlaggedSubstr(source, p, {{'{', '}'}, {'[', ']'}}, P_ENABLE_ESCAPE_CHAR | P_INCLUSIVE);
            // pass the substr to a new Serializer object which will parse it (using this method) as a JSON, recursion, etc.
            element = std::make_shared<Element>(new Serializer(source_substr, JSON), (source_substr[0] == '[')? J_ARRAY : J_OBJECT);
            p = next + source_substr.length() + 1; // parser jumps past substr and the following comma
        // in the case that the next symbol is a comma, then we need to parse the "..."
        } else {
            // read everything from the current parser location to the comma (non-inclusive) into a string called var
            var = p_getNextSubstr(source, p, ',', P_ENABLE_ESCAPE_CHAR);
            // look for certain indications of data types, convert the appropriate substring to that type, construct element
            // not that becasue we are looking for "first of", the presence of multiple will be overridden by the first
            std::string type_flag = p_getFirstOf(var, { "\'", "\"", "true", "false", "null", "." }, P_ENABLE_ESCAPE_CHAR);
            // if no flags were found, the only possible conclusion is that the value was an integer
            if (type_flag.length() == 0) { element = std::make_shared<Element>(new double(std::stoll(var)), J_NUMBER); }
            else switch(type_flag[0]) {
            // 'v' -- if there is a single quote, then the value is a char
            case '\'': { element = std::make_shared<Element>(new char(var[1]), J_CHAR); } break;
            // "var" -- if there is a double quote, then the value is a string
            case '\"': { element = std::make_shared<Element>(new std::string(p_getFlaggedSubstr(var, '\"', P_NONINCLUSIVE)), J_STRING); } break;
            // true -- if there is the exact phrase "true" or "false", then the value is a bool
            case 't': case 'f': { element = std::make_shared<Element>(new bool((type_flag[0] == 't') ? true : false), J_BOOL); } break;
            // null -- if there is the exact phrase "null", then the value is null
            case 'n': { element = std::make_shared<Element>(nullptr, J_NULL); } break;
            // 1.2 -- if the first value is a period, then the value is a double
            case '.': { element = std::make_shared<Element>(new double(std::stod(var)), J_NUMBER); } break;
            }
            p += var.length() + 1; // move parser after line and following comma
        }
        // if this is an object, then add the key to the element and emplace the element on the map
        if (type == J_OBJECT) {
            element->key = key;
            elementMap.emplace(key, element);
        // otherwise use the index as the key
        } else element->key = std::to_string(elements.size());
        // add the element to the array of elements
        elements.push_back(element);
    }
}
std::string Serializer::getJSON() const { return std::string { getJSON(0) }; }
std::string Serializer::getJSON(int tabs) const {
    // child objects and arrays will need to be indented by the same amount on each new line, so we create a tab string that is added
    // after each line break. The _tab string is simply an additional indent that can be used after a brace.
    const int TAB = 4; // defines the size of a tab in terms of spaces, since \t does not always mean the same thing
    std::string source = "", tab(tabs * TAB, ' '), _tab((tabs + 1) * TAB, ' ');

    // JSON files are meant to be human-readable, and this is typically easier to do for objects if there are line breaks between 
    // elements. For arrays, it is easier with line breaks if the array contains child elements and not just normal data types.
    // o_flag and c_flag are braces, square for arrays and curly for objects
    // breaker appears after the open brace and after each element. end_breaker appears after the last element only.
    std::string o_flag, c_flag, breaker, end_breaker;
    if (isArray()) {
        o_flag = "["; c_flag = "]";
        // ... [
        //      ...,
        //      ...   
        // ]
        if (hasChildren()) { breaker = "\n" + _tab; end_breaker = "\n" + tab; }
        // [ ..., ... ]
        else { breaker = " "; end_breaker = " "; }
    } 
    // ... {
    //      ...,
    //      ...
    // }
    else if (isObject()) 
        { o_flag = "{"; c_flag = "}"; breaker = "\n" + _tab;  end_breaker = "\n" + tab; } 
    // ... []
    else { o_flag = "["; c_flag = "]"; breaker = ""; end_breaker = ""; }
    source += o_flag + breaker;

    // add elements
    for (int e = 0; e < elements.size(); e++) {
        // if element type is not defined, skip
        if (elements[e]->type == NO_TYPE) continue;
        // if this is an object, add keys before each element
        if (isObject()) source += "\"" + elements[e]->key + "\" : ";
        // convert elements to their appropiate types and print in the correct format
        switch(elements.at(e)->type) {
            // bools are printed as the string "true" or "false"
            case J_BOOL: { (*static_cast<bool*>(elements[e]->value)) ? source += "true" : source += "false"; } break;
            // chars are printed with a single quote on either side
            case J_CHAR: { source += '\'' + *static_cast<char*>(elements[e]->value) + '\''; } break;
            // numbers are printed either as a real number with a decimal point or an integer (long long) without a decimal point
            case J_NUMBER: { 
                double value = *static_cast<double*>(elements[e]->value);
                // if casting the value to a long long doesn't change it, then print it as an integer, otherwise as a real number
                source += (value != static_cast<double>(static_cast<long long>(value))) ? 
                                    std::to_string(value):
                                    std::to_string(static_cast<long long>(value)); 
            } break;
            // strings are printed with a double quote on either side
            case J_STRING: { source += '\"' + *static_cast<std::string*>(elements[e]->value) + '\"'; } break;
            // for arrays and objects, recursively call this method again, all formatting will be handled by the child
            case J_ARRAY: case J_OBJECT: { source += static_cast<Serializer*>(elements[e]->value)->getJSON(tabs + 1); } break;
            // any null type will be printed as the string "null"
            case J_NULL: { source += "null"; } break;
        }
        // all elements will print breaker, except the last element which will print end_breaker
        source += (e != elements.size() - 1) ? "," + breaker : end_breaker;
    }
    // add the closing flag
    source += c_flag;
    return source;
}

bool Serializer::e_typeMismatch(size_t compare) const {
    for (std::shared_ptr<Element> element : elements) if (element->type != compare) {
        std::cout << "ERROR::SERIALIZER::TYPE_MISMATCH: Serializer array accessed as vector of type " << TYPE_NAMES[compare]
                  << " but at least one element is of type " << TYPE_NAMES[element->type] << "." << std::endl;
        return true;
    } 
    return false;
}
bool Serializer::e_improperAccession(size_t type) const {
    if ((type == J_ARRAY && isObject()) || (type == J_OBJECT && isArray())){
        std::cout << "ERROR::SERIALIZER::IMPROPER_ACCESSION: Serializer of " << (isObject() ? "object" : "array")
                    << " type accessed as " << TYPE_NAMES[type] << "." << std::endl;
        return true;
    }
    return false;
}