#ifndef JSON_IO_HPP
#define JSON_IO_HPP

#define DEBUG_CONSTRUCTOR_CALLS false

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "file_io.hpp"

struct Element;
struct Array;
struct Format;

struct result;

enum data_types : int {
    J_BOOL = 0,
    J_CHAR = 1,
    J_NUMBER = 2,
    J_STRING = 3,
    J_ARRAY = 4,
    J_OBJECT = 5,
    J_NULL = 6
};
extern std::string getString(int data_type);

struct Element {
    friend Format;

    void* value;
    int type;
    std::string key;

    Element(void* value, int type) : value(value), type(type) {}
    Element(void* value, int type, std::string key) : value(value), type(type), key(key) {}

    operator bool&() { checkType(type, key, J_BOOL); return *static_cast<bool*>(value); }
    operator char&() { checkType(type, key, J_CHAR); return *static_cast<char*>(value); }
    operator int() { checkType(type, key, J_NUMBER); return static_cast<int>(*static_cast<double*>(value)); }
    operator unsigned int() { checkType(type, key, J_NUMBER); return static_cast<unsigned int>(*static_cast<double*>(value)); }
    operator float() { checkType(type, key, J_NUMBER); return static_cast<float>(*static_cast<double*>(value)); }
    operator double&() { checkType(type, key, J_NUMBER); return *static_cast<double*>(value); }
    operator void*() { return value; }
    operator std::string&() { checkType(type, key, J_STRING); return *static_cast<std::string*>(value); }
    operator glm::vec3();
    operator Format&();

    Element& operator=(bool _value) { return set(_value, J_BOOL); }
    Element& operator=(char _value) { return set(_value, J_CHAR); }
    Element& operator=(int _value) { return set(static_cast<double>(_value), J_NUMBER); }
    Element& operator=(unsigned int _value) { return set(static_cast<double>(_value), J_NUMBER); }
    Element& operator=(float _value) { return set(static_cast<double>(_value), J_NUMBER); }
    Element& operator=(const std::string& _value) { return set(_value, J_STRING); }
    Element& operator=(const char* _value) { return set(std::string(_value), J_STRING); }
    Element& operator=(const Format& _value);
    Element& operator=(Format&& _value);
    Element& operator=(std::nullptr_t _value) { return set(_value, J_NULL); }

    Element& operator=(std::initializer_list<double> _values) { 
        return setArray(_values, J_NUMBER); 
    }
    Element& operator=(std::initializer_list<std::string> _values) { return setArray(_values, J_STRING); }
    Element& operator=(glm::vec3 _value) { return setArray({_value.x, _value.y, _value.z}, J_NUMBER); }

    Element& operator[](std::string key);
    Element& operator[](const char* key) { return (*this)[std::string(key)]; }
    Element& operator[](int index);

    void print() { checkType(type, key, J_OBJECT); printObject(); }
    unsigned int size();
private:
    template <typename T>
    Element& set(const T& _value, int _type) {
        if (type == -1) {
            type = _type; 
            value = new T(_value);
        } else {
            checkType(type, key, _type);
            *static_cast<T*>(value) = _value; 
        }
        return *this;
    }
    template <typename T>
    Element& setArray(std::initializer_list<T> _values, int _type) {
        if (type != -1) std::cout << "ERROR::JSON_IO::ARRAY_ALREADY_INITIALIZED" << std::endl;
        else {
            type = J_ARRAY;
            value = getFormat();
            for (T _value : _values) addArrayElement(new T(_value), _type);
        }
        return *this;
    }
    bool checkType(int type, std::string key, int compare);

    Format* getFormat();
    Element& getObjectElement(std::string);
    void printObject();
    Element& getArrayElement(int);
    void addArrayElement(void*, int);
};

struct Format {
    friend Element;
public:
    std::map<std::string, std::shared_ptr<Element>> elementMap;
    std::vector<std::shared_ptr<Element>> elements;

    Format() = default;
    Format(std::string path);
    Format(const Format& format);
    Format(Format&& format);
    ~Format();

    Format& operator=(const Format& object);
    Format& operator=(Format&& object);

    Element& operator[](std::string key) {
        std::shared_ptr<Element> e;
        if (elementMap.contains(key)) e = elementMap[key];
        else {
            e = std::make_shared<Element>(nullptr, -1, key);
            elementMap.emplace(key, e);
            elements.push_back(e);
        }
        return *e; 
    }
    Element& operator[](int index) {
        std::shared_ptr<Element> e;
        if (index < elements.size()) e = elements[index];
        else if (index == elements.size()) {
            e = std::make_shared<Element>(nullptr, -1, std::to_string(index));
            elements.push_back(e);
        } else { std::cout << "ERROR::JSON_IO::OUT_OF_BOUNDS: Skipped indices when defining elements." <<std::endl; }
        return *e;
    }

    void addBool(std::string key, bool value) { addElement(key, new bool(value), J_BOOL); }
    void addChar(std::string key, char value) { addElement(key, new char(value), J_CHAR); }
    void addNumber(std::string key, double value) { addElement(key, new double(value), J_NUMBER); }
    void addString(std::string key, std::string value) { addElement(key, new std::string(value), J_STRING); }
    void addArray(std::string key) { addElement(key, new Format, J_ARRAY); }
    void addObject(std::string key) { addElement(key, new Format, J_OBJECT); }

    bool has(std::string key) const { return elementMap.contains(key); }
    bool isArray() const { return (elementMap.size() != elements.size()); }
    unsigned int size() const { return elements.size(); }

    void print() const { std::cout << getJSON(); }
    void save(std::string path);
private:
    Format(std::string source, int format, int type);

    void addElement(std::string key, void* value, int type);
    void addElement(void* value, int type);

    void parseJSON(std::string path, int type);
    std::string getJSON() const { return std::string{"{\n" + getJSON(0, false, "\n") + "}\n"}; }
    std::string getJSON(int tab, bool array, std::string breaker) const;
    size_t findCloseBrace(char open, char close, int start, std::string source) const;

    void parseBin(std::string path, std::string scheme_path, int type) 
        { Format* scheme = new Format(scheme_path); parseBin(path, scheme, type); delete scheme; }
    void parseBin(std::string path, Format* scheme, int type);
};

#endif
