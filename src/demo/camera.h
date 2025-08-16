#ifndef VRAKTAL_DEMO_CAMERA_H
#define VRAKTAL_DEMO_CAMERA_H
#pragma once

#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>


namespace demo
{
    class Camera
    {
    public:
        Camera(GLFWwindow* window);

        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetRotationMatrix() const;
        void Update(float deltaTime);

        void HandleKeyInput(int key, int scancode, int action, int mods);
        void HandleMouseInput(double xpos, double ypos);
        void HandleMouseButtonInput(int button, int action, int mods);

        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);

        float pitch = 0.0f;
        float yaw = 0.0f;

    private:
        GLFWwindow* window;

        bool firstMouse = true;
        double lastX = 0.0;
        double lastY = 0.0;

        bool keys[1024] = { false };
    };
}

#endif //VRAKTAL_DEMO_CAMERA_H
