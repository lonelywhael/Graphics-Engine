#include "gui/window.hpp"

// need to keep track of the number of windows open
static unsigned int nWindows = 0;

static void initiate() {
    //must initialize GLFW
    glfwInit();

    //sets GLFW to be compatible with OpenGL version 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, (int) VERSION);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, (int) (VERSION * 10) % 10);

    //sets GFLW to only use OpenGL core profile (not immediate, which is deprecated)
    unsigned int profile;
    switch(PROFILE) {
    case P_CORE: { profile = GLFW_OPENGL_CORE_PROFILE; } break;
    }
    glfwWindowHint(GLFW_OPENGL_PROFILE, profile);

    //tells GFLW not to use commands deprecated in OpenGL v. 3.3 (only needed for apple OS)
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
}
// close GLFW
static void terminate() { glfwTerminate(); }

//------------------------------------------------------------------------------------------------------------------------------------------

static const float MOUSE_SENSITIVITY = 0.1f;

// indicates whether a window is bound or not
static bool bound = false;
Window* boundWindow = nullptr;

// resizing callback function given to GFLW
void framebuffer_size_callback(GLFWwindow* window, int w, int h);

// public cursor variables that point to values in the bound window
static float lastX, lastY;  // records the last measured location of the mouse
static bool firstMouse;     // stays true for one frame to get good data for last x and y
// mouse callback function given to GFLW
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

Window::Window(const std::string title, const int x, const int y, const int w, const int h) : title(title) {
    if (nWindows++ == 0) initiate(); // when the first window opens, be sure to initiate GFLW

    // use GLFW to create a window
    window = glfwCreateWindow(w, h, title.c_str(), NULL, NULL);
    if (window == NULL) std::cout << "Failed to create GLFW window" << std::endl;

    // by default, bind new windows
    bind();

    // set the position of the new window
    glfwSetWindowPos(window, x, y);

    //because of system features (e.g. retina display), window w and h are not necessarily the same as the pixel w and h
    glfwGetFramebufferSize(window, &width, &height); // ask glfw for pixel w & h info
    glViewport(0, 0, width, height); //set opengl context w & h
}
Window::~Window() {
    // delete the window
    glfwDestroyWindow(window);
    // if there are no more windows open, terminate GLFW
    if (--nWindows == 0) terminate(); 
}

void Window::bind() {
    //bind the window object to the current main thread context
    glfwMakeContextCurrent(window);
    //initialize GLAD using the correct OS
    ///TODO: is this a problem? IDK!
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) std::cout << "Failed to initialize GLAD" << std::endl;

    // set bound window to this
    boundWindow = this;
    firstMouse = true;  // must be true for one frame so last x and y are set to the current mouse position

    bound = true;
}
void Window::unbind() {
    // unbind GLFW window and set everything to null
    glfwMakeContextCurrent(NULL);

    boundWindow = nullptr;

    bound = false;
}

// tell GLFW to use the framebuffer_size_callback function
void Window::enableResizing() { glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); }
void Window::enableCursor(const bool lockCursor) {
    // lock the cursor if lockCusrsor = true (fixes the cursor in the center of the screen)
    if (lockCursor) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // tell GFLW to use the mouse_callback function
    glfwSetCursorPosCallback(window, mouse_callback);
    firstMouse = true;
}
void Window::disableCursor() {
    // lock the cursor if lockCusrsor = true (fixes the cursor in the center of the screen)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    // tell GFLW to use the mouse_callback function
    glfwSetCursorPosCallback(window, NULL);
}

float Window::getDeltaT() {
    // get current time
    float currentFrame = glfwGetTime();
    // set deltaT as current time minus previously measured time
    float deltaT = currentFrame - lastFrame;
    // set previously measured time to current time
    lastFrame = currentFrame;
    return deltaT;
}

int Window::getWidth() {
    glfwGetFramebufferSize(window, &width, &height); // ask glfw for pixel w & h info
    return width;
}
int Window::getHeight() {
    glfwGetFramebufferSize(window, &width, &height); // ask glfw for pixel w & h info
    return height;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void framebuffer_size_callback(GLFWwindow* window, int w, int h) {
    // as long as there is a window bound, send the new with and height to the openGL viewport
    if (bound) {
        boundWindow->setWidth(w);
        boundWindow->setHeight(h);
        glViewport(0, 0, w, h);
    }
}
void null_mouse_callback(GLFWwindow* window, double xpos, double ypos) {}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (bound) { // check is there is actually a window bound
        // on the first frame, set lastX and lastY to the current position of the mouse rather than using their previous values
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        // update ∆x and ∆y and then set lastX and lastY to the current position
        float deltaX = xpos - lastX, deltaY = lastY - ypos; // reversed since y-coordinates range from bottom to top
        lastX = xpos;
        lastY = ypos;

        // multiply by small constant to avoid craziness
        deltaX *= MOUSE_SENSITIVITY;
        deltaY *= MOUSE_SENSITIVITY;

        boundWindow->setDeltaX(deltaX);
        boundWindow->setDeltaY(deltaY);
    }
}