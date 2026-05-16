#include "../include/editorCamera.h"
#include <core/input/input.h>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(core::Input& _input) : direction(0.0f, 0.0f, -1.0f)
{
    aspectRatio = 800.0f / 600.0f;
    projection = glm::perspectiveLH_ZO(
        glm::radians(45.0f),
        aspectRatio,
        0.1f,
        1000.0f
    );
    projection[1][1] *= -1;

    cameraPosition = glm::vec3(0.0f, 0.0f, 5.0f);

    _input.BindMouseCallback<Camera, &Camera::Look>(this);

    _input.AddAction("CameraLook");
    _input.BindActionKey({ input::Key::GLFW_MOUSE_BUTTON_RIGHT }, "CameraLook", false);
    _input.BindActionCallback<Camera, &Camera::EnableLook>("CameraLook", this, input::KeyState::Press);
    _input.BindActionCallback<Camera, &Camera::DisableLook>("CameraLook", this, input::KeyState::Release);

    _input.AddAxis2DAction("MoveCamera", input::Key::D, input::Key::A, input::Key::W, input::Key::S, false);
    _input.BindAxis2DCallack<Camera, &Camera::MoveCamera>("MoveCamera", this);

    _input.AddAction("MoveCameraUp");
    _input.BindActionKey({ input::Key::SPACE }, "MoveCameraUp", false);
    _input.BindActionCallback<Camera, &Camera::MoveCameraUp>("MoveCameraUp", this, input::KeyState::OnGoing);
}

void Camera::MoveCamera(glm::vec2 value)
{
    float speed = 0.1;
    cameraPosition += (value.y * speed) * direction;

    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cameraRight = glm::normalize(glm::cross(up, direction));

    cameraPosition += (value.x * speed) * cameraRight;
}

void Camera::MoveCameraUp()
{
    float speed = 0.05;
    cameraPosition.y += speed;
}

void Camera::Look(glm::vec2 mouseDelta)
{
    if (!bReceiveInputs)
    {
        return;
    }
    float sensitivity = 0.1f;
    static float yaw = -90.0f;
    static float pitch = 0.0f;
    yaw -= mouseDelta.x * sensitivity;
    pitch += mouseDelta.y * sensitivity;

    // Clamp the pitch to prevent flipping
    pitch = glm::clamp(pitch, -89.0f, 89.0f);

    // Calculate the new camera direction
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction = glm::normalize(direction);
}

glm::mat4 Camera::GetView() const
{
    return glm::lookAtLH(cameraPosition, cameraPosition + direction, glm::vec3(0.0f, 1.0f, 0.0f));
}
