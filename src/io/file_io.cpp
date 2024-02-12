#include "io/file_io.hpp"

void e_fileNotRead(bool bad, const std::string path);
void e_fileNotWritten(bool bad, const std::string path);

std::string& f_readText(const std::string path, std::string& file_contents, bool suppress_comments) {
    size_t p;
    std::ifstream file;
    // catch fail and bad exceptions
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        // open ifstream with default read-only mode
        file.open(path);
        std::string line;
        // copy first line without adding a line break as long as the file is not empty
        std::getline(file, line);
        // look for comment flag, and if present only keep contents before flag
        //if (suppress_comments && (p = p_getNext(line, "//")) != std::string::npos) line = line.substr(0, p);
        file_contents += (!file.eof()) ? line : "";
        // continue appending lines until reaching the end of file (eof)
        while(!file.eof() && std::getline(file, line)) {
            //if (suppress_comments && (p = p_getNext(line, "//")) != std::string::npos) line = line.substr(0, p);
            if (line != "") file_contents += '\n' + line;
        }
        file.close();
    } catch(std::ifstream::failure e) { e_fileNotRead(file.bad(), path); }

    // remove comment blocks (easier to do here since they can extend over multiple lines)
    file_contents = suppress_comments ? p_removeFlagged(file_contents, "/*", "*/", P_INCLUSIVE) : 
                                        file_contents;
    file_contents = suppress_comments ? p_removeFlagged(file_contents, "//", "\n", P_INCLUDE_OPEN | P_IGNORE_EXTRANEOUS_CLOSE) : 
                                        file_contents;

    return file_contents;
}
char* f_readBinary(const std::string path, char* data, const size_t length) {
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        // read in binary mode
        file.open(path, std::ios::in | std::ios::binary);

        file.read(data, length * sizeof(char));     // copy the file data to the the RAM
    } catch(std::ifstream::failure e) { e_fileNotRead(file.bad(), path); }
    return data;
}


void f_writeText(const std::string path, const std::string text) {
    std::ofstream file;
    file.exceptions(std::ofstream::badbit | std::ofstream::failbit);
    try {
        // file in default write text mode
        file.open(path);
        // output text to file
        file << text;
        file.close();
    } catch (std::ofstream::failure e) { e_fileNotWritten(file.bad(), path); }
}
void f_writeBinary(const std::string path, const char* const data, const size_t length) {
    std::ofstream file;
    file.exceptions(std::ofstream::badbit | std::ofstream::failbit);

    try {
        // write to file in binary mode
        file.open(path, std::ios::out | std::ios::binary);
        // write given data to stream - will assume that data actually has the correct length
        file.write(data, length * sizeof(char));
        file.close();
    } catch (std::ofstream::failure e) { e_fileNotWritten(file.bad(), path); }
}


bool f_exists(const std::string path) {
    std::ifstream file;
    // attempt to open the file and return good status
    file.open(path);
    return file.good();
}
size_t f_length(const std::string path) {
    size_t length;
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        // read the file in binary as text mode does not always give accurate seekg results
        file.open(path, std::ios::in | std::ios::binary);
        // seek to the end of the file then report position as length
        file.seekg (0, file.end);
        length = file.tellg();
    } catch(std::ifstream::failure e) { e_fileNotRead(file.bad(), path); }
    return length;
}


void e_fileNotRead(bool bad, const std::string path) {
    std::cout << "ERROR::FILE_IO::FILE_NOT_SUCCESFULLY_READ: " << path << std::endl;
    if (bad) std::cout << "badbit error state: Reading/writing on i/o operation." << std::endl;
    else std::cout << "failbit error state: Logical error on i/o operation." << std::endl;
}
void e_fileNotWritten(bool bad, const std::string path) {
    std::cout << "ERROR::FILE_IO::FILE_NOT_SUCCESFULLY_WRITTEN: " << path << std::endl;
    if (bad) std::cout << "badbit error state: Reading/writing on i/o operation." << std::endl;
    else std::cout << "failbit error state: Logical error on i/o operation." << std::endl;
}