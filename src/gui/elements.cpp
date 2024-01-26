#include "gui/elements.hpp"

const float VERSION = 3.3;
const unsigned int PROFILE = P_CORE;

void printMat(glm::mat4 mat) {
    // function that prints out a 4 by 4 matrix to the terminal
    for (int i = 0; i < 4; i++) {
        std::cout << "{ " << mat[i][0];
        for (int j = 1; j < 4; j++) std::cout << ", " << mat[i][j];
        std::cout << " }" << std::endl;
    }
    std::cout << std::endl;
}