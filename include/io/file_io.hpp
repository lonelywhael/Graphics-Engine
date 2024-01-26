#ifndef FILE_IO_HPP
#define FILE_IO_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <tuple>

enum file_format {
    BIN = 0,
    JSON = 1,
    TXT = 2,
    GLSL = 3
};

extern std::tuple<const char*, size_t> f_read(std::string path, int format);
extern void f_write(std::string path, const char* source, size_t size, int format);

extern bool f_exists(std::string path);

#endif