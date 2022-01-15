#include "PrismInputs.h"
#include "PrismRenderer.h"
#include "PrismAudioManager.h"
#include "LogicManager.h"
#include <algorithm>
#include <thread>
#include <iostream>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

struct PrismAppComponents {
    PrismRenderer* renderer = NULL;
    PrismAudioManager* audman = NULL;
    PrismInputs* inputmgr = NULL;
    LogicManager* logicmgr = NULL;
};

PrismAppComponents appComps;

static void mpos_callback(GLFWwindow* window, double xpos, double ypos) {
    PrismAppComponents* app = reinterpret_cast<PrismAppComponents*>(glfwGetWindowUserPointer(window));
    app->inputmgr->newMpos(xpos, ypos);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    PrismAppComponents* app = reinterpret_cast<PrismAppComponents*>(glfwGetWindowUserPointer(window));
    app->inputmgr->updateKeyPressState(key, scancode, action, mods);
}

static void mbut_callback(GLFWwindow* window, int button, int action, int mods) {
    PrismAppComponents* app = reinterpret_cast<PrismAppComponents*>(glfwGetWindowUserPointer(window));
    app->inputmgr->updateKeyPressState(button, 0, action, mods);
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    PrismAppComponents* app = reinterpret_cast<PrismAppComponents*>(glfwGetWindowUserPointer(window));
    glfwWaitEvents();
    if (!app->renderer->framebufferResized) app->renderer->framebufferResized = true;
}

void nextFrameCallBack(float framedeltat, PrismRenderer* renderer) {
    if (appComps.logicmgr != NULL) appComps.logicmgr->pushToRenderer(renderer);
}

int main() {
    // Resolution suggestion
    int WIDTH = 1280;
    int HEIGHT = 720;
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Prism", NULL, NULL);
    std::cout << "Windows init complete" << std::endl;

    // Init input manager and renderer
    PrismInputs inputmgr = PrismInputs();
    appComps.inputmgr = &inputmgr;
    std::cout << "input manager init complete" << std::endl;
    PrismRenderer renderer = PrismRenderer(window, nextFrameCallBack);
    appComps.renderer = &renderer;
    std::cout << "renderer init complete" << std::endl;
    PrismAudioManager audman = PrismAudioManager();
    appComps.audman = &audman;
    std::cout << "audio manager init complete" << std::endl;
    LogicManager logicmgr = LogicManager(&inputmgr, &audman, 1);
    appComps.logicmgr = &logicmgr;
    std::cout << "logic manager init complete" << std::endl;

    // Give addresses of renderer and input manager, so callbacks can use them
    glfwSetWindowUserPointer(window, &appComps);

    // Setup callbacks
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported()) {
        std::cout << "Raw mouse Input Supported!! Using raw input" << std::endl;
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    glfwSetCursorPosCallback(window, mpos_callback);
    glfwSetMouseButtonCallback(window, mbut_callback);
    glfwSetKeyCallback(window, key_callback);
    
    try {
        std::thread render_thread(&PrismRenderer::run, &renderer);
        std::thread logic_thread(&LogicManager::run, &logicmgr);

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
        render_thread.join();
        logicmgr.stop();
        logic_thread.join();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}