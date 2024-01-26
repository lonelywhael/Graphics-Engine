#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <iostream>
#include <string>
#include <vector>

//simplifies the process of retrieving system-specific function pointers that would otherwise have to be individually clarified
#include <glad/glad.h>

//implements OpenGL functionality to the specific context of Apple in order to draw windows and such
#include <GLFW/glfw3.h>

#include "elements.hpp"

// key binding ids
enum key {
    KEY_SPACE = GLFW_KEY_SPACE,
    KEY_A = GLFW_KEY_A,
    KEY_D = GLFW_KEY_D,
    KEY_S = GLFW_KEY_S,
    KEY_W = GLFW_KEY_W,
    KEY_ESCAPE = GLFW_KEY_ESCAPE,
    KEY_LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT,
    KEY_LEFT_CONTROL = GLFW_KEY_LEFT_CONTROL
};
enum button {
    BUTTON_LEFT_CLICK = GLFW_MOUSE_BUTTON_LEFT
};

/* WINDOW CLASS
 * 
 * The window class handles the creation of a GFLW window as well as user inputs from the OS. The class allows a user to set window
 * parameters like size and position, and toggle certain parameters like resizability. The user can also access user input variables, like
 * key presses and mouse movement and respond accordingly.
 * 
 * Windows work within a context-like space organized by GFLW. As such, they exhibit a kind of correspondence to objects within the GFLW
 * space. As such, Windows should not be copied, but instead passed by pointers and references.
 */

class Window;

extern Window* boundWindow;

class Window {
public:
    // need a title and spatial dimensions (in pixels)
    Window(const std::string title, const int x, const int y, const int width = 1440, const int height = 1280);
    // need a destructor to know whether GFLW should be terminated
    ~Window();

    // windows must be bound in order to receive user inputs
    void bind();
    void unbind();

    // after images have been rendered on the openGL framebuffer, the GFLW window must be update so that the framebuffer becomes visible
    void update() const {
        //window uses a double buffer-one to render new data to and one to display in real time; swap to update
        glfwSwapBuffers(window);
        //checks for inputs
        glfwPollEvents();
    }

    // allows window to be resized
    void enableResizing();
    // enables cursor movement as a user input. Cursor can be either locked in the middle of the screen or free to move.
    void enableCursor(const bool lockCursor);
    void disableCursor();

    // checks if a key is pressed (see above enum for bindings)
    bool keyPressed(const int key) const { return (glfwGetKey(window, key) == GLFW_PRESS); }
    bool mouseButtonPressed(const int button) { return glfwGetMouseButton(window, button) == GLFW_PRESS; }

    // tells the window to close on the next frame
    void setToClose() const { glfwSetWindowShouldClose(window, true); }
    // checks whether the window should close
    bool shouldClose() const { return glfwWindowShouldClose(window); }

    // returns the window's aspect ratio
    float getAspectRatio() const { return (float) width / (float) height; }
    // returns how much the cursor has moved since the previous check
    float getDeltaX() { float x = deltaX; deltaX = 0; return x; }
    float getDeltaY() { float y = deltaY; deltaY = 0; return y; }
    void setDeltaX(const float deltaX) { this->deltaX = deltaX; }
    void setDeltaY(const float deltaY) { this->deltaY = deltaY; }

    // returns the current time
    float getTime() const { return glfwGetTime(); }
    // returns the amount of time that has passed since the previous check
    float getDeltaT();

    int getWidth();
    int getHeight();
    void setWidth(const int w) { width = w; }
    void setHeight(const int h) { height = h; }
private:
    // the GFLW window object
    GLFWwindow* window;
    std::string title;    // window title

    int width, height;          // window width and height

    float deltaX, deltaY;       // amount the cursor has moved since previous check

    float lastFrame = 0;        // reference time used to calculate ∆t
};

#endif