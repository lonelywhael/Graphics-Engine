#include "io/file_io.hpp"

std::tuple<const char*, size_t> f_read(std::string path, int format) {
    std::ios_base::openmode mode = std::ios::in;
    if (format == BIN) mode |= std::ios::binary;

    std::ifstream file;
    size_t size;
    char* source;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        (mode == 0) ? file.open(path) : file.open(path, mode);
        // get length of file and allocate memory of the appropriate size
        file.seekg (0, file.end);
        size = file.tellg();
        file.seekg (0, file.beg);
        source = new char[size];  // created as a char pointer in order to use increment of single bytes in pointer arithmetic

        // copy file data to dynamic memory location
        file.read(source, size);
    } catch(std::ifstream::failure e) { std::cout << "ERROR::FILE_IO::FILE_NOT_SUCCESFULLY_READ: " << path << std::endl; }
    return { source, size };
}
void f_write(std::string path, const char* source, size_t size, int format) {
    std::ios_base::openmode mode = std::ios::out;
    if (format == BIN) mode |= std::ios::binary;

    std::ofstream file;
    file.exceptions(std::ofstream::badbit);

    (mode == 0) ? file.open(path) : file.open(path, mode);
    file.write(source, size * sizeof(char));
    file.close();
}


bool f_exists(std::string path) {
    std::ifstream file;
    file.open(path);
    return (bool) file;
}