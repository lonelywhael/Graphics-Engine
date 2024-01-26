#include "gui/camera.hpp"


static const float CAMERA_SPEED = 5.0f;

Camera::Camera(const glm::vec3 pos, const glm::vec3 target, const float fov, const float aspectRatio) 
        : pos(pos), target(target), fov(fov), aspectRatio(aspectRatio) { 
    lookAt();   // update view matrix to look in direction of target; sets dir and up
    setProj();  // update proj matrix with fov/ar
}


void Camera::move(const glm::vec3 v, const float deltaTime) {
    // move camera incrementally in the v direction
    pos += v.x * CAMERA_SPEED * deltaTime * glm::cross(up, dir);    // move right (up x dir)
    pos += v.y * CAMERA_SPEED * deltaTime * up;                     // move up
    pos += v.z * CAMERA_SPEED * deltaTime * dir;                    // move forward (dir)
    // update view matrix
    setView();
}
void Camera::turnTo(const float yaw, const float pitch, const glm::vec3 up) {
    dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));  // xdir = cos(yaw)cos(pitch)
    dir.y = sin(glm::radians(pitch));                           // ydir = sin(pitch)
    dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));  // zdir = sin(yaw)cos(pitch)
    // normalize to remove rounding errors
    dir = glm::normalize(dir);
    // up = ||(dir x (up x dir))||
    this->up = glm::normalize(glm::cross(dir, glm::cross(up, dir)));
    // update view matrix
    setView();
}

void Camera::printPos() const {
    std::cout << "[\t";
    for (int j = 0; j < 3; j++) std::cout << pos[j] << ",\t";
    std::cout << "]" << std::endl;
}
void Camera::printView() const {
    for (int i = 0; i < 4; i++) {
        std::cout << "[\t";
        for (int j = 0; j < 4; j++) std::cout << view[i][j] << ",\t";
        std::cout << "]" << std::endl;
    }
}

void Camera::setDir() {
    // set dir based on target relative position
    dir = glm::normalize(target - pos);
    // up = dir x (Y x dir), fixed in dir-y plane
    up = glm::normalize(glm::cross(dir, glm::normalize(glm::cross(Y_AXIS, dir))));
}