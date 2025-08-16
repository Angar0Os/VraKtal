#ifndef VRAKTAL_DEMO_INPUT_MANAGER_H
#define VRAKTAL_DEMO_INPUT_MANAGER_H
#pragma once

#include <GLFW/glfw3.h>
#include <unordered_map>
#include <functional>

namespace demo
{
    class InputManager
    {
    public:
        static InputManager& getInstance();

        void Initialize(GLFWwindow* window);
        void SetCamera(class Camera* camera);

        static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
        static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    private:
        InputManager() = default;
        GLFWwindow* window = nullptr;
        Camera* camera = nullptr;

        void HandleKey(int key, int scancode, int action, int mods);
        void HandleMouse(double xpos, double ypos);
        void HandleMouseButton(int button, int action, int mods);
    };
}

#endif //VRAKTAL_DEMO_INPUT_MANAGER_H
