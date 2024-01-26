#include <iostream>
#include <memory>

// vector and matrix arithmatic
#include <glm/glm.hpp>

#include "gui/camera.hpp"
#include "gui/elements.hpp"
#include "gui/frame_buffer.hpp"
#include "gui/model.hpp"
#include "gui/renderer.hpp"
#include "gui/scene.hpp"
#include "gui/shader.hpp"
#include "gui/texture.hpp"
#include "gui/vertex_array.hpp"
#include "gui/window.hpp"

#include "io/format.hpp"

// default fov angle and zoom angle (in degrees)
static const float FOV = 45, ZOOM = 5;

// tracks input to feed to camera
static glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 10.0f);
static float yaw = glm::degrees(glm::atan(-cameraPos.z / cameraPos.x)),
             pitch = glm::degrees(glm::atan(-cameraPos.y / std::sqrt(cameraPos.x * cameraPos.x + cameraPos.z * cameraPos.z)));
// tracks time increment to normalize input reactivity
static float deltaT;

Window window = Window("Shader Test", 0, 0, 800, 600);
std::shared_ptr<Camera> camera = std::make_shared<Camera>(cameraPos, FOV, window.getAspectRatio());;

// methods for dealing with user input
void handleKeystrokes();
void handleCursor();

int main() {

    // allow window to be resized
    window.enableResizing();
    // lock cursor in window to be be used for camera movement
    window.enableCursor(true);

    // allow renderer to use depth buffering
    //r_EnableWireframe();
    r_EnableFaceCulling();

    // create scene
    //Scene _scene = Scene(camera, "test.json");
    Scene scene = Scene(window.getWidth(), window.getHeight(), camera);
    scene.setPixelWidth(5);
    scene.enableAntiAliasing();
    scene.setShadowStyle(S_SHADOW_MAPPING);

    int rWidth = window.getWidth() / 5, rHeight = window.getHeight() / 5;
    
    // add textures
    std::shared_ptr<Texture> moomin = std::make_shared<Texture>(TEXTURE_2D, "moomin", ".jpg", FILTER_NEAREST, WRAPPER_CLAMP_TO_EDGE, 0);
    std::shared_ptr<Texture> skybox = std::make_shared<Texture>(TEXTURE_CUBE, "skybox", ".jpg", FILTER_LINEAR, WRAPPER_CLAMP_TO_EDGE, 0);
    std::shared_ptr<TextureGroup> container = std::make_shared<TextureGroup>();
    container->addTexture("container2", ".png", FILTER_NEAREST, WRAPPER_CLAMP_TO_EDGE, 0);
    container->addTexture("container2_sMap", ".png", FILTER_NEAREST, WRAPPER_CLAMP_TO_EDGE, 0);
    container->addTexture("matrix_emap", ".jpeg", FILTER_NEAREST, WRAPPER_CLAMP_TO_EDGE, 0);

    // add material (DSE map, using the above, with moderate shininess value)
    std::shared_ptr<Material> emerald = std::make_shared<Material>(glm::vec3(0.0215f, 0.1745f, 0.0215f),
                                                                   glm::vec3(0.07568f, 0.61424f, 0.07568f),
                                                                   glm::vec3(0.633f, 0.727811f, 0.633f),
                                                                   0.6f * 128);
    std::shared_ptr<Material> moominMap = std::make_shared<Material>(moomin->getSlot(), glm::vec3(0.2f), 16);
    std::shared_ptr<Material> containerMap = std::make_shared<Material>(container->getSlot(DIFFUSE), 
                                                                        container->getSlot(SPECULAR), 
                                                                        container->getSlot(EMISSION), 
                                                                        32);

    // add lights
    std::shared_ptr<Light> light1 = std::make_shared<Light>(glm::vec3(0.0f, -1.0f, 0.0f),                                                                    // 0: light 1
                                                            glm::vec3(0.2, 0.2, 0.2), glm::vec3(0.5, 0.5, 0.5), glm::vec3(1.0, 1.0, 1.0)),
                           light2 = std::make_shared<Light>(glm::vec3(0.0f, -1.0f, 0.0f),                                                                    // 0: light 1
                                                            glm::vec3(0.2, 0.2, 0.2), glm::vec3(0.5, 0.5, 0.5), glm::vec3(1.0, 1.0, 1.0));

    // add vertex arrays
    std::shared_ptr<VertexArray> plane = std::make_shared<VertexArray>();
    plane->makeHeightMap(500, PF_HILL, STATIC);
    std::shared_ptr<VertexArray> sphere = std::make_shared<VertexArray>();
    sphere->makeSphereMap(250, SF_NULL, STATIC);
    std::shared_ptr<VertexArray> cube = std::make_shared<VertexArray>("cube_textured.bin", STATIC);
    std::shared_ptr<VertexArray> cubeMap = std::make_shared<VertexArray>("cube_map.bin", STATIC);
    std::shared_ptr<Model> terrain = std::make_shared<Model>(plane, emerald, glm::vec3(0.0f), glm::vec3(10.0f));
    std::shared_ptr<Model> ball = std::make_shared<Model>(sphere, emerald, glm::vec3(0.0f, 2.0f, 0.0f));
    
    std::shared_ptr<Model> moomin_box = std::make_shared<Model>(cube, moominMap, moomin, glm::vec3(0.0f, 4.0f, 0.0f));
    std::shared_ptr<Model> containerBox = std::make_shared<Model>(cube, containerMap, container, glm::vec3(0.0f, 4.0f, 0.0f));

    std::shared_ptr<Model> skyboxModel = std::make_shared<Model>(cubeMap, skybox, R_SKYBOX);
    std::shared_ptr<Model> lightSource1 = std::make_shared<Model>(sphere, light1, glm::vec3(0.5f)), 
                           lightSource2 = std::make_shared<Model>(sphere, light2, glm::vec3(0.5f));

    scene.addLight(light1);
    scene.addLight(light2);
    scene.addModel(skyboxModel);
    scene.addModel(terrain);
    scene.addModel(ball);
    scene.addModel(moomin_box);

    // load scene
    scene.load();
    //scene.save("test.json");

    //scene.print();

    unsigned int frames = 0;
    float lastTime = window.getTime();
    //keep window open until closed
    while(!window.shouldClose()) {
        //update time interval since last frame
        if (window.getTime() - lastTime < 1.0f) frames++;
        else {
            lastTime = window.getTime();
            std::cout << "FPS: " << frames << std::endl;
            frames = 0;
        }
        deltaT = window.getDeltaT();

        //handle input
        //------------
        handleKeystrokes();                             // handle key input
        handleCursor();                                 // handle mouse input
        camera->setAspectRatio(window.getAspectRatio()); // handle window resizing

        //animations
        //----------
        float angle = 0.1 * window.getTime();
        float c = -1.0f;
        glm::vec3 lightPos1 (0.0f, c * std::sin(angle), c * std::cos(angle)), 
                  lightPos2 (c * std::cos(2 * angle), c * std::sin(2 * angle), 0.0f);
        //light1.setSpatial(lightPos1, glm::vec3(0.0f));
        //light2.setSpatial(lightPos2, glm::vec3(0.0f));
        light1->setSpatial(glm::vec3(0.0f), lightPos1);
        light2->setSpatial(glm::vec3(0.0f), lightPos2);
        //lightSource1->move(lightPos1);
        //lightSource2->move(lightPos2);

        //render
        //------
        scene.draw();    // draws the scene to a buffer

        //update window and check for new inputs
        //--------------------------------------
        window.update(); // swaps the buffered image with what is being displayed
    }
    return 0;
}

void handleKeystrokes() {
    if (window.keyPressed(KEY_ESCAPE)) window.disableCursor();              // press esc to exit program
    if (window.mouseButtonPressed(BUTTON_LEFT_CLICK)) window.enableCursor(true);
    if (window.keyPressed(KEY_W)) camera->move(Z_AXIS, deltaT);              // move forward
    if (window.keyPressed(KEY_S)) camera->move(-Z_AXIS, deltaT);             // move backwards
    if (window.keyPressed(KEY_A)) camera->move(X_AXIS, deltaT);              // move right
    if (window.keyPressed(KEY_D)) camera->move(-X_AXIS, deltaT);             // move left
    if (window.keyPressed(KEY_SPACE)) camera->move(Y_AXIS, deltaT);          // move up
    if (window.keyPressed(KEY_LEFT_SHIFT)) camera->move(-Y_AXIS, deltaT);    // move down
    if (window.keyPressed(KEY_LEFT_CONTROL)) camera->setFOV(ZOOM);           // zoom camera
    else camera->setFOV(FOV);
}
void handleCursor() {
    // read how much mouse has tracked, move camera accordingly
    yaw += window.getDeltaX();
    pitch += window.getDeltaY();
    if (pitch > 89) pitch = 89;         // camera can only look up as far as the ceiling
    else if (pitch < -89) pitch = -89;  // and down as far as the floor
    camera->turnTo(yaw, pitch);
}