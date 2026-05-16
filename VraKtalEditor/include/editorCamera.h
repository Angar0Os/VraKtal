#pragma once
#include <glm/glm.hpp>
namespace core {
    class Input;
}

class Camera
{
public:
    float aspectRatio;
    glm::mat4 projection;
    glm::vec3 cameraPosition;
    glm::vec3 direction;

    Camera(core::Input& _input);

    ~Camera() {};

    void MoveCamera(glm::vec2 value);

    void MoveCameraUp();

    void Look(glm::vec2 mouseDelta);

    bool bReceiveInputs = false;

    void EnableLook() {bReceiveInputs = true;}
    void DisableLook() {bReceiveInputs = false;}
    glm::mat4 GetView() const;
};
