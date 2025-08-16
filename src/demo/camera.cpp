#include "../demo/camera.h"
#include "../demo/inputManager.h"
#include <iostream>

using namespace demo;

Camera::Camera(GLFWwindow* window) : window(window)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    lastX = width / 2.0;
    lastY = height / 2.0;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    InputManager::getInstance().SetCamera(this);
}

glm::mat4 Camera::GetViewMatrix() const
{
    glm::mat4 cameraTranslation = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 cameraRotation = GetRotationMatrix();
    return glm::inverse(cameraTranslation * cameraRotation);
}

glm::mat4 Camera::GetRotationMatrix() const
{
    glm::quat pitchRotation = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat yawRotation = glm::angleAxis(yaw, glm::vec3(0.0f, -1.0f, 0.0f));
    return glm::toMat4(yawRotation) * glm::toMat4(pitchRotation);
}

void Camera::HandleKeyInput(int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        keys[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        keys[key] = false;
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

void Camera::HandleMouseInput(double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.005f;
    yaw += (float)xoffset * sensitivity;
    pitch += (float)yoffset * sensitivity;

    if (pitch > 1.57f) pitch = 1.57f;
    if (pitch < -1.57f) pitch = -1.57f;
}

void Camera::HandleMouseButtonInput(int button, int action, int mods)
{

}

void Camera::Update(float deltaTime)
{
    velocity = glm::vec3(0.0f);

    if (keys[GLFW_KEY_W]) velocity.z = -0.05f;
    if (keys[GLFW_KEY_S]) velocity.z = 0.05f;
    if (keys[GLFW_KEY_A]) velocity.x = -0.05f;
    if (keys[GLFW_KEY_D]) velocity.x = 0.05f;

    glm::mat4 cameraRotation = GetRotationMatrix();
    position += glm::vec3(cameraRotation * glm::vec4(velocity * 0.5f * deltaTime, 0.0f));
}