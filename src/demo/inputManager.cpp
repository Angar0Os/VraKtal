#include "../demo/inputManager.h"
#include "../demo/camera.h"

using namespace demo;

InputManager& InputManager::getInstance()
{
    static InputManager instance;
    return instance;
}

void InputManager::Initialize(GLFWwindow* window)
{
    this->window = window;

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);

    glfwSetWindowUserPointer(window, this);
}

void InputManager::SetCamera(Camera* camera)
{
    this->camera = camera;
}

void InputManager::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    InputManager* manager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (manager)
    {
        manager->HandleKey(key, scancode, action, mods);
    }
}

void InputManager::MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    InputManager* manager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (manager)
    {
        manager->HandleMouse(xpos, ypos);
    }
}

void InputManager::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    InputManager* manager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (manager)
    {
        manager->HandleMouseButton(button, action, mods);
    }
}

void InputManager::HandleKey(int key, int scancode, int action, int mods)
{
    if (camera)
    {
        camera->HandleKeyInput(key, scancode, action, mods);
    }
}

void InputManager::HandleMouse(double xpos, double ypos)
{
    if (camera) {
        camera->HandleMouseInput(xpos, ypos);
    }
}

void InputManager::HandleMouseButton(int button, int action, int mods)
{
    if (camera) {
        camera->HandleMouseButtonInput(button, action, mods);
    }
}