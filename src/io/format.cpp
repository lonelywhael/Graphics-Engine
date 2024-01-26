#include "io/format.hpp"

std::string getString(int data_type) {
    std::string string;
    switch(data_type) {
    case J_BOOL: { string = "\"bool\""; } break;
    case J_CHAR: { string = "\"char\""; } break;
    case J_NUMBER: { string = "\"number\""; } break;
    case J_STRING: { string = "\"string\""; } break;
    case J_ARRAY: { string = "\"array\""; } break;
    case J_OBJECT: { string = "\"object\""; } break;
    case J_NULL: { string = "\"null\""; } break;
    default: { string =  "(" + std::to_string(data_type) + ")"; } break;
    }

    return string;
}

Format::Format(std::string path) {
    if (path.find(".json") != std::string::npos) {
        auto [source, size] = f_read(path, JSON);
        parseJSON(source, J_OBJECT);
        delete[] source;
    }
}
Format::Format(std::string source, int format, int type) {
    switch(format) {
    case JSON: { parseJSON(source, type); } break;
    }
}
Format::Format(const Format& format) {
    #if (DEBUG_CONSTRUCTOR_CALLS)
        std::cout << "Format copied" << std::endl;
    #endif

    for (auto [key, element] : format.elementMap)
        this->elementMap.emplace(key, element);
    for (std::shared_ptr<Element> element : format.elements)
        this->elements.push_back(element);
}
Format::Format(Format&& format) {
    #if (DEBUG_CONSTRUCTOR_CALLS)
        std::cout << "Format moved" << std::endl;
    #endif

    this->elementMap = std::move(format.elementMap);
    this->elements = std::move(format.elements);
    format.elementMap.clear();
    format.elements.clear();
}
Format::~Format() {
    #if (DEBUG_CONSTRUCTOR_CALLS)
        std::cout << "Format deleted" << std::endl;
    #endif

    for (std::shared_ptr<Element> element : elements) if (element->value != nullptr)
        switch(element->type) {
        case J_BOOL: { delete static_cast<bool*>(element->value); } break;
        case J_CHAR: { delete static_cast<char*>(element->value); } break;
        case J_NUMBER: { delete static_cast<double*>(element->value); } break;
        case J_STRING: { delete static_cast<std::string*>(element->value); } break;
        case J_ARRAY: case J_OBJECT: { delete static_cast<Format*>(element->value); } break;
        }
}


Format& Format::operator=(const Format& object) {
    for (auto [key, element] : object.elementMap)
        this->elementMap.emplace(key, element);
    for (std::shared_ptr<Element> element : object.elements)
        this->elements.push_back(element);

    return *this;
}
Format& Format::operator=(Format&& object) {
    this->elementMap = std::move(object.elementMap);
    this->elements = std::move(object.elements);
    object.elementMap.clear();
    object.elements.clear();

    return *this;
}


void Format::addElement(std::string key, void* value, int type) {
    if (value == nullptr) type = J_NULL;
    std::shared_ptr<Element> e = std::make_shared<Element>(value, type, key);
    elementMap.emplace(key, e);
    elements.push_back(e);
}
void Format::addElement(void* value, int type) {
    if (value == nullptr) type = J_NULL;
    std::shared_ptr<Element> e = std::make_shared<Element>(value, type, std::to_string(elements.size()));
    elements.push_back(e);
}


void Format::save(std::string path) {
    int format;
    if (path.find(".json") != std::string::npos) {
        std::string source = getJSON();
        f_write(path, source.c_str(), source.length(), JSON);
    } else if (path.find(".bin") != std::string::npos)  {
        //auto [source, size] = getBin();
        //f_write(path, source, size, BIN);
    }
}

void Format::parseJSON(std::string source, int type) {
    size_t start = 0, end = 0;
    std::string line, key;

    while(end != std::string::npos) {
        start = (type == J_OBJECT) ? source.find('\"', end) + 1 : end + 1;
        if (type == J_OBJECT) key = source.substr(start, source.find('\"', start) - start);
    
        size_t obj = source.find('{', start), arr = source.find('[', start);
        end = source.find(',', start);
        std::shared_ptr<Element> element;
        if (std::min(obj, arr) < end) {
            element = (arr < obj) ? std::make_shared<Element>(new Format(source.substr(arr + 1, (end = findCloseBrace('[', ']', arr, source)) - arr - 1), JSON, J_ARRAY), J_ARRAY) :
                                    std::make_shared<Element>(new Format(source.substr(obj + 1, (end = findCloseBrace('{', '}', obj, source)) - obj - 1), JSON, J_OBJECT), J_OBJECT) ;
            end = source.find(',', end);
        } else {
            if (type == J_OBJECT) start = source.find(':', start) + 1;
            line = source.substr(start, end - start);
            size_t c;
            if ((c = line.find('\'')) != std::string::npos) element = std::make_shared<Element>(new char(line[c + 1]), J_CHAR);
            else if ((c = line.find('\"')) != std::string::npos) 
                element = std::make_shared<Element>(new std::string(line.substr(c + 1, line.find('\"', c + 1) - c - 1)), J_STRING);
            else if (line.find("true") != std::string::npos) element = std::make_shared<Element>(new bool(true), J_BOOL);
            else if (line.find("false") != std::string::npos) element = std::make_shared<Element>(new bool(false), J_BOOL);
            else if (line.find("null") != std::string::npos) element = std::make_shared<Element>(nullptr, J_NULL);
            else element = std::make_shared<Element>(new double(std::stod(line.substr(1))), J_NUMBER);
        }
        if (type == J_OBJECT) {
            element->key = key;
            elementMap.emplace(key, element);
        } else element->key = std::to_string(elements.size());
        elements.push_back(element);
    }
}
std::string Format::getJSON(int tab, bool array, std::string breaker) const {
    std::string source, tabs(tab + 1, '\t');
    for (int e = 0; e < elements.size(); e++) {
        if (elements[e]->type == -1) continue;
        if (!array) source += tabs + '\"' + elements[e]->key + "\" : ";
        switch(elements.at(e)->type) {
        case J_BOOL: { (*static_cast<bool*>(elements[e]->value)) ? source += "true" : source += "false"; } break;
        case J_CHAR: { source += '\'' + *static_cast<char*>(elements[e]->value) + '\''; } break;
        case J_NUMBER: { source += std::to_string(*static_cast<double*>(elements[e]->value)); } break;
        case J_STRING: { source += '\"' + *static_cast<std::string*>(elements[e]->value) + '\"'; } break;
        case J_ARRAY: { 
            int type = static_cast<Format*>(elements[e]->value)->elements[0]->type;
            std::string b = (type == J_ARRAY || type == J_OBJECT) ? "\n" : " ";
            source += "[" + b + static_cast<Format*>(elements[e]->value)->getJSON(tab + 1, true, b) + ((b == "\n") ? tabs : "") + "]"; 
        } break;
        case J_OBJECT: { 
            source += ((source.length() == 0 || source[source.length() - 1] == '\n') ? tabs : "") + "{\n" + static_cast<Format*>(elements[e]->value)->getJSON(tab + 1, false, "\n") + tabs + "}";
        } break;
        case J_NULL: { source += "null"; } break;
        }
        (e != elements.size() - 1) ? source += ',' + breaker : source += breaker;
    }
    return source;
}
size_t Format::findCloseBrace(char open, char close, int start, std::string source) const {
    std::string substr = source.substr(start);

    size_t n_open = 1, n_closed = 0, o, c, i = substr.find(open);
    while (n_open != n_closed) {
        o = substr.find(open, i + 1);
        c = substr.find(close, i + 1);
        (o < c) ? (i = o, n_open++) : (i = c, n_closed++);
        if (i > substr.length()) {
            std::cout << "ERROR::FORMAT::MISSING_BRACE: \'" << close << '\'' << std::endl;
            std::cout << substr.substr(0, i) << std::endl;
            return std::string::npos;
        }
    }

    return start + i;
}


Element::operator glm::vec3() { 
    checkType(type, key, J_ARRAY);
    checkType((*static_cast<Format*>(value))[0].type, "0", J_NUMBER);

    return glm::vec3 { *static_cast<float*>((*static_cast<Format*>(value))[0].value), 
                       *static_cast<float*>((*static_cast<Format*>(value))[1].value), 
                       *static_cast<float*>((*static_cast<Format*>(value))[2].value) };
}
Element::operator Format&() { 
    if (type != J_ARRAY && type != J_OBJECT) {
        std::cout << "ERROR::FORMAT::TYPE_MISMATCH: Incorrect type used for variable \"" << key << "\"." << 
                        " Used \"object\" or \"array\" and should be " << getString(type) << "." << std::endl;
    }
    return *static_cast<Format*>(value);
}

Element& Element::operator=(const Format& _value) { return set(_value, (_value.isArray()) ? J_ARRAY : J_OBJECT); }
Element& Element::operator=(Format&& _value) { 
    if (type == -1) {
        type = (_value.isArray()) ? J_ARRAY : J_OBJECT; 
        value = new Format(std::move(_value));
    } else {
        checkType(type, key, (_value.isArray()) ? J_ARRAY : J_OBJECT);
        *static_cast<Format*>(value) = std::move(_value); 
    }
    return *this;
}

Element& Element::operator[](std::string key) {
    if (type == -1) {
        type = J_OBJECT;
        value = getFormat();
    } else checkType(type, key, J_OBJECT);
    return getObjectElement(key); 
}
Element& Element::operator[](int index) {
    if (type == -1) {
        type = J_ARRAY;
        value = getFormat();
    } else checkType(type, key, J_ARRAY);
    return getArrayElement(index);
}

bool Element::checkType(int type, std::string key, int compare) {
    if (type != compare) {
        std::cout << "ERROR::FORMAT::TYPE_MISMATCH: Incorrect type used for variable \"" << key << "\"." << 
                     " Used " << getString(compare) << " and should be " << getString(type) << "." << std::endl;
        return false;
    } else return true;
}
unsigned int Element::size() { 
    if (type == -1) return 0;
    checkType(type, key, J_ARRAY); 
    return static_cast<Format*>(value)->size();
}

Format* Element::getFormat() { return new Format(); }
Element& Element::getObjectElement(std::string key) { return static_cast<Format*>(value)->operator[](key); }
void Element::printObject() { (*static_cast<Format*>(value)).print(); }
Element& Element::getArrayElement(int index) { return static_cast<Format*>(value)->operator[](index); }
void Element::addArrayElement(void* _value, int _type) { static_cast<Format*>(value)->addElement(_value, _type); }