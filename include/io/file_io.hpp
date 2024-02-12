#ifndef FILE_IO_HPP
#define FILE_IO_HPP

#include <fstream>
#include <iostream>
#include <string>

#include "parser.hpp"

// This function reads text from the file at the given path and writes it to the input string. Defaults to remove comment notation
// from the text, but this option can be turned off.
extern std::string& f_readText(const std::string path, std::string& file_contents, bool suppress_comments = true);
// This function reads binary from a file and copies it to a char array.
extern char* f_readBinary(const std::string path, char* data, const size_t length);

// This function copies text to a file.
extern void f_writeText(const std::string path, const std::string text);
// This function copies binary data to a file.
extern void f_writeBinary(const std::string path, const char* const data, const size_t length);

// This function returns true when there is a file at the given location.
extern bool f_exists(const std::string path);
// This function returns the length of a file.
extern size_t f_length(const std::string path);

#endif